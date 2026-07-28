/// @file
/// @brief      Portable virtual-analog oscillator kernel for tap.vco~ — no Max/Min dependency.
/// @details    The classic analog waveforms with polyBLEP alias suppression, plus the full VCO
///             feature set:
///
///             - One master phase drives everything. saw = 2p-1 with a 2-point polyBLEP at the
///               wrap; pulse = +-1 with polyBLEPs at both edges (pw 1..99%); triangle = leaky
///               integration of the BLEP-corrected square (the classic trick — correctly
///               antialiased with the right amplitude); sine is pure.
///             - `shape` morphs continuously 0 sine -> 1 triangle -> 2 saw -> 3 pulse (linear
///               crossfade of the two adjacent generators, both sharing the master phase), and
///               rides the per-sample ramps, so slow shape sweeps and preset morphs glide
///               through hybrid waveforms.
///             - Through-zero linear FM: the FM input is calibrated in Hz and adds to the
///               effective frequency; the phase increment may go negative (phase runs backward),
///               and the BLEP windows use its magnitude.
///             - Hard sync: a rising zero crossing on the sync input resets the phase with
///               sub-sample accuracy, plus a first-order polynomial correction of the reset
///               discontinuity (one-sided — the reset is unpredictable, so the pre-reset sample
///               cannot be corrected retroactively; minBLEP tables are the flagged upgrade path).
///             - Analog character (all deterministic per `seed`; renders and tests reproduce, and
///               mc. instances decorrelate by seed):
///                 `drift` (0..100 cents) — slow random pitch walk: sample-and-hold noise at ~2 Hz
///                   smoothed by a ~0.5 Hz one-pole. `detune` is a static offset in cents.
///                 `jitter` (0..20 cents) — the fast companion to drift: low-level pitch noise
///                   (~80 Hz sample-and-hold through a ~40 Hz one-pole) that reads as "alive"
///                   rather than chorused. Real VCOs have both time scales.
///                 `imperfect` (0..1) — waveform imperfection, the Minimoog-style departure from
///                   the ideal shapes: the saw ramp takes on an exponential-ish bow (the visible
///                   shark-fin shape; spectrally mostly a phase effect — the bend sits in
///                   quadrature with the saw's own components) and its reset corner is rounded
///                   (a gentle one-pole that closes from ~22 kHz toward
///                   ~8 kHz), the triangle goes slightly asymmetric (even harmonics), the sine
///                   picks up mild waveshaper THD, and the pulse width takes a small static
///                   offset. The *amounts* are scaled by per-unit component "tolerances" drawn
///                   deterministically from `seed` — every seed is a slightly different unit off
///                   the production line, so an mc. stack spreads like real hardware. At 0 the
///                   waveforms are exactly the ideal shapes regardless of seed.
///                 `track` (-10..10 cents/octave) — V/oct calibration error relative to A440:
///                   the pitch offset grows with the distance from the calibration center, like a
///                   real exponential converter drifting away from its trim point.
///
///             As in the other TapTools kernels: per-sample linear ramps on every parameter, a
///             16-slot preset-morph engine, allocation-free processing, setters safe while audio
///             runs.
/// @author     Timothy Place
// SPDX-License-Identifier: MIT
// Copyright 2026 Timothy Place.

#pragma once

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <type_traits>

#include "numeric.h"

namespace tap::tools {
    namespace vco {

        constexpr int    k_presets           = 16;
        constexpr double k_freq_min          = 0.01; // down to LFO rates
        constexpr double k_freq_max          = 20000.0;
        constexpr double k_default_smooth_ms = 20.0;
        constexpr double k_pi                = 3.14159265358979323846;

        enum param_index : int {
            p_gain = 0,  // output gain, dB
            p_frequency, // Hz
            p_shape,     // 0 sine .. 1 triangle .. 2 saw .. 3 pulse (continuous morph)
            p_pw,        // pulse width, 1..99 %
            p_drift,     // slow random pitch walk depth, cents
            p_detune,    // static detune, cents
            p_imperfect, // 0..1 waveform imperfection (Model-D-ish curvature/rounding/asymmetry)
            p_jitter,    // fast pitch noise depth, cents
            p_track,     // V/oct calibration error, cents per octave from A440
            k_num_params
        };

