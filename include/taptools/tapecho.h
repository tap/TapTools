/// @file
/// @brief      Portable multi-head tape-echo kernel for tap.tapecho~ — no Max/Min dependency.
/// @details    The first kernel of the Radiohead family (book/PLAN-radiohead-family.md): the
///             multi-head tape echo of the Watkins/WEM Copicat and Roland Space Echo school —
///             one record head, a span of moving tape, several playback heads at fixed positions
///             along it, and a regeneration path from the heads back to the record head.
///
///             It is a *recreation of the topology*, not a port or a circuit model of any one
///             machine: the tape-path DSP (fractional read, periodic wow/flutter, in-loop
///             coloration) is the published tape-echo modeling literature already carried by
///             tape_loop.h (Arnardottir, Abel, Smith, "A Digital Model of the Echoplex Tape
///             Delay", AES 125, 2008; Valimaki et al.'s tape-echo work). No head spacings,
///             filter curves, or trim values are claimed as measured from any unit — see the
///             head-layout note below.
///
///             Almost all of the machinery is tape_loop.h; this kernel is the composition. Two
///             classes, the airport.h split:
///             - `head` — ONE playback head: its position along the tape path (`ratio` of the
///               motor span), its level, and its place in the stereo field. It carries its own
///               anti-zipper ramps and reads a reel it does not own. Unlike `airport::loop` a
///               head is NOT independently useful — it is a read position, not a machine — so it
///               is a component for composition and testing, not a standalone external.
///             - `machine` — one reel in delay-line topology, one transport, one wear stage, and
///               k_max_heads heads. Its process() is a sum over the heads plus the regeneration
///               write, and nothing else.
///
///             Geometry of the head path: `span_ms` is the motor — the delay of a head at the
///             far end of the path (ratio 1.0) — and each head's delay is `span_ms * ratio`, so
///             moving the motor moves every head together, as a tape speed does. Defaults are
///             four evenly spaced heads (0.25, 0.5, 0.75, 1.0). Even spacing is a nominal layout
///             chosen because it is neutral and audibly "a tape echo"; real machines' head
///             positions vary by model and unit and are not modeled here. Ratios are freely
///             settable, which is how a three-head Copicat-style layout is built.
///
///             The stability story, and the design statement this kernel exists to make: it is
///             `discreet.h`'s inversion carried into a *performed* effect. delay.h caps feedback
///             strictly below 1 so the loop is always contractive; discreet.h lets regeneration
///             reach exactly 1.0 because tape_loop.h `wear` (darken -> bounded saturation -> DC
///             block) is the stabilizer. Here regeneration is allowed to go *past* unity, into
///             deliberate sound-on-sound self-oscillation — the dub move, the reason anyone
///             reaches for this machine live — and it stays bounded for the same reason:
///             `vca::swing_shape` is bounded by 1/drive for any drive > 0, so whatever the loop
///             gain, the tape is bounded by |in|max + regen/drive and the output by the head
///             levels times that. Because that bound *only* exists while the saturator is
///             engaged, the cap is drive-dependent, applied per sample:
///
///                 drive > 0 : regen may reach k_regen_max_driven (howls, bounded)
///                 drive = 0 : regen is capped at 1.0 (wear is then linear with |H| <= 1, so
///                             1.0 sustains and cannot grow — the discreet.h contract)
///
///             Turning drive to 0 while howling therefore lands the loop at 1.0 rather than
///             letting it run away; the *target* keeps its high value and takes effect again
///             when drive returns.
///
///             Geometry: prepare(sr, max_span_seconds) buys the worst-case tape once (4 s at
///             48 kHz is ~1.5 MB of double tape). No later call allocates; setters only retarget
///             ramps and are safe while audio runs. Double-precision, per-sample, mono in /
///             stereo out.
///
///             Honest limits:
///             - `set_span_ms` glides as a tape-speed change and audibly bends pitch while
///               moving — by design, inherited from discreet.h: moving the heads IS the doppler.
///               Use set_smooth_ms to choose how fast the motor changes speed. There is no
///               crossfading "digital" mode.
///             - The transport is periodic only (see tape_loop.h): deterministic, bit-exactly
///               reproducible, with no stochastic capstan drift. That is a family-wide decision,
///               not an oversight — changing it would break the family's bit-exact renders.
///             - Wow/flutter is a single position offset shared by every head. One motor moves
///               the whole tape path, so a speed error displaces all the heads together; the
///               per-head phase differences of a real transport are not modeled.
///             - Regeneration is taken from the same post-level head sum that feeds the output,
///               so a head's level is also its send into the loop (as the head selector on the
///               machines is). There is no separate feedback-source selection.
///             - The read floor is 2.5 samples (Hermite support) and reads are clamped so the
///               wow excursion can never cross the record head; extreme wow at very short spans
///               flattens against that clamp rather than wrapping.
///             - Heads sum with no master gain: four heads at unity can sum past unity, and gain
///               staging is the caller's job (the multitap contract).
/// @author     Timothy Place
// SPDX-License-Identifier: MIT
// Copyright 2026 Timothy Place.

