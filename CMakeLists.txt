cmake_minimum_required(VERSION 3.5)
project(Calculate LANGUAGES CXX VERSION 2.1.1)


set(CMAKE_CONFIGURATION_TYPES "Release;Debug" CACHE STRING "Configurations" FORCE)
if(NOT CMAKE_BUILD_TYPE)
    if(EXISTS "${CMAKE_SOURCE_DIR}/.git")
        set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "Build type" FORCE)
    else()
        set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Build type" FORCE)
    endif()
endif()
message(STATUS "Calculate: Setting build type to '${CMAKE_BUILD_TYPE}'")


add_library(Calculate INTERFACE)

target_compile_features(Calculate INTERFACE cxx_std_14)

target_include_directories(Calculate
INTERFACE
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include>"
    "$<INSTALL_INTERFACE:include>"
)

add_library(Calculate::Calculate ALIAS Calculate)


if(CALCULATE_BUILD_EXAMPLES)
    add_subdirectory(example)
endif()

if(CALCULATE_BUILD_TESTS)
    enable_testing()
    add_subdirectory(test)
endif()
