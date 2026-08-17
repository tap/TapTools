/// @file
/// @brief      Portable Ondes Martenot voice kernels (tap.triode~, tap.ondes~) — no Max/Min dep.
/// @details    The last two pieces of the instrument, and the two the family plan got wrong before
///             the sources were read. The plan assumed a `vco.h` descendant with waveform
///             registers. The Ondes Martenot is nothing of the kind: it is a **heterodyne**
///             instrument whose two 80 kHz oscillators measure as essentially pure (about 0.03 %
///             second-harmonic distortion even coupled to the rest of the circuit), and every bit
///             of its character comes from what happens *after* the two of them are summed —
///             the envelope detector, two triode gain stages, the intensity key, and the
///             diffuseur.
///
///             The circuit is Najnudel, Hélie, Roze & Boutin, "Simulation of an ondes Martenot
///             circuit", IEEE/ACM TASLP **28**, 2651–2660, 2020, modelling instrument No. 169 as
///             five port-Hamiltonian stages. This file is **not** that: their full solve runs at
///             768 kHz and their plugin costs 85 % of a laptop core. What this file takes from
///             them is their *own* published reductions plus their published component values,
///             and it says which is which.
///
///             Three classes and a thin composition:
///             - `triode` — one common-cathode gain stage, solved on its load line from the
///               **enhanced Norman Koren tube model** at a **published operating point**.
///             - `detector` — the heterodyne pair and its envelope detector, in closed form.
///             - `voice` — detector into two triode stages into the intensity key.
///
///             ## The tube (published model, published parameters)
///
///             Koren's model (N. Koren, "Improved vacuum tube models for Spice simulations",
///             *Glass Audio* 8(5), 1996) as extended with a grid-current branch by Cohen & Hélie
///             (AES 129th Convention, 2010), which is what the circuit paper uses:
///
///                 E1 = (vpc/Kp) · ln(1 + exp(Kp · (1/mu + (vgc + Vct)/sqrt(Kvb + vpc²))))
///                 ipc = 2·E1^Ex / Kg    for E1 >= 0, else 0
///                 igc = (vgc - Va)/Rgk  for vgc >= Va, else 0
///
///             The parameter sets are **fitted to the actual tubes in ondes No. 169** and are
///             reproduced verbatim from the circuit paper's Table II — 6F5 in the oscillators,
///             6C5 in the demodulator and preamplifier, 2A3 in the power amplifier — together
///             with that table's stage supply voltages and cathode resistors. Nothing here is
///             voiced by ear; the numbers are the citation.
///
///             A stage is then the static solution of the load line
///             `ipc(vpc, vgc) = (Vbias − Vk − vpc)/Rp` with the cathode bias `Vk = Rk·Ipc` found
///             at the quiescent point — a **memoryless nonlinearity**, which is exactly the term
///             in Yeh, Abel & Smith's DAFx-07 simplified cascade (conditioning filter →
///             memoryless nonlinearity → equalization filter), the same architecture `fuzz.h`
///             uses. The curve is tabulated once per tube/operating-point change and read with
///             linear interpolation, so the audio path costs a lookup rather than a root find.
///
///             ## The detector (exact, and cheaper than the thing it replaces)
///
///             The circuit paper writes the oscillator sum as an amplitude-modulated sinewave,
///             `cos(Φ) + cos(Φ − φm) = 2 cos(Φ − φm/2) cos(φm/2)`, and detects its envelope with
///             a triode whose grid sits near zero bias — so the grid-cathode junction behaves as
///             a diode, conducting only on positive half-cycles — loaded by R4·C21, a time
///             constant of **200 µs**.
///
///             Two consequences, and the second is the one the family plan missed.
///
///             **The envelope is not a sinusoid.** For equal oscillator amplitudes it is
///             `2|cos(π f t)|`, whose Fourier series puts the second harmonic 14.0 dB below the
///             fundamental, the third 21.3 dB down and the fourth 26.4 dB down — a substantial
///             harmonic series generated *before any triode touches the signal*. The plan said to
///             synthesize the difference tone directly as a sinusoid; that would have thrown away
///             the instrument's single largest source of harmonics. (What the paper replaces with
///             a sinewave generator is the **oscillators**, not the demodulator.)
///
///             **So the carrier need never be simulated.** For amplitudes 1 and `depth` the
///             envelope is exactly `sqrt(1 + depth² + 2·depth·cos(2π f t))`, and running the
///             published RC detector on *that* — instant attack through the diode, 200 µs decay
///             through R4 — reproduces the full 80 kHz heterodyne-plus-diode-plus-RC simulation
///             to **within 0.10 dB on every harmonic** at every pitch tried, with no carrier to
///             alias and no 768 kHz to pay for (measured in notebooks/ondes.ipynb §2). The one
///             systematic difference is a level offset: the closed form sits a uniform 3.0–3.2 %
///             high, because a follower chasing real carrier half-cycles never quite reaches the
///             peak between them. A constant scale on a synthesizer with a level control.
///
///             The detector's own pitch dependence comes with it, because the RC cannot follow a
///             fast envelope back down: the second harmonic runs from −14.0 dB at A2 to −19.3 dB
///             at A6, and the level falls 2.0 dB across those five octaves.
///
///             ## The ribbon
///
///             The circuit paper's Eq. 7 gives the variable oscillator's capacitance in terms of
///             ribbon displacement, and the frequency that falls out of it is
///             `f = A1 · 2^(d / (12 d0))` with A1 = 55 Hz the lowest note and d0 the displacement
///             of one semitone. So the ribbon is **linear in semitones**, which is why an ondes
///             glide sounds the way it does, and `set_ribbon()` takes semitones above A1 for
///             exactly that reason. (The ribbon runs up to about 1.2 m at the highest note.)
///
///             ## Oversampling, and a data point for an open question
///
///             The nonlinear chain runs oversampled on `fuzz.h`'s shared Butterworth chain, and
///             here the sequence behaves. Worst non-harmonic energy relative to the fundamental,
///             at 1× / 2× / 4× / 8× (notebooks/ondes.ipynb §5):
///
///                 587 Hz   −79.3  −91.2  −104.5  −103.8
///                1175 Hz   −65.8  −77.2   −90.6   −92.5
///                1760 Hz   −57.6  −70.9   −81.1   −82.2
///                2637 Hz   −51.1  −61.4   −71.8   −83.8
///                3520 Hz   −45.4  −56.8   −67.0   −74.2
///
///             Every doubling is worth about 12 dB up to 4×; past that it is worth 7–12 dB at the
///             top of the range and nothing at the bottom, where the measurement has already
///             bottomed out. **Never worse.** 4× is the default because it is where the cost
///             stops buying uniformly; 8× is there for anyone playing the top octave hard.
///
///             That matters beyond this file, because `fuzz.h` measured the opposite — 4× came
///             out *worse* than 2× there — and left an untested hypothesis behind: that the
///             culprit is *imaging*, since zero-stuffing by N leaves N−1 images for one filter to
///             suppress and their residuals intermodulate in the clipper into products that are
///             not harmonics of the input. This object is a **source**. Nothing is zero-stuffed
///             on the way up; the detector simply runs fast, so there are no images at all — and
///             the sequence never reverses. Evidence for that hypothesis rather than proof of it
///             (the nonlinearity differs too), but it is the first evidence either way, and it
///             points the same direction.
///
///             Honest limits:
///             - **This is not a circuit solve.** It is the published *reductions* of one: the
///               oscillators replaced by their closed-form envelope (the paper's own
///               simplification, and here an exact one), the stages reduced to static load-line
///               curves with their reactive coupling replaced by first-order conditioning and
///               equalization filters. The paper's full model is passive by construction; this
///               one is not, and does not claim to be.
///             - **The power amplifier is off by default**, following the paper: it measures
///               almost 5 % second harmonic in their simulation, but the authors report its
///               contribution to the final sound is much less important than the demodulator's
///               and preamplifier's, and drop it for real-time. Measured here, switching it on
///               moves total harmonic content from 0.248 to 0.251 and the second harmonic by
///               0.1 dB — an independent confirmation of their reason, and the reason it is a
///               switch rather than a deletion.
///             - **Where the intensity key sits in the chain is not published.** The paper's five
///               stages do not include it. Placing it after the triodes (the default) makes it a
///               clean output law; placing it before makes the dirt come up with the pressure.
///               Both are offered, and the choice is labelled a choice.
///             - **No waveform registers.** The real instrument has switchable timbres whose
///               filter shapes are not in any source obtained; adding them from imagination would
///               be the one thing this file is careful not to do.
///             - **No diffuseur.** That is `tap.palme~` / `tap.metallique~` — patch one after
///               this object, which is how the instrument works anyway.
///             - The tube parameters are a fit to *one* instrument's tubes, and tube-to-tube
///               spread in 1930s valves is wide.
/// @author     Timothy Place
// SPDX-License-Identifier: MIT
// Copyright 2026 Timothy Place.

