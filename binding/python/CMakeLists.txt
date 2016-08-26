cmake_minimum_required(VERSION 3.5)
project(calculate_python NONE)

set(SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../../lib")
set(TARGET_DIR "${CMAKE_CURRENT_SOURCE_DIR}/calculate")

add_custom_target(calculate_python
    COMMAND ${CMAKE_COMMAND} -E copy "${SOURCE_DIR}/*" "${TARGET_DIR}"
    COMMAND ${CMAKE_COMMAND} -E remove -f "${TARGET_DIR}/*.a"
    COMMAND ${CMAKE_COMMAND} -E remove -f "${TARGET_DIR}/*.lib"
    DEPENDS calculate
)
