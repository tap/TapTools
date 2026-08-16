/// @file
/// @brief      Portable two-stage fuzz/distortion kernel for tap.fuzz~ — no Max/Min dependency.
/// @details    The third Radiohead-family kernel (book/PLAN-radiohead-family.md): the
///             two-stage, tone-stacked distortion of the early-90s British "shred" pedal class —
///             the OK Computer-era dirt behind *Paranoid Android* and *My Iron Lung*. A cascaded
///             clipper pair with a bass / contrast / treble voicing section, and a sibling of
///             overdrive.h rather than a replacement: that object is a *feedback* soft-clipper
///             chasing the TS lineage, this one is the harder, more scooped school.
///
///             Method, and where it comes from: the architecture is the simplified cascade of
///             Yeh, Abel & Smith, "Simplified, Physically-Informed Models of Distortion and
///             Overdrive Guitar Effects Pedals" (Proc. DAFx-07, Bordeaux) — **conditioning
///             filter -> memoryless nonlinearity -> equalization filter**, twice. That paper's
///             own justification is the one this kernel relies on: the diode limiter is really a
///             lowpass whose pole moves with voltage (dVo/dt = (Vi - Vo)/RC - (2 Is/C)
///             sinh(Vo/Vt), from the Shockley model Id = Is(exp(V/Vt) - 1)), its exact ODE is
///             expensive, and approximating it as a static curve between fixed filters is
///             justified perceptually and measured there against real pedals. The paper compares
///             tanh, arctan and a tanh approximation against the tabulated DC curve; this kernel
///             uses the tanh family, with a sharpness parameter so one curve spans soft knee to
///             near-hard clip. The paper's note that a real op-amp stage clips *asymmetrically*,
///             producing even harmonics where an odd-only model predicts none, is why
///             `asymmetry` exists.
///
///             **What is and is not claimed.** This is a recreation of a *class* of circuit —
///             two cascaded clipping stages and a three-control tone section — not a component
///             model of any one pedal. No resistor, capacitor, diode, or corner frequency here
///             is claimed as measured from a unit, and the control names follow the layout that
///             class of pedal conventionally carries rather than asserting what any particular
///             one does. The voicing constants (k_voice_*) are the "sound" of the object, chosen
///             by design and expected to be retouched in an in-Max voicing pass — the same
///             posture overdrive.h takes about its own, and the same posture tapecho.h takes
///             about head spacings.
///
///             Two components and a thin composition, the family's habit:
///             - `stage` — one conditioning highpass, one gain, one memoryless curve (knee
///               sharpness + asymmetry), one equalization lowpass. The DAFx-07 triple, and the
///               only nonlinear thing in the file. Two of them cascade: the first is the
///               op-amp-ish gain stage (softer knee, most of the gain, the asymmetry), the
///               second the shunt-diode limiter (harder knee, unity gain).
///             - `tone` — the voicing: a low shelf, a high shelf, and a mid scoop whose depth is
///               `contrast`. Linear, entirely outside the nonlinearity, RBJ biquads.
///             - `pedal` — input gain, the two stages inside the oversampled region, the DC
///               blocker, the tone stack, output level.
///
///             Aliasing: the clipper pair runs oversampled (1/2/4/8x, default 4x) — zero-stuff
///             plus an 8th-order Butterworth anti-image on the way up, a matching anti-alias
///             before decimation. The house pattern (tap.ladder~ / overdrive.h) uses 4th order
///             there; it was measured here and found to make oversampling *non-monotone* for
///             this kernel, so this file uses 8th (see butterworth8's comment for the numbers). DAFx-07 notes that
///             typical implementations use 8-10x and that residual aliases at 8x and above tend
///             to be masked by the dense spectrum of guitar distortion; 4x is the default here
///             because this kernel's curve is C-infinity rather than a hard corner, and 8 is one
///             setter away when it is not enough.
///
///             Honest limits:
///             - The nonlinearity is static. The pole-moves-with-voltage behaviour of the real
///               limiter is approximated by fixed filters around a fixed curve — that is the
///               DAFx-07 simplification, adopted deliberately, and it is why this is a
///               *simplified physically-informed* model and not a circuit solver.
///             - No component values, no schematic netlist, no claim of matching a unit. If you
///               need a specific pedal, this is not it; it is that pedal's *class*.
///             - Hard settings alias. `edge` near 1 sharpens the knee toward a corner, which is
///               exactly where a static curve is worst; raise `oversample` before blaming the
///               tone controls.
///             - `contrast` is a mid scoop of this kernel's own design. The name is the class's;
///               the curve is not claimed to be anyone's.
///             - Mono, and gain staging is the caller's job past `level`.
/// @author     Timothy Place
// SPDX-License-Identifier: MIT
// Copyright 2026 Timothy Place.