#pragma once

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>

#include "fuzz.h"   // tap::tools::fuzz — ramp, biquad, butterworth8 (the shared oversampling chain)
#include "touche.h" // tap::tools::touche::key — the published intensity-key gain law

namespace tap::tools {
    namespace ondes {

        constexpr double k_pi = 3.14159265358979323846;

        using fuzz::butterworth8;
        using fuzz::ramp;

        // ---- the tube ------------------------------------------------------------------------

        /// One triode's enhanced-Koren parameter set. Every field is a fitted constant, not a
        /// design choice.
        struct tube_params {
            double mu;  ///< amplification factor
            double ex;  ///< plate-current exponent
            double kg;  ///< plate-current scale
            double kp;  ///< knee sharpness
            double kvb; ///< knee voltage
            double vct; ///< contact-potential offset
            double va;  ///< grid-conduction threshold
            double rgk; ///< grid-conduction resistance
        };

        // Najnudel, Hélie, Roze & Boutin, IEEE/ACM TASLP 28 (2020), Table II — fitted to the
        // datasheets of the tubes actually in ondes Martenot No. 169. Reproduced verbatim.
        constexpr tube_params k_6f5{98.0, 1.6, 2614.0, 905.0, 1.87, 0.5, 0.33, 1300.0};
        constexpr tube_params k_6c5{20.0, 1.5, 2837.0, 138.0, 89.0, 0.8, 0.33, 1300.0};
        constexpr tube_params k_2a3{4.3, 1.5, 1685.0, 43.0, 102.0, -1.2, 0.33, 1300.0};

