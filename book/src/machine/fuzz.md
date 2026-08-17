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

## Mistake two: the house oversampler, and a hypothesis that was right

The oversampling chain in `tap.ladder~`, `tap.svf~` and `overdrive.h` is
zero-stuff plus a 4th-order Butterworth, cut at 0.45 of the base rate
normalized to the oversampled rate. This file started as a copy of it.

Measured, that was wrong here: alias energy at the fold frequencies came out
**worse at 4× than at 2×** (1.7e-2 against 2.8e-3). Twenty-four dB per octave
leaves content just above the base Nyquist barely attenuated, and a higher
factor pushes more clipper-generated content into exactly that band before
decimation. Moving to 8th order improved 4× about sixfold.

It did not fix the ordering. An earlier draft of this file recorded that as an
open question with one hypothesis ruled out and one surviving:

- **Ruled out: numerics.** At 8× the filters are cut at 0.056 normalized,
  where biquad poles crowd the unit circle. Tested by running the cascade's
  impulse response out to 400,000 samples at each factor; it decays cleanly to
  denormal every time.
- **Surviving: imaging.** Zero-stuffing by N leaves N−1 images for a single
  filter to suppress; residual images entering a *nonlinearity* intermodulate
  with the signal into products that are not harmonics of the input, which is
  precisely what the probe measures, and there are more of them at higher N.

`ondes.h` then supplied evidence for the survivor without being built to:
same 8th-order chain, comparably hard nonlinearity, but a **source** with
nothing zero-stuffed on the way up, and its sequence never reversed.

**Acting on it settled it.** The chain is now one 2× stage per doubling, each
filtering at 0.225 of its own operating rate — a corner that never tightens
however deep the cascade goes, which is the whole difference. Same probe, same
material, only the resampler changed:

| tone | old 4× | old 8× | new 4× | new 8× |
|---|---|---|---|---|
| 3733 Hz | 7.4e-4 | 1.8e-3 | 2.1e-5 | 2.2e-5 |
| 4409 Hz | 7.9e-4 | 2.1e-3 | 3.7e-7 | 3.7e-7 |
| 5171 Hz | 9.2e-4 | 2.3e-3 | 2.0e-7 | 1.9e-7 |
| 6421 Hz | 5.8e-4 | 1.3e-3 | 3.9e-7 | 3.6e-7 |
| 9337 Hz | 2.9e-4 | 6.2e-5 | 1.8e-6 | 2.4e-8 |

The worst step-up past 2× is a ratio of 1.017 — flat, where the old chain ran
up to 3× worse per doubling. The 2× column is unchanged in both, as it must
be: one doubling is one stage either way, and that it *is* unchanged is the
best available check that nothing else moved.

The cost is 3.16 % of a core at 8× against 3.02 % before. The filters are
cheap next to the clipper they surround, which is worth knowing in advance
next time this trade looks expensive.

## Mistake three: one tone is not a sweep

Every number in the two sections above — the 4th-order finding, the reversal,
the "2× is best" default that shipped — came from **a single test tone at
3733 Hz**. The tone was chosen carefully, for good reasons that are still
good: it does not divide the sample rate, and its folds land where nothing
else lives. It was still one tone.

Swept properly, 2× does not merely fail to be best. It collapses above about
6 kHz, and at 10499 Hz it measures *worse than no oversampling at all*
(1.7e-1 against 1.5e-1), because the clipper's low harmonics already exceed
the base Nyquist there. The default that shipped was safe only for material
that stays below 6 kHz.

This is the same error as the two in the next section, one level up: those are
about choosing a bad probe, this is about choosing too few. A probe that is
correct at one point on the input domain tells you about that point. The fix
is not cleverness, it is a `for` loop over tones, and it costs seconds.

Two properties of this particular probe bound where the loop can go, and both
are now written down next to it: it only measures folding at all above about
3 kHz, since below that harmonics 8–13 are still under Nyquist and it reads
real harmonics instead; and tones that are simple rational multiples of the
sample rate stack folds on top of each other or put one exactly at Nyquist,
where it reads nonsense.

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
and a cascade of 2× stages because a single zero-stuff by N was what made
bigger measure worse — the imaging hypothesis, recorded as open here for two
waves, then confirmed by acting on it. And a default of 4× rather than 2×,
because the 2× default had been generalized from one test tone and collapses
above 6 kHz.
