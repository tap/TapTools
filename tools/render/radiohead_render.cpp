/// @file
/// @brief      Offline renderer for the Radiohead family — writes demo WAVs for listening checks.
/// @details    Exercises tapecho.h, stammer.h, fuzz.h and touche.h with no Max involved (the kernels' portability,
/// demonstrated).
///             The tape echo is a *performed* effect, so these scenarios move the controls while
///             they render rather than auditioning static settings — that is the only way to hear
///             what the kernel is actually for.
///
///             Scenarios: `tapecho_heads` (a guitar-ish phrase through the four evenly spaced
///             heads, spread across the stereo field), `tapecho_three_head` (a Copicat-style
///             three-head layout, dirtier wear, heads down the middle), `tapecho_selfosc` (the
///             design statement made audible: regeneration pushed past unity into sound-on-sound
///             howl with the saturator holding it bounded, the input faded out under it, then
///             regeneration pulled back to let it decay), and `tapecho_varispeed` (the motor
///             slewed from a short span to a long one mid-phrase — the doppler that a tape
///             machine's speed change *is*); then `stammer_grid` (the stutter's five dials held
///             still so the mechanism is audible), `stammer_disintegrate` (the performance the
///             object exists for — density, chop, hold and reversal all ridden up until the part
///             comes apart, then the reach-back opened so it quotes material from seconds ago),
///             and `stammer_two_seeds` (the same settings on two seeds back to back: a seed is a
///             performance); and `fuzz_gain_sweep` (five gain settings back to back, so the
///             sweep from edge-of-breakup to saturated is audible rather than described),
///             `fuzz_tone` (the voicing section, which is most of that pedal class's identity),
///             and `fuzz_edge_and_bite` (the knee sharpening, then the even harmonics coming
///             in); and `touche_against_a_fade`, which swells one note three times — a linear
///             fade, a fade linear in dB, and the Ondes Martenot's published intensity-key
///             curve — because the measured law is audibly neither of the obvious two; then
///             `metallique_stages` (the same phrase dry, through the gong with a linear driver,
///             with the moving-iron squared term armed, and driven hard enough that the saturator
///             is working — the order is the argument for the transducer being a real stage) and
///             `palme_halo` (the twelve strings answering a phrase, chromatic then harmonic then
///             detuned); and `scrub_gesture` (the position raked across a running loop, then the
///             same rake with the pitch riding its own independent gesture — the two hands are
///             the object) and `scrub_freeze` (two seconds recorded, the recorder stopped, and
///             nine seconds built out of that fixed tape: held, crawled through with drift, then
///             scattered with spray); and finally the Ondes Martenot itself — `triode_tubes`
///             (a sine driven progressively harder through each of the three published valves at
///             its own operating point), `ondes_stages` (the detected envelope alone, then through
///             the demodulator, the preamplifier and the power stage in turn — the first pass is
///             already harmonically rich, which is the object's whole thesis), `ondes_ribbon` (the
///             ribbon and the intensity key played, with continuous glissandi because the ribbon
///             is linear in semitones), and `ondes_diffuseurs` (the same phrase through the
///             principal, the palme and the métallique — the choice an ondes player makes).
///
///             Usage: radiohead_render [output-directory]   (default: current directory)
/// @author     Timothy Place
// SPDX-License-Identifier: MIT
// Copyright 2026 Timothy Place.

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <string>
#include <vector>

#include <taptools/diffuseur.h>
#include <taptools/fuzz.h>
#include <taptools/ondes.h>
#include <taptools/scrub.h>
#include <taptools/stammer.h>
#include <taptools/tapecho.h>
#include <taptools/touche.h>

namespace {

    constexpr double k_r_sr = 48000.0;
    constexpr double k_r_pi = 3.14159265358979323846;

    /// 48 kHz float32 WAV, 1 or 2 channels (interleaved) — same writer as the other render tools.
    bool write_wav(const std::string& path, const std::vector<double>& samples, double sr, uint16_t channels = 1) {
        std::FILE* f = std::fopen(path.c_str(), "wb");
        if (!f) {
            std::fprintf(stderr, "cannot open %s\n", path.c_str());
            return false;
        }
        const uint32_t n          = static_cast<uint32_t>(samples.size());
        const uint32_t data_bytes = n * 4;
        const uint32_t rate       = static_cast<uint32_t>(sr);

        auto u16 = [&](uint16_t v) { std::fwrite(&v, 2, 1, f); };
        auto u32 = [&](uint32_t v) { std::fwrite(&v, 4, 1, f); };

        std::fwrite("RIFF", 1, 4, f);
        u32(36 + data_bytes);
        std::fwrite("WAVE", 1, 4, f);
        std::fwrite("fmt ", 1, 4, f);
        u32(16);
        u16(3); // IEEE float
        u16(channels);
        u32(rate);
        u32(rate * 4 * channels);
        u16(static_cast<uint16_t>(4 * channels)); // block align
        u16(32);                                  // bits
        std::fwrite("data", 1, 4, f);
        u32(data_bytes);
        for (double s : samples) {
            const float v = static_cast<float>(s);
            std::fwrite(&v, 4, 1, f);
        }
        std::fclose(f);
        std::printf("wrote %s (%.1f s)\n", path.c_str(), n / (sr * channels));
        return true;
    }

