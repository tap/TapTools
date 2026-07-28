/// @file
/// @brief      Portable bridged-T resonator kernel — the TR-808's universal voice circuit.
/// @details    An op-amp stage with a bridged-T network in its feedback path: capacitive "arms"
///             (C_a from the inverting node to the center node, C_b from the center node to the
///             op-amp output), a resistive "bridge" (R_bridge, inverting node to output), and a
///             resistive path from the center node to ground (the "leg"). Excited impulsively it
///             rings as a decaying pseudo-sinusoid at
///
///                 fc = 1 / (2*pi*sqrt(R_leg_eff * R_bridge * C_a * C_b)),
///
///             where R_leg_eff is the leg in parallel with every resistive injection into the
///             center node. Roland used this network in *every* TR-808 voice (bass drum, snare,
///             toms/congas, rim shot/claves as resonators; handclap, cowbell, cymbal, hi-hats as
///             band-pass filters), so this one class is the family's shared core.
///
///             Provenance: Werner, Abel & Smith, "A Physically-Informed, Circuit-Bendable,
///             Digital Model of the Roland TR-808 Bass Drum Circuit" (DAFx-14) — the topology
///             here reproduces that paper's printed transfer functions exactly. With injections
///             grounded, Vbt(s)/V+(s) matches their Eqn. (5) coefficients (beta_2 = alpha_2 =
///             R_eff*R167*C41*C42, beta_1 = alpha_1 + R167*C41, beta_0 = alpha_0 = 1, alpha_1 =
///             R_eff*(C41+C42)); the injected paths match their Hbt2/Hbt3, interchanged by
///             injection resistor, and the center node they call Vcomm is exposed for the bass
///             drum's pitch-sigh nonlinearity. (Re-derived by nodal analysis, 2026-07-17, and
///             pinned by the unit tests.)
///
///             Discretization is trapezoidal (capacitor companion models -> a 2x2 linear solve
///             per sample) — algebraically the bilinear transform the paper uses, but solved on
///             the network states directly, so the time-varying leg resistance (the 808 bass
///             drum's attack shift and pitch sigh modulate it per-sample) is handled without
///             per-sample coefficient redesign, in the same zero-delay-feedback family as the
///             house svf.h.
///
///             Plain C++17, stdlib only, allocation-free, no Max/Min dependency.
/// @author     Timothy Place
// SPDX-License-Identifier: MIT
// Copyright 2026 Timothy Place.

#pragma once

#include <algorithm>
#include <cmath>

#include "numeric.h"

namespace tap::tools {
    namespace tr808 {

        /// First-order continuous-time section H(s) = (b1*s + b0) / (a1*s + a0), discretized with
        /// the bilinear transform (c = 2/T, untuned — the DAFx-14 paper's choice; every corner in
        /// these circuits sits far below Nyquist). Direct-form II transposed; coefficients may be
        /// re-set while running (the single state is preserved), which is how the decay and tone
        /// pots are handled.
        template <typename Sample>
        class basic_first_order {
            static_assert(is_sample_profile<Sample>,
                          "basic_first_order supports the two Tap numeric profiles: float and double");

          public:
            using sample_type = Sample;

            void prepare(Sample sample_rate) {
                m_c = Sample(2.0) * sample_rate;
                update();
            }

            /// Set the analog prototype coefficients. Safe while running.
            void set(Sample b1, Sample b0, Sample a1, Sample a0) {
                m_b1 = b1;
                m_b0 = b0;
                m_a1 = a1;
                m_a0 = a0;
                update();
            }

            void reset() { m_z = Sample(0.0); }

            Sample process(Sample x) {
                const Sample y = m_n0 * x + m_z;
                m_z            = m_n1 * x - m_d1 * y;
                return y;
            }

          private:
            void update() {
                const Sample a0d = m_a1 * m_c + m_a0; // denominator at z = 1 side
                m_n0             = (m_b1 * m_c + m_b0) / a0d;
                m_n1             = (m_b0 - m_b1 * m_c) / a0d;
                m_d1             = (m_a0 - m_a1 * m_c) / a0d;
            }

            Sample m_b1{Sample(0.0)}, m_b0{Sample(1.0)}, m_a1{Sample(0.0)}, m_a0{Sample(1.0)};
            Sample m_c{Sample(96000.0)};
            Sample m_n0{Sample(1.0)}, m_n1{Sample(0.0)}, m_d1{Sample(0.0)};
            Sample m_z{Sample(0.0)};
        };

        /// The bridged-T op-amp resonator. Voltages are in volts, resistances in ohms,
        /// capacitances in farads — component values go in as they read off the schematic.
        template <typename Sample>
        class basic_bridged_t {
            static_assert(is_sample_profile<Sample>,
                          "basic_bridged_t supports the two Tap numeric profiles: float and double");

          public:
            using sample_type = Sample;

            struct config {
                Sample c_arm_in{Sample(15e-9)};  ///< arm capacitor, inverting node -> center (BD: C41, 0.015 uF)
                Sample c_arm_out{Sample(15e-9)}; ///< arm capacitor, center -> op-amp output   (BD: C42, 0.015 uF)
                Sample r_bridge{Sample(1e6)};    ///< bridge resistor, inverting node -> output (BD: R167, 1 M)
                Sample r_inject_a{
                    Sample(1e6)}; ///< series resistor of injection A into the center (BD: R161); 0 disables
                Sample r_inject_b{
                    Sample(470e3)};           ///< series resistor of injection B into the center (BD: R170); 0 disables
                Sample r_leg{Sample(53.8e3)}; ///< initial center-to-ground leg (BD: R165 + R166)
            };

            void configure(const config& cfg) {
                m_cfg = cfg;
                m_leg = cfg.r_leg;
                prepare_conductances();
            }

