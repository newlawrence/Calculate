cmake_minimum_required(VERSION 3.5)
project(calculate C CXX)


set(MAJOR "1")
set(MINOR "2")
set(PTH "3")
set(DEV)
set(BUILD "1")
set(WINDOWS_VERSION "${MAJOR},${MINOR},${PTH},${BUILD}")

set(PACKAGE_NAME "Calculate")
set(PACKAGE_VERSION "${MAJOR}.${MINOR}.${PTH}${DEV}")
set(PACKAGE_LICENSE "MIT")
set(PACKAGE_AUTHOR "Alberto Lorenzo")
set(PACKAGE_CONTACT "alorenzo.md@gmail.com")
set(PACKAGE_SITE "https://github.com/newlawrence/Calculate")
set(PACKAGE_DESCRIPTION "Little math expression parser")
string(TIMESTAMP PACKAGE_DATE "%Y/%m/%d")
string(TIMESTAMP PACKAGE_COPYRIGHT "Copyright (C) 2016 - %Y Alberto Lorenzo")


set(TEMPLATES_DIR "${CMAKE_CURRENT_SOURCE_DIR}/template")
macro(apply_template FILENAME ROUTE)
    configure_file(
        ${TEMPLATES_DIR}/${FILENAME}.in
        ${ROUTE}/${FILENAME}
        NEWLINE_STYLE UNIX
    )
endmacro()


set(PATCHES_DIR "${CMAKE_CURRENT_SOURCE_DIR}/patch")
macro(apply_patch COMPILER)
    find_program(PATCH patch)
    find_program(GIT git)
    message("-- Applying ${COMPILER} patch...")
    if(PATCH)
        execute_process(COMMAND ${PATCH} -p1 -i
                                ${PATCHES_DIR}/${COMPILER}.patch)
    elseif(GIT)
        execute_process(COMMAND ${GIT} apply --whitespace=fix
                                ${PATCHES_DIR}/${COMPILER}.patch)
    else()
        message(FATAL_ERROR "Suitable patching command not found!")
    endif()
endmacro()


function(setup_target_for_coverage TARGETNAME OBJFOLDER TESTSNAME)
    find_program(GCOV gcov)
    if(NOT GCOV)
        message(FATAL_ERROR "gcov needed for coverage!")
    endif()
    set(SRC "${CMAKE_CURRENT_SOURCE_DIR}/source/*.cpp")
    set(TSRC "${CMAKE_CURRENT_SOURCE_DIR}/test/*.cpp")
    set(BIN "${CMAKE_BINARY_DIR}/CMakeFiles/${OBJFOLDER}.dir/source")
    set(TBIN "${CMAKE_BINARY_DIR}/CMakeFiles/${TESTSNAME}.dir/test")
    set(COVERAGE_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/coverage")

	find_program(GCOVR gcovr)
    if(GCOVR)
        add_custom_target(${TARGETNAME}
            COMMAND ${CMAKE_COMMAND} -E make_directory ${COVERAGE_DIRECTORY}
            COMMAND ${CMAKE_COMMAND} -E chdir ${COVERAGE_DIRECTORY}
                    ${GCOV} ${SRC} -o ${BIN} -p > /dev/null
            COMMAND ${CMAKE_COMMAND} -E chdir ${COVERAGE_DIRECTORY}
                    ${GCOV} ${TSRC} -o ${TBIN} -p > /dev/null
            COMMAND ${CMAKE_COMMAND} -E chdir ${COVERAGE_DIRECTORY}
                    ${GCOVR} -r .. --object-directory . -o coverage.html
                    "--gcov-filter=\".*#(usr|extern|test)#.*\"" -g 
            COMMAND ${CMAKE_COMMAND} -E chdir ${COVERAGE_DIRECTORY}
                    ${GCOVR} -r .. --object-directory . -o coverage.html
                    -g -k -s --html --html-details
        )
    else()
        add_custom_target(${TARGETNAME}
            COMMAND ${CMAKE_COMMAND} -E make_directory ${COVERAGE_DIRECTORY}
            COMMAND ${CMAKE_COMMAND} -E chdir ${COVERAGE_DIRECTORY}
                    ${GCOV} ${SRC} -o ${BIN} -p > /dev/null
            COMMAND ${CMAKE_COMMAND} -E chdir ${COVERAGE_DIRECTORY}
                    ${GCOV} ${TSRC} -o ${TBIN} -p > /dev/null
        )
    endif()
    add_dependencies(${TARGETNAME} ${TESTSNAME})
endfunction()


set(STATIC_BUILD "$ENV{STATIC}")
set(NORUNTIME "$ENV{NORUNTIME}")
set(BINDING "$ENV{BINDING}")
set(TESTING "$ENV{TESTING}")
set(COVERAGE "$ENV{COVERAGE}")


set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/bin/")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/lib/")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/lib/")


