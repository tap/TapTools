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
///             The voice is a small wind chime: three decaying sine modes at the transverse-
///             vibration ratios of a free-free bar, 1 : 2.756 : 5.404 (Fletcher & Rossing, The
///             Physics of Musical Instruments, 2nd ed. — the bars/tubular-chimes chapter, where
///             f_n grows as (2n+1)^2). The first mode carries the perceived pitch; the upper
///             two are inharmonic, softer (scaled by per-event brightness), and faster-dying,
///             so every strike rings down to its fundamental — which is also where the pitch
///             contract lives: a detector reads the chime in its tail, once the clang has
///             cleared (the tests measure there). Each pass multiplies the event's brightness
///             by `soften`, so a bloom does not just fade: it purifies toward its fundamental.
///             Mode amplitudes ride the shared tr808 decay_env (one per mode); a steal re-aims
///             the envelopes without a reset, so stolen voices glide, not click.
///
///             Randomness: the idle gardener draws from the family's seeded xorshift64*
///             (tr808::white_noise) — deterministic per seed, so renders and tests reproduce and
///             instances decorrelate by seed. This is the library's first randomized *event*
///             source (step_seq.h promises "no randomness anywhere"; this kernel is the deliberate
///             counterpoint, and the seed contract is the bridge back to reproducibility).
///
///             Geometry: everything is fixed arrays — k_max_events events, k_voices bells —
///             so prepare(sr) allocates nothing at all and no later call ever does.
///
///             Honest limits:
///             - Pitch is quantized AT ENTRY: changing root or scale re-pitches nothing already
///               planted, only future plants (replants pick up the new field).
///             - Event timing is the loop grid: a plant returns at its own phase point every
///               pass, exactly — there is no swing, no drift, no humanization.
///             - A full garden (k_max_events live) retires its OLDEST bloom to make room for a
///               new plant: a touch must always speak, and the oldest is the quietest.
///             - The idle gardener is statistical (about one plant per loop pass, uniformly
///               placed), not a transcription of any published piece or app behavior.
///             - Mono out; one chime timbre family. It is an instrument, not a polysynth.
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
        constexpr int k_modes      = 3;  // a small chime: three transverse modes
        // Free-free bar transverse-mode ratios (Fletcher & Rossing, The Physics of Musical
        // Instruments, 2nd ed., ch. on bars and tubular chimes: f_n proportional to (2n+1)^2).
        constexpr double k_mode_ratio[k_modes] = {1.0, 2.756, 5.404};
        // Mode levels at brightness 1, summing to <= 1 so a bell is bounded by its velocity;
        // higher modes ring softer and (below) die faster, as struck chimes do.
        constexpr double k_mode_level[k_modes] = {0.62, 0.28, 0.10};
        constexpr double k_mode_haste[k_modes] = {1.0, 2.5, 6.0}; // decay-time divisor per mode
        constexpr double k_gain_epsilon        = 1e-4;            // below this a voice is "off" (harmonizer.h idiom)

        constexpr double k_min_loop_seconds = 0.25;  // beneath this it is a buzzer, not a garden
        constexpr double k_max_loop_seconds = 120.0; // the loop is a counter — no tape is bought

        constexpr double k_default_loop_seconds = 8.0;
        constexpr double k_default_decay        = 0.85;  // velocity multiplier per pass
        constexpr double k_default_soften       = 0.9;   // brightness multiplier per pass
        constexpr double k_default_floor        = 0.03;  // retirement threshold
        constexpr double k_default_idle_seconds = 30.0;  // the gardener's patience; 0 disables
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

        /// One small wind chime: three decaying sine modes at the free-free bar's transverse
        /// ratios (k_mode_ratio), each with its own decay_env — higher modes softer (scaled by
        /// brightness) and faster-dying (k_mode_haste), the way a struck tube rings down to its
        /// fundamental. Phases free-run so a steal re-aims without a click.
        class bell {
          public:
            void prepare(double sr) {
                m_sr = (sr > 0.0) ? sr : 48000.0;
                for (auto& e : m_env) {
                    e.prepare(m_sr);
                }
                set_times(k_default_attack_s, k_default_decay_s);
            }

            void set_times(double attack_s, double decay_s) {
                for (int m = 0; m < k_modes; ++m) {
                    m_env[static_cast<size_t>(m)].set_times(attack_s, decay_s / k_mode_haste[m]);
                }
            }

            void reset() {
                for (auto& e : m_env) {
                    e.reset();
                }
                for (auto& p : m_phase) {
                    p = 0.0;
                }
            }

            /// Strike at `freq_hz` (the first mode — the perceived pitch), envelope target
            /// `level`, upper-mode weight `brightness` (0..1). Modes above the audio band stay
            /// silent rather than aliasing.
            void trigger(double freq_hz, double level, double brightness) {
                for (int m = 0; m < k_modes; ++m) {
                    const size_t i       = static_cast<size_t>(m);
                    const double mode_hz = k_mode_ratio[m] * freq_hz;
                    m_inc[i]             = mode_hz / m_sr;
                    const double weight  = (m == 0) ? k_mode_level[0] : k_mode_level[m] * brightness;
                    m_env[i].trigger((mode_hz < 0.45 * m_sr) ? level * weight : 0.0);
                }
            }

            double level() const { // the quietest-first steal key
                double sum = 0.0;
                for (const auto& e : m_env) {
                    sum += e.value();
                }
                return sum;
            }

            double process() {
                double sum = 0.0;
                for (size_t i = 0; i < static_cast<size_t>(k_modes); ++i) {
                    m_phase[i] += m_inc[i];
                    m_phase[i] -= std::floor(m_phase[i]);
                    sum += m_env[i].process() * std::sin(2.0 * k_pi * m_phase[i]);
                }
                return sum;
            }

          private:
            double                                m_sr{48000.0};
            std::array<double, k_modes>           m_phase{};
            std::array<double, k_modes>           m_inc{};
            std::array<tr808::decay_env, k_modes> m_env;
        };

        /// The garden bed: plant notes, they bloom on the loop, fade, and retire; left alone,
        /// the gardener plants for you.
        class bed {
          public:
            // -- lifecycle -----------------------------------------------------------------------

            /// Set the rate everywhere and start an empty garden. Allocation-free by construction
            /// (fixed arrays); still not real-time-safe by the house contract.
            void prepare(double sr) {
                m_sr = (sr > 0.0) ? sr : 48000.0;
                for (auto& v : m_bells) {
                    v.prepare(m_sr);
                    v.set_times(m_attack_s, m_decay_s);
                }
                m_prepared = true;
                clear();
            }

            /// Uproot everything: kill all events and voices, rewind the loop, re-seed the
            /// gardener, restart the idle clock. Parameters are untouched.
            void clear() {
                for (auto& e : m_events) {
                    e.alive = false;
                }
                for (auto& v : m_bells) {
                    v.reset();
                }
                m_rng.reset();
                m_pos           = 0;
                m_planted       = 0;
                m_since_note    = 0;
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
                event& e     = allocate();
                e.pitch      = quantize(pitch);
                e.velocity   = std::min(velocity, 1.0);
                e.brightness = m_brightness;
                e.offset     = m_pos; // process() fires it this coming sample, then every pass
                e.alive      = true;
                e.seq        = m_planted++;
                m_since_note = 0;
            }

            // -- parameter targets (safe while audio runs) ---------------------------------------

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

            /// Velocity multiplier per pass, [0, 1]. The stabilizer: with floor f and a plant at
            /// velocity v, a bloom lives ceil(log(f/v)/log(decay)) passes, always.
            void set_decay(double per_pass) { m_decay = std::clamp(per_pass, 0.0, 1.0); }

            /// Brightness multiplier per pass, [0, 1]: each return is purer, collapsing to sine.
            void set_soften(double per_pass) { m_soften = std::clamp(per_pass, 0.0, 1.0); }

            /// Retirement threshold, [1e-4, 1].
            void set_floor(double v) { m_floor = std::clamp(v, 1e-4, 1.0); }

            /// The bell: envelope times in SECONDS (decay_env contract) and base brightness
            /// (0..1 scale on the modulation index). Applies to future blooms; ringing voices
            /// keep their envelope times until retriggered.
            void set_bell(double attack_s, double decay_s, double brightness) {
                m_attack_s   = std::max(attack_s, 1e-6);
                m_decay_s    = std::max(decay_s, 1e-6);
                m_brightness = std::clamp(brightness, 0.0, 1.0);
                for (auto& v : m_bells) {
                    v.set_times(m_attack_s, m_decay_s);
                }
            }

            /// Root pitch class, 0..11 (0 = C). A mode: instant, affects future plants only.
            void set_root(int semitone) { m_root = ((semitone % 12) + 12) % 12; }

            /// Scale preset (scale_index). A mode: instant, affects future plants only.
            void set_scale(int scale) { m_scale = std::clamp(scale, 0, k_num_scales - 1); }

            /// Seconds of silence before the gardener starts planting; 0 disables self-seeding
            /// (and then the seed cannot matter at all — pinned by test).
            void set_idle_seconds(double s) { m_idle_seconds = std::max(0.0, s); }

            /// The gardener's seed — deterministic per seed, house triad contract. Instant.
            void set_seed(uint64_t seed) { m_rng.set_seed(seed); }

            /// Master linear output level, one-pole slewed over smooth_ms.
            void set_level(double lin) { m_level_target = lin; }

            void set_smooth_ms(double ms) { m_smooth_ms = std::max(0.0, ms); }

            // -- introspection -------------------------------------------------------------------

            int active_events() const {
                int n = 0;
                for (const auto& e : m_events) {
                    n += e.alive ? 1 : 0;
                }
                return n;
            }
            int active_voices() const {
                int n = 0;
                for (const auto& v : m_bells) {
                    n += (v.level() > k_gain_epsilon) ? 1 : 0;
                }
                return n;
            }
            double   loop_seconds() const { return m_loop_seconds; }
            double   decay() const { return m_decay; }
            double   soften() const { return m_soften; }
            double   floor_level() const { return m_floor; }
            double   attack_s() const { return m_attack_s; }
            double   decay_s() const { return m_decay_s; }
            double   brightness() const { return m_brightness; }
            int      root() const { return m_root; }
            int      scale() const { return m_scale; }
            double   idle_seconds() const { return m_idle_seconds; }
            uint64_t seed() const { return m_rng.seed(); }
            double   level() const { return m_level_target; }
            double   smooth_ms() const { return m_smooth_ms; }
            double   samplerate() const { return m_sr; }

            // -- audio ---------------------------------------------------------------------------

            /// A source: no input. Advance the loop one sample, fire any blooms whose position
            /// this is, let the gardener plant if the garden has been idle, and sum the bells.
            double process() {
                if (!m_prepared) {
                    return 0.0;
                }
                for (auto& e : m_events) {
                    if (e.alive && e.offset == m_pos) {
                        fire(e);
                        bloom(e);
                    }
                }
                tend();
                if (++m_pos >= loop_samples()) {
                    m_pos = 0;
                }

                double sum = 0.0;
                for (auto& v : m_bells) {
                    sum += v.process();
                }
                const double coeff = (m_smooth_ms > 0.0) ? 1.0 - std::exp(-1.0 / (m_smooth_ms * 0.001 * m_sr)) : 1.0;
                m_level_current += coeff * (m_level_target - m_level_current);
                return sum * m_level_current;
            }

            /// Block form: the trivial loop over the scalar path.
            void process(double* out, size_t n) {
                for (size_t i = 0; i < n; ++i) {
                    out[i] = process();
                }
            }

          private:
            struct event {
                double   pitch{0.0};      // MIDI semitones, already quantized
                double   velocity{0.0};   // decays per pass
                double   brightness{0.0}; // softens per pass
                long     offset{0};       // position on the loop, samples
                uint32_t seq{0};          // plant order; lowest live seq = oldest
                bool     alive{false};
            };

            long loop_samples() const { return static_cast<long>(m_loop_seconds * m_sr); }

            /// Snap MIDI semitones to the nearest pitch in the current root/scale — the tune.h
            /// nearest-allowed search (any non-empty mask has a note within a tritone).
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

            /// Strike this event now on the pool: an idle voice if any, else steal the quietest.
            void fire(event& e) {
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
                const double freq = 440.0 * std::exp2((e.pitch - 69.0) / 12.0);
                voice->trigger(freq, e.velocity, e.brightness);
            }

            /// One pass of wear, one level up: quieter, purer, and gone below the floor.
            void bloom(event& e) {
                e.velocity *= m_decay;
                e.brightness *= m_soften;
                if (e.velocity < m_floor) {
                    e.alive = false;
                }
            }

            /// The idle gardener: after idle_seconds without a caller plant, sow about one seed
            /// per loop pass, uniformly placed, on the scale, within two octaves of middle root.
            void tend() {
                ++m_since_note;
                if (m_idle_seconds <= 0.0) {
                    return; // disabled: the rng is never consumed, so the seed cannot matter
                }
                if (static_cast<double>(m_since_note) < m_idle_seconds * m_sr) {
                    return;
                }
                const double u = 0.5 * (m_rng.process() + 1.0); // [0, 1)
                if (u * static_cast<double>(loop_samples()) >= 1.0) {
                    return; // ~one plant per pass
                }
                const double pitch    = 60.0 + std::floor(12.0 * (m_rng.process() + 1.0)); // [60, 84)
                const double velocity = 0.3 + 0.2 * (m_rng.process() + 1.0);               // [0.3, 0.7)
                event&       e        = allocate();
                e.pitch               = quantize(pitch);
                e.velocity            = velocity;
                e.brightness          = m_brightness;
                e.offset              = m_pos;
                e.alive               = true;
                e.seq                 = m_planted++;
                fire(e);
                bloom(e);
                // Deliberately does NOT reset m_since_note's gate below the threshold: once the
                // gardener starts, it keeps tending until the caller plants again.
            }

            double m_sr{48000.0};
            bool   m_prepared{false};
            double m_loop_seconds{k_default_loop_seconds};
            double m_decay{k_default_decay};
            double m_soften{k_default_soften};
            double m_floor{k_default_floor};
            double m_attack_s{k_default_attack_s};
            double m_decay_s{k_default_decay_s};
            double m_brightness{k_default_brightness};
            int    m_root{0};
            int    m_scale{scale_major_pentatonic}; // anything you plant sounds consonant
            double m_idle_seconds{k_default_idle_seconds};
            double m_level_target{1.0};
            double m_level_current{1.0};
            double m_smooth_ms{k_default_smooth_ms};

            long                            m_pos{0};
            uint32_t                        m_planted{0};
            long long                       m_since_note{0};
            tr808::white_noise              m_rng;
            std::array<event, k_max_events> m_events;
            std::array<bell, k_voices>      m_bells;
        };

    } // namespace garden
} // namespace tap::tools
