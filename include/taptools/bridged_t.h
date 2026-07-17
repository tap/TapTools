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
/// @copyright  Copyright 2026 Timothy Place. Distributed under the New BSD License.

#pragma once

#include <algorithm>
#include <cmath>

namespace taptools {
    namespace tr808 {

        constexpr double k_pi = 3.14159265358979323846;

        /// First-order continuous-time section H(s) = (b1*s + b0) / (a1*s + a0), discretized with
        /// the bilinear transform (c = 2/T, untuned — the DAFx-14 paper's choice; every corner in
        /// these circuits sits far below Nyquist). Direct-form II transposed; coefficients may be
        /// re-set while running (the single state is preserved), which is how the decay and tone
        /// pots are handled.
        class first_order {
          public:
            void prepare(double sample_rate) {
                m_c = 2.0 * sample_rate;
                update();
            }

            /// Set the analog prototype coefficients. Safe while running.
            void set(double b1, double b0, double a1, double a0) {
                m_b1 = b1;
                m_b0 = b0;
                m_a1 = a1;
                m_a0 = a0;
                update();
            }

            void reset() { m_z = 0.0; }

            double process(double x) {
                const double y = m_n0 * x + m_z;
                m_z            = m_n1 * x - m_d1 * y;
                return y;
            }

          private:
            void update() {
                const double a0d = m_a1 * m_c + m_a0; // denominator at z = 1 side
                m_n0             = (m_b1 * m_c + m_b0) / a0d;
                m_n1             = (m_b0 - m_b1 * m_c) / a0d;
                m_d1             = (m_a0 - m_a1 * m_c) / a0d;
            }

            double m_b1{0.0}, m_b0{1.0}, m_a1{0.0}, m_a0{1.0};
            double m_c{96000.0};
            double m_n0{1.0}, m_n1{0.0}, m_d1{0.0};
            double m_z{0.0};
        };

        /// The bridged-T op-amp resonator. Voltages are in volts, resistances in ohms,
        /// capacitances in farads — component values go in as they read off the schematic.
        class bridged_t {
          public:
            struct config {
                double c_arm_in{15e-9};   ///< arm capacitor, inverting node -> center (BD: C41, 0.015 uF)
                double c_arm_out{15e-9};  ///< arm capacitor, center -> op-amp output   (BD: C42, 0.015 uF)
                double r_bridge{1e6};     ///< bridge resistor, inverting node -> output (BD: R167, 1 M)
                double r_inject_a{1e6};   ///< series resistor of injection A into the center (BD: R161); 0 disables
                double r_inject_b{470e3}; ///< series resistor of injection B into the center (BD: R170); 0 disables
                double r_leg{53.8e3};     ///< initial center-to-ground leg (BD: R165 + R166)
            };

            void configure(const config& cfg) {
                m_cfg = cfg;
                m_leg = cfg.r_leg;
                prepare_conductances();
            }

            void prepare(double sample_rate) {
                m_sr = sample_rate;
                prepare_conductances();
                reset();
            }

            void reset() {
                m_ieq_in = m_ieq_out = 0.0;
                m_v_comm = m_v_out = 0.0;
            }

            /// Time-varying center-to-ground leg resistance (the 808 modulates this via Q43).
            void set_leg_resistance(double ohms) { m_leg = std::max(ohms, 1.0); }

            /// Scale both arm capacitors by `scale` (the "tuning" circuit bend: fc scales by
            /// 1/scale). 1.0 is the stock schematic.
            void set_cap_scale(double scale) {
                m_cap_scale = std::clamp(scale, 0.0625, 16.0);
                prepare_conductances();
            }

            /// Effective leg resistance a caller should use for fc math: leg || injections.
            double effective_r(double leg_ohms) const {
                double g = 1.0 / leg_ohms;
                if (m_g_inj_a > 0.0)
                    g += m_g_inj_a;
                if (m_g_inj_b > 0.0)
                    g += m_g_inj_b;
                return 1.0 / g;
            }

