/// @file
/// @brief      Portable generative event-loop kernel for tap.garden~ — no Max/Min dependency.
/// @details    A recreation of the *principle* behind Brian Eno and Peter Chilvers' generative
///             music apps (Bloom, 2008), as described in their published interviews and in Eno's
///             "Generative Music" talk (In Motion Magazine, 1996): a touch becomes a note; the
///             note repeats on a fixed loop, a little quieter and a little purer each pass, until
///             it fades below hearing; pitches snap to a scale so anything you plant sounds
///             consonant; and left alone past an idle threshold, the system starts planting notes
///             itself. The principle only — no scale tables, timings, or sounds are copied from
///             the app, whose name (Bloom) is a live trademark of Opal Limited. The kernel is
///             named for Eno's own metaphor: the composer as gardener, not architect.
///
///             This is the family's third abstraction level: discreet.h recirculates audio,
///             airport.h phases loops, garden.h recirculates *events*. The stability inversion
///             carries over intact, one level up: per-pass decay is the stabilizer. An event's
///             velocity is multiplied by `decay` on every recirculation and the event retires
///             below `floor`, so the live-event population converges no matter how fast you
///             plant — and a fixed bell pool (quietest-stolen) hard-bounds the audio regardless.
///
///             The voice is a small wind chime: four decaying mode doublets at the transverse-
///             vibration ratios of the chosen `material` — the free-free tube's
///             1 : 2.756 : 5.404 : 8.933 or the tuned bar's double-octave 1 : 4 : 10 : 20
///             (Fletcher & Rossing, The Physics of Musical Instruments, 2nd ed. — the
///             bars/tubular-chimes and mallet-percussion chapters; f_n grows as (2n+1)^2 for
///             the free bar, and marimba bars are undercut to the octave tuning; doublet
///             splitting of degenerate tube mode pairs is from the same source, a fixed few
///             cents here so tails beat slowly instead of decaying like lab sines). The first
///             mode carries the perceived pitch;
///             the upper modes are inharmonic, softer (scaled by per-event brightness times
///             strike hardness — a soft strike is a dull strike), progressively steeper in
///             brightness (b, b^2, b^3), and faster-dying (~f^2 radiation damping), the 4th
///             gone in tens of milliseconds: the contact tick. Ring time scales with
///             sqrt(440/f) per strike — small high tubes ring shorter. Every strike therefore
///             rings down to its fundamental, which is where the pitch contract lives: a
///             detector reads the chime in its tail, once the clang has cleared (the tests
///             measure there). Each pass multiplies the event's brightness by `soften`, so a
///             bloom does not just fade: it purifies toward its fundamental, losing its tick
///             first. Mode amplitudes ride the shared tr808 decay_env (one per mode); a strike
///             on a silent tube starts its doublets aligned (fresh initial conditions), while
///             an audible steal keeps free-running phases and glides instead of clicking.
///
///             The tube is the identity. Each struck pitch is a physical tube whose
///             imperfections are fixed properties of the tube, not of the strike: its upper
///             modes sit up to k_scatter_cents off the ideal ratios (the fundamental stays
///             true — a maker tunes the fundamental and the overtones land where the metal
///             puts them), and it hangs at a fixed seat on the stereo rack, spread scaled by
///             `spread`. Both draws come from a stateless hash of the pitch (the metal_bank.h
///             per-index xorshift idiom), so the rack is the same rack in every instance,
///             every return of a bloom rings from the same place with the same flaws, and the
///             gardener's seeded rng is never consumed — the seed-triad contract survives.
///
///             Randomness: the idle gardener draws from the family's seeded xorshift64*
///             (tr808::white_noise) — deterministic per seed, so renders and tests reproduce and
///             instances decorrelate by seed. The gardener is wind: strikes arrive on a
///             calm/gust cycle (`gust` sizes the clusters — up to five neighboring tubes within
///             a fraction of a second — with calms stretched to hold the average near one
///             strike per pass). This is the library's first randomized *event* source
///             (step_seq.h promises "no randomness anywhere"; this kernel is the deliberate
///             counterpoint, and the seed contract is the bridge back to reproducibility).
///
///             Geometry: everything is fixed arrays — k_max_events events, k_voices bells —
///             so prepare(sr) allocates nothing at all and no later call ever does.
///
///             Five classes, because a garden is a system and the parts are worth having alone:
///             - `bell` — one wind chime, four decaying mode doublets (see below).
///             - `rack` — the fixed pool of k_voices bells plus the allocator: an idle bell if
///               there is one, else the quietest is stolen and RE-AIMED (free-running phases, a
///               gliding seat) rather than reset. That allocator lives here, in portable C++,
///               rather than being delegated to a host's polyphony container — Max's poly~ steals
///               round-robin and is Max-only, so a kernel that leaned on it would lose both the
///               glide-not-click promise and every non-Max target. This is what tap.chime~ wraps.
///             - `ring` — the event recirculation: plant, fire on the loop grid, multiply velocity
///               by `decay` and brightness by `soften` each pass, retire below `floor`. It carries
///               the convergence theorem and knows nothing about chimes, which is the point: point
///               it at any voice you like (tap.bloom).
///             - `gardener` — the seeded wind: gusts and calms, emitting plant requests once the
///               garden has been idle. Consumes its rng ONLY while idling (tap.gardener).
///             - `scale_quantizer` — the root/scale snap applied at entry (tap.scale).
///             `bed` is those four wired together and a master level, and nothing else; the wiring
///             is what the null test pins.
///
///             Honest limits:
///             - Pitch is quantized AT ENTRY: changing root or scale re-pitches nothing already
///               planted, only future plants (replants pick up the new field).
///             - Event timing is the loop grid: a plant returns at its own phase point every
///               pass, exactly — there is no swing, no drift, no humanization.
///             - A full garden (k_max_events live) retires its OLDEST bloom to make room for a
///               new plant: a touch must always speak, and the oldest is the quietest.
///             - The idle gardener is a statistical wind (gusts and calms averaging about one
///               strike per pass), not a transcription of any published piece or app behavior.
///             - Stereo out, but the image is a fixed rack of seats keyed by pitch — there is
///               no per-strike pan, no motion. Two materials, one chime timbre family. It is
///               an instrument, not a polysynth.
/// @author     Timothy Place
// SPDX-License-Identifier: MIT
// Copyright 2026 Timothy Place.