        enum waveform : int { // convenience snap points for the shape parameter
            wave_sine = 0,
            wave_triangle,
            wave_saw,
            wave_pulse,
            k_num_waveforms
        };

        /// One full parameter snapshot — a preset slot, and the unit the morph engine interpolates.
        template <typename Sample>
        struct basic_params {
            std::array<Sample, k_num_params> v{};

            static basic_params defaults() {
                basic_params p;
                p.v[p_gain]      = Sample(0.0);
                p.v[p_frequency] = Sample(220.0);
                p.v[p_shape]     = static_cast<Sample>(wave_saw);
                p.v[p_pw]        = Sample(50.0);
                p.v[p_drift]     = Sample(0.0);
                p.v[p_detune]    = Sample(0.0);
                p.v[p_imperfect] = Sample(0.0);
                p.v[p_jitter]    = Sample(0.0);
                p.v[p_track]     = Sample(0.0);
                return p;
            }
        };

        /// Clamp a value to the legal range of a parameter. Gain (dB) is unclamped.
        /// `Sample` is non-deduced and defaults to double so existing call sites — including
        /// the Max wrapper's, which passes a Min `atom` — keep compiling unchanged.
        template <typename Sample = double>
        Sample clamp_param(int index, std::type_identity_t<Sample> value) {
            switch (index) {
            case p_gain:
                return value;
            case p_frequency:
                return std::clamp(value, k_freq_min, k_freq_max);
            case p_shape:
                return std::clamp(value, Sample(0.0), Sample(3.0));
            case p_pw:
                return std::clamp(value, Sample(1.0), Sample(99.0));
            case p_drift:
                return std::clamp(value, Sample(0.0), Sample(100.0));
            case p_detune:
                return std::clamp(value, -Sample(1200.0), Sample(1200.0));
            case p_imperfect:
                return std::clamp(value, Sample(0.0), Sample(1.0));
            case p_jitter:
                return std::clamp(value, Sample(0.0), Sample(20.0));
            case p_track:
                return std::clamp(value, -Sample(10.0), Sample(10.0));
            default:
                return value;
            }
        }

        template <typename Sample>

        class basic_vco_osc {
            static_assert(is_sample_profile<Sample>,

                          "basic_vco_osc supports the two Tap numeric profiles: float and double");

          public:
            using sample_type = Sample;

            basic_vco_osc() {
                const basic_params<Sample> d = basic_params<Sample>::defaults();
                for (int i = 0; i < k_num_params; ++i) {
                    m_ramp[i].current = m_ramp[i].target = d.v[i];
                }
                m_presets.fill(d);
                compute_tolerances();
            }

            // -- lifecycle -------------------------------------------------------------------------------

            void prepare(Sample sr) {
                m_sr = (sr > Sample(0.0)) ? sr : Sample(48000.0);
                clear();
                snap();
                m_prepared = true;
            }

            /// Reset phase, triangle integrator, sync detector, and noise state; parameters untouched.
            void clear() {
                m_phase       = Sample(0.0);
                m_tri_state   = Sample(0.0);
                m_sync_prev   = Sample(0.0);
                m_pending     = Sample(0.0);
                m_rng         = m_seed;
                m_drift_sh    = Sample(0.0);
                m_drift_lp    = Sample(0.0);
                m_drift_count = 0;
                m_jit_sh      = Sample(0.0);
                m_jit_lp      = Sample(0.0);
                m_jit_count   = 0;
                m_round_lp    = Sample(0.0);
            }

            void snap() {
                for (auto& r : m_ramp) {
                    r.current   = r.target;
                    r.inc       = Sample(0.0);
                    r.remaining = 0;
                }
                m_ramps_active = 0;
            }

            // -- modes ------------------------------------------------------------------------------------

            /// Reseed the noise generators and the per-unit component tolerances (deterministic;
            /// different seeds decorrelate instances — and, with `imperfect` up, make each one a
            /// slightly different "unit off the production line").
            void set_seed(uint32_t seed) {
                m_seed = (seed == 0) ? 1u : seed;
                m_rng  = m_seed;
                compute_tolerances();
            }
            uint32_t seed() const { return m_seed; }

