cmake_minimum_required(VERSION 3.5)
project(Calculate)

enable_testing()
set(CMAKE_CXX_STANDARD 14)

include_directories(include)
include_directories(tool/Catch2/single_include)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -pedantic -Wall -Werror")
if(CMAKE_BUILD_TYPE STREQUAL "Release")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3")
endif()

find_package(Boost COMPONENTS system filesystem program_options REQUIRED QUIET)
if(Boost_FOUND)
    include_directories(${Boost_INCLUDE_DIRS})
    add_executable(calculate example/calculate.cpp)
    target_link_libraries(calculate ${Boost_LIBRARIES})
endif()

file(GLOB TEST_SOURCES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} test/*.cpp)
foreach(test_source ${TEST_SOURCES})
    get_filename_component(test_name ${test_source} NAME_WE)
    add_executable(test_${test_name} ${test_source})
    add_test(NAME ${test_name} COMMAND ${CMAKE_CURRENT_BINARY_DIR}/${test_name})
endforeach()
