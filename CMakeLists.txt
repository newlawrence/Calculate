cmake_minimum_required(VERSION 3.5)
project(calculate)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/build/")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/build/")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/build/")

if (${CMAKE_CXX_COMPILER_ID} MATCHES "Clang")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -O3 -std=c11")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -O3 -std=c++11")
elseif (${CMAKE_CXX_COMPILER_ID} MATCHES "GNU")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -O3 -std=c11")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -O3 -std=c++11")
elseif (${CMAKE_CXX_COMPILER_ID} MATCHES "MSVC")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Ox")
endif()

include_directories(${PROJECT_SOURCE_DIR}/include)

file(GLOB sources "src/*.cpp")
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
add_library(calculate SHARED "${sources}")
add_library(calculate_static STATIC "${sources}")

add_executable(test_c "test/test.c")
target_link_libraries(test_c LINK_PUBLIC calculate)

add_executable(test_cpp "test/test.cpp")
target_link_libraries(test_cpp LINK_PUBLIC calculate)
