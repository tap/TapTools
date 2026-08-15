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
///             Two classes, because the piece is a system built from a trivial part:
///             - `loop` — ONE free-running reel: a tape_loop.h spool with a single head that both
///               plays and records, its playback shaded, leveled, and placed on the stereo field.
///               Everything a lane does lives here, so a lane is usable on its own (it is what
///               tap.reel~ wraps) and the bank is not the only way to reach it.
///             - `loop_bank` — up to k_max_loops of those and nothing else: it owns the count, the
///               shared smoothing time, and the composite-period arithmetic, and its process() is
///               a sum over the lanes. Seven lanes patched into a sum ARE the bank; that identity
///               is the point of the split, and it is what the null test pins.
///
///             `record(loop, true)` punches the input onto that loop's tape at wherever its head
///             happens to be — the phase is NEVER reset, by record or by any setter, because the
///             free-run is the piece — and `record(loop, false)` freezes the tape bit-exactly.
///             Playback is read-before-write, so while recording you hear the previous generation
///             under the head. Per-loop level and equal-power pan (exact endpoints, the delay.h
///             multitap law) place each phrase in the stereo field; a per-loop `darken` corner
///             shades its playback tone (a tape_loop.h wear stage with drive fixed at 0 — a real
///             loop replays the *same* magnetic imprint every pass, so there is no per-pass
///             generation loss to model, and pretending otherwise would be dishonest; at the band
///             ceiling the stage is bypassed entirely and playback is bit-transparent).
///
///             Geometry: prepare(sr, max_loop_seconds) buys the worst-case reel — one per lane, so
///             the bank makes the family's largest buy (8 loops x 30 s at 48 kHz is ~92 MB of
///             double tape) while a standalone lane buys exactly one. Size max_loop_seconds to the
///             piece. No later call allocates.
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
#include <cstddef>
#include <limits>

#include "tape_loop.h" // tap::tools::tape — reel / wear / ramp, the shared machinery

namespace tap::tools {
    namespace airport {

        constexpr int    k_max_loops           = 8;    // "2/1" used about seven; eight buys a spare
        constexpr double k_min_loop_seconds    = 0.5;  // shorter is a delay effect, not a phrase loop
        constexpr double k_default_max_seconds = 30.0; // worst case per loop (~92 MB total @ 48k)
        constexpr double k_default_smooth_ms   = 20.0; // anti-zipper ramp for level/pan/darken

        /// One free-running tape loop: a single head that both plays and records, its playback
        /// shaded, leveled, and panned to a seat. A `loop_bank` is an array of these and nothing
        /// more; one on its own is a complete instrument (tap.reel~), and the head is just as
        /// sacred alone as it is in the bank — nothing but prepare()/clear() ever resets it.
        class loop {
          public:
            loop() {
                m_level.snap(1.0);
                m_darken_hz.snap(tape::k_darken_ceil_hz); // transparent until asked to shade
            }

            // -- lifecycle -----------------------------------------------------------------------

            /// Buy the reel for `max_loop_seconds` at `sr`, apply the stored length, snap the
            /// ramps, erase the tape, and rewind the head — a DSP restart is the one thing allowed
            /// to touch the phase. Not real-time-safe.
            void prepare(double sr, double max_loop_seconds = k_default_max_seconds) {
                m_sr = (sr > 0.0) ? sr : 48000.0;
                m_tape.prepare(m_sr, std::max(k_min_loop_seconds, max_loop_seconds));
                m_tape.set_loop_samples(seconds_to_samples(m_length_seconds));
                m_length_seconds = static_cast<double>(m_tape.loop_samples()) / m_sr;
                m_shade.prepare(m_sr);
                m_level.snap(m_level.target());
                m_pan.snap(m_pan.target());
                m_darken_hz.snap(m_darken_hz.target());
                m_shade.set_cutoff_hz(m_darken_hz.current());
                clear();
            }

            /// Erase the tape and rewind the head; parameters (length, level, pan, darken, the
            /// record gate) are untouched.
            void clear() {
                m_tape.clear();
                m_shade.clear();
                m_phase = 0.0;
            }

            bool prepared() const { return m_tape.prepared(); }

            // -- structure (instant; never touches the phase) ------------------------------------

            /// Length in seconds, clamped to [k_min_loop_seconds, the prepared max]. A splice:
            /// content kept, head re-wraps modulo the new length, never rewinds.
            void set_length_seconds(double s) {
                m_length_seconds = std::max(k_min_loop_seconds, s);
                if (m_tape.prepared()) {
                    m_tape.set_loop_samples(seconds_to_samples(m_length_seconds));
                    m_length_seconds = static_cast<double>(m_tape.loop_samples()) / m_sr;
                    const double n   = static_cast<double>(m_tape.loop_samples());
                    m_phase          = m_phase - std::floor(m_phase / n) * n; // re-wrap, no rewind
                }
            }

            /// Punch the input onto the tape (true) or freeze it bit-exactly (false). Recording
            /// replaces — no overdub sum; Eno recorded each phrase once.
            void record(bool on) { m_recording = on; }

            // -- parameter targets (click-free; safe while audio runs) ---------------------------