            void prepare(Sample sample_rate) {
                m_sr = sample_rate;
                prepare_conductances();
                reset();
            }

            void reset() {
                m_ieq_in = m_ieq_out = Sample(0.0);
                m_v_comm = m_v_out = Sample(0.0);
            }

            /// Time-varying center-to-ground leg resistance (the 808 modulates this via Q43).
            void set_leg_resistance(Sample ohms) { m_leg = std::max(ohms, Sample(1.0)); }

            /// Scale both arm capacitors by `scale` (the "tuning" circuit bend: fc scales by
            /// 1/scale). 1.0 is the stock schematic.
            void set_cap_scale(Sample scale) {
                m_cap_scale = std::clamp(scale, Sample(0.0625), Sample(16.0));
                prepare_conductances();
            }

            /// Effective leg resistance a caller should use for fc math: leg || injections.
            Sample effective_r(Sample leg_ohms) const {
                Sample g = Sample(1.0) / leg_ohms;
                if (m_g_inj_a > Sample(0.0))
                    g += m_g_inj_a;
                if (m_g_inj_b > Sample(0.0))
                    g += m_g_inj_b;
                return Sample(1.0) / g;
            }

            /// Advance one sample. `v_plus` drives the op-amp's non-inverting input (the pulse
            /// shaper in the bass drum); `v_inject_a` / `v_inject_b` are the source voltages
            /// behind r_inject_a / r_inject_b (retriggering pulse and feedback buffer in the bass
            /// drum). Returns the op-amp output Vbt; the center node is v_comm() afterwards.
            Sample process(Sample v_plus, Sample v_inject_a, Sample v_inject_b) {
                const Sample g_leg = Sample(1.0) / m_leg;

                // Trapezoidal companion models: each capacitor is a conductance G = 2C/T with a
                // history current source Ieq. Unknowns: the center node (Vcomm) and the op-amp
                // output (Vout); the inverting node is pinned to v_plus by the ideal op-amp.
                //   KCL @ inverting node:  G_in*(Vp - Vcomm) - Ieq_in + (Vp - Vout)*g_br = 0
                //   KCL @ center node:     [G_in*(Vp - Vcomm) - Ieq_in]
                //                        - [G_out*(Vcomm - Vout) - Ieq_out]
                //                        + (Va - Vcomm)*g_ia + (Vb - Vcomm)*g_ib - Vcomm*g_leg = 0
                const Sample g_sum = m_g_in + m_g_out + m_g_inj_a + m_g_inj_b + g_leg;

                // Row 1: G_in*Vcomm + g_br*Vout = (G_in + g_br)*Vp - Ieq_in
                // Row 2: g_sum*Vcomm - G_out*Vout = G_in*Vp - Ieq_in + Ieq_out + g_ia*Va + g_ib*Vb
                const Sample r1 = (m_g_in + m_g_br) * v_plus - m_ieq_in;
                const Sample r2 =
                    m_g_in * v_plus - m_ieq_in + m_ieq_out + m_g_inj_a * v_inject_a + m_g_inj_b * v_inject_b;
                const Sample det = -(m_g_in * m_g_out + m_g_br * g_sum);

                m_v_comm = (-m_g_out * r1 - m_g_br * r2) / det;
                m_v_out  = (m_g_in * r2 - g_sum * r1) / det;

                // Companion updates: Ieq' = 2*G*v - Ieq (v = present capacitor voltage).
                const Sample v_in  = v_plus - m_v_comm;
                const Sample v_out = m_v_comm - m_v_out;
                m_ieq_in           = Sample(2.0) * m_g_in * v_in - m_ieq_in;
                m_ieq_out          = Sample(2.0) * m_g_out * v_out - m_ieq_out;

                // Denormal guard: a decayed tail's history currents shrink geometrically forever.
                if (std::abs(m_ieq_in) < Sample(1e-30))
                    m_ieq_in = Sample(0.0);
                if (std::abs(m_ieq_out) < Sample(1e-30))
                    m_ieq_out = Sample(0.0);

                return m_v_out;
            }

            Sample v_comm() const { return m_v_comm; }

          private:
            void prepare_conductances() {
                const Sample t = Sample(1.0) / m_sr;
                m_g_in         = Sample(2.0) * m_cfg.c_arm_in * m_cap_scale / t;
                m_g_out        = Sample(2.0) * m_cfg.c_arm_out * m_cap_scale / t;
                m_g_br         = Sample(1.0) / m_cfg.r_bridge;
                m_g_inj_a      = m_cfg.r_inject_a > Sample(0.0) ? Sample(1.0) / m_cfg.r_inject_a : Sample(0.0);
                m_g_inj_b      = m_cfg.r_inject_b > Sample(0.0) ? Sample(1.0) / m_cfg.r_inject_b : Sample(0.0);
            }

            config m_cfg{};
            Sample m_sr{Sample(48000.0)};
            Sample m_cap_scale{Sample(1.0)};
            Sample m_leg{Sample(53.8e3)};
            Sample m_g_in{Sample(0.0)}, m_g_out{Sample(0.0)}, m_g_br{Sample(0.0)}, m_g_inj_a{Sample(0.0)},
                m_g_inj_b{Sample(0.0)};
            Sample m_ieq_in{Sample(0.0)}, m_ieq_out{Sample(0.0)};
            Sample m_v_comm{Sample(0.0)}, m_v_out{Sample(0.0)};
        };

        /// The double profile — the golden model.
        using first_order = basic_first_order<double>;
        using bridged_t   = basic_bridged_t<double>;

        /// The float profile — for single-precision targets. See numeric.h.
        using first_order32 = basic_first_order<float>;
        using bridged_t32   = basic_bridged_t<float>;

    } // namespace tr808
} // namespace tap::tools