        /// Which tube. The names are the instrument's: 6F5 oscillates, 6C5 demodulates and
        /// preamplifies, 2A3 drives the diffuseur.
        enum tube_index : int { tube_6f5 = 0, tube_6c5, tube_2a3, k_num_tubes };

        inline const tube_params& tube_at(int index) {
            switch (index) {
            case tube_6f5:
                return k_6f5;
            case tube_2a3:
                return k_2a3;
            default:
                return k_6c5;
            }
        }

        /// One stage's published operating point (TASLP Table II). `rp` is the plate load — the
        /// transformer core-loss resistance for the demodulator and preamplifier, the diffuseur's
        /// input impedance for the power amplifier.
        struct operating_point {
            double vbias;
            double rk;
            double rp;
        };

        constexpr operating_point k_op_demod{100.0, 1000.0, 4000.0};
        constexpr operating_point k_op_preamp{180.0, 1000.0, 4000.0};
        constexpr operating_point k_op_power{230.0, 750.0, 1500.0};

        /// Plate current in amps — the enhanced Koren law, verbatim from TASLP Eq. 8.
        inline double plate_current(const tube_params& t, double vpc, double vgc) {
            if (vpc <= 0.0) {
                return 0.0;
            }
            const double z = t.kp * (1.0 / t.mu + (vgc + t.vct) / std::sqrt(t.kvb + vpc * vpc));
            // log1p(exp(z)) with the standard overflow guard: for large z it is z itself.
            const double s  = (z > 60.0) ? z : std::log1p(std::exp(std::max(z, -60.0)));
            const double e1 = (vpc / t.kp) * s;
            return (e1 > 0.0) ? 2.0 * std::pow(e1, t.ex) / t.kg : 0.0;
        }

        /// Grid current in amps — TASLP Eq. 9. Zero below the conduction threshold, ohmic above.
        /// It is small next to the plate current, and the circuit paper keeps it because without
        /// it the modelled tube is not passive.
        inline double grid_current(const tube_params& t, double vgc) {
            return (vgc < t.va) ? 0.0 : (vgc - t.va) / t.rgk;
        }

        // ---- one triode stage ------------------------------------------------------------------

        constexpr int    k_curve_points = 1025;    // odd, so the quiescent point lands on a sample
        constexpr double k_curve_span_v = 30.0;    // grid swing the table covers, either way
        constexpr int    k_solve_steps  = 48;      // bisection steps per table point
        constexpr double k_stage_hp_hz  = 20.0;    // coupling capacitor / grid leak
        constexpr double k_stage_lp_hz  = 12000.0; // Miller capacitance and the plate load's pole

