# Unit tests for logkit.
#
# Each *_test.cpp becomes one CTest test binary; gtest_discover_tests()
# registers every individual TEST() case, so `ctest -N` shows them one by one.

find_package(GTest REQUIRED)
include(GoogleTest)

file(GLOB logkit_test_sources CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/*_test.cpp")

foreach(test_src IN LISTS logkit_test_sources)
    get_filename_component(test_name "${test_src}" NAME_WE)

    add_executable("${test_name}" "${test_src}")
    target_link_libraries("${test_name}" PRIVATE logkit GTest::gtest_main)

    # logkit's own include directories are PRIVATE, so the tests add them
    # explicitly (sources include "tlString.h" etc.).
    target_include_directories("${test_name}" PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/../logkit")

    gtest_discover_tests("${test_name}")
endforeach()
