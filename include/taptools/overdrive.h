/// @file
/// @brief      Portable overdrive/saturation kernel for tap.overdrive~ — no Max/Min dependency.
/// @details    A voiced feedback soft-clipper chasing the *class* of TS-lineage feedback
///             overdrives (the Mad Professor Little Green Wonder was the listening reference),
///             rather than a memoryless waveshaper. The old Jamoma TTOverdrive that tap.overdrive~
///             wrapped was a static odd-function curve applied to the full spectrum; this is its
///             spiritual successor, not a port — the design goals and their rationale live in the
///             project's overdrive handoff notes. Design points:
///
///             - The clipping stage is a nonlinearity inside a *feedback loop with a lowpass*,
///               emulating the frequency-dependent loop gain of an op-amp/diode-feedback stage:
///               low frequencies see a pinned small gain (bass stays tight and nearly clean at
///               any drive), mids/highs see the full drive gain and break up first. A static
///               curve cannot do this; it is most of the perceptual distance to the reference.
///             - The loop is solved with the house fast one-pass scheme (tap.svf~ driven circuit,
///               tap.ladder~ solver_fast): a linear zero-delay (trapezoidal/TPT) prediction of
///               the loop node, then the saturated value is committed to the one-pole state.
///               No one-sample delay in the loop, so no limit-cycling at high loop gain —
///               w = shape(G*x - g_fb*LP(w)) with the LP integrated implicitly.
///             - shape() is u/sqrt(1+u^2): smooth (C-inf, so no curvature discontinuity to
///               alias), monotonic, asymptotic to +-1, one sqrt — vectorizes as rsqrt and is
///               LUT-able for a later fixed-point port. Asymmetry (even harmonics — the "warmth"
///               the odd-only Jamoma curves structurally lacked) comes from a bias added inside
///               the shaper, output-referred-corrected so silence stays at zero.
///             - A unity clean path is summed around the clipper (y = x + w), the non-inverting
///               op-amp topology's "never fully flattens" trait: the transfer keeps rising with
///               reduced slope instead of plateauing at a rail.
///             - The clipper runs oversampled (1/2/4/8x, default 4x): zero-stuff + 4th-order
///               Butterworth anti-image up, matching anti-alias before decimation (the
///               tap.ladder~ / tap.svf~ pattern, self-contained here per house rule).
///             - "body" is the LGW-signature voicing control, all linear pre/post EQ (not part of
///               the nonlinearity): it slides the pre-clipper highpass corner (CW = thinner,
///               tighter lows into the clipper), adds an upper-mid push CW (a higher mid center
///               than the classic TS hump) and a slight treble lift CCW.
///             - A DC blocker (R = 0.9997, the Jamoma TTDCBlock constant, via tap.dcblock~) sits
///               after the clipper: with asymmetry > 0 the shaper generates DC that the feedback
///               one-pole would otherwise integrate, so it is always on, not a user option.
///             - All musical parameters are normalized (drive/asymmetry 0..1, body -1..+1) with
///               the perceptual mapping done internally (drive sweeps the shaper gain in dB) —
///               deliberate, both for mappability and so the parameter block translates directly
///               to Q15/Q31 on a future fixed-point embedded target. Gains (preamp/output) are
///               the only unit-bearing parameters, in dB.
///             - The voicing constants below (k_voice_*) are placeholders tuned against LGW
///               demos by ear during the in-Max voicing pass — they are the "sound" of the
///               object and are expected to be retouched there, not derived analytically.
///             - The kernel itself is multichannel (tick() once per frame, process(channel, x)
///               per channel — the tap.svf~ frame protocol); the Max wrapper runs it
///               single-channel and leaves multichannel to mc. wrapping.
///
///             As in the other TapTools kernels: parameters ride per-sample linear ramps (no
///             zipper), and everything is allocation-free after prepare(); setters are safe while
///             audio runs.
/// @author     Timothy Place
// SPDX-License-Identifier: BSD-3-Clause
// Copyright 2026 Timothy Place.

#pragma once

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <vector>

namespace tap::tools {
    namespace od {

        constexpr double k_pi                = 3.14159265358979323846;
        constexpr double k_gain_range_db     = 24.0; // preamp/output range
        constexpr double k_default_smooth_ms = 20.0;

        // Drive mapping: drive 0..1 sweeps the shaper (mid-band) gain linearly in dB.
        // 0 is edge-of-breakup (a pedal's gain knob at CCW, still warm), 1 is saturated.
        constexpr double k_drive_min_db = 6.0;
        constexpr double k_drive_max_db = 46.0;

