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
/// @copyright  Copyright 2026 Timothy Place. Distributed under the New BSD License.

#pragma once

#include <algorithm>
#include <cmath>

#include "metal_bank.h"
#include "swing_vca.h"

namespace taptools {
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
        class cowbell {
          public:
            void prepare(double sample_rate) {
                m_sr = sample_rate;
                m_bank.prepare(sample_rate);
                m_bp.prepare(sample_rate, k_cb_bp_hz, k_cb_bp_q);
                m_env_fast.prepare(sample_rate);
                m_env_tail.prepare(sample_rate);
                m_env_fast.set_times(k_cb_att_s, k_cb_fast_tau_s);
                m_env_tail.set_times(k_cb_att_s, k_cb_tail_tau_s);
                reset();
            }

            void reset() {
                m_bank.reset();
                m_bp.reset();
                m_env_fast.reset();
                m_env_tail.reset();
            }

            /// Output level, 0..1 (VR5, CB LEVEL).
            void set_level(double amount) { m_level = std::clamp(amount, 0.0, 1.0); }

            void set_tuning(double ratio) { m_bank.set_tuning(ratio); }
            void set_tolerance(double amount) { m_bank.set_tolerance(amount); }
            void set_seed(uint64_t seed) { m_bank.set_seed(seed); }

            void trigger(double accent = 1.0) {
                const double a = std::clamp(accent, 0.0, 1.0);
                const double v = (k_cb_vtrig_min + a * (k_cb_vtrig_max - k_cb_vtrig_min)) / k_cb_vtrig_max;
                m_env_fast.trigger(v);
                m_env_tail.trigger(v * k_cb_tail_level);
            }

            double process() {
                m_bank.process();
                // The gates pass only the trimpot pair (oscillators #5 and #6: 800 / 540 Hz).
                const double pair = 0.5 * (m_bank.osc(4) + m_bank.osc(5));
                const double env  = m_env_fast.process() + m_env_tail.process();
                return m_bp.process(swing_vca(pair, env)) * m_level * k_cb_out_scale;
            }

          private:
            double m_sr{48000.0};

            metal_bank m_bank;
            bandpass   m_bp;
            decay_env  m_env_fast, m_env_tail;

            double m_level{1.0};
        };

    } // namespace tr808
} // namespace taptools