#pragma once

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <initializer_list>

#include "swing_vca.h" // tap::tools::tr808 — decay_env (the bell's amplitude) + white_noise (the seeded gardener)

namespace tap::tools {
    namespace garden {

        constexpr double k_pi = 3.14159265358979323846;

        constexpr int k_max_events = 64; // live blooms; oldest yields when full
        constexpr int k_voices     = 16; // fixed bell pool; quietest-first steal
        constexpr int k_modes      = 4;  // a small chime: four transverse modes, the 4th the strike's tick

        /// What the tubes are made of — a mode, not a fader (instant; re-voices every bloom at
        /// its next return).
        enum material_index : int {
            material_chime = 0, // free-free tube: the wind-chime rack
            material_bar,       // tuned bar: the mallet-instrument plank
            k_num_materials
        };

        // Transverse-mode ratios per material (Fletcher & Rossing, The Physics of Musical
        // Instruments, 2nd ed.): the free-free bar's 1 : 2.756 : 5.404 : 8.933 (f_n ~ (2n+1)^2,
        // bars/tubular-chimes chapter), and the tuned bar's 1 : 4 : 10 double-octave tuning
        // (mallet-percussion chapter), its 4th continuing the double-octave series.
        constexpr double k_mode_ratio[k_num_materials][k_modes] = {
            {1.0, 2.756, 5.404, 8.933},
            {1.0, 4.0, 10.0, 20.0},
        };
        // Decay-time divisor per mode, ~ratio^2: radiation damping grows roughly with f^2, so
        // higher modes die much faster — the 4th is gone in tens of ms, the contact tick.
        constexpr double k_mode_haste[k_num_materials][k_modes] = {
            {1.0, 7.6, 29.2, 79.8},
            {1.0, 16.0, 100.0, 400.0},
        };
        // Mode levels at full hardness sum to <= 1 so a chime is bounded by its velocity; the
        // upper modes scale with brightness (progressively steeper — see bell::trigger).
        constexpr double k_mode_level[k_modes] = {0.58, 0.25, 0.10, 0.07};
        // Each mode is a doublet: a real tube's degenerate mode pairs are split a few cents by
        // imperfection (Fletcher & Rossing on doublets in bells/chimes), so tails beat slowly
        // instead of decaying like lab sines. Fixed split — deterministic, no RNG.
        constexpr double k_doublet_cents = 1.5;
        // Each TUBE is imperfect in its own fixed way: per-pitch mode detune of up to this many
        // cents, drawn by a stateless hash of (pitch, mode) — the metal_bank.h per-index idiom,
        // so the rack is the rack in every instance and the gardener's seed is never involved.
        constexpr double k_scatter_cents = 3.0;
        // A soft strike is a dull strike: effective brightness scales with velocity through
        // this floor (hardness = floor + (1 - floor) * velocity).
        constexpr double k_hardness_floor = 0.5;
        // Small high tubes ring shorter than long low ones: per-strike decay scales by
        // sqrt(440 / f), clamped to this range.
        constexpr double k_ring_scale_min = 0.5;
        constexpr double k_ring_scale_max = 2.0;
        // A steal glides its seat the way it glides its phases: pan gains slew over this window
        // (a strike on a silent tube snaps them — fresh initial conditions, same rule).
        constexpr double k_pan_slew_ms  = 10.0;
        constexpr double k_gain_epsilon = 1e-4; // below this a voice is "off" (harmonizer.h idiom)

        constexpr double k_min_loop_seconds = 0.25;  // beneath this it is a buzzer, not a garden
        constexpr double k_max_loop_seconds = 120.0; // the loop is a counter — no tape is bought

        constexpr double k_default_loop_seconds = 8.0;
        constexpr double k_default_decay        = 0.85;  // velocity multiplier per pass
        constexpr double k_default_soften       = 0.9;   // brightness multiplier per pass
        constexpr double k_default_floor        = 0.03;  // retirement threshold
        constexpr double k_default_idle_seconds = 30.0;  // the gardener's patience; 0 disables
        constexpr double k_default_gust         = 0.5;   // the wind: 0 calm/even, 1 blustery clusters
        constexpr double k_default_spread       = 0.7;   // the rack's stereo width; 0 collapses to mono
        constexpr double k_default_attack_s     = 0.004; // a clapper's strike, not a bow
        constexpr double k_default_decay_s      = 4.0;
        constexpr double k_default_brightness   = 1.0;
        constexpr double k_default_smooth_ms    = 20.0; // one-pole slew for the master level

        /// Build a 12-bit pitch-class mask from scale degrees — same idiom as tune.h (copied, not
        /// included: tune.h reaches into tap::dsp).
        constexpr unsigned make_mask(std::initializer_list<int> degrees) {
            unsigned mask = 0u;
            for (const int d : degrees) {
                mask |= 1u << (((d % 12) + 12) % 12);
            }
            return mask;
        }

        enum scale_index : int {
            scale_chromatic = 0,
            scale_major,
            scale_minor,
            scale_major_pentatonic,
            scale_minor_pentatonic,
            k_num_scales
        };