            void   set_smooth_ms(Sample ms) { m_smooth_ms = std::max(Sample(0.0), ms); }
            Sample smooth_ms() const { return m_smooth_ms; }

            // -- parameter targets (click-free; safe while audio runs) --------------------------------------

            void set_param(int index, Sample value) {
                if (index < 0 || index >= k_num_params) {
                    return;
                }
                ramp_to(index, clamp_param(index, value), static_cast<long>(m_smooth_ms * Sample(0.001) * m_sr));
            }

            void set_gain(Sample db) { set_param(p_gain, db); }
            void set_frequency(Sample hz) { set_param(p_frequency, hz); }
            void set_shape(Sample s) { set_param(p_shape, s); }
            void set_pw(Sample pct) { set_param(p_pw, pct); }
            void set_drift(Sample cents) { set_param(p_drift, cents); }
            void set_detune(Sample cents) { set_param(p_detune, cents); }
            void set_imperfect(Sample x) { set_param(p_imperfect, x); }
            void set_jitter(Sample cents) { set_param(p_jitter, cents); }
            void set_track(Sample cents_per_oct) { set_param(p_track, cents_per_oct); }

            /// Snap the shape to one of the classic waveforms.
            void set_waveform(int w) { set_shape(static_cast<Sample>(std::clamp(w, 0, k_num_waveforms - 1))); }

            // -- presets / morph ----------------------------------------------------------------------------

            bool store_preset(int slot) {
                if (!valid_slot(slot)) {
                    return false;
                }
                m_presets[slot] = snap_targets();
                return true;
            }

            bool recall_preset(int slot, Sample seconds) {
                if (!valid_slot(slot)) {
                    return false;
                }
                const long n = static_cast<long>(std::max(Sample(0.0), seconds) * m_sr);
                for (int i = 0; i < k_num_params; ++i) {
                    ramp_to(i, clamp_param(i, m_presets[slot].v[i]), n);
                }
                return true;
            }

            bool set_preset(int slot, const basic_params<Sample>& p) {
                if (!valid_slot(slot)) {
                    return false;
                }
                m_presets[slot] = p;
                return true;
            }

            const basic_params<Sample>& preset(int slot) const {
                return m_presets[static_cast<size_t>(std::clamp(slot, 0, k_presets - 1))];
            }

            bool morphing() const { return m_ramps_active > 0; }

            basic_params<Sample> snap_targets() const {
                basic_params<Sample> p;
                for (int i = 0; i < k_num_params; ++i) {
                    p.v[i] = m_ramp[i].target;
                }
                return p;
            }

            basic_params<Sample> snap_current() const {
                basic_params<Sample> p;
                for (int i = 0; i < k_num_params; ++i) {
                    p.v[i] = m_ramp[i].current;
                }
                return p;
            }

            Sample samplerate() const { return m_sr; }

            // -- audio --------------------------------------------------------------------------------------

            /// One sample: fm_hz adds linearly (through-zero); sync resets on a rising zero crossing.
            Sample process(Sample fm_hz = Sample(0.0), Sample sync = Sample(0.0)) {
                return step(m_ramp[p_frequency].current, fm_hz, sync);
            }

            /// One sample with a signal-rate frequency override (Hz).
            Sample process_at(Sample freq_hz, Sample fm_hz = Sample(0.0), Sample sync = Sample(0.0)) {
                return step(clamp_param(p_frequency, freq_hz), fm_hz, sync);
            }

            void process(Sample* out, size_t n) {
                for (size_t i = 0; i < n; ++i) {
                    out[i] = process();
                }
            }

          private:
            struct ramp {
                Sample current{Sample(0.0)};
                Sample target{Sample(0.0)};
                Sample inc{Sample(0.0)};
                long   remaining{0};
            };

            static bool valid_slot(int s) { return s >= 0 && s < k_presets; }

