# Two stages and a knee: `fuzz.h`

This appendix is mostly about two mistakes, because the DSP itself is a
published recipe followed closely and there is little to explain about it
that Yeh, Abel and Smith's DAFx-07 paper does not explain better. What is
worth recording is what went wrong on the way, since both failures are the
kind that recur.

## The recipe, briefly

Two `stage` objects, each a conditioning highpass, a gain, a memoryless
curve, and an equalization lowpass — the paper's cascade, twice. A `tone`
section of three RBJ biquads outside the nonlinearity. A `pedal` that runs
the pair inside an oversampled region, DC-blocks, voices, and trims.

The curve is `shape(x, k) = tanh(kx)/tanh(k)`, chosen from the family the
paper itself compares against a tabulated diode DC curve (tanh, arctan, a
tanh approximation). The normalization matters more than the choice: dividing
by `tanh(k)` fixes the output at full scale for unit input at every knee, so
`edge` changes the shape of the corner without moving the ceiling.

## Mistake one: small-signal gain compounds

The tanh family's slope at the origin is `k/tanh(k)`. It is greater than one
and it grows with the knee — 1.7 at knee 1.6, 2.1 at knee 2, 12 at knee 12.

In one stage that is a curiosity you can absorb into the gain mapping. In a
cascade it multiplies. The first implementation put a fixed ×2.2 in front of
a knee-3 curve, so the second stage's effective small-signal gain was about
6.6, and with the drive floor at +6 dB the limiter was *already saturated
with the gain knob at zero*. The measured harmonic-to-fundamental ratio was
0.401 at gain 0 and 0.408 at gain 1: the knob did essentially nothing.

The reason this is worth a paragraph is that it is inaudible as a bug. The
object sounded like a distortion pedal at every setting, because it *was* one
at every setting. Only a swept measurement showed the knob was inert. The fix
was to lower the drive floor below unity (−12 dB) and the second stage's
fixed gain to 0.5; the ratio now runs 0.010 → 0.358.

The general lesson, stated for the next cascade someone builds here: **a
waveshaper's small-signal slope is part of the gain structure**, and if the
curve family's slope depends on a user-facing parameter, that dependence
propagates to every stage downstream of it.

## Mistake two: the house oversampler, and a hypothesis that died

The oversampling chain in `tap.ladder~`, `tap.svf~` and `overdrive.h` is
zero-stuff plus a 4th-order Butterworth, cut at 0.45 of the base rate
normalized to the oversampled rate. This file started as a copy of it.

Measured, that was wrong here: alias energy at the fold frequencies came out
**worse at 4× than at 2×** (1.7e-2 against 2.8e-3). Twenty-four dB per octave
leaves content just above the base Nyquist barely attenuated, and a higher
factor pushes more clipper-generated content into exactly that band before
decimation. Moving to 8th order improved 4× about sixfold.

It did not fix the ordering, and this is where the appendix has to be careful,
because an earlier draft of this file claimed it did. Measured against a
3733 Hz tone:

| factor | fold energy | vs. 1× |
|--------|-------------|--------|
| 1× | 1.2e-1 | — |
| 2× | 2.7e-5 | 4618× better |
| 4× | 7.4e-4 | 166× better |
| 8× | 1.8e-3 | 69× better |

Every factor is worth having. 2× is the best of them, so 2× is the default.

**The cause is not established, and one hypothesis is dead.** The obvious
suspect was numerical: at 8× the filters are cut at 0.056 normalized, where
biquad poles crowd the unit circle and direct-form sections are known to
misbehave. That was tested — the cascade's impulse response was run out to
400,000 samples at each factor — and it decays cleanly to denormal every
time. Not conditioning.

The surviving hypothesis, untested, is imaging. Zero-stuffing by N leaves
N−1 images for a single filter to suppress; residual images entering a
*nonlinearity* intermodulate with the signal into products that are not
harmonics of the input, which is precisely what the probe measures, and there
are more of them at higher N. If that is right, the fix is the standard one:
cascaded 2× (halfband/polyphase) resampling rather than one stage at 1/N, so
each step suppresses a single image at a comfortable normalized frequency.
That is the known next move on this file.

Whether `overdrive.h` is owed the 8th-order change is a separate question.
Different nonlinearity, different gain structure, different spectrum — it
needs its own measurement, not this one's conclusion.

## Two ways to measure aliasing wrong

Both were committed before being caught, and both are recorded in
`fuzz_test.cpp` because they are easy to repeat.

**Choosing a tone that divides the sample rate.** The first alias test used
3 kHz at 48 kHz. Every harmonic of 3 kHz folds back onto another harmonic of
3 kHz, so every alias hides exactly underneath legitimate content and the
probes read nothing at all. The test passed happily while measuring noise.
3733 Hz puts the folds where nothing else lives.

**Probing too close to the fundamental.** Two of the original probe
frequencies sat a few hundred Hz from a full-scale tone. What they measured
was the window's spectral leakage — around 1e-3, which swamped the aliasing
underneath it. Probes have to be far enough out that leakage from the loudest
component is below the thing being measured.

## Checkpoint

A published cascade, followed closely. One curve whose normalization keeps
the knee from becoming a volume control. A gain floor set below unity because
small-signal slope compounds across stages — the bug that sounded fine. An
8th-order oversampling filter because the house 4th-order one measured worse,
and a default of 2× because bigger measured worse still, with the cause
recorded as open and one hypothesis explicitly ruled out rather than left
hanging.
