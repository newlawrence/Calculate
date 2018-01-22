cmake_minimum_required(VERSION 3.5)
project(Calculate)

enable_testing()
set(CMAKE_CONFIGURATION_TYPES "Release;Debug")
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include_directories(include)
include_directories(tool/Catch2/single_include)

if(MSVC)
    set(
        CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE
        "${CMAKE_BINARY_DIR}/release/bin/"
    )
    set(
        CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG
        "${CMAKE_BINARY_DIR}/debug/bin/"
    )
else()
    if(CMAKE_BUILD_TYPE STREQUAL "Release")
        set(binaries_dir "${CMAKE_BINARY_DIR}/release/bin/")
        set(tests_dir "${CMAKE_BINARY_DIR}/release/test/")
        set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${binaries_dir}")
    else()
        set(binaries_dir "${CMAKE_BINARY_DIR}/debug/bin/")
        set(tests_dir "${CMAKE_BINARY_DIR}/release/test/")
        set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${binaries_dir}")
    endif()
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

file(GLOB TEST_SOURCES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} test/*.cpp)
foreach(test_source ${TEST_SOURCES})
    get_filename_component(test_name ${test_source} NAME_WE)
    set(test_name test_${test_name})
    add_executable(${test_name} ${test_source})
    if(MSVC)
        set_target_properties(${test_name}
        PROPERTIES
            RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/release/test/"
            RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}/debug/test/"
        )
        add_test(
            NAME ${test_name}
            COMMAND "${CMAKE_BINARY_DIR}/$<CONFIGURATION>/test/${test_name}"
        )
    else()
        set_target_properties(${test_name}
        PROPERTIES
            RUNTIME_OUTPUT_DIRECTORY "${tests_dir}"
        )
        add_test(NAME ${test_name} COMMAND ${tests_dir}/${test_name})
    endif()
endforeach()
