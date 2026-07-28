/// @file autowah.h
/// @brief      Portable envelope-filter (auto-wah) kernel for tap.autowah~ — no Max/Min dependency.
/// @details    A behavioral model of the Mad Professor Snow White AutoWah (Björn Juhl's design):
///             a 2-pole state-variable filter whose cutoff is pushed up from a resting point by an
///             envelope follower with a fast attack and a musician-set decay. The hardware is an
///             LM13700 OTA SVF swept by a diode + RC detector; published sweep range 250–2500 Hz.
///             Model (traced circuit facts vs. modeling choices documented per point):
///
///             - Filter core: composes the Simper SVF kernel (svf.h) as a single 2nd-order section,
///               driven per sample through its signal-rate cutoff path (tick(cutoff_hz)) — the TPT
///               core is unconditionally stable under per-sample modulation, which is the whole job
///               here. `mode` selects the lowpass or bandpass tap: the traced circuit has both, the
///               stock voicing is believed to be the resonant lowpass node (inference — flagged in
///               the plan; the default may flip after hardware calibration). Q comes from the
///               normalized resonance (0..1) via the shared svf mapping.
///             - Envelope detector: sensitivity gain (dB) -> rectifier -> one-pole attack/release
///               follower. Attack is fast by default (2 ms, the pedal's fixed-attack character) but
///               exposed; decay is the player control (10 ms .. 5 s, matching the widened
///               1 M-pot revision of the hardware). The rectifier is full-wave by default;
///               half-wave (the traced single-diode topology) is selectable for the hardware's
///               sweep-rate ripple — an A/B for the calibration pass (set_rectifier()).
///             - Sweep law: cutoff = bias * 2^(sweep * range_octaves), where sweep in [0, 1) is the
///               envelope through a tanh soft knee (hard playing compresses into the ceiling
///               instead of slamming a rail). Exponential-in-Hz is a modeling *choice*: the LM13700
///               frequency is linear in control current, so the hardware law hinges on its BJT
///               driver stage — the law lives in one function (map_cutoff()) so the hardware
///               calibration pass can swap it without touching anything else. `range` is signed:
///               negative sweeps downward from bias (a deliberate extension; the pedal is up-only).
///             - Sensitivity at its floor (-60 dB) is treated as exactly off: the filter parks at
///               `bias` and the object becomes the pedal's secondary "cocked wah" — a fixed,
///               manually swept resonant filter (bias *is* the sweep knob).
///             - `drive` (dB, 0 = clean/linear) engages the svf driven circuit — tanh band-node
///               limiting, 2x oversampled — as the optional OTA-flavored color stage.
///             - Wet-only like the pedal by default; `mix` is an equal-power dry/wet extension
///               (100 % = hardware behavior). `gain` is a master output level in dB.
///             - House kit: every audible parameter rides a per-sample linear ramp (no zipper), a
///               16-slot preset-morph engine (store/recall with timed interpolation; slots 0–3 ship
///               factory voicings: guitar, bass, slow swell, cocked wah), allocation-free after
///               prepare(), deterministic, setters safe while audio runs. Single-channel by design
///               (per-channel envelopes are correct under mc. wrapping); process(x, key) exposes a
///               sidechain input for the wrapper's key inlet.
///
///             Reference provenance: freestompboxes.org trace (t=29137), La Révolution Deux block
///             analysis, PedalPCB "Poison Apple" build doc, official Mad Professor manuals. See
///             the TapTools-Max plan doc (plans/tap.autowah~.md) for the validation method.
/// @author     Timothy Place
// SPDX-License-Identifier: MIT
// Copyright 2026 Timothy Place.

#pragma once

#include <algorithm>
#include <array>
#include <cmath>
#include <type_traits>

#include "numeric.h"
#include "svf.h"

namespace tap::tools {
    namespace autowah {

        constexpr double k_pi                = 3.14159265358979323846;
        constexpr int    k_presets           = 16; // house slot count (grm_comb / grm_pitchaccum)
        constexpr double k_default_smooth_ms = 20.0;

