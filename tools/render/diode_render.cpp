/// @file
/// @brief      Offline renderer for the tap.diode~ kernel — writes demo WAVs.
/// @details    Exercises diode_ladder.h with no Max involved: the acid staple (resonant sweep
///             over a saw riff), the stock-vs-bent resonance story (the hardware feedback HPF
///             keeps stock resonance shy of oscillation; the extended range sings), an fbhp
///             A/B, and a drive A/B. 48 kHz mono float32 WAVs.
///
///             Usage: diode_render [output-directory]   (default: current directory)
/// @author     Timothy Place
/// @copyright  Copyright 2026 Timothy Place. Distributed under the New BSD License.

#include <cmath>
#include <cstdint>
#include <cstdio>
#include <string>
#include <vector>

#include <taptools/diode_ladder.h>

using tap::tools::diode::diode_filter;

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

    // A bassline-flavored saw riff (naive saw — the filter is the star, and it tames the edge).
    std::vector<double> saw_riff(double seconds, double amp = 0.35) {
        const double        notes[] = {55.0, 55.0, 110.0, 55.0, 65.41, 58.27, 82.41, 55.0};
        std::vector<double> sig(static_cast<size_t>(seconds * k_g_sr), 0.0);
        double              phase = 0.0;
        for (size_t i = 0; i < sig.size(); ++i) {
            const double t    = i / k_g_sr;
            const int    step = static_cast<int>(t * 4.0) % 8; // 16th-ish notes
            const double gate = (std::fmod(t * 4.0, 1.0) < 0.8) ? 1.0 : 0.0;
            phase += notes[step] / k_g_sr;
            phase -= std::floor(phase);
            sig[i] = amp * gate * (2.0 * phase - 1.0);
        }
        return sig;
    }

    std::vector<double> render(diode_filter& f, const std::vector<double>& in) {
        std::vector<double> out(in.size());
        f.process(in.data(), out.data(), in.size());
        return out;
    }

} // namespace

int main(int argc, char** argv) {
    const std::string dir = (argc > 1) ? std::string(argv[1]) + "/" : "";

    // 1. The acid staple: resonant sweep over a saw riff.
    {
        diode_filter f;
        f.prepare(k_g_sr);
        f.set_smooth_ms(0);
        f.set_resonance(0.9);
        f.set_frequency(150.0);
        f.snap();
        f.set_smooth_ms(9000.0);
        f.set_frequency(6000.0); // 9-second opening sweep
        write_wav(dir + "diode_sweep_saw.wav", render(f, saw_riff(10.0)), k_g_sr);
    }

    // 2. Stock vs bent resonance: four bars at the hardware maximum (shy of oscillation, like
    //    a real TB-303), four bars at the extended Devil-Fish-style range (it sings).
    {
        diode_filter f;
        f.prepare(k_g_sr);
        f.set_smooth_ms(0);
        f.set_frequency(900.0);
        auto                in = saw_riff(8.0);
        std::vector<double> out(in.size());
        const size_t        half = in.size() / 2;
        f.set_resonance(1.0);
        f.process(in.data(), out.data(), half);
        f.set_resonance(1.45);
        f.process(in.data() + half, out.data() + half, in.size() - half);
        write_wav(dir + "diode_res_stock_vs_bent.wav", out, k_g_sr);
    }

    // 3. fbhp A/B: the hardware 150 Hz feedback HPF (bass survives high resonance) against
    //    DC-coupled feedback (transistor-ladder-style low-end droop).
    {
        diode_filter f;
        f.prepare(k_g_sr);
        f.set_smooth_ms(0);
        f.set_frequency(400.0);
        f.set_resonance(1.1);
        auto                in = saw_riff(8.0);
        std::vector<double> out(in.size());
        const size_t        half = in.size() / 2;
        f.process(in.data(), out.data(), half);
        f.set_fbhp(0.0);
        f.process(in.data() + half, out.data() + half, in.size() - half);
        write_wav(dir + "diode_fbhp_ab.wav", out, k_g_sr);
    }

    // 4. Drive A/B: four bars clean, four bars +18 dB into the diodes.
    {
        diode_filter f;
        f.prepare(k_g_sr);
        f.set_smooth_ms(0);
        f.set_frequency(1200.0);
        f.set_resonance(0.6);
        auto                in = saw_riff(8.0);
        std::vector<double> out(in.size());
        const size_t        half = in.size() / 2;
        f.process(in.data(), out.data(), half);
        f.set_drive(18.0);
        f.set_gain(-9.0); // rough makeup
        f.process(in.data() + half, out.data() + half, in.size() - half);
        write_wav(dir + "diode_drive_ab.wav", out, k_g_sr);
    }

    // 5. Self-oscillation melody (fbhp 0 unlocks exact tuning): ping it, play the cutoff.
    {
        diode_filter f;
        f.prepare(k_g_sr);
        f.set_smooth_ms(0);
        f.set_fbhp(0.0);
        f.set_resonance(1.05);
        f.set_gain(18.0); // self-oscillation sits well below full scale; bring it up
        const double        melody[] = {220.0, 261.63, 329.63, 440.0, 329.63, 261.63, 196.0, 220.0};
        const size_t        n        = static_cast<size_t>(8.0 * k_g_sr);
        std::vector<double> out(n);
        for (size_t i = 0; i < n; ++i) {
            const double t    = i / k_g_sr;
            const int    step = static_cast<int>(t) % 8;
            const double ping = (std::fmod(t, 1.0) < 1.0 / k_g_sr) ? 0.4 : 0.0; // one ping per note
            out[i]            = f.process(ping, melody[step]);
        }
        write_wav(dir + "diode_selfosc_melody.wav", out, k_g_sr);
    }

    return 0;
}
