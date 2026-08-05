/// @file
/// @brief      Portable delay kernels for tap.delay~ (line) and tap.multitap~ (multitap) — no
///             Max/Min dependency.
/// @details    A kernel-first rebuild of the 1999-lineage delay pair. The legacy objects wrapped
///             ttblue's tt_delay / tt_multitap: integer-sample delays computed by *truncation*
///             (`delay_samples = long(delay_ms * sr / 1000)`), control-rate parameter stepping,
///             a hard-clipped feedback path, and a mono tap sum. This rebuild keeps the two
///             surfaces but modernizes the plumbing to the house pattern:
///
///             - Delays are FRACTIONAL by default (4-point, 3rd-order Hermite — the same read the
///               family uses in grm_comb.h), standard fractional-delay practice (Laakso, Valimaki,
///               Karjalainen, Laine, "Splitting the Unit Delay", IEEE SP Mag. 1996). `set_interp(0)`
///               restores the legacy integer-sample truncation bit-for-bit for A/B against the old
///               binaries; `set_interp(1)` (default) is the modern path.
///             - Every continuous parameter (time, feedback, mix, per-tap time/gain/pan) rides a
///               per-sample linear ramp — no zippers (the param ramp house pattern, grm_comb.h).
///             - The feedback loop carries a DC blocker normalized to unity peak gain (the
///               grm_comb.h house pattern: the raw (1 - z^-1)/(1 - R z^-1) peaks at 2/(1+R) > 1 at
///               Nyquist), and feedback is capped at k_fb_max = 0.99, so the loop is strictly
///               contractive at every setting: a DC step recirculates once and then decays instead
///               of accumulating.
///             - line's dry/wet mix (0..100) and multitap's per-tap pan (-1..1) are equal-power
///               (cos/sin quarter-cycle), with the endpoints snapped exactly (mix 0 is bitwise dry,
///               mix 100 bitwise wet; pan -1 is bitwise silent on the right).
///
///             Geometry: prepare(sr, max_ms) buys the worst case — one buffer of
///             ceil(max_ms * sr / 1000) + 4 samples per line (one shared buffer serves all 100
///             multitap taps). No later call allocates; setters only retarget ramps and are safe
///             while audio runs. All processing is double-precision, per-sample.
///
///             Honest limits:
///             - The structure reads before it writes (feedback needs the read first), so a
///               zero-time tap is unreachable: truncation mode floors at 1 sample, Hermite mode at
///               k_min_frac_delay (2.5) samples (~52 us at 48 kHz — the interpolator needs its
///               youngest support point already written).
///             - The signal-rate override process(in, time_ms) snaps the time ramp to the incoming
///               value each sample (the host's signal is assumed already smooth); it cancels any
///               pending set_time_ms slew.
///             - set_interp switches instantly and can click mid-signal; it is a configuration
///               choice, not a performance control.
///             - Truncation mode with a moving time zipper-steps by whole samples *by design*
///               (that is the bit-compat contract); Hermite mode is the fix, not a smoother
///               truncation.
///             - Feedback is a plain repeat-echo coefficient (cap 0.99), not a calibrated ring
///               time — for RT60-mapped resonance see grm_comb.h.
///             - multitap sums up to 100 unity-capable taps with no dry path and no master gain;
///               output can exceed unity and gain staging is the caller's job.
/// @author     Timothy Place
// SPDX-License-Identifier: MIT
// Copyright 2003-2026 Timothy Place.

#pragma once

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <vector>

namespace tap::tools {
    namespace delay {

        constexpr int    k_max_taps       = 100;   // tap.multitap~'s MAX_NUM_TAPS since 2004
        constexpr double k_fb_max         = 0.99;  // feedback cap (loop strictly contractive)
        constexpr double k_min_frac_delay = 2.5;   // Hermite floor: support points must be written
        constexpr long   k_min_int_delay  = 1;     // truncation floor: read-before-write structure
        constexpr double k_dc_block_r     = 0.999; // in-loop DC blocker pole (~7 Hz corner @ 48k)
        // Normalized to unity peak gain so the loop gain is bounded by fb alone (grm_comb.h).
        constexpr double k_dc_block_norm     = (1.0 + k_dc_block_r) * 0.5;
        constexpr double k_default_smooth_ms = 20.0; // anti-zipper ramp for setters
        constexpr double k_pi                = 3.14159265358979323846;

