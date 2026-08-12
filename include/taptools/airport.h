/// @file
/// @brief      Portable incommensurate-loop-bank kernel for tap.airport~ — no Max/Min dependency.
/// @details    A recreation of the tape system behind "2/1" on Brian Eno's *Music for Airports*
///             (Ambient 1, EG, 1978), as described in Eno's own published accounts (the album's
///             liner notes and *A Year with Swollen Appendices*, Faber, 1996): a small number of
///             long tape loops — around seven, each holding one recorded phrase — of unequal,
///             incommensurate lengths, all free-running, so the phrases drift in and out of
///             coincidence and the piece never repeats on a human timescale. The composition IS
///             the phase system; the machine just keeps the loops turning.
///
///             Each of up to k_max_loops loops is a tape_loop.h reel with a single free-running
///             head that both plays and records. `record(loop, true)` punches the input onto that
///             loop's tape at wherever its head happens to be — the phase is NEVER reset, by
///             record or by any setter, because the free-run is the piece — and `record(loop,
///             false)` freezes the tape bit-exactly. Playback is read-before-write, so while
///             recording you hear the previous generation under the head. Per-loop level and
///             equal-power pan (exact endpoints, the delay.h multitap law) place each phrase in
///             the stereo field; a per-loop `darken` corner shades its playback tone (a
///             tape_loop.h wear stage with drive fixed at 0 — a real loop replays the *same*
///             magnetic imprint every pass, so there is no per-pass generation loss to model, and
///             pretending otherwise would be dishonest; at the band ceiling the stage is bypassed
///             entirely and playback is bit-transparent).
///
///             Geometry: prepare(sr, max_loop_seconds) buys k_max_loops worst-case reels — the
///             family's largest buy (8 loops x 30 s at 48 kHz is ~92 MB of double tape); size
///             max_loop_seconds to the piece. No later call allocates.
///
///             Honest limits:
///             - A length change is a splice: the tape keeps its content and the head re-wraps
///               modulo the new length. It can land mid-phrase and click — that is what splicing
///               tape does. It never rewinds.
///             - Recording starts at the head's current position, not at a downbeat. There is no
///               quantized punch-in; Eno's rig had none.
///             - The Hermite playback read spans two samples ahead of the record head, so for ~2
///               samples around the punch point one generation blends into the next.
///             - No wow/flutter here: the phasing engine of "2/1" is the incommensurate lengths,
///               not pitch drift. Run a loop's source through tap.discreet~ first if you want
///               tape breath.
///             - composite_period_seconds() is informational (long-long lcm of the active loop
///               lengths in samples; +inf when it overflows — with incommensurate lengths it is
///               astronomically long, which is the point).
///             - No dry path and no master gain: gain staging is the caller's job.
/// @author     Timothy Place
// SPDX-License-Identifier: MIT
// Copyright 2026 Timothy Place.

#pragma once

#include <algorithm>
#include <array>
#include <cmath>
#include <limits>

#include "tape_loop.h" // tap::tools::tape — reel / wear / ramp, the shared machinery

namespace tap::tools {
    namespace airport {

        constexpr int    k_max_loops           = 8;    // "2/1" used about seven; eight buys a spare
        constexpr double k_min_loop_seconds    = 0.5;  // shorter is a delay effect, not a phrase loop
        constexpr double k_default_max_seconds = 30.0; // worst case per loop (~92 MB total @ 48k)
        constexpr double k_default_smooth_ms   = 20.0; // anti-zipper ramp for level/pan/darken

        /// Up to eight free-running tape loops of unequal lengths, summed to stereo.
        class loop_bank {
          public:
            loop_bank() {
                for (auto& l : m_loops) {
                    l.level.snap(1.0);
                    l.darken_hz.snap(tape::k_darken_ceil_hz); // transparent until asked to shade
                }
            }

            // -- lifecycle -----------------------------------------------------------------------

            /// Buy k_max_loops reels for `max_loop_seconds` at `sr`, apply the stored lengths,
            /// snap all ramps, erase all tape, and rewind every head — a DSP restart is the one
            /// thing allowed to touch the phases. Not real-time-safe.
            void prepare(double sr, double max_loop_seconds = k_default_max_seconds) {
                m_sr = (sr > 0.0) ? sr : 48000.0;
                for (auto& l : m_loops) {
                    l.tape.prepare(m_sr, std::max(k_min_loop_seconds, max_loop_seconds));
                    l.tape.set_loop_samples(seconds_to_samples(l.length_seconds));
                    l.length_seconds = static_cast<double>(l.tape.loop_samples()) / m_sr;
                    l.shade.prepare(m_sr);
                    l.level.snap(l.level.target());
                    l.pan.snap(l.pan.target());
                    l.darken_hz.snap(l.darken_hz.target());
                    l.shade.set_cutoff_hz(l.darken_hz.current());
                }
                clear();
            }

