/// @file
/// @brief      Portable TR-808 bass drum kernel for tap.808.kick~ — no Max/Min dependency.
/// @details    A circuit-informed model of the Roland TR-808 bass drum voice, block for block
///             after Werner, Abel & Smith, "A Physically-Informed, Circuit-Bendable, Digital
///             Model of the Roland TR-808 Bass Drum Circuit" (Proc. DAFx-14): trigger logic ->
///             pulse shaper -> bridged-T resonator (with the feedback buffer closing a
///             regeneration loop around it) -> tone -> level -> output coupling, plus the
///             envelope generator that produces the attack frequency shift and the retriggering
///             pulse. Component values read off the TR-808 Service Notes schematic (first
///             edition, June 1981) as reproduced in the paper's Fig. 1; every constant below
///             carries its schematic reference designator.
///
///             The behaviors this buys, none of them bolted on:
///             - Nominal ring at fc = 1/(2*pi*sqrt(Reff*R167*C41*C42)) ~= 49.4 Hz (Roland's
///               chart says 56 Hz "typical"; real units measure as low as 48).
///             - The attack: for the first ~6 ms the envelope generator saturates Q43, shorting
///               R165 out of the bridged-T's leg, raising fc to ~129 Hz and raising Q — the
///               "frequency jump during the attack", distinct from the sigh.
///             - The retriggering pulse: as the envelope collapses and fc falls back, C39/R161/
///               D52 kick the resonator's center node again so the note doesn't step down in
///               amplitude.
///             - The pitch sigh: leakage through R161 lifts Q43's base when the center node
///               swings below about a diode drop; the paper's fitted memoryless nonlinearity
///               (their Eqn. 8: alpha = 14.3150, V0 = -0.5560, m = 1.4765e-5) converts the
///               center-node voltage to a collector current that shrinks the effective leg
///               resistance — big early swings ring sharp and relax down to fc as the note
///               decays. Their Eqn. 9 as printed is garbled; the leg formula below is
///               re-derived from KCL at Q43's collector and matches their stated limits
///               (iC = 0 -> R165 + R166; iC growing -> smaller).
///             - Accent is the trigger *voltage* (4-14 V, the panel accent bus): a hotter pulse
///               excites the network harder and drives the nonlinearities differently — not a
///               post gain.
///             - No "machine gun effect": filter states persist across triggers, so retriggering
///               into a ringing tail interferes constructively/destructively like the hardware.
///
///             Deliberate simplifications (documented in the paper as well): op-amps ideal (no
///             rail saturation), the envelope generator is modeled behaviorally (fast rise,
///             ~1.1 ms release; settles ~5-6 ms after the trigger, per their §8), the level pot
///             is a plain gain (its 47 uF coupling corner is sub-audible), and the leg
///             modulation uses the previous sample's center-node voltage (one-sample lag instead
///             of an implicit nonlinear solve). The wave-digital upgrade path is planned as a
///             selectable circuit, svf.h-style.
///
///             Circuit bends (beyond-hardware attributes, stock by default): `tuning` scales the
///             arm capacitors (the paper's "tuning" bend), `pulse_ms` the trigger pulse width,
///             `sigh` scales the leakage current (0 disconnects the pitch sigh — the paper's §2
///             example bend).
///
///             Plain C++17, stdlib only, per-sample, allocation-free after prepare().
/// @author     Timothy Place
/// @copyright  Copyright 2026 Timothy Place. Distributed under the New BSD License.

#pragma once

#include <algorithm>
#include <cmath>

#include "bridged_t.h"

namespace taptools {
    namespace tr808 {