        // Feedback voicing: the low-frequency loop gain is pinned at k_lf_gain regardless of
        // drive (g_fb = G/k_lf_gain - 1), the TS trait of bass staying nearly clean while mids
        // get the full G; the loop lowpass corner sets where the transition sits.
        constexpr double k_lf_gain     = 2.0;
        constexpr double k_loop_lp_hz  = 660.0;
        constexpr double k_clean_level = 1.0; // unity clean path summed around the clipper

        // Asymmetry: bias inside the shaper at full asymmetry, in units of the clip knee.
        constexpr double k_bias_max = 0.5;

        // Output: fixed trim for the summed clean+clipped path, plus a drive-tracking makeup cut
        // so the perceived level stays roughly constant across the drive sweep.
        constexpr double k_out_trim_db   = -6.0;
        constexpr double k_drive_comp_db = 10.0;

        // Voicing constants — placeholders pending the by-ear voicing pass against LGW demos.
        constexpr double k_voice_hp_min_hz  = 40.0;   // pre-clipper highpass corner, body full CCW
        constexpr double k_voice_hp_max_hz  = 320.0;  // ... body full CW (thin, tight lows)
        constexpr double k_voice_mid_hz     = 1150.0; // upper-mid push center (higher than a TS hump)
        constexpr double k_voice_mid_q      = 0.8;
        constexpr double k_voice_mid_db     = 4.5; // mid push at body full CW
        constexpr double k_voice_mid_fix_db = 1.5; // fixed mid voicing, body-independent
        constexpr double k_voice_shelf_hz   = 3500.0;
        constexpr double k_voice_shelf_db   = 2.5;    // treble lift at body full CCW
        constexpr double k_dc_r             = 0.9997; // DC blocker (Jamoma TTDCBlock, via tap.dcblock~)

        enum param_index : int {
            p_drive = 0, // 0..1, normalized; internally a dB sweep of the shaper gain
            p_body,      // -1..+1 voicing tilt: CCW full lows + slight treble lift, CW thin/tight lows + mid push
            p_asymmetry, // 0..1 even-harmonic amount (0 = odd-only)
            p_preamp,    // input gain, dB
            p_output,    // makeup gain, dB
            k_num_params
        };

        /// Clamp a value to the legal range of a parameter.
        inline double clamp_param(int index, double value) {
            switch (index) {
            case p_drive:
                return std::clamp(value, 0.0, 1.0);
            case p_body:
                return std::clamp(value, -1.0, 1.0);
            case p_asymmetry:
                return std::clamp(value, 0.0, 1.0);
            case p_preamp:
                return std::clamp(value, -k_gain_range_db, k_gain_range_db);
            case p_output:
                return std::clamp(value, -k_gain_range_db, k_gain_range_db);
            default:
                return value;
            }
        }

        class overdrive {
          public:
            overdrive() {
                static constexpr double k_defaults[k_num_params] = {0.35, 0.0, 0.15, 0.0, 0.0};
                for (int i = 0; i < k_num_params; ++i) {
                    m_ramp[i].current = m_ramp[i].target = k_defaults[i];
                }
                m_state.resize(1);
            }

            // -- lifecycle -------------------------------------------------------------------------------

            /// Set the sample rate and channel count, (re)configure oversampling, clear state, snap ramps.
            /// Allocates (the per-channel state vector) — call from the main thread, not the perform loop.
            void prepare(double sr, int channels = 1) {
                m_sr       = (sr > 0.0) ? sr : 48000.0;
                m_channels = std::max(1, channels);
                m_state.assign(static_cast<size_t>(m_channels), channel_state{});
                configure_resampler();
                snap();
            }

            /// Zero all filter, loop, and resampler state in every channel; parameters untouched.
            void clear() {
                for (auto& c : m_state) {
                    c.clear();
                }
            }

            /// Jump all parameter ramps to their targets.
            void snap() {
                for (auto& r : m_ramp) {
                    r.current   = r.target;
                    r.inc       = 0.0;
                    r.remaining = 0;
                }
                m_ramps_active = 0;
                m_dirty        = true;
            }

            int    channels() const { return m_channels; }
            double samplerate() const { return m_sr; }

            // -- structural settings (not ramped) --------------------------------------------------------