#pragma once

#include <algorithm>
#include <cmath>
#include <cstddef>

namespace tap::tools {
    namespace fuzz {

        constexpr double k_pi                = 3.14159265358979323846;
        constexpr double k_default_smooth_ms = 20.0;
        constexpr double k_dc_r              = 0.9997; // the Jamoma TTDCBlock constant, via tap.dcblock~

        // Gain mapping: `gain` 0..1 sweeps the first stage's drive linearly in dB. The floor is
        // below unity on purpose: the second stage carries its own small-signal gain (the tanh
        // family's slope is knee/tanh(knee), ~2 at the stock knee), so a floor at unity would
        // arrive at the limiter already saturated and the knob would do nothing over most of its
        // travel. Gain staging across a cascade is the whole game; these two numbers are it.
        constexpr double k_gain_min_db    = -12.0;
        constexpr double k_gain_max_db    = 36.0;
        constexpr double k_level_range_db = 24.0;

        // Voicing: the sound of the object, chosen by design rather than measured (see the
        // banner). Retouch these in the in-Max voicing pass, not analytically.
        constexpr double k_voice_stage1_hp_hz = 90.0;   // what reaches the first clipper
        constexpr double k_voice_stage1_lp_hz = 7500.0; // the limiter's embedded lowpass, fixed
        constexpr double k_voice_stage2_hp_hz = 150.0;  // tighter into the second stage
        constexpr double k_voice_stage2_lp_hz = 5200.0;
        constexpr double k_voice_stage1_knee  = 1.6;  // softer: the op-amp-ish stage
        constexpr double k_voice_stage2_knee  = 2.0;  // harder: the shunt limiter, at edge 0
        constexpr double k_voice_edge_knee    = 12.0; // ...and at edge 1
        constexpr double k_voice_stage2_gain  = 0.5;  // fixed drive into the second stage
        constexpr double k_voice_bass_hz      = 180.0;
        constexpr double k_voice_treble_hz    = 2600.0;
        constexpr double k_voice_mid_hz       = 620.0;
        constexpr double k_voice_mid_q        = 0.85;
        constexpr double k_voice_shelf_db     = 12.0; // full-scale bass/treble travel
        constexpr double k_voice_scoop_db     = 14.0; // full-scale contrast scoop

