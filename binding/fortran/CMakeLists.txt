cmake_minimum_required(VERSION 3.7)
project(calculate_fortran_wrapper Fortran)


set(CMAKE_Fortran_MODULE_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/module/")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/lib/")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/lib/")


if(${CMAKE_Fortran_COMPILER_ID} MATCHES "GNU")
    set(CMAKE_Fortran_FLAGS "${CMAKE_Fortran_FLAGS} -O3 -std=f2008")
    set(CMAKE_Fortran_FLAGS "${CMAKE_Fortran_FLAGS} -Wall -Wno-surprising")
elseif(${CMAKE_Fortran_COMPILER_ID} MATCHES "Intel")
    if(WIN32)
        set(CMAKE_Fortran_FLAGS "${CMAKE_Fortran_FLAGS} /O3 /stand:f08")
        set(CMAKE_Fortran_FLAGS "${CMAKE_Fortran_FLAGS} /free /warn:all")
    else()
        set(CMAKE_Fortran_FLAGS "${CMAKE_Fortran_FLAGS} -O3 -stand f08")
        set(CMAKE_Fortran_FLAGS "${CMAKE_Fortran_FLAGS} -free -warn all")
    endif()
endif()

file(GLOB sources "${PROJECT_SOURCE_DIR}/source/*.f90")


add_library(calculate_fortran STATIC "${sources}")
target_link_libraries(calculate_fortran PUBLIC calculate)
set_target_properties(calculate_fortran PROPERTIES OUTPUT_NAME
                      "calculate_fortran.${PACKAGE_VERSION}")

add_library(calculate_fortran_static STATIC "${sources}")
target_link_libraries(calculate_fortran_static PUBLIC calculate_static)
set_target_properties(calculate_fortran_static PROPERTIES OUTPUT_NAME
                      "calculate_fortran_static.${PACKAGE_VERSION}")


include_directories("${CMAKE_CURRENT_SOURCE_DIR}/module")
add_executable(calc_for "${CMAKE_CURRENT_SOURCE_DIR}/../../examples/calc.f90")
target_link_libraries(calc_for PUBLIC calculate_fortran)
set_target_properties(calc_for PROPERTIES LINKER_LANGUAGE CXX)
