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
        LatticeArray const& lattice_buffer,
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
                    dt::beam_element_type >( lattice_buffer[ slot_idx ] );

                switch( type_id )
                {
                    /* all elements are stored with their type_id as the first
                     * data member -> retrieve this number and dispatch
                     * the track method accordingly */

                    case dt::BEAM_ELEMENT_DRIFT: // cf. beam_elements.h
                    {
                        const dt::Drift *const __restrict__ elem =
                            reinterpret_cast< dt::Drift const* >(
                                &lattice_buffer[ slot_idx ] );

                        dt::uint64_type const next_slot_idx =
                            elem->track( p, slot_idx );

                        dt::Drift::GLOBAL_APERTURE_CHECK( p );
                        slot_idx = next_slot_idx;
                        break;
                    }

                    case dt::BEAM_ELEMENT_DRIFT_EXACT: // cf. beam_elements.h
                    {
                        const dt::DriftExact *const __restrict__ elem =
                            reinterpret_cast< dt::DriftExact const* >(
                                &lattice_buffer[ slot_idx ] );

                        dt::uint64_type const next_slot_idx =
                            elem->track( p, slot_idx );

                        // Use GLOBAL_APERTURE_CHECK from Drift -> it's the same
                        dt::Drift::GLOBAL_APERTURE_CHECK( p );
                        slot_idx = next_slot_idx;
                        break;
                    }

                    case dt::BEAM_ELEMENT_MULTIPOLE: // cf. beam_elements.h
                    {
                        const dt::Multipole *const __restrict__ elem =
                            reinterpret_cast< dt::Multipole const* >(
                                &lattice_buffer[ slot_idx ] );

                        dt::uint64_type const next_slot_idx =
                            elem->track( p, slot_idx );

                        slot_idx = next_slot_idx;
                        ++p.at_element;
                        break;
                    }

                    case dt::BEAM_ELEMENT_XY_SHIFT: // cf. beam_elements.h
                    {
                        const dt::XYShift *const __restrict__ elem =
                            ( dt::XYShift const* )&lattice_buffer[ slot_idx ];

                        dt::uint64_type const next_slot_idx =
                            elem->track( p, slot_idx );

                        slot_idx = next_slot_idx;
                        break;
                    }

                    case dt::BEAM_ELEMENT_SROTATION: // cf. beam_elements.h
                    {
                        const dt::SRotation *const __restrict__ elem =
                            ( dt::SRotation const* )&lattice_buffer[ slot_idx ];

                        dt::uint64_type const next_slot_idx =
                            elem->track( p, slot_idx );

                        slot_idx = next_slot_idx;
                        break;
                    }

                    case dt::BEAM_ELEMENT_CAVITY: // cf. beam_elements.h
                    {
                        const dt::Cavity *const __restrict__ elem =
                            reinterpret_cast< dt::Cavity const* >(
                                &lattice_buffer[ slot_idx ] );

                        dt::uint64_type const next_slot_idx =
                            elem->track( p, slot_idx );

                        slot_idx = next_slot_idx;
                        ++p.at_element;
                        break;
                    }

                    case dt::BEAM_ELEMENT_LIMIT_RECT: // cf. beam_elements.h
                    {
                        const dt::LimitRect *const __restrict__ elem =
                            ( dt::LimitRect const* )&lattice_buffer[ slot_idx ];

                        dt::uint64_type const next_slot_idx =
                            elem->track( p, slot_idx );

                        slot_idx = next_slot_idx;
                        break;
                    }

                    case dt::BEAM_ELEMENT_LIMIT_ELLIPSE: // cf. beam_elements.h
                    {
                        const dt::LimitEllipse *const __restrict__ elem =
                            ( dt::LimitEllipse const* )&lattice_buffer[ slot_idx ];

                        dt::uint64_type const next_slot_idx =
                            elem->track( p, slot_idx );

                        slot_idx = next_slot_idx;
                        break;
                    }

                    case dt::BEAM_ELEMENT_LIMIT_RECT_ELLIPSE: // cf. beam_elements.h
                    {
                        const dt::LimitRectEllipse *const __restrict__ elem =
                            ( dt::LimitRectEllipse const* )&lattice_buffer[ slot_idx ];

                        dt::uint64_type const next_slot_idx =
                            elem->track( p, slot_idx );

                        slot_idx = next_slot_idx;
                        break;
                    }

                    case dt::BEAM_ELEMENT_DIPEDGE: // cf. beam_elements.h
                    {
                        const dt::DipoleEdge *const __restrict__ elem =
                            ( dt::DipoleEdge const* )&lattice_buffer[ slot_idx ];

                        dt::uint64_type const next_slot_idx =
                            elem->track( p, slot_idx );

                        slot_idx = next_slot_idx;
                        break;
                    }

                    #if defined( DEMOTRACK_ENABLE_BEAMFIELDS ) && \
                        DEMOTRACK_ENABLE_BEAMFIELDS == 1

                    case dt::BEAM_ELEMENT_SC_COASTING: // cf. beamfields.h
                    {
                        const dt::SpaceChargeCoasting *const __restrict__ elem =
                            reinterpret_cast< dt::SpaceChargeCoasting const* >(
                                &lattice_buffer[ slot_idx ] );

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
