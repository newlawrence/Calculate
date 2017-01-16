cmake_minimum_required(VERSION 3.5)
project(calculate C CXX)


set(MAJOR "1")
set(MINOR "1")
set(PTH "4")
set(DEV ".dev0")
set(BUILD "0")
set(WINDOWS_VERSION "${MAJOR},${MINOR},${PTH},${BUILD}")

set(PACKAGE_VERSION "${MAJOR}.${MINOR}.${PTH}${DEV}")
set(PACKAGE_LICENSE "MIT")
set(PACKAGE_AUTHOR "Alberto Lorenzo")
set(PACKAGE_CONTACT "alorenzo.md@gmail.com")
set(PACKAGE_SITE "https://github.com/newlawrence/Calculate")
set(PACKAGE_DESCRIPTION "Little math expression parser")
string(TIMESTAMP PACKAGE_DATE "%Y/%m/%d")


set(PATCH_DIR "${CMAKE_CURRENT_SOURCE_DIR}/patch")
macro(apply_patch COMPILER)
    find_program(PATCH patch)
    find_program(GIT git)
    message("-- Applying ${COMPILER} patch...")
    if(PATCH)
        execute_process(
            COMMAND ${PATCH} -p1 -i
                    ${PATCH_DIR}/${COMPILER}.patch
        )
    elseif(GIT)
        execute_process(
            COMMAND ${GIT} apply --whitespace=fix
                    ${PATCH_DIR}/${COMPILER}.patch
        )
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


if(DEFINED ENV{NORUNTIME})
    set(NORUNTIME "$ENV{NORUNTIME}")
else()
    set(NORUNTIME "False")
endif()
if(DEFINED ENV{BINDING})
    set(BINDING "$ENV{BINDING}")
else()
    set(BINDING "False")
endif()
if(DEFINED ENV{TESTING})
    set(TESTING "$ENV{TESTING}")
else()
    set(TESTING "False")
endif()
if(DEFINED ENV{COVERAGE})
    set(COVERAGE "$ENV{COVERAGE}")
else()
    set(COVERAGE "False")
endif()


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
file(GLOB sources ${PROJECT_SOURCE_DIR}/source/*.cpp)
if(WIN32)
    set(RESOURCE_IN "${PROJECT_SOURCE_DIR}/resource/version.rc.in")
    set(RESOURCE "${PROJECT_SOURCE_DIR}/resource/version.rc")
    configure_file(${RESOURCE_IN} ${RESOURCE})
    set(sources "${sources};${RESOURCE}")
    if(NOT MSVC)
        set(CMAKE_RC_COMPILER_INIT "windres")
        ENABLE_LANGUAGE(RC)
        SET(CMAKE_RC_COMPILE_OBJECT
            "<CMAKE_RC_COMPILER> -O coff <DEFINES> -i <SOURCE> -o <OBJECT>"
        )
    endif()
endif()


add_library(build
    OBJECT ${sources}
)
set_target_properties(build
    PROPERTIES POSITION_INDEPENDENT_CODE 1
)
target_compile_definitions(build
    PUBLIC "_CRT_SECURE_NO_WARNINGS"
    PUBLIC "__COMPILING_LIBRARY__"
)

add_library(calculate
    SHARED $<TARGET_OBJECTS:build>
)
set_target_properties(calculate
    PROPERTIES PREFIX "lib"
    PROPERTIES VERSION ${PACKAGE_VERSION}
    PROPERTIES SOVERSION ${MAJOR}
)

add_library(calculate_static
    STATIC $<TARGET_OBJECTS:build>
)


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

    file(GLOB tests ${PROJECT_SOURCE_DIR}/test/*.cpp)
    add_executable(unittests ${tests})
    target_link_libraries(unittests
        LINK_PUBLIC calculate
    )

    add_custom_command(
        TARGET unittests
        COMMENT "Run tests"
        POST_BUILD
        COMMAND unittests
    )
endif()


add_executable(calc_c ${PROJECT_SOURCE_DIR}/example/calc.c)
target_link_libraries(calc_c
    PRIVATE calculate
)
target_compile_definitions(calc_c
    PUBLIC "_CRT_SECURE_NO_WARNINGS"
)

add_executable(calc_cpp ${PROJECT_SOURCE_DIR}/example/calc.cpp)
target_link_libraries(calc_cpp
    PRIVATE calculate
)


if(BINDING OR DEFINED ENV{FC})
    add_subdirectory(binding/fortran)
endif()

if(BINDING OR DEFINED ENV{PY})
    add_subdirectory(binding/python)
endif()
