cmake_minimum_required(VERSION 3.9)
project(Calculate)

set(CMAKE_CXX_STANDARD 14)

include_directories(include)
include_directories(tool/Catch2/single_include)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -pedantic -Wall -Werror")
if(CMAKE_BUILD_TYPE AND (CMAKE_BUILD_TYPE STREQUAL "Debug"))
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3")
endif()

find_package(Boost COMPONENTS system filesystem program_options REQUIRED)
if(Boost_FOUND)
    message(STATUS "Boost_INCLUDE_DIRS: ${Boost_INCLUDE_DIRS}")
    message(STATUS "Boost_LIBRARIES: ${Boost_LIBRARIES}")
    message(STATUS "Boost_VERSION: ${Boost_VERSION}")
    include_directories(${Boost_INCLUDE_DIRS})
endif()

if(Boost_FOUND)
    add_executable(calculate example/calculate.cpp)
    target_link_libraries(calculate ${Boost_LIBRARIES})
endif()
