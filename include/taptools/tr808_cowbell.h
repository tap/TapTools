/// @file
/// @brief      Portable TR-808 cowbell kernel for tap.808.cowbell~ — no Max/Min dependency.
/// @details    A circuit-informed model of the Roland TR-808 cowbell, after the TR-808 Service
///             Notes (p.6: "uses the outputs of two square waveform oscillators with different
///             frequencies ... each oscillation output passes the corresponding exclusive gate
///             (VCA, Q14, Q15) and mixed by the filter IC2. A series of R82 and C34 connected
///             in parallel with C9 forms an envelope having abrupt level decay at the initial
///             trailing edge to emphasize attack effect") and the voicing-board schematic:
///
///             - The two oscillators are the metal bank's trimpot-tuned pair — 540 Hz (TM1,
///               Roland's "1.85 ms") and 800 Hz (TM2, "1.25 ms").
///             - The two-slope envelope: C9 (0.47 uF) gives a fast initial drop, C34 (1 uF)
///               through R82 (33k) the tail (33 ms nominal; calibrated to the measured 88 ms
///               — see the §7.2 note) — modeled as two summed RC decays. The chart's ~50 ms
///               class is the audible clank; the -40 dB tail measures ~320 ms.
///             - The IC2 voicing filter: a band-pass centered ~860 Hz, derived from the
///               schematic's multiple-feedback values (R26 10k in, R25 470k feedback,
///               C30 0.0022 uF / C29 0.0033 uF): fc = 1/(2*pi*sqrt(R26*R25*C30*C29)). Its Q
///               here is a fit to the voice's known clank (the exact section is third-order).
///
///             Family contract: trigger(accent); seeded per-unit oscillator spread
///             (`tolerance`), tuning bend, deterministic renders.
///
///             §7.2 calibration (2026-07-17), vs the Fischer 1994 set (unit 103852):
///             R82/C34 tail tau 33 -> 88 ms — the chart's ~50 ms class is the clank, the
///             real tail measures 322 ms to -40 dB (ours 324). Pitch -2.9% (800 Hz
///             nominal pair vs 824 measured — inside the trimpot/tolerance spread).
///
///             Plain C++17, stdlib only, per-sample, allocation-free after prepare().
/// @author     Timothy Place
// SPDX-License-Identifier: MIT
// Copyright 2026 Timothy Place.

#pragma once

#include <algorithm>
#include <cmath>

#include "metal_bank.h"
#include "numeric.h"
#include "swing_vca.h"

namespace tap::tools {
    namespace tr808 {

        // Two-slope envelope (see header): the fast C9 component and the R82/C34 tail.
        constexpr double k_cb_fast_tau_s = 0.0045;
        constexpr double k_cb_tail_tau_s = 0.088;
        constexpr double k_cb_tail_level = 0.35; // tail level relative to the initial peak
        constexpr double k_cb_att_s      = 0.2e-3;

        // IC2 voicing band-pass, from the schematic (see header).
        constexpr double k_cb_bp_hz = 860.0;
        constexpr double k_cb_bp_q  = 1.2;

        constexpr double k_cb_vtrig_min = 4.0;
        constexpr double k_cb_vtrig_max = 14.0;

        constexpr double k_cb_out_scale = 0.86;

        /// The TR-808 cowbell voice.
        template <typename Sample>
        class basic_cowbell {
            static_assert(is_sample_profile<Sample>,
                          "basic_cowbell supports the two Tap numeric profiles: float and double");

          public:
            using sample_type = Sample;

            void prepare(Sample sample_rate) {
                m_sr = sample_rate;
                m_bank.prepare(sample_rate);
                m_bp.prepare(sample_rate, Sample(k_cb_bp_hz), Sample(k_cb_bp_q));
                m_env_fast.prepare(sample_rate);
                m_env_tail.prepare(sample_rate);
                m_env_fast.set_times(Sample(k_cb_att_s), Sample(k_cb_fast_tau_s));
                m_env_tail.set_times(Sample(k_cb_att_s), Sample(k_cb_tail_tau_s));
                reset();
            }

            void reset() {
                m_bank.reset();
                m_bp.reset();
                m_env_fast.reset();
                m_env_tail.reset();
            }

            /// Output level, 0..1 (VR5, CB LEVEL).
            void set_level(Sample amount) { m_level = std::clamp(amount, Sample(0.0), Sample(1.0)); }

            /// Swing-VCA drive on the oscillator pair (0 = calibrated linear model, bit-identical;
            /// > 0 engages the swing VCA's symmetric harmonic saturation before the bandpass — the
            /// experimental fidelity hook for the tonal-voice sweep, plans/tap.808.md).
            void   set_drive(Sample amount) { m_drive = std::max(Sample(0.0), amount); }
            Sample drive() const { return m_drive; }

            void set_tuning(Sample ratio) { m_bank.set_tuning(ratio); }
            void set_tolerance(Sample amount) { m_bank.set_tolerance(amount); }
            void set_seed(uint64_t seed) { m_bank.set_seed(seed); }

            void trigger(Sample accent = Sample(1.0)) {
                const Sample a = std::clamp(accent, Sample(0.0), Sample(1.0));
                const Sample v = (Sample(k_cb_vtrig_min) + a * (Sample(k_cb_vtrig_max) - Sample(k_cb_vtrig_min)))
                                 / Sample(k_cb_vtrig_max);
                m_env_fast.trigger(v);
                m_env_tail.trigger(v * Sample(k_cb_tail_level));
            }

            Sample process() {
                m_bank.process();
                // The gates pass only the trimpot pair (oscillators #5 and #6: 800 / 540 Hz).
                const Sample pair = Sample(0.5) * (m_bank.osc(4) + m_bank.osc(5));
                const Sample env  = m_env_fast.process() + m_env_tail.process();
                return m_bp.process(swing_vca(pair, env, m_drive)) * m_level * Sample(k_cb_out_scale);
            }

          private:
            Sample m_sr{Sample(48000.0)};

            basic_metal_bank<Sample> m_bank;
            basic_bandpass<Sample>   m_bp;
            basic_decay_env<Sample>  m_env_fast, m_env_tail;

            Sample m_level{Sample(1.0)};
            Sample m_drive{Sample(0.0)}; // swing-VCA saturation on the osc pair; 0 = linear (default)
        };

        /// The double profile — the golden model.
        using cowbell = basic_cowbell<double>;

        /// The float profile — for single-precision targets. See numeric.h.
        using cowbell32 = basic_cowbell<float>;

    } // namespace tr808
} // namespace tap::tools
