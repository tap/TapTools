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
public:
    // The shape and mode attributes are integer-indexed enums, matching the original object and
    // the help patcher's umenus (which send the menu index).
    enum class shapes : int { equalpower, linear, enum_count };
    enum class modes  : int { lookuptable, calculate, enum_count };

    MIN_DESCRIPTION { "Crossfade between two signals. The position runs from 0 (input A) to 1 "
                      "(input B), using either an equal-power or a linear curve." };
    MIN_TAGS    { "filters" };
    MIN_AUTHOR  { "Timothy Place" };
    MIN_RELATED { "tap.pan~, matrix~, *~, +~" };

    inlet<>  m_in_a   { this, "(signal) input A" };
    inlet<>  m_in_b   { this, "(signal) input B" };
    inlet<>  m_in_pos { this, "(signal/float) crossfade position, 0 (A) to 1 (B)" };
    outlet<> m_out    { this, "(signal) the crossfaded output", "signal" };

    enum_map shapes_range = { "equalpower", "linear" };
    enum_map modes_range  = { "lookuptable", "calculate" };

    attribute<shapes> shape { this, "shape", shapes::equalpower, shapes_range,
        description { "Crossfade curve: equal-power or linear." }
    };

    attribute<modes> mode { this, "mode", modes::lookuptable, modes_range,
        description { "Computation method. Provided for compatibility with the original object; "
                      "both settings produce identical results in this implementation, which "
                      "always computes the curve directly." }
    };

    attribute<number> position { this, "position", 0.5,
        range { 0.0, 1.0 },
        description { "Crossfade position, from 0 (input A) to 1 (input B). "
                      "Overridden by a signal connected to the right inlet." }
    };

    message<threadsafe::yes> number { this, "number", "Set the crossfade position (0..1).",
        MIN_FUNCTION { position = args; return {}; }
    };

    sample operator()(sample a, sample b, sample pos = 0.5) {
        double p = m_in_pos.has_signal_connection() ? static_cast<double>(pos)
                                                    : static_cast<double>(position);
        p = MIN_CLAMP(p, 0.0, 1.0);

        double wa, wb;
        if (shape == shapes::equalpower) {
            const double rad = p * c_half_pi;
            wa = std::cos(rad);
            wb = std::sin(rad);
        }
        else {
            wa = 1.0 - p;
            wb = p;
        }
        return a * wa + b * wb;
    }

private:
    static constexpr double c_half_pi { 1.57079632679489661923 };
};


MIN_EXTERNAL(crossfade);
