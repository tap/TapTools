/// @file
/// @brief      Portable long-loop tape regeneration kernel for tap.discreet~ — no Max/Min
///             dependency.
/// @details    A recreation of the two-tape-machine long-delay system Brian Eno printed as a
///             signal-flow schematic on the back cover of *Discreet Music* (Obscure/EG, 1975) —
///             the same rig Robert Fripp ran for the *No Pussyfooting* loops: input is recorded
///             onto tape by machine A, the tape spools for seconds to machine B, and machine B's
///             playback is both the output and the signal folded back into machine A's record
///             head. The tape-path DSP (fractional read, periodic wow/flutter, in-loop
///             coloration) follows the published tape-echo modeling literature (Arnardottir,
///             Abel, Smith, "A Digital Model of the Echoplex Tape Delay", AES 125, 2008;
///             Valimaki et al.'s tape-echo work).
///
///             The design inversion this kernel exists to state: delay.h keeps its feedback loop
///             stable by capping feedback strictly below 1; here regeneration deliberately
///             reaches 1.0, and stability comes from the tape path itself (tape_loop.h `wear`:
///             darkening lowpass -> bounded soft saturation -> DC blocker). Each pass survives
///             *because* it is degraded — wear is the stabilizer, not an fb cap. With drive
///             engaged the loop output is absolutely bounded at any regeneration in [0, 1]; at
///             drive 0 and regen 1.0 the band below the darkening cutoff sustains indefinitely,
///             cleanly — the Frippertronics contract.
///
///             The performance surface mirrors the rig: `input_level` is the send fader Eno rode
///             (fade the input while the loop sustains and the piece keeps evolving without you),
///             `regen` is the return level into the record head, `loop_seconds` is the tape span
///             between the machines, and wow/flutter are the transport.
///
///             Geometry: prepare(sr, max_loop_seconds) buys the worst case once — the family's
///             biggest single buy (30 s at 48 kHz is ~11.5 MB of double tape); size it to the
///             piece. No later call allocates; setters only retarget ramps and are safe while
///             audio runs. All processing is double-precision, per-sample, mono.
///
///             Honest limits:
///             - `set_loop_seconds` glides as a tape-speed change and audibly bends pitch while
///               moving — by design (moving the read head IS the doppler); use set_smooth_ms to
///               choose how fast the transport re-spools. There is no crossfading "digital" mode.
///             - regen 1.0 sustains forever by design. Bring `regen` down (or darken harder) to
///               end a piece; clear() is the eject button.
///             - The wow/flutter transport is periodic only (see tape_loop.h) — deterministic,
///               reproducible, no stochastic capstan drift.
///             - The read floor is 2.5 samples (Hermite support), and wow excursion is clamped so
///               the read can never cross the record head; extreme wow depths at very short loops
///               flatten against that clamp rather than wrapping.
///             - Fresh input reaches the output only via the tape (the dry path is the input
///               itself, mixed equal-power): you hear a new note un-recirculated once, a loop
///               later it returns worn. No gain staging beyond input_level — that is the
///               caller's job.
/// @author     Timothy Place
// SPDX-License-Identifier: MIT
// Copyright 2026 Timothy Place.

#pragma once

#include <algorithm>
#include <cmath>

#include "tape_loop.h" // tap::tools::tape — reel / wow_flutter / wear / ramp, the shared machinery

namespace tap::tools {
    namespace discreet {

        constexpr double k_min_loop_seconds     = 0.1;    // below this it is a comb, not a loop
        constexpr double k_regen_max            = 1.0;    // deliberately reaches unity: wear is the stabilizer
        constexpr double k_default_loop_seconds = 5.0;    // the order of Eno's machine-to-machine span
        constexpr double k_default_max_seconds  = 30.0;   // default worst-case buy (~11.5 MB @ 48k)
        constexpr double k_default_darken_hz    = 3000.0; // gentle generation loss per pass
        constexpr double k_default_drive        = 0.5;    // mild record-head saturation
        constexpr double k_default_wow_ms       = 1.0;    // ~9 cents peak at 0.8 Hz (see the tests)
        constexpr double k_default_wow_hz       = 0.8;
        constexpr double k_default_flutter_ms   = 0.05;
        constexpr double k_default_flutter_hz   = 8.0;
        constexpr double k_default_mix          = 50.0; // half in the room, half on the tape
        constexpr double k_default_smooth_ms    = 20.0; // anti-zipper ramp for setters

