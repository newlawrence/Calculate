cmake_minimum_required(VERSION 3.5)
project(calculate_python_wrapper NONE)
set(PYTHON_SUPPORTED_VERSIONS 2.7 3.4 3.5 3.6)


set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/bin/")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/lib/")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/lib/")


set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
add_library(calculate_python SHARED $<TARGET_OBJECTS:build>)
set_target_properties(calculate_python PROPERTIES PREFIX "lib")

if(NORUNTIME)
    if(UNIX AND NOT APPLE)
        target_link_libraries(calculate_python
            PUBLIC -static-libgcc -static-libstdc++
        )
    elseif(WIN32)
        if(
            (${CMAKE_CXX_COMPILER_ID} MATCHES "GNU") OR
            (${CMAKE_CXX_COMPILER_ID} MATCHES "Clang")
        )
            target_link_libraries(calculate_python
                PUBLIC -static-libgcc -static-libstdc++
                       -Wl,-Bstatic -lstdc++ -lpthread
                       -Wl,-Bdynamic
            )
        elseif(MSVC)
            set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MT")
            set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MTd")
        endif()
    endif()
endif()

add_custom_command(
    TARGET calculate_python
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
            $<TARGET_FILE:calculate_python>
            ${CMAKE_CURRENT_SOURCE_DIR}/calculate
)


if(DEFINED ENV{PY})
    set(PYTHON "$ENV{PY}")
else()
    find_program(PYTHON python)
endif()
if(DEFINED ENV{CONDA})
    set(CONDA "$ENV{CONDA}")
else()
    find_program(CONDA conda)
endif()

if(PYTHON)
    execute_process(
        COMMAND ${PYTHON} --version
        OUTPUT_VARIABLE PYTHON_VER
        ERROR_VARIABLE PYTHON_VER
    )
    string(STRIP ${PYTHON_VER} PYTHON_VER)
    message("-- The Python interpreter specification is ${PYTHON_VER}")
    set(SETUP "${CMAKE_CURRENT_SOURCE_DIR}/setup.py")

    apply_template(__init__.py ${CMAKE_CURRENT_SOURCE_DIR}/calculate)
    apply_template(meta.yaml ${CMAKE_CURRENT_SOURCE_DIR}/recipe)

    add_custom_target(python_wheel
        DEPENDS calculate_python
        COMMAND ${PYTHON} -W ignore::UserWarning:distutils.dist
                          -W ignore::RuntimeWarning:wheel.pep425tags
                ${SETUP} bdist_wheel
                1> ${CMAKE_CURRENT_SOURCE_DIR}/build.log 2>&1
    )

    if(CONDA)
        execute_process(
            COMMAND ${CONDA} --version
            OUTPUT_VARIABLE CONDA_VER
            ERROR_VARIABLE CONDA_VER
        )
        string(STRIP ${CONDA_VER} CONDA_VER)
        message("-- The Conda package manager specification is ${CONDA_VER}")

        foreach(PYTHON_VERSION ${PYTHON_SUPPORTED_VERSIONS})
            add_custom_target(conda_package-Py${PYTHON_VERSION}
                DEPENDS calculate_python
                COMMAND ${CONDA} build purge
                COMMAND ${CONDA} build --python=${PYTHON_VERSION}
                        --output-folder ${CMAKE_CURRENT_SOURCE_DIR}/dist
                        ${CMAKE_CURRENT_SOURCE_DIR}/recipe
                        1> ${CMAKE_CURRENT_SOURCE_DIR}/build.log 2>&1
                COMMAND ${CONDA} build purge
            )
        endforeach()
    else()
        message("-- Conda distribution not found.")
    endif()
else()
    message(FATAL_ERROR "Non-existent Python interpreter!")
endif()
