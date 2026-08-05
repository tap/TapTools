/// @file
/// @brief      Portable virtual-analog ADSR envelope kernel — no Max/Min dependency.
/// @details    The envelope behind tap.adsr~, rebuilt as a circuit model. The default `analog`
///             mode is the classic analog EG shape from the published sources (the CEM 3310
///             datasheet's architecture; standard Electronotes ADSR practice): the attack stage
///             is an RC charge toward an asymptote *above* full scale — the 3310 charges toward
///             its +7 V rail and a comparator ends the stage at the +5 V peak, a 1.4× overshoot
///             target, which is where the perceived punch of an analog attack lives — and the
///             decay and release stages are true RC discharges that approach their targets
///             asymptotically instead of stopping dead. Retrigger always rises from the current
///             level, like the capacitor it models.
///
///             The three curves of the 2003 Jamoma TTAdsr are preserved verbatim as the
///             `hybrid` / `linear` / `exponential` compatibility modes (straight lines in
///             amplitude or in dB, hard stops, the −120 dB floor) — a faithful port, magic
///             constants and all.
///
///             Trigger contract (the family's, at last): `process(gate)` opens above a
///             `threshold` (default 0.005 — above the trigger bus's 1e-3 edge floor, below the
///             sequencer's 0.01 plain level, and far below the old hard-coded 0.5), and the
///             gate's amplitude is velocity: with `velocity` sensitivity s, peak and sustain
///             scale by 1 + s·(amp − 1), so the 303 convention (1.0 plain / 2.0 accented) and
///             the 808 rows' amplitudes land meaningfully. s = 0 (the default) is exactly the
///             legacy amplitude-blind behavior. The captured amplitude is the maximum gate
///             level seen during the attack stage.
///
///             Contract numbers, pinned by the test battery: the analog attack reaches full
///             scale at exactly the attack time (the truncated-charge law τ = t_a / ln(T/(T−1)),
///             T = 1.4, so the midpoint sits near 0.65, not 0.5); analog decay and release
///             close 95 % of their gap at the knob time (τ = t/3) and keep closing
///             asymptotically. Honest limit: an asymptotic release never *reaches* zero, so the
///             stage ends (exactly zero, state inactive) once the level falls below 1e-6.
/// @author     Timothy Place
// SPDX-License-Identifier: MIT
// Copyright 2003-2026 Timothy Place.

#pragma once

#include <algorithm>
#include <cmath>

namespace tap::tools {
    namespace adsr {

        constexpr double k_attack_target     = 1.4;    // CEM 3310: +7 V asymptote, +5 V comparator peak
        constexpr double k_noise_floor_db    = -120.0; // Jamoma TTAdsr's dB basement (legacy modes)
        constexpr double k_default_threshold = 0.005;  // above the 1e-3 trigger floor, below seq plain 0.01
        constexpr double k_min_time_ms       = 1.0;    // legacy clamp, kept
        constexpr double k_max_time_ms       = 60000.0;
        constexpr double k_end_level         = 1e-6; // release ends (exact zero) below this
        constexpr double k_settle_fraction   = 0.95; // decay/release close this much at the knob time

        enum class mode : int { analog = 0, hybrid, linear, exponential };

        /// Virtual-analog ADSR generator. House shape: prepare(sr), per-sample process(gate),
        /// allocation-free setters safe while audio runs. Output is 0..1 at unity velocity
        /// (up to 2 with full velocity sensitivity and an accented 2.0 gate).
        class generator {
          public:
            void prepare(double sr) {
                m_sr = sr;
                update_coeffs();
            }

            bool prepared() const { return m_sr > 0.0; }

            void set_attack_ms(double ms) {
                m_attack_ms = std::clamp(ms, k_min_time_ms, k_max_time_ms);
                update_coeffs();
            }
            void set_decay_ms(double ms) {
                m_decay_ms = std::clamp(ms, k_min_time_ms, k_max_time_ms);
                update_coeffs();
            }
            /// Sustain level in decibels (unclamped, the legacy contract).
            void set_sustain_db(double db) {
                m_sustain_db  = db;
                m_sustain_amp = std::pow(10.0, db * 0.05);
            }
            void set_release_ms(double ms) {
                m_release_ms = std::clamp(ms, k_min_time_ms, k_max_time_ms);
                update_coeffs();
            }
            void set_mode(mode m) { m_mode = m; }
            void set_mode(int m) { m_mode = static_cast<mode>(std::clamp(m, 0, 3)); }
            /// Gate-open threshold (0..1). The default hears the sequencer's plain level.
            void set_threshold(double t) { m_threshold = std::clamp(t, 0.0, 1.0); }
            /// Velocity sensitivity, 0..1: peak and sustain scale by 1 + s·(gate amp − 1).
            void set_velocity(double s) { m_velocity = std::clamp(s, 0.0, 1.0); }

