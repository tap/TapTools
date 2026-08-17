/// @file
/// @brief      Portable driven-resonator kernels for the Ondes Martenot diffuseurs
///             (tap.metallique~, tap.palme~) — no Max/Min dependency.
/// @details    The Ondes Martenot does not have one loudspeaker, it has a rack of them, and the
///             player chooses which. Beyond the plain cabinet (the *principal*) Martenot built
///             resonating *diffuseurs* whose whole job is to colour the signal with a physical
///             body: the **métallique** (1944–45, patented 1947), a gong driven by a motor
///             transducer, and the **palme** (1949–50), an electromagnet driving twelve metal
///             strings stretched on a soundboard. Najnudel, Hélie, Roze & Boutin ("Simulation of
///             an ondes Martenot circuit", IEEE/ACM TASLP 28, 2020) name the diffuseur as the
///             stage that "converts the electrical waveform into sound and in turn modifies its
///             spectral content"; Wijnand, Boutin, Jossic & Maniguet (Forum Acusticum 2023)
///             describe the instruments themselves and measure the transducer.
///
///             **The correction that shapes this file: these are DRIVEN, not struck.** garden.h's
///             modal maths carries over intact — mode ratios, doublet splitting, per-mode decay —
///             but its strike envelopes do not. There is no `decay_env` here and no trigger. The
///             input signal excites the body continuously and the body rings at its own rates,
///             which is grm_comb.h's sustained-resonance situation rather than the chime's.
///
///             Signal order follows the instrument, and it matters: the electrical signal reaches
///             the *transducer* first, and the transducer's motion is what excites the body. So
///             the nonlinearity sits UPSTREAM of the resonator, not after it — drive the
///             transducer hard and you are driving a distorted waveform into a gong, which is a
///             different sound from distorting a gong.
///
///             Five classes, the family's parts-plus-thin-composition habit:
///             - `mode` — one driven resonant mode: the constant-peak-gain two-pole resonator
///               (zeros at ±1, b0 = (1−R²)/2; Steiglitz, and Smith, *Introduction to Digital
///               Filters*). Peak gain is 1 at any Q, so a bank of weighted modes is bounded by
///               the sum of its weights and needs no output limiter, and the zeros put exact
///               nulls at DC and Nyquist so no DC blocker is needed either.
///             - `plate` — the métallique's body: eight modes at the free circular plate's
///               transverse ratios, each split into a slowly beating doublet.
///             - `sympathetic` — one string: a damped, DC-blocked delay loop (the waveguide
///               idiom, same fractional Hermite read as delay.h / grm_comb.h), returning its
///               *ringing* rather than its through-signal so a caller can balance the two.
///             - `harp` — the palme's body: twelve sympathetic strings on one soundboard.
///             - `transducer` — the moving-iron driver.
///             - `metallique` / `palme` — a transducer, a body, a balance, a level. Nothing else.
///
///             **Provenance, and where recreation begins.** The instruments, their dates, their
///             excitation and their transducer type are from the peer-reviewed sources above. The
///             *modal data is not* — no ondes-specific measurement of either body exists in any
///             of them — so the ratios come from Fletcher & Rossing, *The Physics of Musical
///             Instruments*, 2nd ed.: the free circular plate's transverse modes for the gong
///             (Rayleigh's classical ratios at Poisson 0.3, the Chladni set: 1 : 1.730 : 2.328 :
///             3.910 : 4.110 : 6.300 : 6.710 : 7.340) and the harmonic series for the strings.
///             That makes both bodies **recreations of the general physics, not models of
///             Martenot's instruments**, and the difference is stated here rather than left for
///             the reader to discover.
///
///             The palme has **twelve** strings. Widely copied hobbyist build pages say
///             twenty-four (two banks of twelve); the peer-reviewed source says twelve, and this
///             file follows the peer-reviewed source. Their *tuning* is not published anywhere
///             found, so it is a parameter: chromatic across an octave by default (a string for
///             every pitch class, so the halo answers whatever you play) or the harmonic series
///             on the root (a drone that answers one key).
///
///             **The transducer.** Wijnand et al.'s point is that the early diffuseurs use a
///             moving-iron driver whose operating principle is *inherently* nonlinear —
///             Thiele–Small does not describe it — so a diffuseur modelled as a pure resonator
///             is missing a documented stage. What is modelled here is the principle, not a fit:
///             in a moving-iron motor the force follows the square of the gap flux, so with a
///             bias current I₀ and signal i the force carries a term in (I₀ + i)² whose residual
///             i² produces second-harmonic distortion growing with drive. Hence `asymmetry`,
///             a squared term, is the transducer's own even-harmonic signature. The bounded
///             saturator after it (vca::swing_shape, exactly linear at 0) is a **modelling
///             necessity, not a measured stage** — the squared law is expansive and something
///             has to bound it — and its coefficient is a knob, not a number from a paper.
///
///             Geometry: prepare(sr) buys the string loops once (twelve times sr/k_min_string_hz
///             doubles, ~115 kB at 48 kHz) and the plate allocates nothing at all. No later call
///             allocates; setters are allocation-free and safe while audio runs.
///
///             Honest limits:
///             - **The bodies are recreations.** See above. Nothing here was fitted to a
///               recording, a measurement, or a photograph of either diffuseur.
///             - **No transducer coefficient is measured.** The source establishes *that* the
///               moving-iron driver is nonlinear and that the linear loudspeaker model does not
///               apply to it. It does not hand over a curve, so `asymmetry` and `saturation` are
///               voiced by ear and labelled as such. A diffuseur run with both at 0 is a linear
///               resonator and is missing a real stage; that is a choice the caller may make.
///             - **No radiation model.** Neither body's directivity, cabinet, nor the soundboard's
///               own resonance is modelled. The output is the body's modal response, not a room.
///             - **The strings are ideal.** A real steel string is stiff and its partials stretch
///               sharp (Fletcher & Rossing's inharmonicity B); a plain delay loop's partials are
///               exactly harmonic. Dispersion is not modelled — `detune` scatters strings against
///               each other, which is a different thing and does not stand in for it.
///             - **A pitch sweep recomputes coefficients per sample.** Like grm_comb.h, the
///               derived values are recomputed on every sample while a ramp is moving and cached
///               once it settles — sixteen resonators or twelve loops of transcendentals, which
///               is real cost during a glide and none at rest.
///             - Mono in, mono out. A diffuseur is one cabinet; wrap in `mc.` for multichannel.
/// @author     Timothy Place
// SPDX-License-Identifier: MIT
// Copyright 2026 Timothy Place.

