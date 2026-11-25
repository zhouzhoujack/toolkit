# Standalone demo project.
#
# It consumes the *installed* toolkit package exactly like a third-party user
# would (find_package + <toolkit/tlLog.h> + toolkit::logkit) instead of linking
# the in-tree target, so it doubles as a smoke test of the exported tree.
#
#   ./compile.sh                        # builds + installs into ./toolkit_0_1_0
#   # ...which then also does:
#   cmake -S demo -B build/demo -DCMAKE_PREFIX_PATH=<prefix>
#   cmake --build build/demo -j3
#   ./build/demo/logkit_log_demo

cmake_minimum_required(VERSION 3.16)
project(logkit_demo CXX)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(toolkit REQUIRED COMPONENTS logkit)

add_executable(logkit_log_demo 
    tlLog_demo.cpp
)

# toolkit::logkit carries <prefix>/include in its interface, so
# "#include <toolkit/tlLog.h>" resolves without any extra -I.
target_link_libraries(logkit_log_demo PRIVATE 
    toolkit::logkit
)
