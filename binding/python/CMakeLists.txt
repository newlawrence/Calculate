cmake_minimum_required(VERSION 3.5)
project(calculate_python_wrapper NONE)
set(PYTHON_SUPPORTED_VERSIONS 2.7 3.3 3.4 3.5 3.6)


set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
add_library(calculate_python SHARED "${sources}")
target_compile_definitions(calculate_python PUBLIC "_CRT_SECURE_NO_WARNINGS")
set_target_properties(calculate_python PROPERTIES PREFIX "lib")

if(DEFINED ENV{NORUNTIME})
    if(UNIX AND NOT APPLE)
        target_link_libraries(calculate_python PUBLIC
            -static-libgcc -static-libstdc++
        )
    elseif(WIN32)
        if(
            (${CMAKE_CXX_COMPILER_ID} MATCHES "GNU") OR
            (${CMAKE_CXX_COMPILER_ID} MATCHES "Clang")
        )
            target_link_libraries(calculate_python PUBLIC
                -static-libgcc -static-libstdc++
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
            "$<TARGET_FILE:calculate_python>"
            "${CMAKE_CURRENT_SOURCE_DIR}/calculate"
)


find_program(PYTHON "python")
find_program(CONDA "conda")

if(PYTHON)
    execute_process(
        COMMAND ${PYTHON} --version
        OUTPUT_VARIABLE PYTHON_VER
        ERROR_VARIABLE PYTHON_VER
    )
    string(STRIP ${PYTHON_VER} PYTHON_VER)
    message("-- The Python interpreter specification is ${PYTHON_VER}")
    set(SETUP "${CMAKE_CURRENT_SOURCE_DIR}/setup.py")

    set(INIT_IN "${CMAKE_CURRENT_SOURCE_DIR}/calculate/__init__.py.in")
    set(INIT "${CMAKE_CURRENT_SOURCE_DIR}/calculate/__init__.py")
    configure_file(${INIT_IN} ${INIT})

    set(RECIPE_IN "${CMAKE_CURRENT_SOURCE_DIR}/recipe/meta.yaml.in")
    set(RECIPE "${CMAKE_CURRENT_SOURCE_DIR}/recipe/meta.yaml")
    configure_file(${RECIPE_IN} ${RECIPE})

    add_custom_target(python_wheel
        DEPENDS calculate_python
        COMMAND ${PYTHON} ${SETUP} bdist_wheel --universal
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
                        ${CMAKE_CURRENT_SOURCE_DIR}/recipe
                COMMAND ${CONDA} build purge
            )
        endforeach()
    endif()
endif()