#pragma once

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>

#include "tape_loop.h" // tap::tools::tape — reel (the string loops), ramp, the DC-blocker constants
#include "vca.h"       // tap::tools::vca::swing_shape — the shared bounded saturator

namespace tap::tools {
    namespace diffuseur {

        constexpr double k_pi = 3.14159265358979323846;

        constexpr int k_plate_modes = 8;  // the free circular plate's first eight transverse modes
        constexpr int k_strings     = 12; // the palme's string count, per the peer-reviewed source

        // Transverse-mode ratios of a flat circular plate with a free edge (Fletcher & Rossing,
        // The Physics of Musical Instruments, 2nd ed., the plates chapter — Rayleigh's classical
        // values at Poisson's ratio 0.3, the Chladni set), referenced to the (2,0) mode. A gong is
        // not a flat plate — it is dished, with a rim, and often a nipple — so this is the general
        // physics of the family, deliberately not a claim about any particular métallique.
        constexpr std::array<double, k_plate_modes> k_plate_ratio = {1.000, 1.730, 2.328, 3.910,
                                                                     4.110, 6.300, 6.710, 7.340};
        // Mode weights sum to exactly 1, so with every resonator's peak gain equal to 1 the plate
        // is bounded by its input at every setting — the reason no output limiter is needed.
        constexpr std::array<double, k_plate_modes> k_plate_level = {0.30, 0.22, 0.16, 0.12, 0.08, 0.05, 0.04, 0.03};
        // Each mode is a doublet: real plates split their degenerate mode pairs by a few cents
        // (Fletcher & Rossing on doublets), so the body beats slowly instead of ringing like a
        // bank of lab sines. Fixed split — deterministic, no RNG.
        constexpr double k_doublet_cents = 1.5;
        // And the upper modes sit a few cents off the textbook ratios, drawn by a stateless hash
        // of the MODE INDEX only (metal_bank.h's per-index xorshift64* idiom). Keying on the index
        // rather than the pitch is deliberate: a pitch sweep must not make the scatter jump.
        constexpr double k_scatter_cents = 4.0;

        // Only the strings whose partials line up with the drive ring loudly, so the twelve
        // responses are largely incoherent and 1/sqrt(12) is the right sum normalization.
        constexpr double k_harp_norm = 0.28867513459481288;

