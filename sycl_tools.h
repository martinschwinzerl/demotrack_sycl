#ifndef DEMOTRACK_SYCL_SYCL_TOOLS_H__
#define DEMOTRACK_SYCL_SYCL_TOOLS_H__

#include <memory>
#include <ostream>

#include <CL/sycl.hpp>

namespace demotrack
{
    std::unique_ptr< cl::sycl::queue > Queue_factory()
    {
        namespace dpcpp = cl::sycl;
        #if defined( DEMOTRACK_DEVICE_SELECTOR_HOST ) && \
                   ( DEMOTRACK_DEVICE_SELECTOR_HOST == 1 )
        using selector_type = dpcpp::host_selector;
        #elif defined( DEMOTRACK_DEVICE_SELECTOR_CPU ) && \
                     ( DEMOTRACK_DEVICE_SELECTOR_CPU == 1 )
        using selector_type = dpcpp::cpu_selector;
        #elif defined( DEMOTRACK_DEVICE_SELECTOR_GPU ) && \
                     ( DEMOTRACK_DEVICE_SELECTOR_GPU == 1 )
        using selector_type = dpcpp::gpu_selector;
        #elif defined( DEMOTRACK_DEVICE_SELECTOR_ACCELERATOR ) && \
                     ( DEMOTRACK_DEVICE_SELECTOR_ACCELERATOR == 1 )
        using selector_type = dpcpp::accelerator_selector;
        #else
        using selector_type = dpcpp::default_selector;
        #endif /* defined( DEMOTRACK_DEVICE_SELECTOR_* ) */

        #if ( defined( DEMOTRACK_ENABLE_PROFILING ) && \
                     ( DEMOTRACK_ENABLE_PROFILING == 1 ) ) && \
            ( defined( DEMOTRACK_IN_ORDER_QUEUE ) && \
                     ( DEMOTRACK_IN_ORDER_QUEUE == 1 ) )
        dpcpp::property_list queue_properties{
            dpcpp::property::queue::enable_profiling(),
            dpcpp::property::queue::in_order() };
        #elif (  defined( DEMOTRACK_ENABLE_PROFILING ) && \
                        ( DEMOTRACK_ENABLE_PROFILING == 1 ) ) && \
              ( !defined( DEMOTRACK_IN_ORDER_QUEUE ) || \
                        ( DEMOTRACK_IN_ORDER_QUEUE == 0 ) )
        dpcpp::property_list queue_properties{
            dpcpp::property::queue::enable_profiling() };
        #elif ( !defined( DEMOTRACK_ENABLE_PROFILING ) || \
                        ( DEMOTRACK_ENABLE_PROFILING == 0 ) ) && \
              (  defined( DEMOTRACK_IN_ORDER_QUEUE ) && \
                        ( DEMOTRACK_IN_ORDER_QUEUE == 1 ) )
        dpcpp::property_list queue_properties{
            dpcpp::property::queue::in_order() };

        #else
        dpcpp::property_list queue_properties{};
        #endif /* DEMOTRACK_ENABLE_PROFILING || DEMOTRACK_IN_ORDER_QUEUE */

        return std::make_unique< dpcpp::queue >(
            selector_type{}, queue_properties );
    }

    void Queue_print_info( std::ostream& ostr,
                           const cl::sycl::queue *const queue )
    {
        namespace dpcpp = cl::sycl;
        if( queue == nullptr ) return;

        ostr << "Device type           : ";

        if( queue->is_host() )
        {
            ostr << "HOST\r\n";
        }
        else
        {
            auto dev = queue->get_device();
            auto platform = dev.get_platform();

            if( dev.is_cpu() )
            {
                ostr << "CPU\r\n";
            }
            else if( dev.is_gpu() )
            {
                ostr << "GPU\r\n";
            }
            else if( dev.is_accelerator() )
            {
                ostr << "ACCELERATOR\r\n";
            }
            else
            {
                ostr << "UNKNOWN\r\n";
            }

            ostr << "Platform              : "
                 << platform.get_info< dpcpp::info::platform::name >() << "\r\n"
                 << "Device                : "
                 << dev.get_info< dpcpp::info::device::name >() << "\r\n";
        }
    }
}

#endif /* DEMOTRACK_SYCL_SYCL_TOOLS_H__ */