            /// Erase every tape and rewind every head; parameters (lengths, levels, pans, darken,
            /// record gates) are untouched.
            void clear() {
                for (auto& l : m_loops) {
                    l.tape.clear();
                    l.shade.clear();
                    l.phase = 0.0;
                }
            }

            bool prepared() const { return m_loops[0].tape.prepared(); }

            // -- structure (instant; never touches a phase) --------------------------------------

            /// Number of active loops, clamped to [0, k_max_loops]. Newly activated loops come in
            /// at their stored settings, their heads wherever they last were.
            void set_loops(int count) { m_num_loops = std::clamp(count, 0, k_max_loops); }

            /// Per-loop length in seconds, clamped to [k_min_loop_seconds, the prepared max].
            /// A splice: content kept, head re-wraps modulo the new length, never rewinds.
            void set_length_seconds(int loop, double s) {
                if (!valid_loop(loop)) {
                    return;
                }
                loop_state& l    = m_loops[static_cast<size_t>(loop)];
                l.length_seconds = std::max(k_min_loop_seconds, s);
                if (l.tape.prepared()) {
                    l.tape.set_loop_samples(seconds_to_samples(l.length_seconds));
                    l.length_seconds = static_cast<double>(l.tape.loop_samples()) / m_sr;
                    const double n   = static_cast<double>(l.tape.loop_samples());
                    l.phase          = l.phase - std::floor(l.phase / n) * n; // re-wrap, no rewind
                }
            }

            /// Punch the input onto this loop's tape (true) or freeze it bit-exactly (false).
            /// Recording replaces — no overdub sum; Eno recorded each phrase once.
            void record(int loop, bool on) {
                if (valid_loop(loop)) {
                    m_loops[static_cast<size_t>(loop)].recording = on;
                }
            }

            // -- parameter targets (click-free; safe while audio runs) ---------------------------

            /// Per-loop linear playback level, slewed. Unclamped (negative flips polarity).
            void set_level(int loop, double lin) {
                if (valid_loop(loop)) {
                    m_loops[static_cast<size_t>(loop)].level.to(lin, smooth_samples());
                }
            }

            /// Per-loop equal-power pan, -1 (hard left) .. 1 (hard right), slewed. Endpoints are
            /// exact: a hard-panned loop is bitwise absent from the far bus (delay.h law).
            void set_pan(int loop, double pan) {
                if (valid_loop(loop)) {
                    m_loops[static_cast<size_t>(loop)].pan.to(std::clamp(pan, -1.0, 1.0), smooth_samples());
                }
            }

            /// Per-loop playback darkening corner in Hz, slewed. At the band ceiling (the
            /// default) the stage is bypassed and playback is bit-transparent.
            void set_darken_hz(int loop, double hz) {
                if (valid_loop(loop)) {
                    m_loops[static_cast<size_t>(loop)].darken_hz.to(
                        std::clamp(hz, tape::k_darken_floor_hz, tape::k_darken_ceil_hz), smooth_samples());
                }
            }

            void set_smooth_ms(double ms) { m_smooth_ms = std::max(0.0, ms); }

            // -- introspection -------------------------------------------------------------------

            int    loops() const { return m_num_loops; }
            double length_seconds(int loop) const {
                return valid_loop(loop) ? m_loops[static_cast<size_t>(loop)].length_seconds : 0.0;
            }
            bool recording(int loop) const { return valid_loop(loop) && m_loops[static_cast<size_t>(loop)].recording; }
            double level(int loop) const {
                return valid_loop(loop) ? m_loops[static_cast<size_t>(loop)].level.target() : 0.0;
            }
            double pan(int loop) const {
                return valid_loop(loop) ? m_loops[static_cast<size_t>(loop)].pan.target() : 0.0;
            }
            double darken_hz(int loop) const {
                return valid_loop(loop) ? m_loops[static_cast<size_t>(loop)].darken_hz.target() : 0.0;
            }
            double smooth_ms() const { return m_smooth_ms; }
            double samplerate() const { return m_sr; }
            double max_loop_seconds() const {
                return prepared() ? static_cast<double>(m_loops[0].tape.capacity()) / m_sr : 0.0;
            }

