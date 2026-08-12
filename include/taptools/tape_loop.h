/// @file
/// @brief      Shared tape-loop machinery for the Eno family (discreet.h, airport.h) — no Max/Min
///             dependency.
/// @details    The building blocks both tape kernels compose, factored the way swing_vca.h holds
///             the drum family's shared stages: a class with state is shared by include, a
///             few-line expression is copied with a citation.
///
///             - `reel` — a circular span of tape. Storage is bought once at prepare() (the
///               worst-case loop), and reads/writes wrap at a settable loop length, so the same
///               class serves a delay-line topology (loop length == capacity, an advancing write
///               head trailed by a read head — discreet.h) and a fixed-loop topology (loop length
///               set per piece, one free-running head that both plays and records — airport.h).
///               Fractional reads use the family's 4-point, 3rd-order Hermite.
///             - `wow_flutter` — the tape-transport speed error as a deterministic pair of sines
///               (slow/deep wow, fast/shallow flutter) returning a read-position offset in
///               samples. Periodic-only by design: the periodic term is the dominant one in the
///               tape-echo literature (Arnardottir, Abel, Smith, "A Digital Model of the Echoplex
///               Tape Delay", AES 125, 2008), and a deterministic transport means renders and
///               tests reproduce bit-exactly. Real capstan drift also has a stochastic term; that
///               is a documented non-goal here.
///             - `wear` — one pass of generation loss: a record/playback darkening one-pole
///               lowpass, then the shared soft saturator (vca::swing_shape — tanh(d*v)/d, exact
///               linear passthrough at drive 0), then the family's normalized DC blocker. This is
///               the load-bearing inversion of delay.h's stability story: swing_shape is bounded
///               by 1/drive for any drive > 0, so a regeneration loop built on wear is BIBO-
///               bounded even at unity regeneration — degradation is the stability mechanism,
///               where delay.h caps feedback below 1 instead. At drive 0 the saturator is exactly
///               linear and only the lowpass and DC blocker contract the loop; a sub-cutoff band
///               fed back at unity then sustains indefinitely — that is the Frippertronics
///               contract, stated, not hidden.
///             - `ramp` — the per-sample linear anti-zipper unit, same shape as delay.h, copied
///               here so both tape kernels share one without dragging in a whole delay kernel.
///
///             All processing is double-precision, per-sample, allocation-free after prepare().
/// @author     Timothy Place
// SPDX-License-Identifier: MIT
// Copyright 2026 Timothy Place.

#pragma once

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <vector>

#include "vca.h" // tap::tools::vca::swing_shape — the shared soft saturator

namespace tap::tools {
    namespace tape {

        constexpr double k_pi               = 3.14159265358979323846;
        constexpr long   k_min_loop_samples = 8;     // Hermite needs 4 support points; margin as delay_buffer
        constexpr double k_min_frac_delay   = 2.5;   // same floor, same reason as delay.h
        constexpr double k_dc_block_r       = 0.999; // in-loop DC blocker pole (~7 Hz corner @ 48k)
        // Normalized to unity peak gain so the blocker never amplifies the loop (grm_comb.h).
        constexpr double k_dc_block_norm       = (1.0 + k_dc_block_r) * 0.5;
        constexpr double k_darken_floor_hz     = 20.0; // wear cutoff range: the audible band
        constexpr double k_darken_ceil_hz      = 20000.0;
        constexpr double k_wow_rate_max_hz     = 5.0;  // transport wow lives below a few Hz
        constexpr double k_flutter_rate_max_hz = 30.0; // flutter sits above wow, below audio rate

        /// Per-sample linear parameter ramp — the anti-zipper unit every setter retargets.
        /// Same shape as delay.h's ramp.
        class ramp {
          public:
            void snap(double v) {
                m_current = m_target = v;
                m_inc                = 0.0;
                m_remaining          = 0;
            }

