/// @file
/// @brief      Portable TR-808 snare drum kernel for tap.808.snare~ — no Max/Min dependency.
/// @details    A circuit-informed model of the Roland TR-808 snare drum voice, from the TR-808
///             Service Notes (first edition, June 15 1981): circuit description p.6 ("two
///             bridged T-networks for fundamental waveforms and harmonic waveforms; the output
///             ratio of the two can be changed by VR8 [SD TONE]; the amplitude of snappy
///             envelope can be controlled by VR9 [SNAPPY]"), schematic p.9 (component values,
///             reference designators below), and the p.14 voice chart (SD: 3 Vpp normal /
///             10 Vpp accent, ~60 ms decay).
///
///             Structure, mirrored block for block:
///             - Trigger (Q45/Q46) -> pulse shaping (R189/C57/R190) -> the non-inverting inputs
///               of two bridged-T resonators (IC14a/b, the same op-amp topology as the bass
///               drum's — reused from bridged_t.h, without the BD's regeneration loop):
///                 fundamental: bridge R197 820k, arms C58/C59, leg R196 680R  (natural Q ~16)
///                 harmonic:    bridge R198 1M,   arms C60/C61, leg R195 2.2k  (natural Q ~10,
///                              rings shorter — like a real drum's upper mode)
///             - Snappy: the shared white noise -> ~4 kHz high-pass (C66/C67 0.0018 uF with
///               R201 22k / R202 47k around Q48/Q49) -> swing VCA driven by an RC decay
///               envelope (C51 0.47 uF into R184+R185 30k -> tau ~14 ms nominal; calibrated
///               to 34 ms measured — see the §7.2 note), depth set by VR9.
///             - Mix: VR8 crossfades the two resonators; the snappy path sums after it; VR7 is
///               the output level (IC13 mixer).
///
///             Tuning provenance: this manual's schematic gives C58 = C59 = 0.027 uF and
///             C60 = C61 = 0.0068 uF (~249.6 Hz / ~499 Hz — matching its own p.14 chart, ~238 /
///             ~476 measured). Roland later revised C58 -> 0.056 uF and C61 -> 0.015 uF,
///             retuning the pair to ~173.3 / ~336 Hz — the classic 808 snare (documented in
///             K. J. Werner's TR-808 SD analysis). STOCK HERE IS THE LATER REVISION; the
///             `tuning` bend scales both resonators from there.
///
///             Behavioral simplifications: the hardware VCA's harmonic generation is linear by
///             default, with an opt-in `drive` for the swing VCA's symmetric saturation (see
///             swing_vca.h); the slight re-excitation of the resonators by the snappy envelope
///             (the composite-trigger path through Q47) is omitted, as in Werner's model; accent
///             maps to the common trigger bus voltage as in the kick.
///
///             §7.2 calibration (2026-07-17), vs the Fischer 1994 set (unit 103852):
///             fundamentals within 1.2% at every dial position, including the tone-max
///             flip to the ~336 Hz mode. Calibrated: the snappy band-limit (two ~4.7 kHz
///             low-pass sections after the schematic high-pass — the HP alone left the
///             noise white to Nyquist, centroid ~15 kHz vs the measured ~5 kHz), the
///             snappy envelope (tau 14 -> 34 ms: measured t40 ~155 ms), and the VR9 law
///             (gain 42 with a ^1.5 dial taper, fit to the measured noise-power fraction
///             across the dial). Residuals, documented: noise centroid +12-20%; the
///             harmonic-only ring (tone max, snappy 0) decays ~2x faster than unit
///             103852 (schematic values kept).
///
///             Plain C++17, stdlib only, per-sample, allocation-free after prepare().
/// @author     Timothy Place
/// @copyright  Copyright 2026 Timothy Place. Distributed under the New BSD License.

#pragma once

#include <algorithm>
#include <cmath>

#include "bridged_t.h"
#include "swing_vca.h"

namespace tap::tools {
    namespace tr808 {

