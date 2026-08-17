/// @file
/// @brief      Portable live buffer-stutter kernel for tap.stammer~ — no Max/Min dependency.
/// @details    The second kernel of the Radiohead family (book/PLAN-radiohead-family.md), and the
///             one with the most direct lineage to this package: the live re-firing rig Jonny
///             Greenwood plays through his own Max patches — the guitar coming apart at the end
///             of "Go To Sleep", the mangling in "The Gloaming". Capture the input continuously,
///             then on a rhythmic grid roll dice and re-fire a slice of what just went past.
///
///             This is an ORIGINAL DESIGN in the brassage / granular tradition (Roads,
///             *Microsound*, MIT Press 2001), not a port and not a reconstruction of anyone's
///             patch. The band's rig is known from published interviews and broadcast films; that
///             record informs *what the object is for* and nothing about what the code does. No
///             preset, timing, or parameter value is taken from any product.
///
///             Two components and a thin composition, the airport.h / tapecho.h split:
///             - `capture` — the live tape: a tape_loop.h `reel` in delay-line topology with one
///               advancing write head, holding the last `max_history_ms` of input. Reads are at
///               integer positions (slices play at +-1 rate, see the limits), so the family's
///               Hermite read reduces to an exact sample fetch — the same code path a future
///               rate-varying sibling would need.
///             - `slicer` — the dice and the playback head. It owns every random draw and the
///               state of the slice in flight, and reads a capture it does not own. Like
///               tapecho.h's `head` and unlike airport.h's `loop`, a slicer is not independently
///               useful — it is a read pattern, not a machine — so it is a component for
///               composition and testing rather than a standalone external.
///             - `machine` — one capture, one slicer, the input send and the balance.
///
///             The performance surface, and the family thesis (the control is the instrument):
///             `density` is how often the machine grabs, `divisions` how finely it chops,
///             `repeats` how long it holds on, `reverse` how often a repeat runs backwards, and
///             `jump` how far back it may reach. Riding those four while a part plays is the
///             instrument; none of them is a set-and-forget.
///
///             Randomness is the family's seeded xorshift64* (tr808::white_noise, via
///             swing_vca.h), consumed in a fixed order, so **a seed is a performance you can
///             replay**: two runs of the same seed and the same moves are bit-identical, and two
///             instances on different seeds decorrelate. At `density` 0 the dice are never rolled
///             at all, so the seed provably cannot matter (pinned by test — the garden.h idle
///             contract, same shape).
///
///             Geometry: prepare(sr, max_history_ms) buys the capture once (4 s at 48 kHz is
///             ~1.5 MB of double tape). No later call allocates; setters are allocation-free and
///             safe while audio runs.
///
///             Honest limits:
///             - **Material contract.** This wants transient material — drums, struck or plucked
///               strings, consonants. On a sustained pad a stutter is barely distinguishable from
///               a tremolo: the object re-articulates rhythm that is already in the sound, it
///               does not invent it. Tested with plucks, not sines, for exactly that reason.
///             - Slices play at +-1 rate only. There is no pitch shifting and no varispeed; a
///               performable, pitch-bending playhead over live capture is a different object
///               (tap.scrub~, planned in the same family) and sharing this capture is the plan.
///             - A slice reads from the ring rather than a private copy (a copy would be a burst
///               memcpy in the audio thread). If a repeat train outlives the buffered history —
///               `repeats * length + jump` beyond `max_history_ms` — its tail reads fresher
///               material as the write head laps the origin. Size the history to the longest
///               train you intend to fire.
///             - A grid point can only start a slice while the machine is idle: a slice in flight
///               is never interrupted, so `repeats` (not `density`) is what decides how long the
///               machine stays busy, and raising density past the point where trains overlap
///               stops having an effect.
///             - `mix` is the balance between the live input and the slice, and it only bites
///               while a slice is firing. When the machine is idle the input passes through
///               untouched and bitwise, at any mix — there is nothing to blend against, and
///               equal-power blending a signal with itself would just make it louder.
///             - Mono. Per-slice stereo scatter is not modeled; wrap in `mc.` for multichannel.
/// @author     Timothy Place
// SPDX-License-Identifier: MIT
// Copyright 2026 Timothy Place.

#pragma once

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>

#include "swing_vca.h" // tap::tools::tr808::white_noise — the family's seeded xorshift64*
#include "tape_loop.h" // tap::tools::tape — reel (the capture) + ramp, the shared machinery

namespace tap::tools {
    namespace stammer {

