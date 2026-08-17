/// @file
/// @brief      Portable granular-scrub kernel for tap.scrub~ — no Max/Min dependency.
/// @details    The fifth kernel of the Radiohead family (book/PLAN-radiohead-family.md), and its
///             most direct piece of stagecraft: the Kaoss-school scrub pad. Record the input
///             continuously, then put a granular playhead on it whose *position* and *pitch* are
///             two independent performable signals. Drag the position and you rake back and forth
///             through the last few seconds of the performance; hold it still and you have a
///             granular freeze; move the pitch and the material transposes without the position
///             moving at all. That decoupling is the whole object — a tape head cannot do it, and
///             it is why the pad feels like an instrument rather than a delay.
///
///             This is an ORIGINAL DESIGN in the brassage / granular tradition (Roads,
///             *Microsound*, MIT Press 2001), not a port and not a reconstruction of any product.
///             No preset, timing, or parameter value is taken from any hardware.
///
///             **What it shares, and why that was the plan.** stammer.h's `capture` is the same
///             live tape — one `tape_loop.h` reel under an advancing write head — and this kernel
///             uses it directly rather than keeping a second copy. stammer.h's own header says so
///             in its limits ("slices play at ±1 rate … a performable, pitch-bending playhead over
///             live capture is a different object, and sharing this capture is the plan"); the one
///             thing that had to be added for this kernel is `capture::read_frac`, the fractional
///             Hermite read the stutter never needed.
///
///             Two classes and a thin composition, the family's habit:
///             - `head` — the grain scheduler. It owns the grain pool, the hop clock, and the
///               spray dice, and reads a capture it does not own. Like tapecho.h's `head` and
///               stammer.h's `slicer`, it is a read pattern rather than a machine, so it is a
///               component for composition and testing, not a standalone external.
///             - `machine` — one capture, one head, the freeze gate, the drift, and the balance.
///
///             **The window, and the null it buys.** Grains are Hann-windowed and fired every
///             `size / overlap` samples. Hann satisfies the constant-overlap-add condition at
///             those hops, so at `overlap` 2 the windows sum to exactly 1 — which means that with
///             pitch at unity, spray at zero, and the position held on a whole sample, the scrub
///             is *the input, delayed*, to within floating point. That is the load-bearing
///             scenario: everything else this object does is a departure from a plain delay, and
///             the departure is only trustworthy if the identity is exact when it should be.
///             (Normalization is 2/overlap, so the level holds across overlap settings.)
///
///             Randomness is the family's seeded xorshift64* (tr808::white_noise, via
///             swing_vca.h). `spray` is the only consumer, and at exactly 0 it is never drawn from
///             at all, so the seed provably cannot matter — the garden.h / stammer.h contract,
///             same shape, pinned by the same kind of test.
///
///             Geometry: prepare(sr, max_history_ms) buys the capture once. No later call
///             allocates; setters are allocation-free and safe while audio runs.
///
///             Honest limits:
///             - **A grain can read past the write head.** A grain born `lag` samples behind the
///               edge and playing at rate r reaches `lag − size·(r−1)` behind it by its end, so
///               transposing up with the position near the live edge runs the grain's tail off the
///               front of the tape and into the oldest material. It is the constraint every live
///               granulator has; keep the position at least `size·(rate−1)` back, or accept the
///               seam. Nothing clamps it, because clamping would silently bend the pitch.
///             - **Transposition warbles.** Reading tape at a rate the write head does not share
///               means the read pointer drifts, and it has to be wrapped back if the position is
///               to keep meaning anything. Every wrap is a splice between two grains reading
///               material a wander apart, at a rate of sr·|rate−1| / (wander · size) per second.
///               What that costs is not the pitch — measured over 7 fundamentals x 7 intervals,
///               98.8 % of a perfect shifter's energy lands in a ±15 Hz band around the transposed
///               pitch (worst case 91.7 %) — but the *concentration* of it: the band holds a
///               narrow comb rather than a single line, 92.0 % as concentrated as a clean shift
///               and 75.0 % in the worst case. Audibly that is a warble, and it is the classic
///               single-delay-line pitch-shifting artifact rather than anything specific to this
///               kernel. For clean transposition reach for `tap.pitchaccum~` or `tap.shift~`;
///               `spray` trades the comb for a broadband smear if that suits the material better.
///             - **`freeze` stops the recorder, not the playhead.** Frozen, the position addresses
///               fixed tape and the grains loop the same window — a granular hold. What it does
///               not do is stop time inside a grain: the tail of a grain in flight when freeze
///               engages was already scheduled.
///             - **The grain pool can starve.** Shrinking `size` sharply while grains are in
///               flight can leave every slot busy at the moment the next grain is due; that grain
///               is dropped rather than stealing a slot mid-window, because a steal would click.
///               The audible cost is a momentary dip, and it is bounded by the pool being two
///               deeper than the maximum overlap.
///             - **COLA is exact only when `size` divides by `overlap`.** The hop is integer
///               samples, so a size that does not divide evenly leaves a small periodic ripple in
///               the window sum. It is inaudible at musical sizes and it is why the null test
///               chooses its numbers.
///             - **No transient detection.** Grains fire on a clock, not on the material. A
///               scrub across a drum hit will chop it wherever the clock happens to be.
///             - Mono. Per-grain stereo scatter is not modeled; wrap in `mc.` for multichannel.
/// @author     Timothy Place
// SPDX-License-Identifier: MIT
// Copyright 2026 Timothy Place.

