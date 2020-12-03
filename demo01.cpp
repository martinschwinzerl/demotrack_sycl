#include <CL/sycl.hpp>

#include <chrono>
#include <iostream>
#include <vector>


#include "definitions.h"
#include "particle.h"
#include "fodo_lattice.h"
#include "track.h"

namespace demotrack
{
    class TrackUntil;
}

int main( int const argc, char* argv[] )
{
    namespace dt = demotrack;
    namespace dpcpp = cl::sycl;

    /* ********************************************************************* */
    /* Prepare particle set to track */

    std::size_t NUM_PARTICLES = 50 * 1024;
    dt::int64_type  TRACK_UNTIL_TURN = 10;

    if( argc >= 2 )
    {
        NUM_PARTICLES = std::stoi( argv[ 1 ] );

        if( argc >= 3 )
        {
            TRACK_UNTIL_TURN = std::stoi( argv[ 2 ] );
        }
    }
    else
    {
        std::cout << "Usage : " << argv[ 0 ]
                  << " [NUM_PARTICLES] [TRACK_UNTIL_TURN]\r\n"
                  << std::endl;
    }

    /* ********************************************************************** */
    /* Select device and create a queue */

    dpcpp::queue queue{ dpcpp::cpu_selector{} };
    //dpcpp::queue queue{ dpcpp::host_selector{} };

    if( queue.is_host() )
    {
        std::cout << "Selected host\r\n";
    }
    else
    {
        std::cout << "Selected device: "
                  << queue.get_device().get_info< dpcpp::info::device::name >()
                  << "\r\n";
    }

    /* ********************************************************************** */
    /* prepare host side data */

    double lattice_data[ 52 ];
    dt::uint64_type const lattice_size =
        dt::create_fodo_lattice( &lattice_data[ 0 ], 52 );

    std::vector< dt::Particle > particle_data( NUM_PARTICLES, dt::Particle{} );

    double const P0_C    = 470e9;  /* Kinetic energy, [eV]  */
    double const MASS0   = 938.272081e6; /* Proton rest mass, [eV] */
    double const CHARGE0 = 1.0; /* Reference particle charge; here == proton */
    double const DELTA   = 0.0; /* momentum deviation from reference particle */

    dt::uint64_type ii = dt::uint64_type{ 0 };
    for( auto& pp : particle_data )
    {
        pp.init( MASS0, CHARGE0, P0_C, DELTA );
        pp.id = ii++;
    }

    /* ********************************************************************** */
    /* prepare device buffers */

    dpcpp::buffer< double, 1 > lattice_buffer(
        lattice_data, dpcpp::range< 1 >( lattice_size ) );

    dpcpp::buffer< dt::Particle, 1 > particle_buffer(
        particle_data.data(), dpcpp::range< 1 >( particle_data.size() ) );

    /* ********************************************************************** */
    /* Run tracking kernel */

    auto start_time = std::chrono::steady_clock::now();

    queue.submit( [&]( dpcpp::handler& h ){
        auto p = particle_buffer.get_access< dpcpp::access::mode::read_write >( h );
        auto lattice = lattice_buffer.get_access< dpcpp::access::mode::read >( h );

        h.parallel_for< dt::TrackUntil >( dpcpp::range< 1 >{ NUM_PARTICLES },
            [=]( dpcpp::id< 1 > idx )
            {
                demotrack::Particle_track_until_turn(
                    p[ idx ], lattice, lattice_size, TRACK_UNTIL_TURN );
            }
        );
    } ).wait();

    auto stop_time = std::chrono::steady_clock::now();
    std::chrono::duration< double > const wtime = stop_time - start_time;

    std::cout << "-------------------------------------------------------\r\n"
              << "Elapsed time        : " << wtime.count() << " sec total \r\n"
              << "                    : " << wtime.count() / (
                std::max( NUM_PARTICLES * TRACK_UNTIL_TURN,
                          dt::uint64_type{ 1 } ) ) << " sec / particle / turn\r\n"
              << std::endl;

    /* ********************************************************************** */
    /* Retrieve tracking result */

    auto p_acc = particle_buffer.get_access< dpcpp::access::mode::read >();

    /* ********************************************************************* */
    /* Verify tracking results */

    dt::uint64_type num_active_particles = 0u;
    dt::uint64_type num_lost_particles = 0u;

    for( std::size_t ii = 0u ; ii < NUM_PARTICLES ; ++ii )
    {
        auto& p = p_acc[ ii ];

        if( ( p.state == 1 ) && ( p.at_turn == TRACK_UNTIL_TURN ) )
        {
            ++num_active_particles;
        }
        else if( ( p.state == 0 ) && ( p.at_turn < TRACK_UNTIL_TURN ) )
        {
            ++num_lost_particles;
        }
        else
        {
            std::cerr << "illegal particle id = " << p.id
                      << ", at_turn = " << p.at_turn
                      << ", at_element = " << p.at_element
                      << ", state = " << p.state << std::endl;
        }
    }

    std::cout << "-------------------------------------------------------\r\n"
              << "num lost particles    : " << num_lost_particles << "\r\n"
              << "num active particles  : " << num_active_particles << "\r\n"
              << std::endl;

    return 0;
}