        // Service Notes schematic values (snare drum voice). Later-revision arm capacitors —
        // see the tuning-provenance note in the header comment.
        constexpr double k_sd_r196 = 680.0;  // fundamental bridged-T leg
        constexpr double k_sd_r197 = 820e3;  // fundamental bridged-T bridge
        constexpr double k_sd_c58  = 56e-9;  // fundamental arm (first edition: 0.027 uF)
        constexpr double k_sd_c59  = 27e-9;  // fundamental arm
        constexpr double k_sd_r195 = 2.2e3;  // harmonic bridged-T leg
        constexpr double k_sd_r198 = 1e6;    // harmonic bridged-T bridge
        constexpr double k_sd_c60  = 6.8e-9; // harmonic arm
        constexpr double k_sd_c61  = 15e-9;  // harmonic arm (first edition: 0.0068 uF)

        // Trigger network into the resonators (R189 100k bypassed by C57 0.0068 uF, R190 680R
        // shunt): a high-shelf divider — DC gain R190/(R189+R190) ~= 0.0068, HF gain ~1, zero
        // at 1/(2*pi*R189*C57) ~= 234 Hz. The resonators are excited mostly by the pulse edges.
        constexpr double k_sd_r189 = 100e3;
        constexpr double k_sd_r190 = 680.0;
        constexpr double k_sd_c57  = 6.8e-9;

        // Snappy noise path: ~4 kHz second-order high-pass (C66/C67 0.0018 uF with R201 22k /
        // R202 47k), then the calibrated band-limit low-pass pair; envelope tau nominal
        // C51 * (R184 + R185) ~= 14 ms, calibrated to the measured 34 ms (§7.2 note).
        constexpr double k_sd_snappy_hp_hz = 4000.0;
        constexpr double k_sd_snappy_lp_hz = 4700.0;
        constexpr double k_sd_snappy_tau_s = 34e-3;
        constexpr double k_sd_snappy_att_s = 0.2e-3;

        // Trigger bus (family convention, as the kick): accent rides the pulse voltage.
        constexpr double k_sd_vtrig_min = 4.0;
        constexpr double k_sd_vtrig_max = 14.0;
        constexpr double k_sd_pulse_ms  = 1.0;

        // Volts -> sample units, set so a full-accent, default-knob hit peaks just under +-1.
        constexpr double k_sd_out_scale = 1.0 / 17.0;
        // Snappy path gain at snappy = 1 in the mixer's volt units (VR9 span; set against the
        // p.14 chart balance and reference recordings). The envelope already rides the accent.
        constexpr double k_sd_snappy_gain = 42.0;
        // VR9 dial taper (calibration fit): the hardware's snappy pot moves most of its
        // range in the upper half of the dial.
        constexpr double k_sd_snappy_taper = 1.5;

        /// The TR-808 snare drum voice. prepare(), set the panel, trigger(accent), process().
        class snare {
          public:
            void prepare(double sample_rate) {
                m_sr = sample_rate;

                bridged_t::config f;
                f.c_arm_in   = k_sd_c58;
                f.c_arm_out  = k_sd_c59;
                f.r_bridge   = k_sd_r197;
                f.r_inject_a = 0.0;
                f.r_inject_b = 0.0;
                f.r_leg      = k_sd_r196;
                m_fund.configure(f);
                m_fund.prepare(sample_rate);

                bridged_t::config h;
                h.c_arm_in   = k_sd_c60;
                h.c_arm_out  = k_sd_c61;
                h.r_bridge   = k_sd_r198;
                h.r_inject_a = 0.0;
                h.r_inject_b = 0.0;
                h.r_leg      = k_sd_r195;
                m_harm.configure(h);
                m_harm.prepare(sample_rate);

                m_shaper.prepare(sample_rate);
                // H(s) for the R189/C57/R190 divider (see the constants' comment).
                m_shaper.set(k_sd_r189 * k_sd_r190 * k_sd_c57, k_sd_r190, k_sd_r189 * k_sd_r190 * k_sd_c57,
                             k_sd_r189 + k_sd_r190);

                m_env.prepare(sample_rate);
                m_env.set_times(k_sd_snappy_att_s, k_sd_snappy_tau_s);

                m_noise_hp1.prepare(sample_rate);
                m_noise_hp2.prepare(sample_rate);
                m_noise_lp.prepare(sample_rate);
                const double tau = 1.0 / (2.0 * k_pi * k_sd_snappy_hp_hz);
                m_noise_hp1.set(tau, 0.0, tau, 1.0);
                m_noise_hp2.set(tau, 0.0, tau, 1.0);
                const double tau_lp = 1.0 / (2.0 * k_pi * k_sd_snappy_lp_hz);
                m_noise_lp.set(0.0, 1.0, tau_lp, 1.0);
                m_noise_lp2.prepare(sample_rate);
                m_noise_lp2.set(0.0, 1.0, tau_lp, 1.0);

                set_tuning(m_tuning);
                reset();
            }

