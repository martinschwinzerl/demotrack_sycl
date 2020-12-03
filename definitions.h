#ifndef DEMOTRACK_SYCL_DEFINITIONS_H__
#define DEMOTRACK_SYCL_DEFINITIONS_H__

#include <CL/sycl.hpp>

#if defined( __APPLE__ ) && __APPLE__
    #include <TargetConditionals.h>
#endif /* defined( __APPLE__ ) */

#if !defined( DEMOTRACK_DEVICE_FN )
    #if defined( __SYCL_DEVICE_ONLY__ )
        #define DEMOTRACK_DEVICE_FN __SYCL_DEVICE_ONLY__
    #else /* !defined( DEMOTRACK_DEVICE_FN ) */
        #define DEMOTRACK_DEVICE_FN
    #endif /* !defined( DEMOTRACK_DEVICE_FN ) */
#endif /* !defined( DEMOTRACK_DEVICE_FN ) */

#if !defined( DEMOTRACK_HOST_FN )
    #define DEMOTRACK_HOST_FN
#endif /* !defined( DEMOTRACK_HOST_FN ) */

#if !defined( DEMOTRACK_FN )
    #if defined( __SYCL_SINGLE_SOURCE__ )
        #define DEMOTRACK_FN __SYCL_SINGLE_SOURCE__
    #else /* !defined( __SYCL_SINGLE_SOURCE__ ) */
        #define DEMOTRACK_FN
    #endif /* !defined( __SYCL_SINGLE_SOURCE__ ) */
#endif /* !defined( DEMOTRACK_FN ) */

#if !defined( DEMOTRACK_EXTERN )
    #if defined( SYCL_EXTERNAL )
        #define DEMOTRACK_EXTERN SYCL_EXTERNAL
    #else
        #define DEMOTRACK_EXTERN
    #endif /* defined( SYCL_EXTERNAL ) */
#endif /* !defined( DEMOTRACK_EXTERN ) */

namespace demotrack
{
    #if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
        using int32_type = long int ;
        using uint32_type = unsigned long int;
        using int64_type = long long int ;
        using uint64_type = unsigned long long int ;
    #elif __APPLE__ && defined( TARGET_OS_MAC )
        using int32_type = long int ;
        using uint32_type = unsigned long int;

        using int64_type = long long int ;
        using uint64_type = unsigned long long int ;
    #elif __linux__
        using int32_type = int ;
        using uint32_type = unsigned int;

        using int64_type = long long int ;
        using uint64_type = unsigned long long int ;
    #else
        #error "Unknown platform"
    #endif

    using size_type = uint64_type;
}

#endif /* DEMOTRACK_SYCL_DEFINITIONS_H__ */
