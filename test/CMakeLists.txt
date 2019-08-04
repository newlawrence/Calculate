cmake_minimum_required(VERSION 3.5)
project(Calculate-Tests LANGUAGES CXX)


if(CMAKE_SOURCE_DIR STREQUAL PROJECT_SOURCE_DIR)
    set(CMAKE_CONFIGURATION_TYPES "Release;Debug" CACHE STRING "Configurations" FORCE)
    if(NOT CMAKE_BUILD_TYPE)
        set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "Build type" FORCE)
    endif()
else()
    message(STATUS "Calculate: tests enabled")
endif()
enable_testing()

find_package(Catch2 QUIET)
if(EXISTS "${CMAKE_BINARY_DIR}/test/conanbuildinfo.cmake")
    include("${CMAKE_BINARY_DIR}/test/conanbuildinfo.cmake")
    conan_basic_setup(TARGETS NO_OUTPUT_DIRS)
    set(CALCULATE_TESTS_USE_CONAN TRUE)
elseif(EXISTS "${CMAKE_BINARY_DIR}/conanbuildinfo.cmake")
    include("${CMAKE_BINARY_DIR}/conanbuildinfo.cmake")
    conan_basic_setup(TARGETS NO_OUTPUT_DIRS)
    set(CALCULATE_TESTS_USE_CONAN TRUE)
elseif(Catch2_FOUND)
    set(CALCULATE_TESTS_USE_CONAN FALSE)
else()
    message(FATAL_ERROR "Calculate: missing Catch2 library")
endif()



if(DEFINED ENV{CV})
    find_program(GCOV $ENV{CV})
else()
    find_program(GCOV gcov)
endif()


set(CALCULATE_BUILD_COVERAGE FALSE)
if(
    (CMAKE_CXX_COMPILER_ID MATCHES GNU OR CMAKE_CXX_COMPILER_ID MATCHES Clang) AND
    CMAKE_BUILD_TYPE MATCHES Debug AND GCOV
)
    set(CALCULATE_BUILD_COVERAGE TRUE)
    set(CALCULATE_COVERAGE_DIR "${CMAKE_BINARY_DIR}/coverage")
    file(MAKE_DIRECTORY "${CALCULATE_COVERAGE_DIR}")
    file(MAKE_DIRECTORY "${CALCULATE_COVERAGE_DIR}/bin")
    file(MAKE_DIRECTORY "${CALCULATE_COVERAGE_DIR}/source")
    file(MAKE_DIRECTORY "${CALCULATE_COVERAGE_DIR}/source/temp")
    add_custom_target(coverage)
    message(STATUS "Calculate: targets configured for coverage using '${GCOV}'")

    add_custom_command(TARGET coverage
    POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E echo "Generating gcov sources..."
        COMMAND ${CMAKE_COMMAND} -E make_directory "${CALCULATE_COVERAGE_DIR}/source/temp"
        COMMAND ${CMAKE_COMMAND} -E chdir "${CALCULATE_COVERAGE_DIR}/source/temp"
            ${GCOV} "${CMAKE_CURRENT_SOURCE_DIR}/source/*.cpp"
            -o "${CALCULATE_COVERAGE_DIR}/bin" -p > /dev/null
        COMMAND ${CMAKE_COMMAND} -E copy
            "${CALCULATE_COVERAGE_DIR}/source/temp/*Calculate#include*.gcov"
            "${CALCULATE_COVERAGE_DIR}/source"
        COMMAND ${CMAKE_COMMAND} -E remove_directory "${CALCULATE_COVERAGE_DIR}/source/temp"
    )

    find_program(LCOV lcov)
    find_program(GENHTML genhtml)
    if(LCOV AND GENHTML)
        file(MAKE_DIRECTORY "${CALCULATE_COVERAGE_DIR}/report")
        set(coverage_info "${CALCULATE_COVERAGE_DIR}/report/coverage.info")
        add_custom_command(TARGET coverage
        POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E echo
                "Generating coverage report..."
            COMMAND ${LCOV} -c -d "${CALCULATE_COVERAGE_DIR}/bin"
                -o "${coverage_info}" --gcov-tool ${GCOV} > /dev/null
            COMMAND ${LCOV} -e "${coverage_info}" "*Calculate/include*"
                -o "${coverage_info}" > /dev/null
            COMMAND ${GENHTML} "${coverage_info}"
                -o "${CALCULATE_COVERAGE_DIR}/report" > /dev/null
        )
        message(STATUS "Calculate: enabled coverage report in HTML format")
    endif()
endif()


file(GLOB TEST_SOURCES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" source/*.cpp)
add_custom_target(make_test)
foreach(test_source ${TEST_SOURCES})
    get_filename_component(name "${test_source}" NAME_WE)
    set(test_name "test_${name}")
    set(test_folder "${CMAKE_BINARY_DIR}/bin")

    add_executable(${test_name} "${test_source}")
    set_target_properties(${test_name}
    PROPERTIES
        CXX_STANDARD_REQUIRED YES
        CXX_EXTENSIONS NO
        RUNTIME_OUTPUT_DIRECTORY "${test_folder}"
    )
    if(CMAKE_SOURCE_DIR STREQUAL PROJECT_SOURCE_DIR)
        target_include_directories(${test_name} PRIVATE "${CMAKE_SOURCE_DIR}/../include")
    else()
        target_include_directories(${test_name} PRIVATE "${CMAKE_SOURCE_DIR}/include")
    endif()

    target_compile_features(${test_name} PRIVATE cxx_std_14)
    target_compile_options(${test_name}
    PRIVATE
        $<$<CXX_COMPILER_ID:GNU>: -pedantic -Wall -Wextra -Werror -Wno-noexcept-type>
        $<$<CXX_COMPILER_ID:Clang>: -pedantic -Wall -Wextra -Werror -Qunused-arguments>
    )
    if(CALCULATE_BUILD_COVERAGE)
        set_target_properties(${test_name} PROPERTIES LINK_FLAGS --coverage)
        target_compile_options(${test_name} PRIVATE -O0 -fno-inline -fprofile-arcs -ftest-coverage)
    endif()

    if(CALCULATE_TESTS_USE_CONAN)
        target_link_libraries(${test_name} CONAN_PKG::Catch2)
    else()
        target_link_libraries(${test_name} Catch2::Catch2)
    endif()
    add_dependencies(make_test ${test_name})
    add_test(NAME ${test_name} COMMAND "${test_folder}/${test_name}")

    if(CALCULATE_BUILD_COVERAGE)
        set(coverage_name "coverage_${name}")
        if(CMAKE_SOURCE_DIR STREQUAL PROJECT_SOURCE_DIR)
            set(CALCULATE_GCOV_DIR "${CMAKE_BINARY_DIR}/CMakeFiles/${test_name}.dir/source/")
        else()
            set(CALCULATE_GCOV_DIR "${CMAKE_BINARY_DIR}/test/CMakeFiles/${test_name}.dir/source/")
        endif()

        add_custom_target(${coverage_name}
            COMMAND ${CMAKE_COMMAND} -E copy
                "${CALCULATE_GCOV_DIR}/${name}.cpp.gcda"
                "${CALCULATE_COVERAGE_DIR}/bin/${name}.gcda"
            COMMAND ${CMAKE_COMMAND} -E copy
                "${CALCULATE_GCOV_DIR}/${name}.cpp.gcno"
                "${CALCULATE_COVERAGE_DIR}/bin/${name}.gcno"
        )
        add_dependencies(coverage ${coverage_name})
    endif()
endforeach()