        constexpr int      k_max_divisions          = 8;      // slice = step / k, k in [1, divisions]
        constexpr int      k_max_repeats            = 16;     // repeats per fired slice
        constexpr long     k_min_slice_samples      = 16;     // below this it is a click, not a slice
        constexpr double   k_min_step_ms            = 1.0;    // the grid floor
        constexpr double   k_default_max_history_ms = 4000.0; // default buy (~1.5 MB @ 48k)
        constexpr double   k_default_step_ms        = 250.0;  // a plausible grid out of the box
        constexpr double   k_default_density        = 0.5;    // grabs about half the idle grid points
        constexpr int      k_default_divisions      = 4;
        constexpr int      k_default_repeats        = 4;
        constexpr double   k_default_reverse        = 0.25; // a quarter of repeats run backwards
        constexpr double   k_default_jump_ms        = 0.0;  // the classic stutter: the material just past
        constexpr double   k_default_fade_ms        = 3.0;  // per-repeat flank, click-free
        constexpr double   k_default_mix            = 100.0;
        constexpr double   k_default_smooth_ms      = 20.0; // anti-zipper ramp for the level setters
        constexpr uint64_t k_default_seed           = 1;

        /// The live tape: the last `max_history_ms` of input under one advancing write head.
        class capture {
          public:
            /// Buy the history once. Not real-time-safe.
            void prepare(double sr, double max_history_ms) {
                m_sr = (sr > 0.0) ? sr : 48000.0;
                m_reel.prepare(m_sr, std::max(k_min_step_ms, max_history_ms) * 0.001);
                clear();
            }

            /// Erase the tape and rewind the write head.
            void clear() {
                m_reel.clear();
                m_write = 0;
            }

            bool   prepared() const { return m_reel.prepared(); }
            long   capacity() const { return m_reel.capacity(); }
            long   position() const { return m_write; } // absolute position of the NEXT write
            double samplerate() const { return m_sr; }
            double history_ms() const { return static_cast<double>(capacity()) * 1000.0 / m_sr; }

            /// Record one sample and advance the head.
            void write(double x) {
                m_reel.write(m_write, x);
                if (++m_write >= m_reel.capacity()) { // keep the head in [0, capacity): a long can
                    m_write = 0;                      // overflow in half a day of audio on LLP64
                }
            }

            /// Read at an absolute position (wraps). Positions are integers here, so the family's
            /// Hermite read returns the stored sample exactly (fraction 0 reads x0).
            double read(long pos) const { return m_reel.read_hermite(static_cast<double>(pos)); }

            /// Read at a FRACTIONAL absolute position (wraps) — the same 4-point Hermite, exposed
            /// for the family's rate-varying sibling (scrub.h), which shares this capture rather
            /// than keeping its own. The slicer never calls it: its slices play at ±1 rate.
            double read_frac(double pos) const { return m_reel.read_hermite(pos); }

          private:
            double     m_sr{48000.0};
            long       m_write{0};
            tape::reel m_reel;
        };

        /// The dice and the playback head: decides when to grab, how much, how many times, and
        /// which way round, then plays it back out of a capture it does not own.
        class slicer {
          public:
            struct out {
                double value{0.0};    // the slice sample (0 when idle)
                bool   firing{false}; // whether a slice was sounding for this sample
            };

            slicer() { m_rng.set_seed(k_default_seed); }

            /// Reset the grid, drop any slice in flight, and re-seed — so a restart replays the
            /// same performance. Not a parameter move.
            void prepare(double sr) {
                m_sr = (sr > 0.0) ? sr : 48000.0;
                clear();
            }

            void clear() {
                m_rng.reset();
                m_countdown   = 0;
                m_playing     = false;
                m_pos         = 0;
                m_len         = 0;
                m_left        = 0;
                m_reverse_now = false;
            }

            // -- the performance surface (allocation-free, safe while audio runs) ----------------

            /// The rhythmic grid in ms: how often the machine may decide to grab. Takes effect at
            /// the next grid point (it is a rhythm, not a level — nothing to zipper).
            void set_step_ms(double ms) { m_step_ms = std::max(k_min_step_ms, ms); }

            /// Probability in [0, 1] of firing at an idle grid point. At exactly 0 the dice are
            /// never rolled, so the seed cannot matter.
            void set_density(double p) { m_density = std::clamp(p, 0.0, 1.0); }

            /// How finely the grid may be chopped: a slice is step / k with k drawn uniformly
            /// from [1, divisions]. 1 means whole-step slices only.
            void set_divisions(int n) { m_divisions = std::clamp(n, 1, k_max_divisions); }

            /// Upper bound on how many times a fired slice repeats; the count is drawn uniformly
            /// from [1, repeats].
            void set_repeats(int n) { m_repeats = std::clamp(n, 1, k_max_repeats); }

            /// Probability in [0, 1] that any given repeat plays backwards. Drawn per repeat, so
            /// a train can stagger forwards and back.
            void set_reverse(double p) { m_reverse = std::clamp(p, 0.0, 1.0); }