        constexpr double k_default_drive_v   = 1.0; // grid volts for a unit input
        constexpr double k_min_drive_v       = 0.001;
        constexpr double k_max_drive_v       = 40.0;
        constexpr double k_default_smooth_ms = 20.0;

        /// One common-cathode triode stage: conditioning highpass → drive → the tube's own load-line
        /// curve → equalization lowpass. The curve is the static solution of
        ///
        ///     ipc(vpc, vgc) = (Vbias − Vk − vpc) / Rp,   Vk = Rk · Ipc(quiescent)
        ///
        /// which is a memoryless nonlinearity in the DAFx-07 sense (Yeh, Abel & Smith 2007), so
        /// tabulating it is not an approximation of the model — it *is* the model, evaluated once.
        ///
        /// Output is normalized by the stage's own small-signal gain, so `drive` changes the
        /// distortion without changing the level. That is the gain-staging lesson `fuzz.h`
        /// learned the hard way, applied here from the start.
        class triode {
          public:
            void prepare(double sr) {
                m_sr = (sr > 0.0) ? sr : 48000.0;
                set_corners(k_stage_hp_hz, k_stage_lp_hz);
                build();
                clear();
            }

            void clear() { m_hp = m_lp = 0.0; }

            /// Which tube (tube_index). **A mode, not a fader**: it rebuilds the curve table
            /// (about a thousand load-line solves — sub-millisecond, but not something to automate
            /// at audio rate).
            void set_tube(int index) {
                m_tube = std::clamp(index, 0, k_num_tubes - 1);
                build();
            }

            /// The stage's supply, cathode resistor and plate load. Also a mode — same reason.
            void set_operating_point(const operating_point& op) {
                m_op = op;
                build();
            }

            /// Peak grid volts for a unit input. This is the harmonics control — the circuit
            /// paper's own plugin exposes demodulator input gain the same way, a knob the real
            /// instrument does not have.
            void set_drive(double volts) { m_drive = std::clamp(volts, k_min_drive_v, k_max_drive_v); }

            /// Conditioning highpass and equalization lowpass corners, in Hz.
            void set_corners(double hp_hz, double lp_hz) {
                m_a_hp = 1.0 - std::exp(-2.0 * k_pi * std::max(hp_hz, 0.0) / m_sr);
                m_a_lp = 1.0 - std::exp(-2.0 * k_pi * std::max(lp_hz, 1.0) / m_sr);
            }

            int    tube() const { return m_tube; }
            double drive() const { return m_drive; }
            double samplerate() const { return m_sr; }

            /// The quiescent point the curve was built around: cathode bias, plate voltage and
            /// plate current (volts, volts, amps). Worth exposing — it is the check that the
            /// published operating point produces a sane bias rather than a cut-off tube.
            double bias_v() const { return m_vk; }
            double quiescent_plate_v() const { return m_vp0; }
            double quiescent_current_a() const { return m_ip0; }

            /// Small-signal voltage gain magnitude at the quiescent point. The stage itself
            /// inverts; `process` preserves that, so this is reported unsigned.
            double small_signal_gain() const { return m_gain; }

            /// The stage's static curve: a normalized input in, the normalized plate swing out,
            /// exactly as `process` would map it with its filters bypassed. No state touched, so
            /// a notebook can plot the transfer without running audio.
            double curve_at(double x) const { return lookup(x * m_drive) / (m_gain * m_drive); }

            /// The same curve in the tube's own units: grid volts in, plate volts of swing out.
            double plate_swing_at(double grid_volts) const { return lookup(grid_volts); }

            /// One sample. `x` is normalized (±1 is a full-scale signal into `drive` volts).
            /// The output is INVERTED, because the stage is.
            double process(double x) {
                m_hp += m_a_hp * (x - m_hp);
                const double y = lookup((x - m_hp) * m_drive) / (m_gain * m_drive);
                m_lp += m_a_lp * (y - m_lp);
                return m_lp;
            }

          private:
            /// Solve the load line for the plate voltage at a given grid-to-cathode voltage.
            /// `plate_current` rises with vpc and the load line falls, so the difference is
            /// monotone and bisection cannot miss.
            double solve_plate(double vgc, double vk) const {
                const tube_params& t  = tube_at(m_tube);
                double             lo = 0.0;
                double             hi = m_op.vbias;
                for (int i = 0; i < k_solve_steps; ++i) {
                    const double mid = 0.5 * (lo + hi);
                    if (plate_current(t, mid, vgc) < (m_op.vbias - vk - mid) / m_op.rp) {
                        lo = mid;
                    }
                    else {
                        hi = mid;
                    }
                }
                return 0.5 * (lo + hi);
            }

