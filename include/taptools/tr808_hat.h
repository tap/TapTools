/// @file
/// @brief      Portable TR-808 hi-hat kernel for tap.808.hat~ — no Max/Min dependency.
/// @details    A circuit-informed model of the Roland TR-808 hi-hat: closed and open are ONE
///             circuit — "[the CH] shares the same sound source with the OH" (TR-808 Service
///             Notes p.6) — which is why they live in one kernel with two triggers, and why
///             the choke works: "when the CLOSED HI-HAT is triggered while the OH circuit is
///             activated, Q23 turns on by the voltage applied through R173 — at this moment,
///             the decay time of the OH circuit terminates."
///
///             Sound source: the metal bank's high-range component (metal_bank.h, the ~7100 Hz
///             band-pass of the six squares), gated per-path (Q27/Q30) into its own filter
///             (Q26/Q31) and swing VCA. Decay classes per the Service Notes p.14 chart:
///             CH fixed ~50 ms; OH 90-600 ms across its decay pot (VR3, 2 M). Envelope
///             constants are behavioral fits to that chart; the per-path transistor filters
///             are voiced as a gentle high-pass brightening over the shared band.
///
///             Family contract: trigger_closed(accent) / trigger_open(accent), accent = the
///             trigger-bus voltage; seeded per-unit oscillator spread; deterministic renders.
///
///             §7.2 calibration (2026-07-17), vs the Fischer 1994 set (unit 103852):
///             CH tau 11 -> 16 ms; OH decay dial within +-10% over its whole span with
///             the chart classes kept; per-path corners split (OH ~8.8 kHz, CH ~12 kHz).
///             The unit's closed hat measures far brighter than the shared 7.1 kHz band
///             alone can produce — resolved by the sizzle blend (the bank's raw upper
///             harmonics, ~13 kHz high-passed, weighted per path): centroids now within
///             1.3% (CH, 10.9 kHz measured) and 1.3% (OH, 8.0 kHz), t40s within 10%.
///
///             Plain C++17, stdlib only, per-sample, allocation-free after prepare().
/// @author     Timothy Place
/// @copyright  Copyright 2026 Timothy Place. Distributed under the New BSD License.

#pragma once

#include <algorithm>
#include <cmath>

#include "metal_bank.h"
#include "swing_vca.h"

namespace taptools {
    namespace tr808 {

        // Decay classes: chart fits for the OH span (90/450/600 ms); CH calibrated to the
        // measured t40 (see the §7.2 note).
        constexpr double k_hh_closed_tau_s   = 0.016;
        constexpr double k_hh_open_tau_min_s = 0.020;
        constexpr double k_hh_open_tau_max_s = 0.130;
        constexpr double k_hh_choke_tau_s    = 0.0015; // Q23 terminates the OH decay
        constexpr double k_hh_att_s          = 0.3e-3;

        // Per-path brightening high-pass corner (the Q26/Q31 filters).
        constexpr double k_hh_hp_open_hz   = 8800.0;
        constexpr double k_hh_hp_closed_hz = 12000.0;

        // Sizzle: the bank's upper square harmonics, above what the shared ~7.1 kHz
        // band-pass keeps — blended into each path per its own weight (the per-path
        // Q26/Q31 filters pass different amounts of raw HF). This is what makes the real
        // CH read far brighter than the OH ring. Weights calibrated against the measured
        // CH/OH centroids (§7.2 note).
        constexpr double k_hh_sizzle_hp_hz  = 13000.0;
        constexpr double k_hh_sizzle_closed = 2.2;
        constexpr double k_hh_sizzle_open   = 0.35;

        // Per-path renormalization after the sizzle blend, restoring the family-band
        // full-accent peaks (CH ~0.3, OH ~0.4) the paths had before the blend.
        constexpr double k_hh_trim_closed = 0.15;
        constexpr double k_hh_trim_open   = 0.76;

        constexpr double k_hh_vtrig_min = 4.0;
        constexpr double k_hh_vtrig_max = 14.0;

        constexpr double k_hh_out_scale = 2.6;