            /// Linear playback level, slewed. Unclamped (negative flips polarity).
            void set_level(double lin) { m_level.to(lin, smooth_samples()); }

            /// Equal-power pan, -1 (hard left) .. 1 (hard right), slewed. Endpoints are exact: a
            /// hard-panned loop is bitwise absent from the far bus (delay.h law).
            void set_pan(double pan) { m_pan.to(std::clamp(pan, -1.0, 1.0), smooth_samples()); }

            /// Playback darkening corner in Hz, slewed. At the band ceiling (the default) the
            /// stage is bypassed and playback is bit-transparent.
            void set_darken_hz(double hz) {
                m_darken_hz.to(std::clamp(hz, tape::k_darken_floor_hz, tape::k_darken_ceil_hz), smooth_samples());
            }

            void set_smooth_ms(double ms) { m_smooth_ms = std::max(0.0, ms); }

            // -- introspection -------------------------------------------------------------------

            double length_seconds() const { return m_length_seconds; }
            bool   recording() const { return m_recording; }
            double level() const { return m_level.target(); }
            double pan() const { return m_pan.target(); }
            double darken_hz() const { return m_darken_hz.target(); }
            double smooth_ms() const { return m_smooth_ms; }
            double samplerate() const { return m_sr; }
            double max_loop_seconds() const { return prepared() ? static_cast<double>(m_tape.capacity()) / m_sr : 0.0; }

            /// The active loop length in samples — what the bank's lcm arithmetic reads.
            long loop_samples() const { return m_tape.loop_samples(); }

            /// The head position as a fraction of the length, 0..1 — read-only, so tests can pin
            /// the promise that nothing but prepare()/clear() ever resets it.
            double phase() const { return prepared() ? m_phase / static_cast<double>(m_tape.loop_samples()) : 0.0; }

            // -- audio ---------------------------------------------------------------------------

            /// Play the head onto the stereo busses, punch `in` onto the tape if recording, then
            /// advance. ACCUMULATES onto the busses (the garden.h bell idiom) so a bank can sum
            /// lanes without a scratch buffer; a standalone caller zeroes them first.
            void process(double in, double& out_left, double& out_right) {
                if (!prepared()) {
                    return;
                }
                const double played   = m_tape.read_hermite(m_phase);
                const double shade_hz = m_darken_hz.tick();
                double       toned    = played;
                if (shade_hz < tape::k_darken_ceil_hz) { // ceiling = bypass, bit-transparent
                    if (shade_hz != m_shade.cutoff_hz()) {
                        m_shade.set_cutoff_hz(shade_hz);
                    }
                    toned = m_shade.process(played);
                }
                const double g   = m_level.tick() * toned;
                const double pan = m_pan.tick();
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
                if (m_recording) { // read-before-write: you hear the old pass under the head
                    m_tape.write(static_cast<long>(std::floor(m_phase)), in);
                }
                m_phase += 1.0;
                if (m_phase >= static_cast<double>(m_tape.loop_samples())) {
                    m_phase -= static_cast<double>(m_tape.loop_samples());
                }
            }

            /// Block form for a lane used on its own: ASSIGNS, because the scalar form
            /// accumulates. A bank sums the scalar form instead.
            void process(const double* in, double* out_left, double* out_right, size_t n) {
                for (size_t i = 0; i < n; ++i) {
                    out_left[i]  = 0.0;
                    out_right[i] = 0.0;
                    process(in[i], out_left[i], out_right[i]);
                }
            }

          private:
            long smooth_samples() const { return static_cast<long>(m_smooth_ms * 0.001 * m_sr); }
            long seconds_to_samples(double s) const { return static_cast<long>(std::ceil(s * m_sr)); }

            tape::reel m_tape;
            tape::wear m_shade;      // playback tone only: drive stays 0, bypassed at ceiling
            double     m_phase{0.0}; // samples into the loop; the piece lives here
            double     m_sr{48000.0};
            double     m_smooth_ms{k_default_smooth_ms};
            double     m_length_seconds{k_min_loop_seconds};
            bool       m_recording{false};
            tape::ramp m_level;     // linear
            tape::ramp m_pan;       // -1..1
            tape::ramp m_darken_hz; // Hz
        };

        /// Up to eight free-running tape loops of unequal lengths, summed to stereo — an array of
        /// `loop` plus the count, the shared smoothing time, and the lcm arithmetic.
        class loop_bank {
          public:
            // -- lifecycle -----------------------------------------------------------------------

            /// Buy k_max_loops reels for `max_loop_seconds` at `sr` and restart every lane — a DSP
            /// restart is the one thing allowed to touch the phases. Not real-time-safe.
            void prepare(double sr, double max_loop_seconds = k_default_max_seconds) {
                m_sr = (sr > 0.0) ? sr : 48000.0;
                for (auto& l : m_loops) {
                    l.prepare(m_sr, max_loop_seconds);
                }
            }

            /// Erase every tape and rewind every head; parameters (lengths, levels, pans, darken,
            /// record gates) are untouched.
            void clear() {
                for (auto& l : m_loops) {
                    l.clear();
                }
            }