    /// The kernel sums its heads with no master gain — gain staging is the caller's job, and for
    /// these renders this tool is the caller. Each scenario carries an explicit trim chosen so the
    /// file peaks below unity on playback; nothing here is normalized after the fact, so the
    /// relative loudness *within* a render (a howl building over a phrase) is the kernel's own.
    void write_scenario(const std::string& path, std::vector<double> samples, double trim, uint16_t channels = 2) {
        for (double& s : samples) {
            s *= trim;
        }
        write_wav(path, samples, k_r_sr, channels);
    }

    double midi_hz(double pitch) {
        return 440.0 * std::exp2((pitch - 69.0) / 12.0);
    }

    /// A plucked-string-ish source: a decaying harmonic stack with a bright, fast attack. Echo
    /// scenarios want transients — a pad would hide the head layout entirely.
    double pluck(double t, double hz) {
        if (t < 0.0) {
            return 0.0;
        }
        const double attack = 1.0 - std::exp(-t * 900.0);
        double       sum    = 0.0;
        for (int k = 1; k <= 6; ++k) {
            const double kk    = static_cast<double>(k);
            const double decay = std::exp(-t * (3.0 + 2.2 * kk)); // higher partials die first
            sum += (1.0 / kk) * decay * std::sin(2.0 * k_r_pi * hz * kk * t);
        }
        return 0.45 * attack * sum;
    }

    /// A phrase as a list of (onset seconds, midi pitch) plucks, summed at time t.
    struct note {
        double onset;
        double pitch;
    };

    double phrase(const std::vector<note>& notes, double t) {
        double sum = 0.0;
        for (const note& n : notes) {
            sum += pluck(t - n.onset, midi_hz(n.pitch));
        }
        return sum;
    }

    /// The house progression for these renders — a slow minor arpeggio with room between the
    /// notes for the repeats to be heard against.
    const std::vector<note>& demo_phrase() {
        static const std::vector<note> notes = {{0.00, 45.0}, {0.75, 52.0}, {1.50, 57.0}, {2.25, 60.0},
                                                {3.00, 64.0}, {4.50, 57.0}, {6.00, 52.0}};
        return notes;
    }

    /// The phrase, looped — the stammer scenarios run long enough that seven notes would leave
    /// the machine chewing on silence.
    double looping_phrase(double t) {
        return phrase(demo_phrase(), std::fmod(t, 7.5));
    }

    // ---- scenarios -----------------------------------------------------------------------------

    void tapecho_heads(const std::string& dir) {
        tap::tools::tapecho::machine m;
        m.prepare(k_r_sr, 2.0);
        m.set_span_ms(480.0);
        m.set_heads(4); // the default even spacing: 0.25, 0.5, 0.75, 1.0 of the span
        const double pans[4] = {-0.7, 0.5, -0.35, 0.8};
        for (int i = 0; i < 4; ++i) {
            m.set_head_pan(i, pans[i]);
            m.set_head_level(i, 0.9 - 0.15 * static_cast<double>(i)); // nearer heads a touch louder
        }
        m.set_regen(0.45);
        m.set_drive(0.4);
        m.set_darken_hz(4200.0);
        m.set_mix(45.0);

        const size_t        frames = static_cast<size_t>(16.0 * k_r_sr);
        std::vector<double> stereo(2 * frames);
        for (size_t i = 0; i < frames; ++i) {
            const double t = static_cast<double>(i) / k_r_sr;
            m.process(phrase(demo_phrase(), t), stereo[2 * i], stereo[2 * i + 1]);
        }
        write_scenario(dir + "/tapecho_heads.wav", stereo, 0.5);
    }