        enum interp_mode : int {
            interp_trunc   = 0, // legacy integer-sample truncation (bit-compat with tt_delay)
            interp_hermite = 1  // 4-point 3rd-order Hermite fractional taps (default)
        };

        /// Per-sample linear parameter ramp — the anti-zipper unit every setter retargets.
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

        /// Circular delay buffer shared by both kernels: write head plus Hermite / truncated reads.
        class delay_buffer {
          public:
            /// Allocate for the worst case; +4 samples of headroom for the Hermite support points.
            void prepare(double sr, double max_ms) {
                m_max_samples = std::max(8.0, std::ceil(std::max(0.0, max_ms) * 0.001 * sr));
                m_buffer.assign(static_cast<size_t>(m_max_samples) + 4, 0.0);
                m_write = 0;
            }

            void clear() { std::fill(m_buffer.begin(), m_buffer.end(), 0.0); }

            bool   prepared() const { return !m_buffer.empty(); }
            double max_samples() const { return m_max_samples; }

            void write(double x) {
                m_buffer[m_write] = x;
                if (++m_write >= m_buffer.size()) {
                    m_write = 0;
                }
            }

            /// 4-point, 3rd-order Hermite, d samples behind the write head; needs d > 2 strictly
            /// (clamped to k_min_frac_delay by the callers). Same read as grm_comb.h.
            double read_hermite(double d) const {
                const double pos  = static_cast<double>(m_write) - d;
                const double fpos = std::floor(pos);
                const double frac = pos - fpos;
                const long   base = static_cast<long>(fpos);
                const double xm1  = m_buffer[wrap(base - 1)];
                const double x0   = m_buffer[wrap(base)];
                const double x1   = m_buffer[wrap(base + 1)];
                const double x2   = m_buffer[wrap(base + 2)];
                const double c    = (x1 - xm1) * 0.5;
                const double v    = x0 - x1;
                const double w    = c + v;
                const double a    = w + v + (x2 - x0) * 0.5;
                const double b    = w + a;
                return (((a * frac - b) * frac + c) * frac + x0);
            }

            /// Integer-sample read, d whole samples behind the write head (legacy truncation path).
            double read_int(long d) const { return m_buffer[wrap(static_cast<long>(m_write) - d)]; }

          private:
            size_t wrap(long i) const {
                const long n = static_cast<long>(m_buffer.size());
                return static_cast<size_t>(((i % n) + n) % n);
            }

            std::vector<double> m_buffer;
            size_t              m_write{0};
            double              m_max_samples{0.0};
        };

        /// Single feedback delay — the kernel behind tap.delay~.
        class line {
          public:
            // -- lifecycle -----------------------------------------------------------------------

            /// (Re)allocate the buffer for `max_ms` at `sr`, snap all ramps (a DSP restart is not
            /// a parameter move), and clear signal state. Not real-time-safe.
            void prepare(double sr, double max_ms) {
                m_sr = (sr > 0.0) ? sr : 48000.0;
                m_buffer.prepare(m_sr, max_ms);
                m_time.snap(m_time.target());
                m_feedback.snap(m_feedback.target());
                m_mix.snap(m_mix.target());
                clear();
            }

            /// Zero the delay line and the loop filter state; parameters are untouched.
            void clear() {
                m_buffer.clear();
                m_dc_x1 = m_dc_y1 = 0.0;
            }

            // -- parameter targets (click-free; safe while audio runs) ---------------------------

            /// Delay time in ms, slewed. Floors at 1 sample (truncation) / 2.5 samples (Hermite);
            /// ceiling is the prepared max_ms.
            void set_time_ms(double ms) { m_time.to(std::max(0.0, ms), smooth_samples()); }

            /// Feedback coefficient, clamped to [0, k_fb_max], slewed.
            void set_feedback(double fb) { m_feedback.to(std::clamp(fb, 0.0, k_fb_max), smooth_samples()); }

            /// Dry/wet mix 0..100, equal-power, slewed. 0 is bitwise dry, 100 bitwise wet.
            void set_mix(double pct) { m_mix.to(std::clamp(pct, 0.0, 100.0), smooth_samples()); }

            /// interp_trunc (0) or interp_hermite (1, default). Switches instantly — may click.
            void set_interp(int mode) { m_interp = (mode == interp_trunc) ? interp_trunc : interp_hermite; }