        constexpr double k_min_string_hz = 40.0;    // sizes the string loops bought at prepare()
        constexpr double k_fb_max        = 0.9995;  // string loop gain cap: long, but strictly contractive
        constexpr double k_pole_max      = 0.99999; // resonator pole radius cap, same reason
        constexpr double k_min_delay     = 2.5;     // Hermite headroom, same floor as delay.h

        constexpr double k_min_t60      = 0.01;
        constexpr double k_max_t60      = 60.0;
        constexpr double k_min_pitch_hz = 20.0;
        constexpr double k_max_pitch_hz = 4000.0;
        constexpr double k_min_damp_hz  = 200.0;
        constexpr double k_max_damp_hz  = 20000.0;

        /// How the palme's twelve strings are tuned. Not a published detail — see the banner.
        enum tuning_index : int {
            tuning_chromatic = 0, // twelve semitones from the root: a string for every pitch class
            tuning_harmonic,      // partials 1..12 of the root: a drone that answers one key
            k_num_tunings
        };

        constexpr double k_default_pitch_hz  = 180.0; // the métallique's (2,0) mode
        constexpr double k_default_root_hz   = 110.0; // the palme's lowest string
        constexpr double k_default_decay_s   = 6.0;
        constexpr double k_default_tilt      = 1.0; // upper modes die this power of their ratio faster
        constexpr double k_default_bright    = 0.7; // upper-mode weight
        constexpr double k_default_damp_hz   = 4000.0;
        constexpr double k_default_detune_c  = 6.0; // per-string scatter depth, cents
        constexpr double k_default_drive     = 1.0;
        constexpr double k_default_asymmetry = 0.15; // the moving-iron squared term
        constexpr double k_default_sat       = 0.5;  // the bounding saturator; 0 is exactly linear
        constexpr double k_default_mix       = 100.0;
        constexpr double k_default_level     = 1.0;
        constexpr double k_default_smooth_ms = 20.0;

        /// Stateless draw in [-1, 1) keyed by an index — the metal_bank.h / garden.h per-index
        /// xorshift64* idiom. Same body in every instance, forever, and no generator state.
        inline double index_unit(uint64_t index) {
            uint64_t s = (index + 1) * 0x9e3779b97f4a7c15ULL;
            s ^= s >> 12;
            s ^= s << 25;
            s ^= s >> 27;
            const double u = static_cast<double>((s * 0x2545f4914f6cdd1dULL) >> 11) / 9007199254740992.0; // [0, 1)
            return 2.0 * u - 1.0;
        }

        inline double anti_denormal(double x) {
            return (std::abs(x) < 1e-15) ? 0.0 : x; // same guard as tap.comb~
        }

        /// One driven resonant mode: the constant-peak-gain two-pole resonator,
        ///
        ///     H(z) = b0 (1 - z^-2) / (1 - 2R cos(w) z^-1 + R^2 z^-2),   b0 = (1 - R^2) / 2
        ///
        /// (Steiglitz; Smith, *Introduction to Digital Filters*, the two-pole resonator section).
        /// The b0 normalization makes the peak magnitude 1 for any pole radius, which is what lets
        /// a bank of these be bounded by the sum of its weights — pinned by test. The zeros at
        /// z = ±1 put exact nulls at DC and Nyquist, so a driven bank cannot accumulate DC and
        /// needs no blocker.
        ///
        /// Ring time is specified as a T60 in seconds and converted the same way grm_comb.h does:
        /// R = 10^(-3 / (T60 · sr)), one thousandth of the amplitude after T60 seconds.
        class mode {
          public:
            void prepare(double sr) {
                m_sr = (sr > 0.0) ? sr : 48000.0;
                set(m_hz, m_t60);
                clear();
            }

            void clear() { m_x1 = m_x2 = m_y1 = m_y2 = 0.0; }

            /// Resonant frequency in Hz and ring time in seconds. Allocation-free; the caller
            /// decides how often to call it (the machines recompute per sample while ramping).
            void set(double hz, double t60) {
                m_hz            = hz;
                m_t60           = t60;
                const double f  = std::clamp(hz, 1.0, 0.49 * m_sr);
                const double w  = 2.0 * k_pi * f / m_sr;
                const double r  = std::min(std::pow(10.0, -3.0 / (std::max(t60, 1e-4) * m_sr)), k_pole_max);
                const double rr = r * r;
                m_a1            = 2.0 * r * std::cos(w);
                m_a2            = -rr;
                m_b0            = 0.5 * (1.0 - rr);
            }

