# The pedal that listens

A wah pedal is a filter with a foot attached. An *auto*-wah cuts out the
foot: it listens to how hard you play and sweeps the filter for you — hit a
string and the filter opens; let it ring and the filter settles back down.
`tap.autowah~` models a specific, beloved instance of the idea: the **Mad
Professor Snow White AutoWah**, Björn Juhl's OTA-based envelope filter,
grounded in the traced circuit and the published behavior. This chapter is
how to drive it — and how we will know, measurably, when the model matches
the pedal.

Companion material: the reference page and help patcher in the TapTools-Max
package; the design document (`plans/tap.autowah~.md` in TapTools-Max) with
the full hardware research; and the
[validation notebook](https://github.com/tap/TapTools/blob/main/notebooks/autowah_validation.ipynb),
which measures everything below and ends with a cell waiting for recordings
of the real pedal.

## What the hardware is, in one paragraph

A 2-pole state-variable filter (an LM13700 OTA circuit) whose frequency is
pushed **up** from a resting point by an envelope detector — a diode and a
capacitor, charged fast, discharged at a rate you set. Four knobs:
Sensitivity (how hard your signal drives the sweep), Decay (how fast it falls
back), Bias (the resting frequency), Resonance (the Q). Published sweep:
250 Hz to about 2.5 kHz — a throaty, vocal range, deliberately unlike the
quack of a Mu-Tron-style filter. One secret feature: with Sensitivity at
minimum it becomes a fixed, manually swept filter — the "cocked wah."

The model composes `tap.svf~`'s Simper core (one 2nd-order section, driven
per sample — the modulation stability that filter chapter promised, cashed
in) behind a rectifier → attack/release follower and an exponential sweep
law. The measured control behavior:

- **Sweep law:** cutoff = bias · 2^(sweep · range). Measured against the
  design curve across the full envelope range: max error **0.000 cents**. The
  law lives in one function on purpose — if the real pedal turns out to sweep
  linearly in Hz, one function changes and nothing else moves.
- **Timing:** attack set to 2 ms measures 1.94 ms; decay set to 250 ms
  measures 256 ms, and the release fits a pure exponential with residual
  σ = 0.004 — an RC discharge, like the hardware.

## The knobs, one by one

### `sensitivity` — the trigger level, and the secret mode

Detector input gain in dB (−60..+24). Tune it to your instrument and touch:
too low and only your hardest hits open the filter; too high and everything
pins at the ceiling (a tanh soft knee compresses hard playing into the top
rather than slamming a rail). **At −60 the envelope is exactly off** and the
object becomes the cocked wah: a fixed resonant filter with `bias` as the
manual sweep control. Factory preset 4 ships that voicing.

### `decay` — the personality knob

How fast the filter falls back to `bias`, in ms (10..5000). Fast (tens of
ms) gives a wah articulation on **every note** — the funk setting. Slow
(hundreds of ms up) gives classic auto-wah swells that ride your phrasing.
This is the knob to perform.

### `bias` and `range` — where the sweep lives

`bias` is the resting frequency (default 250 Hz, the hardware's home);
`range` is the sweep span in octaves above it (default 3.3, the hardware's
250 → ~2500 Hz). Both go far beyond the hardware if you want them to, and
`direction 1` sweeps *down* from bias instead — a TapTools extension the
pedal never had.

### `resonance`, `mode`, `drive`

`resonance` (0..1) is the Q — the vocalness. `mode` picks the filter tap:
lowpass is the stock voicing; bandpass is the circuit's other node, a known
hardware mod — quackier and noticeably quieter. `drive` (dB) engages the
saturating SVF circuit for OTA-flavored color; 0 keeps it pure.

### `attack`, `mix`, and the rectifier

The hardware's attack is fast and fixed; ours defaults to the same 2 ms but
is exposed (0.05..100 ms) for softer onsets. `mix` is an equal-power dry/wet
the pedal never had — 100 % (wet-only) *is* the hardware. And under the hood
the detector's rectifier is selectable: full-wave (default, cleaner
tracking) or half-wave (the traced single-diode topology). Measured: the
half-wave detector carries 2.6 % signal-rate ripple on a low tone against
the full-wave's 0.7 % — a real, quantified flavor difference awaiting the
hardware A/B.

### The sidechain inlet and the envelope outlet

A signal in the right inlet takes over the detector: one sound wahs another
(a kick opening a pad is the classic). The right outlet emits the envelope
(0..1) as a signal — the detector as a free modulation source for anything
else in the patch.

### Factory voicings

Preset slots 1–4 ship **guitar**, **bass** (lower bias, tighter range — the
GB pedal's instrument switch, as a preset you can morph to), **slow swell**,
and **cocked wah**. `recall 2 4000` morphing from guitar to bass over four
seconds is its own effect.

## How we'll know it matches

The validation harness is built and proven on ground truth. An STFT
peak-trajectory extractor recovers the swept resonant peak from **wet audio
alone** — no dry reference needed — and against the kernel's own cutoff trace
it correlates at 0.979 in log-frequency, with the small measured offset
(−37 cents) close to what resonant-peak physics predicts (−20 cents at that
Q). The same code runs on demo videos and on the real pedal. A Snow White is
on order; when it arrives, reamped recordings drop into
`notebooks/reference/` and the notebook's last cell overlays hardware against
model. Disagreements map one-to-one onto kernel constants. That pass may flip
the default filter tap or the sweep law — both are flagged, isolated, and
waiting.

## When it is not the right tool

- **You want the filter on a knob or LFO, not your dynamics.** That's
  `tap.svf~` with a signal in its frequency inlet — this object's own core,
  without the detector.
- **Your source has no dynamics.** A static pad through an auto-wah is a
  static filter. Feed the sidechain something rhythmic instead.
- **You want the Mu-Tron quack.** Different circuit, different voicing —
  raise `resonance`, try `mode 1`, but know you're modding a Snow White, not
  summoning a Mu-Tron.

## Checkpoint

An envelope detector with a fast attack and a musician's decay knob, driving
a 2-pole resonant filter up from `bias` through an exponential law that
measures exact to the design. Sensitivity at the floor is the cocked wah;
the sidechain inlet and envelope outlet make the detector patchable; the
factory slots hold the four voicings that matter. And the model doesn't ask
to be trusted — the extractor that will judge it against the real pedal is
already built, already proven, and already waiting in the notebook.