#pragma once

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>

#include "stammer.h"   // tap::tools::stammer::capture — the shared live tape
#include "swing_vca.h" // tap::tools::tr808::white_noise — the family's seeded xorshift64*
#include "tape_loop.h" // tap::tools::tape — ramp, k_pi

namespace tap::tools {
    namespace scrub {

        constexpr int    k_max_overlap            = 4;
        constexpr int    k_max_grains             = k_max_overlap + 2; // two deeper than the worst overlap
        constexpr long   k_min_grain_samples      = 16;                // below this it is a click, not a grain
        constexpr double k_min_size_ms            = 1.0;
        constexpr double k_max_size_ms            = 500.0;
        constexpr double k_max_pitch_st           = 24.0;   // ±2 octaves, the pitchaccum.h range
        constexpr double k_max_drift              = 8.0;    // playback-rate units of self-motion
        constexpr double k_default_max_history_ms = 4000.0; // the stammer's default buy (~1.5 MB @ 48k)

        // How far the phase-continuous read head may wander from where the position says it is,
        // in grain lengths, before it is wrapped back. Larger means rarer wraps and so fewer
        // splices, at the cost of that much position error while transposing. 3 (a wander of
        // +-1.5 grains) is measured rather than guessed: swept over 5 fundamentals x 7 intervals,
        // the energy landing in a +-15 Hz band around the transposed pitch runs
        // 0.933 / 0.958 / 0.965 / 0.990 / 0.993 of a perfect shifter's at wanders of
        // 0.5 / 1 / 2 / 3 / 4 grains, with worst cases 0.716 / 0.820 / 0.874 / 0.918 / 0.940.
        // The curve is flat past 3, and every grain of extra wander is a grain of position error,
        // so 3 is where it stops.
        constexpr double k_wander_grains = 3.0;

        constexpr double   k_default_size_ms     = 80.0;
        constexpr int      k_default_overlap     = 2;
        constexpr double   k_default_spray_ms    = 0.0;
        constexpr double   k_default_position_ms = 0.0; // at the live edge
        constexpr double   k_default_pitch_st    = 0.0;
        constexpr double   k_default_drift       = 0.0;
        constexpr double   k_default_mix         = 100.0;
        constexpr double   k_default_level       = 1.0;
        constexpr double   k_default_smooth_ms   = 20.0;
        constexpr uint64_t k_default_seed        = 1;

        /// The grain scheduler: fires Hann-windowed grains on a hop clock, each anchored at the
        /// current position and locked to the pitch it was born at (the standard granular
        /// contract — a grain whose rate moved mid-window would smear its own content).
        class head {
          public:
            head() { m_rng.set_seed(k_default_seed); }

            /// Reset the clock, kill every grain, and restart the seeded stream.
            void prepare(double sr) {
                m_sr = (sr > 0.0) ? sr : 48000.0;
                clear();
            }

            void clear() {
                m_rng.reset();
                for (auto& g : m_grain) {
                    g.alive = false;
                }
                m_countdown = 0;
                m_error     = 0.0;
            }

            // -- the performance surface (allocation-free, safe while audio runs) ----------------