            double attack_ms() const { return m_attack_ms; }
            double decay_ms() const { return m_decay_ms; }
            double sustain_db() const { return m_sustain_db; }
            double release_ms() const { return m_release_ms; }
            mode   curve() const { return m_mode; }
            double threshold() const { return m_threshold; }
            double velocity() const { return m_velocity; }
            bool   active() const { return m_state != state::inactive; }

            /// Reset to silence and idle; parameters keep their values.
            void clear() {
                m_state     = state::inactive;
                m_output    = 0.0;
                m_output_db = k_noise_floor_db;
                m_amp       = 1.0;
            }

            /// Consume one gate sample (amplitude-as-velocity above `threshold`); produce the
            /// envelope sample. Returns 0 before prepare().
            double process(double gate) {
                if (!prepared()) {
                    return 0.0;
                }

                const bool open = gate > m_threshold;
                if (open) {
                    if (m_state == state::inactive || m_state == state::release) {
                        m_state = state::attack;
                        m_amp   = std::max(0.0, gate);
                    }
                    else if (m_state == state::attack) {
                        m_amp = std::max(m_amp, gate); // velocity = max gate level during attack
                    }
                }
                else {
                    if (m_state != state::inactive && m_state != state::release) {
                        m_state = state::release;
                    }
                }

                switch (m_mode) {
                case mode::analog:
                    process_analog();
                    break;
                case mode::linear:
                    process_linear();
                    break;
                case mode::exponential:
                    process_exponential();
                    break;
                case mode::hybrid:
                default:
                    process_hybrid();
                    break;
                }
                return m_output;
            }

          private:
            enum class state : int { inactive = 0, attack, decay, sustain, release };

            // ---- the analog model --------------------------------------------------------

            /// Peak scale for the current gate amplitude: 1 + s·(amp − 1), floored at 0.
            double scale() const { return std::max(0.0, 1.0 + m_velocity * (m_amp - 1.0)); }

            void process_analog() {
                const double sc = scale();
                switch (m_state) {
                case state::attack:
                    // RC charge toward the overshoot target, truncated at the peak.
                    m_output += m_attack_coeff * (k_attack_target * sc - m_output);
                    if (m_output >= sc) {
                        m_output = sc;
                        m_state  = state::decay;
                    }
                    break;
                case state::decay:
                case state::sustain:
                    // True RC toward sustain — asymptotic, never a hard stop.
                    m_output += m_decay_coeff * (m_sustain_amp * sc - m_output);
                    break;
                case state::release:
                    m_output += m_release_coeff * (0.0 - m_output);
                    if (m_output < k_end_level) {
                        m_output = 0.0;
                        m_state  = state::inactive;
                    }
                    break;
                case state::inactive:
                default:
                    break;
                }
            }

            // ---- the Jamoma TTAdsr curves, ported faithfully -----------------------------

            static double db_to_amp(double db) { return std::pow(10.0, db * 0.05); }
            static double amp_to_db(double amp) { return (amp <= 0.0) ? k_noise_floor_db : 20.0 * std::log10(amp); }

            void process_linear() {
                switch (m_state) {
                case state::attack:
                    m_output += m_attack_step;
                    if (m_output >= 1.0) {
                        m_output = 1.0;
                        m_state  = state::decay;
                    }
                    break;
                case state::decay:
                    m_output -= m_decay_step;
                    if (m_output <= m_sustain_amp) {
                        m_state  = state::sustain;
                        m_output = m_sustain_amp;
                    }
                    break;
                case state::sustain:
                    break;
                case state::release:
                    m_output -= m_release_step;
                    if (m_output <= 0.0) {
                        m_state  = state::inactive;
                        m_output = 0.0;
                    }
                    break;
                case state::inactive:
                default:
                    break;
                }
            }

