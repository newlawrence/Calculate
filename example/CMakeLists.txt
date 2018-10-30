cmake_minimum_required(VERSION 3.5)
project(Calculate-Examples LANGUAGES CXX)


if(CMAKE_SOURCE_DIR STREQUAL PROJECT_SOURCE_DIR)
    set(CMAKE_CONFIGURATION_TYPES "Release;Debug" CACHE STRING "Configurations" FORCE)
    if(NOT CMAKE_BUILD_TYPE)
        set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "Build type" FORCE)
    endif()
else()
    message(STATUS "Calculate: examples enabled")
endif()

find_package(Boost QUIET)
if(EXISTS "${CMAKE_BINARY_DIR}/example/conanbuildinfo.cmake")
    include("${CMAKE_BINARY_DIR}/example/conanbuildinfo.cmake")
    conan_basic_setup(TARGETS NO_OUTPUT_DIRS)
    set(CALCULATE_EXAMPLES_USE_CONAN TRUE)
elseif(EXISTS "${CMAKE_BINARY_DIR}/conanbuildinfo.cmake")
    include("${CMAKE_BINARY_DIR}/conanbuildinfo.cmake")
    conan_basic_setup(TARGETS NO_OUTPUT_DIRS)
    set(CALCULATE_EXAMPLES_USE_CONAN TRUE)
elseif(Boost_FOUND)
    find_package(Boost REQUIRED COMPONENTS system filesystem program_options)
    include_directories(${Boost_INCLUDE_DIRS})
    string(REPLACE ";" "\n       " boost_libraries "${Boost_LIBRARIES}")
    set(CALCULATE_EXAMPLES_USE_CONAN FALSE)
else()
    message(FATAL_ERROR "Calculate: missing Boost libraries")
endif()


add_executable(example "${CMAKE_CURRENT_SOURCE_DIR}/source/calculate.cpp")

set_target_properties(example
PROPERTIES
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS NO
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
    $<$<CXX_COMPILER_ID:GNU>: -pedantic -Wall -Wextra -Werror -Wno-noexcept-type>
    $<$<CXX_COMPILER_ID:Clang>: -pedantic -Wall -Wextra -Werror -Qunused-arguments>
)

if(CALCULATE_EXAMPLES_USE_CONAN)
    target_link_libraries(example
        CONAN_PKG::boost_system
        CONAN_PKG::boost_filesystem
        CONAN_PKG::boost_program_options
    )
else()
    target_link_libraries(example ${Boost_LIBRARIES})
endif()
