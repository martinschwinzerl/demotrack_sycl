cmake_minimum_required( VERSION 3.11 FATAL_ERROR )
project( demotrack_sycl LANGUAGES CXX )

set( DEMOTRACK_SYCL_IMPL "computecpp" CACHE STRING
    "Intel dpc++ sycl implementation [values: dpcpp, trisycl, computecpp, hipsycl, sycl-gtx]" )

set( CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/Modules ${CMAKE_MODULE_PATH} )

set( DEMOTRACK_SYCL_INCLUDE_DIRS )
set( DEMOTRACK_SYCL_LINK_LIBRARIES )
set( DEMOTRACK_SYCL_COMPILE_DEFINITIONS )
set( DEMOTRACK_SYCL_COMPILE_OPTIONS )
set( DEMOTRACK_SYCL_LINK_DEFINITIONS )
set( DEMOTRACK_SYCL_LINK_OPTIONS )

if( "${DEMOTRACK_SYCL_IMPL}" STREQUAL "dpcpp" )
    string( APPEND DEMOTRACK_SYCL_COMPILE_OPTIONS "-fsycl" )
    string( APPEND DEMOTRACK_SYCL_LINK_OPTIONS "-fsycl" )
elseif( "${DEMOTRACK_SYCL_IMPL}" STREQUAL "trisycl" )
    find_package( triSYCL REQUIRED )
    find_package( Threads REQUIRED )
elseif( "${DEMOTRACK_SYCL_IMPL}" STREQUAL "computecpp" )
    find_package( ComputeCpp REQUIRED )
    set( DEMOTRACK_SYCL_INCLUDE_DIRS ${DEMOTRACK_SYCL_INCLUDE_DIRS}
         ${COMPUTECPP_INCLUDE_DIRECTORY} ${OpenCL_INCLUDE_DIR} )
elseif( "${DEMOTRACK_SYCL_IMPL}" STREQUAL "sycl-gtx" )
    find_package( sycl-gtx REQUIRED )
endif()

# ******************************************************************************
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
elseif( TARGET sycl-gtx::sycl-gtx )
    target_link_libraries( demo01_sc0 PRIVATE sycl-gtx::sycl-gtx )
endif()

if( DEMOTRACK_SYCL_INCLUDE_DIRS )
    target_include_directories( demo01_sc0
        PRIVATE $<BUILD_INTERFACE:${DEMOTRACK_SYCL_INCLUDE_DIRS}> )
endif()

if( DEMOTRACK_SYCL_LINK_LIBRARIES )
    target_link_libraries( demo01_sc0 PRIVATE ${DEMOTRACK_SYCL_LINK_LIBRARIES} )
endif()