            /// Find the self-bias point (Vk = Rk·Ip with the tube sitting at −Vk on its grid) and
            /// tabulate the transfer curve around it.
            void build() {
                const tube_params& t = tube_at(m_tube);

                // Cathode self-bias: a damped fixed-point iteration, which converges because
                // raising Vk lowers the current that sets it.
                m_vk = 1.0;
                for (int i = 0; i < 200; ++i) {
                    const double vp = solve_plate(-m_vk, m_vk);
                    m_vk            = 0.8 * m_vk + 0.2 * (plate_current(t, vp, -m_vk) * m_op.rk);
                }
                m_vp0 = solve_plate(-m_vk, m_vk);
                m_ip0 = plate_current(t, m_vp0, -m_vk);

                // The curve is the plate's true swing from quiescent, sign included: a
                // common-cathode stage INVERTS, and that matters here rather than being a
                // cosmetic detail — the tube's asymmetry is polarity-sensitive, so a stage that
                // quietly un-inverted itself would apply its curve to the wrong side of the
                // waveform and generate the wrong harmonics.
                for (int i = 0; i < k_curve_points; ++i) {
                    const double v = span_of(i);
                    m_curve[static_cast<size_t>(i)] =
                        solve_plate(v - m_vk, m_vk) - m_vp0; // grid volts v around the bias point
                }

                // Small-signal gain from a central difference across two table steps, as a
                // magnitude — the inversion lives in the curve, not in the normalization.
                const int    c  = k_curve_points / 2;
                const double dv = span_of(c + 1) - span_of(c - 1);
                m_gain = std::abs((m_curve[static_cast<size_t>(c + 1)] - m_curve[static_cast<size_t>(c - 1)]) / dv);
                if (!(m_gain > 1e-9)) {
                    m_gain = 1.0; // a cut-off operating point has no gain to normalize by
                }
            }

            static double span_of(int i) {
                return (2.0 * static_cast<double>(i) / static_cast<double>(k_curve_points - 1) - 1.0) * k_curve_span_v;
            }

            /// Linear interpolation into the curve, clamped at both ends (cut-off below, grid
            /// conduction above — the tube does not do anything interesting past either).
            double lookup(double grid_volts) const {
                const double p = (grid_volts / k_curve_span_v + 1.0) * 0.5 * (k_curve_points - 1);
                if (p <= 0.0) {
                    return m_curve[0];
                }
                if (p >= static_cast<double>(k_curve_points - 1)) {
                    return m_curve[k_curve_points - 1];
                }
                const double f = std::floor(p);
                const size_t i = static_cast<size_t>(f);
                const double u = p - f;
                return m_curve[i] + u * (m_curve[i + 1] - m_curve[i]);
            }

            double          m_sr{48000.0};
            int             m_tube{tube_6c5};
            operating_point m_op{k_op_demod};
            double          m_drive{k_default_drive_v};
            double          m_vk{0.0}, m_vp0{0.0}, m_ip0{0.0}, m_gain{1.0};
            double          m_a_hp{1.0}, m_a_lp{1.0};
            double          m_hp{0.0}, m_lp{0.0};

            std::array<double, k_curve_points> m_curve{};
        };

        // ---- the heterodyne detector -------------------------------------------------------------

        constexpr double k_a1_hz         = 55.0; // the ribbon's lowest note, per TASLP Eq. 7
        constexpr double k_detect_ms     = 0.2;  // R4 * C21 = 1 MOhm * 200 pF, TASLP Table II
        constexpr double k_min_detect_ms = 0.005;
        constexpr double k_max_detect_ms = 20.0;
        constexpr double k_min_note_hz   = 20.0;
        constexpr double k_max_note_hz   = 4000.0;
        constexpr double k_default_depth = 1.0;  // equal oscillator amplitudes: the published case
        constexpr double k_max_semitones = 72.0; // six octaves of ribbon, comfortably past 1.2 m