        constexpr double k_freq_floor_hz = 20.0;
        constexpr double k_freq_ceil_hz  = 20000.0;
        constexpr double k_sens_floor_db = -60.0; // exactly off: fixed-filter ("cocked wah") mode
        constexpr double k_sens_ceil_db  = 24.0;
        constexpr double k_attack_min_ms = 0.05;
        constexpr double k_attack_max_ms = 100.0;
        constexpr double k_decay_min_ms  = 10.0;
        constexpr double k_decay_max_ms  = 5000.0;
        constexpr double k_range_max_oct = 5.0; // signed; hardware span is ~ +3.3
        constexpr double k_drive_max_db  = 24.0;
        constexpr double k_env_knee      = 1.5; // tanh soft-knee gain, envelope -> sweep
        constexpr int    k_oversample    = 2;   // fixed for the driven circuit (svf house default)

        enum param_index : int {
            p_sensitivity = 0, // envelope-detector input gain, dB (k_sens_floor_db = off/manual)
            p_attack,          // follower attack, ms (fast + fixed on the pedal; default 2)
            p_decay,           // follower release, ms — the pedal's Decay knob
            p_bias,            // resting/center frequency, Hz — the pedal's Bias knob
            p_range,           // sweep span in octaves, signed (negative sweeps down)
            p_resonance,       // 0..1 normalized Q — the pedal's Resonance knob
            p_drive,           // dB into the svf driven circuit; 0 = clean linear
            p_gain,            // master output gain, dB
            p_mix,             // 0..100 dry->wet, equal-power (100 = wet-only, the hardware)
            k_num_params
        };

        enum filter_mode : int {
            mode_lowpass = 0, // stock voicing (inferred tap — see header note)
            mode_bandpass,    // the circuit's other tap (the known hardware mod)
            k_num_modes
        };

        enum rectifier_mode : int {
            rect_fullwave = 0, // |x| — cleaner tracking (default)
            rect_halfwave,     // max(x, 0) — the traced single-diode topology
            k_num_rectifiers
        };

        /// One full parameter snapshot — a preset slot, and the unit the morph engine interpolates.
        template <typename Sample>
        struct basic_params {
            std::array<Sample, k_num_params> v{};

            static basic_params defaults() {
                basic_params p;
                p.v[p_sensitivity] = Sample(0.0);
                p.v[p_attack]      = Sample(2.0);
                p.v[p_decay]       = Sample(250.0);
                p.v[p_bias]        = Sample(250.0); // hardware resting point
                p.v[p_range]       = Sample(3.3);   // hardware span, 250 -> ~2500 Hz
                p.v[p_resonance]   = Sample(0.55);
                p.v[p_drive]       = Sample(0.0);
                p.v[p_gain]        = Sample(0.0);
                p.v[p_mix]         = Sample(100.0);
                return p;
            }
        };

        /// Clamp a value to the legal range of a parameter. Gain (dB) is unclamped, house-style.
        /// `Sample` is non-deduced and defaults to double so existing call sites — including
        /// the Max wrapper's, which passes a Min `atom` — keep compiling unchanged.
        template <typename Sample = double>
        Sample clamp_param(int index, std::type_identity_t<Sample> value) {
            switch (index) {
            case p_sensitivity:
                return std::clamp(value, k_sens_floor_db, k_sens_ceil_db);
            case p_attack:
                return std::clamp(value, k_attack_min_ms, k_attack_max_ms);
            case p_decay:
                return std::clamp(value, k_decay_min_ms, k_decay_max_ms);
            case p_bias:
                return std::clamp(value, k_freq_floor_hz, k_freq_ceil_hz);
            case p_range:
                return std::clamp(value, -k_range_max_oct, k_range_max_oct);
            case p_resonance:
                return std::clamp(value, Sample(0.0), Sample(1.0));
            case p_drive:
                return std::clamp(value, Sample(0.0), k_drive_max_db);
            case p_mix:
                return std::clamp(value, Sample(0.0), Sample(100.0));
            default:
                return value;
            }
        }

        /// The envelope filter: detector + sweep law + composed Simper SVF + the preset-morph engine.
        template <typename Sample>
        class basic_wah_filter {
            static_assert(is_sample_profile<Sample>,
                          "basic_wah_filter supports the two Tap numeric profiles: float and double");

