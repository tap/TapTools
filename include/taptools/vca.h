/// @file
/// @brief      Portable voltage-controlled-amplifier kernel for tap.vca~ — no Max/Min dependency.
/// @details    A gain stage with a selectable circuit model, following the same two-circuit idiom
///             as svf.h (clean vs. driven). The `warm` circuit is the discrete class-A transistor
///             VCA extracted from the TB-303 voice (tb303_voice.h, phase 2) so there is one
///             implementation shared between `tap.303~` and the standalone object:
///
///             - `mode_clean` (default) is the pure linear multiply, x * gain — bit-identical to a
///               `*~`. Cheap, transparent, no added harmonics or DC.
///             - `mode_warm` models a one-transistor class-A VCA stage as a slope-normalized biased
///               saturator applied to the ALREADY-GAINED signal:
///                   S(v) = (tanh(d*v + b) - tanh(b)) / (d * sech^2(b)),   d = drive, b = bias
///               The `- tanh(b)` term removes the operating-point DC and the `1/(d*sech^2(b))`
///               normalization gives unity slope at v = 0, so quiet signals pass essentially clean
///               (a few % harmonic content) while hot signals pick up even harmonics (~11% h2 on a
///               full-scale sine at the stock drive) and audible compression (~-4 dB at full whack).
///               Because the saturator sees the post-gain signal, the warmth TRACKS the control
///               voltage — the discrete-VCA character a linear multiply cannot produce.
///
///             The asymmetric shaper leaves a signal-dependent DC component on AC material (the
///             mean of tanh over a symmetric swing is not tanh(b)); the discrete hardware sheds it
///             through the stage's output coupling capacitor. The kernel models that with an
///             optional one-pole DC block (leaky-integrator high-pass, sub-audio corner) engaged in
///             `warm` mode — matching the hardware order (saturate, then couple out). `tap.303~`
///             does NOT use this path: it composes `shape()` directly and runs its own Open303
///             output-coupling high-pass, so its numbers are untouched by this extraction.
///
///             Defaults (drive 2.0, bias 0.3) are the probe-calibrated 303 constants; both are
///             exposed so the same stage can be driven harder or biased more asymmetrically for
///             non-303 use. Plain C++17, stdlib only, allocation-free, no Max/Min dependency.
/// @author     Timothy Place
/// @copyright  Copyright 2026 Timothy Place. Distributed under the New BSD License.

#pragma once

#include <algorithm>
#include <cmath>

namespace taptools {

    /// Selectable-circuit gain stage: linear multiply, or the 303's class-A transistor saturator.
    class vca {
      public:
        enum mode : int {
            mode_clean = 0, // pure linear multiply — bit-identical to *~
            mode_warm,      // one-transistor class-A stage (biased, slope-normalized tanh)
            k_num_modes
        };

        // Stock 303 transistor-stage constants (tb303_voice.h k_vca_sat_drive / k_vca_sat_bias):
        // probe-calibrated for musical THD. Schematic-derived values are an audition-time refinement.
        static constexpr double k_default_drive = 2.0;
        static constexpr double k_default_bias  = 0.3;
        static constexpr double k_dc_block_hz   = 20.0; // output-coupling corner (warm mode)

        void prepare(double sample_rate) {
            m_dc.set(k_dc_block_hz, sample_rate);
            update();
        }

        void reset() { m_dc.reset(); }

        // -- circuit / parameters ---------------------------------------------------------------
        void set_mode(int m) { m_mode = std::clamp(m, 0, k_num_modes - 1); }
        int  circuit() const { return m_mode; }

        void   set_drive(double d) { m_drive = std::max(1e-6, d); update(); }
        double drive() const { return m_drive; }

        void   set_bias(double b) { m_bias = b; update(); }
        double bias() const { return m_bias; }

        void set_dc_block(bool on) { m_dc_block = on; }
        bool dc_block() const { return m_dc_block; }

        // -- processing -------------------------------------------------------------------------

        /// The transistor-stage saturator applied to an already-gained sample. `clean` is the
        /// identity, so upstream code that multiplies by its own gain gets a bit-identical passthru.
        /// This is the shared primitive tb303_voice.h composes (with its own gain and coupling).
        double shape(double v) const {
            if (m_mode == mode_clean)
                return v;
            return (std::tanh(m_drive * v + m_bias) - m_off) * m_norm;
        }

        /// The full standalone path: apply `gain`, run the circuit, and in `warm` mode couple the
        /// output through the DC block (unless disabled). `gain` is a plain linear multiplier —
        /// feed it a control signal, an envelope, or a constant.
        double process(double x, double gain) {
            const double s = shape(x * gain);
            if (m_mode == mode_warm && m_dc_block)
                return m_dc.tick(s);
            return s;
        }

      private:
        // One-pole leaky-integrator high-pass: y = x - lp, lp += (x - lp) * a. Denormal-flushed.
        struct dc_hp {
            double a{0.0}, lp{0.0};
            void   set(double hz, double sr) { a = 1.0 - std::exp(-2.0 * 3.14159265358979323846 * hz / sr); }
            double tick(double x) {
                lp += (x - lp) * a;
                lp = (std::abs(lp) < 1e-15) ? 0.0 : lp;
                return x - lp;
            }
            void reset() { lp = 0.0; }
        };

        // Recompute the operating-point offset and the unity-slope normalization from drive/bias.
        void update() {
            m_off  = std::tanh(m_bias);
            m_norm = 1.0 / (m_drive * (1.0 - m_off * m_off)); // 1 / (d * sech^2(b))
        }

        int    m_mode{mode_clean};
        double m_drive{k_default_drive};
        double m_bias{k_default_bias};
        double m_off{0.0}, m_norm{1.0};
        bool   m_dc_block{true};
        dc_hp  m_dc;
    };

} // namespace taptools