            void process_exponential() {
                switch (m_state) {
                case state::attack:
                    m_output_db += m_attack_step_db;
                    if (m_output_db >= 0.0) {
                        m_state  = state::decay;
                        m_output = 1.0;
                    }
                    else {
                        m_output = db_to_amp(m_output_db);
                    }
                    break;
                case state::decay:
                    m_output_db -= m_decay_step_db;
                    m_output = db_to_amp(m_output_db);
                    if (m_output <= m_sustain_amp) {
                        m_state  = state::sustain;
                        m_output = m_sustain_amp;
                    }
                    break;
                case state::sustain:
                    break;
                case state::release:
                    m_output_db -= m_release_step_db;
                    if (m_output_db <= k_noise_floor_db) {
                        m_state  = state::inactive;
                        m_output = 0.0;
                    }
                    else {
                        m_output = db_to_amp(m_output_db);
                    }
                    break;
                case state::inactive:
                default:
                    break;
                }
            }

            void process_hybrid() {
                switch (m_state) {
                case state::attack:
                    m_output += m_attack_step;
                    if (m_output >= 1.0) {
                        m_output    = 1.0;
                        m_output_db = 0.0;
                        m_state     = state::decay;
                    }
                    else {
                        m_output_db = amp_to_db(m_output);
                    }
                    break;
                case state::decay:
                    m_output_db -= m_decay_step_db;
                    m_output = db_to_amp(m_output_db);
                    if (m_output <= m_sustain_amp) {
                        m_state  = state::sustain;
                        m_output = m_sustain_amp;
                    }
                    break;
                case state::sustain:
                    break;
                case state::release:
                    m_output_db -= m_release_step_db;
                    if (m_output_db <= k_noise_floor_db) {
                        m_state  = state::inactive;
                        m_output = 0.0;
                    }
                    else {
                        m_output = db_to_amp(m_output_db);
                    }
                    break;
                case state::inactive:
                default:
                    break;
                }
            }

            // ---- geometry ----------------------------------------------------------------

            void update_coeffs() {
                if (!prepared()) {
                    return;
                }
                const double attack_samples  = std::max(1.0, m_attack_ms * 0.001 * m_sr);
                const double decay_samples   = std::max(1.0, m_decay_ms * 0.001 * m_sr);
                const double release_samples = std::max(1.0, m_release_ms * 0.001 * m_sr);

                // Analog: attack knob = time to reach the peak on the truncated charge
                // (tau = t / ln(T / (T - 1))); decay/release knobs = time to close 95 %
                // of the gap (tau = t / 3).
                const double attack_tau  = attack_samples / std::log(k_attack_target / (k_attack_target - 1.0));
                const double settle_logs = -std::log(1.0 - k_settle_fraction); // 3 for 95 %
                m_attack_coeff           = 1.0 - std::exp(-1.0 / attack_tau);
                m_decay_coeff            = 1.0 - std::exp(-settle_logs / decay_samples);
                m_release_coeff          = 1.0 - std::exp(-settle_logs / release_samples);

                // Legacy: straight-line steps, exactly the TTAdsr math.
                m_attack_step     = 1.0 / attack_samples;
                m_decay_step      = 1.0 / decay_samples;
                m_release_step    = 1.0 / release_samples;
                m_attack_step_db  = -(k_noise_floor_db / attack_samples);
                m_decay_step_db   = -(k_noise_floor_db / decay_samples);
                m_release_step_db = -(k_noise_floor_db / release_samples);
            }

            double m_sr = 0.0;

            double m_attack_ms   = 50.0;
            double m_decay_ms    = 100.0;
            double m_sustain_db  = -6.0;
            double m_sustain_amp = 0.5011872336272722; // -6 dB, the legacy default
            double m_release_ms  = 500.0;
            mode   m_mode        = mode::analog;
            double m_threshold   = k_default_threshold;
            double m_velocity    = 0.0;

            double m_attack_coeff  = 0.0;
            double m_decay_coeff   = 0.0;
            double m_release_coeff = 0.0;

            double m_attack_step     = 0.0;
            double m_decay_step      = 0.0;
            double m_release_step    = 0.0;
            double m_attack_step_db  = 0.0;
            double m_decay_step_db   = 0.0;
            double m_release_step_db = 0.0;

            double m_output    = 0.0;
            double m_output_db = k_noise_floor_db;
            double m_amp       = 1.0;
            state  m_state     = state::inactive;
        };

    } // namespace adsr
} // namespace tap::tools