if(
    (${CMAKE_CXX_COMPILER_ID} MATCHES "GNU") OR
    (${CMAKE_CXX_COMPILER_ID} MATCHES "Clang")
)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Werror -std=c11")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Werror -std=c++14")
    set(CMAKE_CXX_FLAGS
        "${CMAKE_CXX_FLAGS} -Wno-gnu-string-literal-operator-template"
    )
    if(COVERAGE)
        if(NOT(CMAKE_BUILD_TYPE STREQUAL "Debug"))
            message(FATAL_ERROR "Code coverage in an optimized build.")
        endif()
        set(CMAKE_C_OUTPUT_EXTENSION_REPLACE "1")
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O0 -coverage")
        set(CMAKE_CXX_OUTPUT_EXTENSION_REPLACE "1")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0 -coverage")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-elide-constructors")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-inline")
        setup_target_for_coverage(coverage build unittests)
    else()
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O3")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3")
    endif()
elseif(MSVC)
    apply_patch(msvc)
    set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS "ON")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /std:c++14 /sdl")
elseif(${CMAKE_CXX_COMPILER_ID} MATCHES "Intel")
    apply_patch(icpc)
    if(WIN32)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /O3 /std:c++14")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Wall")
    else()
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -std=c++14")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")
    endif()
endif()

include_directories(${PROJECT_SOURCE_DIR}/include)
apply_template(info.h ${PROJECT_SOURCE_DIR}/include/calculate)
file(GLOB sources ${PROJECT_SOURCE_DIR}/source/*.cpp)
if(WIN32)
    apply_template(version.rc ${PROJECT_SOURCE_DIR}/resource)
    set(sources "${sources};${PROJECT_SOURCE_DIR}/resource/version.rc")
    if(NOT MSVC)
        set(CMAKE_RC_COMPILER_INIT "windres")
        ENABLE_LANGUAGE(RC)
        SET(CMAKE_RC_COMPILE_OBJECT
            "<CMAKE_RC_COMPILER> -O coff <DEFINES> -i <SOURCE> -o <OBJECT>"
        )
    endif()
endif()


add_library(build OBJECT ${sources})
set_target_properties(build PROPERTIES POSITION_INDEPENDENT_CODE 1)
target_compile_definitions(build
    PUBLIC "_CRT_SECURE_NO_WARNINGS"
    PUBLIC "__COMPILING_LIBRARY__"
)

if(STATIC_BUILD)
    add_library(calculate STATIC $<TARGET_OBJECTS:build>)
else()
    add_library(calculate SHARED $<TARGET_OBJECTS:build>)
    set_target_properties(calculate PROPERTIES PREFIX "lib")
    set_property(TARGET calculate PROPERTY VERSION ${PACKAGE_VERSION})
    set_property(TARGET calculate PROPERTY SOVERSION ${MAJOR})
endif()


if(TESTING OR COVERAGE)
    set(CATCH_GIT "https://raw.githubusercontent.com/philsquared/Catch/master")
    set(CATCH_URL "${CATCH_GIT}/single_include/catch.hpp")
    set(CATCH_PATH "${CMAKE_CURRENT_SOURCE_DIR}/extern/include/catch.hpp")
    include_directories(${PROJECT_SOURCE_DIR}/extern/include)
    if(NOT EXISTS ${CATCH_PATH})
        message("-- Downloading catch library...")
        file(DOWNLOAD ${CATCH_URL} ${CATCH_PATH})
        message("-- Catch library successfully downloaded.")
    endif()

    apply_template(0_catch.cpp ${PROJECT_SOURCE_DIR}/test)
    file(GLOB tests ${PROJECT_SOURCE_DIR}/test/*.cpp)
    add_executable(unittests ${tests})
    target_link_libraries(unittests LINK_PUBLIC calculate)

    add_custom_command(
        TARGET unittests
        COMMENT "Run tests"
        POST_BUILD
        COMMAND unittests
    )
endif()


add_executable(calc_c ${PROJECT_SOURCE_DIR}/example/calc.c)
target_link_libraries(calc_c PRIVATE calculate)
target_compile_definitions(calc_c PUBLIC "_CRT_SECURE_NO_WARNINGS")

add_executable(calc_cpp ${PROJECT_SOURCE_DIR}/example/calc.cpp)
target_link_libraries(calc_cpp PRIVATE calculate)


if(BINDING OR DEFINED ENV{FC})
    add_subdirectory(binding/fortran)
endif()

if(BINDING OR DEFINED ENV{PY})
    add_subdirectory(binding/python)
endif()