        // Scale presets relative to the root, addressed by scale_index — public-domain scale
        // theory, deliberately NOT any app's preset list.
        constexpr std::array<unsigned, k_num_scales> k_scale_masks = {
            0xFFFu,                            // chromatic
            make_mask({0, 2, 4, 5, 7, 9, 11}), // major
            make_mask({0, 2, 3, 5, 7, 8, 10}), // minor
            make_mask({0, 2, 4, 7, 9}),        // major pentatonic
            make_mask({0, 3, 5, 7, 10}),       // minor pentatonic
        };

        /// Stateless draw in [-1, 1) keyed by (tube, index) — the metal_bank.h per-index
        /// xorshift64* idiom: identity-keyed imperfection with no generator state. The tube key
        /// is its fundamental in centihertz; index 0 is the tube's seat on the rack, 1..3 the
        /// scatter of its upper modes. Same tube, same flaws, in every instance, forever —
        /// and the gardener's seeded rng is never consumed.
        inline double tube_unit(uint64_t tube, uint64_t index) {
            uint64_t s = tube * 0x9e3779b97f4a7c15ULL + (index + 1) * 0xbf58476d1ce4e5b9ULL;
            s ^= s >> 12;
            s ^= s << 25;
            s ^= s >> 27;
            const double u = static_cast<double>((s * 0x2545f4914f6cdd1dULL) >> 11) / 9007199254740992.0; // [0, 1)
            return 2.0 * u - 1.0;
        }

        /// A tube's identity key: its fundamental, in centihertz (fractional pitches stay
        /// distinct; equal pitches strike the same tube).
        inline uint64_t tube_key(double freq_hz) {
            return static_cast<uint64_t>(std::llround(freq_hz * 100.0));
        }

        /// One small wind chime: four decaying mode doublets at the struck material's transverse
        /// ratios (k_mode_ratio) — each mode a pair of sines split k_doublet_cents so the tail
        /// beats slowly, the upper modes scattered a fixed few cents by the tube's own hash,
        /// scaled by brightness (progressively steeper per mode) and hardness (a soft strike is
        /// duller), all dying faster than the fundamental (k_mode_haste, ~f^2 radiation damping
        /// — the 4th mode is the strike's tick). Ring time scales with sqrt(440/f) at trigger:
        /// small high tubes ring shorter. Output is panned to the tube's fixed seat on the
        /// rack. Phases free-run and the seat glides, so a steal re-aims without a click.
        class bell {
          public:
            void prepare(double sr) {
                m_sr        = (sr > 0.0) ? sr : 48000.0;
                m_pan_coeff = 1.0 - std::exp(-1.0 / (k_pan_slew_ms * 0.001 * m_sr));
                for (auto& e : m_env) {
                    e.prepare(m_sr);
                }
                set_times(k_default_attack_s, k_default_decay_s);
            }

            /// Stored and applied per strike (ring time depends on the struck pitch), so a
            /// ringing chime keeps its envelope until retriggered.
            void set_times(double attack_s, double decay_s) {
                m_attack_s = attack_s;
                m_decay_s  = decay_s;
            }

            void reset() {
                for (auto& e : m_env) {
                    e.reset();
                }
                for (auto& p : m_phase_a) {
                    p = 0.0;
                }
                for (auto& p : m_phase_b) {
                    p = 0.0;
                }
                m_gain_l = m_gain_r = 0.0; // the first strike snaps the seat (silent-tube rule)
                m_gain_l_target = m_gain_r_target = 0.0;
            }

            /// Strike at `freq_hz` (the first mode — the perceived pitch), envelope target
            /// `level`, upper-mode weight `brightness` (0..1), a `material` (mode-ratio table),
            /// and a seat `pan` (-1 left .. +1 right, equal-power). Effective brightness couples
            /// to the strike level (soft strikes are duller) and steepens per mode (b, b^2,
            /// b^3), so softening kills the highest partials first. The upper modes are
            /// detuned by the tube's fixed scatter (the fundamental stays true — a maker tunes
            /// the fundamental). Modes above the audio band stay silent rather than aliasing.
            void trigger(double freq_hz, double level, double brightness, int material, double pan) {
                const bool silent = this->level() <= k_gain_epsilon;
                if (silent) {                   // a strike on a silent tube sets fresh
                    for (auto& p : m_phase_a) { // initial conditions: the doublet starts
                        p = 0.0;                // aligned and its beat blooms from the
                    } // strike. Audible steals keep free-running
                    for (auto& p : m_phase_b) { // phases and glide instead.
                        p = 0.0;
                    }
                }
                // Equal-power seat with exact endpoints; sqrt form so pan 0 is bitwise l == r.
                const double p  = std::clamp(pan, -1.0, 1.0);
                m_gain_l_target = std::sqrt(0.5 * (1.0 - p));
                m_gain_r_target = std::sqrt(0.5 * (1.0 + p));
                if (silent) { // the seat snaps with the phases; an audible steal glides there
                    m_gain_l = m_gain_l_target;
                    m_gain_r = m_gain_r_target;
                }
                const size_t   mat    = static_cast<size_t>(std::clamp(material, 0, k_num_materials - 1));
                const uint64_t tube   = tube_key(freq_hz);
                m_freq_hz             = freq_hz; // which tube this bell is holding, for per-voice callers
                const double hardness = k_hardness_floor + (1.0 - k_hardness_floor) * std::clamp(level, 0.0, 1.0);
                const double b        = std::clamp(brightness, 0.0, 1.0) * hardness;
                const double ring     = std::clamp(std::sqrt(440.0 / freq_hz), k_ring_scale_min, k_ring_scale_max);
                const double split    = std::exp2(k_doublet_cents / 2400.0); // half the split, up and down
                double       shine    = 1.0;                                 // b^0, b^1, b^2, b^3 per mode
                for (int m = 0; m < k_modes; ++m) {
                    const size_t i = static_cast<size_t>(m);
                    const double scatter =
                        (m > 0) ? std::exp2(k_scatter_cents * tube_unit(tube, static_cast<uint64_t>(m)) / 1200.0) : 1.0;
                    const double mode_hz = k_mode_ratio[mat][i] * freq_hz * scatter;
                    m_inc_a[i]           = mode_hz * split / m_sr;
                    m_inc_b[i]           = mode_hz / split / m_sr;
                    m_env[i].set_times(m_attack_s, m_decay_s * ring / k_mode_haste[mat][i]);
                    m_env[i].trigger((mode_hz < 0.45 * m_sr) ? level * k_mode_level[i] * shine : 0.0);
                    shine *= b;
                }
            }

