add_executable( create_particle_data create_particle_data.cpp )
set_target_properties( create_particle_data PROPERTIES CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON POSITION_INDEPENDENT_CODE ON )
target_include_directories( create_particle_data
    PRIVATE $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}> )
if( DEMOTRACK_SYCL_INCLUDE_DIRS )
    target_include_directories( create_particle_data
        PRIVATE $<BUILD_INTERFACE:${DEMOTRACK_SYCL_INCLUDE_DIRS}> )
endif()
if( "${DEMOTRACK_SYCL_IMPL}" STREQUAL "computecpp" )
    add_sycl_to_target( TARGET create_particle_data
                        SOURCES create_particle_data.cpp )
endif()


add_executable( compare_particle_data compare_particle_data.cpp )
set_target_properties( compare_particle_data PROPERTIES CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON POSITION_INDEPENDENT_CODE ON )
target_include_directories( compare_particle_data
    PRIVATE $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}> )
if( DEMOTRACK_SYCL_INCLUDE_DIRS )
    target_include_directories( compare_particle_data
        PRIVATE $<BUILD_INTERFACE:${DEMOTRACK_SYCL_INCLUDE_DIRS}> )
endif()
if( "${DEMOTRACK_SYCL_IMPL}" STREQUAL "computecpp" )
    add_sycl_to_target( TARGET compare_particle_data
                        SOURCES compare_particle_data.cpp )
endif()


add_executable( print_particle_data print_particle_data.cpp )
set_target_properties( print_particle_data PROPERTIES CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON POSITION_INDEPENDENT_CODE ON )
target_include_directories( print_particle_data
    PRIVATE $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}> )
if( DEMOTRACK_SYCL_INCLUDE_DIRS )
    target_include_directories( print_particle_data
        PRIVATE $<BUILD_INTERFACE:${DEMOTRACK_SYCL_INCLUDE_DIRS}> )
endif()
if( "${DEMOTRACK_SYCL_IMPL}" STREQUAL "computecpp" )
    add_sycl_to_target( TARGET print_particle_data
                        SOURCES print_particle_data.cpp )
endif()
