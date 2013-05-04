cmake_minimum_required(VERSION 2.8)

# undocumented flags from here:
# http://stackoverflow.com/questions/1208681/with-cmake-how-would-you-disable-in-source-builds
set(CMAKE_DISABLE_SOURCE_CHANGES ON)
set(CMAKE_DISABLE_IN_SOURCE_BUILD ON)

project(sorter_all)

set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/CMakeModules/;${CMAKE_MODULE_PATH}")

enable_testing()
add_test(AutoTests ${EXECUTABLE_OUTPUT_PATH}/test_auto)

add_subdirectory(generator)
add_subdirectory(sorter)
add_subdirectory(tester)
add_subdirectory(tests)
