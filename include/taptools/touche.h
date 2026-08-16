/// @file
/// @brief      Portable Ondes Martenot intensity-key gain law for tap.touche~ — no Max/Min
///             dependency.
/// @details    The first piece of the Ondes Martenot to land (book/PLAN-ondes.md), and the one
///             that is useful on its own: the *touche d'intensité*, the pressure key Maurice
///             Martenot's left hand rides, reduced to what it actually is — a measured,
///             published, expressive gain curve. Olivier Messiaen called it the instrument's
///             greatest invention. It is a graphite/mica powder bag working as a rheostat (the
///             carbon-microphone principle: compress it and the number of conducting bead paths
///             rises, so resistance falls), and what the player feels is a well-chosen nonlinear
///             spring.
///
///             **The curve is not modelled or fitted — it is the published measurement.**
///             Quartier, Meurisse, Colmars, Frelat & Vaiedelich, "Intensity Key of the Ondes
///             Martenot: An Early Mechanical Haptic Device", *Acta Acustica united with
///             Acustica* 101(2), 421–428, 2015 (doi:10.3813/AAA.918837) measured finger force,
///             key displacement and the resulting sound simultaneously on instrument No. 320,
///             and reports the boundaries of the six musical nuances over the key's travel.
///             Those seven points are `k_table_*` below, and this kernel interpolates them.
///
///             Three findings from that paper drive the design, and each is a decision this file
///             did not get to make:
///
///             - **The input is a position, not a force and not a velocity.** The paper states
///               that the change in sound intensity depends on the key's displacement (and on
///               the force that displacement implies), and explicitly that it does *not* depend
///               on the speed of the gesture. A static, memoryless map is therefore the finding
///               rather than a simplification. Force is offered as a secondary input because the
///               same table has a force column, not because it is the primary story.
///             - **50 dB over about 4.5 mm.** From 4.3 mm (the instrument's noise floor, 45
///               dB_SPL) to 8.8 mm (95 dB_SPL). For comparison the paper cites most traditional
///               instruments as rarely exceeding 25 dB of per-note dynamic range.
///             - **The shape is not a line.** Equal 8.3 dB steps correspond to displacement
///               steps of 1.0, 0.6, 0.5, 0.4, 0.5 and 1.5 mm — the curve steepens through the
///               middle and flattens hard at the top. Fitting a straight line in dB-against-mm
///               would throw away the entire reason the key is expressive, so the kernel
///               interpolates the table with monotone cubic (Fritsch–Carlson PCHIP) segments,
///               which pass through every measured point and cannot overshoot between them.
///
///             Interpolation is precomputed into a dense table at construction, so `process()`
///             is a lookup and a lerp. Nothing here depends on the sample rate except the
///             anti-zipper ramp.
///
///             Honest limits:
///             - **One instrument.** The measurements are of ondes No. 320, and the paper notes
///               that weight and thickness of the key vary by more than 10 % between instruments.
///               This is that instrument's curve, not a universal constant.
///             - **Pressing only.** The paper says the released-key case was not investigated,
///               so the same curve is used in both directions here. That is an assumption, and
///               it is this file's, not the paper's.
///             - **The dB values are relative, not absolute.** The published numbers are dB_SPL
///               at one metre through that instrument's own amplifier and speaker; the constant
///               92 dB offset the paper reports between its electrical and acoustic scales is
///               rig-specific. What is used here is the *shape*, normalized so full press is
///               0 dB. No absolute level is claimed.
///             - **Below the first point is silence, not extrapolation.** 4.3 mm is where the
///               instrument sits at its own noise floor, so the kernel outputs exact zero below
///               it rather than inventing curve outside the measured domain. Above 8.8 mm it
///               clamps at unity for the same reason. Note the consequence for a 0..1 control:
///               normalized position spans the *physical* 9.5 mm travel the paper describes, so
///               roughly the first 45 % of the throw is silent. That dead zone is the key's own
///               first phase — pure bending of the elastic strip before it reaches the powder
///               bag — not a modelling choice, and it is why the instrument can be played with
///               such sharp attacks: the useful 50 dB lives in 4.5 mm right after it.
///             - The force map covers the same seven points; the paper's observation that dB
///               rises linearly with log(force) holds below about 85 dB_SPL and 1.3 N, and above
///               that the force required climbs steeply (9.6 N at full press — past where a
///               player would stay for long).
/// @author     Timothy Place
// SPDX-License-Identifier: MIT
// Copyright 2026 Timothy Place.

