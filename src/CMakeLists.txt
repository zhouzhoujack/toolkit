cmake_minimum_required(VERSION 3.16)

project(toolkit)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include("/home/hz/toolchain/cmake/toolchain.cmake")

find_package(ZLIB REQUIRED)

# CMAKE_INSTALL_{LIBDIR,INCLUDEDIR,...} must be defined before they are used in
# the INSTALL_INTERFACE generator expression below.
include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

# ---------------------------------------------------------------------------
# Library target
# ---------------------------------------------------------------------------
# All tl* sources in logkit/ (CONFIGURE_DEPENDS re-globs when files are added).
file(GLOB logkit_sources CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/logkit/*.cc")

add_library(logkit SHARED ${logkit_sources})

# atomic/spinlock.h includes "atomic.h" (same directory), so that directory
# must be on the include path.
target_include_directories(logkit PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/logkit
    ${CMAKE_CURRENT_SOURCE_DIR}/logkit/atomic)

# tlDeflate streams through zlib.
target_link_libraries(logkit PRIVATE ZLIB::ZLIB)

# A kit is exposed as toolkit::<kit>.  Only one kit exists today (logkit) and it
# produces liblogkit.so; further kits can be added to the same package as
# toolkit::<name> without renaming anything for consumers.
add_library(toolkit::logkit ALIAS logkit)

# Default symbol visibility: the whole logkit API is meant to be exported and is
# declared in plain headers, so no visibility preset / export-macro machinery is
# used -- same approach as the zlib tree in this toolchain.
# No VERSION/SOVERSION either: the artifact stays a plain liblogkit.so.

target_include_directories(logkit PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/logkit>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)

# ---------------------------------------------------------------------------
# Install / export
#
# Ships
#   <prefix>/include/toolkit/*.h      (consumer: #include <toolkit/tlLog.h>)
#   <prefix>/lib/liblogkit.so
#   <prefix>/lib/cmake/toolkit/       (find_package(toolkit) CONFIG-mode package)
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
# Public headers: tlLog.h + tlGLog.h
#
# tlLog.h is installed as the public header, but it is not self-contained: it includes
# tlCommon.h, tlString.h, tlThreads.h and tlObjectCollection.h, which pull in
# ~10 more.  Those are implementation details, so they are installed under
# include/toolkit/detail/ instead of next to the public header, and tlLog.h's
# quoted includes are rewritten to "detail/xxx.h" for the installed copy.
#
# tlGLog.h defines the LOGI/LOGW/LOGE/LOGF macros and is public as well -- it only
# does #include "tlLog.h", so it is installed verbatim.
#
# The relative includes among the detail headers keep working as-is (they are
# all siblings under detail/, and atomic/ stays a subdirectory because
# tlThreads.h says #include "atomic/spinlock.h").
# ---------------------------------------------------------------------------
file(GLOB logkit_headers CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/logkit/*.h")

set(logkit_detail_headers ${logkit_headers})
list(REMOVE_ITEM logkit_detail_headers
    "${CMAKE_CURRENT_SOURCE_DIR}/logkit/tlLog.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/logkit/tlGLog.h")

# Regenerate the installed copy whenever the source header changes.
set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/logkit/tlLog.h")

file(READ "${CMAKE_CURRENT_SOURCE_DIR}/logkit/tlLog.h" logkit_tllog_h)
string(REGEX REPLACE
    "#include \"(tl[A-Za-z0-9_]*\\.h)\""
    "#include \"detail/\\1\""
    logkit_tllog_h "${logkit_tllog_h}")
file(WRITE "${PROJECT_BINARY_DIR}/include/toolkit/tlLog.h" "${logkit_tllog_h}")

install(FILES "${PROJECT_BINARY_DIR}/include/toolkit/tlLog.h"
    DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/toolkit")

install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/logkit/tlGLog.h"
    DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/toolkit")

install(FILES ${logkit_detail_headers}
    DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/toolkit/detail")

install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/logkit/atomic"
    DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/toolkit/detail"
    FILES_MATCHING PATTERN "*.h")

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