          public:
            using sample_type = Sample;

            basic_wah_filter() {
                const basic_params<Sample> d = basic_params<Sample>::defaults();
                for (int i = 0; i < k_num_params; ++i) {
                    m_ramp[i].current = m_ramp[i].target = d.v[i];
                }
                install_factory_presets();
            }

            // -- lifecycle -------------------------------------------------------------------------------

            /// Set the sample rate, configure the composed filter, clear state, snap ramps.
            /// Allocates (inside the svf) — call from the main thread, not the perform loop.
            void prepare(Sample sr) {
                m_sr = (sr > Sample(0.0)) ? sr : Sample(48000.0);
                m_svf.prepare(m_sr, 1);
                m_svf.set_smooth_ms(Sample(0.0)); // this kernel owns all smoothing; svf setters snap
                m_svf.set_order(2);
                m_svf.set_oversample(k_oversample);
                apply_mode();
                m_svf_resonance = m_svf_drive = -Sample(1.0); // force a forward on the first sample
                m_svf_circuit                 = -1;
                snap();
                m_env = m_sweep = Sample(0.0);
            }

            /// Zero the filter state and the envelope; parameters untouched.
            void clear() {
                m_svf.clear();
                m_env = m_sweep = Sample(0.0);
            }

            /// Jump all parameter ramps to their targets.
            void snap() {
                for (auto& r : m_ramp) {
                    r.current   = r.target;
                    r.inc       = Sample(0.0);
                    r.remaining = 0;
                }
                m_ramps_active  = 0;
                m_derived_dirty = true;
            }

            Sample samplerate() const { return m_sr; }

            // -- structural modes (not ramped, not morphed) ------------------------------------------------

            void set_mode(int mode) {
                m_mode = std::clamp(mode, 0, k_num_modes - 1);
                apply_mode();
            }
            int mode() const { return m_mode; }

            void set_rectifier(int rect) { m_rectifier = std::clamp(rect, 0, k_num_rectifiers - 1); }
            int  rectifier() const { return m_rectifier; }

            /// Anti-zipper ramp time for direct setters, in ms. 0 = instant (useful for tests).
            void   set_smooth_ms(Sample ms) { m_smooth_ms = std::max(Sample(0.0), ms); }
            Sample smooth_ms() const { return m_smooth_ms; }

            // -- parameter targets (click-free; safe while audio runs) ------------------------------------

            void set_param(int index, Sample value) {
                if (index < 0 || index >= k_num_params) {
                    return;
                }
                ramp_to(index, clamp_param(index, value), smooth_samples());
            }

            void set_sensitivity(Sample db) { set_param(p_sensitivity, db); }
            void set_attack(Sample ms) { set_param(p_attack, ms); }
            void set_decay(Sample ms) { set_param(p_decay, ms); }
            void set_bias(Sample hz) { set_param(p_bias, hz); }
            void set_range(Sample octaves) { set_param(p_range, octaves); }
            void set_resonance(Sample r) { set_param(p_resonance, r); }
            void set_drive(Sample db) { set_param(p_drive, db); }
            void set_gain(Sample db) { set_param(p_gain, db); }
            void set_mix(Sample pct) { set_param(p_mix, pct); }

            Sample param(int index) const {
                return (index >= 0 && index < k_num_params) ? m_ramp[index].target : Sample(0.0);
            }

            // -- presets / morph -----------------------------------------------------------------------

            /// Capture the current *targets* (knob positions, not mid-ramp instantaneous values).
            bool store_preset(int slot) {
                if (!valid_slot(slot)) {
                    return false;
                }
                m_presets[slot] = snap_targets();
                return true;
            }

            /// Morph every parameter from wherever it currently is to the preset, over `seconds`.
            /// Re-targeting mid-morph stays continuous; seconds <= 0 jumps.
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

            /// Programmatic preset load (state restore; overwrite a factory slot).
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

            // -- introspection ---------------------------------------------------------------------------

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

            /// The current sweep position, 0..1 (post-knee) — the wrapper's envelope outlet.
            Sample envelope() const { return m_sweep; }