            double frequency() const { return m_hz; }
            double t60() const { return m_t60; }

            double process(double x) {
                const double y = m_b0 * (x - m_x2) + m_a1 * m_y1 + m_a2 * m_y2;
                m_x2           = m_x1;
                m_x1           = x;
                m_y2           = m_y1;
                m_y1           = anti_denormal(y);
                return m_y1;
            }

          private:
            double m_sr{48000.0};
            double m_hz{k_default_pitch_hz};
            double m_t60{k_default_decay_s};
            double m_b0{0.0}, m_a1{0.0}, m_a2{0.0};
            double m_x1{0.0}, m_x2{0.0}, m_y1{0.0}, m_y2{0.0};
        };

        /// The métallique's body: eight plate modes, each a beating doublet, driven continuously.
        /// Weighted by `brightness` the way garden.h weights a chime's upper partials (b, b², b³…
        /// so softening kills the highest first), decaying faster with frequency by `tilt`, and
        /// silent above the audio band rather than aliasing.
        class plate {
          public:
            void prepare(double sr) {
                m_sr = (sr > 0.0) ? sr : 48000.0;
                for (auto& m : m_mode) {
                    m.prepare(m_sr);
                }
                retune();
                clear();
            }

            void clear() {
                for (auto& m : m_mode) {
                    m.clear();
                }
            }

            /// Frequency of the (2,0) mode — the body's perceived pitch.
            void set_pitch_hz(double hz) {
                m_pitch = std::clamp(hz, k_min_pitch_hz, k_max_pitch_hz);
                retune();
            }

            /// Ring time of the fundamental, in seconds.
            void set_decay(double t60) {
                m_t60 = std::clamp(t60, k_min_t60, k_max_t60);
                retune();
            }

            /// How much faster the upper modes die: T60_n = T60 / ratio_n^tilt. Radiation damping
            /// grows roughly with f² for a struck bar (garden.h uses that), but a driven gong's
            /// shimmer lives in its upper modes, so this is exposed rather than fixed and defaults
            /// to 1 — a recreation choice, not a measurement.
            void set_tilt(double t) {
                m_tilt = std::clamp(t, 0.0, 3.0);
                retune();
            }

            /// Upper-mode weight, [0, 1]: 1 is the full published weight table, 0 leaves the
            /// fundamental doublet alone.
            void set_brightness(double b) {
                m_bright = std::clamp(b, 0.0, 1.0);
                retune();
            }

            double pitch_hz() const { return m_pitch; }
            double decay() const { return m_t60; }
            double tilt() const { return m_tilt; }
            double brightness() const { return m_bright; }
            double samplerate() const { return m_sr; }

            /// The gain this mode's doublet is contributing (both halves together) — what the
            /// boundedness argument sums, so a test can check the sum rather than trust it.
            double mode_level(int m) const {
                return (m >= 0 && m < k_plate_modes) ? 2.0 * m_level[static_cast<size_t>(m)] : 0.0;
            }
            double mode_hz(int m) const {
                return (m >= 0 && m < k_plate_modes) ? m_mode[static_cast<size_t>(2 * m)].frequency() : 0.0;
            }

            double process(double x) {
                double sum = 0.0;
                for (int m = 0; m < k_plate_modes; ++m) {
                    const size_t i = static_cast<size_t>(m);
                    const double g = m_level[i];
                    sum += g
                           * (m_mode[static_cast<size_t>(2 * m)].process(x)
                              + m_mode[static_cast<size_t>(2 * m + 1)].process(x));
                }
                return sum;
            }

          private:
            /// Recompute every doublet from pitch / decay / tilt / brightness.
            void retune() {
                const double split = std::exp2(k_doublet_cents / 2400.0); // half the split, up and down
                double       shine = 1.0;                                 // 1, b, b², b³ … per mode
                for (int m = 0; m < k_plate_modes; ++m) {
                    const size_t i = static_cast<size_t>(m);
                    const double ratio =
                        k_plate_ratio[i]
                        * ((m > 0) ? std::exp2(k_scatter_cents * index_unit(static_cast<uint64_t>(m)) / 1200.0) : 1.0);
                    const double hz  = m_pitch * ratio;
                    const double t60 = std::max(m_t60 / std::pow(k_plate_ratio[i], m_tilt), k_min_t60);
                    m_mode[static_cast<size_t>(2 * m)].set(hz * split, t60);
                    m_mode[static_cast<size_t>(2 * m + 1)].set(hz / split, t60);
                    // Half the published weight into each half of the doublet, and nothing at all
                    // above the band — a mode past Nyquist would otherwise fold.
                    m_level[i] = (hz < 0.45 * m_sr) ? 0.5 * k_plate_level[i] * shine : 0.0;
                    shine *= m_bright;
                }
            }