            /// Oversampling factor 1, 2, 4, or 8 for the clipping stage (default 4). Takes effect
            /// immediately; resampler state is cleared.
            void set_oversample(int os) {
                const int v = (os >= 8) ? 8 : (os >= 4) ? 4 : (os >= 2) ? 2 : 1;
                if (v != m_os) {
                    m_os = v;
                    configure_resampler();
                    for (auto& c : m_state) {
                        c.up.reset();
                        c.down.reset();
                    }
                    m_dirty = true;
                }
            }
            int oversample() const { return m_os; }

            void   set_smooth_ms(double ms) { m_smooth_ms = std::max(0.0, ms); }
            double smooth_ms() const { return m_smooth_ms; }

            // -- parameter targets (click-free; safe while audio runs) ------------------------------------

            void set_param(int index, double value) {
                if (index < 0 || index >= k_num_params) {
                    return;
                }
                ramp_to(index, clamp_param(index, value), static_cast<long>(m_smooth_ms * 0.001 * m_sr));
            }

            void set_drive(double v) { set_param(p_drive, v); }
            void set_body(double v) { set_param(p_body, v); }
            void set_asymmetry(double v) { set_param(p_asymmetry, v); }
            void set_preamp(double db) { set_param(p_preamp, db); }
            void set_output(double db) { set_param(p_output, db); }

            double param(int index) const { return (index >= 0 && index < k_num_params) ? m_ramp[index].target : 0.0; }

            // -- audio -----------------------------------------------------------------------------------
            //
            // Frame protocol for multichannel use (the tap.svf~ pattern): call tick() once per sample
            // frame (it advances the parameter ramps and refreshes the derived coefficients), then
            // process(ch, x) once per channel. The mono conveniences below fold the two together.

            /// Advance ramps and refresh the derived coefficients if any parameter moved.
            void tick() {
                tick_ramps();
                if (m_dirty) {
                    update_derived();
                }
            }

            /// Process one sample of one channel using the coefficients computed by the last tick().
            /// Precondition: 0 <= channel < channels().
            double process(int channel, double x) {
                channel_state& c = m_state[static_cast<size_t>(channel)];

                // input gain, then the body pre-voicing highpass (what stays out of this stays clean)
                x *= m_pre_gain;
                c.hp_lp += m_a_hp * (x - c.hp_lp);
                const double xn = x - c.hp_lp;

                // the feedback clipper, inside the oversampled region
                double y;
                if (m_os == 1) {
                    y = core(c, xn);
                }
                else {
                    // zero-stuff + anti-image filter up, core at the high rate, anti-alias + decimate down
                    y = 0.0;
                    for (int j = 0; j < m_os; ++j) {
                        const double up = c.up.tick(j == 0 ? xn * m_os : 0.0);
                        y               = c.down.tick(core(c, up));
                    }
                }

                // DC block (asymmetry generates DC), then the post voicing EQ and makeup gain
                const double d = y - c.dc_x1 + k_dc_r * c.dc_y1;
                c.dc_x1        = y;
                c.dc_y1        = anti_denormal(d);
                return c.shelf.tick(c.mid.tick(d)) * m_post_gain;
            }

            /// Mono conveniences.
            double process(double x) {
                tick();
                return process(0, x);
            }

            void process(const double* in, double* out, size_t n) {
                for (size_t i = 0; i < n; ++i) {
                    out[i] = process(in[i]);
                }
            }

            /// Multichannel block processing: nch channel pointers in and out.
            void process(const double* const* in, double* const* out, int nch, size_t n) {
                const int chans = std::min(nch, m_channels);
                for (size_t i = 0; i < n; ++i) {
                    tick();
                    for (int ch = 0; ch < chans; ++ch) {
                        out[ch][i] = process(ch, in[ch][i]);
                    }
                }
            }

            /// The static transfer curve of the shaper alone (for tests/plots): smooth, monotonic,
            /// asymptotic to +-1.
            static double shape(double u) { return u / std::sqrt(1.0 + u * u); }

          private:
            struct ramp {
                double current{0.0};
                double target{0.0};
                double inc{0.0};
                long   remaining{0};
            };

            // RBJ biquad (Transposed Direct Form II) — lowpass pairs make the 4th-order Butterworth
            // anti-image/anti-alias filters for the oversampling chain (tap.ladder~ / tap.svf~
            // pattern); peaking and high-shelf voice the body control.
            struct biquad {
                double b0{1.0}, b1{0.0}, b2{0.0}, a1{0.0}, a2{0.0};
                double z1{0.0}, z2{0.0};

