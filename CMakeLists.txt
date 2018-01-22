cmake_minimum_required(VERSION 3.5)
project(Calculate)

enable_testing()
set(CMAKE_CONFIGURATION_TYPES "Release;Debug")
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include_directories(include)
include_directories(tool/Catch2/single_include)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/release/bin/")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}/debug/bin/")

if(WIN32 AND NOT MINGW AND NOT MSYS AND NOT CYGWIN)
    if(MSVC)
        set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS} /Ox")
        set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS} /Od")
    else()
        set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS} /O3")
        set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS} /O0")
    endif()
    set(CMAKE_CXX_FLAGS "")
else()
    if(
        (${CMAKE_CXX_COMPILER_ID} MATCHES "GNU") OR
        (${CMAKE_CXX_COMPILER_ID} MATCHES "Clang")
    )
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pedantic -Wall -Werror")
    endif()
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS} -O3")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS} -O0")
    set(CMAKE_CXX_FLAGS "")
endif()

if(MSVC)
    add_definitions(-DBOOST_ALL_DYN_LINK)
    set(Boost_USE_STATIC_LIBS OFF)
    set(Boost_USE_MULTITHREADED ON)
    set(Boost_USE_STATIC_RUNTIME OFF)
endif()
find_package(Boost COMPONENTS system filesystem program_options)
if(Boost_FOUND)
    include_directories(${Boost_INCLUDE_DIRS})
    string(REPLACE ";" "\n       " boost_libraries "${Boost_LIBRARIES}")
    message(STATUS "Boost version: ${Boost_VERSION}")
    message(STATUS "Boost libraries:\n       ${boost_libraries}")

    add_executable(calculate example/calculate.cpp)
    if(MSVC)
        target_link_libraries(calculate
            optimized ${Boost_SYSTEM_LIBRARY_RELEASE}
            debug ${Boost_SYSTEM_LIBRARY_DEBUG}
        )
        target_link_libraries(calculate
            optimized ${Boost_FILESYSTEM_LIBRARY_RELEASE}
            debug ${Boost_FILESYSTEM_LIBRARY_DEBUG}
        )
        target_link_libraries(calculate
            optimized ${Boost_PROGRAM_OPTIONS_LIBRARY_RELEASE}
            debug ${Boost_PROGRAM_OPTIONS_LIBRARY_DEBUG}
        )
    else()
        target_link_libraries(calculate ${Boost_LIBRARIES})
    endif()
endif()

file(GLOB TEST_SOURCES RELATIVE "${CMAKE_SOURCE_DIR}" test/*.cpp)
foreach(test_source ${TEST_SOURCES})
    get_filename_component(test_name "${test_source}" NAME_WE)
    set(test_name "test_${test_name}")
    add_executable("${test_name}" "${test_source}")
    if(MSVC)
        set_target_properties("${test_name}"
        PROPERTIES
            RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/release/test/"
            RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}/debug/test/"
        )
        add_test(
            NAME "${test_name}"
            COMMAND "${CMAKE_BINARY_DIR}/$<CONFIGURATION>/test/${test_name}"
        )
    else()
        set_target_properties("${test_name}"
        PROPERTIES
            RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/release/test/"
            RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}/debug/test/"
        )
        if(CMAKE_BUILD_TYPE STREQUAL "Release")
            add_test(
                NAME "${test_name}"
                COMMAND "${CMAKE_BINARY_DIR}/release/test/${test_name}"
            )
        else()
            add_test(
                NAME "${test_name}"
                COMMAND "${CMAKE_BINARY_DIR}/debug/test/${test_name}"
            )
        endif()
    endif()
endforeach()