            /// The cutoff computed on the last sample, Hz — for tests and analysis.
            Sample cutoff_hz() const { return m_cutoff; }

            // -- audio -----------------------------------------------------------------------------------

            /// Process one sample; the envelope tracks the input itself (the pedal).
            Sample process(Sample x) { return process(x, x); }

            /// Process one sample with a sidechain: the filter runs on `x`, the envelope tracks `key`.
            Sample process(Sample x, Sample key) {
                if (m_ramps_active > 0) {
                    for (auto& r : m_ramp) {
                        if (r.remaining > 0) {
                            r.current += r.inc;
                            if (--r.remaining == 0) {
                                r.current = r.target;
                                --m_ramps_active;
                            }
                        }
                    }
                    m_derived_dirty = true;
                }
                if (m_derived_dirty) {
                    update_derived();
                    m_derived_dirty = (m_ramps_active > 0); // keep recomputing per sample while moving
                }

                // detector: gain -> rectify -> one-pole attack/release
                const Sample driven = key * m_sens_gain;
                const Sample rect   = (m_rectifier == rect_halfwave) ? std::max(driven, Sample(0.0)) : std::abs(driven);
                const Sample coef   = (rect > m_env) ? m_attack_coef : m_decay_coef;
                m_env               = anti_denormal(m_env + coef * (rect - m_env));

                // sweep law — isolated so the hardware calibration pass can swap it (see header)
                m_sweep  = std::tanh(Sample(k_env_knee) * m_env);
                m_cutoff = map_cutoff(m_sweep);

                m_svf.tick(m_cutoff);
                const Sample wet = m_svf.process(0, x);

                return x * m_dry_gain + wet * m_wet_gain;
            }

