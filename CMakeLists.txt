cmake_minimum_required(VERSION 3.5)
project(Calculate)

enable_testing()
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include_directories(include)
include_directories(tool/Catch2/single_include)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pedantic -Wall -Werror")
if(CMAKE_BUILD_TYPE STREQUAL "Release")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3")
endif()

find_package(Boost COMPONENTS system filesystem program_options REQUIRED QUIET)
if(Boost_FOUND)
    include_directories(${Boost_INCLUDE_DIRS})
    string(REPLACE ";" "\n       " boost_libraries "${Boost_LIBRARIES}")
    message(STATUS "Boost version: ${Boost_VERSION}")
    message(STATUS "Boost libraries:\n       ${boost_libraries}")

    add_executable(calculate example/calculate.cpp)
    target_link_libraries(calculate ${Boost_LIBRARIES})
endif()

file(GLOB TEST_SOURCES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} test/*.cpp)
foreach(test_source ${TEST_SOURCES})
    get_filename_component(test_name ${test_source} NAME_WE)
    set(test_name test_${test_name})
    add_executable(${test_name} ${test_source})
    add_test(NAME ${test_name} COMMAND ${CMAKE_CURRENT_BINARY_DIR}/${test_name})
endforeach()