            double                              m_sr{48000.0};
            double                              m_pitch{k_default_pitch_hz};
            double                              m_t60{k_default_decay_s};
            double                              m_tilt{k_default_tilt};
            double                              m_bright{k_default_bright};
            std::array<double, k_plate_modes>   m_level{};
            std::array<mode, 2 * k_plate_modes> m_mode;
        };

        /// One sympathetic string: a damped, DC-blocked delay loop driven by the input, returning
        /// the loop's *own ringing* rather than its through-signal — so the caller balances the
        /// drive against the resonance instead of getting them pre-mixed.
        ///
        /// The loop gain is derived from a T60 the way grm_comb.h derives it, and compensated for
        /// what the damping lowpass and the DC blocker take out at the fundamental, so the stated
        /// ring time is the ring time you get at any damping setting. The k_fb_max cap keeps the
        /// loop strictly contractive whatever the compensation asks for.
        class sympathetic {
          public:
            /// Buy the longest loop once (the period of k_min_string_hz plus Hermite margin).
            void prepare(double sr) {
                m_sr = (sr > 0.0) ? sr : 48000.0;
                m_reel.prepare(m_sr, 1.0 / k_min_string_hz + 0.001);
                set(m_hz, m_t60, m_damp_hz);
                clear();
            }

            void clear() {
                m_reel.clear();
                m_write = 0;
                m_lp    = 0.0;
                m_dc_x1 = m_dc_y1 = 0.0;
            }

            bool prepared() const { return m_reel.prepared(); }

            /// Pitch (Hz), ring time (s), damping corner (Hz). Allocation-free.
            void set(double hz, double t60, double damp_hz) {
                m_hz      = hz;
                m_t60     = t60;
                m_damp_hz = std::clamp(damp_hz, k_min_damp_hz, k_max_damp_hz);
                if (!prepared()) {
                    return;
                }
                const double f = std::clamp(hz, k_min_string_hz, 0.49 * m_sr);
                m_delay        = std::clamp(m_sr / f, k_min_delay, static_cast<double>(m_reel.capacity() - 4));

                const double w = 2.0 * k_pi * f / m_sr;
                m_lp_a         = 1.0 - std::exp(-2.0 * k_pi * m_damp_hz / m_sr);

                // What one trip round the loop loses to the two in-loop filters at the fundamental.
                const double p     = 1.0 - m_lp_a;
                const double lp_g  = m_lp_a / std::sqrt(std::max(1.0 - 2.0 * p * std::cos(w) + p * p, 1e-30));
                const double dcnum = 2.0 - 2.0 * std::cos(w);
                const double dcden =
                    1.0 - 2.0 * tape::k_dc_block_r * std::cos(w) + tape::k_dc_block_r * tape::k_dc_block_r;
                const double dc_g = tape::k_dc_block_norm * std::sqrt(dcnum / std::max(dcden, 1e-30));

                const double want = std::pow(10.0, -3.0 * m_delay / (std::max(m_t60, k_min_t60) * m_sr));
                m_fb              = std::min(want / std::max(lp_g * dc_g, 1e-3), k_fb_max);
            }

            double frequency() const { return m_hz; }
            double feedback() const { return m_fb; }
            double delay_samples() const { return m_delay; }

            /// Drive the string one sample and return what it is ringing with.
            double process(double x) {
                if (!prepared()) {
                    return 0.0;
                }
                const double delayed = m_reel.read_hermite(static_cast<double>(m_write) - m_delay);
                m_lp += m_lp_a * (delayed - m_lp);
                const double dc = tape::k_dc_block_norm * (m_lp - m_dc_x1) + tape::k_dc_block_r * m_dc_y1;
                m_dc_x1         = m_lp;
                m_dc_y1         = anti_denormal(dc);

                const double ring = m_fb * m_dc_y1;
                m_reel.write(m_write, anti_denormal(x + ring));
                if (++m_write >= m_reel.capacity()) {
                    m_write = 0;
                }
                return ring;
            }