#pragma once

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>

namespace tap::tools {
    namespace touche {

        constexpr double k_default_smooth_ms = 20.0;
        constexpr int    k_points            = 7;    // the published nuance boundaries
        constexpr int    k_table_size        = 1025; // dense lookup resolution (~4.4 um per step)

        /// The measurement, verbatim: Quartier et al. 2015, Table II — mean displacement and
        /// mean finger force at the boundaries of six musical nuances equally distributed over
        /// the instrument's 50 dB dynamic range (means over notes C3a..C3h).
        constexpr std::array<double, k_points> k_table_db = {45.0, 53.3, 61.6, 70.0, 78.3, 86.6, 95.0};
        constexpr std::array<double, k_points> k_table_mm = {4.3, 5.3, 5.9, 6.4, 6.8, 7.3, 8.8};
        constexpr std::array<double, k_points> k_table_n  = {0.39, 0.47, 0.52, 0.62, 0.82, 1.34, 9.60};

        constexpr double k_min_mm = 4.3; // the noise floor: below this the instrument is silent
        constexpr double k_max_mm = 8.8; // full press
        constexpr double k_min_n  = 0.39;
        constexpr double k_max_n  = 9.60;

        // The *physical* travel the player moves through, which is wider than the measured band.
        // Quartier et al.: "all instrumental gestures take place in a displacement of only a few
        // millimetres (about between 3mm and 9.5mm)", and the key's first phase is pure bending
        // of the elastic strip before it even contacts the powder bag. So the bottom of the
        // travel is genuinely silent — normalized position spans the travel, not the measured
        // sub-range, and the dead zone below k_min_mm is the instrument's, not an invention.
        constexpr double k_travel_mm = 9.5;
        constexpr double k_travel_n  = 9.60; // the force axis bottoms at zero for the same reason
        constexpr double k_range_db  = 50.0; // k_table_db.back() - k_table_db.front()

        /// Which measured column drives the gain.
        enum input_mode : int {
            mode_displacement = 0, // millimetres of key travel — the primary map
            mode_force        = 1  // newtons of finger force — the same table's other column
        };

        /// Monotone cubic (Fritsch–Carlson PCHIP) through the published points. Chosen over a
        /// natural spline because it passes through every measurement *and* cannot overshoot
        /// between them — an overshoot here would be a non-monotone gain curve, which is
        /// audible as a dip while pressing harder.
        class pchip {
          public:
            void build(const std::array<double, k_points>& x, const std::array<double, k_points>& y) {
                m_x = x;
                m_y = y;

                std::array<double, k_points - 1> h{}, delta{};
                for (int i = 0; i < k_points - 1; ++i) {
                    h[static_cast<size_t>(i)] = x[static_cast<size_t>(i + 1)] - x[static_cast<size_t>(i)];
                    delta[static_cast<size_t>(i)] =
                        (y[static_cast<size_t>(i + 1)] - y[static_cast<size_t>(i)]) / h[static_cast<size_t>(i)];
                }

                // Interior slopes: weighted harmonic mean, zeroed at any sign change so the
                // interpolant stays monotone (Fritsch & Carlson, SIAM J. Numer. Anal. 17, 1980).
                for (int i = 1; i < k_points - 1; ++i) {
                    const double d0 = delta[static_cast<size_t>(i - 1)];
                    const double d1 = delta[static_cast<size_t>(i)];
                    if (d0 * d1 <= 0.0) {
                        m_m[static_cast<size_t>(i)] = 0.0;
                    }
                    else {
                        const double w1             = 2.0 * h[static_cast<size_t>(i)] + h[static_cast<size_t>(i - 1)];
                        const double w2             = h[static_cast<size_t>(i)] + 2.0 * h[static_cast<size_t>(i - 1)];
                        m_m[static_cast<size_t>(i)] = (w1 + w2) / (w1 / d0 + w2 / d1);
                    }
                }
                m_m[0] = end_slope(h[0], h[1], delta[0], delta[1]);
                m_m[k_points - 1] =
                    end_slope(h[k_points - 2], h[k_points - 3], delta[k_points - 2], delta[k_points - 3]);
            }

