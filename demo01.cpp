#include <CL/sycl.hpp>

#include <chrono>
#include <iostream>
#include <stdexcept>
#include <vector>

#include "definitions.h"
#include "config.h"
#include "particle.h"
#include "lattice.h"
#include "sycl_tools.h"
#include "track.h"

namespace demotrack
{
    class TrackUntil; // forward declare kernel functor
}

int main( int const argc, char* argv[] )
{
    namespace dt = demotrack;
    namespace dpcpp = cl::sycl;

    /* ********************************************************************* */
    /* Prepare particle set to track */

    dt::uint64_type NUM_PARTICLES = 50 * 1024;
    dt::int64_type  TRACK_UNTIL_TURN = 1000;
    std::string path_to_lattice_data = std::string{};
    std::string path_to_particle_data = std::string{};
    std::string path_to_output_data = std::string{};

    if( argc >= 2 )
    {
        NUM_PARTICLES = std::stoi( argv[ 1 ] );

        if( argc >= 3 )
        {
            TRACK_UNTIL_TURN = std::stoi( argv[ 2 ] );

            if( argc >= 4 )
            {
                path_to_particle_data = std::string{ argv[ 3 ] };
                if( path_to_particle_data.compare( "default" ) == 0 ) {
                    path_to_particle_data.clear(); }

                if( argc >= 5 )
                {
                    path_to_lattice_data = std::string{ argv[ 4 ] };
                    if( path_to_lattice_data.compare( "default" ) == 0 ) {
                        path_to_lattice_data.clear(); }

                    if( argc >= 6 )
                    {
                        path_to_output_data = std::string{ argv[ 5 ] };
                        if( path_to_output_data.compare( "none" ) == 0 ) {
                            path_to_output_data.clear(); }
                    }
                }
            }
        }
    }
    else
    {
        std::cout << "Usage : " << argv[ 0 ]
                  << " [NUM_PARTICLES] [TRACK_UNTIL_TURN]"
                  << " [PATH_TO_PARTICLE_DATA] [PATH_TO_LATTICE_DATA]"
                  << " [PATH_TO_OUTPUT_DATA]\r\n"
                  << std::endl;
    }

    /* ********************************************************************** */
    /* Select device and create a queue */

    auto queue = dt::Queue_factory();
    if( queue.get() == nullptr )
        throw std::runtime_error( "unable to create queue" );

    std::cout << "number of particles   : " << NUM_PARTICLES << "\r\n"
              << "number of turns       : " << TRACK_UNTIL_TURN << "\r\n";

    if( !path_to_particle_data.empty() )
    {
        std::cout << "particle data         : "
                  << path_to_particle_data << "\r\n";
    }
    else
    {
        std::cout << "particle data         : generated\r\n";
    }

    if( !path_to_lattice_data.empty() )
    {
        std::cout << "lattice               : "
                  << path_to_lattice_data << "\r\n";
    }
    else
    {
        std::cout << "lattice               : generated fodo lattice\r\n";
    }

    #if defined( DEMOTRACK_ENABLE_BEAMFIELDS ) && \
               ( DEMOTRACK_ENABLE_BEAMFIELDS == 1 )
    std::cout << "space-charge enabled  : true\r\n";
    #else
    std::cout << "space-charge enabled  : false\r\n";
    #endif /* SC emabled */

    dt::Queue_print_info( std::cout, queue.get() );

    if( !path_to_output_data.empty() )
    {
        std::cout << "path to output data : " << path_to_output_data << "\r\n";
    }

    /* ********************************************************************* */
    /* Prepare particle data: */

    std::vector< dt::Particle > particles_host;
    dt::Particles_load( particles_host, NUM_PARTICLES, path_to_particle_data );

    /* ********************************************************************* */
    /* Prepare lattice / machine description: */

    std::vector< double > lattice_host;
    dt::uint64_type const LATTICE_SIZE = dt::load_lattice(
        lattice_host, path_to_lattice_data );

    /* ********************************************************************** */
    /* prepare device buffers */

    dpcpp::buffer< double, 1 > lattice_dev(
        lattice_host.data(), dpcpp::range< 1 >( LATTICE_SIZE ) );

    dpcpp::buffer< dt::Particle, 1 > particle_dev(
        particles_host.data(), dpcpp::range< 1 >( NUM_PARTICLES ) );

    /* ********************************************************************** */
    /* Run tracking kernel */

    #if !defined( DEMOTRACK_ENABLE_PROFILING ) || \
                ( DEMOTRACK_ENABLE_PROFILING != 1 )
    auto start_time = std::chrono::steady_clock::now();
    #endif /* !defined( DEMOTRACK_ENABLE_PROFILING ) */

    auto ev = queue->submit(
        [&]( dpcpp::handler& h ){
        auto pset = particle_dev.get_access< dpcpp::access::mode::read_write >( h );
        auto lattice = lattice_dev.get_access< dpcpp::access::mode::read >( h );

        h.parallel_for< dt::TrackUntil >( dpcpp::range< 1 >{ NUM_PARTICLES },
            [=]( dpcpp::id< 1 > idx ) {
                demotrack::Particle_track_until_turn(
                    pset[ idx ], lattice, LATTICE_SIZE, TRACK_UNTIL_TURN ); }
        ); } );

    ev.wait(); // Synchronizes kernel call

    #if !defined( DEMOTRACK_ENABLE_PROFILING ) || \
                ( DEMOTRACK_ENABLE_PROFILING != 1 )
    auto stop_time = std::chrono::steady_clock::now();
    std::chrono::duration< double > const d_time = stop_time - start_time;
    double const elapsed_time = d_time.count();
    #else /* defined( DEMOTRACK_ENABLE_PROFILING ) */
    auto start_time = ev.get_profiling_info<
        dpcpp::info::event_profiling::command_start>();

    auto stop_time = ev.get_profiling_info<
        dpcpp::info::event_profiling::command_end >();

    double const elapsed_time = ( stop_time - start_time ) * double{ 1e-9 };
    #endif /* !defined( DEMOTRACK_ENABLE_PROFILING ) */

    double const elapsed_time_per_turn_particle = elapsed_time /
        std::max( static_cast< double >( NUM_PARTICLES * TRACK_UNTIL_TURN ),
                  double{ 1. } );

    std::cout << "-------------------------------------------------------\r\n"
              << "Elapsed time        : " << elapsed_time << " sec total \r\n"
              << "                    : " << elapsed_time_per_turn_particle
              << " sec /  particle / turn\r\n"
              << std::endl;

    /* ********************************************************************** */
    /* Retrieve tracking result */

    auto pset = particle_dev.get_access< dpcpp::access::mode::read >();

    /* ********************************************************************* */
    /* Verify tracking results */

    dt::uint64_type num_active_particles = 0u;
    dt::uint64_type num_lost_particles = 0u;

    for( std::size_t ii = 0u ; ii < NUM_PARTICLES ; ++ii )
    {
        auto& p = pset[ ii ];

        if( ( pset[ ii ].state == 1 ) &&
            ( pset[ ii ].at_turn == TRACK_UNTIL_TURN ) )
        {
            ++num_active_particles;
        }
        else if( ( pset[ ii ].state == 0 ) &&
                 ( pset[ ii ].at_turn < TRACK_UNTIL_TURN ) )
        {
            ++num_lost_particles;
        }
        else
        {
            std::cerr << "illegal particle id = " << pset[ ii ].id
                      << ", at_turn = "           << pset[ ii ].at_turn
                      << ", at_element = "        << pset[ ii ].at_element
                      << ", state = "             << pset[ ii ].state
                      << std::endl;
        }
    }

    std::cout << "-------------------------------------------------------\r\n"
              << "num lost particles    : " << num_lost_particles   << "\r\n"
              << "num active particles  : " << num_active_particles << "\r\n"
              << std::endl;

    /* ********************************************************************* */
    /* Create output file */

    if( !path_to_output_data.empty() )
    {
        ::FILE* fp = std::fopen( path_to_output_data.c_str(), "wb" );
        double const temp = static_cast< double >( particles_host.size() );
        auto ret = std::fwrite( &temp, sizeof( double ), 1u, fp );
        bool success = ( ret == 1 );

        for( auto const& p : particles_host )
        {
            ret = std::fwrite( &p, sizeof( dt::Particle ), 1u, fp );
            success &= ( ret == 1 );
        }

        std::fflush( fp );
        std::fclose( fp );
        fp = nullptr;

        if( success )
        {
            std::cout << "Written particle state to " << path_to_output_data
                      << "\r\n" << std::endl;
        }
        else
        {
            std::remove( path_to_output_data.c_str() );
        }
    }

    return 0;
}