          private:
            double     m_sr{48000.0};
            double     m_hz{k_default_root_hz};
            double     m_t60{k_default_decay_s};
            double     m_damp_hz{k_default_damp_hz};
            double     m_delay{100.0};
            double     m_fb{0.0};
            double     m_lp_a{1.0};
            double     m_lp{0.0};
            double     m_dc_x1{0.0}, m_dc_y1{0.0};
            long       m_write{0};
            tape::reel m_reel;
        };

        /// The palme's body: twelve sympathetic strings on one soundboard. Only the strings whose
        /// partials line up with the drive ring loudly, which is the halo the instrument is famous
        /// for; the sum is normalized by sqrt(12) because those responses are largely incoherent.
        class harp {
          public:
            void prepare(double sr) {
                m_sr = (sr > 0.0) ? sr : 48000.0;
                for (auto& s : m_string) {
                    s.prepare(m_sr);
                }
                retune();
                clear();
            }

            void clear() {
                for (auto& s : m_string) {
                    s.clear();
                }
            }

            bool prepared() const { return m_string[0].prepared(); }

            /// Pitch of the lowest string, in Hz.
            void set_root_hz(double hz) {
                m_root = std::clamp(hz, k_min_pitch_hz, k_max_pitch_hz);
                retune();
            }

            /// How the twelve are laid out (tuning_index) — see the banner: not a published detail.
            void set_tuning(int t) {
                m_tuning = std::clamp(t, 0, k_num_tunings - 1);
                retune();
            }

            void set_decay(double t60) {
                m_t60 = std::clamp(t60, k_min_t60, k_max_t60);
                retune();
            }

            /// In-loop damping corner in Hz: how quickly a string loses its upper partials.
            void set_damping(double hz) {
                m_damp_hz = std::clamp(hz, k_min_damp_hz, k_max_damp_hz);
                retune();
            }

            /// Depth in cents of the fixed per-string scatter — no two strings on a real soundboard
            /// are in perfect relation. Deterministic (index-keyed hash), so the harp is the same
            /// harp in every instance.
            void set_detune(double cents) {
                m_detune = std::clamp(cents, 0.0, 50.0);
                retune();
            }

            double root_hz() const { return m_root; }
            int    tuning() const { return m_tuning; }
            double decay() const { return m_t60; }
            double damping() const { return m_damp_hz; }
            double detune() const { return m_detune; }
            double string_hz(int i) const {
                return (i >= 0 && i < k_strings) ? m_string[static_cast<size_t>(i)].frequency() : 0.0;
            }
            /// The loop gain a string settled on. Worth exposing rather than deriving twice: it is
            /// where the damping-versus-ring-time cap becomes visible (see the header's limits).
            double string_feedback(int i) const {
                return (i >= 0 && i < k_strings) ? m_string[static_cast<size_t>(i)].feedback() : 0.0;
            }
            double samplerate() const { return m_sr; }

            double process(double x) {
                double sum = 0.0;
                for (auto& s : m_string) {
                    sum += s.process(x);
                }
                return sum * k_harp_norm;
            }

          private:
            void retune() {
                for (int i = 0; i < k_strings; ++i) {
                    const double step  = (m_tuning == tuning_harmonic) ? static_cast<double>(i + 1)
                                                                       : std::exp2(static_cast<double>(i) / 12.0);
                    const double drift = std::exp2(m_detune * index_unit(static_cast<uint64_t>(100 + i)) / 1200.0);
                    m_string[static_cast<size_t>(i)].set(m_root * step * drift, m_t60, m_damp_hz);
                }
            }

            double                             m_sr{48000.0};
            double                             m_root{k_default_root_hz};
            int                                m_tuning{tuning_chromatic};
            double                             m_t60{k_default_decay_s};
            double                             m_damp_hz{k_default_damp_hz};
            double                             m_detune{k_default_detune_c};
            std::array<sympathetic, k_strings> m_string;
        };