        // TR-808 Service Notes component values (bass drum voice, schematic designators).
        constexpr double k_bd_r161 = 1e6;    // retrigger injection into the center node
        constexpr double k_bd_r162 = 4.7e3;  // pulse shaper divider, top
        constexpr double k_bd_r163 = 100e3;  // pulse shaper divider, bottom
        constexpr double k_bd_r164 = 47e3;   // feedback buffer input
        constexpr double k_bd_r165 = 47e3;   // bridged-T leg, shorted by Q43 during the attack
        constexpr double k_bd_r166 = 6.8e3;  // bridged-T leg, always in circuit
        constexpr double k_bd_r167 = 1e6;    // bridged-T bridge
        constexpr double k_bd_r169 = 47e3;   // feedback buffer shunt
        constexpr double k_bd_r170 = 470e3;  // feedback injection into the center node
        constexpr double k_bd_r171 = 220.0;  // tone network series
        constexpr double k_bd_r172 = 10e3;   // tone network, parallel with VR5
        constexpr double k_bd_r176 = 100e3;  // output coupling, shunt
        constexpr double k_bd_r177 = 82e3;   // output coupling, series
        constexpr double k_bd_c39  = 33e-9;  // retrigger high-pass (0.033 uF)
        constexpr double k_bd_c40  = 15e-9;  // pulse shaper (0.015 uF)
        constexpr double k_bd_c41  = 15e-9;  // bridged-T arm (0.015 uF)
        constexpr double k_bd_c42  = 15e-9;  // bridged-T arm (0.015 uF)
        constexpr double k_bd_c43  = 33e-6;  // feedback buffer (33 uF electrolytic)
        constexpr double k_bd_c45  = 100e-9; // tone low-pass (0.1 uF)
        constexpr double k_bd_c49  = 470e-9; // output coupling (0.47 uF)
        constexpr double k_bd_vr5  = 10e3;   // tone pot
        constexpr double k_bd_vr6  = 500e3;  // decay pot, linear taper ("500k(B)")

        // Trigger bus: the CPU's 1 ms common trigger rides the accent voltage, 4-14 V
        // depending on the global accent level VR3 (paper §3).
        constexpr double k_bd_vtrig_min = 4.0;
        constexpr double k_bd_vtrig_max = 14.0;
        constexpr double k_bd_pulse_ms  = 1.0;

        // One silicon diode drop — the pulse shaper's D53 and the retrigger's D52 both clamp
        // negative swings at this (paper §4.3, Eqn. 4).
        constexpr double k_bd_diode_v = 0.71;

        // Behavioral envelope generator (paper §8: swings up fast, settles back to ground
        // "approximately 5 ms after the trigger swings low"): one-pole rise during the pulse,
        // one-pole release after — 5 * 1.1 ms ~= the observed settle.
        constexpr double k_bd_env_attack_s  = 50e-6;
        constexpr double k_bd_env_release_s = 1.1e-3;

        // Q43 treated as saturated (leg = R166) while the envelope is above roughly a
        // base-emitter drop; crossfaded below it so the fc glide back down is smooth.
        constexpr double k_bd_q43_sat_v = 0.6;

        // Pitch-sigh nonlinearity fit, paper Eqn. 8: iC(Vcomm) in amperes.
        constexpr double k_bd_sigh_alpha = 14.3150;
        constexpr double k_bd_sigh_v0    = -0.5560;
        constexpr double k_bd_sigh_m     = 1.4765e-5;

        // Volts -> sample units: a full-accent, default-knob hit peaks just under +-1.
        // (Empirical over the model; the hardware op-amp would clip near its +-15 V rails.)
        constexpr double k_bd_out_scale = 1.0 / 12.0;

