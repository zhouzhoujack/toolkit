cmake_minimum_required(VERSION 3.16)

# Library version, driven by ./compile.sh --ver (passed in as -DTOOLKIT_VERSION).
# Falls back to 0.1.0 when configured directly with a plain `cmake -S src -B build`.
if(NOT DEFINED TOOLKIT_VERSION OR TOOLKIT_VERSION STREQUAL "")
    set(TOOLKIT_VERSION "0.1.0")
endif()

project(toolkit VERSION ${TOOLKIT_VERSION})

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include("/home/hz/toolchain/cmake/toolchain.cmake")

# CMAKE_INSTALL_{LIBDIR,INCLUDEDIR,...} must be defined before they are used in
# the INSTALL_INTERFACE generator expression below.
include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

# ---------------------------------------------------------------------------
# Library target
# ---------------------------------------------------------------------------
# All lk* sources in logkit/ (CONFIGURE_DEPENDS re-globs when files are added).
file(GLOB logkit_sources CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/logkit/*.cc")

# Compile the sources once into an OBJECT library so both the shared library and
# the unit tests reuse the same objects. The tests exercise the internal API, so
# they link these objects (hidden symbols stay usable within the test exe) instead
# of the shared lib, which only exports the LK_EXPORT public API.
add_library(logkit_objects OBJECT ${logkit_sources})

# atomic/spinlock.h includes "atomic.h" (same directory), so that directory
# must be on the include path.
target_include_directories(logkit_objects PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/logkit
    ${CMAKE_CURRENT_SOURCE_DIR}/logkit/atomic)

# Only symbols marked LK_EXPORT (see lkCommon.h) are exported; everything else
# -- including inline functions and vtables of internal classes -- is hidden.
# Objects also carry hidden visibility; that is fine for the tests, since hidden
# symbols are still usable within the same link unit.
set_target_properties(logkit_objects PROPERTIES
    POSITION_INDEPENDENT_CODE ON
    CXX_VISIBILITY_PRESET hidden
    VISIBILITY_INLINES_HIDDEN ON)

# Dead-code elimination: per-function/-object sections let the linker drop
# everything in the .so that is not reachable from the exported API. The unit
# tests still link the full objects, so nothing covered by tests is lost.
target_compile_options(logkit_objects PRIVATE -ffunction-sections -fdata-sections)

add_library(logkit SHARED $<TARGET_OBJECTS:logkit_objects>)
target_link_options(logkit PRIVATE -Wl,--gc-sections)

# A kit is exposed as toolkit::<kit>.  Only one kit exists today (logkit) and it
# produces liblogkit.so; further kits can be added to the same package as
# toolkit::<name> without renaming anything for consumers.
add_library(toolkit::logkit ALIAS logkit)

# ELF version / SONAME derived from the package version (./compile.sh --ver X.Y.Z):
#   liblogkit.so.<X.Y.Z>  (real file)
#   liblogkit.so.<X>      (SONAME symlink; SOVERSION = major)
#   liblogkit.so          (dev symlink)
# Same major version => same SONAME => ABI-compatible drop-in upgrades.
set_target_properties(logkit PROPERTIES
    VERSION   ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR})

target_include_directories(logkit PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/logkit>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)

# ---------------------------------------------------------------------------
# Install / export
#
# Ships
#   <prefix>/include/toolkit/lkGLog.h       (public: #include <toolkit/lkGLog.h>)
#   <prefix>/include/toolkit/detail/*.h     (implementation details, lkLog.h included)
#   <prefix>/lib/liblogkit.so -> .so.<major> -> .so.<version>
#   <prefix>/lib/cmake/toolkit/             (find_package(toolkit) CONFIG-mode package)
#
#   find_package(toolkit REQUIRED)                  # or COMPONENTS logkit
#   target_link_libraries(app PRIVATE toolkit::logkit)
#
# compile.sh drives the prefix, e.g. ./compile.sh --ver 0.1.0 produces
#   toolkit_0_1_0/{include,lib}
# ---------------------------------------------------------------------------
install(TARGETS logkit
    EXPORT  toolkitTargets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

install(EXPORT toolkitTargets
    FILE      toolkitTargets.cmake
    NAMESPACE toolkit::
    DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/toolkit")

# ---------------------------------------------------------------------------
# Headers
#
# lkGLog.h is the single public header (installed to include/toolkit/). Its own
# #include "lkLog.h" is rewritten to "detail/lkLog.h" for the installed copy.
#
# Only the headers reachable from the public header (lkGLog.h -> lkLog.h -> ...)
# are installed, under include/toolkit/detail/. Headers that the implementation
# needs but a consumer does not (there are none right now - everything reachable
# from lkGLog.h is installed) would NOT be installed at all: their symbols are
# hidden anyway, so a consumer could not use them.
#
# KEEP THIS LIST IN SYNC: if a public header starts including a new tl*.h, add
# it here or consumers will fail to compile. It is the include-closure of
# lkGLog.h over the "lkXxx.h" quoted includes.
# ---------------------------------------------------------------------------
set(_logkit_installed_header_names
    lkCommon.h
    lkException.h
    lkHelper.h
    lkLog.h
    lkObject.h
    lkObjectCollection.h
    lkString.h
    lkThreads.h)

set(logkit_detail_headers)
foreach(_hdr IN LISTS _logkit_installed_header_names)
    set(_hdr_path "${CMAKE_CURRENT_SOURCE_DIR}/logkit/${_hdr}")
    if(NOT EXISTS "${_hdr_path}")
        message(FATAL_ERROR "logkit: installed header not found: ${_hdr_path}")
    endif()
    list(APPEND logkit_detail_headers "${_hdr_path}")
endforeach()

install(FILES ${logkit_detail_headers}
    DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/toolkit/detail")

install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/logkit/atomic"
    DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/toolkit/detail"
    FILES_MATCHING PATTERN "*.h")

# Regenerate the installed copy whenever the source header changes.
set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/logkit/lkGLog.h")

file(READ "${CMAKE_CURRENT_SOURCE_DIR}/logkit/lkGLog.h" logkit_tlglog_h)
string(REGEX REPLACE
    "#include \"(lk[A-Za-z0-9_]*\\.h)\""
    "#include \"detail/\\1\""
    logkit_tlglog_h "${logkit_tlglog_h}")
file(WRITE "${PROJECT_BINARY_DIR}/include/toolkit/lkGLog.h" "${logkit_tlglog_h}")

install(FILES "${PROJECT_BINARY_DIR}/include/toolkit/lkGLog.h"
    DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/toolkit")

# Generated under <build>/cmake/toolkit/ on purpose: if the install prefix is ever
# the build dir, a config at the prefix root would shadow the installed one
# (<prefix>/toolkitConfig.cmake is checked before <prefix>/lib/cmake/toolkit/)
# and could not find toolkitTargets.cmake next to it.
configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/../cmake/toolkitConfig.cmake"
    "${PROJECT_BINARY_DIR}/cmake/toolkit/toolkitConfig.cmake"
    INSTALL_DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/toolkit")

install(FILES
    "${PROJECT_BINARY_DIR}/cmake/toolkit/toolkitConfig.cmake"
    DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/toolkit")

# ---------------------------------------------------------------------------
# Unit tests (test/)
# ---------------------------------------------------------------------------
enable_testing()
add_subdirectory(test)

# NOTE: demo/ is deliberately *not* added here. It is a standalone project that
# consumes the installed package (see demo/CMakeLists.txt); compile.sh builds it
# with CMAKE_PREFIX_PATH pointing at the install prefix.