        /// The two-machine loop: record head, seconds of tape, playback head, worn return path.
        class machine {
          public:
            /// Defaults are a tape machine, not a neutral bypass: a 5 s span, gentle wear, the
            /// stock transport breathing. Regen starts at 0 — the loop recirculates when you send.
            machine() {
                m_loop_seconds.snap(k_default_loop_seconds);
                m_darken_hz.snap(k_default_darken_hz);
                m_drive.snap(k_default_drive);
                m_input_level.snap(1.0);
                m_mix.snap(k_default_mix);
                m_transport.set_wow(k_default_wow_ms, k_default_wow_hz);
                m_transport.set_flutter(k_default_flutter_ms, k_default_flutter_hz);
            }

            // -- lifecycle -----------------------------------------------------------------------

            /// (Re)allocate tape for `max_loop_seconds` at `sr`, snap all ramps (a DSP restart is
            /// not a parameter move), and erase the tape. Not real-time-safe.
            void prepare(double sr, double max_loop_seconds = k_default_max_seconds) {
                m_sr = (sr > 0.0) ? sr : 48000.0;
                m_reel.prepare(m_sr, std::max(k_min_loop_seconds, max_loop_seconds));
                m_transport.prepare(m_sr);
                m_wear.prepare(m_sr);
                m_loop_seconds.snap(std::min(m_loop_seconds.target(), static_cast<double>(m_reel.capacity()) / m_sr));
                m_regen.snap(m_regen.target());
                m_darken_hz.snap(m_darken_hz.target());
                m_drive.snap(m_drive.target());
                m_input_level.snap(m_input_level.target());
                m_mix.snap(m_mix.target());
                m_wear.set_cutoff_hz(m_darken_hz.current());
                m_wear.set_drive(m_drive.current());
                clear();
            }

            /// Erase the tape and the wear/transport state; parameters are untouched. The eject
            /// button: regen 1.0 material is gone for good.
            void clear() {
                m_reel.clear();
                m_wear.clear();
                m_transport.clear();
                m_head = 0;
            }

            bool prepared() const { return m_reel.prepared(); }

            // -- parameter targets (click-free; safe while audio runs) ---------------------------

            /// Tape span between the machines, in seconds, clamped to [k_min_loop_seconds, the
            /// prepared max]. Slewed — and the slew IS the tape-speed doppler (see Honest limits).
            void set_loop_seconds(double s) {
                m_loop_seconds.to(std::clamp(s, k_min_loop_seconds, max_loop_seconds()), smooth_samples());
            }

            /// Return level into the record head, clamped to [0, 1]. 1.0 is legal and sustains.
            void set_regen(double r) { m_regen.to(std::clamp(r, 0.0, k_regen_max), smooth_samples()); }

            /// Per-pass darkening corner in Hz (tape_loop.h wear band), slewed.
            void set_darken_hz(double hz) {
                m_darken_hz.to(std::clamp(hz, tape::k_darken_floor_hz, tape::k_darken_ceil_hz), smooth_samples());
            }

            /// Record-head saturation drive, >= 0, slewed. 0 is exactly linear (no wear boundedness
            /// — the loop then relies on darkening alone; see the header banner).
            void set_drive(double d) { m_drive.to(std::max(0.0, d), smooth_samples()); }

            /// The send fader: input level into the record head, linear, slewed. Fading this while
            /// the loop sustains is the Discreet Music performance move.
            void set_input_level(double lin) { m_input_level.to(lin, smooth_samples()); }

            /// Dry/wet mix 0..100, equal-power, slewed. 0 is bitwise dry, 100 bitwise wet.
            void set_mix(double pct) { m_mix.to(std::clamp(pct, 0.0, 100.0), smooth_samples()); }

