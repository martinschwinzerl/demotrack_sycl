if( NOT  DEMOTRACK_CMAKE_SETUP_SYCL_FINISHED )
    set( DEMOTRACK_CMAKE_SETUP_SYCL_FINISHED 1 )
    message(STATUS "---- Processing cmake/SetupSyCL.cmake")

    set( DEMOTRACK_SYCL_INCLUDE_DIRS )
    set( DEMOTRACK_SYCL_LINK_LIBRARIES )
    set( DEMOTRACK_SYCL_COMPILE_DEFINITIONS )
    set( DEMOTRACK_SYCL_COMPILE_OPTIONS )
    set( DEMOTRACK_SYCL_LINK_DEFINITIONS )
    set( DEMOTRACK_SYCL_LINK_OPTIONS )

    set( DEMOTRACK_SYCL_USE_COMPUTECPP "0" )
    set( DEMOTRACK_SYCL_USE_DPCPP      "0" )
    set( DEMOTRACK_SYCL_USE_TRISYCL    "0" )

    if( "${DEMOTRACK_SYCL_IMPL}" STREQUAL "dpcpp" )
        string( APPEND DEMOTRACK_SYCL_COMPILE_OPTIONS "-fsycl" )
        string( APPEND DEMOTRACK_SYCL_LINK_OPTIONS "-fsycl" )
        set( DEMOTRACK_SYCL_USE_DPCPP "1" )
    elseif( "${DEMOTRACK_SYCL_IMPL}" STREQUAL "trisycl" )
        find_package( triSYCL REQUIRED )
        find_package( Threads REQUIRED )
        set( DEMOTRACK_SYCL_USE_TRISYCL "1" )
    elseif( "${DEMOTRACK_SYCL_IMPL}" STREQUAL "computecpp" )
        find_package( ComputeCpp REQUIRED )
        set( DEMOTRACK_SYCL_USE_COMPUTECPP "1" )
        set( DEMOTRACK_SYCL_INCLUDE_DIRS ${DEMOTRACK_SYCL_INCLUDE_DIRS}
            ${COMPUTECPP_INCLUDE_DIRECTORY} ${OpenCL_INCLUDE_DIR} )
    endif()

    set( DEMOTRACK_DEVICE_SELECTOR_STR  "n/a" )
    set( DEMOTRACK_DEVICE_SELECTOR_HOST "0" )
    set( DEMOTRACK_DEVICE_SELECTOR_CPU  "0" )
    set( DEMOTRACK_DEVICE_SELECTOR_GPU  "0" )
    set( DEMOTRACK_DEVICE_SELECTOR_ACCELERATOR "0" )

    if( DEFINED DEMOTRACK_DEVICE_SELECTOR )
        if( "${DEMOTRACK_DEVICE_SELECTOR}" STREQUAL "host" )
            set( DEMOTRACK_DEVICE_SELECTOR_HOST "1" )
            set( DEMOTRACK_DEVICE_SELECTOR_STR  "host" )
        elseif( "${DEMOTRACK_DEVICE_SELECTOR}" STREQUAL "cpu" )
            set( DEMOTRACK_DEVICE_SELECTOR_CPU "1" )
            set( DEMOTRACK_DEVICE_SELECTOR_STR "cpu" )
        elseif( "${DEMOTRACK_DEVICE_SELECTOR}" STREQUAL "gpu" )
            set( DEMOTRACK_DEVICE_SELECTOR_GPU "1" )
            set( DEMOTRACK_DEVICE_SELECTOR_STR "gpu" )
        elseif( "${DEMOTRACK_DEVICE_SELECTOR}" STREQUAL "accelerator" )
            set( DEMOTRACK_DEVICE_SELECTOR_GPU "1" )
            set( DEMOTRACK_DEVICE_SELECTOR_STR "accelerator" )
        else()
            message( STATUS "No legal device selector configured -> use host as fallback" )
            set( DEMOTRACK_DEVICE_SELECTOR_HOST "1" )
            set( DEMOTRACK_DEVICE_SELECTOR_STR  "host" )
        endif()
    else()
        message( STATUS "No legal device selector available -> use host as fallback" )
        set( DEMOTRACK_DEVICE_SELECTOR "host" CACHE STRING "" FORCE )
        set( DEMOTRACK_DEVICE_SELECTOR_HOST "1" )
    endif()

    if( DEFINED DEMOTRACK_ENABLE_PROFILING AND DEMOTRACK_ENABLE_PROFILING )
        set( DEMOTRACK_ENABLE_PROFILING_FLAG "1" )
    else()
        set( DEMOTRACK_ENABLE_PROFILING_FLAG "0" )
    endif()

    if( DEFINED DEMOTRACK_IN_ORDER_QUEUE AND DEMOTRACK_IN_ORDER_QUEUE )
        set( DEMOTRACK_IN_ORDER_QUEUE_FLAG "1" )
    else()
        set( DEMOTRACK_IN_ORDER_QUEUE_FLAG "0" )
    endif()

endif()
