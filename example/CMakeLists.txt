cmake_minimum_required(VERSION 3.5)
project(Calculate-Examples LANGUAGES CXX)


if(CMAKE_SOURCE_DIR STREQUAL PROJECT_SOURCE_DIR)
    set(CMAKE_CONFIGURATION_TYPES "Release;Debug" CACHE STRING "Configurations" FORCE)
    if(NOT CMAKE_BUILD_TYPE)
        set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "Build type" FORCE)
    endif()
else()
    message(STATUS "Calculate: Examples enabled")
endif()
include("${CMAKE_BINARY_DIR}/example/conanbuildinfo.cmake")
conan_basic_setup(TARGETS NO_OUTPUT_DIRS)


add_executable(example "${CMAKE_CURRENT_SOURCE_DIR}/source/calculate.cpp")

set_target_properties(example
PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    OUTPUT_NAME calculate
)

if(CMAKE_SOURCE_DIR STREQUAL PROJECT_SOURCE_DIR)
    target_include_directories(example PRIVATE "${CMAKE_SOURCE_DIR}/../include")
else()
    target_include_directories(example PRIVATE "${CMAKE_SOURCE_DIR}/include")
endif()

target_compile_features(example PRIVATE cxx_std_14)

target_compile_options(example
PRIVATE
    "$<$<CXX_COMPILER_ID:GNU>: -pedantic -Wall -Wextra -Werror -Wno-noexcept-type>"
    "$<$<CXX_COMPILER_ID:Clang>: -pedantic -Wall -Wextra -Werror -Qunused-arguments>"
)

target_link_libraries(example
    CONAN_PKG::boost_system
    CONAN_PKG::boost_filesystem
    CONAN_PKG::boost_program_options
)
