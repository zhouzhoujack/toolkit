#!/usr/bin/env bash

# Remove build artifacts. Only directories inside this repo are touched.
#
# Usage:
#   ./clear.sh                  # remove build/, build-*/, logkit_*/ and toolkit*/ (install output)
#   ./clear.sh <dir> [...]      # remove the given build dirs only
#
# Safe by design: refuses to delete anything outside the repo root (or the repo
# root itself), never follows a symlink out of the tree, and needs no sudo.

set -euo pipefail

cd "$(dirname "$0")"
ROOT="$(pwd)"

if [[ $# -eq 0 ]]; then
    shopt -s nullglob
    dirs=(build/ build-*/ logkit_*/ toolkit*/)
    shopt -u nullglob
else
    dirs=("$@")
fi

removed=0
for dir in "${dirs[@]}"; do
    target="$(realpath -m -- "${dir}")"

    # Guard: only paths strictly below the repo root.
    if [[ "${target}" != "${ROOT}"/* ]]; then
        echo "clear.sh: refusing to remove '${dir}' (not inside ${ROOT})" >&2
        exit 1
    fi

    if [[ -e "${target}" ]]; then
        rm -rf -- "${target}"
        echo "removed: ${dir}"
        removed=$((removed + 1))
    else
        echo "skip (missing): ${dir}"
    fi
done

if [[ ${removed} -eq 0 ]]; then
    echo "nothing to remove"
fi