#pragma once

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>

#include "tape_loop.h" // tap::tools::tape — reel / wow_flutter / wear / ramp, the shared machinery

namespace tap::tools {
    namespace tapecho {

        constexpr int    k_max_heads           = 4;     // four slots; a three-head layout is heads=3
        constexpr double k_min_span_ms         = 1.0;   // below this it is a comb, not an echo
        constexpr double k_default_max_seconds = 4.0;   // default worst-case buy (~1.5 MB @ 48k)
        constexpr double k_default_span_ms     = 375.0; // a plausible motor-speed default
        constexpr double k_regen_max_driven    = 1.5;   // past unity: sound-on-sound, bounded by the saturator
        constexpr double k_regen_max_linear    = 1.0;   // drive 0: |H_wear| <= 1, so 1.0 sustains, cannot grow
        constexpr double k_default_regen       = 0.35;  // a few audible repeats
        constexpr double k_default_darken_hz   = 4000.0;
        constexpr double k_default_drive       = 0.5; // mild record-head saturation
        constexpr double k_default_wow_ms      = 0.4; // a smaller, faster transport than discreet.h's
        constexpr double k_default_wow_hz      = 1.2;
        constexpr double k_default_flutter_ms  = 0.03;
        constexpr double k_default_flutter_hz  = 11.0;
        constexpr double k_default_mix         = 35.0; // an effect in a chain, not a wet-only loop
        constexpr double k_default_smooth_ms   = 20.0; // anti-zipper ramp for setters

        /// One playback head: a position along the tape path, a level, and a place in the stereo
        /// field, all slewed. Reads a reel it does not own (the machine owns the tape).
        class head {
          public:
            head() {
                m_ratio.snap(1.0);
                m_level.snap(1.0);
                m_pan.snap(0.0);
            }

            /// Snap the ramps to their targets — a DSP restart is not a parameter move.
            void prepare() {
                m_ratio.snap(m_ratio.target());
                m_level.snap(m_level.target());
                m_pan.snap(m_pan.target());
            }

            /// Position along the head path as a fraction of the motor span, clamped to (0, 1].
            void set_ratio(double r, long smooth) { m_ratio.to(std::clamp(r, k_min_ratio, 1.0), smooth); }

            /// Linear level. Unclamped: negative flips polarity, as a mixer channel does.
            void set_level(double lin, long smooth) { m_level.to(lin, smooth); }

            /// Equal-power pan, -1 (hard left) .. 1 (hard right). Endpoints are exact.
            void set_pan(double p, long smooth) { m_pan.to(std::clamp(p, -1.0, 1.0), smooth); }

            double ratio() const { return m_ratio.target(); }
            double level() const { return m_level.target(); }
            double pan() const { return m_pan.target(); }

