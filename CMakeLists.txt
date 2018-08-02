cmake_minimum_required(VERSION 3.5)
project(Calculate LANGUAGES CXX VERSION 2.1.1)

if (CALCULATE_BUILD_TESTS)
    enable_testing()
    add_subdirectory(test)
endif()

add_library(Calculate INTERFACE)
target_include_directories(Calculate
INTERFACE
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include>"
)
add_library(Calculate::Calculate ALIAS Calculate)
