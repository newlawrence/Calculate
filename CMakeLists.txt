cmake_minimum_required(VERSION 3.5)
project(calculate)

set(CATCH_REPO "https://raw.githubusercontent.com/philsquared/Catch/master")
set(CATCH_URL "${CATCH_REPO}/single_include/catch.hpp")
set(CATCH_PATH "${CMAKE_CURRENT_SOURCE_DIR}/extern/include/catch.hpp")
if(NOT EXISTS "${CATCH_PATH}")
    file(DOWNLOAD "${CATCH_URL}" "${CATCH_PATH}")
endif()

function(setup_target_for_coverage TARGETNAME LIBRARYNAME)
    FIND_PROGRAM(GCOV gcov)
    if (NOT GCOV)
    	message(FATAL_ERROR "gcov not found.")
    endif ()
    set(SRC "${CMAKE_CURRENT_SOURCE_DIR}/source/*.cpp")
    set(BIN "${CMAKE_CURRENT_SOURCE_DIR}/CMakeFiles/${LIBRARYNAME}.dir/source")
    add_custom_target(${TARGETNAME}
        COMMAND ${CMAKE_COMMAND} -E make_directory coverage
        COMMAND ${CMAKE_COMMAND} -E chdir
                "${CMAKE_CURRENT_SOURCE_DIR}/coverage"
                ${GCOV} ${SRC} -o ${BIN} -p > /dev/null
        COMMAND ${CMAKE_COMMAND} -E remove
                "${CMAKE_CURRENT_SOURCE_DIR}/coverage/*usr*.gcov"
    )
endfunction ()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/bin/")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/lib/")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/lib/")

if ((${CMAKE_CXX_COMPILER_ID} MATCHES "GNU") OR
    (${CMAKE_CXX_COMPILER_ID} MATCHES "Clang"))
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Werror -std=c11")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Werror -std=c++11")
    if (DEFINED ENV{COVERAGE})
        if ($ENV{COVERAGE} STREQUAL "True")
            if (NOT(CMAKE_BUILD_TYPE STREQUAL "Debug"))
                message(FATAL_ERROR "Code coverage in an optimized build.")
            endif ()
            set(CMAKE_C_OUTPUT_EXTENSION_REPLACE 1)
            set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O0 -coverage")
            set(CMAKE_CXX_OUTPUT_EXTENSION_REPLACE 1)
            set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0 -coverage")
            set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-elide-constructors")
            setup_target_for_coverage(coverage calculate)
        endif ()
    else ()
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O3")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3")
    endif ()
elseif (${CMAKE_CXX_COMPILER_ID} MATCHES "MSVC")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Ox /sdl")
endif ()

include_directories("${PROJECT_SOURCE_DIR}/include")
include_directories("${PROJECT_SOURCE_DIR}/extern/include")
file(GLOB sources "source/*.cpp")

set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
add_library(calculate SHARED "${sources}")
target_compile_definitions(calculate PUBLIC "_CRT_SECURE_NO_WARNINGS")

add_library(calculate_static STATIC "${sources}")
target_compile_definitions(calculate_static PUBLIC "_CRT_SECURE_NO_WARNINGS")

file(GLOB tests "tests/*.cpp")
add_executable(unittests "${tests}")
target_link_libraries(unittests LINK_PUBLIC calculate)

add_executable(calc "examples/calc.c")
target_link_libraries(calc LINK_PUBLIC calculate)

add_executable(calcpp "examples/calc.cpp")
target_link_libraries(calcpp LINK_PUBLIC calculate)

add_custom_command(
    TARGET unittests
    COMMENT "Run tests"
    POST_BUILD
    COMMAND unittests
)
