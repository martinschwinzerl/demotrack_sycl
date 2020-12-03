#ifndef DEMOTRACK_SYCL_BEAM_ELEMENTS_H__
#define DEMOTRACK_SYCL_BEAM_ELEMENTS_H__

#include <CL/sycl.hpp>

#include "definitions.h"
#include "particle.h"

namespace demotrack
{
    typedef enum
    {
        BEAM_ELEMENT_DRIFT = 2,
        BEAM_ELEMENT_DRIFT_EXACT = 3,
        BEAM_ELEMENT_MULTIPOLE = 4,
        BEAM_ELEMENT_CAVITY = 5,
        BEAM_ELEMENT_SC_COASTING = 16,
        BEAM_ELEMENT_END_OF_TRACK = 255
    }
    beam_element_type;

    struct __attribute__((aligned( 8 ))) Drift
    {
        static constexpr uint64_type NUM_SLOTS() noexcept
        {
            return uint64_type{ 2 };
        }

        static void GLOBAL_APERTURE_CHECK(
            Particle& __restrict__ p ) noexcept {

            double const sign_x = ( ( double )( 0. < p.x ) ) -
                                  ( ( double )( p.x < 0. ) );

            double const sign_y = ( ( double )( 0. < p.y ) ) -
                                  ( ( double )( p.y < 0. ) );

            /* Check whether |p.x| < 1.0 and |p.y| < 1.0 */
            p.state &= ( uint64_type )( ( sign_x * p.x < 1. ) &
                                        ( sign_y * p.y < 1. ) );

            // Effectively only increment if particle is not lost
            p.at_element += p.state;
        }

        uint64_type track( Particle& __restrict__ p,
            uint64_type const slot_index ) const noexcept
        {
            using cl::sycl::sqrt;

            double const one_plus_delta = p.delta + 1.;
            double const lpzi = 1. / sqrt( one_plus_delta * one_plus_delta
                - ( p.px * p.px + p.py * p.py ) );

            p.x += p.px * this->length * lpzi;
            p.y += p.py * this->length * lpzi;
            p.zeta += p.rvv * this->length - one_plus_delta * lpzi;

            /* NOTE: we do not increment p.at_element here -> this is done in
             * GLOBAL_APERTURE_CHECK */

            return slot_index + Drift::NUM_SLOTS();
        }

        double type_id;
        double length;
    };

    struct __attribute__((aligned( 8 ))) Multipole
    {
        static constexpr uint64_type NUM_SLOTS() noexcept
        {
            return uint64_type{ 9 };
        }

        uint64_type track( Particle& __restrict__ p,
            uint64_type const slot_index ) const noexcept
        {
            int64_type const order = ( int64_type )this->order;
            int64_type index_x = 2 * order;
            int64_type index_y = index_x + 1;

            double dPx = this->bal[ index_x ];
            double dPy = this->bal[ index_y ];

            for( index_x -= 2, index_y -= 2 ; index_x >= 0 ;
                 index_x -= 2, index_y -= 2 )
            {
                double const z_real = dPx * p.x - dPy * p.y;
                double const z_imag = dPx * p.y + dPy * p.x;

                dPx = this->bal[ index_x ] + z_real;
                dPy = this->bal[ index_y ] + z_imag;
            }

            dPx *= -p.chi;
            dPy *=  p.chi;

            if( ( this->hxl > 0. ) || ( this->hxl < 0. ) ||
                ( this->hyl > 0. ) || ( this->hyl < 0. ) )
            {
                double const hxl_x = this->hxl * p.x;
                double const hyl_y = this->hyl * p.y;

                p.zeta += p.chi * ( hyl_y - hxl_x );
                dPx += this->hxl + this->hxl * p.delta;
                dPy -= this->hyl + this->hyl * p.delta;

                if( this->length > 0. )
                {
                    dPx -= ( p.chi * this->bal[ 0 ] * hxl_x ) / this->length;
                    dPy += ( p.chi * this->bal[ 1 ] * hyl_y ) / this->length;
                }
            }

            p.px += dPx;
            p.py += dPy;

            // NOTE: The particle should not be here if it is lost!!!
            // But: effectively only increment if particle is not lost
            p.at_element += p.state;

            return slot_index + Multipole::NUM_SLOTS();
        }

        double type_id;
        double order;
        double length;
        double hxl;
        double hyl;
        double bal[ 4 ];
    };

    struct __attribute__((aligned( 8 ))) Cavity
    {
        static constexpr uint64_type NUM_SLOTS() noexcept
        {
            return uint64_type{ 4 };
        }

        uint64_type track( Particle& __restrict__ p,
            uint64_type const slot_index ) const noexcept
        {
            using cl::sycl::sin;

            double const PI = double{ 3.141592653589793 };
            double const C0 = double{ 299792458. };
            double const beta = p.beta0 * p.rvv;
            double const phase = ( PI / 180. ) * this->lag -
                ( ( 2. * PI ) / C0 ) * ( p.zeta / beta ) * this->frequency;

            double const charge = p.charge0 * p.charge_ratio;
            p.add_to_energy( this->voltage * charge * sin( phase ) );

            // NOTE: The particle should not be here if it is lost!!!
            // But: effectively only increment if particle is not lost
            p.at_element += p.state;

            return slot_index + Cavity::NUM_SLOTS();
        }

        double type_id;
        double voltage;
        double frequency;
        double lag;
    };

    struct __attribute__((aligned( 8 ))) EndOfTrack
    {
        static constexpr uint64_type NUM_SLOTS() noexcept
        {
            return uint64_type{ 4 };
        }

        uint64_type track( Particle& __restrict__ p,
            uint64_type const slot_index ) const noexcept
        {
            uint64_t const stored_slot_idx = ( uint64_type )this->next_slot_idx;
            uint64_type const min_next_slot_idex =
                slot_index + EndOfTrack::NUM_SLOTS();

            // NOTE: The particle should not be here if it is lost!!!
            // But: effectively only increment if particle is not lost
            p.at_element += p.state;

            if( ends_turn > 0. )
            {
                p.at_element = ( uint64_type )this->start_turn_at_element;
                ++p.at_turn;
            }

            return ( ( ends_turn > 0 ) || ( stored_slot_idx < slot_index ) ||
                     ( stored_slot_idx >= min_next_slot_idex ) )
                ? stored_slot_idx : min_next_slot_idex;
        }

        double type_id;
        double start_turn_at_element;
        double next_slot_idx;
        double ends_turn;
    };
}

#endif /* DEMOTRACK_SYCL_BEAM_ELEMENTS_H__ */