    void tapecho_three_head(const std::string& dir) {
        tap::tools::tapecho::machine m;
        m.prepare(k_r_sr, 2.0);
        m.set_span_ms(390.0);
        m.set_heads(3); // a Copicat-style three-head layout, set explicitly
        for (int i = 0; i < 3; ++i) {
            m.set_head_ratio(i, static_cast<double>(i + 1) / 3.0);
            m.set_head_pan(i, 0.0);
            m.set_head_level(i, 1.0);
        }
        m.set_regen(0.6);
        m.set_drive(0.9); // a hotter record head: the repeats thicken as they recirculate
        m.set_darken_hz(2600.0);
        m.set_wow(0.9, 0.9); // a tired transport
        m.set_flutter(0.06, 13.0);
        m.set_mix(50.0);

        const size_t        frames = static_cast<size_t>(16.0 * k_r_sr);
        std::vector<double> stereo(2 * frames);
        for (size_t i = 0; i < frames; ++i) {
            const double t = static_cast<double>(i) / k_r_sr;
            m.process(phrase(demo_phrase(), t), stereo[2 * i], stereo[2 * i + 1]);
        }
        write_scenario(dir + "/tapecho_three_head.wav", stereo, 0.5);
    }

    void tapecho_selfosc(const std::string& dir) {
        // The kernel's design statement, as a performance: play a phrase, push regeneration past
        // unity, fade the input away and let the loop howl on its own — bounded by the saturator,
        // not by a feedback cap — then pull regeneration back and let it die.
        tap::tools::tapecho::machine m;
        m.prepare(k_r_sr, 2.0);
        m.set_span_ms(420.0);
        m.set_heads(2);
        m.set_head_ratio(0, 0.5);
        m.set_head_pan(0, -0.5);
        m.set_head_ratio(1, 1.0);
        m.set_head_pan(1, 0.5);
        m.set_regen(0.5);
        m.set_drive(0.7);
        m.set_darken_hz(3800.0);
        m.set_smooth_ms(400.0); // the controls are being *ridden*, so slew them like a fader
        m.set_mix(60.0);

        const size_t        frames = static_cast<size_t>(40.0 * k_r_sr);
        std::vector<double> stereo(2 * frames);
        for (size_t i = 0; i < frames; ++i) {
            const double t = static_cast<double>(i) / k_r_sr;
            if (i == static_cast<size_t>(9.0 * k_r_sr)) {
                m.set_regen(1.35); // past unity: the loop starts building on itself
            }
            if (i == static_cast<size_t>(14.0 * k_r_sr)) {
                m.set_input_level(0.0); // hands off the instrument; the machine plays alone
            }
            if (i == static_cast<size_t>(26.0 * k_r_sr)) {
                m.set_darken_hz(1400.0); // ride the tone control while it howls
            }
            if (i == static_cast<size_t>(32.0 * k_r_sr)) {
                m.set_regen(0.55); // and bring it home
            }
            m.process(phrase(demo_phrase(), t), stereo[2 * i], stereo[2 * i + 1]);
        }
        write_scenario(dir + "/tapecho_selfosc.wav", stereo, 0.35);
    }

    void tapecho_varispeed(const std::string& dir) {
        // A motor change is a tape-speed change: the repeats already on the tape bend in pitch as
        // the heads move. Slow slews make it a dive; there is no crossfaded "digital" mode.
        tap::tools::tapecho::machine m;
        m.prepare(k_r_sr, 3.0);
        m.set_span_ms(200.0);
        m.set_heads(2);
        m.set_head_ratio(0, 0.5);
        m.set_head_pan(0, -0.4);
        m.set_head_ratio(1, 1.0);
        m.set_head_pan(1, 0.4);
        m.set_regen(0.7);
        m.set_drive(0.5);
        m.set_darken_hz(5000.0);
        m.set_mix(65.0);

        const size_t        frames = static_cast<size_t>(24.0 * k_r_sr);
        std::vector<double> stereo(2 * frames);
        for (size_t i = 0; i < frames; ++i) {
            const double t = static_cast<double>(i) / k_r_sr;
            if (i == static_cast<size_t>(8.0 * k_r_sr)) {
                m.set_smooth_ms(3000.0);
                m.set_span_ms(900.0); // spool out over three seconds: the dive
            }
            if (i == static_cast<size_t>(16.0 * k_r_sr)) {
                m.set_smooth_ms(1200.0);
                m.set_span_ms(200.0); // and back up
            }
            m.process(phrase(demo_phrase(), t), stereo[2 * i], stereo[2 * i + 1]);
        }
        write_scenario(dir + "/tapecho_varispeed.wav", stereo, 0.35);
    }

    // ---- tap.stammer~ ----------------------------------------------------------------------------

    /// The five dials that are the instrument, held still so the mechanism is audible on its own.
    void stammer_grid(const std::string& dir) {
        tap::tools::stammer::machine m;
        m.prepare(k_r_sr, 2000.0);
        m.set_step_ms(250.0);
        m.set_density(0.55);
        m.set_divisions(4);
        m.set_repeats(4);
        m.set_reverse(0.2);
        m.set_fade_ms(3.0);
        m.set_seed(1999); // the year the first TapTools shipped
        m.set_mix(100.0);

        const size_t        frames = static_cast<size_t>(24.0 * k_r_sr);
        std::vector<double> mono(frames);
        for (size_t i = 0; i < frames; ++i) {
            mono[i] = m.process(looping_phrase(static_cast<double>(i) / k_r_sr));
        }
        write_scenario(dir + "/stammer_grid.wav", mono, 0.8, 1);
    }