            void to(double tgt, long nsamples) {
                if (nsamples < 1 || tgt == m_current) {
                    snap(tgt);
                }
                else {
                    m_target    = tgt;
                    m_inc       = (tgt - m_current) / static_cast<double>(nsamples);
                    m_remaining = nsamples;
                }
            }

            double tick() {
                if (m_remaining > 0) {
                    m_current += m_inc;
                    if (--m_remaining == 0) {
                        m_current = m_target;
                    }
                }
                return m_current;
            }

            double current() const { return m_current; }
            double target() const { return m_target; }

          private:
            double m_current{0.0};
            double m_target{0.0};
            double m_inc{0.0};
            long   m_remaining{0};
        };

        /// One spool of tape: position-addressed circular storage with fractional Hermite reads.
        /// Positions may be any long/double — they wrap modulo the active loop length, so callers
        /// keep monotonically advancing heads and never do their own modular arithmetic.
        class reel {
          public:
            /// Buy the worst-case loop once. Loop length starts at full capacity. Not real-time-safe.
            void prepare(double sr, double max_seconds) {
                const double worst = std::ceil(std::max(0.0, max_seconds) * ((sr > 0.0) ? sr : 48000.0));
                m_tape.assign(static_cast<size_t>(std::max<double>(k_min_loop_samples, worst)), 0.0);
                m_loop_samples = static_cast<long>(m_tape.size());
            }

            /// Erase the tape; loop length and caller-held heads are untouched.
            void clear() { std::fill(m_tape.begin(), m_tape.end(), 0.0); }

            bool prepared() const { return !m_tape.empty(); }
            long capacity() const { return static_cast<long>(m_tape.size()); }
            long loop_samples() const { return m_loop_samples; }

            /// Set the active loop length, clamped to [k_min_loop_samples, capacity]. A splice:
            /// tape content is kept, positions simply re-wrap modulo the new length.
            void set_loop_samples(long n) { m_loop_samples = std::clamp(n, k_min_loop_samples, capacity()); }

            void write(long pos, double x) { m_tape[wrap(pos)] = x; }

            /// 4-point, 3rd-order Hermite at fractional position `pos` (wrapped modulo the loop
            /// length). Same read as delay.h / grm_comb.h.
            double read_hermite(double pos) const {
                const double fpos = std::floor(pos);
                const double frac = pos - fpos;
                const long   base = static_cast<long>(fpos);
                const double xm1  = m_tape[wrap(base - 1)];
                const double x0   = m_tape[wrap(base)];
                const double x1   = m_tape[wrap(base + 1)];
                const double x2   = m_tape[wrap(base + 2)];
                const double c    = (x1 - xm1) * 0.5;
                const double v    = x0 - x1;
                const double w    = c + v;
                const double a    = w + v + (x2 - x0) * 0.5;
                const double b    = w + a;
                return (((a * frac - b) * frac + c) * frac + x0);
            }

          private:
            // Same wrap as delay.h, against the loop length rather than the buffer size.
            size_t wrap(long i) const {
                return static_cast<size_t>(((i % m_loop_samples) + m_loop_samples) % m_loop_samples);
            }

            std::vector<double> m_tape;
            long                m_loop_samples{k_min_loop_samples};
        };

        /// Deterministic transport speed error: wow + flutter as two sines, returning a read-
        /// position offset in samples. Phases start at zero at prepare()/clear(), so two runs of
        /// the same settings are bit-exact.
        class wow_flutter {
          public:
            void prepare(double sr) {
                m_sr = (sr > 0.0) ? sr : 48000.0;
                clear();
            }

            void clear() { m_wow_phase = m_flutter_phase = 0.0; }

            /// Wow: excursion depth in ms of tape position, rate in Hz (clamped to the wow band).
            void set_wow(double depth_ms, double rate_hz) {
                m_wow_depth_ms = std::max(0.0, depth_ms);
                m_wow_rate_hz  = std::clamp(rate_hz, 0.0, k_wow_rate_max_hz);
            }

