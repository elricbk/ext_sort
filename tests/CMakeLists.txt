cmake_minimum_required(VERSION 2.8)

project(tests)

set(LIBRARY_OUTPUT_PATH ../)
set(EXECUTABLE_OUTPUT_PATH ../)

include_directories(../)

set(Boost_USE_MULTITHREAD ON)
find_package(Boost COMPONENTS timer system unit_test_framework REQUIRED)
if(NOT Boost_FOUND)
  message(SEND_ERROR "Failed to find Boost libraries")
  return()
else()
  include_directories(${Boost_INCLUDE_DIRS})
endif()

find_package(log4cpp REQUIRED)
if(NOT LOG4CPP_FOUND)
  message(SEND_ERROR "Failed to find log4cpp libraries")
  return()
else()
  include_directories(${LOG4CPP_INCLUDE_DIRS})
endif()

add_executable(test_auto 
  test_auto.cpp
  ../sorter/info_container.cpp
  ../sorter/input_buffer.cpp)

add_executable(test_input_buffer 
  test_input_buffer.cpp
  ../sorter/input_buffer.cpp)

target_link_libraries(test_auto ${Boost_LIBRARIES} ${LOG4CPP_LIBRARIES})
target_link_libraries(test_input_buffer ${Boost_LIBRARIES} ${LOG4CPP_LIBRARIES})