    /// The performance the object exists for: a part that comes apart in your hands. Density,
    /// chop, hold and reversal all ride up over the render, and the reach-back opens at the end so
    /// the machine starts quoting material from seconds ago rather than the bar just played.
    void stammer_disintegrate(const std::string& dir) {
        tap::tools::stammer::machine m;
        m.prepare(k_r_sr, 4000.0);
        m.set_step_ms(250.0);
        m.set_density(0.15);
        m.set_divisions(1);
        m.set_repeats(1);
        m.set_reverse(0.0);
        m.set_fade_ms(4.0);
        m.set_seed(2003); // Hail to the Thief
        m.set_mix(100.0);

        const double        seconds = 40.0;
        const size_t        frames  = static_cast<size_t>(seconds * k_r_sr);
        std::vector<double> mono(frames);
        for (size_t i = 0; i < frames; ++i) {
            const double t = static_cast<double>(i) / k_r_sr;
            const double u = t / seconds; // 0 -> 1 across the render: the hands on the machine
            m.set_density(0.15 + 0.8 * u);
            m.set_divisions(1 + static_cast<int>(u * 7.99));
            m.set_repeats(1 + static_cast<int>(u * 9.99));
            m.set_reverse(0.6 * u);
            m.set_step_ms(250.0 - 130.0 * u);
            if (t > 0.75 * seconds) {
                m.set_jump_ms(1500.0); // and now it reaches back past the bar
            }
            mono[i] = m.process(looping_phrase(t));
        }
        write_scenario(dir + "/stammer_disintegrate.wav", mono, 0.8, 1);
    }

    /// A seed is a performance: the same settings on two seeds, back to back in one file, so the
    /// difference is the dice and nothing else. Each half is bit-reproducible on its own.
    void stammer_two_seeds(const std::string& dir) {
        const double        half   = 12.0;
        const size_t        frames = static_cast<size_t>(half * k_r_sr);
        std::vector<double> mono;
        mono.reserve(2 * frames);

        for (uint64_t seed : {uint64_t{1}, uint64_t{2}}) {
            tap::tools::stammer::machine m;
            m.prepare(k_r_sr, 2000.0);
            m.set_step_ms(200.0);
            m.set_density(0.7);
            m.set_divisions(4);
            m.set_repeats(5);
            m.set_reverse(0.35);
            m.set_fade_ms(3.0);
            m.set_seed(seed);
            m.set_mix(100.0);
            for (size_t i = 0; i < frames; ++i) {
                mono.push_back(m.process(looping_phrase(static_cast<double>(i) / k_r_sr)));
            }
        }
        write_scenario(dir + "/stammer_two_seeds.wav", mono, 0.8, 1);
    }

    // ---- tap.fuzz~ -------------------------------------------------------------------------------

    /// The gain knob doing what a gain knob should: the same phrase at five settings, so the
    /// sweep from edge-of-breakup to saturated is audible in one file rather than described.
    void fuzz_gain_sweep(const std::string& dir) {
        const double        step   = 4.0;
        const size_t        frames = static_cast<size_t>(step * k_r_sr);
        std::vector<double> mono;
        mono.reserve(5 * frames);

        for (double g : {0.0, 0.25, 0.5, 0.75, 1.0}) {
            tap::tools::fuzz::pedal p;
            p.prepare(k_r_sr);
            p.set_gain(g);
            p.set_edge(0.4);
            p.set_asymmetry(0.15);
            p.set_bass(0.2);
            p.set_treble(0.1);
            p.set_contrast(0.35);
            for (size_t i = 0; i < frames; ++i) {
                mono.push_back(p.process(looping_phrase(static_cast<double>(i) / k_r_sr)));
            }
        }
        write_scenario(dir + "/fuzz_gain_sweep.wav", mono, 0.7, 1);
    }