                void design_lowpass(double fc_norm, double q) { // fc_norm = fc / fs
                    const double w     = 2.0 * k_pi * fc_norm;
                    const double alpha = std::sin(w) / (2.0 * q);
                    const double cw    = std::cos(w);
                    const double a0    = 1.0 + alpha;
                    b0                 = ((1.0 - cw) * 0.5) / a0;
                    b1                 = (1.0 - cw) / a0;
                    b2                 = b0;
                    a1                 = (-2.0 * cw) / a0;
                    a2                 = (1.0 - alpha) / a0;
                }
                void design_peaking(double fc_norm, double q, double gain_db) {
                    const double A     = std::pow(10.0, gain_db / 40.0);
                    const double w     = 2.0 * k_pi * fc_norm;
                    const double alpha = std::sin(w) / (2.0 * q);
                    const double cw    = std::cos(w);
                    const double a0    = 1.0 + alpha / A;
                    b0                 = (1.0 + alpha * A) / a0;
                    b1                 = (-2.0 * cw) / a0;
                    b2                 = (1.0 - alpha * A) / a0;
                    a1                 = b1;
                    a2                 = (1.0 - alpha / A) / a0;
                }
                void design_highshelf(double fc_norm, double gain_db) { // shelf slope S = 1
                    const double A    = std::pow(10.0, gain_db / 40.0);
                    const double w    = 2.0 * k_pi * fc_norm;
                    const double cw   = std::cos(w);
                    const double sa   = std::sin(w) * 0.5 * std::sqrt(2.0); // RBJ alpha at S = 1
                    const double ap1  = A + 1.0;
                    const double am1  = A - 1.0;
                    const double sqA2 = 2.0 * std::sqrt(A) * sa;
                    const double a0   = ap1 - am1 * cw + sqA2;
                    b0                = A * (ap1 + am1 * cw + sqA2) / a0;
                    b1                = -2.0 * A * (am1 + ap1 * cw) / a0;
                    b2                = A * (ap1 + am1 * cw - sqA2) / a0;
                    a1                = 2.0 * (am1 - ap1 * cw) / a0;
                    a2                = (ap1 - am1 * cw - sqA2) / a0;
                }
                double tick(double x) {
                    const double y = b0 * x + z1;
                    z1             = b1 * x - a1 * y + z2;
                    z2             = b2 * x - a2 * y;
                    return y;
                }
                void reset() { z1 = z2 = 0.0; }
            };

            struct butterworth4 {
                biquad s1, s2;
                void   design(double fc_norm) {
                    s1.design_lowpass(fc_norm, 0.54119610);
                    s2.design_lowpass(fc_norm, 1.30656296);
                }
                double tick(double x) { return s2.tick(s1.tick(x)); }
                void   reset() {
                    s1.reset();
                    s2.reset();
                }
            };

            struct channel_state {
                double       hp_lp{0.0}; // body pre-voicing highpass (one-pole lowpass state)
                double       lp_s{0.0};  // loop lowpass TPT state
                double       dc_x1{0.0}, dc_y1{0.0};
                biquad       mid, shelf; // post voicing EQ
                butterworth4 up, down;   // oversampling chain
                void         clear() {
                    hp_lp = lp_s = dc_x1 = dc_y1 = 0.0;
                    mid.reset();
                    shelf.reset();
                    up.reset();
                    down.reset();
                }
            };

            static double anti_denormal(double x) {
                return (std::abs(x) < 1e-15) ? 0.0 : x; // house idiom (tap.comb~)
            }

            void ramp_to(int index, double tgt, long nsamples) {
                ramp&      r   = m_ramp[index];
                const bool was = r.remaining > 0;
                if (nsamples < 1 || tgt == r.current) {
                    r.current   = tgt;
                    r.target    = tgt;
                    r.inc       = 0.0;
                    r.remaining = 0;
                    m_dirty     = true;
                }
                else {
                    r.target    = tgt;
                    r.inc       = (tgt - r.current) / static_cast<double>(nsamples);
                    r.remaining = nsamples;
                }
                m_ramps_active += static_cast<int>(r.remaining > 0) - static_cast<int>(was);
            }

            void tick_ramps() {
                if (m_ramps_active <= 0) {
                    return;
                }
                for (auto& r : m_ramp) {
                    if (r.remaining > 0) {
                        r.current += r.inc;
                        if (--r.remaining == 0) {
                            r.current = r.target;
                            --m_ramps_active;
                        }
                        m_dirty = true;
                    }
                }
            }