            /// Evaluate at t, clamped to the measured domain (callers handle out-of-range policy).
            double at(double t) const {
                const double lo = m_x[0];
                const double hi = m_x[k_points - 1];
                if (t <= lo) {
                    return m_y[0];
                }
                if (t >= hi) {
                    return m_y[k_points - 1];
                }
                int i = 0;
                while (i < k_points - 2 && t >= m_x[static_cast<size_t>(i + 1)]) {
                    ++i;
                }
                const double h  = m_x[static_cast<size_t>(i + 1)] - m_x[static_cast<size_t>(i)];
                const double s  = (t - m_x[static_cast<size_t>(i)]) / h;
                const double s2 = s * s;
                const double s3 = s2 * s;
                // Cubic Hermite basis.
                const double h00 = 2.0 * s3 - 3.0 * s2 + 1.0;
                const double h10 = s3 - 2.0 * s2 + s;
                const double h01 = -2.0 * s3 + 3.0 * s2;
                const double h11 = s3 - s2;
                return h00 * m_y[static_cast<size_t>(i)] + h10 * h * m_m[static_cast<size_t>(i)]
                       + h01 * m_y[static_cast<size_t>(i + 1)] + h11 * h * m_m[static_cast<size_t>(i + 1)];
            }

          private:
            /// One-sided three-point end slope, limited so the end segment stays monotone.
            static double end_slope(double h0, double h1, double d0, double d1) {
                double m = ((2.0 * h0 + h1) * d0 - h0 * d1) / (h0 + h1);
                if (m * d0 <= 0.0) {
                    m = 0.0;
                }
                else if (d0 * d1 <= 0.0 && std::abs(m) > std::abs(3.0 * d0)) {
                    m = 3.0 * d0;
                }
                return m;
            }

            std::array<double, k_points> m_x{}, m_y{}, m_m{};
        };

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

        /// The intensity key: a position in, a gain out, and an input scaled by it.
        class key {
          public:
            key() {
                m_disp.build(k_table_mm, k_table_db);
                m_force.build(k_table_n, k_table_db);
                rebuild();
                m_position.snap(0.0);
            }

            // -- lifecycle -----------------------------------------------------------------------

            /// Only the anti-zipper ramp depends on the sample rate; the curve does not.
            void prepare(double sr) {
                m_sr = (sr > 0.0) ? sr : 48000.0;
                m_position.snap(m_position.target());
            }

            /// Return the key to rest (silent).
            void clear() { m_position.snap(0.0); }

            // -- parameters ----------------------------------------------------------------------

            /// Key travel as 0..1 over the *physical* range (0 to 9.5 mm), slewed. The measured
            /// band sits inside it: everything below 4.3 mm is silent, because that is where the
            /// key is still bending before it compresses the powder bag. Expect roughly the
            /// first 45 % of the throw to do nothing — that dead zone is the instrument's.
            void set_position(double p) { m_position.to(std::clamp(p, 0.0, 1.0), smooth_samples()); }

            /// The same control in the published unit, millimetres of key travel.
            void set_position_mm(double mm) { set_position(mm / k_travel_mm); }

            /// Finger force in newtons, for `mode_force`. Below k_min_n is silence.
            void set_force_n(double n) { set_position(n / k_travel_n); }

            /// Which measured column the position drives. Changing it rebuilds the dense table;
            /// not real-time-safe.
            void set_mode(int mode) {
                const int v = (mode == mode_force) ? mode_force : mode_displacement;
                if (v != m_mode) {
                    m_mode = v;
                    rebuild();
                }
            }

            /// Anti-zipper ramp time in ms (0 = instant).
            void set_smooth_ms(double ms) { m_smooth_ms = std::max(0.0, ms); }

            // -- introspection -------------------------------------------------------------------

            double position() const { return m_position.target(); }
            double position_mm() const { return m_position.target() * k_travel_mm; }
            int    mode() const { return m_mode; }
            double smooth_ms() const { return m_smooth_ms; }
            double samplerate() const { return m_sr; }

            /// The linear gain the key is currently applying (after the ramp).
            double gain() const { return lookup(m_position.current()); }