    /// The tone section, which on this class of pedal is most of the identity: flat, scooped,
    /// and the two shelves at their travel, twelve seconds apiece.
    void fuzz_tone(const std::string& dir) {
        struct setting {
            double bass, treble, contrast;
        };
        const setting settings[4] = {{0.0, 0.0, 0.0}, {0.0, 0.0, 1.0}, {0.6, -0.6, 0.5}, {-0.6, 0.8, 0.5}};

        const size_t        frames = static_cast<size_t>(8.0 * k_r_sr);
        std::vector<double> mono;
        mono.reserve(4 * frames);
        for (const setting& v : settings) {
            tap::tools::fuzz::pedal p;
            p.prepare(k_r_sr);
            p.set_gain(0.7);
            p.set_edge(0.5);
            p.set_bass(v.bass);
            p.set_treble(v.treble);
            p.set_contrast(v.contrast);
            for (size_t i = 0; i < frames; ++i) {
                mono.push_back(p.process(looping_phrase(static_cast<double>(i) / k_r_sr)));
            }
        }
        write_scenario(dir + "/fuzz_tone.wav", mono, 0.4, 1); // the bass-shelf setting is the peak here
    }

    /// The two controls a static-curve model has to be honest about, ridden rather than set:
    /// `edge` sharpening the knee toward a corner, and `asymmetry` bringing in the even
    /// harmonics an odd-only curve cannot make.
    void fuzz_edge_and_bite(const std::string& dir) {
        tap::tools::fuzz::pedal p;
        p.prepare(k_r_sr);
        p.set_gain(0.8);
        p.set_contrast(0.4);
        p.set_smooth_ms(300.0);
        p.set_oversample(8); // the honest setting for a hard knee

        const double        seconds = 24.0;
        const size_t        frames  = static_cast<size_t>(seconds * k_r_sr);
        std::vector<double> mono(frames);
        for (size_t i = 0; i < frames; ++i) {
            const double t = static_cast<double>(i) / k_r_sr;
            const double u = t / seconds;
            p.set_edge(u < 0.5 ? 2.0 * u : 1.0);              // first half: the knee sharpens
            p.set_asymmetry(u < 0.5 ? 0.0 : 2.0 * (u - 0.5)); // second half: the bite comes in
            mono[i] = p.process(looping_phrase(t));
        }
        write_scenario(dir + "/fuzz_edge_and_bite.wav", mono, 0.7, 1);
    }

    // ---- tap.touche~ -----------------------------------------------------------------------------

    /// The intensity key against the obvious alternative. The same note is swelled and released
    /// three times: once through a straight linear fade, once through a straight fade in dB, and
    /// once through the published curve. The point is that the measured law is neither — it
    /// steepens through the middle of the travel and flattens at the top, which is what lets a
    /// player place a crescendo where they want it instead of where the taper puts it.
    void touche_against_a_fade(const std::string& dir) {
        tap::tools::touche::key k;
        k.prepare(k_r_sr);
        k.set_smooth_ms(0.0);

        const double        gesture = 3.0; // seconds up, then the same back down
        const size_t        frames  = static_cast<size_t>(2.0 * gesture * k_r_sr);
        std::vector<double> mono;
        mono.reserve(3 * frames);

        for (int law = 0; law < 3; ++law) {
            double phase = 0.0;
            for (size_t i = 0; i < frames; ++i) {
                const double t = static_cast<double>(i) / k_r_sr;
                // A triangle over the gesture: press in, release out.
                const double u = (t < gesture) ? (t / gesture) : (2.0 - t / gesture);
                // A steady tone, so the only thing moving is the gain law.
                phase += 220.0 / k_r_sr;
                phase -= std::floor(phase);
                const double tone = 0.4 * std::sin(2.0 * k_r_pi * phase);

                double g = 0.0;
                if (law == 0) {
                    g = u; // linear in amplitude
                }
                else if (law == 1) {
                    g = (u <= 0.0) ? 0.0 : std::pow(10.0, (-50.0 * (1.0 - u)) / 20.0); // linear in dB
                }
                else {
                    g = k.gain_at(u); // the published curve
                }
                mono.push_back(tone * g);
            }
        }
        write_scenario(dir + "/touche_against_a_fade.wav", mono, 0.9, 1);
    }

    // ---- the diffuseurs ---------------------------------------------------------------------------

    /// The métallique against a bare signal. The same phrase runs four times: dry, then through
    /// the gong with the transducer linear, then with the moving-iron squared term armed, then
    /// with the drive pushed so the saturator is doing real work. The order is the argument —
    /// a diffuseur modelled as a resonator alone is missing a documented stage, and the third
    /// and fourth passes are what that stage sounds like.
    void metallique_stages(const std::string& dir) {
        const double        pass   = 9.0;
        const size_t        frames = static_cast<size_t>(pass * k_r_sr);
        std::vector<double> mono;
        mono.reserve(4 * frames);

        for (int stage = 0; stage < 4; ++stage) {
            tap::tools::diffuseur::metallique m;
            m.prepare(k_r_sr);
            m.set_pitch_hz(146.0);
            m.set_decay(7.0);
            m.set_tilt(0.9);
            m.set_brightness(0.85);
            m.set_mix((stage == 0) ? 0.0 : 70.0);
            m.set_drive((stage == 3) ? 6.0 : 1.0);
            m.set_asymmetry((stage >= 2) ? 0.45 : 0.0);
            m.set_saturation((stage >= 2) ? 0.6 : 0.0);
            // Levelled so the four passes are comparable by ear: the body is a colouring, not a
            // boost, and the hard-driven pass is far louder than the rest.
            m.set_level((stage == 0) ? 1.0 : ((stage == 3) ? 0.45 : 1.9));

            for (size_t i = 0; i < frames; ++i) {
                mono.push_back(m.process(phrase(demo_phrase(), static_cast<double>(i) / k_r_sr)));
            }
        }
        write_scenario(dir + "/metallique_stages.wav", mono, 0.7, 1);
    }

