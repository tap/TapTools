# One tape, two read patterns: `scrub.h`

When `stammer.h` shipped, its header made a promise in its own limits
section: slices play at ±1 rate, a performable pitch-bending playhead over
live capture is a different object, and *sharing this capture is the plan*.
This file is that promise being kept, and it is worth recording that the
sharing turned out to be literal. `scrub.h` includes `stammer.h` and uses
`stammer::capture` itself — one `tape_loop.h` reel under an advancing write
head — rather than keeping a second copy of the same idea.

The only thing the stutter had to grow was `capture::read_frac`, a
fractional Hermite read. Its ±1-rate slices never needed one.

The rest of the file is two classes: `head`, the grain scheduler, which owns
the grain pool, the hop clock and the spray dice and reads a capture it does
not own; and `machine`, which is one capture, one head, the freeze gate, the
drift and the balance. Same parts-then-composition habit as `tapecho.h` and
`stammer.h`, for the same reason: `head` is a read pattern, not a machine,
so it is testable and composable without being an external.

## The defect that measurement caught and nothing else could

The first cut anchored every grain at the position. That is the obvious
thing to do — the position is where the user is pointing — and it is wrong
in a way that is genuinely hard to hear.

Here is the mechanism. If every grain's origin is `write_head − lag`, then
origins advance at the *write head's* speed, which is exactly 1. Each grain
then plays from its origin at `rate`. Inside a grain the pitch is correct.
Across grains the average read rate comes back to 1, because the origins
reset it every hop.

So a steady tone comes out **at its original pitch**, with a comb of
grain-rate sidebands around it. The pitch knob did not transpose. It added
texture, and texture is what you expect from a granulator, which is exactly
why no amount of listening was going to find this.

The fix is a phase-continuous read head: the origin advances at `rate`, and
is pulled back toward the position only once it has wandered more than
±1.5 grains. Every pull-back is a splice, which is the cost, and the bound
is chosen by sweep rather than taste. Band energy retained around the
transposed pitch, at wanders of ±0.5 / ±1 / ±2 / ±3 / ±4 grains:

| wander (grains) | mean | worst |
|-----------------|------|-------|
| ±0.5 | 0.933 | 0.716 |
| ±1 | 0.958 | 0.820 |
| ±2 | 0.965 | 0.874 |
| ±3 | 0.990 | 0.918 |
| ±4 | 0.993 | 0.940 |

Flat past 3, and every extra grain of wander is a grain of position error,
so `k_wander_grains = 3.0`.

The unity case is special-cased to zero error rather than accumulated,
which is what keeps the null exact: at `rate == 1` there is nothing to
wander from.

## Measure the band, not the bin

This is the second thing worth carrying out of this file, and it nearly
inverted the conclusion above.

A single-bin probe reads the *fixed* kernel as badly broken. The splices
spread the transposed partial into a comb a few hertz wide; a
rectangular-window Goertzel sitting on one line saw **0.02** where the band
figure was **0.43**. Had that been the first measurement taken, the fix
would have looked like the bug.

Measured properly — energy in a ±15 Hz band around the transposed pitch,
against the same band of a perfect shifter — 98.8 % lands where it should,
worst case 91.7 %. What the splices cost is concentration, not pitch:
92.0 % as focused as a clean shift, 75.0 % at worst.

The general rule, stated for the next time someone here measures a
pitch-shifter: **if the process can smear a partial, a single-bin probe is
measuring the smear, not the partial.** Integrate a band wide enough to
contain the artifact you already know about.

And then, immediately, the same mistake in its other half. The comparison
against `tap.pitchaccum~` used that ±15 Hz band unchanged across the whole
sweep — but ±15 Hz is about 115 cents wide at 220 Hz and only 26 cents at
932 Hz, so at the top of the sweep the probe was again narrower than the
process it was measuring, and it produced two readings of 0.0001 and 0.0006
that were recorded as near-total cancellations of a shipped object. Widened
to a constant 3 %, they read 0.63 and 0.85 and no cancellation exists. The
retraction and what survives it are [issue #33](https://github.com/tap/TapTools/issues/33).

So the rule has a second half: a band wide enough **in the units the process
works in**. A pitch shifter works in cents. A fixed hertz window is a
different width at every pitch, and the place it is narrowest is exactly
where a shifter's error is largest.

Two related mistakes are recorded here because both were committed:

- **Analysing mostly silence.** The first wander sweep ran 1 second of
  material with a 900 ms position lag, so most of the analysed window was
  tape that had not been written yet. Extended to 3 seconds, analysing the
  last third.
- **Feeding a discontinuity into the test.** A slew test drove the object
  with a sine and then, mid-test, called `process(0.5)` with a literal DC
  sample to change a parameter. That step was an input transient, and the
  0.48 jump it produced was the test's own fault. Continuous tone index,
  and the same bug was then fixed pre-emptively in `diffuseur_test.cpp`.

## The null, and the arithmetic that makes it exact

Hann satisfies constant-overlap-add at hop = size/overlap, so the window sum
is exactly 1 at overlap 2 and above, and normalization is `2/overlap` so the
level holds across settings. With pitch at unity, spray at zero and the
position on a whole sample, the object is the input delayed to 4.4e-16.

It is exact only when `size` divides evenly by `overlap`, because the hop is
an integer number of samples; otherwise a small periodic ripple survives in
the window sum. It is inaudible at musical sizes, and it is why the null
test chooses the numbers it does (480 samples of lag, 96 of size) rather
than round milliseconds.

The `mix` control needed the same care as the diffuseurs' — an equal-power
blend written as `cos`/`sin` does not return exactly zero at the endpoint,
and a wiring null that reads 6.1e-17 instead of 0 is not a null. Both ends
are short-circuited exactly.

## The grain pool starves rather than steals

Shrinking `size` sharply while grains are in flight can leave every slot
busy at the moment the next grain is due. That grain is **dropped**, not
allocated by stealing a slot from a grain mid-window, because a steal cuts a
Hann window in half and clicks. The audible cost is a momentary dip, bounded
by the pool being two slots deeper than the maximum overlap.

## A limit that is not fixed, on purpose

A grain born `lag` samples behind the write head and playing at rate `r`
reaches `lag − size·(r−1)` behind it by its end. Transpose up with the
position near the live edge and the grain's tail runs off the front of the
tape into the oldest material.

Nothing clamps this. Clamping would silently bend the pitch to keep the
grain in bounds, which is a worse failure than the seam — the object would
stop playing the interval you asked for and never say so. The constraint is
documented (`keep the position at least size·(rate−1) back`) and left to the
player.

## Checkpoint

One capture, shared literally with the stutter, plus one fractional read
that the stutter did not need. A phase-continuous read head, because
anchoring grains at the position quietly cancels the transposition — the
defect of this file, invisible to listening and obvious to a sweep. A wander
bound measured rather than chosen. And a measurement lesson worth more than
the kernel: a single-bin probe on a smeared partial reads the fix as the
bug.