            bool prepared() const { return m_loops[0].prepared(); }

            // -- structure (instant; never touches a phase) --------------------------------------

            /// Number of active loops, clamped to [0, k_max_loops]. Newly activated loops come in
            /// at their stored settings, their heads wherever they last were.
            void set_loops(int count) { m_num_loops = std::clamp(count, 0, k_max_loops); }

            /// Per-loop length in seconds. A splice — see loop::set_length_seconds.
            void set_length_seconds(int index, double s) {
                if (valid_loop(index)) {
                    lane_ref(index).set_length_seconds(s);
                }
            }

            /// Punch the input onto this loop's tape (true) or freeze it bit-exactly (false).
            void record(int index, bool on) {
                if (valid_loop(index)) {
                    lane_ref(index).record(on);
                }
            }

            // -- parameter targets (click-free; safe while audio runs) ---------------------------

            void set_level(int index, double lin) {
                if (valid_loop(index)) {
                    lane_ref(index).set_level(lin);
                }
            }

            void set_pan(int index, double pan) {
                if (valid_loop(index)) {
                    lane_ref(index).set_pan(pan);
                }
            }

            void set_darken_hz(int index, double hz) {
                if (valid_loop(index)) {
                    lane_ref(index).set_darken_hz(hz);
                }
            }

            /// The anti-zipper window, shared by every lane.
            void set_smooth_ms(double ms) {
                m_smooth_ms = std::max(0.0, ms);
                for (auto& l : m_loops) {
                    l.set_smooth_ms(m_smooth_ms);
                }
            }

            // -- lanes ---------------------------------------------------------------------------

            /// Direct access to one lane — the same object a standalone tap.reel~ holds, so a
            /// caller (or a null test) can drive bank and lane through one code path.
            loop&       lane(int index) { return lane_ref(index); }
            const loop& lane(int index) const {
                return m_loops[static_cast<size_t>(std::clamp(index, 0, k_max_loops - 1))];
            }

            // -- introspection -------------------------------------------------------------------

            int    loops() const { return m_num_loops; }
            double length_seconds(int index) const { return valid_loop(index) ? lane(index).length_seconds() : 0.0; }
            bool   recording(int index) const { return valid_loop(index) && lane(index).recording(); }
            double level(int index) const { return valid_loop(index) ? lane(index).level() : 0.0; }
            double pan(int index) const { return valid_loop(index) ? lane(index).pan() : 0.0; }
            double darken_hz(int index) const { return valid_loop(index) ? lane(index).darken_hz() : 0.0; }
            double smooth_ms() const { return m_smooth_ms; }
            double samplerate() const { return m_sr; }
            double max_loop_seconds() const { return m_loops[0].max_loop_seconds(); }

            /// This loop's head position as a fraction of its length, 0..1 — read-only, so tests
            /// can pin the promise that nothing but prepare()/clear() ever resets it.
            double phase(int index) const { return valid_loop(index) ? lane(index).phase() : 0.0; }

            /// Least common multiple of the active loop lengths, in seconds — how long until the
            /// whole system realigns. Informational; +inf on 64-bit overflow (incommensurate
            /// lengths overflow fast, which is the point of the piece).
            double composite_period_seconds() const {
                if (!prepared() || m_num_loops < 1) {
                    return 0.0;
                }
                long long acc = 1;
                for (int i = 0; i < m_num_loops; ++i) {
                    const long long n = static_cast<long long>(lane(i).loop_samples());
                    const long long g = gcd_ll(acc, n);
                    if (acc / g > std::numeric_limits<long long>::max() / n) {
                        return std::numeric_limits<double>::infinity();
                    }
                    acc = acc / g * n;
                }
                return static_cast<double>(acc) / m_sr;
            }

            // -- audio ---------------------------------------------------------------------------

            /// Sum the active loops to the stereo bus; punch `in` onto any recording loop. This is
            /// the whole of the bank's DSP: the lanes do the rest.
            void process(double in, double& out_left, double& out_right) {
                out_left  = 0.0;
                out_right = 0.0;
                if (!prepared()) {
                    return;
                }
                for (int i = 0; i < m_num_loops; ++i) {
                    m_loops[static_cast<size_t>(i)].process(in, out_left, out_right);
                }
            }

            /// Block form: the trivial loop over the scalar path.
            void process(const double* in, double* out_left, double* out_right, size_t n) {
                for (size_t i = 0; i < n; ++i) {
                    process(in[i], out_left[i], out_right[i]);
                }
            }

          private:
            static long long gcd_ll(long long a, long long b) {
                while (b != 0) {
                    const long long t = a % b;
                    a                 = b;
                    b                 = t;
                }
                return a;
            }

            bool  valid_loop(int index) const { return index >= 0 && index < k_max_loops; }
            loop& lane_ref(int index) { return m_loops[static_cast<size_t>(std::clamp(index, 0, k_max_loops - 1))]; }

            double                        m_sr{48000.0};
            double                        m_smooth_ms{k_default_smooth_ms};
            int                           m_num_loops{0};
            std::array<loop, k_max_loops> m_loops;
        };

    } // namespace airport
} // namespace tap::tools