        /// The TR-808 hi-hat: one circuit, closed and open paths, hardware choke.
        class hat {
          public:
            void prepare(double sample_rate) {
                m_sr = sample_rate;
                m_bank.prepare(sample_rate);
                m_bp.prepare(sample_rate, k_bank_bp2_hz, k_bank_bp_q);
                m_hp_c.prepare(sample_rate);
                m_hp_o.prepare(sample_rate);
                const double tau_c = 1.0 / (2.0 * k_pi * k_hh_hp_closed_hz);
                const double tau_o = 1.0 / (2.0 * k_pi * k_hh_hp_open_hz);
                m_hp_c.set(tau_c, 0.0, tau_c, 1.0);
                m_hp_o.set(tau_o, 0.0, tau_o, 1.0);
                m_hp_sz.prepare(sample_rate);
                const double tau_sz = 1.0 / (2.0 * k_pi * k_hh_sizzle_hp_hz);
                m_hp_sz.set(tau_sz, 0.0, tau_sz, 1.0);
                m_env_c.prepare(sample_rate);
                m_env_o.prepare(sample_rate);
                m_env_c.set_times(k_hh_att_s, k_hh_closed_tau_s);
                set_decay(m_decay);
                reset();
            }

            void reset() {
                m_bank.reset();
                m_bp.reset();
                m_hp_c.reset();
                m_hp_o.reset();
                m_hp_sz.reset();
                m_env_c.reset();
                m_env_o.reset();
            }

            // -- panel ---------------------------------------------------------------------

            /// Open-hat ring length, 0..1 (VR3, OH DECAY). The closed hat is fixed, like the
            /// hardware.
            void set_decay(double amount) {
                m_decay = std::clamp(amount, 0.0, 1.0);
                if (!m_choked)
                    m_env_o.set_times(k_hh_att_s,
                                      k_hh_open_tau_min_s + m_decay * (k_hh_open_tau_max_s - k_hh_open_tau_min_s));
            }

            /// Closed-hat level, 0..1 (the CH channel's level pot).
            void set_closed_level(double amount) { m_level_c = std::clamp(amount, 0.0, 1.0); }

            /// Open-hat level, 0..1 (the OH channel's level pot).
            void set_open_level(double amount) { m_level_o = std::clamp(amount, 0.0, 1.0); }

            // -- bends ---------------------------------------------------------------------

            void set_tuning(double ratio) { m_bank.set_tuning(ratio); }
            void set_tolerance(double amount) { m_bank.set_tolerance(amount); }
            void set_seed(uint64_t seed) { m_bank.set_seed(seed); }

            // -- performance ---------------------------------------------------------------

            /// Closed hit. Chokes a sounding open hat, as in the hardware (Q23/R173).
            void trigger_closed(double accent = 1.0) {
                const double v = bus(accent);
                m_env_c.trigger(v);
                if (m_env_o.value() > 1e-6) {
                    m_env_o.set_times(k_hh_att_s, k_hh_choke_tau_s);
                    m_choked = true;
                }
            }

            /// Open hit.
            void trigger_open(double accent = 1.0) {
                if (m_choked) {
                    m_choked = false;
                    set_decay(m_decay); // restore the pot's decay
                }
                m_env_o.trigger(bus(accent));
            }

            double process() {
                m_bank.process();
                const double band = m_bp.process(m_bank.sum());

                if (m_choked && m_env_o.value() <= 1e-6) {
                    m_choked = false;
                    set_decay(m_decay);
                }

                const double sz     = m_hp_sz.process(m_bank.sum());
                const double closed = swing_vca(m_hp_c.process(band) + k_hh_sizzle_closed * sz, m_env_c.process())
                                      * k_hh_trim_closed * m_level_c;
                const double open = swing_vca(m_hp_o.process(band) + k_hh_sizzle_open * sz, m_env_o.process())
                                    * k_hh_trim_open * m_level_o;
                return (closed + open) * k_hh_out_scale;
            }

          private:
            static double bus(double accent) {
                const double a = std::clamp(accent, 0.0, 1.0);
                return (k_hh_vtrig_min + a * (k_hh_vtrig_max - k_hh_vtrig_min)) / k_hh_vtrig_max;
            }

            double m_sr{48000.0};

            metal_bank  m_bank;
            bandpass    m_bp;
            first_order m_hp_c, m_hp_o, m_hp_sz;
            decay_env   m_env_c, m_env_o;

            double m_decay{0.5}, m_level_c{1.0}, m_level_o{1.0};
            bool   m_choked{false};
        };

    } // namespace tr808
} // namespace taptools