            void ramp_to(int index, Sample tgt, long nsamples) {
                ramp&      r   = m_ramp[index];
                const bool was = r.remaining > 0;
                if (nsamples < 1 || tgt == r.current) {
                    r.current   = tgt;
                    r.target    = tgt;
                    r.inc       = Sample(0.0);
                    r.remaining = 0;
                }
                else {
                    r.target    = tgt;
                    r.inc       = (tgt - r.current) / static_cast<Sample>(nsamples);
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
                    }
                }
            }

            Sample uniform() { // deterministic white noise in [-1, 1]
                m_rng = m_rng * 1664525u + 1013904223u;
                return (m_rng / Sample(2147483648.0)) - Sample(1.0);
            }

            // Slow random pitch walk in cents: ~2 Hz sample-and-hold through a ~0.5 Hz one-pole.
            Sample tick_drift(Sample depth_cents) {
                if (depth_cents <= Sample(0.0)) {
                    return Sample(0.0);
                }
                if (--m_drift_count <= 0) {
                    m_drift_count = static_cast<int>(m_sr / Sample(2.0));
                    m_drift_sh    = uniform();
                }
                const Sample a = Sample(1.0) - std::exp(-Sample(2.0) * Sample(k_pi_for<Sample>) * Sample(0.5) / m_sr);
                m_drift_lp += a * (m_drift_sh - m_drift_lp);
                return depth_cents * m_drift_lp;
            }

            // Fast pitch noise in cents: ~80 Hz sample-and-hold through a ~40 Hz one-pole — the
            // short-time instability of a real VCO core (drift's fast companion).
            Sample tick_jitter(Sample depth_cents) {
                if (depth_cents <= Sample(0.0)) {
                    return Sample(0.0);
                }
                if (--m_jit_count <= 0) {
                    m_jit_count = std::max(1, static_cast<int>(m_sr / Sample(80.0)));
                    m_jit_sh    = uniform();
                }
                const Sample a = Sample(1.0) - std::exp(-Sample(2.0) * Sample(k_pi_for<Sample>) * Sample(40.0) / m_sr);
                m_jit_lp += a * (m_jit_sh - m_jit_lp);
                return depth_cents * m_jit_lp;
            }

            // Per-unit component tolerances, drawn deterministically from the seed (a separate
            // stream from the runtime noise, so clear() never changes the "unit"). All are scaled
            // by the `imperfect` parameter at use — at imperfect 0 every seed is the ideal unit.
            void compute_tolerances() {
                uint32_t r    = m_seed * 2654435761u + 12345u;
                auto     next = [&r]() {
                    r = r * 1664525u + 1013904223u;
                    return (r / Sample(2147483648.0)) - Sample(1.0);
                };
                m_tol_pw    = next() * Sample(1.5);               // pulse-width offset, % at imperfect 1
                m_tol_tri   = next() * Sample(6.0);               // triangle skew, % at imperfect 1
                m_tol_cents = next() * Sample(2.0);               // static pitch offset, cents at imperfect 1
                m_tol_curve = Sample(0.8) + Sample(0.2) * next(); // per-unit scaling of the saw curvature
            }

            // Canonical 2-point polyBLEP residual around a phase wrap.
            static Sample poly_blep(Sample t, Sample dt) {
                if (t < dt) {
                    t /= dt;
                    return t + t - t * t - Sample(1.0);
                }
                if (t > Sample(1.0) - dt) {
                    t = (t - Sample(1.0)) / dt;
                    return t * t + t + t + Sample(1.0);
                }
                return Sample(0.0);
            }

            static Sample wrap01(Sample p) { return p - std::floor(p); }

            // Ramp curvature: p + bend*p*(1-p) keeps the endpoints (and thus the BLEP step size)
            // fixed while bowing the interior — the exponential-ish discharge of a real saw core.
            static Sample bent(Sample p, Sample bend) { return p + bend * p * (Sample(1.0) - p); }

            Sample saw_at(Sample p, Sample dt, Sample bend) const {
                return Sample(2.0) * bent(p, bend) - Sample(1.0) - poly_blep(p, dt);
            }