        /// The two oscillators, their sum, and the diode-plus-RC that detects its envelope —
        /// without simulating either oscillator.
        ///
        /// For amplitudes 1 and `depth` the envelope of `cos(Φ) + depth·cos(Φ − φ)` is exactly
        /// `sqrt(1 + depth² + 2 depth cos(φ))`, so the carrier drops out of the arithmetic
        /// entirely. At depth 1 that is `2|cos(φ/2)|`, the published case, whose harmonics sit at
        /// −14.0 / −21.3 / −26.4 dB before anything nonlinear happens.
        ///
        /// The detector itself is the paper's: the triode's grid sits near zero bias, so the
        /// grid-cathode junction conducts only on positive half-cycles and charges instantly,
        /// while R4·C21 discharges it with a 200 µs time constant. That asymmetry is why the
        /// object gets quieter and purer as it goes up — the RC cannot follow a fast envelope
        /// back down.
        class detector {
          public:
            void prepare(double sr) {
                m_sr = (sr > 0.0) ? sr : 48000.0;
                set_detect_ms(m_detect_ms);
                clear();
            }

            void clear() {
                m_phase = 0.0;
                m_env   = 0.0;
            }

            /// The note, in Hz.
            void set_frequency(double hz) { m_hz = std::clamp(hz, k_min_note_hz, k_max_note_hz); }

            /// The note, as the ribbon gives it: semitones above A1 (55 Hz). The circuit paper's
            /// Eq. 7 makes the ribbon linear in semitones, which is the whole feel of an ondes
            /// glide, so this is the primary way in.
            void set_ribbon(double semitones) {
                set_frequency(k_a1_hz * std::exp2(std::clamp(semitones, 0.0, k_max_semitones) / 12.0));
            }

            /// Relative amplitude of the second oscillator, 0..1. 1 is the published case (equal
            /// amplitudes, 100 % modulation, the envelope reaching zero); below that the envelope
            /// never closes and the harmonic series thins out. A real mismatch between two
            /// oscillators, and the cheapest timbre control this object has.
            void set_depth(double d) { m_depth = std::clamp(d, 0.0, 1.0); }

            /// The detector time constant in ms. Defaults to the published 200 µs (R4·C21).
            void set_detect_ms(double ms) {
                m_detect_ms = std::clamp(ms, k_min_detect_ms, k_max_detect_ms);
                m_decay     = std::exp(-1.0 / (m_detect_ms * 0.001 * m_sr));
            }

            double frequency() const { return m_hz; }
            double semitones() const { return 12.0 * std::log2(m_hz / k_a1_hz); }
            double depth() const { return m_depth; }
            double detect_ms() const { return m_detect_ms; }
            double samplerate() const { return m_sr; }

            /// The ideal envelope at a phase in [0, 1) — the closed form, with no detector on it.
            /// Exposed so a test can compare the detector against what it is detecting.
            double envelope_at(double phase) const {
                return std::sqrt(std::max(0.0, 1.0 + m_depth * m_depth + 2.0 * m_depth * std::cos(2.0 * k_pi * phase)));
            }

            /// Advance one sample and return the detected envelope. The DC it carries is real —
            /// the circuit's coupling capacitor removes it downstream, and so does the triode
            /// stage's conditioning highpass.
            double process() {
                m_phase += m_hz / m_sr;
                m_phase -= std::floor(m_phase);
                m_env = std::max(envelope_at(m_phase), m_env * m_decay);
                return m_env;
            }

          private:
            double m_sr{48000.0};
            double m_hz{k_a1_hz * 4.0};
            double m_depth{k_default_depth};
            double m_detect_ms{k_detect_ms};
            double m_decay{0.0};
            double m_phase{0.0};
            double m_env{0.0};
        };

        // ---- the voice -----------------------------------------------------------------------

        constexpr int    k_max_oversample = 8;
        constexpr int    k_default_os     = 4;
        constexpr double k_default_level  = 0.25; // two triode stages add up; this is a sane start
        constexpr double k_dc_r           = 0.999;

        /// Where the intensity key sits. The circuit paper's five stages do not include it, so
        /// this is a modelling choice rather than a reconstruction — and both readings are
        /// musical, which is why both are offered.
        enum key_placement : int {
            key_after = 0, ///< a clean output law: pressure changes level, not dirt
            key_before,    ///< pressure drives the tubes: soft is clean, hard is loud and dirty
            k_num_key_placements
        };

        /// The instrument, minus the diffuseur: the heterodyne detector into the demodulator
        /// triode into the preamplifier triode into the intensity key. The power amplifier is a
        /// switch, off by default, following the circuit paper's own reduction.
        ///
        /// The nonlinear part runs oversampled on the shared `fuzz.h` chain. Patch a
        /// `tap.palme~` or `tap.metallique~` after this object for the rest of the instrument.
        class voice {
          public:
            voice() {
                m_drive.snap(1.0);
                m_level.snap(k_default_level);
            }

