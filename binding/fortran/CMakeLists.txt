cmake_minimum_required(VERSION 3.5)
project(calculate_forwrap Fortran)

set(CMAKE_Fortran_MODULE_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/module/")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/lib/")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/lib/")

file(GLOB sources "${PROJECT_SOURCE_DIR}/source/*.f90")

if (${CMAKE_Fortran_COMPILER_ID} MATCHES "GNU")
    set(CMAKE_Fortran_FLAGS "${CMAKE_Fortran_FLAGS} -O3 -std=f2008ts")
    set(CMAKE_Fortran_FLAGS "${CMAKE_Fortran_FLAGS} -Wall")
elseif (${CMAKE_Fortran_COMPILER_ID} MATCHES "Intel")
    if (WIN32)
        set(CMAKE_Fortran_FLAGS "${CMAKE_Fortran_FLAGS} /O3 /stand:f08")
        set(CMAKE_Fortran_FLAGS "${CMAKE_Fortran_FLAGS} /free /warn:all")
    else ()
        set(CMAKE_Fortran_FLAGS "${CMAKE_Fortran_FLAGS} -O3 -stand f08")
        set(CMAKE_Fortran_FLAGS "${CMAKE_Fortran_FLAGS} -free -warn all")
    endif ()
endif ()

add_library(calculate_fortran STATIC "${sources}")
target_link_libraries(calculate_fortran PUBLIC calculate)

add_library(calculate_fortran_static STATIC "${sources}")
target_link_libraries(calculate_fortran_static PUBLIC calculate_static)