            /// Advance the ramps one sample WITHOUT reading. Disabled heads still tick, so
            /// enabling one mid-glide continues its ramp instead of jumping (the count is a
            /// mute, not a freeze).
            void tick() {
                m_ratio.tick();
                m_level.tick();
                m_pan.tick();
            }

            /// Advance the ramps and read: accumulate this head's panned contribution onto the
            /// stereo busses and return its post-level mono value (what the regeneration path
            /// sums). `span_samples` is the motor span; `offset` the shared transport error.
            double read(const tape::reel& reel, long write_pos, double span_samples, double offset, double& out_left,
                        double& out_right) {
                const double ratio = m_ratio.tick();
                const double level = m_level.tick();
                const double pan   = m_pan.tick();

                // Clamped so the Hermite support can never cross the record head (discreet.h).
                const double span = span_samples * ratio - offset;
                const double d = std::clamp(span, tape::k_min_frac_delay, static_cast<double>(reel.capacity()) - 2.0);
                const double g = level * reel.read_hermite(static_cast<double>(write_pos) - d);

                // Equal-power with exact endpoints — the delay.h multitap law: a hard-panned head
                // is bitwise absent from the far bus.
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
                return g;
            }

          private:
            static constexpr double k_min_ratio = 0.001; // a head at the record head is not a head

            tape::ramp m_ratio; // (0, 1] of the motor span
            tape::ramp m_level; // linear
            tape::ramp m_pan;   // -1..1
        };

        /// The machine: one reel, one transport, one wear stage, k_max_heads heads.
        class machine {
          public:
            /// Defaults are a tape echo, not a neutral bypass: four evenly spaced heads on a
            /// 375 ms motor, a few repeats, gentle wear, the transport breathing.
            machine() {
                m_span_ms.snap(k_default_span_ms);
                m_regen.snap(k_default_regen);
                m_darken_hz.snap(k_default_darken_hz);
                m_drive.snap(k_default_drive);
                m_input_level.snap(1.0);
                m_mix.snap(k_default_mix);
                m_transport.set_wow(k_default_wow_ms, k_default_wow_hz);
                m_transport.set_flutter(k_default_flutter_ms, k_default_flutter_hz);
                for (int i = 0; i < k_max_heads; ++i) {
                    // Nominal even spacing along the path, the last head at the motor span.
                    m_heads[static_cast<size_t>(i)].set_ratio(static_cast<double>(i + 1) / k_max_heads, 0);
                }
            }

            // -- lifecycle -----------------------------------------------------------------------

            /// (Re)allocate tape for `max_span_seconds` at `sr`, snap all ramps, erase the tape.
            /// Not real-time-safe.
            void prepare(double sr, double max_span_seconds = k_default_max_seconds) {
                m_sr = (sr > 0.0) ? sr : 48000.0;
                m_reel.prepare(m_sr, std::max(k_min_span_ms * 0.001, max_span_seconds));
                m_transport.prepare(m_sr);
                m_wear.prepare(m_sr);
                m_span_ms.snap(std::min(m_span_ms.target(), max_span_ms()));
                m_regen.snap(m_regen.target());
                m_darken_hz.snap(m_darken_hz.target());
                m_drive.snap(m_drive.target());
                m_input_level.snap(m_input_level.target());
                m_mix.snap(m_mix.target());
                for (auto& h : m_heads) {
                    h.prepare();
                }
                m_wear.set_cutoff_hz(m_darken_hz.current());
                m_wear.set_drive(m_drive.current());
                clear();
            }

            /// Erase the tape and the wear/transport state; parameters are untouched. The eject
            /// button — it is also how you stop a self-oscillating loop instantly.
            void clear() {
                m_reel.clear();
                m_wear.clear();
                m_transport.clear();
                m_write = 0;
            }

            bool prepared() const { return m_reel.prepared(); }

            // -- parameter targets (click-free; safe while audio runs) ---------------------------

            /// Motor speed as the delay of a ratio-1.0 head, in ms, clamped to [k_min_span_ms,
            /// the prepared max]. Slewed — and the slew IS the varispeed doppler.
            void set_span_ms(double ms) {
                m_span_ms.to(std::clamp(ms, k_min_span_ms, max_span_ms()), smooth_samples());
            }