    /// The palme's halo. A phrase in A minor into the twelve chromatic strings, then the same
    /// phrase into the harmonic tuning on the same root — the first answers every note, the
    /// second answers only what belongs to A. Then the detune opened, so the board beats.
    void palme_halo(const std::string& dir) {
        const double        pass   = 11.0;
        const size_t        frames = static_cast<size_t>(pass * k_r_sr);
        std::vector<double> mono;
        mono.reserve(3 * frames);

        const int    tunings[3] = {tap::tools::diffuseur::tuning_chromatic, tap::tools::diffuseur::tuning_harmonic,
                                   tap::tools::diffuseur::tuning_chromatic};
        const double detunes[3] = {0.0, 0.0, 22.0};

        for (int pass_index = 0; pass_index < 3; ++pass_index) {
            tap::tools::diffuseur::palme p;
            p.prepare(k_r_sr);
            p.set_root_hz(110.0);
            p.set_tuning(tunings[pass_index]);
            p.set_decay(5.0);
            p.set_damping(3500.0);
            p.set_detune(detunes[pass_index]);
            p.set_drive(1.0);
            p.set_asymmetry(0.2);
            p.set_saturation(0.35);
            p.set_mix(55.0);
            p.set_level(0.5); // twelve resonant loops add up

            for (size_t i = 0; i < frames; ++i) {
                mono.push_back(p.process(phrase(demo_phrase(), static_cast<double>(i) / k_r_sr)));
            }
        }
        write_scenario(dir + "/palme_halo.wav", mono, 0.4, 1); // twelve high-Q loops peak hard
    }

    // ---- tap.scrub~ -------------------------------------------------------------------------------

    /// The scrub as it is actually played: the position dragged back and forth across the last
    /// second and a half of a loop that keeps running underneath, first at unity pitch, then with
    /// the pitch riding its own independent gesture. The two hands are the object, so the render
    /// moves both.
    void scrub_gesture(const std::string& dir) {
        const double        pass   = 12.0;
        const size_t        frames = static_cast<size_t>(pass * k_r_sr);
        std::vector<double> mono;
        mono.reserve(2 * frames);

        for (int with_pitch = 0; with_pitch < 2; ++with_pitch) {
            tap::tools::scrub::machine m;
            m.prepare(k_r_sr, 3000.0);
            m.set_size_ms(70.0);
            m.set_overlap(2);
            m.set_mix(100.0);
            m.set_smooth_ms(8.0);

            for (size_t i = 0; i < frames; ++i) {
                const double t = static_cast<double>(i) / k_r_sr;
                // A rake back and forth: slow at first, then faster as the gesture takes hold.
                const double rate  = 0.25 + 0.45 * (t / pass);
                const double sweep = 0.5 - 0.5 * std::cos(2.0 * k_r_pi * rate * t);
                m.set_position_ms(1500.0 * sweep);
                if (with_pitch != 0) {
                    m.set_pitch(7.0 * std::sin(2.0 * k_r_pi * 0.11 * t));
                }
                mono.push_back(m.process(looping_phrase(t)));
            }
        }
        write_scenario(dir + "/scrub_gesture.wav", mono, 0.8, 1);
    }