            void process(const Sample* in, Sample* out, size_t n) {
                for (size_t i = 0; i < n; ++i) {
                    out[i] = process(in[i]);
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

            long smooth_samples() const { return static_cast<long>(m_smooth_ms * Sample(0.001) * m_sr); }

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
                m_derived_dirty = true;
            }

            void apply_mode() { m_svf.set_mode((m_mode == mode_bandpass) ? svf::mode_bandpass : svf::mode_lowpass); }

            /// The sweep law: resting point at sweep 0, `range` octaves away at sweep 1, exponential
            /// in between. THE swappable function — if the hardware calibration pass finds the OTA
            /// driver is a linear V->I stage, this becomes bias + sweep * span_hz, and nothing else
            /// in the kernel changes.
            Sample map_cutoff(Sample sweep) const {
                const Sample hz = m_bias * std::exp2(sweep * m_range);
                return std::clamp(hz, Sample(k_freq_floor_hz), m_cutoff_ceil);
            }

            // Recompute all derived per-sample DSP values from the (smoothed) parameter values.
            // Runs every sample while any ramp is active, once when everything has settled.
            void update_derived() {
                const auto& val = m_ramp;

                // sensitivity: dB -> linear, with the floor treated as exactly off (manual mode)
                const Sample sens_db = val[p_sensitivity].current;
                m_sens_gain          = (sens_db <= Sample(k_sens_floor_db) + Sample(1e-9))
                                           ? Sample(0.0)
                                           : std::pow(Sample(10.0), sens_db * Sample(0.05));

                // follower coefficients: 63% time constants from the ms parameters
                m_attack_coef =
                    Sample(1.0) - std::exp(-Sample(1000.0) / (std::max(val[p_attack].current, Sample(0.01)) * m_sr));
                m_decay_coef =
                    Sample(1.0) - std::exp(-Sample(1000.0) / (std::max(val[p_decay].current, Sample(0.01)) * m_sr));

                m_bias        = val[p_bias].current;
                m_range       = val[p_range].current;
                m_cutoff_ceil = std::min(Sample(k_freq_ceil_hz), Sample(0.45) * m_sr);

                // forward into the composed svf only on change (its setters snap; see prepare())
                if (val[p_resonance].current != m_svf_resonance) {
                    m_svf_resonance = val[p_resonance].current;
                    m_svf.set_resonance(m_svf_resonance);
                }
                if (val[p_drive].current != m_svf_drive) {
                    m_svf_drive = val[p_drive].current;
                    m_svf.set_drive(m_svf_drive);
                }
                // drive 0 runs the pure linear circuit; any drive engages the saturating one. The
                // switch happens where tanh is near-identity for typical levels, so it is benign.
                const int circuit = (m_svf_drive > Sample(1e-6)) ? svf::circuit_driven : svf::circuit_clean;
                if (circuit != m_svf_circuit) {
                    m_svf_circuit = circuit;
                    m_svf.set_circuit(circuit);
                }

                const Sample g     = std::pow(Sample(10.0), val[p_gain].current * Sample(0.05));
                const Sample theta = std::clamp(val[p_mix].current, Sample(0.0), Sample(100.0)) * Sample(0.01)
                                     * (Sample(k_pi_for<Sample>) * Sample(0.5));
                m_dry_gain = std::cos(theta) * g; // equal-power, like tap.crossfade~
                m_wet_gain = std::sin(theta) * g;
            }

            // Factory voicings in slots 0-3 (author-approved 2026-07-15); the rest hold defaults.
            void install_factory_presets() {
                basic_params<Sample> guitar = basic_params<Sample>::defaults(); // slot 0: the hardware's home position

                basic_params<Sample> bass = basic_params<Sample>::defaults(); // slot 1: the GB switch, as a preset
                bass.v[p_bias]            = Sample(120.0);
                bass.v[p_range]           = Sample(3.0);
                bass.v[p_decay]           = Sample(300.0);
                bass.v[p_resonance]       = Sample(0.5);

                basic_params<Sample> swell = basic_params<Sample>::defaults(); // slot 2: slow filter swells
                swell.v[p_bias]            = Sample(200.0);
                swell.v[p_decay]           = Sample(1500.0);
                swell.v[p_attack]          = Sample(5.0);
                swell.v[p_resonance]       = Sample(0.65);

                basic_params<Sample> cocked =
                    basic_params<Sample>::defaults(); // slot 3: sensitivity off = fixed filter
                cocked.v[p_sensitivity] = Sample(k_sens_floor_db);
                cocked.v[p_bias]        = Sample(800.0);
                cocked.v[p_resonance]   = Sample(0.7);

                m_presets.fill(basic_params<Sample>::defaults());
                m_presets[0] = guitar;
                m_presets[1] = bass;
                m_presets[2] = swell;
                m_presets[3] = cocked;
            }

            Sample m_sr{Sample(48000.0)};
            Sample m_smooth_ms{Sample(k_default_smooth_ms)};
            int    m_mode{mode_lowpass};
            int    m_rectifier{rect_fullwave};

            std::array<ramp, k_num_params>              m_ramp;
            std::array<basic_params<Sample>, k_presets> m_presets;
            int                                         m_ramps_active{0};
            bool                                        m_derived_dirty{true};

            svf::basic_svf_filter<Sample> m_svf;

            // detector / sweep state
            Sample m_env{Sample(0.0)};
            Sample m_sweep{Sample(0.0)};
            Sample m_cutoff{Sample(250.0)};

            // derived (cached while parameters are settled)
            Sample m_sens_gain{Sample(1.0)};
            Sample m_attack_coef{Sample(0.0)};
            Sample m_decay_coef{Sample(0.0)};
            Sample m_bias{Sample(250.0)};
            Sample m_range{Sample(3.3)};
            Sample m_cutoff_ceil{Sample(k_freq_ceil_hz)};
            Sample m_dry_gain{Sample(0.0)};
            Sample m_wet_gain{Sample(1.0)};
            Sample m_svf_resonance{-Sample(1.0)};
            Sample m_svf_drive{-Sample(1.0)};
            int    m_svf_circuit{-1};
        };

        using params   = basic_params<double>;
        using params32 = basic_params<float>;

        /// The double profile — the golden model.
        using wah_filter = basic_wah_filter<double>;

        /// The float profile — for single-precision targets. See numeric.h.
        using wah_filter32 = basic_wah_filter<float>;

    } // namespace autowah
} // namespace tap::tools
