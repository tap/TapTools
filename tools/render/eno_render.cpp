/// @file
/// @brief      Offline renderer for the Eno family — writes demo WAVs for listening checks.
/// @details    Exercises discreet.h, airport.h, and garden.h with no Max involved (the kernels'
///             portability, demonstrated) — and, for this family, the only practical audition:
///             these are long-timescale systems, so the scenarios run minutes, not seconds.
///
///             Scenarios: `discreet_basic` (a phrase into the two-machine loop at regen 0.95),
///             `discreet_sustain` (regen 1.0 with drive — the Frippertronics wash, input faded
///             out at the halfway mark), `airport_two_one` (seven incommensurate loops, stereo,
///             three minutes), `garden_played` (four planted notes recirculating to silence),
///             and `garden_idle` (the seeded gardener left alone for two minutes).
///
///             Usage: eno_render [output-directory]   (default: current directory)
/// @author     Timothy Place
// SPDX-License-Identifier: MIT
// Copyright 2026 Timothy Place.

#include <cmath>
#include <cstdint>
#include <cstdio>
#include <string>
#include <vector>

#include <taptools/airport.h>
#include <taptools/discreet.h>
#include <taptools/garden.h>

namespace {

    constexpr double k_g_sr = 48000.0;
    constexpr double k_g_pi = 3.14159265358979323846;

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

    /// A soft additive phrase tone: fundamental + two harmonics under a sine^2 swell.
    double phrase_tone(double t, double dur, double hz) {
        if (t < 0.0 || t >= dur) {
            return 0.0;
        }
        const double env = std::pow(std::sin(k_g_pi * std::min(t / (0.66 * dur), 1.0)), 2.0);
        return env
               * (0.5 * std::sin(2.0 * k_g_pi * hz * t) + 0.22 * std::sin(2.0 * k_g_pi * 2.0 * hz * t)
                  + 0.1 * std::sin(2.0 * k_g_pi * 3.0 * hz * t));
    }

    double midi_hz(double pitch) {
        return 440.0 * std::exp2((pitch - 69.0) / 12.0);
    }

    void discreet_basic(const std::string& dir) {
        tap::tools::discreet::machine m;
        m.prepare(k_g_sr, 10.0);
        m.set_loop_seconds(5.0);
        m.set_regen(0.95);
        m.set_drive(0.4);
        m.set_darken_hz(3500.0);
        m.set_mix(60.0);

        // Four slow notes in the first fifteen seconds, then the machine on its own.
        const double        notes[][2] = {{57, 0.5}, {64, 8.0}, {62, 15.0}, {69, 21.0}};
        std::vector<double> y(static_cast<size_t>(90.0 * k_g_sr));
        for (size_t i = 0; i < y.size(); ++i) {
            const double t  = static_cast<double>(i) / k_g_sr;
            double       in = 0.0;
            for (const auto& n : notes) {
                in += 0.5 * phrase_tone(t - n[1], 4.0, midi_hz(n[0]));
            }
            y[i] = m.process(in);
        }
        write_wav(dir + "/discreet_basic.wav", y, k_g_sr);
    }

    void discreet_sustain(const std::string& dir) {
        tap::tools::discreet::machine m;
        m.prepare(k_g_sr, 10.0);
        m.set_loop_seconds(6.5);
        m.set_regen(1.0); // the point of the kernel: wear is the stabilizer
        m.set_drive(0.7);
        m.set_darken_hz(2200.0);
        m.set_mix(100.0);
        m.set_smooth_ms(2000.0);

        const double        notes[][2] = {{45, 0.5}, {57, 5.0}, {60, 11.0}, {64, 17.0}, {67, 24.0}};
        std::vector<double> y(static_cast<size_t>(120.0 * k_g_sr));
        bool                faded = false;
        for (size_t i = 0; i < y.size(); ++i) {
            const double t  = static_cast<double>(i) / k_g_sr;
            double       in = 0.0;
            for (const auto& n : notes) {
                in += 0.45 * phrase_tone(t - n[1], 5.0, midi_hz(n[0]));
            }
            if (!faded && t >= 60.0) { // the performance move: fade the send, the wash remains
                m.set_input_level(0.0);
                faded = true;
            }
            y[i] = m.process(in);
        }
        write_wav(dir + "/discreet_sustain.wav", y, k_g_sr);
    }