            double level() const { // the quietest-first steal key
                double sum = 0.0;
                for (const auto& e : m_env) {
                    sum += e.value();
                }
                return sum;
            }

            /// Advance this chime one sample and return its RAW mono sum, before the seat is
            /// applied — the tube as it would sound with your ear against it. The seat still
            /// glides on this call, so the two process paths stay in step and a caller that
            /// wants the dry voice does not have to give up the panning state.
            double process_mono() {
                double sum = 0.0;
                for (size_t i = 0; i < static_cast<size_t>(k_modes); ++i) {
                    m_phase_a[i] += m_inc_a[i];
                    m_phase_a[i] -= std::floor(m_phase_a[i]);
                    m_phase_b[i] += m_inc_b[i];
                    m_phase_b[i] -= std::floor(m_phase_b[i]);
                    sum += m_env[i].process() * 0.5
                           * (std::sin(2.0 * k_pi * m_phase_a[i]) + std::sin(2.0 * k_pi * m_phase_b[i]));
                }
                m_gain_l += m_pan_coeff * (m_gain_l_target - m_gain_l);
                m_gain_r += m_pan_coeff * (m_gain_r_target - m_gain_r);
                return sum;
            }

            /// Sum this chime, panned to its seat, into the running busses.
            void process(double& out_left, double& out_right) {
                const double sum = process_mono();
                out_left += sum * m_gain_l;
                out_right += sum * m_gain_r;
            }

            /// The seat gains this chime is currently sounding at — what process() multiplies
            /// the mono sum by, so a caller holding the dry voice can rebuild the rack image.
            double gain_left() const { return m_gain_l; }
            double gain_right() const { return m_gain_r; }

            /// The fundamental this chime was last struck at, in Hz (0 before any strike). The
            /// pool reassigns bells as it steals, so this is how a caller knows which tube a
            /// given voice is currently holding.
            double frequency() const { return m_freq_hz; }

          private:
            double                                m_sr{48000.0};
            double                                m_attack_s{k_default_attack_s};
            double                                m_decay_s{k_default_decay_s};
            double                                m_pan_coeff{1.0};
            double                                m_freq_hz{0.0};
            double                                m_gain_l{0.0};
            double                                m_gain_r{0.0};
            double                                m_gain_l_target{0.0};
            double                                m_gain_r_target{0.0};
            std::array<double, k_modes>           m_phase_a{};
            std::array<double, k_modes>           m_phase_b{};
            std::array<double, k_modes>           m_inc_a{};
            std::array<double, k_modes>           m_inc_b{};
            std::array<tr808::decay_env, k_modes> m_env;
        };

        /// The root/scale snap applied at entry — the tune.h nearest-allowed search (any non-empty
        /// mask has a note within a tritone). Copied, not included: tune.h reaches into tap::dsp.
        /// A mode, not a fader: changing it re-pitches nothing already planted, only future plants.
        class scale_quantizer {
          public:
            /// Root pitch class, 0..11 (0 = C).
            void set_root(int semitone) { m_root = ((semitone % 12) + 12) % 12; }

            /// Scale preset (scale_index).
            void set_scale(int scale) { m_scale = std::clamp(scale, 0, k_num_scales - 1); }

            int root() const { return m_root; }
            int scale() const { return m_scale; }

            /// Snap MIDI semitones to the nearest pitch in the current root/scale.
            double quantize(double pitch) const {
                const unsigned mask = k_scale_masks[static_cast<size_t>(m_scale)];
                const int      p    = static_cast<int>(std::lround(pitch));
                for (int off = 0; off <= 6; ++off) {
                    for (const int cand : {p + off, p - off}) {
                        const int pc = (((cand - m_root) % 12) + 12) % 12;
                        if ((mask & (1u << pc)) != 0u) {
                            return static_cast<double>(cand);
                        }
                    }
                }
                return static_cast<double>(p); // unreachable for any non-empty mask
            }

          private:
            int m_root{0};
            int m_scale{scale_major_pentatonic}; // anything you plant sounds consonant
        };

        /// The chime rack: a fixed pool of k_voices bells and the allocator that hands them out.
        /// An idle bell if there is one, else the QUIETEST is stolen — and stolen by re-aiming,
        /// not resetting, so its phases keep free-running and its seat glides instead of clicking.
        ///
        /// The allocator lives here rather than in the host on purpose. Max's poly~ steals
        /// round-robin, and it does not exist off Max at all; a kernel that delegated voice
        /// stealing would lose the glide-not-click promise and every non-Max target with it. The
        /// pool is also the hard bound on the audio: however fast strikes arrive, k_voices chimes
        /// is all that can ever be ringing.
        class rack {
          public:
            void prepare(double sr) {
                m_sr = (sr > 0.0) ? sr : 48000.0;
                for (auto& v : m_bells) {
                    v.prepare(m_sr);
                    v.set_times(m_attack_s, m_decay_s);
                }
                clear();
            }

            /// Silence every bell.
            void clear() {
                for (auto& v : m_bells) {
                    v.reset();
                }
            }

            /// Envelope times in SECONDS (decay_env contract). Ringing voices keep their envelope
            /// until retriggered; the strike scales decay by sqrt(440/f) on top of this.
            void set_times(double attack_s, double decay_s) {
                m_attack_s = std::max(attack_s, 1e-6);
                m_decay_s  = std::max(decay_s, 1e-6);
                for (auto& v : m_bells) {
                    v.set_times(m_attack_s, m_decay_s);
                }
            }

