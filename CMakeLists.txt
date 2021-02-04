cmake_minimum_required( VERSION 3.11 FATAL_ERROR )
project( demotrack_sycl LANGUAGES CXX )

set( CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake ${CMAKE_MODULE_PATH} )
set( CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/Modules ${CMAKE_MODULE_PATH} )

if( EXISTS ${CMAKE_SOURCE_DIR}/Settings.cmake )
    message( STATUS "loading custom settings file ${CMAKE_SOURCE_DIR}/Settings.cmake" )
    include( ${CMAKE_SOURCE_DIR}/Settings.cmake )
else()
    message( STATUS "loading default settings file ${CMAKE_SOURCE_DIR}/Settings.cmake.original" )
    include( ${CMAKE_SOURCE_DIR}/Settings.cmake.original )
endif()

# -----------------------------------------------------------------------------
# Setup SyCL and generate config.h

include( SetupSyCL )

configure_file( ${CMAKE_SOURCE_DIR}/config.h.template
                ${CMAKE_SOURCE_DIR}/config.h @ONLY )

# -----------------------------------------------------------------------------
# tools

add_subdirectory( tools )

# -----------------------------------------------------------------------------
# demo applications

add_executable( demo01_sc0 demo01.cpp )

set_target_properties( demo01_sc0 PROPERTIES
    CXX_STANDARD 17 CXX_STANDARD_REQUIRED 17 )

if( "${DEMOTRACK_SYCL_IMPL}" STREQUAL "computecpp" )
    add_sycl_to_target( TARGET demo01_sc0 SOURCES demo01.cpp )
endif()

if( DEMOTRACK_SYCL_COMPILE_OPTIONS )
    target_compile_options( demo01_sc0 PRIVATE ${DEMOTRACK_SYCL_COMPILE_OPTIONS} )
endif()

if( DEMOTRACK_SYCL_LINK_OPTIONS )
    target_link_options( demo01_sc0 PRIVATE ${DEMOTRACK_SYCL_LINK_OPTIONS} )
endif()

target_include_directories( demo01_sc0
    PRIVATE $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}> )

if( TARGET triSYCL::triSYCL )
    target_link_libraries( demo01_sc0 PRIVATE triSYCL::triSYCL Threads::Threads )
endif()

if( DEMOTRACK_SYCL_INCLUDE_DIRS )
    target_include_directories( demo01_sc0
        PRIVATE $<BUILD_INTERFACE:${DEMOTRACK_SYCL_INCLUDE_DIRS}> )
endif()

if( DEMOTRACK_SYCL_LINK_LIBRARIES )
    target_link_libraries( demo01_sc0 PRIVATE ${DEMOTRACK_SYCL_LINK_LIBRARIES} )
endif()
