#ifndef DEMOTRACK_SYCL_TRACK_H__
#define DEMOTRACK_SYCL_TRACK_H__

#include "definitions.h"
#include "particle.h"
#include "beam_elements.h"

#if defined( DEMOTRACK_ENABLE_BEAMFIELDS ) && \
    DEMOTRACK_ENABLE_BEAMFIELDS == 1
    #include "beamfields.h"
#endif /* DEMOTRACK_ENABLE_BEAMFIELDS ) */

namespace demotrack
{
    template< class LatticeArray >
    DEMOTRACK_EXTERN void Particle_track_until_turn(
        demotrack::Particle& p,
        LatticeArray const& beam_elements,
        demotrack::uint64_type const max_lattice_buffer_index,
        demotrack::int64_type const until_turn )
    {
        namespace dt = demotrack;
        dt::uint64_type const start_at_element = p.at_element;

        while( ( p.state == 1 ) && ( p.at_turn < until_turn ) )
        {
            dt::uint64_type slot_idx = 0u;

            while( ( p.state == 1 ) && ( slot_idx < max_lattice_buffer_index ) )
            {
                dt::beam_element_type const type_id = static_cast<
                    dt::beam_element_type >( beam_elements[ slot_idx ] );

                switch( type_id )
                {
                    case dt::BEAM_ELEMENT_DRIFT: // cf. beam_elements.h
                    {
                        const dt::Drift *const __restrict__ elem =
                            reinterpret_cast< dt::Drift const* >(
                                &beam_elements[ slot_idx ] );

                        dt::uint64_type const next_slot_idx =
                            elem->track( p, slot_idx );

                        dt::Drift::GLOBAL_APERTURE_CHECK( p );
                        slot_idx = next_slot_idx;
                        break;
                    }

                    case dt::BEAM_ELEMENT_MULTIPOLE: // cf. beam_elements.h
                    {
                        const dt::Multipole *const __restrict__ elem =
                            reinterpret_cast< dt::Multipole const* >(
                                &beam_elements[ slot_idx ] );

                        dt::uint64_type const next_slot_idx =
                            elem->track( p, slot_idx );

                        slot_idx = next_slot_idx;
                        ++p.at_element;
                        break;
                    }

                    case dt::BEAM_ELEMENT_CAVITY: // cf. beam_elements.h
                    {
                        const dt::Cavity *const __restrict__ elem =
                            reinterpret_cast< dt::Cavity const* >(
                                &beam_elements[ slot_idx ] );

                        dt::uint64_type const next_slot_idx =
                            elem->track( p, slot_idx );

                        slot_idx = next_slot_idx;
                        ++p.at_element;
                        break;
                    }

                    #if defined( DEMOTRACK_ENABLE_BEAMFIELDS ) && \
                        DEMOTRACK_ENABLE_BEAMFIELDS == 1

                    case dt::BEAM_ELEMENT_SC_COASTING: // cf. beamfields.h
                    {
                        const dt::SpaceChargeCoasting *const __restrict__ elem =
                            reinterpret_cast< dt::SpaceChargeCoasting const* >(
                                &beam_elements[ slot_idx ] );

                        dt::uint64_type const next_slot_idx =
                            elem->track( p, slot_idx );

                        ++p.at_element;
                        slot_idx = next_slot_idx;
                        break;
                    }

                    #endif /* beamfields enabled */

                    default:
                    {
                        /* unknown beam element -> loose particle and quit */
                        p.state = 0;
                        slot_idx = max_lattice_buffer_index;
                    }
                };

            }

            if( p.state == 1 )
            {
                p.at_element = start_at_element;
                ++p.at_turn;
            }
        }
    }
}

#endif /* DEMOTRACK_SYCL_TRACK_H__ */