            /// Number of active heads, clamped to [0, k_max_heads]. A mute, not a freeze: inactive
            /// heads keep ramping, so enabling one mid-glide does not jump.
            void set_heads(int count) { m_num_heads = std::clamp(count, 0, k_max_heads); }

            /// Per-head position along the path, (0, 1] of the motor span, slewed. 0-based index.
            void set_head_ratio(int head, double ratio) {
                if (valid_head(head)) {
                    m_heads[static_cast<size_t>(head)].set_ratio(ratio, smooth_samples());
                }
            }

            /// Per-head linear level, slewed. Also the head's send into the regeneration path.
            void set_head_level(int head, double lin) {
                if (valid_head(head)) {
                    m_heads[static_cast<size_t>(head)].set_level(lin, smooth_samples());
                }
            }

            /// Per-head equal-power pan, -1..1, slewed. Endpoints are bitwise exact.
            void set_head_pan(int head, double pan) {
                if (valid_head(head)) {
                    m_heads[static_cast<size_t>(head)].set_pan(pan, smooth_samples());
                }
            }

            /// Regeneration into the record head, clamped to [0, k_regen_max_driven], slewed.
            /// Values above 1.0 self-oscillate and are only reached while drive > 0 (see the
            /// header banner: the saturator is what bounds them).
            void set_regen(double r) { m_regen.to(std::clamp(r, 0.0, k_regen_max_driven), smooth_samples()); }

            /// Per-pass darkening corner in Hz (tape_loop.h wear band), slewed.
            void set_darken_hz(double hz) {
                m_darken_hz.to(std::clamp(hz, tape::k_darken_floor_hz, tape::k_darken_ceil_hz), smooth_samples());
            }

            /// Record-head saturation drive, >= 0, slewed. 0 is exactly linear — and caps the
            /// effective regeneration at 1.0 for as long as it stays there.
            void set_drive(double d) { m_drive.to(std::max(0.0, d), smooth_samples()); }

            /// Input level into the record head, linear, slewed. Fading this while the loop
            /// self-oscillates is the sound-on-sound performance move.
            void set_input_level(double lin) { m_input_level.to(lin, smooth_samples()); }

            /// Dry/wet mix 0..100, equal-power, slewed. 0 is bitwise dry on both busses, 100 wet.
            void set_mix(double pct) { m_mix.to(std::clamp(pct, 0.0, 100.0), smooth_samples()); }

            /// Wow: depth in ms of tape position, rate in Hz. Instant transport config, not ramped.
            void set_wow(double depth_ms, double rate_hz) { m_transport.set_wow(depth_ms, rate_hz); }

            /// Flutter: the faster, shallower partner. Instant transport config, not ramped.
            void set_flutter(double depth_ms, double rate_hz) { m_transport.set_flutter(depth_ms, rate_hz); }

            /// Anti-zipper ramp time for the setters, in ms. 0 = instant (useful for tests).
            void set_smooth_ms(double ms) { m_smooth_ms = std::max(0.0, ms); }

            // -- introspection -------------------------------------------------------------------

            double span_ms() const { return m_span_ms.target(); }
            int    heads() const { return m_num_heads; }
            double head_ratio(int head) const {
                return valid_head(head) ? m_heads[static_cast<size_t>(head)].ratio() : 0.0;
            }
            double head_level(int head) const {
                return valid_head(head) ? m_heads[static_cast<size_t>(head)].level() : 0.0;
            }
            double head_pan(int head) const {
                return valid_head(head) ? m_heads[static_cast<size_t>(head)].pan() : 0.0;
            }
            double regen() const { return m_regen.target(); }
            double darken_hz() const { return m_darken_hz.target(); }
            double drive() const { return m_drive.target(); }
            double input_level() const { return m_input_level.target(); }
            double mix() const { return m_mix.target(); }
            double wow_depth_ms() const { return m_transport.wow_depth_ms(); }
            double wow_rate_hz() const { return m_transport.wow_rate_hz(); }
            double flutter_depth_ms() const { return m_transport.flutter_depth_ms(); }
            double flutter_rate_hz() const { return m_transport.flutter_rate_hz(); }
            double smooth_ms() const { return m_smooth_ms; }
            double max_span_ms() const { return static_cast<double>(m_reel.capacity()) * 1000.0 / m_sr; }
            double samplerate() const { return m_sr; }