    /// The frozen half of the object: two seconds of the phrase go in, the recorder stops, and
    /// everything afterwards is made out of that fixed tape — first held still, then crawled
    /// through with `drift`, then scattered with `spray`. Nothing is going into the input at all
    /// after the freeze, which is the point.
    void scrub_freeze(const std::string& dir) {
        tap::tools::scrub::machine m;
        m.prepare(k_r_sr, 3000.0);
        m.set_size_ms(110.0);
        m.set_overlap(3);
        m.set_mix(100.0);
        m.set_smooth_ms(20.0);
        m.set_position_ms(900.0);
        m.set_seed(7);

        const size_t        live = static_cast<size_t>(2.5 * k_r_sr);
        const size_t        held = static_cast<size_t>(9.0 * k_r_sr);
        std::vector<double> mono;
        mono.reserve(live + held);

        for (size_t i = 0; i < live; ++i) {
            mono.push_back(m.process(looping_phrase(static_cast<double>(i) / k_r_sr)));
        }
        m.set_freeze(true);
        for (size_t i = 0; i < held; ++i) {
            const double t = static_cast<double>(i) / k_r_sr;
            if (t > 3.0 && t <= 6.0) {
                m.set_drift(-0.35); // crawl backwards through the frozen tape
            }
            else if (t > 6.0) {
                m.set_drift(0.0);
                m.set_spray_ms(260.0); // and then let the origin scatter
            }
            mono.push_back(m.process(0.0)); // nothing going in: all of this is the frozen tape
        }
        write_scenario(dir + "/scrub_freeze.wav", mono, 0.8, 1);
    }

    // ---- the Ondes Martenot -------------------------------------------------------------------

    /// The chain, one stage at a time. The same held note four times: the detected envelope on
    /// its own, then through the demodulator triode, then the preamplifier as well, then with the
    /// 2A3 power stage switched in. The point of hearing it this way is that the first pass is
    /// already harmonically rich — the demodulator makes those harmonics, not the tubes.
    void ondes_stages(const std::string& dir) {
        const double        pass   = 4.0;
        const size_t        frames = static_cast<size_t>(pass * k_r_sr);
        std::vector<double> mono;
        mono.reserve(4 * frames);

        for (int stage = 0; stage < 4; ++stage) {
            if (stage == 0) {
                // The detector alone: no tube in the path at all.
                tap::tools::ondes::detector d;
                d.prepare(k_r_sr);
                d.set_ribbon(24.0);
                double dc = 0.0;
                for (size_t i = 0; i < frames; ++i) {
                    const double e = d.process();
                    dc += 0.001 * (e - dc); // the coupling capacitor the tubes would have provided
                    mono.push_back(0.35 * (e - dc));
                }
                continue;
            }
            tap::tools::ondes::voice v;
            v.prepare(k_r_sr);
            v.set_ribbon(24.0);
            v.set_key(1.0);
            v.set_level(0.35);
            v.set_drive(stage >= 2 ? 4.0 : 1.0);
            v.set_power_stage(stage == 3);
            for (size_t i = 0; i < frames; ++i) {
                mono.push_back(v.process());
            }
        }
        write_scenario(dir + "/ondes_stages.wav", mono, 0.8, 1);
    }

    /// The ribbon and the key, played. A slow phrase whose pitch glides continuously — the ribbon
    /// is linear in semitones, so a linear hand movement is a linear glissando — with the
    /// intensity key shaping every note. Nothing here is quantized, because the instrument does
    /// not quantize.
    void ondes_ribbon(const std::string& dir) {
        tap::tools::ondes::voice v;
        v.prepare(k_r_sr);
        v.set_smooth_ms(4.0);
        v.set_drive(2.5);
        v.set_level(0.5);

        // (arrival time, semitones above A1) — the hand slides between them.
        const std::vector<note> stops  = {{0.0, 19.0}, {2.2, 26.0},  {4.0, 24.0}, {6.0, 31.0},
                                          {8.4, 29.0}, {10.5, 22.0}, {13.0, 19.0}};
        const size_t            frames = static_cast<size_t>(16.0 * k_r_sr);
        std::vector<double>     mono(frames);

        for (size_t i = 0; i < frames; ++i) {
            const double t = static_cast<double>(i) / k_r_sr;
            // Where the hand is: interpolation between stops, which on this instrument really is
            // a glissando rather than a portamento between fixed pitches.
            double st = stops.back().pitch;
            for (size_t k = 0; k + 1 < stops.size(); ++k) {
                if (t >= stops[k].onset && t < stops[k + 1].onset) {
                    const double u = (t - stops[k].onset) / (stops[k + 1].onset - stops[k].onset);
                    const double e = u * u * (3.0 - 2.0 * u); // eased, so it reads as a hand
                    st             = stops[k].pitch + e * (stops[k + 1].pitch - stops[k].pitch);
                    break;
                }
            }
            v.set_ribbon(st);
            // The left hand on the key: a swell per note.
            double press = 0.0;
            for (size_t k = 0; k < stops.size(); ++k) {
                const double dt = t - stops[k].onset;
                if (dt >= -0.35 && dt < 1.9) {
                    const double a = std::clamp((dt + 0.35) / 0.5, 0.0, 1.0);
                    const double r = std::clamp(1.0 - (dt - 0.9) / 1.0, 0.0, 1.0);
                    press          = std::max(press, std::min(a, r));
                }
            }
            v.set_key(0.45 + 0.55 * press); // never quite off: the key's dead zone does the rest
            mono[i] = v.process();
        }
        write_scenario(dir + "/ondes_ribbon.wav", mono, 0.8, 1);
    }