            /// Grain length in ms. Takes effect at the next grain birth; grains in flight keep the
            /// length they were born with.
            void set_size_ms(double ms) { m_size_ms = std::clamp(ms, k_min_size_ms, k_max_size_ms); }

            /// How many grains overlap: the hop is size / overlap. 1 leaves gaps (a chopped
            /// texture); 2 and up satisfy Hann's overlap-add condition and hold a constant level.
            void set_overlap(int n) { m_overlap = std::clamp(n, 1, k_max_overlap); }

            /// Random scatter of each grain's origin, in ms back from the position. At exactly 0
            /// the dice are never rolled, so the seed cannot matter.
            void set_spray_ms(double ms) { m_spray_ms = std::max(0.0, ms); }

            /// The performance seed. Instant; takes effect on the next draw (clear() restarts it).
            void set_seed(uint64_t seed) { m_rng.set_seed(seed); }

            // -- introspection -------------------------------------------------------------------

            double   size_ms() const { return m_size_ms; }
            int      overlap() const { return m_overlap; }
            double   spray_ms() const { return m_spray_ms; }
            uint64_t seed() const { return m_rng.seed(); }

            int active_grains() const {
                int n = 0;
                for (const auto& g : m_grain) {
                    n += g.alive ? 1 : 0;
                }
                return n;
            }

            /// The window-sum normalization currently in force — 2/overlap once the windows
            /// overlap-add, 1 when they do not. Exposed because the level contract depends on it.
            double normalization() const { return (m_overlap >= 2) ? 2.0 / static_cast<double>(m_overlap) : 1.0; }

            // -- audio ---------------------------------------------------------------------------

            /// Advance one sample. `lag` is how far behind the capture's write head the playhead
            /// sits, in samples; `rate` is the grain playback ratio (1 = as recorded). Call once
            /// per sample AFTER the capture has recorded this sample.
            double process(const stammer::capture& tape, double lag, double rate) {
                const long len = std::max(k_min_grain_samples, static_cast<long>(m_size_ms * 0.001 * m_sr));
                const long hop = std::max(1L, len / static_cast<long>(m_overlap));
                if (--m_countdown <= 0) {
                    m_countdown = hop;
                    birth(tape, lag, rate, len, hop);
                }

                double sum = 0.0;
                for (auto& g : m_grain) {
                    if (!g.alive) {
                        continue;
                    }
                    const double phase = static_cast<double>(g.age) / static_cast<double>(g.len);
                    const double w     = 0.5 - 0.5 * std::cos(2.0 * tape::k_pi * phase);
                    sum += w * tape.read_frac(g.origin + static_cast<double>(g.age) * g.rate);
                    if (++g.age >= g.len) {
                        g.alive = false;
                    }
                }
                return sum * normalization();
            }

          private:
            struct grain {
                double origin{0.0}; // absolute capture position this grain started reading at
                double rate{1.0};   // locked at birth
                long   len{0};
                long   age{0};
                bool   alive{false};
            };

            /// [0, 1) from the family's seeded xorshift64* (which returns [-1, 1)).
            double uniform() { return 0.5 * (m_rng.process() + 1.0); }