            /// What the tubes are made of (material_index). A mode, not a fader: instant, and read
            /// at strike time, so every later strike re-voices.
            void set_material(int material) { m_material = std::clamp(material, 0, k_num_materials - 1); }

            /// The rack's stereo width, [0, 1]. Each tube hangs at a fixed seat drawn from its
            /// pitch (the same stateless hash as its scatter), scaled by spread; 0 collapses the
            /// rack to center mono, bitwise equal on both busses.
            void set_spread(double amount) { m_spread = std::clamp(amount, 0.0, 1.0); }

            /// Strike the tube at a MIDI pitch (fractional accepted — fractional pitches are
            /// distinct tubes with their own flaws and their own seat).
            void strike(double pitch, double velocity, double brightness) {
                strike_hz(440.0 * std::exp2((pitch - 69.0) / 12.0), velocity, brightness);
            }

            /// Strike the tube at a frequency: allocate an idle bell if there is one, else steal
            /// the quietest and re-aim it. The seat comes from the tube's own hash, so the rack is
            /// the same rack in every instance and a returning bloom rings from the same place.
            void strike_hz(double freq_hz, double velocity, double brightness) {
                bell* voice = &m_bells[0];
                for (auto& v : m_bells) {
                    if (v.level() <= k_gain_epsilon) {
                        voice = &v;
                        break;
                    }
                    if (v.level() < voice->level()) {
                        voice = &v;
                    }
                }
                const double pan = m_spread * tube_unit(tube_key(freq_hz), 0); // index 0: the seat
                voice->trigger(freq_hz, velocity, brightness, m_material, pan);
            }

            /// Sum every ringing chime onto the stereo busses. ACCUMULATES, like bell::process.
            void process(double& out_left, double& out_right) {
                for (auto& v : m_bells) {
                    v.process(out_left, out_right);
                }
            }

            /// Per-voice mono taps: one sample per bell, RAW — before each bell's seat is
            /// applied — written into `out`, which must hold at least `count` doubles (extra
            /// entries beyond k_voices are zeroed). This is the same advance as process(); a
            /// caller takes one or the other on a given sample, never both.
            ///
            /// Summing these back through voice_gain_left/right reproduces process() exactly,
            /// which is what the pinned scenario checks. The point of taking them apart is that
            /// you do not have to: place, filter, or gate each tube yourself.
            void process_voices(double* out, int count) {
                for (int i = 0; i < count; ++i) {
                    out[static_cast<size_t>(i)] = (i < k_voices) ? m_bells[static_cast<size_t>(i)].process_mono() : 0.0;
                }
            }

            int active_voices() const {
                int n = 0;
                for (const auto& v : m_bells) {
                    n += (v.level() > k_gain_epsilon) ? 1 : 0;
                }
                return n;
            }

            /// Which tube a given voice is currently holding, and how loudly — the pool
            /// reassigns bells as it steals, so voice i is whatever was last put there.
            double voice_hz(int i) const { return valid_voice(i) ? m_bells[static_cast<size_t>(i)].frequency() : 0.0; }
            double voice_level(int i) const { return valid_voice(i) ? m_bells[static_cast<size_t>(i)].level() : 0.0; }
            double voice_gain_left(int i) const {
                return valid_voice(i) ? m_bells[static_cast<size_t>(i)].gain_left() : 0.0;
            }
            double voice_gain_right(int i) const {
                return valid_voice(i) ? m_bells[static_cast<size_t>(i)].gain_right() : 0.0;
            }

            double attack_s() const { return m_attack_s; }
            double decay_s() const { return m_decay_s; }
            int    material() const { return m_material; }
            double spread() const { return m_spread; }
            double samplerate() const { return m_sr; }

          private:
            static bool valid_voice(int i) { return i >= 0 && i < k_voices; }

            double                     m_sr{48000.0};
            double                     m_attack_s{k_default_attack_s};
            double                     m_decay_s{k_default_decay_s};
            int                        m_material{material_chime};
            double                     m_spread{k_default_spread};
            std::array<bell, k_voices> m_bells;
        };

        /// One strike falling out of the ring: what to hit, how hard, how bright.
        struct strike {
            double pitch{0.0};      // MIDI semitones, already quantized by whoever planted it
            double velocity{0.0};   // this pass's velocity, before the pass's decay is applied
            double brightness{0.0}; // this pass's brightness, likewise
        };

        /// The event ring: plant a bloom and it returns at its own position every pass, a little
        /// quieter (`decay`) and a little purer (`soften`), until it falls below `floor` and
        /// retires. It knows nothing about chimes — it emits strikes, and what sounds them is the
        /// caller's business, which is the whole reason it is worth having on its own.
        ///
        /// The stabilizer and its theorem: with floor f and a plant at velocity v, a bloom lives
        /// exactly ceil(log(f/v)/log(decay)) passes, so the live population converges no matter
        /// how fast you plant. A full ring retires its OLDEST bloom to make room for a new plant —
        /// a touch must always speak, and the oldest is the quietest.
        class ring {
          public:
            void prepare(double sr) {
                m_sr = (sr > 0.0) ? sr : 48000.0;
                clear();
            }

            /// Uproot everything: kill every event and rewind the loop. Parameters are untouched.
            void clear() {
                for (auto& e : m_events) {
                    e.alive = false;
                }
                m_pos     = 0;
                m_planted = 0;
            }

            /// Loop length in seconds, clamped to [k_min_loop_seconds, k_max_loop_seconds].
            /// Instant (the loop is a counter): blooms keep their positions modulo the new length.
            void set_loop_seconds(double s) {
                m_loop_seconds = std::clamp(s, k_min_loop_seconds, k_max_loop_seconds);
                const long n   = loop_samples();
                m_pos          = m_pos % n;
                for (auto& e : m_events) {
                    e.offset = e.offset % n;
                }
            }

