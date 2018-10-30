cmake_minimum_required(VERSION 3.5)
project(Calculate LANGUAGES CXX VERSION 2.1.1)


option(CALCULATE_BUILD_EXAMPLES "Add Calculate's example targets" OFF)
option(CALCULATE_BUILD_TESTS "Add Calculate's test targets" OFF)

if(CMAKE_SOURCE_DIR STREQUAL PROJECT_SOURCE_DIR)
    set(CMAKE_CONFIGURATION_TYPES "Release;Debug" CACHE STRING "Configurations" FORCE)
    if(NOT CMAKE_BUILD_TYPE)
        if(EXISTS "${CMAKE_SOURCE_DIR}/.git")
            set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "Build type" FORCE)
        else()
            set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Build type" FORCE)
        endif()
    endif()
    message(STATUS "Calculate: setting build type to '${CMAKE_BUILD_TYPE}'")

    if(NOT CMAKE_INSTALL_INCLUDEDIR})
        set(CMAKE_INSTALL_INCLUDEDIR "${CMAKE_INSTALL_PREFIX}/include")
    endif()

    if(NOT CMAKE_INSTALL_LIBDIR})
        set(CMAKE_INSTALL_LIBDIR "${CMAKE_INSTALL_PREFIX}/lib")
    endif()
endif()


add_library(Calculate INTERFACE)

target_compile_features(Calculate INTERFACE cxx_std_14)

target_include_directories(Calculate
INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

add_library(Calculate::Calculate ALIAS Calculate)


include(CMakePackageConfigHelpers)
set(CALCULATE_CONFIG_DIR "${CMAKE_INSTALL_LIBDIR}/cmake/Calculate")

configure_package_config_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake/CalculateConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/CalculateConfig.cmake
    INSTALL_DESTINATION ${CALCULATE_CONFIG_DIR}
)

write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/CalculateConfigVersion.cmake"
    COMPATIBILITY SameMajorVersion
)

install(TARGETS Calculate EXPORT CalculateTargets DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
install(EXPORT CalculateTargets NAMESPACE Calculate:: DESTINATION ${CALCULATE_CONFIG_DIR})
install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/include/" DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}")

install(
    FILES
        "${CMAKE_CURRENT_BINARY_DIR}/CalculateConfig.cmake"
        "${CMAKE_CURRENT_BINARY_DIR}/CalculateConfigVersion.cmake"
    DESTINATION ${CALCULATE_CONFIG_DIR}
)


if(CALCULATE_BUILD_EXAMPLES)
    add_subdirectory(example)
endif()

if(CALCULATE_BUILD_TESTS)
    enable_testing()
    add_subdirectory(test)
endif()
