# The transistor ladder

Some filters are tools; this one is a character actor. The four-stage
transistor ladder — the Moog circuit — colors everything it touches: the
resonance pushes back against the bass, the stages saturate into one another,
and at the top of the resonance range it stops filtering and starts singing.
`tap.ladder~` is a zero-delay-feedback model of that circuit with a tanh
saturator in every stage. This chapter is what each control trades, and what
the measurements say the model actually delivers.

Companion material: the reference page and help patcher in the TapTools-Max
package, and the [verification notebook](https://github.com/tap/TapTools/blob/main/notebooks/ladder.ipynb) —
every number below is an executed measurement. For the *linear* ladder — the
cheap, polite Stilson/Smith model — see `tap.fourpole~`; this object is its
nonlinear sibling.

## What the model gets right

Two things separate a serious ladder model from a filter with a "Moog" label:

- **Tuning that survives the top octaves.** The classic digital shortcut goes
  audibly flat as the cutoff rises. This model is prewarped ZDF: measured
  self-oscillation lands at 1000.2 Hz for a 1 kHz cutoff (0.02 % error) —
  and, the part that's actually hard, **8009 Hz for an 8 kHz cutoff**
  (0.11 %). You can play the resonance like an oscillator anywhere on the
  keyboard.
- **Nonlinearity inside the loop, not bolted on.** Each stage saturates, and
  the feedback fights the saturation the way the hardware does. That is where
  the compression, the "sag," and the bounded self-oscillation come from.

## The knobs, one by one

### `frequency` and the right inlet

Cutoff in Hz; a signal in the right inlet drives it with true per-sample
resolution. Like everything here it rides the `smooth` ramp when set by
message.

### `resonance` — up to and past the edge

0 to 1.1. At 1.0 the loop gain reaches the oscillation threshold; above it
the filter sings at the cutoff, amplitude-limited by the tanh stages (ping it
to start — silence is a fixed point). Under the edge, resonance does the
authentic ladder thing: it **eats your passband** (see `comp`).

### `drive` — how hard to lean on the stages

Input gain (dB) into the saturating ladder. Measured THD on a 100 Hz tone:
0.5 % at 0 dB, 3.5 % at 8, 16.5 % at 16, 33 % at 24 — a smooth walk from
"slightly thick" to "fuzz pedal's cousin." All odd harmonics, because tanh is
symmetric — which is exactly why `asym` exists.

### `asym` — the even harmonics of real hardware

Real transistors don't match; their operating points sit slightly off-center,
and that asymmetry is where a hardware ladder's even-harmonic warmth lives.
`asym` (0..1) models the mismatch. Measured on a driven tone: the 2nd
harmonic sits at −156 dB (numerically absent) at `asym 0` and rises to
−18.6 dB relative to the fundamental at 0.6. One honest warning from the
reference page: an asymmetric saturator can produce slight signal-dependent
DC — follow with `tap.dcblock~` if something downstream cares.

### `comp` — the passband bargain

A real ladder trades passband level for resonance: the feedback subtracts
from the input. Measured at resonance 0.9: the passband sits at −13.2 dB with
`comp 0` (the authentic droop) and at 0.0 dB with `comp 1` (fully restored).
Vintage behavior or modern behavior — your call, continuously.

### `mode` — pole mixing, the Xpander trick

`lp24, lp12, bp12, bp24, hp12, hp24`: mixing the ladder's stage taps yields
whole families of responses from the same four poles (the Oberheim Xpander's
famous trick). Measured small-signal slopes: 23.4 dB/oct for lp24, 11.7 for
lp12. The resonance and saturation behavior carries into every mode — a
resonant bp24 through drive is a very different animal from `tap.svf~`'s
clean bandpass.

### `oversample` — paying for the saturation honestly

The tanh stages generate harmonics past Nyquist that fold back as inharmonic
alias tones. Measured on a hard-driven 5 kHz tone: going from 1× to 4×
oversampling drops the non-harmonic (alias) energy by **13.5 dB**. The
default 2× is the working compromise; use 4× when you drive high notes hard,
1× when you're filtering bass and counting CPU.

### `solver` — fast or exact

The nonlinear loop can be solved with one predictor-corrector pass (`fast`,
the default) or by Newton iteration to convergence (`exact`,
circuit-simulation accuracy). They are audibly identical until drive *and*
resonance are both pushed hard; `exact` is there for when you want to know,
and for renders where CPU is free.

## Recipes

- **The bass patch:** `tap.vco~` saw stack (see the oscillator chapter's Moog
  recipe) → `@mode lp24 @resonance 0.35 @drive 9 @asym 0.45 @comp 0.25`. Keep
  `comp` low; the droop is the vintage glue.
- **The acid line:** `@resonance 0.85 @drive 15`, envelope into the frequency
  inlet, and let the resonance fight the saturation.
- **The kick synthesizer:** `@resonance 1.05`, ping it with a click, and ride
  `frequency` down fast — a self-oscillating ladder is a sine with attitude.

## When it is not the right tool

- **Transparent filtering.** Every pole of this filter has an opinion. For
  surgical work use `tap.svf~` (clean circuit) or `tap.filter~`.
- **Morphing responses.** The pole-mix modes switch; they don't glide.
  Continuous response morphing is `tap.svf~`'s `morph`.
- **CPU-constrained patches that just need "4-pole lowpass."**
  `tap.fourpole~` is the linear ladder at a fraction of the cost — no
  saturation, no oversampling, no opinions.

## Checkpoint

A prewarped ZDF four-stage ladder with tanh in every stage: self-oscillation
in tune within 0.11 % even at 8 kHz, drive that walks THD from 0.5 % to 33 %,
`asym` switching on the even harmonics of mismatched transistors, `comp`
choosing between authentic passband droop and modern flatness, pole-mixed
multimode outputs, and oversampling that measurably pays down the
saturation's aliasing. The character filter — spend your tone budget here.