            /// Velocity multiplier per pass, [0, 1] — the stabilizer.
            void set_decay(double per_pass) { m_decay = std::clamp(per_pass, 0.0, 1.0); }

            /// Brightness multiplier per pass, [0, 1]: each return is purer, collapsing to sine.
            void set_soften(double per_pass) { m_soften = std::clamp(per_pass, 0.0, 1.0); }

            /// Retirement threshold, [1e-4, 1].
            void set_floor(double v) { m_floor = std::clamp(v, 1e-4, 1.0); }

            /// The brightness a new plant starts at, [0, 1]; it softens from there.
            void set_brightness(double b) { m_brightness = std::clamp(b, 0.0, 1.0); }

            /// Plant a bloom at the current loop position. Velocity is clamped to (0, 1]. It fires
            /// on the next due() — the coming sample — and then every pass until it retires.
            void plant(double pitch, double velocity) { plant_event(pitch, velocity); }

            /// The blooms due on this sample: each is reported and then worn by one pass. Writes
            /// at most `max` strikes into `out` and returns how many. Does NOT advance the loop —
            /// call step() once the gardener has had its turn, which is the order bed keeps.
            int due(strike* out, int max) {
                int n = 0;
                for (auto& e : m_events) {
                    if (e.alive && e.offset == m_pos) {
                        if (n < max) {
                            out[n].pitch      = e.pitch;
                            out[n].velocity   = e.velocity;
                            out[n].brightness = e.brightness;
                            ++n;
                        }
                        bloom(e);
                    }
                }
                return n;
            }

            /// Plant at the current position and take its first strike immediately — the
            /// gardener's door, which opens after due() has already run for this sample.
            strike plant_now(double pitch, double velocity) {
                event& e = plant_event(pitch, velocity);
                strike s;
                s.pitch      = e.pitch;
                s.velocity   = e.velocity;
                s.brightness = e.brightness;
                bloom(e);
                return s;
            }

            /// Advance the loop one sample.
            void step() {
                if (++m_pos >= loop_samples()) {
                    m_pos = 0;
                }
            }

            int active_events() const {
                int n = 0;
                for (const auto& e : m_events) {
                    n += e.alive ? 1 : 0;
                }
                return n;
            }

            long   loop_samples() const { return static_cast<long>(m_loop_seconds * m_sr); }
            long   position() const { return m_pos; }
            double loop_seconds() const { return m_loop_seconds; }
            double decay() const { return m_decay; }
            double soften() const { return m_soften; }
            double floor_level() const { return m_floor; }
            double brightness() const { return m_brightness; }
            double samplerate() const { return m_sr; }

          private:
            struct event {
                double   pitch{0.0};      // MIDI semitones, already quantized
                double   velocity{0.0};   // decays per pass
                double   brightness{0.0}; // softens per pass
                long     offset{0};       // position on the loop, samples
                uint32_t seq{0};          // plant order; lowest live seq = oldest
                bool     alive{false};
            };

            /// Find a slot for a new plant: a dead one if any, else the oldest live bloom yields.
            event& allocate() {
                event* oldest = &m_events[0];
                for (auto& e : m_events) {
                    if (!e.alive) {
                        return e;
                    }
                    if (e.seq < oldest->seq) {
                        oldest = &e;
                    }
                }
                return *oldest;
            }

            /// Take a slot and stamp a plant into it, handing back the slot so a caller that
            /// wants the first strike immediately (plant_now) does not have to hunt for it.
            event& plant_event(double pitch, double velocity) {
                event& e     = allocate();
                e.pitch      = pitch;
                e.velocity   = std::min(velocity, 1.0);
                e.brightness = m_brightness;
                e.offset     = m_pos;
                e.alive      = true;
                e.seq        = m_planted++;
                return e;
            }

            /// One pass of wear, one level up: quieter, purer, and gone below the floor.
            void bloom(event& e) {
                e.velocity *= m_decay;
                e.brightness *= m_soften;
                if (e.velocity < m_floor) {
                    e.alive = false;
                }
            }

            double                          m_sr{48000.0};
            double                          m_loop_seconds{k_default_loop_seconds};
            double                          m_decay{k_default_decay};
            double                          m_soften{k_default_soften};
            double                          m_floor{k_default_floor};
            double                          m_brightness{k_default_brightness};
            long                            m_pos{0};
            uint32_t                        m_planted{0};
            std::array<event, k_max_events> m_events;
        };

        /// The idle gardener as wind: after idle_seconds without a caller plant, strikes arrive on
        /// a calm/gust cycle. Each gust catches 1 to 5 neighboring tubes (sized by `gust`) within a
        /// fraction of a second; calms between gusts stretch so the average rate stays near one
        /// strike per loop pass at any gust setting.
        ///
        /// The rng is consumed ONLY while idling — the seed-triad contract depends on that
        /// discipline, and with idling disabled the seed cannot matter at all. A caller plant
        /// closes the idle gate mid-gust; the gust resumes if the garden idles again.
        class gardener {
          public:
            /// What the wind wants planted this sample. `pitch` is RAW — the caller quantizes,
            /// because the scale is the caller's field, not the wind's.
            struct request {
                double pitch{0.0};
                double velocity{0.0};
                bool   wanted{false};
            };

            void prepare(double sr) {
                m_sr = (sr > 0.0) ? sr : 48000.0;
                clear();
            }

            /// Re-seed the rng and restart the idle clock and the wind.
            void clear() {
                m_rng.reset();
                m_since_note = 0;
                m_gust_wait  = -1;
                m_gust_left  = 0;
                m_gust_size  = 1;
                m_gust_pitch = 69.0;
            }