            void reset() {
                m_fund.reset();
                m_harm.reset();
                m_shaper.reset();
                m_env.reset();
                m_noise_hp1.reset();
                m_noise_hp2.reset();
                m_noise_lp.reset();
                m_noise_lp2.reset();
                m_noise.reset();
                m_pulse_remaining = 0;
                m_vtrig           = 0.0;
            }

            // -- panel ---------------------------------------------------------------------

            /// Resonator balance, 0..1 (VR8, SD TONE): 0 = fundamental (~173 Hz) only,
            /// 1 = harmonic (~336 Hz) only. Hardware noon is an even blend.
            void set_tone(double amount) { m_tone = std::clamp(amount, 0.0, 1.0); }

            /// Snappy amount, 0..1 (VR9): depth of the enveloped noise path.
            void set_snappy(double amount) { m_snappy = std::clamp(amount, 0.0, 1.0); }

            /// Output level, 0..1 (VR7).
            void set_level(double amount) { m_level = std::clamp(amount, 0.0, 1.0); }

            // -- circuit bends (stock hardware at the defaults) ----------------------------

            /// Swing-VCA drive on the snappy noise path (0 = the calibrated linear model,
            /// bit-identical; > 0 engages the swing VCA's symmetric harmonic saturation — grit and
            /// compression that ride the snappy envelope, hardest on the transient crack). See
            /// swing_vca.h / vca.h swing_shape.
            void   set_drive(double amount) { m_drive = std::max(0.0, amount); }
            double drive() const { return m_drive; }

            /// Pitch as a ratio of the stock tuning (0.25..4): scales both resonators' arm
            /// capacitors together. 1.0 = the late-revision schematic (~173/336 Hz).
            void set_tuning(double ratio) {
                m_tuning = std::clamp(ratio, 0.25, 4.0);
                m_fund.set_cap_scale(1.0 / m_tuning);
                m_harm.set_cap_scale(1.0 / m_tuning);
            }

            /// Noise-source seed (deterministic; mc. instances decorrelate by seed).
            void set_seed(uint64_t seed) { m_noise.set_seed(seed); }

            // -- performance ---------------------------------------------------------------

            /// Fire the voice. `accent` 0..1 maps to the 4-14 V trigger bus.
            void trigger(double accent = 1.0) {
                const double a    = std::clamp(accent, 0.0, 1.0);
                m_vtrig           = k_sd_vtrig_min + a * (k_sd_vtrig_max - k_sd_vtrig_min);
                m_pulse_remaining = std::max(1, static_cast<int>(k_sd_pulse_ms * 0.001 * m_sr));
                m_env.trigger(m_vtrig / k_sd_vtrig_max); // snappy envelope rides the accent too
            }

            double process() {
                double v_pulse = 0.0;
                if (m_pulse_remaining > 0) {
                    v_pulse = m_vtrig;
                    --m_pulse_remaining;
                }

                const double v_plus = m_shaper.process(v_pulse);
                const double f      = m_fund.process(v_plus, 0.0, 0.0);
                const double h      = m_harm.process(v_plus, 0.0, 0.0);

                const double snap = swing_vca(m_noise_lp2.process(m_noise_lp.process(
                                                  m_noise_hp2.process(m_noise_hp1.process(m_noise.process())))),
                                              m_env.process(), m_drive)
                                    * std::pow(m_snappy, k_sd_snappy_taper) * k_sd_snappy_gain;

                const double mix = f * (1.0 - m_tone) + h * m_tone + snap;
                return mix * m_level * k_sd_out_scale;
            }

          private:
            double m_sr{48000.0};

            bridged_t   m_fund, m_harm;
            decay_env   m_env;
            white_noise m_noise;
            first_order m_shaper, m_noise_hp1, m_noise_hp2, m_noise_lp, m_noise_lp2;

            double m_tone{0.5}, m_snappy{0.5}, m_level{1.0};
            double m_tuning{1.0};
            double m_drive{0.0}; // swing-VCA saturation on the snappy path; 0 = linear (default)
            double m_vtrig{0.0};
            int    m_pulse_remaining{0};
        };

    } // namespace tr808
} // namespace tap::tools