        /// Per-sample linear parameter ramp — the anti-zipper unit, the delay.h shape.
        class ramp {
          public:
            void snap(double v) {
                m_current = m_target = v;
                m_inc                = 0.0;
                m_remaining          = 0;
            }
            void to(double tgt, long n) {
                if (n < 1 || tgt == m_current) {
                    snap(tgt);
                }
                else {
                    m_target    = tgt;
                    m_inc       = (tgt - m_current) / static_cast<double>(n);
                    m_remaining = n;
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
            double m_current{0.0}, m_target{0.0}, m_inc{0.0};
            long   m_remaining{0};
        };

        /// RBJ biquad (transposed direct form II) — the cookbook designs, copied with citation
        /// per the house reuse rule (same struct as overdrive.h; kernels stay self-contained).
        struct biquad {
            double b0{1.0}, b1{0.0}, b2{0.0}, a1{0.0}, a2{0.0};
            double z1{0.0}, z2{0.0};

            void design_lowpass(double fc_norm, double q) {
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
            void design_lowshelf(double fc_norm, double gain_db) { // shelf slope S = 1
                const double A    = std::pow(10.0, gain_db / 40.0);
                const double w    = 2.0 * k_pi * fc_norm;
                const double cw   = std::cos(w);
                const double sa   = std::sin(w) * 0.5 * std::sqrt(2.0);
                const double ap1  = A + 1.0;
                const double am1  = A - 1.0;
                const double sqA2 = 2.0 * std::sqrt(A) * sa;
                const double a0   = ap1 + am1 * cw + sqA2;
                b0                = A * (ap1 - am1 * cw + sqA2) / a0;
                b1                = 2.0 * A * (am1 - ap1 * cw) / a0;
                b2                = A * (ap1 - am1 * cw - sqA2) / a0;
                a1                = -2.0 * (am1 + ap1 * cw) / a0;
                a2                = (ap1 + am1 * cw - sqA2) / a0;
            }
            void design_highshelf(double fc_norm, double gain_db) {
                const double A    = std::pow(10.0, gain_db / 40.0);
                const double w    = 2.0 * k_pi * fc_norm;
                const double cw   = std::cos(w);
                const double sa   = std::sin(w) * 0.5 * std::sqrt(2.0);
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

        /// 8th-order Butterworth lowpass as four cascaded biquads — the oversampling chain's
        /// anti-image and anti-alias filter.
        ///
        /// The house pattern (tap.ladder~ / overdrive.h) uses a 4th-order pair here. It is not
        /// steep enough for this kernel, and the difference is measurable rather than
        /// theoretical: with the 4th-order filter, alias energy fell from 1x to 2x and then rose
        /// again at 4x and 8x — more oversampling made it *worse*, because 24 dB/octave leaves
        /// content just above the base Nyquist barely touched, and a higher factor pushes more
        /// clipper-generated harmonics into that barely-touched band before decimation. Eighth
        /// order restores the monotone improvement the setting promises. (Whether the same change
        /// is owed to overdrive.h is a live question — it is a different nonlinearity at a
        /// different gain structure, so it needs its own measurement, not this one's conclusion.)
        ///
        /// Pole Qs are the standard 8th-order Butterworth set, Q_k = 1/(2 cos((2k+1)pi/16)).
        struct butterworth8 {
            biquad s1, s2, s3, s4;
            void   design(double fc_norm) {
                s1.design_lowpass(fc_norm, 0.50979558);
                s2.design_lowpass(fc_norm, 0.60134489);
                s3.design_lowpass(fc_norm, 0.89997622);
                s4.design_lowpass(fc_norm, 2.56291545);
            }
            double tick(double x) { return s4.tick(s3.tick(s2.tick(s1.tick(x)))); }
            void   reset() {
                s1.reset();
                s2.reset();
                s3.reset();
                s4.reset();
            }
        };

        /// The tanh family with an adjustable knee: `k` small is nearly linear, `k` large
        /// approaches a hard corner. Normalized so shape(1, k) == 1 for every k, which keeps the
        /// stage's gain structure independent of the knee setting.
        inline double shape(double x, double k) {
            if (k < 1e-6) {
                return x;
            }
            return std::tanh(k * x) / std::tanh(k);
        }

        /// One DAFx-07 stage: conditioning highpass -> gain -> memoryless curve -> equalization
        /// lowpass. Allocation-free; coefficients are set by the owner each block.
        class stage {
          public:
            void prepare(double sr, double hp_hz, double lp_hz) {
                m_sr = (sr > 0.0) ? sr : 48000.0;
                set_corners(hp_hz, lp_hz);
                clear();
            }

            /// Recompute the fixed corners for a (possibly oversampled) rate.
            void set_corners(double hp_hz, double lp_hz) {
                m_a_hp = 1.0 - std::exp(-2.0 * k_pi * hp_hz / m_sr);
                m_a_lp = 1.0 - std::exp(-2.0 * k_pi * lp_hz / m_sr);
            }

            void clear() { m_hp = m_lp = 0.0; }

            /// `gain` is linear into the curve, `knee` the sharpness, `bias` the asymmetry that
            /// buys even harmonics. The bias is corrected at the output so silence stays exactly
            /// at zero (the overdrive.h contract).
            double process(double x, double gain, double knee, double bias) {
                m_hp += m_a_hp * (x - m_hp);
                const double u = (x - m_hp) * gain;
                const double y = shape(u + bias, knee) - shape(bias, knee);
                m_lp += m_a_lp * (y - m_lp);
                return m_lp;
            }

          private:
            double m_sr{48000.0};
            double m_a_hp{1.0}, m_a_lp{1.0};
            double m_hp{0.0}, m_lp{0.0};
        };

        /// The voicing section: low shelf, mid scoop, high shelf. Linear, and entirely outside
        /// the nonlinearity — the "equalization filter" of the cascade, at the audio rate.
        class tone {
          public:
            void prepare(double sr) {
                m_sr = (sr > 0.0) ? sr : 48000.0;
                design(0.0, 0.0, 0.0);
                clear();
            }

            void clear() {
                m_low.reset();
                m_mid.reset();
                m_high.reset();
            }

            /// bass/treble in -1..1 (full travel is k_voice_shelf_db either way), contrast in
            /// 0..1 (0 flat, 1 the full mid scoop).
            void design(double bass, double treble, double contrast) {
                m_low.design_lowshelf(k_voice_bass_hz / m_sr, bass * k_voice_shelf_db);
                m_high.design_highshelf(k_voice_treble_hz / m_sr, treble * k_voice_shelf_db);
                m_mid.design_peaking(k_voice_mid_hz / m_sr, k_voice_mid_q, -contrast * k_voice_scoop_db);
            }

            double process(double x) { return m_high.tick(m_mid.tick(m_low.tick(x))); }

          private:
            double m_sr{48000.0};
            biquad m_low, m_mid, m_high;
        };

        /// The pedal: two stages inside the oversampled region, then DC block, tone, level.
        class pedal {
          public:
            pedal() {
                m_gain.snap(0.5);
                m_edge.snap(0.5);
                m_asymmetry.snap(0.0);
                m_bass.snap(0.0);
                m_treble.snap(0.0);
                m_contrast.snap(0.35);
                m_level_db.snap(0.0);
            }

            // -- lifecycle -----------------------------------------------------------------------

            void prepare(double sr) {
                m_sr = (sr > 0.0) ? sr : 48000.0;
                configure();
                m_gain.snap(m_gain.target());
                m_edge.snap(m_edge.target());
                m_asymmetry.snap(m_asymmetry.target());
                m_bass.snap(m_bass.target());
                m_treble.snap(m_treble.target());
                m_contrast.snap(m_contrast.target());
                m_level_db.snap(m_level_db.target());
                m_tone.design(m_bass.current(), m_treble.current(), m_contrast.current());
                clear();
            }

            void clear() {
                m_s1.clear();
                m_s2.clear();
                m_tone.clear();
                m_up.reset();
                m_down.reset();
                m_dc_x1 = m_dc_y1 = 0.0;
            }

            bool prepared() const { return m_sr > 0.0 && m_configured; }

            // -- parameters (click-free; safe while audio runs) ----------------------------------

            /// 0..1, sweeping the first stage's drive linearly in dB.
            void set_gain(double g) { m_gain.to(std::clamp(g, 0.0, 1.0), smooth_samples()); }

            /// 0..1: how sharp the second stage's knee is — soft-ish limiter through to near-hard
            /// clip. High settings alias; raise oversample.
            void set_edge(double e) { m_edge.to(std::clamp(e, 0.0, 1.0), smooth_samples()); }

            /// 0..1 of clipping asymmetry — the even-harmonic control, and the thing an odd-only
            /// static curve structurally cannot produce.
            void set_asymmetry(double a) { m_asymmetry.to(std::clamp(a, 0.0, 1.0), smooth_samples()); }

            void set_bass(double b) { m_bass.to(std::clamp(b, -1.0, 1.0), smooth_samples()); }
            void set_treble(double t) { m_treble.to(std::clamp(t, -1.0, 1.0), smooth_samples()); }

            /// 0..1 mid-scoop depth. This kernel's own curve — see the header.
            void set_contrast(double c) { m_contrast.to(std::clamp(c, 0.0, 1.0), smooth_samples()); }

            /// Output level in dB, +-k_level_range_db.
            void set_level_db(double db) {
                m_level_db.to(std::clamp(db, -k_level_range_db, k_level_range_db), smooth_samples());
            }

            /// 1, 2, 4 or 8. Reconfigures the stage corners and the filters — not real-time-safe.
            void set_oversample(int os) {
                const int v = (os >= 8) ? 8 : (os >= 4) ? 4 : (os >= 2) ? 2 : 1;
                if (v != m_os) {
                    m_os = v;
                    configure();
                    clear();
                }
            }

            void set_smooth_ms(double ms) { m_smooth_ms = std::max(0.0, ms); }

            // -- introspection -------------------------------------------------------------------

            double gain() const { return m_gain.target(); }
            double edge() const { return m_edge.target(); }
            double asymmetry() const { return m_asymmetry.target(); }
            double bass() const { return m_bass.target(); }
            double treble() const { return m_treble.target(); }
            double contrast() const { return m_contrast.target(); }
            double level_db() const { return m_level_db.target(); }
            int    oversample() const { return m_os; }
            double smooth_ms() const { return m_smooth_ms; }
            double samplerate() const { return m_sr; }

            // -- audio ---------------------------------------------------------------------------

            double process(double x) {
                if (!prepared()) {
                    return x;
                }
                const double gain     = m_gain.tick();
                const double edge     = m_edge.tick();
                const double asym     = m_asymmetry.tick();
                const double bass     = m_bass.tick();
                const double treble   = m_treble.tick();
                const double contrast = m_contrast.tick();
                const double level    = m_level_db.tick();

                if (bass != m_tone_bass || treble != m_tone_treble || contrast != m_tone_contrast) {
                    m_tone.design(bass, treble, contrast);
                    m_tone_bass     = bass;
                    m_tone_treble   = treble;
                    m_tone_contrast = contrast;
                }

                const double drive = std::pow(10.0, (k_gain_min_db + gain * (k_gain_max_db - k_gain_min_db)) / 20.0);
                const double knee2 = k_voice_stage2_knee + edge * (k_voice_edge_knee - k_voice_stage2_knee);
                const double bias  = asym * 0.7; // inside the curve; corrected at the stage output

                double y = 0.0;
                if (m_os == 1) {
                    y = core(x, drive, knee2, bias);
                }
                else {
                    // zero-stuff + anti-image up, the clipper pair at the high rate, anti-alias
                    // + decimate down (the tap.ladder~ / overdrive.h chain).
                    for (int j = 0; j < m_os; ++j) {
                        const double up = m_up.tick(j == 0 ? x * m_os : 0.0);
                        y               = m_down.tick(core(up, drive, knee2, bias));
                    }
                }

                // Asymmetry generates DC that the shelves would otherwise pass; always on.
                const double d = y - m_dc_x1 + k_dc_r * m_dc_y1;
                m_dc_x1        = y;
                m_dc_y1        = anti_denormal(d);

                return m_tone.process(m_dc_y1) * std::pow(10.0, level / 20.0);
            }

            void process(const double* in, double* out, size_t n) {
                for (size_t i = 0; i < n; ++i) {
                    out[i] = process(in[i]);
                }
            }

          private:
            /// The two clipping stages, at whatever rate the caller is running.
            double core(double x, double drive, double knee2, double bias) {
                const double a = m_s1.process(x, drive, k_voice_stage1_knee, bias);
                return m_s2.process(a, k_voice_stage2_gain, knee2, 0.0);
            }

            void configure() {
                const double osr = m_sr * m_os;
                m_s1.prepare(osr, k_voice_stage1_hp_hz, k_voice_stage1_lp_hz);
                m_s2.prepare(osr, k_voice_stage2_hp_hz, k_voice_stage2_lp_hz);
                m_tone.prepare(m_sr);
                if (m_os > 1) {
                    // Cut just below the original Nyquist, normalized to the oversampled rate.
                    const double fc_norm = 0.45 / static_cast<double>(m_os);
                    m_up.design(fc_norm);
                    m_down.design(fc_norm);
                }
                m_tone_bass = m_tone_treble = m_tone_contrast = std::nan("");
                m_configured                                  = true;
            }

            static double anti_denormal(double x) { return (std::abs(x) < 1e-15) ? 0.0 : x; }

            long smooth_samples() const { return static_cast<long>(m_smooth_ms * 0.001 * m_sr); }

            double m_sr{48000.0};
            double m_smooth_ms{k_default_smooth_ms};
            int    m_os{4};
            bool   m_configured{false};

            stage        m_s1, m_s2;
            tone         m_tone;
            butterworth8 m_up, m_down;
            double       m_dc_x1{0.0}, m_dc_y1{0.0};

            // Cached tone targets so the biquads are only redesigned when a control actually moves.
            double m_tone_bass{0.0}, m_tone_treble{0.0}, m_tone_contrast{0.0};

            ramp m_gain, m_edge, m_asymmetry, m_bass, m_treble, m_contrast, m_level_db;
        };

    } // namespace fuzz
} // namespace tap::tools