            // -- lifecycle -----------------------------------------------------------------------

            void prepare(double sr) {
                m_sr = (sr > 0.0) ? sr : 48000.0;
                m_key.prepare(m_sr);
                configure();
                m_drive.snap(m_drive.target());
                m_level.snap(m_level.target());
                m_prepared = true;
                clear();
            }

            /// Silence the detector and every filter. Parameters are untouched.
            void clear() {
                m_detector.clear();
                m_demod.clear();
                m_preamp.clear();
                m_power.clear();
                m_down.reset();
                m_dc_x1 = m_dc_y1 = 0.0;
            }

            bool prepared() const { return m_prepared; }

            // -- the performance surface ----------------------------------------------------------

            /// The note, as the ribbon gives it: semitones above A1.
            void set_ribbon(double semitones) { m_detector.set_ribbon(semitones); }
            void set_frequency(double hz) { m_detector.set_frequency(hz); }

            /// Oscillator balance, 0..1 — see detector::set_depth.
            void set_depth(double d) { m_detector.set_depth(d); }

            /// Detector time constant in ms; the published value is 0.2.
            void set_detect_ms(double ms) { m_detector.set_detect_ms(ms); }

            /// Grid drive into the two triode stages, as a multiple of the published nominal.
            /// This is the harmonics control the paper's own plugin exposes.
            void set_drive(double x) { m_drive.to(std::clamp(x, 0.0, 8.0), smooth_samples()); }

            /// The intensity key, 0..1 over the physical travel (touche.h's contract: the bottom
            /// 45 % is silent, because that is the key bending before it reaches the powder bag).
            void set_key(double position) { m_key.set_position(position); }
            void set_key_mm(double mm) { m_key.set_position_mm(mm); }

            /// Where the key sits in the chain (key_placement) — a choice, see the header.
            void set_key_placement(int where) { m_key_where = std::clamp(where, 0, k_num_key_placements - 1); }

            /// Run the 2A3 power stage. Off by default: the paper measures almost 5 % second
            /// harmonic there but reports its contribution as much less important than the two
            /// stages before it, and drops it for real-time.
            void set_power_stage(bool on) { m_power_on = on; }

            /// Sign of the coupling between the two triode stages, +1 or -1. The circuit couples
            /// them through a transformer whose winding sense is not in the source, and the sign
            /// decides which side of the waveform the preamplifier's asymmetry acts on — so it is
            /// audible, and it is offered as a switch rather than guessed at silently.
            void set_polarity(int sign) { m_polarity = (sign < 0) ? -1.0 : 1.0; }

            void set_level(double lin) { m_level.to(lin, smooth_samples()); }

            /// Oversampling for the nonlinear chain: 1, 2, 4 or 8.
            void set_oversample(int os) {
                const int v = std::clamp(os, 1, k_max_oversample);
                m_os        = (v >= 8) ? 8 : (v >= 4) ? 4 : (v >= 2) ? 2 : 1;
                if (m_prepared) {
                    configure();
                    clear();
                }
            }

            void set_smooth_ms(double ms) { m_smooth_ms = std::max(0.0, ms); }

            // -- introspection ---------------------------------------------------------------------

            double semitones() const { return m_detector.semitones(); }
            double frequency() const { return m_detector.frequency(); }
            double depth() const { return m_detector.depth(); }
            double detect_ms() const { return m_detector.detect_ms(); }
            double drive() const { return m_drive.target(); }
            double key() const { return m_key.position(); }
            int    key_placement() const { return m_key_where; }
            bool   power_stage() const { return m_power_on; }
            int    polarity() const { return (m_polarity < 0.0) ? -1 : 1; }
            double level() const { return m_level.target(); }
            int    oversample() const { return m_os; }
            double smooth_ms() const { return m_smooth_ms; }
            double samplerate() const { return m_sr; }

            detector&          heterodyne() { return m_detector; }
            const detector&    heterodyne() const { return m_detector; }
            triode&            demodulator() { return m_demod; }
            const triode&      demodulator() const { return m_demod; }
            triode&            preamplifier() { return m_preamp; }
            const triode&      preamplifier() const { return m_preamp; }
            touche::key&       intensity_key() { return m_key; }
            const touche::key& intensity_key() const { return m_key; }