            /// Seconds of silence before the gardener starts planting; 0 disables self-seeding
            /// (and then the seed cannot matter at all — pinned by test).
            void set_idle_seconds(double s) { m_idle_seconds = std::max(0.0, s); }

            /// The wind, 0..1: at 0 the gardener strikes singly and evenly (about one per pass);
            /// up from there, strikes arrive in gusts — clusters of up to five on neighboring
            /// tubes within a fraction of a second, then longer calms, same average rate.
            void set_gust(double amount) { m_gust = std::clamp(amount, 0.0, 1.0); }

            /// The gardener's seed — deterministic per seed, house triad contract. Instant.
            void set_seed(uint64_t seed) { m_rng.set_seed(seed); }

            /// A caller planted: close the idle gate.
            void notice_plant() { m_since_note = 0; }

            /// Advance the idle clock one sample and report whether the wind wants a strike.
            /// `loop_samples` is the ring's current loop length, which sizes gusts and calms.
            request tick(long loop_samples) {
                request req;
                ++m_since_note;
                if (m_idle_seconds <= 0.0) {
                    return req; // disabled: the rng is never consumed, so the seed cannot matter
                }
                if (static_cast<double>(m_since_note) < m_idle_seconds * m_sr) {
                    return req;
                }
                if (m_gust_wait < 0) { // the wind arriving: the first strike lands within half a loop
                    m_gust_wait = static_cast<long>(0.5 * uniform() * static_cast<double>(loop_samples));
                    m_gust_left = 0;
                }
                if (m_gust_wait > 0) {
                    --m_gust_wait;
                    return req;
                }
                if (m_gust_left <= 0) { // a fresh gust: how many tubes does this one catch?
                    m_gust_size  = 1 + static_cast<int>(uniform() * (1.0 + 4.0 * m_gust));
                    m_gust_left  = m_gust_size;
                    m_gust_pitch = 55.0 + 29.0 * uniform(); // a fresh place on the rack
                }
                else { // the clapper swings on to a neighboring tube
                    m_gust_pitch = std::clamp(m_gust_pitch + std::floor(9.0 * uniform()) - 4.0, 48.0, 90.0);
                }
                req.velocity = 0.3 + 0.4 * uniform();
                req.pitch    = m_gust_pitch;
                req.wanted   = true;
                --m_gust_left;
                if (m_gust_left > 0) { // within a gust: strikes tumble 30..280 ms apart
                    m_gust_wait = static_cast<long>((0.03 + 0.25 * uniform()) * m_sr);
                }
                else { // calm, stretched by the gust just spent: the average rate holds
                    m_gust_wait = static_cast<long>((0.5 + uniform()) * static_cast<double>(m_gust_size)
                                                    * static_cast<double>(loop_samples));
                }
                // Deliberately does NOT reset m_since_note's gate below the threshold: once the
                // gardener starts, it keeps tending until the caller plants again.
                return req;
            }

            double   idle_seconds() const { return m_idle_seconds; }
            double   gust() const { return m_gust; }
            uint64_t seed() const { return m_rng.seed(); }
            double   samplerate() const { return m_sr; }

          private:
            double uniform() { return 0.5 * (m_rng.process() + 1.0); } // [0, 1), the gardener's die

            double             m_sr{48000.0};
            double             m_idle_seconds{k_default_idle_seconds};
            double             m_gust{k_default_gust};
            long long          m_since_note{0};
            long               m_gust_wait{-1};
            int                m_gust_left{0};
            int                m_gust_size{1};
            double             m_gust_pitch{69.0};
            tr808::white_noise m_rng;
        };

        /// The garden bed: plant notes, they bloom on the loop, fade, and retire; left alone,
        /// the gardener plants for you. A quantizer, a ring, a rack, and a gardener wired
        /// together, plus a master level — the wiring is all this class is.
        class bed {
          public:
            // -- lifecycle -----------------------------------------------------------------------

            /// Set the rate everywhere and start an empty garden. Allocation-free by construction
            /// (fixed arrays); still not real-time-safe by the house contract.
            void prepare(double sr) {
                m_sr = (sr > 0.0) ? sr : 48000.0;
                m_rack.prepare(m_sr);
                m_ring.prepare(m_sr);
                m_gardener.prepare(m_sr);
                m_prepared = true;
                clear();
            }

            /// Uproot everything: kill all events and voices, rewind the loop, re-seed the
            /// gardener, restart the idle clock. Parameters are untouched.
            void clear() {
                m_ring.clear();
                m_rack.clear();
                m_gardener.clear();
                m_level_current = m_level_target;
            }

            bool prepared() const { return m_prepared; }

            // -- events --------------------------------------------------------------------------

            /// Plant a note: MIDI pitch (semitones, fractional accepted), velocity in (0, 1].
            /// The pitch snaps to the current root/scale, the bell sounds on the next processed
            /// sample, and the bloom returns at this loop position every pass until it fades
            /// below the floor. Resets the gardener's idle clock. A full garden retires its
            /// oldest bloom to make room.
            void note(double pitch, double velocity) {
                if (!m_prepared || velocity <= 0.0) {
                    return;
                }
                m_ring.plant(m_quantizer.quantize(pitch), velocity);
                m_gardener.notice_plant();
            }

            // -- parameter targets (safe while audio runs) ---------------------------------------

            /// Loop length in seconds — see ring::set_loop_seconds.
            void set_loop_seconds(double s) { m_ring.set_loop_seconds(s); }

            /// Velocity multiplier per pass, [0, 1]. The stabilizer: with floor f and a plant at
            /// velocity v, a bloom lives ceil(log(f/v)/log(decay)) passes, always.
            void set_decay(double per_pass) { m_ring.set_decay(per_pass); }

            /// Brightness multiplier per pass, [0, 1]: each return is purer, collapsing to sine.
            void set_soften(double per_pass) { m_ring.set_soften(per_pass); }