            /// Wow: depth in ms of tape position, rate in Hz. Instant transport config, not ramped.
            void set_wow(double depth_ms, double rate_hz) { m_transport.set_wow(depth_ms, rate_hz); }

            /// Flutter: the faster, shallower partner. Instant transport config, not ramped.
            void set_flutter(double depth_ms, double rate_hz) { m_transport.set_flutter(depth_ms, rate_hz); }

            /// Anti-zipper ramp time for the setters, in ms. 0 = instant (useful for tests).
            void set_smooth_ms(double ms) { m_smooth_ms = std::max(0.0, ms); }

            // -- introspection -------------------------------------------------------------------

            double loop_seconds() const { return m_loop_seconds.target(); }
            double regen() const { return m_regen.target(); }
            double darken_hz() const { return m_darken_hz.target(); }
            double drive() const { return m_drive.target(); }
            double input_level() const { return m_input_level.target(); }
            double mix() const { return m_mix.target(); }
            double wow_depth_ms() const { return m_transport.wow_depth_ms(); }
            double wow_rate_hz() const { return m_transport.wow_rate_hz(); }
            double flutter_depth_ms() const { return m_transport.flutter_depth_ms(); }
            double flutter_rate_hz() const { return m_transport.flutter_rate_hz(); }
            double smooth_ms() const { return m_smooth_ms; }
            double max_loop_seconds() const { return static_cast<double>(m_reel.capacity()) / m_sr; }
            double samplerate() const { return m_sr; }

            // -- audio ---------------------------------------------------------------------------

            double process(double in) {
                if (!prepared()) {
                    return in;
                }
                const double loop_s = m_loop_seconds.tick();
                const double regen  = m_regen.tick();
                const double darken = m_darken_hz.tick();
                const double drive  = m_drive.tick();
                const double send   = m_input_level.tick();
                const double mix    = m_mix.tick();

                if (darken != m_wear.cutoff_hz()) {
                    m_wear.set_cutoff_hz(darken);
                }
                if (drive != m_wear.drive()) {
                    m_wear.set_drive(drive);
                }

                // Machine B's playback head: loop_s behind the record head, breathed by the
                // transport, clamped so the Hermite support can never cross the record head.
                const double span = loop_s * m_sr - m_transport.tick();
                const double d_samples =
                    std::clamp(span, tape::k_min_frac_delay, static_cast<double>(m_reel.capacity()) - 2.0);
                const double played = m_reel.read_hermite(static_cast<double>(m_head) - d_samples);

                // The return path: playback -> wear (darken, saturate, DC block) -> record head.
                m_reel.write(m_head, send * in + regen * m_wear.process(played));
                if (++m_head >= m_reel.capacity()) { // keep the head in [0, capacity): a long can
                    m_head = 0;                      // overflow in half a day of audio on LLP64
                }

                // Equal-power dry/wet with exact endpoints (delay.h law: 0 bitwise dry, 100 wet).
                if (mix <= 0.0) {
                    return in;
                }
                if (mix >= 100.0) {
                    return played;
                }
                const double theta = mix * 0.01 * (tape::k_pi * 0.5);
                return std::cos(theta) * in + std::sin(theta) * played;
            }

            /// Block form: the trivial loop over the scalar path.
            void process(const double* in, double* out, size_t n) {
                for (size_t i = 0; i < n; ++i) {
                    out[i] = process(in[i]);
                }
            }

          private:
            long smooth_samples() const { return static_cast<long>(m_smooth_ms * 0.001 * m_sr); }

            double            m_sr{48000.0};
            double            m_smooth_ms{k_default_smooth_ms};
            long              m_head{0};
            tape::reel        m_reel;
            tape::wow_flutter m_transport;
            tape::wear        m_wear;
            tape::ramp        m_loop_seconds; // seconds
            tape::ramp        m_regen;        // 0..1
            tape::ramp        m_darken_hz;    // Hz
            tape::ramp        m_drive;        // >= 0
            tape::ramp        m_input_level;  // linear
            tape::ramp        m_mix;          // 0..100
        };

    } // namespace discreet
} // namespace tap::tools