            Sample pulse_at(Sample p, Sample dt, Sample pw) const {
                Sample y = (p < pw) ? Sample(1.0) : -Sample(1.0);
                y += poly_blep(p, dt);
                y -= poly_blep(wrap01(p - pw), dt);
                return y;
            }

            // Triangle: leaky integration of the BLEP square. Only ticked when the morph needs it.
            // tri_pw skews the square's duty away from 0.5 (imperfect: asymmetry -> even harmonics).
            Sample tri_tick(Sample p, Sample adt, Sample tri_pw) {
                const Sample sq = pulse_at(p, adt, tri_pw);
                m_tri_state     = Sample(0.999) * m_tri_state + Sample(4.0) * adt * sq;
                return m_tri_state;
            }

            // The morphed waveform at the current phase. adt = |dt| for the BLEP windows.
            Sample waveform_out(Sample p, Sample adt, Sample shape, Sample pw, Sample bend, Sample tri_pw) {
                if (shape <= Sample(1.0)) { // sine -> triangle
                    const Sample a = shape;
                    const Sample s = std::sin(Sample(2.0) * Sample(k_pi_for<Sample>)
                                              * bent(p, Sample(0.5) * bend)); // mild shaper THD
                    if (a <= Sample(0.0)) {
                        return s;
                    }
                    return (Sample(1.0) - a) * s + a * tri_tick(p, adt, tri_pw);
                }
                if (shape <= Sample(2.0)) { // triangle -> saw
                    const Sample a = shape - Sample(1.0);
                    const Sample t = tri_tick(p, adt, tri_pw);
                    if (a <= Sample(0.0)) {
                        return t;
                    }
                    return (Sample(1.0) - a) * t + a * saw_at(p, adt, bend);
                }
                // saw -> pulse
                const Sample a = shape - Sample(2.0);
                const Sample s = saw_at(p, adt, bend);
                if (a <= Sample(0.0)) {
                    return s;
                }
                return (Sample(1.0) - a) * s + a * pulse_at(p, adt, pw);
            }

            Sample step(Sample base_hz, Sample fm_hz, Sample sync) {
                tick_ramps();

                const Sample imp = m_ramp[p_imperfect].current;

                Sample cents = m_ramp[p_detune].current + tick_drift(m_ramp[p_drift].current)
                               + tick_jitter(m_ramp[p_jitter].current) + imp * m_tol_cents;
                const Sample track = m_ramp[p_track].current;
                if (track != Sample(0.0) && base_hz > Sample(0.0)) {
                    cents += track * std::log2(base_hz / Sample(440.0)); // V/oct error from the trim point
                }
                const Sample f_eff = base_hz * std::exp2(cents / Sample(1200.0)) + fm_hz; // through-zero
                Sample       dt    = f_eff / m_sr;
                dt                 = std::clamp(dt, -Sample(0.49), Sample(0.49));
                const Sample adt   = std::max(std::abs(dt), Sample(1.0e-8));

                const Sample shape = m_ramp[p_shape].current;
                const Sample bend  = Sample(0.35) * imp * m_tol_curve;
                const Sample tri_pw =
                    std::clamp(Sample(0.5) + imp * m_tol_tri * Sample(0.01), Sample(0.05), Sample(0.95));
                const Sample pw = std::clamp(m_ramp[p_pw].current * Sample(0.01) + imp * m_tol_pw * Sample(0.01),
                                             Sample(0.01), Sample(0.99));

                // hard sync: rising zero crossing resets the phase at the fractional crossing point
                Sample correction = m_pending;
                m_pending         = Sample(0.0);
                if (m_sync_prev <= Sample(0.0) && sync > Sample(0.0)) {
                    const Sample frac  = m_sync_prev / (m_sync_prev - sync); // 0..1 within this sample
                    const Sample p_old = wrap01(m_phase + dt * frac);
                    const Sample p_new = (Sample(1.0) - frac) * dt;
                    const Sample d =
                        waveform_out_peek(p_old, shape, pw, bend) - waveform_out_peek(wrap01(p_new), shape, pw, bend);
                    // one-sided first-order correction of the reset step (minBLEP is the upgrade path)
                    const Sample x = Sample(1.0) - frac;
                    correction += d * Sample(0.5) * x * x;
                    m_phase = wrap01(p_new);
                }
                else {
                    m_phase = wrap01(m_phase + dt);
                }
                m_sync_prev = sync;

                Sample y = waveform_out(m_phase, adt, shape, pw, bend, tri_pw) + correction;

                // imperfect: rounded reset corner — a gentle one-pole closing from ~22 kHz toward
                // ~8 kHz as the imperfection rises. Exactly bypassed at 0.
                if (imp > Sample(0.0)) {
                    if (imp != m_round_imp) {
                        const Sample fc = std::min(Sample(22000.0) - Sample(14000.0) * imp, Sample(0.45) * m_sr);
                        m_round_a       = Sample(1.0) - std::exp(-Sample(2.0) * Sample(k_pi_for<Sample>) * fc / m_sr);
                        m_round_imp     = imp;
                    }
                    m_round_lp += m_round_a * (y - m_round_lp);
                    y = m_round_lp;
                }
                else {
                    m_round_lp = y; // keep the state primed so engaging imperfect is click-free
                }

                const Sample g = std::pow(Sample(10.0), m_ramp[p_gain].current * Sample(0.05));
                return y * g;
            }