            /// Take a free slot and anchor a grain. A full pool DROPS the grain rather than
            /// stealing one mid-window — a steal would click, and the dip is bounded (see limits).
            void birth(const stammer::capture& tape, double lag, double rate, long len, long hop) {
                grain* g = nullptr;
                for (auto& c : m_grain) {
                    if (!c.alive) {
                        g = &c;
                        break;
                    }
                }
                if (g == nullptr) {
                    return;
                }
                // The dice are consulted only when spray is armed, so the seed cannot matter at 0.
                const double spray = (m_spray_ms > 0.0) ? uniform() * m_spray_ms * 0.001 * m_sr : 0.0;

                // The read pointer has to advance through the tape at `rate`, ACROSS grains and not
                // only inside them. Anchoring every grain at the position instead advances the
                // origins at the write head's speed, and then the transposition applies only within
                // each grain: the average read rate comes back to 1 and a steady tone comes out at
                // its original pitch with a comb of grain-rate sidebands around it, which is the
                // pitch shift not happening. (Measured — see notebooks/scrub.ipynb; the same trap
                // catches any delay-line shifter, tap.pitchaccum~ included.)
                //
                // So the origin tracks a phase-continuous head, and `m_error` is how far that head
                // has drifted from where the position says it should be. It is wrapped into
                // +-len/2 so the position stays meaningful: the read stays continuous between
                // wraps, and a wrap costs one splice at a grain boundary rather than a splice on
                // every grain. That is the classic delay-line-shifter bargain, and the warble it
                // leaves is at the wrap rate, hop*|rate-1| / len grains apart.
                if (rate == 1.0) {
                    m_error = 0.0; // back at unity the position is the truth again, exactly
                }
                else {
                    m_error += static_cast<double>(hop) * (rate - 1.0);
                    const double span = static_cast<double>(len) * k_wander_grains;
                    m_error -= span * std::floor(m_error / span + 0.5);
                }

                // position() is the NEXT write, and this sample has already been recorded, so the
                // live edge is one behind it — the offset that makes lag 0 read the newest sample
                // and lag n read exactly n samples ago. At rate 1 the error is exactly 0, so the
                // null against a plain delay is untouched.
                g->origin = static_cast<double>(tape.position()) - 1.0 - lag + m_error - spray;
                g->rate   = rate;
                g->len    = len;
                g->age    = 0;
                g->alive  = true;
            }

            double m_sr{48000.0};
            double m_size_ms{k_default_size_ms};
            int    m_overlap{k_default_overlap};
            double m_spray_ms{k_default_spray_ms};
            long   m_countdown{0};
            double m_error{0.0}; // the phase-continuous head's drift from the anchor, wrapped

            std::array<grain, k_max_grains> m_grain;
            tr808::white_noise              m_rng;
        };

        /// The machine: one capture, one head, the freeze gate, the drift, and the balance.
        class machine {
          public:
            machine() {
                m_position.snap(k_default_position_ms);
                m_pitch.snap(k_default_pitch_st);
                m_drift.snap(k_default_drift);
                m_mix.snap(k_default_mix);
                m_level.snap(k_default_level);
            }

            // -- lifecycle -----------------------------------------------------------------------

            /// (Re)allocate the capture for `max_history_ms` at `sr`, snap the ramps, reset the
            /// grain clock and re-seed. Not real-time-safe.
            void prepare(double sr, double max_history_ms = k_default_max_history_ms) {
                m_sr = (sr > 0.0) ? sr : 48000.0;
                m_capture.prepare(m_sr, max_history_ms);
                m_head.prepare(m_sr);
                m_position.snap(m_position.target());
                m_pitch.snap(m_pitch.target());
                m_drift.snap(m_drift.target());
                m_mix.snap(m_mix.target());
                m_level.snap(m_level.target());
                m_drift_acc = 0.0;
            }

            /// Erase the tape, kill every grain, rewind the drift, and restart the seeded stream.
            void clear() {
                m_capture.clear();
                m_head.clear();
                m_drift_acc = 0.0;
            }

            bool prepared() const { return m_capture.prepared(); }

            // -- parameters ----------------------------------------------------------------------

            /// Where the playhead sits, as a lag behind the live edge in ms. 0 is the newest
            /// sample. Slewed, because this is the scrub gesture.
            void set_position_ms(double ms) { m_position.to(std::clamp(ms, 0.0, max_history_ms()), smooth_samples()); }

            /// Transposition in semitones, ±2 octaves. Independent of the position — that
            /// independence is the object.
            void set_pitch(double semitones) {
                m_pitch.to(std::clamp(semitones, -k_max_pitch_st, k_max_pitch_st), smooth_samples());
            }

            /// The playhead's own motion through the tape, in playback-rate units: positive runs
            /// forward toward the live edge, negative backwards, 0 holds station. It wraps around
            /// the bought history rather than clamping, so a slow drift is a loop.
            void set_drift(double rate) { m_drift.to(std::clamp(rate, -k_max_drift, k_max_drift), smooth_samples()); }

            /// Stop the recorder. The playhead keeps going, so the position now addresses fixed
            /// tape — a granular hold you can still scrub, transpose, and drift through.
            void set_freeze(bool on) { m_freeze = on; }

            void set_size_ms(double ms) { m_head.set_size_ms(ms); }
            void set_overlap(int n) { m_head.set_overlap(n); }
            void set_spray_ms(double ms) { m_head.set_spray_ms(ms); }
            void set_seed(uint64_t seed) { m_head.set_seed(seed); }

