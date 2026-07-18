/// @file
/// @brief      Portable TR-808 cymbal kernel for tap.808.cymbal~ — no Max/Min dependency.
/// @details    A circuit-informed model of the Roland TR-808 cymbal voice, after the TR-808
///             Service Notes (p.6: "the combined square wave outputs of six Schmitt triggers
///             ... separated into high and low range components by two filters ... the output
///             of the gate Q16 has the highest frequency component ... its decay time is
///             short. The output of Q17 is in a frequency range slightly lower ... its decay
///             time is controllable. These three signals ... outputted with their level ratio
///             controlled by VR4 [CY TONE]") and Werner, Abel & Smith's cymbal paper
///             (ICMC|SMC 2014): the metal bank (metal_bank.h), band-passed at ~3440 and
///             ~7100 Hz, split into three components with separate swing-VCA envelopes:
///
///             - "highest" — the 7100 Hz band with a short fixed decay (the strike);
///             - "mid"     — the same high range, slightly lower voicing, with the decay pot
///                           (VR2, 2 M) setting its release — the ring the player controls;
///             - "low"     — the 3440 Hz band with a fixed medium decay (the body/sizzle).
///
///             The tone pot (VR4) sets the level ratio of the strike against the body. Decay
///             constants are behavioral fits to the Service Notes p.14 chart (overall decay
///             ~350 / ~800 / ~1200 ms at short/mid/long); the exact three-EG diode network
///             (D5-D8, C36-C41) is simplified to per-band RC decays — flagged for the
///             circuit-simulation phase along with the swing-VCA nonlinearity.
///
///             Family contract: trigger(accent) with accent = the 7-14 V cymbal trigger bus
///             (the paper's §5; its attack smoother's ~0.1 ms lag informs the envelope attack
///             constant here); seeded per-unit oscillator
///             spread (`tolerance`), tuning bend, deterministic renders.
///
///             §7.2 calibration (2026-07-17), vs the Fischer 1994 set (unit 103852): the
///             chart's 350-1200 ms are knob classes, not tails — the real -40 dB range
///             measures ~0.65 s (pot min) to ~2.7 s (max). Calibrated: fixed-band taus
///             0.09/0.15 -> 0.15/0.15 s, the decay pot's mid-band span 0.06-0.30 ->
///             0.13-0.60 s (t40 now within ~+-15% across the tone x decay grid), the
///             ~9 kHz high-band pre-emphasis, and the 0.45 body weight (centroids from
///             -35..-53% to mostly within +-15%; tone-max residual ~-15%: the unit runs
///             brighter than the shared-band model).
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

        // Per-band decay time constants (behavioral fits to the p.14 chart; see header).
        constexpr double k_cy_hi_tau_s      = 0.15; // "highest" strike band, fixed
        constexpr double k_cy_mid_tau_min_s = 0.13; // VR2 fully counterclockwise
        constexpr double k_cy_mid_tau_max_s = 0.60; // VR2 fully clockwise
        constexpr double k_cy_lo_tau_s      = 0.15; // body band, fixed

        // High-band pre-emphasis: the BP's lower skirt passes the oscillators' hot low
        // fundamentals, darkening the band well below its nominal center.
        constexpr double k_cy_hi_hp_hz = 9000.0;
        constexpr double k_cy_att_s    = 0.4e-3;

        // Trigger bus: the cymbal's is narrowed to 7-14 V (paper §5).
        constexpr double k_cy_vtrig_min = 7.0;
        constexpr double k_cy_vtrig_max = 14.0;

        constexpr double k_cy_out_scale = 3.7; // full-accent default-knob peak just under +-1

        /// The TR-808 cymbal voice.
        class cymbal {
          public:
            void prepare(double sample_rate) {
                m_sr = sample_rate;
                m_bank.prepare(sample_rate);
                m_bp_hi.prepare(sample_rate, k_bank_bp2_hz, k_bank_bp_q);
                m_bp_lo.prepare(sample_rate, k_bank_bp1_hz, k_bank_bp_q);
                m_hi_hp.prepare(sample_rate);
                const double tau_hp = 1.0 / (2.0 * k_pi * k_cy_hi_hp_hz);
                m_hi_hp.set(tau_hp, 0.0, tau_hp, 1.0);
                m_env_hi.prepare(sample_rate);
                m_env_mid.prepare(sample_rate);
                m_env_lo.prepare(sample_rate);
                m_env_hi.set_times(k_cy_att_s, k_cy_hi_tau_s);
                m_env_lo.set_times(k_cy_att_s, k_cy_lo_tau_s);
                set_decay(m_decay);
                reset();
            }

            void reset() {
                m_bank.reset();
                m_bp_hi.reset();
                m_bp_lo.reset();
                m_hi_hp.reset();
                m_env_hi.reset();
                m_env_mid.reset();
                m_env_lo.reset();
                m_vtrig = 0.0;
            }

            // -- panel ---------------------------------------------------------------------

            /// Strike/body balance, 0..1 (VR4, CY TONE): 0 = body only, 1 = strike only.
            void set_tone(double amount) { m_tone = std::clamp(amount, 0.0, 1.0); }

            /// Ring length, 0..1 (VR2, CY DECAY).
            void set_decay(double amount) {
                m_decay = std::clamp(amount, 0.0, 1.0);
                m_env_mid.set_times(k_cy_att_s,
                                    k_cy_mid_tau_min_s + m_decay * (k_cy_mid_tau_max_s - k_cy_mid_tau_min_s));
            }

            /// Output level, 0..1.
            void set_level(double amount) { m_level = std::clamp(amount, 0.0, 1.0); }

            // -- bends ---------------------------------------------------------------------

            void set_tuning(double ratio) { m_bank.set_tuning(ratio); }
            void set_tolerance(double amount) { m_bank.set_tolerance(amount); }
            void set_seed(uint64_t seed) { m_bank.set_seed(seed); }

            // -- performance ---------------------------------------------------------------

            void trigger(double accent = 1.0) {
                const double a = std::clamp(accent, 0.0, 1.0);
                m_vtrig        = (k_cy_vtrig_min + a * (k_cy_vtrig_max - k_cy_vtrig_min)) / k_cy_vtrig_max;
                m_env_hi.trigger(m_vtrig);
                m_env_mid.trigger(m_vtrig);
                m_env_lo.trigger(m_vtrig * 0.8);
            }

            double process() {
                m_bank.process();
                const double s  = m_bank.sum();
                const double hi = m_hi_hp.process(m_bp_hi.process(s));
                const double lo = m_bp_lo.process(s);

                const double strike = swing_vca(hi, m_env_hi.process());
                const double ring   = swing_vca(hi, m_env_mid.process());
                const double body   = swing_vca(lo, m_env_lo.process());

                const double mix = m_tone * (strike + 0.6 * ring) + (1.0 - m_tone) * (0.45 * body + ring);
                return mix * m_level * k_cy_out_scale;
            }

          private:
            double m_sr{48000.0};

            metal_bank  m_bank;
            bandpass    m_bp_hi, m_bp_lo;
            first_order m_hi_hp;
            decay_env   m_env_hi, m_env_mid, m_env_lo;

            double m_tone{0.5}, m_decay{0.5}, m_level{1.0};
            double m_vtrig{0.0};
        };

    } // namespace tr808
} // namespace taptools
