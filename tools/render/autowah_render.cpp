/// @file
/// @brief      Offline renderer for the tap.autowah~ kernel — demo WAVs plus a WAV-in mode.
/// @details    Exercises autowah.h with no Max involved. Two uses:
///
///             1. Demo mode (no input file): renders the listening-check scenarios — the four
///                factory voicings on funk-chop noise bursts, a slow swell, a cocked-wah bias
///                sweep, a sidechain demo, and a driven high-resonance pass — as 48 kHz mono
///                float32 WAVs.
///             2. WAV-in mode (`--in file.wav [slot]`): runs a mono float32/PCM16 WAV (e.g. a dry
///                DI recording, or the exact stimulus that was reamped through the hardware pedal)
///                through a factory slot, for the hardware-vs-model comparisons in the validation
///                plan (TapTools-Max plans/tap.autowah~.md §6).
///
///             Usage: autowah_render [output-directory]
///                    autowah_render --in dry.wav [preset-slot] [output-directory]
/// @author     Timothy Place
/// @copyright  Copyright 2026 Timothy Place. Distributed under the New BSD License.

#include <cstdint>
#include <cstdio>
#include <cstring>
#include <string>
#include <vector>

#include <taptools/autowah.h>

using taptools::autowah::wah_filter;
namespace k = taptools::autowah;

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
        u16(3); // IEEE float
        u16(1); // mono
        u32(rate);
        u32(rate * 4);
        u16(4);  // block align
        u16(32); // bits
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

    // Minimal mono WAV reader: accepts float32 and PCM16, takes channel 0 of interleaved files.
    bool read_wav(const std::string& path, std::vector<double>& out, double& sr) {
        std::FILE* f = std::fopen(path.c_str(), "rb");
        if (!f) {
            std::fprintf(stderr, "cannot open %s\n", path.c_str());
            return false;
        }
        auto u16 = [&]() {
            uint16_t v = 0;
            std::fread(&v, 2, 1, f);
            return v;
        };
        auto u32 = [&]() {
            uint32_t v = 0;
            std::fread(&v, 4, 1, f);
            return v;
        };

        char tag[5] = {0};
        std::fread(tag, 1, 4, f);
        u32();
        char wave[5] = {0};
        std::fread(wave, 1, 4, f);
        if (std::strcmp(tag, "RIFF") != 0 || std::strcmp(wave, "WAVE") != 0) {
            std::fprintf(stderr, "%s: not a RIFF/WAVE file\n", path.c_str());
            std::fclose(f);
            return false;
        }

        uint16_t fmt = 0, channels = 0, bits = 0;
        sr = 0.0;
        while (std::fread(tag, 1, 4, f) == 4) {
            const uint32_t size = u32();
            if (std::strcmp(tag, "fmt ") == 0) {
                fmt      = u16();
                channels = u16();
                sr       = static_cast<double>(u32());
                u32();
                u16();
                bits = u16();
                if (size > 16) {
                    std::fseek(f, static_cast<long>(size - 16), SEEK_CUR);
                }
            }
            else if (std::strcmp(tag, "data") == 0) {
                if (channels < 1 || (fmt != 1 && fmt != 3)) {
                    std::fprintf(stderr, "%s: unsupported format (need PCM16 or float32)\n", path.c_str());
                    std::fclose(f);
                    return false;
                }
                const uint32_t bytes_per = bits / 8;
                const uint32_t frames    = size / (bytes_per * channels);
                out.resize(frames);
                for (uint32_t i = 0; i < frames; ++i) {
                    for (uint16_t c = 0; c < channels; ++c) {
                        double v = 0.0;
                        if (fmt == 3 && bits == 32) {
                            float s = 0.0f;
                            std::fread(&s, 4, 1, f);
                            v = s;
                        }
                        else if (fmt == 1 && bits == 16) {
                            int16_t s = 0;
                            std::fread(&s, 2, 1, f);
                            v = s / 32768.0;
                        }
                        else {
                            std::fprintf(stderr, "%s: unsupported bit depth %u\n", path.c_str(), bits);
                            std::fclose(f);
                            return false;
                        }
                        if (c == 0) {
                            out[i] = v;
                        }
                    }
                }
                std::fclose(f);
                return true;
            }
            else {
                std::fseek(f, static_cast<long>(size + (size & 1)), SEEK_CUR);
            }
        }
        std::fclose(f);
        std::fprintf(stderr, "%s: no data chunk\n", path.c_str());
        return false;
    }

    // Deterministic white noise in [-1, 1].
    struct lcg {
        uint32_t state{33333};
        double   operator()() {
            state = state * 1664525u + 1013904223u;
            return (state / 2147483648.0) - 1.0;
        }
    };

    // Funk-chop stimulus: short plucky noise bursts with a sharp attack and exponential decay —
    // enough dynamics to exercise the follower the way staccato playing does.
    std::vector<double> funk_chops(double total_s, double rate_hz = 4.0, double amp = 0.8) {
        std::vector<double> sig(static_cast<size_t>(total_s * k_g_sr), 0.0);
        lcg                 rng;
        const size_t        period = static_cast<size_t>(k_g_sr / rate_hz);
        for (size_t i = 0; i < sig.size(); ++i) {
            const size_t ph = i % period;
            if (ph < period / 3) { // gated pluck
                const double env = std::exp(-8.0 * static_cast<double>(ph) / (k_g_sr * 0.1));
                sig[i]           = rng() * amp * env;
            }
        }
        return sig;
    }

    std::vector<double> render(wah_filter& w, const std::vector<double>& in) {
        std::vector<double> out(in.size());
        w.process(in.data(), out.data(), in.size());
        return out;
    }

} // namespace