            /// Retirement threshold, [1e-4, 1].
            void set_floor(double v) { m_ring.set_floor(v); }

            /// The bell: envelope times in SECONDS (decay_env contract) and base brightness
            /// (0..1 scale on the modulation index). Applies to future blooms; ringing voices
            /// keep their envelope times until retriggered.
            void set_bell(double attack_s, double decay_s, double brightness) {
                m_rack.set_times(attack_s, decay_s);
                m_ring.set_brightness(brightness);
            }

            /// What the tubes are made of (material_index). A mode, not a fader: instant, and read
            /// at strike time, so every live bloom re-voices at its next return.
            void set_material(int material) { m_rack.set_material(material); }

            /// The rack's stereo width, [0, 1] — see rack::set_spread.
            void set_spread(double amount) { m_rack.set_spread(amount); }

            /// Root pitch class, 0..11 (0 = C). A mode: instant, affects future plants only.
            void set_root(int semitone) { m_quantizer.set_root(semitone); }

            /// Scale preset (scale_index). A mode: instant, affects future plants only.
            void set_scale(int scale) { m_quantizer.set_scale(scale); }

            /// Seconds of silence before the gardener starts planting; 0 disables self-seeding.
            void set_idle_seconds(double s) { m_gardener.set_idle_seconds(s); }

            /// The wind, 0..1 — see gardener::set_gust.
            void set_gust(double amount) { m_gardener.set_gust(amount); }

            /// The gardener's seed — deterministic per seed, house triad contract. Instant.
            void set_seed(uint64_t seed) { m_gardener.set_seed(seed); }

            /// Master linear output level, one-pole slewed over smooth_ms.
            void set_level(double lin) { m_level_target = lin; }

            void set_smooth_ms(double ms) { m_smooth_ms = std::max(0.0, ms); }

            // -- components ----------------------------------------------------------------------

            /// Direct access to the parts, so a caller (or a null test) can drive the bed and the
            /// components it is made of through one code path. Named for the part rather than the
            /// type, so the accessors do not shadow the class names inside this scope.
            ring&                  event_ring() { return m_ring; }
            const ring&            event_ring() const { return m_ring; }
            rack&                  chime_rack() { return m_rack; }
            const rack&            chime_rack() const { return m_rack; }
            gardener&              wind() { return m_gardener; }
            const gardener&        wind() const { return m_gardener; }
            scale_quantizer&       quantizer() { return m_quantizer; }
            const scale_quantizer& quantizer() const { return m_quantizer; }

            // -- introspection -------------------------------------------------------------------

            int      active_events() const { return m_ring.active_events(); }
            int      active_voices() const { return m_rack.active_voices(); }
            double   loop_seconds() const { return m_ring.loop_seconds(); }
            double   decay() const { return m_ring.decay(); }
            double   soften() const { return m_ring.soften(); }
            double   floor_level() const { return m_ring.floor_level(); }
            double   attack_s() const { return m_rack.attack_s(); }
            double   decay_s() const { return m_rack.decay_s(); }
            double   brightness() const { return m_ring.brightness(); }
            int      material() const { return m_rack.material(); }
            double   spread() const { return m_rack.spread(); }
            int      root() const { return m_quantizer.root(); }
            int      scale() const { return m_quantizer.scale(); }
            double   idle_seconds() const { return m_gardener.idle_seconds(); }
            double   gust() const { return m_gardener.gust(); }
            uint64_t seed() const { return m_gardener.seed(); }
            double   level() const { return m_level_target; }
            double   smooth_ms() const { return m_smooth_ms; }
            double   samplerate() const { return m_sr; }

            // -- audio ---------------------------------------------------------------------------

            /// A source: no input. Advance the loop one sample, fire any blooms whose position
            /// this is, let the gardener plant if the garden has been idle, and sum the bells
            /// onto the stereo busses, each at its tube's seat.
            void process(double& out_left, double& out_right) {
                if (!m_prepared) {
                    out_left  = 0.0;
                    out_right = 0.0;
                    return;
                }
                const int n = m_ring.due(m_fired.data(), k_max_events);
                for (int i = 0; i < n; ++i) {
                    m_rack.strike(m_fired[static_cast<size_t>(i)].pitch, m_fired[static_cast<size_t>(i)].velocity,
                                  m_fired[static_cast<size_t>(i)].brightness);
                }
                const gardener::request req = m_gardener.tick(m_ring.loop_samples());
                if (req.wanted) { // the wind plants raw; the bed's scale is what it lands on
                    const strike s = m_ring.plant_now(m_quantizer.quantize(req.pitch), req.velocity);
                    m_rack.strike(s.pitch, s.velocity, s.brightness);
                }
                m_ring.step();

                double sum_l = 0.0;
                double sum_r = 0.0;
                m_rack.process(sum_l, sum_r);
                const double coeff = (m_smooth_ms > 0.0) ? 1.0 - std::exp(-1.0 / (m_smooth_ms * 0.001 * m_sr)) : 1.0;
                m_level_current += coeff * (m_level_target - m_level_current);
                out_left  = sum_l * m_level_current;
                out_right = sum_r * m_level_current;
            }

            /// Block form: the trivial loop over the scalar path.
            void process(double* out_left, double* out_right, size_t n) {
                for (size_t i = 0; i < n; ++i) {
                    process(out_left[i], out_right[i]);
                }
            }

          private:
            double m_sr{48000.0};
            bool   m_prepared{false};
            double m_level_target{1.0};
            double m_level_current{1.0};
            double m_smooth_ms{k_default_smooth_ms};

            scale_quantizer                  m_quantizer;
            ring                             m_ring;
            rack                             m_rack;
            gardener                         m_gardener;
            std::array<strike, k_max_events> m_fired; // due() scratch: a member, so no per-sample cost
        };

    } // namespace garden
} // namespace tap::tools