            /// How far back beyond the immediately-past material a slice may reach, in ms; the
            /// actual reach is drawn uniformly from [0, jump]. 0 is the classic stutter.
            void set_jump_ms(double ms) { m_jump_ms = std::max(0.0, ms); }

            /// Raised-sine flank width per repeat, in ms — the anti-click. Clamped per slice to
            /// half the slice so the flanks never overlap.
            void set_fade_ms(double ms) { m_fade_ms = std::max(0.0, ms); }

            /// The performance seed. Instant; takes effect on the next draw (clear() restarts the
            /// stream from it).
            void set_seed(uint64_t seed) { m_rng.set_seed(seed); }

            // -- introspection -------------------------------------------------------------------

            double   step_ms() const { return m_step_ms; }
            double   density() const { return m_density; }
            int      divisions() const { return m_divisions; }
            int      repeats() const { return m_repeats; }
            double   reverse() const { return m_reverse; }
            double   jump_ms() const { return m_jump_ms; }
            double   fade_ms() const { return m_fade_ms; }
            uint64_t seed() const { return m_rng.seed(); }
            bool     playing() const { return m_playing; }

            // -- audio ---------------------------------------------------------------------------

            /// Advance one sample: tick the grid, maybe fire, and play whatever is in flight.
            /// Call once per sample AFTER the capture has recorded this sample.
            out process(const capture& tape) {
                if (--m_countdown <= 0) {
                    m_countdown = std::max(1L, static_cast<long>(m_step_ms * 0.001 * m_sr));
                    if (!m_playing) {
                        maybe_fire(tape);
                    }
                }
                if (!m_playing) {
                    return {};
                }

                const long   read_at = m_reverse_now ? (m_origin + m_len - 1 - m_pos) : (m_origin + m_pos);
                const double value   = tape.read(read_at) * envelope(m_pos, m_len, m_fade);

                if (++m_pos >= m_len) { // this repeat is done
                    m_pos = 0;
                    if (--m_left <= 0) {
                        m_playing = false;
                    }
                    else {
                        m_reverse_now = (uniform() < m_reverse); // each repeat rolls its own way round
                    }
                }
                return {value, true};
            }

          private:
            /// [0, 1) from the family's seeded xorshift64* (which returns [-1, 1)).
            double uniform() { return 0.5 * (m_rng.process() + 1.0); }

            /// Raised-sine flanks: exactly 0 at both edges, exactly 1 across the plateau. Repeats
            /// are sequential rather than overlapped, so each junction dips to zero — that is the
            /// articulation of a stutter, not a defect.
            static double envelope(long i, long len, long fade) {
                if (fade <= 0) {
                    return 1.0;
                }
                const double half_pi = tape::k_pi * 0.5;
                double       g       = 1.0;
                if (i < fade) {
                    g = std::sin(half_pi * static_cast<double>(i) / static_cast<double>(fade));
                }
                const long from_end = len - 1 - i;
                if (from_end < fade) {
                    g = std::min(g, std::sin(half_pi * static_cast<double>(from_end) / static_cast<double>(fade)));
                }
                return g;
            }

            /// Roll for a slice. The draw order is fixed — fire, division, repeats, jump, reverse
            /// — because it is what makes a seed reproducible.
            void maybe_fire(const capture& tape) {
                if (m_density <= 0.0) {
                    return; // the dice are never rolled, so the seed provably cannot matter
                }
                if (uniform() >= m_density) {
                    return;
                }

                const long step_samples = std::max(1L, static_cast<long>(m_step_ms * 0.001 * m_sr));
                const int  k            = 1 + static_cast<int>(uniform() * static_cast<double>(m_divisions));
                const long divisor      = std::clamp(static_cast<long>(k), 1L, static_cast<long>(m_divisions));
                long       len          = std::max(k_min_slice_samples, step_samples / divisor);

                const int  n    = 1 + static_cast<int>(uniform() * static_cast<double>(m_repeats));
                const long jump = static_cast<long>(uniform() * m_jump_ms * 0.001 * m_sr);

                // The slice must fit the bought history, origin and reach together.
                const long room = tape.capacity() - jump - 2;
                len             = std::clamp(len, k_min_slice_samples, std::max(k_min_slice_samples, room));

                m_origin      = tape.position() - len - jump;
                m_len         = len;
                m_fade        = std::min(static_cast<long>(m_fade_ms * 0.001 * m_sr), len / 2);
                m_left        = std::clamp(n, 1, m_repeats);
                m_pos         = 0;
                m_reverse_now = (uniform() < m_reverse);
                m_playing     = true;
            }

            double m_sr{48000.0};

            // performance surface
            double m_step_ms{k_default_step_ms};
            double m_density{k_default_density};
            int    m_divisions{k_default_divisions};
            int    m_repeats{k_default_repeats};
            double m_reverse{k_default_reverse};
            double m_jump_ms{k_default_jump_ms};
            double m_fade_ms{k_default_fade_ms};

