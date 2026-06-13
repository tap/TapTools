/// @file
/// @brief      tap.pan~ — stereo panner.
/// @details    Pans a mono signal between left and right outputs. The position runs from -1 (hard
///             left) through 0 (center) to 1 (hard right); internally this maps to 0..1 and drives
///             an equal-power (cos/sin), square-root, or linear gain curve. Faithful to Jamoma's
///             TTPanorama "calculate" formulas. Position may be driven by the second (signal)
///             inlet, or set via the `position` attribute / a float.
/// @author     Timothy Place
/// @copyright  Copyright 2001-2026 Timothy Place. Distributed under the New BSD License.

#include "c74_min.h"
#include <cmath>

using namespace c74::min;


class pan : public object<pan>, public sample_operator<2, 2> {
private:
    static constexpr double c_half_pi { 1.57079632679489661923 };

    enum shape_type { equal_power = 0, linear = 1, square_root = 2 };

    // Cached state — declared before the attributes so it is initialized before their setters run.
    int    m_shape    { equal_power };
    double m_position { 0.0 };       // -1 (left) .. 1 (right)
    double m_weight_l { 0.0 };
    double m_weight_r { 0.0 };

public:
    MIN_DESCRIPTION { "Stereo panner. Pan a mono signal between the left and right outputs using a "
                      "position from -1 (left) through 0 (center) to 1 (right)." };
    MIN_TAGS    { "filters" };
    MIN_AUTHOR  { "Timothy Place" };
    MIN_RELATED { "tap.crossfade~, matrix~, *~" };

    inlet<>  m_in     { this, "(signal) audio input" };
    inlet<>  m_in_pos { this, "(signal/float) pan position, -1 (left) to 1 (right)" };
    outlet<> m_out_l  { this, "(signal) left output", "signal" };
    outlet<> m_out_r  { this, "(signal) right output", "signal" };

    attribute<symbol> shape { this, "shape", "equalpower",
        range { "equalpower", "squareroot", "linear" },
        setter { MIN_FUNCTION {
            if (args[0] == "linear")
                m_shape = linear;
            else if (args[0] == "squareroot")
                m_shape = square_root;
            else
                m_shape = equal_power;
            update_weights();
            return args;
        }},
        description { "Panning curve: equalpower, squareroot, or linear." }
    };

    attribute<number> position { this, "position", 0.0,
        range { -1.0, 1.0 },
        setter { MIN_FUNCTION {
            m_position = MIN_CLAMP(static_cast<double>(args[0]), -1.0, 1.0);
            update_weights();
            return { m_position };
        }},
        description { "Pan position, from -1 (left) through 0 (center) to 1 (right)." }
    };

    message<threadsafe::yes> number { this, "number", "Set the pan position (-1..1).",
        MIN_FUNCTION { position = args; return {}; }
    };

    samples<2> operator()(sample x, sample pos = 0.0) {
        double wl = m_weight_l;
        double wr = m_weight_r;
        if (m_in_pos.has_signal_connection())
            weights_for(MIN_CLAMP(pos, -1.0, 1.0), wl, wr);
        return { x * wl, x * wr };
    }

private:
    void weights_for(double p, double& wl, double& wr) const {
        const double scaled = 0.5 * (p + 1.0);     // map -1..1 to 0..1
        switch (m_shape) {
            case linear:
                wl = 1.0 - scaled;
                wr = scaled;
                break;
            case square_root:
                wl = std::sqrt(1.0 - scaled);
                wr = std::sqrt(scaled);
                break;
            case equal_power:
            default: {
                const double rad = scaled * c_half_pi;
                wl = std::cos(rad);
                wr = std::sin(rad);
                break;
            }
        }
    }

    void update_weights() {
        weights_for(m_position, m_weight_l, m_weight_r);
    }
};


MIN_EXTERNAL(pan);