            // -- audio ---------------------------------------------------------------------------

            void process(double in, double& out_left, double& out_right) {
                if (!prepared()) {
                    out_left = out_right = in;
                    return;
                }
                const double span   = m_span_ms.tick() * 0.001 * m_sr;
                const double regen  = m_regen.tick();
                const double darken = m_darken_hz.tick();
                const double drive  = m_drive.tick();
                const double send   = m_input_level.tick();
                const double mix    = m_mix.tick();

                if (darken != m_wear.cutoff_hz()) {
                    m_wear.set_cutoff_hz(darken);
                }
                if (drive != m_wear.drive()) {
                    m_wear.set_drive(drive);
                }

                // One motor: a single transport error displaces every head together.
                const double offset = m_transport.tick();

                double wet_left  = 0.0;
                double wet_right = 0.0;
                double head_sum  = 0.0;
                for (int i = 0; i < k_max_heads; ++i) {
                    head& h = m_heads[static_cast<size_t>(i)];
                    if (i < m_num_heads) {
                        head_sum += h.read(m_reel, m_write, span, offset, wet_left, wet_right);
                    }
                    else {
                        h.tick(); // muted, not frozen
                    }
                }

                // Past unity only while the saturator is there to bound it (header banner).
                const double regen_eff = std::min(regen, (drive > 0.0) ? k_regen_max_driven : k_regen_max_linear);

                // The return path: head sum -> wear (darken, saturate, DC block) -> record head.
                m_reel.write(m_write, send * in + regen_eff * m_wear.process(head_sum));
                if (++m_write >= m_reel.capacity()) { // keep the head in [0, capacity): a long can
                    m_write = 0;                      // overflow in half a day of audio on LLP64
                }

                // Equal-power dry/wet with exact endpoints (delay.h law: 0 bitwise dry, 100 wet).
                if (mix <= 0.0) {
                    out_left = out_right = in;
                    return;
                }
                if (mix >= 100.0) {
                    out_left  = wet_left;
                    out_right = wet_right;
                    return;
                }
                const double theta = mix * 0.01 * (tape::k_pi * 0.5);
                const double dry_g = std::cos(theta);
                const double wet_g = std::sin(theta);
                out_left           = dry_g * in + wet_g * wet_left;
                out_right          = dry_g * in + wet_g * wet_right;
            }

            /// Block form: the trivial loop over the scalar path.
            void process(const double* in, double* out_left, double* out_right, size_t n) {
                for (size_t i = 0; i < n; ++i) {
                    process(in[i], out_left[i], out_right[i]);
                }
            }

          private:
            static bool valid_head(int head) { return head >= 0 && head < k_max_heads; }
            long        smooth_samples() const { return static_cast<long>(m_smooth_ms * 0.001 * m_sr); }

            double                        m_sr{48000.0};
            double                        m_smooth_ms{k_default_smooth_ms};
            int                           m_num_heads{k_max_heads};
            long                          m_write{0};
            tape::reel                    m_reel;
            tape::wow_flutter             m_transport;
            tape::wear                    m_wear;
            std::array<head, k_max_heads> m_heads;
            tape::ramp                    m_span_ms;     // ms, the motor
            tape::ramp                    m_regen;       // 0..k_regen_max_driven
            tape::ramp                    m_darken_hz;   // Hz
            tape::ramp                    m_drive;       // >= 0
            tape::ramp                    m_input_level; // linear
            tape::ramp                    m_mix;         // 0..100
        };

    } // namespace tapecho
} // namespace tap::tools
