cmake_minimum_required(VERSION 3.5)
project(calculate C CXX)


function(setup_target_for_coverage TARGETNAME LIBRARYNAME TESTSNAME)
    find_program(GCOV gcov)
    if (NOT GCOV)
        message(FATAL_ERROR "gcov not found.")
    endif ()
    find_program(GCOVR gcovr)
    if (NOT GCOVR)
        message(FATAL_ERROR "gcovr not found.")
    endif ()
    set(SRC "${CMAKE_CURRENT_SOURCE_DIR}/source/*.cpp")
    set(TSRC "${CMAKE_CURRENT_SOURCE_DIR}/tests/*.cpp")
    set(BIN "${CMAKE_CURRENT_SOURCE_DIR}/CMakeFiles/${LIBRARYNAME}.dir/source")
    set(TBIN "${CMAKE_CURRENT_SOURCE_DIR}/CMakeFiles/${TESTSNAME}.dir/tests")
    set(COVERAGE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/coverage")
    add_custom_target(${TARGETNAME}
        COMMAND ${CMAKE_COMMAND} -E make_directory coverage
        COMMAND ${CMAKE_COMMAND} -E chdir "${COVERAGE_DIR}"
                ${GCOV} ${SRC} -o ${BIN} -p > /dev/null
        COMMAND ${CMAKE_COMMAND} -E chdir "${COVERAGE_DIR}"
                ${GCOV} ${TSRC} -o ${TBIN} -p > /dev/null
        COMMAND ${CMAKE_COMMAND} -E chdir "${COVERAGE_DIR}"
                ${GCOVR} -r .. --object-directory . -o coverage.html
                "--gcov-filter=\".*#(usr|extern|tests)#.*\"" -g 
        COMMAND ${CMAKE_COMMAND} -E chdir "${COVERAGE_DIR}"
                ${GCOVR} -r .. --object-directory . -o coverage.html
                -g -k -s --html --html-details
    )
endfunction ()


set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/bin/")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/lib/")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/lib/")

if (
    (${CMAKE_CXX_COMPILER_ID} MATCHES "GNU") OR
    (${CMAKE_CXX_COMPILER_ID} MATCHES "Clang")
)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Werror -std=c11")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Werror -std=c++11")
    if (DEFINED ENV{COVERAGE})
        if (NOT(CMAKE_BUILD_TYPE STREQUAL "Debug"))
            message(FATAL_ERROR "Code coverage in an optimized build.")
        endif ()
        set(CMAKE_C_OUTPUT_EXTENSION_REPLACE 1)
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O0 -coverage")
        set(CMAKE_CXX_OUTPUT_EXTENSION_REPLACE 1)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0 -coverage")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-elide-constructors")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-inline")
        setup_target_for_coverage(coverage calculate unittests)
    else ()
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O3")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3")
    endif ()
elseif (${CMAKE_CXX_COMPILER_ID} MATCHES "MSVC")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Ox /sdl")
endif ()

include_directories("${PROJECT_SOURCE_DIR}/include")
include_directories("${PROJECT_SOURCE_DIR}/extern/include")
file(GLOB sources "${PROJECT_SOURCE_DIR}/source/*.cpp")


set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
add_library(calculate SHARED "${sources}")
target_compile_definitions(calculate PUBLIC "_CRT_SECURE_NO_WARNINGS")

add_library(calculate_static STATIC "${sources}")
target_compile_definitions(calculate_static PUBLIC "_CRT_SECURE_NO_WARNINGS")
set_target_properties(calculate_static PROPERTIES OUTPUT_NAME "calculate")


if (DEFINED ENV{TESTING} OR DEFINED ENV{COVERAGE})
    set(CATCH_GIT "https://raw.githubusercontent.com/philsquared/Catch/master")
    set(CATCH_URL "${CATCH_GIT}/single_include/catch.hpp")
    set(CATCH_PATH "${CMAKE_CURRENT_SOURCE_DIR}/extern/include/catch.hpp")
    if (NOT EXISTS "${CATCH_PATH}")
        message("Downloading catch library...")
        file(DOWNLOAD "${CATCH_URL}" "${CATCH_PATH}")
        message("Catch library successfully downloaded.")
    endif ()

    file(GLOB tests "${PROJECT_SOURCE_DIR}/tests/*.cpp")
    add_executable(unittests "${tests}")
    target_link_libraries(unittests LINK_PUBLIC calculate)

    add_custom_command(
        TARGET unittests
        COMMENT "Run tests"
        POST_BUILD
        COMMAND unittests
    )
endif()


add_executable(calc "${PROJECT_SOURCE_DIR}/examples/calc.c")
target_link_libraries(calc PRIVATE calculate)

add_executable(calcpp "${PROJECT_SOURCE_DIR}/examples/calc.cpp")
target_link_libraries(calcpp PRIVATE calculate)


if (DEFINED ENV{BINDING} OR DEFINED ENV{FC})
    enable_language(Fortran)
    add_subdirectory(binding/fortran)

    if (${CMAKE_Fortran_COMPILER_ID} MATCHES "GNU")
        set(CMAKE_Fortran_FLAGS "${CMAKE_Fortran_FLAGS} -O3 -std=f2008ts")
        set(CMAKE_Fortran_FLAGS "${CMAKE_Fortran_FLAGS} -Wall")
    elseif (${CMAKE_Fortran_COMPILER_ID} MATCHES "Intel")
        if (WIN32)
            set(CMAKE_Fortran_FLAGS "${CMAKE_Fortran_FLAGS} /O3 /stand:f08")
            set(CMAKE_Fortran_FLAGS "${CMAKE_Fortran_FLAGS} /free /warn:all")
        else ()
            set(CMAKE_Fortran_FLAGS "${CMAKE_Fortran_FLAGS} -O3 -stand f08")
            set(CMAKE_Fortran_FLAGS "${CMAKE_Fortran_FLAGS} -free -warn all")
        endif ()
    endif()

    include_directories("${PROJECT_SOURCE_DIR}/binding/fortran/module")

    add_executable(calcfor "${PROJECT_SOURCE_DIR}/examples/calc.f90")
    target_link_libraries(calcfor PRIVATE calculate_forwrapd)
endif ()
