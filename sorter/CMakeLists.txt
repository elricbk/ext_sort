cmake_minimum_required(VERSION 2.8)

project(sorter)

add_executable(sorter main.cpp info_container.cpp)

include_directories(../)

set(Boost_USE_MULTITHREAD ON)
find_package(Boost COMPONENTS system program_options REQUIRED)
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

target_link_libraries(sorter ${Boost_LIBRARIES} ${LOG4CPP_LIBRARIES})