int main(int argc, char** argv) {
    // ---- WAV-in mode ----------------------------------------------------------------------------
    if (argc >= 3 && std::strcmp(argv[1], "--in") == 0) {
        std::vector<double> in;
        double              sr   = k_g_sr;
        const int           slot = (argc >= 4) ? std::atoi(argv[3]) : 0;
        const std::string   dir  = (argc >= 5) ? std::string(argv[4]) + "/" : "";
        if (!read_wav(argv[2], in, sr)) {
            return 1;
        }
        wah_filter w;
        w.prepare(sr);
        w.recall_preset(slot, 0.0);
        std::printf("processing %s (%.0f Hz, %.1f s) through factory slot %d\n", argv[2], sr, in.size() / sr, slot);
        return write_wav(dir + "autowah_processed.wav", render(w, in), sr) ? 0 : 1;
    }

    const std::string dir = (argc > 1) ? std::string(argv[1]) + "/" : "";

    // ---- demo scenarios -------------------------------------------------------------------------
    const auto chops = funk_chops(6.0);

    { // the four factory voicings on the same funk chops
        const char* names[4] = {"guitar", "bass", "swell", "cocked"};
        for (int slot = 0; slot < 4; ++slot) {
            wah_filter w;
            w.prepare(k_g_sr);
            w.recall_preset(slot, 0.0);
            write_wav(dir + "autowah_factory_" + names[slot] + ".wav", render(w, chops), k_g_sr);
        }
    }

    { // slow swell on sustained noise pads
        wah_filter w;
        w.prepare(k_g_sr);
        w.recall_preset(2, 0.0);
        std::vector<double> pad(static_cast<size_t>(8.0 * k_g_sr));
        lcg                 rng;
        for (size_t i = 0; i < pad.size(); ++i) {
            const bool on = (i / static_cast<size_t>(2.0 * k_g_sr)) % 2 == 0;
            pad[i]        = on ? rng() * 0.5 : 0.0;
        }
        write_wav(dir + "autowah_swell_pads.wav", render(w, pad), k_g_sr);
    }

    { // cocked wah: sensitivity off, bias swept manually over 8 s (the pedal's secondary mode)
        wah_filter w;
        w.prepare(k_g_sr);
        w.recall_preset(3, 0.0);
        w.set_smooth_ms(0.0);
        std::vector<double> in(static_cast<size_t>(8.0 * k_g_sr)), out(in.size());
        lcg                 rng;
        for (size_t i = 0; i < in.size(); ++i) {
            const double pos = static_cast<double>(i) / static_cast<double>(in.size());
            w.set_bias(150.0 * std::exp2(4.0 * pos)); // 150 Hz -> 2.4 kHz glide
            out[i] = w.process(rng() * 0.4);
        }
        write_wav(dir + "autowah_cocked_sweep.wav", out, k_g_sr);
    }

    { // sidechain: a steady saw drone wah'd by the funk-chop envelope
        wah_filter w;
        w.prepare(k_g_sr);
        std::vector<double> out(chops.size());
        double              phase = 0.0;
        for (size_t i = 0; i < out.size(); ++i) {
            phase += 110.0 / k_g_sr;
            phase -= std::floor(phase);
            const double drone = (2.0 * phase - 1.0) * 0.4;
            out[i]             = w.process(drone, chops[i]);
        }
        write_wav(dir + "autowah_sidechain_drone.wav", out, k_g_sr);
    }

    { // driven + resonant: the OTA-flavored color stage pushed hard
        wah_filter w;
        w.prepare(k_g_sr);
        w.set_smooth_ms(0.0);
        w.set_resonance(0.85);
        w.set_drive(18.0);
        write_wav(dir + "autowah_driven_funk.wav", render(w, chops), k_g_sr);
    }

    return 0;
}