            // Waveform value without advancing the triangle integrator (for sync discontinuity sizing).
            // No adt parameter: unlike waveform_out(), peek adds no BLEP correction (it reads the
            // triangle integrator rather than ticking it), so it has no use for the window width.
            Sample waveform_out_peek(Sample p, Sample shape, Sample pw, Sample bend) const {
                if (shape <= Sample(1.0)) {
                    const Sample a = shape;
                    const Sample s = std::sin(Sample(2.0) * Sample(k_pi_for<Sample>) * bent(p, Sample(0.5) * bend));
                    return (Sample(1.0) - a) * s + a * m_tri_state;
                }
                if (shape <= Sample(2.0)) {
                    const Sample a = shape - Sample(1.0);
                    return (Sample(1.0) - a) * m_tri_state + a * (Sample(2.0) * bent(p, bend) - Sample(1.0));
                }
                const Sample a = shape - Sample(2.0);
                const Sample s = Sample(2.0) * bent(p, bend) - Sample(1.0);
                const Sample q = (p < pw) ? Sample(1.0) : -Sample(1.0);
                return (Sample(1.0) - a) * s + a * q;
            }

            // configuration
            Sample   m_sr{Sample(48000.0)};
            Sample   m_smooth_ms{Sample(k_default_smooth_ms)};
            uint32_t m_seed{1u};
            bool     m_prepared{false};

            // parameters
            std::array<ramp, k_num_params>              m_ramp;
            std::array<basic_params<Sample>, k_presets> m_presets;
            int                                         m_ramps_active{0};

            // state
            Sample   m_phase{Sample(0.0)};
            Sample   m_tri_state{Sample(0.0)};
            Sample   m_sync_prev{Sample(0.0)};
            Sample   m_pending{Sample(0.0)};
            uint32_t m_rng{1u};
            Sample   m_drift_sh{Sample(0.0)};
            Sample   m_drift_lp{Sample(0.0)};
            int      m_drift_count{0};
            Sample   m_jit_sh{Sample(0.0)};
            Sample   m_jit_lp{Sample(0.0)};
            int      m_jit_count{0};
            Sample   m_round_lp{Sample(0.0)};
            Sample   m_round_a{Sample(1.0)};
            Sample   m_round_imp{-Sample(1.0)};

            // per-unit component tolerances (deterministic from the seed; scaled by `imperfect`)
            Sample m_tol_pw{Sample(0.0)};
            Sample m_tol_tri{Sample(0.0)};
            Sample m_tol_cents{Sample(0.0)};
            Sample m_tol_curve{Sample(1.0)};
        };

        using params   = basic_params<double>;
        using params32 = basic_params<float>;

        /// The double profile — the golden model.
        using vco_osc = basic_vco_osc<double>;

        /// The float profile — for single-precision targets. See numeric.h.
        using vco_osc32 = basic_vco_osc<float>;

    } // namespace vco
} // namespace tap::tools