            /// Balance between the live input and the scrub, 0..100, equal-power.
            void set_mix(double pct) { m_mix.to(std::clamp(pct, 0.0, 100.0), smooth_samples()); }

            /// Output level, linear.
            void set_level(double lin) { m_level.to(lin, smooth_samples()); }

            void set_smooth_ms(double ms) { m_smooth_ms = std::max(0.0, ms); }

            // -- introspection -------------------------------------------------------------------

            double   position_ms() const { return m_position.target(); }
            double   pitch() const { return m_pitch.target(); }
            double   drift() const { return m_drift.target(); }
            bool     freeze() const { return m_freeze; }
            double   size_ms() const { return m_head.size_ms(); }
            int      overlap() const { return m_head.overlap(); }
            double   spray_ms() const { return m_head.spray_ms(); }
            uint64_t seed() const { return m_head.seed(); }
            double   mix() const { return m_mix.target(); }
            double   level() const { return m_level.target(); }
            double   smooth_ms() const { return m_smooth_ms; }
            double   max_history_ms() const { return m_capture.history_ms(); }
            int      active_grains() const { return m_head.active_grains(); }
            double   samplerate() const { return m_sr; }

            head&                   grains() { return m_head; }
            const head&             grains() const { return m_head; }
            stammer::capture&       tape() { return m_capture; }
            const stammer::capture& tape() const { return m_capture; }

            // -- audio ---------------------------------------------------------------------------

            /// Attribute-driven path: position and pitch come from their ramps.
            double process(double in) { return core(in, m_position.tick(), m_pitch.tick()); }

            /// Signal-driven path: position (ms behind the edge) and pitch (semitones) are taken
            /// straight from the caller and the ramps are bypassed — a signal is already smooth.
            /// The ramps are still ticked so a later switch back to the attribute path is
            /// continuous rather than a jump.
            double process(double in, double position_ms, double pitch_st) {
                m_position.tick();
                m_pitch.tick();
                return core(in, std::clamp(position_ms, 0.0, max_history_ms()),
                            std::clamp(pitch_st, -k_max_pitch_st, k_max_pitch_st));
            }

            /// Block form: the trivial loop over the scalar path.
            void process(const double* in, double* out, size_t n) {
                for (size_t i = 0; i < n; ++i) {
                    out[i] = process(in[i]);
                }
            }

          private:
            long smooth_samples() const { return static_cast<long>(m_smooth_ms * 0.001 * m_sr); }

            double core(double in, double position_ms, double pitch_st) {
                if (!prepared()) {
                    return in;
                }
                if (!m_freeze) {
                    m_capture.write(in);
                }

                // The playhead's own motion, accumulated as an extra lag and wrapped around the
                // bought history so a drift loops the tape rather than running off it.
                const double cap = static_cast<double>(m_capture.capacity());
                m_drift_acc -= m_drift.tick();
                m_drift_acc = std::fmod(m_drift_acc, cap);
                if (m_drift_acc < 0.0) {
                    m_drift_acc += cap;
                }

                double lag = position_ms * 0.001 * m_sr + m_drift_acc;
                lag        = std::fmod(lag, cap);
                if (lag < 0.0) {
                    lag += cap;
                }

                const double wet = m_head.process(m_capture, lag, std::exp2(pitch_st / 12.0));

                const double pct = m_mix.tick();
                const double lin = m_level.tick();
                // The endpoints are exact rather than cos(pi/2)-approximate, so a fully wet scrub
                // really is the scrub and a fully dry one really is the input (the stammer.h rule).
                if (pct >= 100.0) {
                    return wet * lin;
                }
                if (pct <= 0.0) {
                    return in * lin;
                }
                const double theta = pct * 0.01 * (tape::k_pi * 0.5);
                return (std::cos(theta) * in + std::sin(theta) * wet) * lin;
            }

            double m_sr{48000.0};
            double m_smooth_ms{k_default_smooth_ms};
            bool   m_freeze{false};
            double m_drift_acc{0.0};

            stammer::capture m_capture;
            head             m_head;
            tape::ramp       m_position, m_pitch, m_drift, m_mix, m_level;
        };

    } // namespace scrub
} // namespace tap::tools
