cmake_minimum_required(VERSION 3.5)
project(Calculate)

enable_testing()
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(default_build_type "Release")
if(EXISTS "${CMAKE_SOURCE_DIR}/.git")
    set(default_build_type "Debug")
endif()
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE "${default_build_type}" CACHE
        STRING "Choose the type of build." FORCE
    )
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release")
    set(CMAKE_CONFIGURATION_TYPES "Release;Debug")
endif()
message(STATUS "Setting build type to '${CMAKE_BUILD_TYPE}'")

set(gcov_able FALSE)
set(coverage FALSE)
set(gcov_program "$ENV{CV}")
if(NOT gcov_program)
    set(gcov_program gcov)
endif()
find_program(GCOV ${gcov_program})
if((CMAKE_CXX_COMPILER_ID MATCHES GNU) OR (CMAKE_CXX_COMPILER_ID MATCHES Clang))
    set(gcov_able TRUE)
    if(CMAKE_BUILD_TYPE MATCHES Debug AND GCOV)
        set(coverage TRUE)
        set(coverage_dir "${CMAKE_BINARY_DIR}/cov")
        file(MAKE_DIRECTORY "${coverage_dir}")
        file(MAKE_DIRECTORY "${coverage_dir}/bin")
        file(MAKE_DIRECTORY "${coverage_dir}/src")
        message(STATUS "Targets configured for coverage '${gcov_program}'")
    endif()
endif()

include_directories(include)
include_directories(tool/include)

if(CMAKE_GENERATOR MATCHES "^Visual Studio.*$")
    set(msvc_toolchain TRUE)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/Release/bin")
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}/Debug/bin")
else()
    set(msvc_toolchain FALSE)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
endif()

if(WIN32 AND NOT MINGW AND NOT MSYS AND NOT CYGWIN)
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /Ox")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /Od /Debug:full")
else()
    if(gcov_able)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pedantic -Wall -Wextra -Werror")
        if(CMAKE_CXX_COMPILER_ID MATCHES GNU)
            set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-noexcept-type")
        elseif(CMAKE_CXX_COMPILER_ID MATCHES Clang)
            set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Qunused-arguments")
        endif()
    endif()
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0")
    if(coverage)
        set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fno-elide-constructors")
        set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fno-inline")
        set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fprofile-arcs")
        set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -ftest-coverage")
    endif()
endif()

if(MSVC)
    add_definitions(-DBOOST_ALL_NO_LIB)
    add_definitions(-DBOOST_ALL_DYN_LINK)
    set(Boost_USE_STATIC_LIBS OFF)
    set(Boost_USE_MULTITHREADED ON)
    set(Boost_USE_STATIC_RUNTIME OFF)
endif()
find_package(Boost QUIET)
if(Boost_FOUND)
    find_package(Boost REQUIRED COMPONENTS system filesystem program_options)
    include_directories(${Boost_INCLUDE_DIRS})
    string(REPLACE ";" "\n       " boost_libraries "${Boost_LIBRARIES}")
    add_executable(calculate ${CMAKE_SOURCE_DIR}/example/calculate.cpp)
    target_link_libraries(calculate ${Boost_LIBRARIES})
    message(STATUS "Added 'calculate' target")
else()
    message(STATUS "Skipped 'calculate' target")
endif()

file(GLOB TEST_SOURCES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" test/*.cpp)
add_custom_target(all_test)
if(coverage)
    file(MAKE_DIRECTORY "${coverage_dir}/src/temp")
    add_custom_target(coverage)
    add_custom_command(
        TARGET coverage
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory "${coverage_dir}/src/temp"
        COMMAND ${CMAKE_COMMAND} -E chdir "${coverage_dir}/src/temp"
            ${GCOV} "${CMAKE_CURRENT_SOURCE_DIR}/test/*.cpp"
            -o "${coverage_dir}/bin" -p > /dev/null
        COMMAND ${CMAKE_COMMAND} -E copy
            "${coverage_dir}/src/temp/*Calculate#include*.gcov"
            "${coverage_dir}/src"
        COMMAND ${CMAKE_COMMAND} -E remove_directory "${coverage_dir}/src/temp"
    )

    find_program(LCOV lcov)
    find_program(GENHTML genhtml)
    if(LCOV AND GENHTML)
        file(MAKE_DIRECTORY "${coverage_dir}/html")
        set(coverage_info "${coverage_dir}/html/coverage.info")
        add_custom_command(
            TARGET coverage
            POST_BUILD
            COMMAND ${LCOV} -c -d "${coverage_dir}/bin"
                -o "${coverage_info}" > /dev/null
            COMMAND ${LCOV} -e "${coverage_info}" "*Calculate/include*"
                -o "${coverage_info}" > /dev/null
            COMMAND ${GENHTML} "${coverage_info}"
                -o "${coverage_dir}/html" > /dev/null
        )
        message(STATUS "Enabled coverage report in HTML format")
    endif()
endif()

foreach(test_source ${TEST_SOURCES})
    get_filename_component(name "${test_source}" NAME_WE)
    set(test_name "test_${name}")
    if(msvc_toolchain)
        set(test_folder "${CMAKE_BINARY_DIR}/$<CONFIGURATION>/test")
    else()
        set(test_folder "${CMAKE_BINARY_DIR}/test")
    endif()

    add_executable(${test_name} "${test_source}")
    set_target_properties(${test_name}
    PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY_RELEASE "${test_folder}"
        RUNTIME_OUTPUT_DIRECTORY_DEBUG "${test_folder}"
    )
    add_test(NAME ${test_name} COMMAND "${test_folder}/${test_name}")
    add_dependencies(all_test ${test_name})

    if(coverage)
        set(coverage_name "coverage_${name}")
        set(binary_dir "${CMAKE_BINARY_DIR}/CMakeFiles/${test_name}.dir/test/")
        add_custom_target(${coverage_name}
            COMMAND ${CMAKE_COMMAND} -E copy
                "${binary_dir}/${name}.cpp.gcda"
                "${coverage_dir}/bin/${name}.gcda"
            COMMAND ${CMAKE_COMMAND} -E copy
                "${binary_dir}/${name}.cpp.gcno"
                "${coverage_dir}/bin/${name}.gcno"
        )
        add_dependencies(coverage ${coverage_name})
    endif()
endforeach()