            /// Anti-zipper ramp time for the setters, in ms. 0 = instant (useful for tests).
            void set_smooth_ms(double ms) { m_smooth_ms = std::max(0.0, ms); }

            // -- introspection -------------------------------------------------------------------

            double time_ms() const { return m_time.target(); }
            double feedback() const { return m_feedback.target(); }
            double mix() const { return m_mix.target(); }
            int    interp() const { return m_interp; }
            double smooth_ms() const { return m_smooth_ms; }
            double max_ms() const { return m_buffer.max_samples() * 1000.0 / m_sr; }
            double samplerate() const { return m_sr; }

            // -- audio ---------------------------------------------------------------------------

            /// Message-rate time: the slewed set_time_ms target drives the tap.
            double process(double in) { return tick(in, m_time.tick()); }

            /// Signal-rate time override: `time_ms` drives the tap directly this sample (and snaps
            /// the time ramp so a later message-rate move continues from here without a jump).
            double process(double in, double time_ms) {
                m_time.snap(std::max(0.0, time_ms));
                return tick(in, m_time.current());
            }

          private:
            double tick(double in, double time_ms) {
                if (!m_buffer.prepared()) {
                    return in;
                }
                const double fb  = m_feedback.tick();
                const double mix = m_mix.tick();

                const double d_samples = time_ms * 0.001 * m_sr;
                double       delayed   = 0.0;
                if (m_interp == interp_hermite) {
                    delayed = m_buffer.read_hermite(std::clamp(d_samples, k_min_frac_delay, m_buffer.max_samples()));
                }
                else {
                    // Legacy truncation: long(ms * sr / 1000), exactly tt_delay's k_delay_ms map.
                    const long d = std::clamp(static_cast<long>(d_samples), k_min_int_delay,
                                              static_cast<long>(m_buffer.max_samples()));
                    delayed      = m_buffer.read_int(d);
                }

                // Feedback path: delayed -> normalized DC blocker -> * fb -> back into the line.
                const double dc_out = k_dc_block_norm * (delayed - m_dc_x1) + k_dc_block_r * m_dc_y1;
                m_dc_x1             = delayed;
                m_dc_y1             = anti_denormal(dc_out);

                m_buffer.write(anti_denormal(in + fb * dc_out));

                // Equal-power dry/wet with exact endpoints (mix 0 == in bitwise, 100 == wet bitwise).
                if (mix <= 0.0) {
                    return in;
                }
                if (mix >= 100.0) {
                    return delayed;
                }
                const double theta = mix * 0.01 * (k_pi * 0.5);
                return std::cos(theta) * in + std::sin(theta) * delayed;
            }

            static double anti_denormal(double x) { return (std::abs(x) < 1e-15) ? 0.0 : x; }

            long smooth_samples() const { return static_cast<long>(m_smooth_ms * 0.001 * m_sr); }

            double       m_sr{48000.0};
            double       m_smooth_ms{k_default_smooth_ms};
            int          m_interp{interp_hermite};
            delay_buffer m_buffer;
            ramp         m_time;                     // ms
            ramp         m_feedback;                 // 0..k_fb_max
            ramp         m_mix;                      // 0..100
            double       m_dc_x1{0.0}, m_dc_y1{0.0}; // DC blocker: y = norm*(x - x1) + R*y1
        };

        /// Up to 100 feedforward taps off one shared line, each with slewed time / linear gain /
        /// equal-power pan, summed to stereo — the kernel behind tap.multitap~.
        class multitap {
          public:
            multitap() {
                for (auto& t : m_taps) {
                    t.gain.snap(1.0);
                }
            }

            // -- lifecycle -----------------------------------------------------------------------

            /// (Re)allocate the shared buffer for `max_ms` at `sr`, snap all ramps, clear state.
            /// Not real-time-safe.
            void prepare(double sr, double max_ms) {
                m_sr = (sr > 0.0) ? sr : 48000.0;
                m_buffer.prepare(m_sr, max_ms);
                for (auto& t : m_taps) {
                    t.time.snap(t.time.target());
                    t.gain.snap(t.gain.target());
                    t.pan.snap(t.pan.target());
                }
                clear();
            }

            void clear() { m_buffer.clear(); }

            // -- parameter targets (click-free; safe while audio runs) ---------------------------