            void configure_resampler() {
                if (m_os > 1) {
                    // cut just below the original Nyquist, normalized to the oversampled rate
                    const double fc_norm = 0.45 / m_os;
                    for (auto& c : m_state) {
                        c.up.design(fc_norm);
                        c.down.design(fc_norm);
                    }
                }
            }

            // Refresh everything derived from the (ramped) parameters. Runs only when a parameter
            // actually moved — per sample during a ramp, then never again until the next change.
            void update_derived() {
                const double drive = m_ramp[p_drive].current;
                const double body  = m_ramp[p_body].current;
                const double asym  = m_ramp[p_asymmetry].current;

                // drive: dB sweep of the shaper gain; feedback pins the LF loop gain at k_lf_gain
                const double gain_db = k_drive_min_db + drive * (k_drive_max_db - k_drive_min_db);
                m_shaper_gain        = std::pow(10.0, gain_db / 20.0);
                const double g_fb    = std::max(0.0, m_shaper_gain / k_lf_gain - 1.0);

                // loop lowpass (TPT, at the oversampled rate) and the zero-delay solve constants
                const double g_lp = std::tan(k_pi * k_loop_lp_hz / (m_sr * m_os));
                m_lp_norm         = 1.0 / (1.0 + g_lp);
                m_g_lp            = g_lp;
                m_gfb_lp          = g_fb * m_lp_norm;                      // g_fb / (1+g)
                m_loop_norm       = 1.0 / (1.0 + g_fb * g_lp * m_lp_norm); // 1 / (1+beta)

                m_bias     = k_bias_max * asym;
                m_bias_out = shape(m_bias);

                m_pre_gain = std::pow(10.0, m_ramp[p_preamp].current / 20.0);
                m_post_gain =
                    std::pow(10.0, (m_ramp[p_output].current + k_out_trim_db - drive * k_drive_comp_db) / 20.0);

                // body voicing: pre-clipper highpass corner slides CCW->CW across a log range;
                // post EQ adds the CW upper-mid push and the CCW treble lift
                const double hp_hz =
                    k_voice_hp_min_hz * std::pow(k_voice_hp_max_hz / k_voice_hp_min_hz, 0.5 * (body + 1.0));
                m_a_hp = 1.0 - std::exp(-2.0 * k_pi * hp_hz / m_sr);

                const double mid_db   = k_voice_mid_fix_db + k_voice_mid_db * std::max(0.0, body);
                const double shelf_db = k_voice_shelf_db * std::max(0.0, -body);
                for (auto& c : m_state) {
                    c.mid.design_peaking(k_voice_mid_hz / m_sr, k_voice_mid_q, mid_db);
                    c.shelf.design_highshelf(k_voice_shelf_hz / m_sr, shelf_db);
                }

                m_dirty = false;
            }

            // The feedback clipper for one channel at the (possibly oversampled) processing rate.
            // Linear zero-delay solve of w = shape(G*x - g_fb*v), v = TPT-one-pole(w): predict the
            // loop node with shape() as identity, saturate, commit the saturated value to the
            // one-pole state (the tap.svf~ / tap.ladder~ fast one-pass scheme — shape'() <= 1 only
            // ever reduces the loop gain below the prediction, so the step is stable). The unity
            // clean path summed at the end is the non-inverting topology's never-flat trait.
            double core(channel_state& c, double x) {
                const double w_lin = (m_shaper_gain * x - m_gfb_lp * c.lp_s) * m_loop_norm;
                const double w     = shape(w_lin + m_bias) - m_bias_out;
                const double v     = (m_g_lp * w + c.lp_s) * m_lp_norm;
                c.lp_s             = anti_denormal(2.0 * v - c.lp_s);
                return w + k_clean_level * x;
            }

            // configuration
            double m_sr{48000.0};
            double m_smooth_ms{k_default_smooth_ms};
            int    m_channels{1};
            int    m_os{4};

            // parameters
            std::array<ramp, k_num_params> m_ramp;
            int                            m_ramps_active{0};
            bool                           m_dirty{true};

            // derived
            double m_shaper_gain{1.0};
            double m_g_lp{0.0}, m_lp_norm{1.0}, m_gfb_lp{0.0}, m_loop_norm{1.0};
            double m_bias{0.0}, m_bias_out{0.0};
            double m_pre_gain{1.0}, m_post_gain{1.0};
            double m_a_hp{0.0};

            // per-channel state
            std::vector<channel_state> m_state;
        };

    } // namespace od
} // namespace tap::tools