            /// This loop's head position as a fraction of its length, 0..1 — read-only, so tests
            /// can pin the promise that nothing but prepare()/clear() ever resets it.
            double phase(int loop) const {
                if (!valid_loop(loop) || !prepared()) {
                    return 0.0;
                }
                const loop_state& l = m_loops[static_cast<size_t>(loop)];
                return l.phase / static_cast<double>(l.tape.loop_samples());
            }

            /// Least common multiple of the active loop lengths, in seconds — how long until the
            /// whole system realigns. Informational; +inf on 64-bit overflow (incommensurate
            /// lengths overflow fast, which is the point of the piece).
            double composite_period_seconds() const {
                if (!prepared() || m_num_loops < 1) {
                    return 0.0;
                }
                long long acc = 1;
                for (int i = 0; i < m_num_loops; ++i) {
                    const long long n = static_cast<long long>(m_loops[static_cast<size_t>(i)].tape.loop_samples());
                    const long long g = gcd_ll(acc, n);
                    if (acc / g > std::numeric_limits<long long>::max() / n) {
                        return std::numeric_limits<double>::infinity();
                    }
                    acc = acc / g * n;
                }
                return static_cast<double>(acc) / m_sr;
            }

            // -- audio ---------------------------------------------------------------------------

            /// Sum the active loops to the stereo bus; punch `in` onto any recording loop.
            void process(double in, double& out_left, double& out_right) {
                out_left  = 0.0;
                out_right = 0.0;
                if (!prepared()) {
                    return;
                }
                for (int i = 0; i < m_num_loops; ++i) {
                    loop_state&  l        = m_loops[static_cast<size_t>(i)];
                    const double played   = l.tape.read_hermite(l.phase);
                    const double shade_hz = l.darken_hz.tick();
                    double       toned    = played;
                    if (shade_hz < tape::k_darken_ceil_hz) { // ceiling = bypass, bit-transparent
                        if (shade_hz != l.shade.cutoff_hz()) {
                            l.shade.set_cutoff_hz(shade_hz);
                        }
                        toned = l.shade.process(played);
                    }
                    const double g   = l.level.tick() * toned;
                    const double pan = l.pan.tick();
                    // Equal-power with exact endpoints — same law as delay.h multitap.
                    if (pan <= -1.0) {
                        out_left += g;
                    }
                    else if (pan >= 1.0) {
                        out_right += g;
                    }
                    else {
                        const double theta = (pan + 1.0) * 0.25 * tape::k_pi;
                        out_left += std::cos(theta) * g;
                        out_right += std::sin(theta) * g;
                    }
                    if (l.recording) { // read-before-write: you hear the old pass under the head
                        l.tape.write(static_cast<long>(std::floor(l.phase)), in);
                    }
                    l.phase += 1.0;
                    if (l.phase >= static_cast<double>(l.tape.loop_samples())) {
                        l.phase -= static_cast<double>(l.tape.loop_samples());
                    }
                }
            }

            /// Block form: the trivial loop over the scalar path.
            void process(const double* in, double* out_left, double* out_right, size_t n) {
                for (size_t i = 0; i < n; ++i) {
                    process(in[i], out_left[i], out_right[i]);
                }
            }

          private:
            struct loop_state {
                tape::reel tape;
                tape::wear shade;      // playback tone only: drive stays 0, bypassed at ceiling
                double     phase{0.0}; // samples into the loop; the piece lives here
                double     length_seconds{k_min_loop_seconds};
                bool       recording{false};
                tape::ramp level;     // linear
                tape::ramp pan;       // -1..1
                tape::ramp darken_hz; // Hz
            };

            static long long gcd_ll(long long a, long long b) {
                while (b != 0) {
                    const long long t = a % b;
                    a                 = b;
                    b                 = t;
                }
                return a;
            }

            bool valid_loop(int loop) const { return loop >= 0 && loop < k_max_loops; }
            long smooth_samples() const { return static_cast<long>(m_smooth_ms * 0.001 * m_sr); }
            long seconds_to_samples(double s) const { return static_cast<long>(std::ceil(s * m_sr)); }

            double                              m_sr{48000.0};
            double                              m_smooth_ms{k_default_smooth_ms};
            int                                 m_num_loops{0};
            std::array<loop_state, k_max_loops> m_loops;
        };

    } // namespace airport
} // namespace tap::tools