            /// The curve itself, for plotting and for callers who want the law without the VCA:
            /// linear gain at a normalized position, 0 dB at full press.
            double gain_at(double p) const { return lookup(std::clamp(p, 0.0, 1.0)); }

            /// The curve in dB relative to full press (-inf at rest).
            double db_at(double p) const {
                const double g = gain_at(p);
                return (g > 0.0) ? 20.0 * std::log10(g) : -std::numeric_limits<double>::infinity();
            }

            // -- audio ---------------------------------------------------------------------------

            /// Message-rate position: the slewed target drives the gain.
            double process(double in) { return in * lookup(m_position.tick()); }

            /// Signal-rate position override (a pedal, a sensor, a control signal). Snaps the
            /// ramp so a later message-rate move continues from here without a jump.
            double process(double in, double position) {
                m_position.snap(std::clamp(position, 0.0, 1.0));
                return in * lookup(m_position.current());
            }

            void process(const double* in, double* out, size_t n) {
                for (size_t i = 0; i < n; ++i) {
                    out[i] = process(in[i]);
                }
            }

          private:
            /// Fill the dense linear-gain table from whichever measured column is selected. The
            /// published dB values are referenced to full press, so the top of the curve is
            /// exactly unity and the bottom is exactly -50 dB.
            /// The dense table spans the *measured band only* — 4.3..8.8 mm, or 0.39..9.60 N —
            /// so entry 0 is exactly the first published point. The silent dead zone below the
            /// band is handled in lookup() rather than by zeroing entries here: a hard zero
            /// adjacent to the floor puts a cliff in the table, and a query landing on the floor
            /// then lerps toward it and reads several dB low. (Measured: -54.2 dB instead of the
            /// published -50.0 at the first force point. Caught by the reproduce-the-table test,
            /// which is exactly what that test is for.)
            void rebuild() {
                const bool   force = (m_mode == mode_force);
                const pchip& curve = force ? m_force : m_disp;
                const double lo    = force ? k_min_n : k_min_mm;
                const double hi    = force ? k_max_n : k_max_mm;
                // Band edges cached in the *normalized* domain, so a caller who converts
                // millimetres the same way this does lands exactly on the edge rather than a
                // rounding error below it.
                m_floor_p = lo / (force ? k_travel_n : k_travel_mm);
                m_top_p   = hi / (force ? k_travel_n : k_travel_mm);
                for (int i = 0; i < k_table_size; ++i) {
                    const double q                 = static_cast<double>(i) / (k_table_size - 1);
                    const double db                = curve.at(lo + q * (hi - lo)) - k_table_db[k_points - 1];
                    m_gain[static_cast<size_t>(i)] = std::pow(10.0, db / 20.0);
                }
            }

            /// Position (0..1 of the physical travel) to linear gain. Below the measured floor
            /// the answer is exact zero — that region is real travel the instrument spends
            /// silent, and extrapolating curve into it would be inventing data. Inside the band
            /// it is a dense-table lookup with a linear step between entries.
            double lookup(double p) const {
                const double q = std::clamp(p, 0.0, 1.0);
                // Strictly below the floor is silence; *at* the floor is the first published
                // point (-50 dB), not zero — 4.3 mm is a measurement, not the edge of nothing.
                if (q < m_floor_p) {
                    return 0.0;
                }
                if (q >= m_top_p) {
                    return m_gain[k_table_size - 1];
                }
                const double t = (q - m_floor_p) / (m_top_p - m_floor_p) * (k_table_size - 1);
                const double f = std::floor(t);
                const int    i = static_cast<int>(f);
                const double a = t - f;
                return m_gain[static_cast<size_t>(i)] * (1.0 - a) + m_gain[static_cast<size_t>(i + 1)] * a;
            }

            long smooth_samples() const { return static_cast<long>(m_smooth_ms * 0.001 * m_sr); }

            double                           m_sr{48000.0};
            double                           m_smooth_ms{k_default_smooth_ms};
            int                              m_mode{mode_displacement};
            pchip                            m_disp, m_force;
            std::array<double, k_table_size> m_gain{};
            double                           m_floor_p{0.0}, m_top_p{1.0};
            ramp                             m_position;
        };

    } // namespace touche
} // namespace tap::tools