    void airport_two_one(const std::string& dir) {
        tap::tools::airport::loop_bank b;
        b.prepare(k_g_sr, 32.0);

        // Seven loops in the spirit of the published description: long, mutually incommensurate,
        // one soft phrase each. Lengths are deliberately awkward ratios of one another.
        const double lengths[7] = {17.8, 19.1, 21.3, 23.9, 26.2, 28.7, 30.9};
        const double pitches[7] = {57, 60, 62, 64, 65, 69, 72};
        const double pans[7]    = {-0.8, 0.8, -0.45, 0.45, -0.15, 0.15, 0.0};
        b.set_loops(7);
        for (int i = 0; i < 7; ++i) {
            b.set_length_seconds(i, lengths[i]);
            b.set_level(i, 0.45);
            b.set_pan(i, pans[i]);
        }
        b.set_darken_hz(2, 4000.0);
        b.set_darken_hz(4, 4000.0);

        std::vector<double> stereo;
        stereo.reserve(static_cast<size_t>(180.0 * k_g_sr) * 2);
        double l = 0.0, r = 0.0;

        // Record one phrase onto each loop in turn, then let the system run free.
        for (int i = 0; i < 7; ++i) {
            const double dur = 0.55 * lengths[i];
            b.record(i, true);
            const size_t n = static_cast<size_t>(dur * k_g_sr);
            for (size_t s = 0; s < n; ++s) {
                b.process(phrase_tone(static_cast<double>(s) / k_g_sr, dur, midi_hz(pitches[i])), l, r);
                stereo.push_back(l);
                stereo.push_back(r);
            }
            b.record(i, false);
        }
        while (stereo.size() < static_cast<size_t>(180.0 * k_g_sr) * 2) {
            b.process(0.0, l, r);
            stereo.push_back(l);
            stereo.push_back(r);
        }
        write_wav(dir + "/airport_two_one.wav", stereo, k_g_sr, 2);
    }

    void garden_played(const std::string& dir) {
        tap::tools::garden::bed g;
        g.prepare(k_g_sr);
        g.set_loop_seconds(5.0);
        g.set_decay(0.8);
        g.set_soften(0.85);
        g.set_bell(0.1, 2.5, 0.9);
        g.set_scale(tap::tools::garden::scale_major_pentatonic);
        g.set_root(9);
        g.set_idle_seconds(0.0); // played only: no gardener in this render
        g.set_level(0.4);

        const double        plants[][3] = {{69, 0.7, 0.2}, {76, 0.5, 1.7}, {64, 0.6, 3.4}, {81, 0.4, 4.6}};
        std::vector<double> y(static_cast<size_t>(75.0 * k_g_sr));
        size_t              next = 0;
        for (size_t i = 0; i < y.size(); ++i) {
            const double t = static_cast<double>(i) / k_g_sr;
            if (next < 4 && t >= plants[next][2]) {
                g.note(plants[next][0], plants[next][1]);
                ++next;
            }
            y[i] = g.process();
        }
        write_wav(dir + "/garden_played.wav", y, k_g_sr);
    }

    void garden_idle(const std::string& dir) {
        tap::tools::garden::bed g;
        g.prepare(k_g_sr);
        g.set_loop_seconds(6.0);
        g.set_decay(0.85);
        g.set_soften(0.9);
        g.set_bell(0.12, 3.0, 0.9);
        g.set_scale(tap::tools::garden::scale_minor_pentatonic);
        g.set_root(2);
        g.set_idle_seconds(3.0);
        g.set_seed(2008);
        g.set_level(0.4);

        std::vector<double> y(static_cast<size_t>(120.0 * k_g_sr));
        for (auto& s : y) {
            s = g.process();
        }
        write_wav(dir + "/garden_idle.wav", y, k_g_sr);
    }

} // namespace

int main(int argc, char** argv) {
    const std::string dir = (argc > 1) ? argv[1] : ".";
    discreet_basic(dir);
    discreet_sustain(dir);
    airport_two_one(dir);
    garden_played(dir);
    garden_idle(dir);
    return 0;
}