        /// The TR-808 bass drum voice. prepare(), set the panel, trigger(accent), then process()
        /// per sample.
        class kick {
          public:
            void prepare(double sample_rate) {
                m_sr = sample_rate;

                bridged_t::config cfg;
                cfg.c_arm_in   = k_bd_c41;
                cfg.c_arm_out  = k_bd_c42;
                cfg.r_bridge   = k_bd_r167;
                cfg.r_inject_a = k_bd_r161;
                cfg.r_inject_b = k_bd_r170;
                cfg.r_leg      = k_bd_r165 + k_bd_r166;
                m_bt.configure(cfg);
                m_bt.prepare(sample_rate);

                m_shaper.prepare(sample_rate);
                // Pulse shaper low shelf, paper Eqn. 3: unity at HF, R162/(R162+R163) at DC,
                // transition ~106 Hz .. ~2.4 kHz.
                m_shaper.set(k_bd_r162 * k_bd_r163 * k_bd_c40, k_bd_r162, k_bd_r162 * k_bd_r163 * k_bd_c40,
                             k_bd_r162 + k_bd_r163);

                m_retrig_hp.prepare(sample_rate);
                // Retrigger high-pass C39 into R161 (paper §8.1, Fig. 8).
                m_retrig_hp.set(k_bd_r161 * k_bd_c39, 0.0, k_bd_r161 * k_bd_c39, 1.0);

                m_fb.prepare(sample_rate);
                m_tone_lp.prepare(sample_rate);
                m_out_hp.prepare(sample_rate);
                // Output coupling (paper Table 1 "High pass"): HF gain R177/R176, corner ~3.4 Hz.
                m_out_hp.set(k_bd_r177 * k_bd_c49, 0.0, k_bd_r176 * k_bd_c49, 1.0);

                set_decay(m_decay);
                set_tone(m_tone);
                set_tuning(m_tuning);
                reset();
            }

            void reset() {
                m_bt.reset();
                m_shaper.reset();
                m_retrig_hp.reset();
                m_fb.reset();
                m_tone_lp.reset();
                m_out_hp.reset();
                m_env = m_vfb_z   = 0.0;
                m_pulse_remaining = 0;
                m_vtrig           = 0.0;
            }

            // -- panel ---------------------------------------------------------------------

            /// Note length, 0..1 (VR6). Hardware range is roughly 50 ms .. 800 ms.
            void set_decay(double amount) {
                m_decay           = std::clamp(amount, 0.0, 1.0);
                const double vr6k = m_decay * k_bd_vr6; // linear taper
                // Feedback buffer high shelf, paper Eqn. 6 (inverting: DC gain -R169/R164).
                m_fb.set(-k_bd_r169 * vr6k * k_bd_c43, -k_bd_r169, k_bd_r164 * (k_bd_r169 + vr6k) * k_bd_c43,
                         k_bd_r164);
            }

            /// Attack brightness, 0..1 (VR5): 1 leaves the click in (~7.2 kHz corner),
            /// 0 rounds it off (~305 Hz).
            void set_tone(double amount) {
                m_tone           = std::clamp(amount, 0.0, 1.0);
                const double l   = 1.0 - m_tone; // pot travel: full resistance = darkest
                const double vr5 = l * k_bd_vr5;
                const double req = k_bd_r171 + (k_bd_r172 * vr5) / (k_bd_r172 + vr5);
                m_tone_lp.set(0.0, 1.0, req * k_bd_c45, 1.0);
            }

            /// Output level, 0..1 (VR4).
            void set_level(double amount) { m_level = std::clamp(amount, 0.0, 1.0); }

            // -- circuit bends (stock hardware at the defaults) ----------------------------

            /// Pitch as a ratio of the stock tuning (0.25..4): scales the bridged-T arm
            /// capacitors, the paper's "tuning" bend. 1.0 = the schematic (~49 Hz). Both arms
            /// scale together, so fc is proportional to the ratio directly.
            void set_tuning(double ratio) {
                m_tuning = std::clamp(ratio, 0.25, 4.0);
                m_bt.set_cap_scale(1.0 / m_tuning);
            }

            /// Depth of the Q43 attack frequency shift, 0..1: 1 = stock (the leg drops to R166
            /// for the first ~6 ms), 0 = the shift disconnected (a softer, rounder attack).
            void set_attack(double amount) { m_attack = std::clamp(amount, 0.0, 1.0); }

            /// Trigger pulse width in ms (the paper's "input pulse width" bend). Stock: 1 ms.
            void set_pulse_ms(double ms) { m_pulse_ms = std::clamp(ms, 0.05, 50.0); }

            /// Pitch-sigh depth, 0..1: scales the Q43 leakage current. 1 = stock, 0 = the
            /// "disconnect the pitch sigh" bend.
            void set_sigh(double amount) { m_sigh = std::clamp(amount, 0.0, 1.0); }

            // -- performance ---------------------------------------------------------------

