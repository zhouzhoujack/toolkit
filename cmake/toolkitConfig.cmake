# Package config for the toolkit package. @PACKAGE_INIT@ below is expanded by
# configure_package_config_file() (see src/CMakeLists.txt), so the file that
# ends up as <prefix>/lib/cmake/toolkit/toolkitConfig.cmake is relocatable.
#
# Kits are exposed as toolkit::<kit>; today there is exactly one: logkit.
#   find_package(toolkit REQUIRED)
#   find_package(toolkit REQUIRED COMPONENTS logkit)
@PACKAGE_INIT@

include(CMakeFindDependencyMacro)

# The core library has no third-party dependency: zlib is linked into
# liblogkit.so (PRIVATE), so consumers do not need to find it.

# Component bookkeeping.  Add one line per kit as the package grows.
set(toolkit_logkit_FOUND TRUE)

include("${CMAKE_CURRENT_LIST_DIR}/toolkitTargets.cmake")

check_required_components(toolkit)
