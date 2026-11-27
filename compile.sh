#!/usr/bin/env bash

# Usage: ./compile.sh [--ver <x.y.z>] [--path <dir>] [third_party_dir]
#   ./compile.sh                                  # install into ./toolkit_0_1_0
#   ./compile.sh --ver 1.2.3                      # install into ./toolkit_1_2_3
#   ./compile.sh --path /tmp/out                  # install into /tmp/out
#   ./compile.sh --ver 1.2.3 /path/to/third_party # install into <dir>/toolkit_1_2_3

set -euo pipefail

cd "$(dirname "$0")"

BUILD_TYPE="${BUILD_TYPE:-Release}"
BUILD_DIR="${BUILD_DIR:-build}"

# --ver <x.y.z>   package version -> install dir "toolkit_<ver>"
#                 (dots become underscores: 0.1.0 -> toolkit_0_1_0)
# --path <dir>    exact export directory (highest priority)
# [third_party_dir]  install into <dir>/toolkit_<ver>
# PREFIX=<path>   exact install prefix (no toolkit_<ver> suffix is appended)
VERSION="0.1.0"
THIRD_PARTY_DIR=""
OUT_PATH=""

while [[ $# -gt 0 ]]; do
    case "$1" in
    --ver)
        if [[ $# -lt 2 ]]; then
            echo "compile.sh: --ver needs a value, e.g. --ver 1.2.3" >&2
            exit 1
        fi
        VERSION="$2"
        shift 2
        ;;
    --ver=*)
        VERSION="${1#--ver=}"
        shift
        ;;
    --path)
        if [[ $# -lt 2 ]]; then
            echo "compile.sh: --path needs a value, e.g. --path /tmp/out" >&2
            exit 1
        fi
        OUT_PATH="$2"
        shift 2
        ;;
    --path=*)
        OUT_PATH="${1#--path=}"
        if [[ -z "${OUT_PATH}" ]]; then
            echo "compile.sh: --path must not be empty" >&2
            exit 1
        fi
        shift
        ;;
    -*)
        echo "compile.sh: unknown option '$1'" >&2
        exit 1
        ;;
    *)
        THIRD_PARTY_DIR="$1"
        shift
        ;;
    esac
done

if [[ -z "${VERSION}" ]]; then
    echo "compile.sh: --ver must not be empty" >&2
    exit 1
fi

PKG_DIR="toolkit_${VERSION//./_}"

# Destination precedence:
#   --path <dir>        -> exactly <dir>
#   [third_party_dir]   -> <dir>/toolkit_<ver>
#   PREFIX=<dir>        -> exactly <dir>
#   (nothing)           -> <repo>/toolkit_<ver>
if [[ -n "${OUT_PATH}" ]]; then
    PREFIX="${OUT_PATH}"
elif [[ -n "${THIRD_PARTY_DIR}" ]]; then
    PREFIX="${THIRD_PARTY_DIR}/${PKG_DIR}"
else
    PREFIX="${PREFIX:-$(pwd)/${PKG_DIR}}"
fi

# cmake --install resolves a relative --prefix against the build dir, so make
# the prefix absolute before handing it over.
if [[ "${PREFIX}" != /* ]]; then
    PREFIX="$(pwd)/${PREFIX}"
fi

cmake -S src -B "${BUILD_DIR}" -DCMAKE_BUILD_TYPE="${BUILD_TYPE}" -DTOOLKIT_VERSION="${VERSION}"
cmake --build "${BUILD_DIR}" -j3

echo "Built:     $(ls "${BUILD_DIR}"/liblogkit.so* 2>/dev/null | tr '\n' ' ')"

cmake --install "${BUILD_DIR}" --prefix "${PREFIX}"
echo "Installed: ${PREFIX}"

# The demo is a separate project that consumes the *installed* package exactly
# like a third-party user: find_package(toolkit) against the prefix above. This
# is also the smoke test for the exported headers/targets.
DEMO_BUILD_DIR="${BUILD_DIR}/demo"

# Older revisions built demo/ as a subdirectory of the library build, so a stale
# cache may sit there. Its cached source dir would not match demo/; drop it.
if [[ -f "${DEMO_BUILD_DIR}/CMakeCache.txt" ]] &&
   ! grep -qx "CMAKE_HOME_DIRECTORY:INTERNAL=$(pwd)/demo" "${DEMO_BUILD_DIR}/CMakeCache.txt"; then
    rm -rf "${DEMO_BUILD_DIR}"
fi

cmake -S demo -B "${DEMO_BUILD_DIR}" \
    -DCMAKE_BUILD_TYPE="${BUILD_TYPE}" \
    -DCMAKE_PREFIX_PATH="${PREFIX}"
cmake --build "${DEMO_BUILD_DIR}" -j3

echo "Demo:      ${DEMO_BUILD_DIR}/logkit_log_demo"
echo "Tests:     ctest --test-dir ${BUILD_DIR} --output-on-failure"