            // -- audio ---------------------------------------------------------------------------

            /// A source: no input. One sample of the instrument, minus its loudspeaker.
            double process() {
                if (!m_prepared) {
                    return 0.0;
                }
                const double drive = m_drive.tick();
                const double level = m_level.tick();
                m_demod.set_drive(std::max(k_min_drive_v, k_nominal_demod_v * drive));
                m_preamp.set_drive(std::max(k_min_drive_v, k_nominal_preamp_v * drive));
                m_power.set_drive(std::max(k_min_drive_v, k_nominal_power_v * drive));

                // The key's ramp is ticked ONCE per output sample whichever side of the chain it
                // is on, so its slew time means the same thing at every oversampling factor.
                const double key_gain = m_key.process(1.0);

                double y = 0.0;
                for (int j = 0; j < m_os; ++j) {
                    // The detector runs at the oversampled rate too — its cusp is a nonlinearity
                    // like any other, and it is the loudest source of aliasing in the chain.
                    const double raw = m_detector.process();
                    const double s   = core((m_key_where == key_before) ? raw * key_gain : raw);
                    y                = (m_os == 1) ? s : m_down.tick(s);
                }

                // The envelope carries a large DC term; the coupling that removes it in the
                // circuit is the stages' own highpass, and this catches whatever is left.
                const double d = y - m_dc_x1 + k_dc_r * m_dc_y1;
                m_dc_x1        = y;
                m_dc_y1        = (std::abs(d) < 1e-15) ? 0.0 : d;

                const double keyed = (m_key_where == key_after) ? m_dc_y1 * key_gain : m_dc_y1;
                return keyed * level;
            }

            /// Block form: the trivial loop over the scalar path.
            void process(double* out, size_t n) {
                for (size_t i = 0; i < n; ++i) {
                    out[i] = process();
                }
            }

          private:
            // The grid swing each stage sees at `drive` 1, chosen so the published operating
            // points are worked but not slammed — the fuzz.h lesson, applied before it bit.
            static constexpr double k_nominal_demod_v  = 0.9;
            static constexpr double k_nominal_preamp_v = 0.6;
            static constexpr double k_nominal_power_v  = 0.5;

            /// The nonlinear chain at whatever rate the caller is running.
            ///
            /// The demodulator's grid signal is the NEGATED envelope. That is grid-leak
            /// detection: the grid conducts on positive carrier half-cycles and charges the
            /// coupling capacitor negative, so a growing envelope drives the grid toward cutoff.
            /// The stage then inverts on the way out, which is why the demodulator as a whole is
            /// in phase with the envelope — but the tube's asymmetry has meanwhile been applied
            /// to the envelope's *underside*, and that is a different set of harmonics from
            /// applying it the other way up.
            double core(double x) {
                double y = m_preamp.process(m_polarity * m_demod.process(-x));
                if (m_power_on) {
                    y = m_power.process(y);
                }
                return y;
            }

            void configure() {
                const double osr = m_sr * m_os;
                m_detector.prepare(osr);
                m_demod.prepare(osr);
                m_demod.set_tube(tube_6c5);
                m_demod.set_operating_point(k_op_demod);
                m_preamp.prepare(osr);
                m_preamp.set_tube(tube_6c5);
                m_preamp.set_operating_point(k_op_preamp);
                m_power.prepare(osr);
                m_power.set_tube(tube_2a3);
                m_power.set_operating_point(k_op_power);
                if (m_os > 1) {
                    // Cut just below the original Nyquist, normalized to the oversampled rate.
                    // There is no anti-image filter to go with it: this object is a SOURCE, so
                    // nothing is zero-stuffed on the way up — the detector simply runs fast.
                    m_down.design(0.45 / static_cast<double>(m_os));
                }
            }

            long smooth_samples() const { return static_cast<long>(m_smooth_ms * 0.001 * m_sr); }

            double m_sr{48000.0};
            bool   m_prepared{false};
            double m_smooth_ms{k_default_smooth_ms};
            int    m_os{k_default_os};
            int    m_key_where{key_after};
            bool   m_power_on{false};
            double m_polarity{1.0};

            detector     m_detector;
            triode       m_demod, m_preamp, m_power;
            touche::key  m_key;
            butterworth8 m_down;
            double       m_dc_x1{0.0}, m_dc_y1{0.0};
            ramp         m_drive, m_level;
        };

    } // namespace ondes
} // namespace tap::tools