            /// Number of active taps, clamped to [0, k_max_taps]. Newly activated taps come in at
            /// their stored targets (defaults: time 0 -> the delay floor, gain 1, pan center).
            void set_taps(int count) { m_num_taps = std::clamp(count, 0, k_max_taps); }

            /// Per-tap delay time in ms, slewed; same floors/ceiling as line. `tap` is 0-based.
            void set_time_ms(int tap, double ms) {
                if (valid_tap(tap)) {
                    m_taps[static_cast<size_t>(tap)].time.to(std::max(0.0, ms), smooth_samples());
                }
            }

            /// Per-tap linear gain, slewed. Unclamped (negative flips polarity, like a mixer).
            void set_gain(int tap, double gain) {
                if (valid_tap(tap)) {
                    m_taps[static_cast<size_t>(tap)].gain.to(gain, smooth_samples());
                }
            }

            /// Per-tap equal-power pan, -1 (hard left) .. 1 (hard right), slewed. Endpoints are
            /// exact: pan -1 contributes bitwise zero to the right bus (and mirrored).
            void set_pan(int tap, double pan) {
                if (valid_tap(tap)) {
                    m_taps[static_cast<size_t>(tap)].pan.to(std::clamp(pan, -1.0, 1.0), smooth_samples());
                }
            }

            /// interp_trunc (0) or interp_hermite (1, default), for all taps. May click on switch.
            void set_interp(int mode) { m_interp = (mode == interp_trunc) ? interp_trunc : interp_hermite; }

            void set_smooth_ms(double ms) { m_smooth_ms = std::max(0.0, ms); }

            // -- introspection -------------------------------------------------------------------

            int    taps() const { return m_num_taps; }
            double time_ms(int tap) const {
                return valid_tap(tap) ? m_taps[static_cast<size_t>(tap)].time.target() : 0.0;
            }
            double gain(int tap) const { return valid_tap(tap) ? m_taps[static_cast<size_t>(tap)].gain.target() : 0.0; }
            double pan(int tap) const { return valid_tap(tap) ? m_taps[static_cast<size_t>(tap)].pan.target() : 0.0; }
            int    interp() const { return m_interp; }
            double smooth_ms() const { return m_smooth_ms; }
            double samplerate() const { return m_sr; }

            // -- audio ---------------------------------------------------------------------------

            /// Sum the active taps to the stereo bus. Pure feedforward — no dry path, no feedback.
            void process(double in, double& out_left, double& out_right) {
                out_left  = 0.0;
                out_right = 0.0;
                if (!m_buffer.prepared()) {
                    return;
                }
                for (int i = 0; i < m_num_taps; ++i) {
                    tap_state&   t         = m_taps[static_cast<size_t>(i)];
                    const double d_samples = t.time.tick() * 0.001 * m_sr;
                    double       delayed   = 0.0;
                    if (m_interp == interp_hermite) {
                        delayed =
                            m_buffer.read_hermite(std::clamp(d_samples, k_min_frac_delay, m_buffer.max_samples()));
                    }
                    else {
                        const long d = std::clamp(static_cast<long>(d_samples), k_min_int_delay,
                                                  static_cast<long>(m_buffer.max_samples()));
                        delayed      = m_buffer.read_int(d);
                    }
                    const double g   = t.gain.tick() * delayed;
                    const double pan = t.pan.tick();
                    // Equal-power with exact endpoints (a hard-panned tap is bitwise absent from
                    // the far bus).
                    if (pan <= -1.0) {
                        out_left += g;
                    }
                    else if (pan >= 1.0) {
                        out_right += g;
                    }
                    else {
                        const double theta = (pan + 1.0) * 0.25 * k_pi;
                        out_left += std::cos(theta) * g;
                        out_right += std::sin(theta) * g;
                    }
                }
                m_buffer.write(in);
            }

          private:
            struct tap_state {
                ramp time; // ms
                ramp gain; // linear
                ramp pan;  // -1..1
            };

            bool valid_tap(int tap) const { return tap >= 0 && tap < k_max_taps; }
            long smooth_samples() const { return static_cast<long>(m_smooth_ms * 0.001 * m_sr); }

            double                            m_sr{48000.0};
            double                            m_smooth_ms{k_default_smooth_ms};
            int                               m_interp{interp_hermite};
            int                               m_num_taps{0};
            delay_buffer                      m_buffer;
            std::array<tap_state, k_max_taps> m_taps;
        };

    } // namespace delay
} // namespace tap::tools
