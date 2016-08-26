cmake_minimum_required(VERSION 3.5)
project(calculate_python NONE)

add_custom_target(calculate_python
    COMMAND ${CMAKE_COMMAND} -E copy
            "$<TARGET_FILE:calculate>"
            "${CMAKE_CURRENT_SOURCE_DIR}/calculate"
    DEPENDS calculate
)

if (WIN32)
    add_custom_command(
        TARGET calculate_python
        POST_BUILD
        COMMAND cmake -E rename
                "${CMAKE_CURRENT_SOURCE_DIR}/calculate/calculate.dll"
                "${CMAKE_CURRENT_SOURCE_DIR}/calculate/libcalculate.dll"
    )
endif ()