    /// The whole instrument, finally: the voice into each of its loudspeakers in turn. The same
    /// phrase through the principal (dry), the palme, and the métallique — which is the choice an
    /// ondes player actually makes.
    void ondes_diffuseurs(const std::string& dir) {
        const size_t        frames = static_cast<size_t>(11.0 * k_r_sr);
        std::vector<double> mono;
        mono.reserve(3 * frames);

        for (int cabinet = 0; cabinet < 3; ++cabinet) {
            tap::tools::ondes::voice v;
            v.prepare(k_r_sr);
            v.set_smooth_ms(4.0);
            v.set_drive(3.0);
            v.set_level(0.5);

            tap::tools::diffuseur::palme palme;
            palme.prepare(k_r_sr);
            palme.set_root_hz(110.0);
            palme.set_decay(4.0);
            palme.set_damping(3000.0);
            palme.set_drive(1.0);
            palme.set_asymmetry(0.2);
            palme.set_saturation(0.3);
            palme.set_mix(60.0);
            palme.set_level(0.5);

            tap::tools::diffuseur::metallique gong;
            gong.prepare(k_r_sr);
            gong.set_pitch_hz(165.0);
            gong.set_decay(6.0);
            gong.set_brightness(0.8);
            gong.set_drive(1.4);
            gong.set_asymmetry(0.35);
            gong.set_saturation(0.5);
            gong.set_mix(60.0);
            gong.set_level(1.6);

            for (size_t i = 0; i < frames; ++i) {
                const double t  = static_cast<double>(i) / k_r_sr;
                const double st = 24.0 + 5.0 * std::sin(2.0 * k_r_pi * 0.09 * t) + ((t > 5.5) ? 7.0 : 0.0);
                v.set_ribbon(st);
                v.set_key(0.55 + 0.45 * (0.5 - 0.5 * std::cos(2.0 * k_r_pi * 0.22 * t)));
                const double dry = v.process();
                mono.push_back(cabinet == 0 ? dry : ((cabinet == 1) ? palme.process(dry) : gong.process(dry)));
            }
        }
        write_scenario(dir + "/ondes_diffuseurs.wav", mono, 0.45, 1); // the gong pass is much the loudest
    }

    /// The three tubes, on the same signal. A 220 Hz sine driven progressively harder through the
    /// 6C5 at the demodulator's operating point, then the preamplifier's, then the 2A3 — same
    /// sweep each time, so what changes is only what each valve does with it.
    void triode_tubes(const std::string& dir) {
        struct setup {
            int                                tube;
            tap::tools::ondes::operating_point op;
        };
        const setup setups[3] = {{tap::tools::ondes::tube_6c5, tap::tools::ondes::k_op_demod},
                                 {tap::tools::ondes::tube_6c5, tap::tools::ondes::k_op_preamp},
                                 {tap::tools::ondes::tube_2a3, tap::tools::ondes::k_op_power}};

        const size_t        frames = static_cast<size_t>(6.0 * k_r_sr);
        std::vector<double> mono;
        mono.reserve(3 * frames);

        for (const setup& s : setups) {
            tap::tools::ondes::triode t;
            t.prepare(k_r_sr);
            t.set_tube(s.tube);
            t.set_operating_point(s.op);
            double phase = 0.0;
            for (size_t i = 0; i < frames; ++i) {
                const double u = static_cast<double>(i) / static_cast<double>(frames);
                t.set_drive(0.2 + 12.0 * u * u); // the grid swing rising through the whole pass
                phase += 220.0 / k_r_sr;
                phase -= std::floor(phase);
                mono.push_back(0.5 * t.process(std::sin(2.0 * k_r_pi * phase)));
            }
        }
        write_scenario(dir + "/triode_tubes.wav", mono, 0.7, 1);
    }

} // namespace

int main(int argc, char** argv) {
    const std::string dir = (argc > 1) ? argv[1] : ".";
    tapecho_heads(dir);
    tapecho_three_head(dir);
    tapecho_selfosc(dir);
    tapecho_varispeed(dir);
    stammer_grid(dir);
    stammer_disintegrate(dir);
    stammer_two_seeds(dir);
    fuzz_gain_sweep(dir);
    fuzz_tone(dir);
    fuzz_edge_and_bite(dir);
    touche_against_a_fade(dir);
    metallique_stages(dir);
    palme_halo(dir);
    scrub_gesture(dir);
    scrub_freeze(dir);
    triode_tubes(dir);
    ondes_stages(dir);
    ondes_ribbon(dir);
    ondes_diffuseurs(dir);
    return 0;
}