            /// Flutter: the faster, shallower partner (clamped to the flutter band).
            void set_flutter(double depth_ms, double rate_hz) {
                m_flutter_depth_ms = std::max(0.0, depth_ms);
                m_flutter_rate_hz  = std::clamp(rate_hz, 0.0, k_flutter_rate_max_hz);
            }

            double wow_depth_ms() const { return m_wow_depth_ms; }
            double wow_rate_hz() const { return m_wow_rate_hz; }
            double flutter_depth_ms() const { return m_flutter_depth_ms; }
            double flutter_rate_hz() const { return m_flutter_rate_hz; }

            /// Advance one sample; returns this sample's position offset in samples.
            double tick() {
                const double wow     = m_wow_depth_ms * 0.001 * m_sr * std::sin(2.0 * k_pi * m_wow_phase);
                const double flutter = m_flutter_depth_ms * 0.001 * m_sr * std::sin(2.0 * k_pi * m_flutter_phase);
                m_wow_phase += m_wow_rate_hz / m_sr;
                m_wow_phase -= std::floor(m_wow_phase);
                m_flutter_phase += m_flutter_rate_hz / m_sr;
                m_flutter_phase -= std::floor(m_flutter_phase);
                return wow + flutter;
            }

          private:
            double m_sr{48000.0};
            double m_wow_depth_ms{0.0};
            double m_wow_rate_hz{0.0};
            double m_flutter_depth_ms{0.0};
            double m_flutter_rate_hz{0.0};
            double m_wow_phase{0.0};
            double m_flutter_phase{0.0};
        };

        /// One pass of generation loss: darkening one-pole lowpass -> bounded soft saturation ->
        /// normalized DC blocker. The stabilizer of the regeneration loops built on it (see the
        /// file banner: bounded by 1/drive for any drive > 0, contractive above the cutoff).
        class wear {
          public:
            void prepare(double sr) {
                m_sr = (sr > 0.0) ? sr : 48000.0;
                update_coeff();
                clear();
            }

            void clear() {
                m_lp    = 0.0;
                m_dc_x1 = 0.0;
                m_dc_y1 = 0.0;
            }

            /// Record/playback darkening corner, clamped to the audible band. Exact one-pole
            /// coefficient, same map as grm_comb.h (tap.comb~'s cruder hz*2/sr was rejected there).
            void set_cutoff_hz(double hz) {
                m_cutoff_hz = std::clamp(hz, k_darken_floor_hz, k_darken_ceil_hz);
                update_coeff();
            }

            /// Saturation drive, >= 0. 0 is an exact linear passthrough (vca::swing_shape contract).
            void set_drive(double d) { m_drive = std::max(0.0, d); }

            double cutoff_hz() const { return m_cutoff_hz; }
            double drive() const { return m_drive; }

            double process(double x) {
                m_lp += m_lp_a * (x - m_lp);
                const double sat    = vca::swing_shape(m_lp, m_drive);
                const double dc_out = k_dc_block_norm * (sat - m_dc_x1) + k_dc_block_r * m_dc_y1;
                m_dc_x1             = sat;
                m_dc_y1             = anti_denormal(dc_out);
                return m_dc_y1;
            }

          private:
            void update_coeff() { m_lp_a = 1.0 - std::exp(-2.0 * k_pi * m_cutoff_hz / m_sr); }

            static double anti_denormal(double x) { return (std::abs(x) < 1e-15) ? 0.0 : x; } // same guard as tap.comb~

            double m_sr{48000.0};
            double m_cutoff_hz{k_darken_ceil_hz};
            double m_drive{0.0};
            double m_lp_a{1.0};
            double m_lp{0.0};
            double m_dc_x1{0.0};
            double m_dc_y1{0.0};
        };

    } // namespace tape
} // namespace tap::tools
