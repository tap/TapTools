/// @file
/// @brief      Offline renderer for the tap.303~ voice kernel — writes demo WAVs.
/// @details    Exercises tb303_voice.h with no Max involved: a classic 16-step acid pattern
///             with accents and slides, a decay/envmod tour, and a saw-vs-square A/B. The
///             pattern renders are the calibration material for the plan's §7.2 A/B against
///             Open303 and real-unit recordings. 48 kHz mono float32 WAVs.
///
///             Usage: tb303_render [output-directory]   (default: current directory)
/// @author     Timothy Place
/// @copyright  Copyright 2026 Timothy Place. Distributed under the New BSD License.

#include <cmath>
#include <cstdint>
#include <cstdio>
#include <string>
#include <vector>

#include <taptools/tb303_voice.h>

using taptools::tb303::voice;
namespace tb = taptools::tb303;

namespace {

    constexpr double k_g_sr = 48000.0;

    bool write_wav(const std::string& path, const std::vector<double>& samples, double sr) {
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
        u16(3);
        u16(1);
        u32(rate);
        u32(rate * 4);
        u16(4);
        u16(32);
        std::fwrite("data", 1, 4, f);
        u32(data_bytes);
        for (double s : samples) {
            const float v = static_cast<float>(s);
            std::fwrite(&v, 4, 1, f);
        }
        std::fclose(f);
        std::printf("wrote %s (%.1f s)\n", path.c_str(), n / sr);
        return true;
    }

    struct step {
        int  note; // MIDI, -1 = rest
        bool accent;
        bool slide; // hold the gate into the NEXT step and glide
    };

    // Drive the voice through a 16-step pattern at `bpm` (16th notes), `bars` times through.
    std::vector<double> play_pattern(voice& v, const std::vector<step>& pat, double bpm, int bars) {
        const double        step_s = 60.0 / bpm / 4.0;
        const size_t        step_n = static_cast<size_t>(step_s * k_g_sr);
        const size_t        gate_n = static_cast<size_t>(step_n * 0.6); // ~classic 60% gate
        std::vector<double> out;
        out.reserve(step_n * pat.size() * static_cast<size_t>(bars));
        for (int bar = 0; bar < bars; ++bar) {
            for (size_t s = 0; s < pat.size(); ++s) {
                const step& st = pat[s];
                for (size_t i = 0; i < step_n; ++i) {
                    if (st.note >= 0 && i == 0) {
                        if (v.gate()) {
                            v.set_pitch(st.note); // arrived here slid: legato glide
                        }
                        else {
                            v.note_on(st.note, st.accent ? 1.0 : 0.0);
                        }
                    }
                    if (i == gate_n && st.note >= 0 && !st.slide) {
                        v.note_off();
                    }
                    if (st.note < 0 && i == 0) {
                        v.note_off(); // rest
                    }
                    out.push_back(v.process());
                }
            }
        }
        v.note_off();
        for (size_t i = 0; i < static_cast<size_t>(0.5 * k_g_sr); ++i) {
            out.push_back(v.process());
        }
        return out;
    }

    // The house acid line: A1 pedal with octave jumps, two accents, two slides.
    const std::vector<step> k_line = {
        {33, true, false},  {33, false, false}, {45, false, true},  {33, false, false},
        {33, false, false}, {36, true, false},  {33, false, false}, {40, false, true},
        {38, false, false}, {33, false, false}, {33, false, false}, {45, false, false},
        {-1, false, false}, {36, false, true},  {35, false, false}, {31, false, false},
    };

} // namespace

int main(int argc, char** argv) {
    const std::string dir = (argc > 1) ? std::string(argv[1]) + "/" : "";

    // 1. The pattern, played straight.
    {
        voice v;
        v.prepare(k_g_sr);
        v.set_smooth_ms(0);
        v.set_cutoff(500.0);
        v.set_resonance(0.9);
        v.set_envmod(0.7);
        v.set_decay(300.0);
        v.set_accent(0.8);
        write_wav(dir + "tb303_pattern.wav", play_pattern(v, k_line, 130.0, 4), k_g_sr);
    }

    // 2. The same pattern under a slow cutoff ride — the filter-knob performance.
    {
        voice v;
        v.prepare(k_g_sr);
        v.set_smooth_ms(0);
        v.set_cutoff(250.0);
        v.set_resonance(1.0);
        v.set_envmod(0.6);
        v.set_decay(300.0);
        v.set_accent(0.9);
        v.snap();
        v.set_smooth_ms(14000.0);
        v.set_cutoff(3000.0); // ~14 s opening ride
        write_wav(dir + "tb303_cutoff_ride.wav", play_pattern(v, k_line, 130.0, 4), k_g_sr);
    }

    // 3. Envmod/decay tour: four bars each — closed, punchy, open, long.
    {
        voice v;
        v.prepare(k_g_sr);
        v.set_smooth_ms(0);
        v.set_cutoff(400.0);
        v.set_resonance(0.85);
        v.set_accent(0.8);
        std::vector<double> out;
        const double        moods[4][2] = {{0.2, 200.0}, {0.8, 200.0}, {0.8, 800.0}, {1.0, 2000.0}};
        for (auto& m : moods) {
            v.set_envmod(m[0]);
            v.set_decay(m[1]);
            auto seg = play_pattern(v, k_line, 130.0, 1);
            out.insert(out.end(), seg.begin(), seg.end());
        }
        write_wav(dir + "tb303_envmod_decay_tour.wav", out, k_g_sr);
    }

    // 4. The wow: one bar plain, then every step accented — listen to the sweep build
    //    note over note as C13 charges, then reset after the rest.
    {
        voice v;
        v.prepare(k_g_sr);
        v.set_smooth_ms(0);
        v.set_cutoff(350.0);
        v.set_resonance(1.0);
        v.set_envmod(0.25);
        v.set_decay(300.0);
        v.set_accent(1.0);
        std::vector<step> all_accent = k_line;
        for (auto& st : all_accent) {
            st.accent = (st.note >= 0);
        }
        std::vector<double> out;
        auto                plain = play_pattern(v, k_line, 130.0, 1);
        auto                wow   = play_pattern(v, all_accent, 130.0, 2);
        out.insert(out.end(), plain.begin(), plain.end());
        out.insert(out.end(), wow.begin(), wow.end());
        write_wav(dir + "tb303_wow.wav", out, k_g_sr);
    }

    // 5. Saw vs square, two bars each.
    {
        voice v;
        v.prepare(k_g_sr);
        v.set_smooth_ms(0);
        v.set_cutoff(600.0);
        v.set_resonance(0.9);
        v.set_envmod(0.6);
        v.set_decay(350.0);
        std::vector<double> out;
        for (int w : {tb::wave_saw, tb::wave_square}) {
            v.set_waveform(w);
            auto seg = play_pattern(v, k_line, 130.0, 2);
            out.insert(out.end(), seg.begin(), seg.end());
        }
        write_wav(dir + "tb303_saw_vs_square.wav", out, k_g_sr);
    }

    return 0;
}
