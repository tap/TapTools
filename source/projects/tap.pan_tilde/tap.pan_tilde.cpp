/// @file
/// @brief      tap.pan~ — stereo panner.
/// @details    Pans a mono signal between left and right outputs. The position runs from -1 (hard
///             left) through 0 (center) to 1 (hard right); internally this maps to 0..1 and drives
///             an equal-power (cos/sin), linear, or square-root gain curve. Faithful to Jamoma's
///             TTPanorama "calculate" formulas. Position may be driven by the second (signal)
///             inlet, or set via the `position` attribute / a float.
/// @author     Timothy Place
/// @copyright  Copyright 2001-2026 Timothy Place. Distributed under the New BSD License.

#include "c74_min.h"
#include <cmath>

using namespace c74::min;


class pan : public object<pan>, public sample_operator<2, 2> {
public:
    // Integer-indexed enums, matching the help patcher's umenus (which send the menu index).
    // The shape order matches the original object: equalpower(0), linear(1), squareroot(2).
    enum class shapes : int { equalpower, linear, squareroot, enum_count };
    enum class modes  : int { lookuptable, calculate, enum_count };

    MIN_DESCRIPTION { "Stereo panner. Pan a mono signal between the left and right outputs using a "
                      "position from -1 (left) through 0 (center) to 1 (right)." };
    MIN_TAGS    { "filters" };
    MIN_AUTHOR  { "Timothy Place" };
    MIN_RELATED { "tap.crossfade~, matrix~, *~" };

    inlet<>  m_in     { this, "(signal) audio input" };
    inlet<>  m_in_pos { this, "(signal/float) pan position, -1 (left) to 1 (right)" };
    outlet<> m_out_l  { this, "(signal) left output", "signal" };
    outlet<> m_out_r  { this, "(signal) right output", "signal" };

    enum_map shapes_range = { "equalpower", "linear", "squareroot" };
    enum_map modes_range  = { "lookuptable", "calculate" };

    attribute<shapes> shape { this, "shape", shapes::equalpower, shapes_range,
        description { "Panning curve: equalpower, linear, or squareroot." }
    };

    attribute<modes> mode { this, "mode", modes::lookuptable, modes_range,
        description { "Computation method. Provided for compatibility with the original object; "
                      "both settings produce identical results in this implementation, which "
                      "always computes the curve directly." }
    };

    attribute<number> position { this, "position", 0.0,
        range { -1.0, 1.0 },
        description { "Pan position, from -1 (left) through 0 (center) to 1 (right). "
                      "Overridden by a signal connected to the right inlet." }
    };

    message<threadsafe::yes> number { this, "number", "Set the pan position (-1..1).",
        MIN_FUNCTION { position = args; return {}; }
    };

    samples<2> operator()(sample x, sample pos = 0.0) {
        double p = m_in_pos.has_signal_connection() ? static_cast<double>(pos)
                                                    : static_cast<double>(position);
        p = MIN_CLAMP(p, -1.0, 1.0);
        const double scaled = 0.5 * (p + 1.0);    // map -1..1 to 0..1

        double        wl, wr;
        const shapes  s = shape;
        switch (s) {
            case shapes::linear:
                wl = 1.0 - scaled;
                wr = scaled;
                break;
            case shapes::squareroot:
                wl = std::sqrt(1.0 - scaled);
                wr = std::sqrt(scaled);
                break;
            case shapes::equalpower:
            default: {
                const double rad = scaled * c_half_pi;
                wl = std::cos(rad);
                wr = std::sin(rad);
                break;
            }
        }
        return { x * wl, x * wr };
    }

private:
    static constexpr double c_half_pi { 1.57079632679489661923 };
};


MIN_EXTERNAL(pan);
