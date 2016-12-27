cmake_minimum_required(VERSION 3.7)
project(calculate_fortran_wrapper Fortran)


set(CMAKE_Fortran_MODULE_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/module/")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/lib/")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/lib/")

file(GLOB sources "${PROJECT_SOURCE_DIR}/source/*.f90")

if(${CMAKE_Fortran_COMPILER_ID} MATCHES "GNU")
    set(CMAKE_Fortran_FLAGS "${CMAKE_Fortran_FLAGS} -O3 -std=f2008")
    set(CMAKE_Fortran_FLAGS "${CMAKE_Fortran_FLAGS} -Wall")
elseif(${CMAKE_Fortran_COMPILER_ID} MATCHES "Intel")
    if(WIN32)
        set(CMAKE_Fortran_FLAGS "${CMAKE_Fortran_FLAGS} /O3 /stand:f08")
        set(CMAKE_Fortran_FLAGS "${CMAKE_Fortran_FLAGS} /free /warn:all")
    else()
        set(CMAKE_Fortran_FLAGS "${CMAKE_Fortran_FLAGS} -O3 -stand f08")
        set(CMAKE_Fortran_FLAGS "${CMAKE_Fortran_FLAGS} -free -warn all")
    endif()
endif()


add_library(calculate_fortran_d STATIC "${sources}")
target_link_libraries(calculate_fortran_d PUBLIC calculate)
set_target_properties(calculate_fortran_d PROPERTIES OUTPUT_NAME
                      "calculate_fortran_d.${PACKAGE_VERSION}")

add_library(calculate_fortran_s STATIC "${sources}")
target_link_libraries(calculate_fortran_s PUBLIC calculate_static)
set_target_properties(calculate_fortran_s PROPERTIES OUTPUT_NAME
                      "calculate_fortran_s.${PACKAGE_VERSION}")


include_directories("${CMAKE_CURRENT_SOURCE_DIR}/module")
add_executable(calc_for "${CMAKE_CURRENT_SOURCE_DIR}/../../examples/calc.f90")
target_link_libraries(calc_for PUBLIC calculate_fortran_d)
set_target_properties(calc_for PROPERTIES LINKER_LANGUAGE CXX)