        /// The moving-iron driver — the stage a diffuseur modelled as a pure resonator is missing.
        ///
        /// `drive` gains the signal into the motor. `asymmetry` is the moving-iron principle: force
        /// follows the square of the gap flux, so with a bias current the residual squared term
        /// puts second-harmonic distortion on the output in proportion to level. `saturation` is
        /// the shared bounded soft clipper (vca::swing_shape, exactly linear at 0), which is here
        /// because the squared law is expansive and something must bound it — a modelling
        /// necessity, not a measured stage. A DC blocker follows, because a squared term rectifies.
        ///
        /// Neither nonlinear coefficient is fitted to anything; see the file banner.
        ///
        /// The output is bounded by 2/`saturation` rather than the saturator's own 1/`saturation`:
        /// a hard-driven squared law is a nearly-constant positive waveform with brief negative
        /// excursions, and removing that large DC offset doubles the worst-case swing. Measured
        /// and pinned by test, because 1/saturation is the number you would expect and it is wrong.
        class transducer {
          public:
            void prepare(double sr) {
                m_sr = (sr > 0.0) ? sr : 48000.0;
                clear();
            }

            void clear() {
                m_dc_x1 = 0.0;
                m_dc_y1 = 0.0;
            }

            void set_drive(double lin) { m_drive = std::max(0.0, lin); }
            void set_asymmetry(double a) { m_asym = std::clamp(a, 0.0, 1.0); }
            void set_saturation(double s) { m_sat = std::max(0.0, s); }

            double drive() const { return m_drive; }
            double asymmetry() const { return m_asym; }
            double saturation() const { return m_sat; }

            /// With drive 1, asymmetry 0 and saturation 0 this is a bitwise passthrough apart from
            /// the DC blocker — pinned by test, and the reason a caller can switch the stage off.
            double process(double x) {
                const double u  = m_drive * x;
                const double v  = u + m_asym * u * u; // (I0 + i)^2 leaves a squared term
                const double s  = vca::swing_shape(v, m_sat);
                const double dc = tape::k_dc_block_norm * (s - m_dc_x1) + tape::k_dc_block_r * m_dc_y1;
                m_dc_x1         = s;
                m_dc_y1         = anti_denormal(dc);
                return m_dc_y1;
            }

          private:
            double m_sr{48000.0};
            double m_drive{k_default_drive};
            double m_asym{k_default_asymmetry};
            double m_sat{k_default_sat};
            double m_dc_x1{0.0}, m_dc_y1{0.0};
        };

        /// Shared plumbing for both cabinets: the transducer, the equal-power balance against the
        /// dry signal, the output level, and the anti-zipper ramps they ride. The body is the only
        /// thing the two machines do not share, so it is the only thing they define themselves.
        class cabinet {
          public:
            cabinet() {
                m_mix.snap(k_default_mix);
                m_level.snap(k_default_level);
                m_drive.snap(k_default_drive);
                m_asym.snap(k_default_asymmetry);
                m_sat.snap(k_default_sat);
            }

            /// Transducer drive, linear and slewed.
            void set_drive(double lin) { m_drive.to(std::max(0.0, lin), smooth_samples()); }

            /// The moving-iron squared term, [0, 1], slewed.
            void set_asymmetry(double a) { m_asym.to(std::clamp(a, 0.0, 1.0), smooth_samples()); }

            /// The bounding saturator's drive; 0 is exactly linear (vca::swing_shape contract).
            void set_saturation(double s) { m_sat.to(std::max(0.0, s), smooth_samples()); }

            /// Balance between the dry signal and the diffuseur, 0..100, equal-power.
            void set_mix(double pct) { m_mix.to(std::clamp(pct, 0.0, 100.0), smooth_samples()); }

            /// Output level, linear.
            void set_level(double lin) { m_level.to(lin, smooth_samples()); }

            void set_smooth_ms(double ms) { m_smooth_ms = std::max(0.0, ms); }

            double drive() const { return m_drive.target(); }
            double asymmetry() const { return m_asym.target(); }
            double saturation() const { return m_sat.target(); }
            double mix() const { return m_mix.target(); }
            double level() const { return m_level.target(); }
            double smooth_ms() const { return m_smooth_ms; }
            double samplerate() const { return m_sr; }

            /// Direct access to the driver, so a caller (or a null test) can reach the same stage
            /// the machine drives.
            transducer&       driver() { return m_driver; }
            const transducer& driver() const { return m_driver; }

            bool prepared() const { return m_prepared; }

          protected:
            void prepare_common(double sr) {
                m_sr       = (sr > 0.0) ? sr : 48000.0;
                m_prepared = true;
                m_driver.prepare(m_sr);
                m_mix.snap(m_mix.target());
                m_level.snap(m_level.target());
                m_drive.snap(m_drive.target());
                m_asym.snap(m_asym.target());
                m_sat.snap(m_sat.target());
            }