            // the slice in flight
            bool m_playing{false};
            bool m_reverse_now{false};
            long m_origin{0};
            long m_len{0};
            long m_fade{0};
            long m_pos{0};
            long m_left{0};
            long m_countdown{0};

            tr808::white_noise m_rng;
        };

        /// The machine: one capture, one slicer, the input send and the balance.
        class machine {
          public:
            machine() {
                m_input_level.snap(1.0);
                m_mix.snap(k_default_mix);
            }

            // -- lifecycle -----------------------------------------------------------------------

            /// (Re)allocate the capture for `max_history_ms` at `sr`, snap the ramps, reset the
            /// grid and re-seed. Not real-time-safe.
            void prepare(double sr, double max_history_ms = k_default_max_history_ms) {
                m_sr = (sr > 0.0) ? sr : 48000.0;
                m_capture.prepare(m_sr, max_history_ms);
                m_slicer.prepare(m_sr);
                m_input_level.snap(m_input_level.target());
                m_mix.snap(m_mix.target());
            }

            /// Erase the capture, drop any slice in flight, and restart the seeded stream.
            void clear() {
                m_capture.clear();
                m_slicer.clear();
            }

            bool prepared() const { return m_capture.prepared(); }

            // -- parameters ----------------------------------------------------------------------

            void set_step_ms(double ms) { m_slicer.set_step_ms(ms); }
            void set_density(double p) { m_slicer.set_density(p); }
            void set_divisions(int n) { m_slicer.set_divisions(n); }
            void set_repeats(int n) { m_slicer.set_repeats(n); }
            void set_reverse(double p) { m_slicer.set_reverse(p); }
            void set_jump_ms(double ms) { m_slicer.set_jump_ms(ms); }
            void set_fade_ms(double ms) { m_slicer.set_fade_ms(ms); }
            void set_seed(uint64_t seed) { m_slicer.set_seed(seed); }

            /// Input level into the capture, linear, slewed.
            void set_input_level(double lin) { m_input_level.to(lin, smooth_samples()); }

            /// Balance between the live input and the slice, 0..100, equal-power — and it only
            /// bites while a slice is firing (see the header's limits).
            void set_mix(double pct) { m_mix.to(std::clamp(pct, 0.0, 100.0), smooth_samples()); }

            void set_smooth_ms(double ms) { m_smooth_ms = std::max(0.0, ms); }

            // -- introspection -------------------------------------------------------------------

            double   step_ms() const { return m_slicer.step_ms(); }
            double   density() const { return m_slicer.density(); }
            int      divisions() const { return m_slicer.divisions(); }
            int      repeats() const { return m_slicer.repeats(); }
            double   reverse() const { return m_slicer.reverse(); }
            double   jump_ms() const { return m_slicer.jump_ms(); }
            double   fade_ms() const { return m_slicer.fade_ms(); }
            uint64_t seed() const { return m_slicer.seed(); }
            bool     playing() const { return m_slicer.playing(); }
            double   input_level() const { return m_input_level.target(); }
            double   mix() const { return m_mix.target(); }
            double   smooth_ms() const { return m_smooth_ms; }
            double   max_history_ms() const { return m_capture.history_ms(); }
            double   samplerate() const { return m_sr; }

            // -- audio ---------------------------------------------------------------------------

            double process(double in) {
                if (!prepared()) {
                    return in;
                }
                const double send = m_input_level.tick();
                const double mix  = m_mix.tick();

                m_capture.write(send * in);
                const slicer::out s = m_slicer.process(m_capture);

                // Idle is a bitwise passthrough at any mix: there is nothing to blend against,
                // and equal-power blending a signal with itself would only make it louder.
                if (!s.firing) {
                    return in;
                }
                if (mix <= 0.0) {
                    return in;
                }
                if (mix >= 100.0) {
                    return s.value;
                }
                const double theta = mix * 0.01 * (tape::k_pi * 0.5);
                return std::cos(theta) * in + std::sin(theta) * s.value;
            }

            /// Block form: the trivial loop over the scalar path.
            void process(const double* in, double* out, size_t n) {
                for (size_t i = 0; i < n; ++i) {
                    out[i] = process(in[i]);
                }
            }

          private:
            long smooth_samples() const { return static_cast<long>(m_smooth_ms * 0.001 * m_sr); }

            double     m_sr{48000.0};
            double     m_smooth_ms{k_default_smooth_ms};
            capture    m_capture;
            slicer     m_slicer;
            tape::ramp m_input_level; // linear
            tape::ramp m_mix;         // 0..100
        };

    } // namespace stammer
} // namespace tap::tools
