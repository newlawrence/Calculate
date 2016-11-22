cmake_minimum_required(VERSION 3.5)
project(calculate_python_wrapper NONE)

add_custom_target(calculate_python ALL
    COMMAND ${CMAKE_COMMAND} -E copy
            "$<TARGET_FILE:calculate>"
            "${CMAKE_CURRENT_SOURCE_DIR}/calculate"
    DEPENDS calculate
)