            /// Tick the transducer ramps and run the driver — the stage that comes BEFORE the body.
            double drive_stage(double in) {
                m_driver.set_drive(m_drive.tick());
                m_driver.set_asymmetry(m_asym.tick());
                m_driver.set_saturation(m_sat.tick());
                return m_driver.process(in);
            }

            /// Balance the body's output against the dry input and apply the level. The endpoints
            /// are exact rather than cos(pi/2)-approximate, so a fully wet cabinet really is the
            /// cabinet and a fully dry one really is the input (the stammer.h rule) — which is
            /// what lets the wiring null test compare bitwise.
            double blend(double dry, double wet) {
                const double pct = m_mix.tick();
                const double lin = m_level.tick();
                if (pct >= 100.0) {
                    return wet * lin;
                }
                if (pct <= 0.0) {
                    return dry * lin;
                }
                const double theta = pct * 0.01 * (k_pi * 0.5);
                return (std::cos(theta) * dry + std::sin(theta) * wet) * lin;
            }

            long smooth_samples() const { return static_cast<long>(m_smooth_ms * 0.001 * m_sr); }

            double     m_sr{48000.0};
            bool       m_prepared{false};
            double     m_smooth_ms{k_default_smooth_ms};
            transducer m_driver;
            tape::ramp m_drive, m_asym, m_sat, m_mix, m_level;
        };

        /// **tap.metallique~** — the motor-driven gong. A transducer into a plate, balanced against
        /// the dry signal. The body's parameters are not ramped: they retune sixteen resonators, so
        /// they are set-and-hold rather than sweepable (see the header's limits).
        class metallique : public cabinet {
          public:
            void prepare(double sr) {
                prepare_common(sr);
                m_plate.prepare(m_sr);
                clear();
            }

            void clear() {
                m_plate.clear();
                m_driver.clear();
            }

            void set_pitch_hz(double hz) { m_plate.set_pitch_hz(hz); }
            void set_decay(double t60) { m_plate.set_decay(t60); }
            void set_tilt(double t) { m_plate.set_tilt(t); }
            void set_brightness(double b) { m_plate.set_brightness(b); }

            double pitch_hz() const { return m_plate.pitch_hz(); }
            double decay() const { return m_plate.decay(); }
            double tilt() const { return m_plate.tilt(); }
            double brightness() const { return m_plate.brightness(); }

            plate&       body() { return m_plate; }
            const plate& body() const { return m_plate; }

            double process(double in) {
                if (!prepared()) {
                    return in;
                }
                return blend(in, m_plate.process(drive_stage(in)));
            }

            void process(const double* in, double* out, size_t n) {
                for (size_t i = 0; i < n; ++i) {
                    out[i] = process(in[i]);
                }
            }

          private:
            plate m_plate;
        };

        /// **tap.palme~** — the electromagnet and its twelve strings. A transducer into a harp,
        /// balanced against the dry signal. Run a guitar through it.
        class palme : public cabinet {
          public:
            void prepare(double sr) {
                prepare_common(sr);
                m_harp.prepare(m_sr);
                clear();
            }

            void clear() {
                m_harp.clear();
                m_driver.clear();
            }

            void set_root_hz(double hz) { m_harp.set_root_hz(hz); }
            void set_tuning(int t) { m_harp.set_tuning(t); }
            void set_decay(double t60) { m_harp.set_decay(t60); }
            void set_damping(double hz) { m_harp.set_damping(hz); }
            void set_detune(double cents) { m_harp.set_detune(cents); }

            double root_hz() const { return m_harp.root_hz(); }
            int    tuning() const { return m_harp.tuning(); }
            double decay() const { return m_harp.decay(); }
            double damping() const { return m_harp.damping(); }
            double detune() const { return m_harp.detune(); }

            harp&       body() { return m_harp; }
            const harp& body() const { return m_harp; }

            double process(double in) {
                if (!prepared()) {
                    return in;
                }
                return blend(in, m_harp.process(drive_stage(in)));
            }

            void process(const double* in, double* out, size_t n) {
                for (size_t i = 0; i < n; ++i) {
                    out[i] = process(in[i]);
                }
            }

          private:
            harp m_harp;
        };

    } // namespace diffuseur
} // namespace tap::tools