            /// Advance one sample. `v_plus` drives the op-amp's non-inverting input (the pulse
            /// shaper in the bass drum); `v_inject_a` / `v_inject_b` are the source voltages
            /// behind r_inject_a / r_inject_b (retriggering pulse and feedback buffer in the bass
            /// drum). Returns the op-amp output Vbt; the center node is v_comm() afterwards.
            double process(double v_plus, double v_inject_a, double v_inject_b) {
                const double g_leg = 1.0 / m_leg;

                // Trapezoidal companion models: each capacitor is a conductance G = 2C/T with a
                // history current source Ieq. Unknowns: the center node (Vcomm) and the op-amp
                // output (Vout); the inverting node is pinned to v_plus by the ideal op-amp.
                //   KCL @ inverting node:  G_in*(Vp - Vcomm) - Ieq_in + (Vp - Vout)*g_br = 0
                //   KCL @ center node:     [G_in*(Vp - Vcomm) - Ieq_in]
                //                        - [G_out*(Vcomm - Vout) - Ieq_out]
                //                        + (Va - Vcomm)*g_ia + (Vb - Vcomm)*g_ib - Vcomm*g_leg = 0
                const double g_sum = m_g_in + m_g_out + m_g_inj_a + m_g_inj_b + g_leg;

                // Row 1: G_in*Vcomm + g_br*Vout = (G_in + g_br)*Vp - Ieq_in
                // Row 2: g_sum*Vcomm - G_out*Vout = G_in*Vp - Ieq_in + Ieq_out + g_ia*Va + g_ib*Vb
                const double r1 = (m_g_in + m_g_br) * v_plus - m_ieq_in;
                const double r2 =
                    m_g_in * v_plus - m_ieq_in + m_ieq_out + m_g_inj_a * v_inject_a + m_g_inj_b * v_inject_b;
                const double det = -(m_g_in * m_g_out + m_g_br * g_sum);

                m_v_comm = (-m_g_out * r1 - m_g_br * r2) / det;
                m_v_out  = (m_g_in * r2 - g_sum * r1) / det;

                // Companion updates: Ieq' = 2*G*v - Ieq (v = present capacitor voltage).
                const double v_in  = v_plus - m_v_comm;
                const double v_out = m_v_comm - m_v_out;
                m_ieq_in           = 2.0 * m_g_in * v_in - m_ieq_in;
                m_ieq_out          = 2.0 * m_g_out * v_out - m_ieq_out;

                // Denormal guard: a decayed tail's history currents shrink geometrically forever.
                if (std::abs(m_ieq_in) < 1e-30)
                    m_ieq_in = 0.0;
                if (std::abs(m_ieq_out) < 1e-30)
                    m_ieq_out = 0.0;

                return m_v_out;
            }

            double v_comm() const { return m_v_comm; }

          private:
            void prepare_conductances() {
                const double t = 1.0 / m_sr;
                m_g_in         = 2.0 * m_cfg.c_arm_in * m_cap_scale / t;
                m_g_out        = 2.0 * m_cfg.c_arm_out * m_cap_scale / t;
                m_g_br         = 1.0 / m_cfg.r_bridge;
                m_g_inj_a      = m_cfg.r_inject_a > 0.0 ? 1.0 / m_cfg.r_inject_a : 0.0;
                m_g_inj_b      = m_cfg.r_inject_b > 0.0 ? 1.0 / m_cfg.r_inject_b : 0.0;
            }

            config m_cfg{};
            double m_sr{48000.0};
            double m_cap_scale{1.0};
            double m_leg{53.8e3};
            double m_g_in{0.0}, m_g_out{0.0}, m_g_br{0.0}, m_g_inj_a{0.0}, m_g_inj_b{0.0};
            double m_ieq_in{0.0}, m_ieq_out{0.0};
            double m_v_comm{0.0}, m_v_out{0.0};
        };

    } // namespace tr808
} // namespace taptools
