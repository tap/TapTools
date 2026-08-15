/// @file
/// @brief      Offline renderer for the Radiohead family — writes demo WAVs for listening checks.
/// @details    Exercises tapecho.h and stammer.h with no Max involved (the kernels' portability, demonstrated).
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
///             performance).
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

#include <taptools/stammer.h>
#include <taptools/tapecho.h>

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
    return 0;
}
