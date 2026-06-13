/// @file
/// @brief      tap.crossfade~ — crossfade between two signals.
/// @details    Equal-power (out = A·cos(θ) + B·sin(θ), θ = position·π/2) or linear
///             (out = A·(1−position) + B·position) crossfade, with `position` running 0 (all A)
///             to 1 (all B). Faithful to Jamoma's TTCrossfade "calculate" formulas. Position may
///             be driven by the third (signal) inlet, or set via the `position` attribute / a float.
/// @author     Timothy Place
/// @copyright  Copyright 2001-2026 Timothy Place. Distributed under the New BSD License.

#include "c74_min.h"
#include <cmath>

using namespace c74::min;


class crossfade : public object<crossfade>, public sample_operator<3, 1> {
private:
    static constexpr double c_half_pi { 1.57079632679489661923 };

    // Cached state — declared before the attributes so it is initialized before their setters run.
    bool   m_equal_power { true };
    double m_position    { 0.5 };
    double m_weight_a    { 1.0 };
    double m_weight_b    { 0.0 };

public:
    MIN_DESCRIPTION { "Crossfade between two signals. The position runs from 0 (input A) to 1 "
                      "(input B), using either an equal-power or a linear curve." };
    MIN_TAGS    { "filters" };
    MIN_AUTHOR  { "Timothy Place" };
    MIN_RELATED { "tap.pan~, matrix~, *~, +~" };

    inlet<>  m_in_a   { this, "(signal) input A" };
    inlet<>  m_in_b   { this, "(signal) input B" };
    inlet<>  m_in_pos { this, "(signal/float) crossfade position, 0 (A) to 1 (B)" };
    outlet<> m_out    { this, "(signal) the crossfaded output", "signal" };

    attribute<symbol> shape { this, "shape", "equalpower",
        range { "equalpower", "linear" },
        setter { MIN_FUNCTION {
            m_equal_power = !(args[0] == "linear");
            update_weights();
            return args;
        }},
        description { "Crossfade curve: equalpower or linear." }
    };

    attribute<number> position { this, "position", 0.5,
        range { 0.0, 1.0 },
        setter { MIN_FUNCTION {
            m_position = MIN_CLAMP(static_cast<double>(args[0]), 0.0, 1.0);
            update_weights();
            return { m_position };
        }},
        description { "Crossfade position, from 0 (input A) to 1 (input B)." }
    };

    message<threadsafe::yes> number { this, "number", "Set the crossfade position (0..1).",
        MIN_FUNCTION { position = args; return {}; }
    };

    sample operator()(sample a, sample b, sample pos = 0.5) {
        double wa = m_weight_a;
        double wb = m_weight_b;
        if (m_in_pos.has_signal_connection())
            weights_for(MIN_CLAMP(pos, 0.0, 1.0), wa, wb);
        return a * wa + b * wb;
    }

private:
    void weights_for(double p, double& wa, double& wb) const {
        if (m_equal_power) {
            const double rad = p * c_half_pi;
            wa = std::cos(rad);
            wb = std::sin(rad);
        }
        else {
            wa = 1.0 - p;
            wb = p;
        }
    }

    void update_weights() {
        weights_for(m_position, m_weight_a, m_weight_b);
    }
};


MIN_EXTERNAL(crossfade);
