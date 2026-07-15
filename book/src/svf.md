# The filter that morphs

Every synthesizer needs one filter it can trust with anything: a bass line, a
noise sweep, a parametric EQ move, an audio-rate modulation stunt. `tap.svf~`
is that filter — a state-variable design in Andy Simper's trapezoidal
(zero-delay-feedback) formulation, the same lineage as the filters in Ableton
Live, including Auto Filter's Morph type. This chapter is what each attribute
trades, and why the design earns the trust.

Companion material: the reference page and help patcher in the TapTools-Max
package, and the [verification notebook](https://github.com/tap/TapTools/blob/main/notebooks/svf.ipynb),
where every number below is an executed, plotted measurement.

## Why "state-variable," and why this one

A state-variable filter computes all its responses — lowpass, bandpass,
highpass, notch — from the same two internal states at once, which is what
makes continuous morphing between them possible at all. The classic digital
version (Chamberlin) famously misbehaves at high cutoffs and under fast
modulation. Simper's TPT formulation fixes both: the tuning is prewarped
(exact all the way to Nyquist) and the filter is **unconditionally stable
under per-sample cutoff modulation** — the property that later let this same
kernel become the sweep engine inside `tap.autowah~`. The notebook slams the
cutoff across five octaves with a 90 Hz LFO under full-band noise; the output
stays bounded, no oversampling tricks required.

## The knobs, one by one

### `type` — the discrete responses, the morph, and the EQ family

Ten responses from one core. The classics — lowpass, highpass, bandpass,
notch, peak, allpass — plus:

- **`morph`**: one continuous parameter sweeps LP → BP → HP → notch → LP
  (0 → 0.25 → 0.5 → 0.75 → 1). The corners are **bit-identical** to the
  discrete modes — measured max difference exactly 0 — so morphing to a corner
  *is* that filter. A slow morph under a held chord is a patch element the
  discrete modes can't give you.
- **`bell`, `lowshelf`, `highshelf`**: the parametric-EQ trio from Simper's
  coefficient tables, with a ±24 dB `gain`. Measured: a +12 dB bell peaks at
  +12.00 dB; a −9 dB low shelf lands −9.00 dB in its plateau and 0.00 dB on
  the other side. These always run a single 2nd-order section — cascading
  would square the boost, so `order` is ignored for them, on purpose.

### `order` — 2, 4, or 8 poles that stay flat

Orders 2/4/8 (12/24/48 dB per octave) run as a cascade with the Butterworth Q
spread, so at resonance 0 the response is maximally flat and sits at −3.01 dB
at the cutoff *regardless of order* — measured −3.01 at every one, with
slopes of 12.3/24.7/49.4 dB per octave. The trade against a naive cascade of
identical sections (which droops long before fc): none. This is just the
correct way to stack poles.

### `resonance` — normalized, and honest about the top

0 to 1: 0 is the Butterworth-flat base, 1 is the edge of self-oscillation.
Resonance sharpens **only the final section** of a cascade, so you get one
clean resonant peak on a flat passband instead of a compounding stack of
peaks. A `q` message converts to and from engineering Q if you think in those
units.

### `circuit` — clean or driven

- **`clean`** is the pure linear filter: cheapest, transparent, never
  oversampled. Also the reference: the EQ modes and every measured Bode plot
  above are this circuit.
- **`driven`** adds `drive` (dB) into a tanh limiter on each section's band
  node — an OTA-flavored color stage, oversampled (1/2/4×, default 2×). Two
  measured consequences: a 200 Hz tone through +18 dB of drive grows odd
  harmonics that simply do not exist in the clean circuit (the 3rd harmonic
  appears out of the numerical floor, ~140 dB up), and at resonance 1.0 the
  filter **self-oscillates at the cutoff** — measured 999.7 Hz for a 1 kHz
  setting, amplitude bounded by the saturator. It needs a ping to start: a
  perfectly silent filter is a fixed point.

### `frequency`, the right inlet, and `smooth`

Float or attribute sets the cutoff through the anti-zipper ramp (`smooth`,
ms). A **signal in the right inlet takes over per sample** — that's the path
for audio-rate filter FM and for envelope-follower patches. Sixteen preset
slots morph via `store`/`recall` over `interp` milliseconds, everything
gliding together.

## Recipes

- **The synth voice:** `@type lowpass @order 4 @resonance 0.4`, envelope into
  the frequency inlet. Order 4 is the "synth filter" slope; order 2 is the
  polite one; order 8 is a wall.
- **The DJ sweep:** `@type morph`, sweep `morph` 0 → 0.5 while easing
  `frequency` — the LP-through-BP-to-HP arc is the whole move in one
  parameter.
- **Tone control:** `bell`/shelves with modest gains. It measures exact, so
  trust the numbers you type.
- **A sine with character:** `@circuit driven @resonance 1`, ping it, and
  tune with `frequency` — a self-oscillating test-tone-with-a-temper.

## When it is not the right tool

- **You want the classic squelchy 4-pole growl.** That's a transistor-ladder
  sound — resonance that compresses the passband, saturation *inside* the
  loop. Next chapter: `tap.ladder~`.
- **You need many static EQ bands.** One `tap.svf~` per band works, but a
  dedicated multiband EQ (or `tap.filter~`, the RBJ multimode biquad) is the
  boring, correct choice.
- **You want the filter to follow your playing.** That's `tap.autowah~`,
  which is this filter plus an envelope detector and a sweep law.

## Checkpoint

One TPT core, every response as an output mix: discrete modes, a morph whose
corners are bit-identical to them, and an exact parametric-EQ trio.
Butterworth-spread orders stay −3.01 dB flat at any slope; resonance sharpens
only the last section; the driven circuit adds tanh color and true bounded
self-oscillation at the cutoff. Unconditionally stable under per-sample
modulation — which is why other objects build on it.
