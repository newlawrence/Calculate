cmake_minimum_required(VERSION 3.5)
project(Calculate_Tests LANGUAGES CXX)

set(CMAKE_CONFIGURATION_TYPES "Release;Debug" CACHE STRING "Configurations" FORCE)
if(NOT CMAKE_BUILD_TYPE)
    if(EXISTS "${CMAKE_SOURCE_DIR}/.git")
        set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "Build type" FORCE)
    else()
        set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Build type" FORCE)
    endif()
endif()
message(STATUS "Calculate: Tests enabled")
enable_testing()

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

include_directories("${CMAKE_SOURCE_DIR}/include")
include("${CMAKE_BINARY_DIR}/test/conanbuildinfo.cmake")
conan_basic_setup(TARGETS NO_OUTPUT_DIRS)

set(gcov_program "$ENV{CV}")
if(NOT gcov_program)
    set(gcov_program gcov)
endif()

set(gcov_able FALSE)
set(coverage FALSE)
find_program(GCOV ${gcov_program})
if((CMAKE_CXX_COMPILER_ID MATCHES GNU) OR (CMAKE_CXX_COMPILER_ID MATCHES Clang))
    set(gcov_able TRUE)
    if(CMAKE_BUILD_TYPE MATCHES Debug AND GCOV)
        set(coverage TRUE)
        set(coverage_dir "${CMAKE_BINARY_DIR}/cov")
        file(MAKE_DIRECTORY "${coverage_dir}")
        file(MAKE_DIRECTORY "${coverage_dir}/bin")
        file(MAKE_DIRECTORY "${coverage_dir}/src")
        message(STATUS "Calculate: Targets configured for coverage using '${gcov_program}'")
    endif()
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
        set(CMAKE_LINKER_FLAGS_DEBUG "${CMAKE_STATIC_LINKER_FLAGS_DEBUG} --coverage")
    endif()
endif()

file(GLOB TEST_SOURCES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" source/*.cpp)
add_custom_target(make_test)
if(coverage)
    file(MAKE_DIRECTORY "${coverage_dir}/src/temp")
    add_custom_target(coverage)
    add_custom_command(TARGET coverage POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E echo "Generating gcov sources..."
        COMMAND ${CMAKE_COMMAND} -E make_directory "${coverage_dir}/src/temp"
        COMMAND ${CMAKE_COMMAND} -E chdir "${coverage_dir}/src/temp"
            ${GCOV} "${CMAKE_CURRENT_SOURCE_DIR}/source/*.cpp"
            -o "${coverage_dir}/bin" -p > /dev/null
        COMMAND ${CMAKE_COMMAND} -E copy
            "${coverage_dir}/src/temp/*Calculate#include*.gcov"
            "${coverage_dir}/src"
        COMMAND ${CMAKE_COMMAND} -E remove_directory "${coverage_dir}/src/temp"
    )

    find_program(LCOV lcov)
    find_program(GENHTML genhtml)
    if(LCOV AND GENHTML)
        file(MAKE_DIRECTORY "${coverage_dir}/rep")
        set(coverage_info "${coverage_dir}/rep/coverage.info")
        add_custom_command(TARGET coverage POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E echo "Generating coverage report..."
            COMMAND ${LCOV} -c -d "${coverage_dir}/bin"
                -o "${coverage_info}" --gcov-tool ${GCOV} > /dev/null
            COMMAND ${LCOV} -e "${coverage_info}" "*Calculate/include*"
                -o "${coverage_info}" > /dev/null
            COMMAND ${GENHTML} "${coverage_info}"
                -o "${coverage_dir}/rep" > /dev/null
        )
        message(STATUS "Calculate: Enabled coverage report in HTML format")
    endif()
endif()

foreach(test_source ${TEST_SOURCES})
    get_filename_component(name "${test_source}" NAME_WE)
    set(test_name "test_${name}")
    set(test_folder "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")

    add_executable(${test_name} "${test_source}")
    target_link_libraries(${test_name} CONAN_PKG::catch2)
    add_dependencies(make_test ${test_name})
    add_test(NAME ${test_name} COMMAND "${test_folder}/${test_name}")

    if(coverage)
        set(coverage_name "coverage_${name}")
        set(binary_dir "${CMAKE_BINARY_DIR}/test/CMakeFiles/${test_name}.dir/source/")
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
