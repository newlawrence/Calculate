cmake_minimum_required(VERSION 3.5)
project(calculate)

set(CATCH_REPO "https://raw.githubusercontent.com/philsquared/Catch/master")
set(CATCH_URL "${CATCH_REPO}/single_include/catch.hpp")
set(CATCH_PATH "${CMAKE_CURRENT_SOURCE_DIR}/extern/include/catch.hpp")

if(NOT EXISTS "${CATCH_PATH}")
    file(DOWNLOAD "${CATCH_URL}" "${CATCH_PATH}")
endif()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/bin/")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/lib/")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/lib/")

if (${CMAKE_CXX_COMPILER_ID} MATCHES "Clang")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -O3 -std=c11")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -O3 -std=c++11")
elseif (${CMAKE_CXX_COMPILER_ID} MATCHES "GNU")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -O3 -std=c11")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -O3 -std=c++11")
elseif (${CMAKE_CXX_COMPILER_ID} MATCHES "MSVC")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Ox /sdl")
endif()

include_directories("${PROJECT_SOURCE_DIR}/include")
include_directories("${PROJECT_SOURCE_DIR}/extern/include")

file(GLOB sources "source/*.cpp")
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
add_library(calculate SHARED "${sources}")
target_compile_definitions(calculate PUBLIC "_CRT_SECURE_NO_WARNINGS")
add_library(calculate_static STATIC "${sources}")
target_compile_definitions(calculate_static PUBLIC "_CRT_SECURE_NO_WARNINGS")

file(GLOB tests "tests/*.cpp")
add_executable(unittests "${tests}")
target_link_libraries(unittests LINK_PUBLIC calculate)

add_executable(calc "examples/calc.c")
target_link_libraries(calc LINK_PUBLIC calculate)

add_executable(calcpp "examples/calc.cpp")
target_link_libraries(calcpp LINK_PUBLIC calculate)