            /// Fire the voice. `accent` 0..1 maps to the 4-14 V trigger bus; states are NOT
            /// reset (retriggering into a ringing tail behaves like the hardware).
            void trigger(double accent = 1.0) {
                const double a    = std::clamp(accent, 0.0, 1.0);
                m_vtrig           = k_bd_vtrig_min + a * (k_bd_vtrig_max - k_bd_vtrig_min);
                m_pulse_remaining = std::max(1, static_cast<int>(m_pulse_ms * 0.001 * m_sr));
            }

            double process() {
                // Trigger logic: a pulse_ms-long pulse at the accent voltage (paper §3).
                double v_pulse = 0.0;
                if (m_pulse_remaining > 0) {
                    v_pulse = m_vtrig;
                    --m_pulse_remaining;
                }

                // Pulse shaper: low shelf + D53 clamp of negative swings (paper §4).
                const double v_plus = diode_clamp(m_shaper.process(v_pulse));

                // Envelope generator (behavioral, paper §8).
                const double tau = (m_pulse_remaining > 0) ? k_bd_env_attack_s : k_bd_env_release_s;
                const double a   = 1.0 - std::exp(-1.0 / (tau * m_sr));
                m_env += a * (((m_pulse_remaining > 0) ? m_vtrig : 0.0) - m_env);

                // Retriggering pulse: high-passed envelope, D52-clamped (paper §8.1).
                const double v_rp = diode_clamp(m_retrig_hp.process(m_env));

                // Q43 leg modulation: attack saturation crossfaded with the sigh leakage,
                // driven by the previous sample's center-node voltage (see header note).
                const double v_comm = m_bt.v_comm();
                const double ic     = m_sigh * sigh_current(v_comm);
                double       r_sigh = k_bd_r165 + k_bd_r166;
                const double denom  = v_comm + k_bd_r165 * ic;
                if (v_comm < 0.0 && denom < 0.0)
                    r_sigh = std::clamp(v_comm * (k_bd_r165 + k_bd_r166) / denom, k_bd_r166, k_bd_r165 + k_bd_r166);
                const double sat = std::min(m_env / k_bd_q43_sat_v, 1.0) * m_attack;
                m_bt.set_leg_resistance(k_bd_r166 + (r_sigh - k_bd_r166) * (1.0 - sat));

                // The resonator, with the feedback buffer closing the loop through a unit
                // delay (paper §10).
                const double v_bt = m_bt.process(v_plus, v_rp, m_vfb_z);
                m_vfb_z           = m_fb.process(v_bt);

                // Tone -> level -> output coupling (paper §9).
                const double v_out = m_out_hp.process(m_tone_lp.process(v_bt)) * m_level;
                return v_out * k_bd_out_scale;
            }

          private:
            /// Paper Eqn. 4: leaves positive voltages alone, lets negative ones swing only
            /// about one diode drop.
            static double diode_clamp(double v) { return v >= 0.0 ? v : k_bd_diode_v * (std::exp(v) - 1.0); }

            /// Paper Eqn. 8: the fitted Vcomm -> iC memoryless nonlinearity (iC <= 0).
            static double sigh_current(double v_comm) {
                const double x = -k_bd_sigh_alpha * (v_comm - k_bd_sigh_v0);
                // log1p(exp(x)) without overflow for large x.
                const double softplus = x > 30.0 ? x : std::log1p(std::exp(x));
                return -(k_bd_sigh_m / k_bd_sigh_alpha) * softplus;
            }

            double m_sr{48000.0};

            bridged_t   m_bt;
            first_order m_shaper, m_retrig_hp, m_fb, m_tone_lp, m_out_hp;

            double m_decay{0.5}, m_tone{0.5}, m_level{0.5};
            double m_tuning{1.0}, m_pulse_ms{k_bd_pulse_ms}, m_sigh{1.0}, m_attack{1.0};

            double m_env{0.0};
            double m_vfb_z{0.0};
            double m_vtrig{0.0};
            int    m_pulse_remaining{0};
        };

    } // namespace tr808
} // namespace taptools
