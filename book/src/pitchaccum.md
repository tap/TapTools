# The spiral staircase

Most pitch shifters are a one-way trip: in, transposed, out. The GRM Tools
"PitchAccum" closed the loop — the transposed signal is delayed and fed **back
into the transposer**, so every pass around the loop shifts it again. Set
+7 semitones and a note becomes a rising spiral: +7, then +14, then +21, each
echo climbing, the whole thing dissolving upward like light on water. That
loop is the effect everyone now calls *shimmer*, years before the name.
`tap.pitchaccum~` is the recreation: two independent transposer-delay loops
("shadows") with the accumulation wired in. This chapter is how to climb.

Companion material: the reference page and help patcher in the TapTools-Max
package, and the `grm_pitchaccum_render` tool in the kernel repo for
listening checks outside Max.

## The loop, and why it doesn't collapse

Each shadow is: granular transposer (±24 semitones) → delay (up to 3 s) →
feedback → back into the transposer. Three design choices keep the spiral
musical instead of muddy:

- **Constant-level grains.** The transposer sweeps two taps half a cycle
  apart, each windowed so the pair sums *exactly* to 1 at every phase and
  every crossfade width. The original tt_shift engine's window pair didn't
  quite sum flat, which imposed an amplitude ripple at the grain rate — after
  ten trips around a feedback loop, ripple compounds into tremolo. Here the
  tenth pass is as steady as the first.
- **Hermite-interpolated taps.** Fractional delays keep each pass in tune, so
  the spiral's steps are the interval you set, not the interval plus drift.
- **A capped, DC-blocked loop.** Feedback tops out at 0.99 with a DC blocker
  in the path — the spiral can run for a very long time, but it is
  unconditionally bounded (unit-tested at the cap).

The signature is measurable: set +7 semitones and the kernel test finds
energy at +7 **and +14** — the second pass, the accumulation itself.

![Two signal-flow diagrams contrasted: tap.pitchaccum~ feeds its output back
into the delay buffer upstream of the moving transposer taps, so every pass
is transposed again; the ordinary shifter-in-a-feedback-loop patch taps the
delay output and only ever shifts once](images/pitchaccum/block-diagram.svg)

*The topology is the effect. Feedback re-enters upstream of the taps, so the
staircase climbs; in the ordinary patch every echo is the same interval.*

## The knobs, one by one (per shadow, ×2)

### `pitch1` / `pitch2` — the step of the staircase

±24 semitones, continuous. Musical intervals (+7, +12, +5) make harmony;
small offsets (±0.1–0.3 st) make lush detune-echo instead of a spiral;
negative values descend into the dark version nobody expects.

### `delay1` / `delay2` — the tread depth

Up to 3 s per shadow. Short (50–150 ms) blurs the passes into a texture;
long (0.5–2 s) articulates each step of the climb as an audible echo.

### `feedback1` / `feedback2` — how many steps

How much survives each trip. 0.3 gives two or three audible generations; 0.7
a long climb; 0.9+ a texture that essentially sustains until the transposition
walks it out of range (energy shifted past the audible band is the spiral's
natural exit).

### `xfade` — the grain crossfade

GRM's Cross-fade control: the width of the grain envelope's flanks. Narrow is
more articulate and more grain-rate flavored; wide is smoother and softer in
attack. Because the envelope pair always sums to 1, this changes *texture*,
never level.

### The modulation section, and `follow`

A global LFO (with `modphase` offsetting shadow 2, so the two loops breathe
against each other) plus per-voice deterministic random transposition
modulation — a little of either keeps long spirals from sounding cloned.
`follow` (off by default) engages a pitch follower — decimated normalized
autocorrelation, confidence-gated, deliberately picking the *smallest*
plausible lag so it doesn't lock onto subharmonics — which adapts the grain
window toward the detected period: cleaner transposition on monophonic
sources, ignored gracefully on noise.

### Presets

The sixteen-slot morph engine, as everywhere in the GRM pair: two stored
spirals and a timed `recall` between them is a gesture in itself.

## Recipes

- **Shimmer, the classic:** shadow 1 at +12, delay ~400 ms, feedback 0.75;
  shadow 2 at +7, delay ~650 ms, feedback 0.6; both into a reverb
  (`tap.convolve~` with a long church, or `tap.verb~`). The reverb is
  load-bearing — shimmer is spiral *plus* wash.
- **The descent:** −5 and −12, long delays, moderate feedback — a staircase
  into the basement, much rarer and much creepier.
- **Micro-thickener:** ±0.15 st, 60/90 ms delays, feedback 0.5, `xfade`
  wide — not a spiral at all, just an expensive-sounding widener.

## When it is not the right tool

- **One clean transposition, no loop:** `tap.shift~` is the plain shifter —
  same modernized engine, none of the plumbing.
- **Formant-true vocal shifting:** granular transposition shifts formants
  with the pitch; chipmunks live this way. A vocoder-based resynthesis
  (`tap.vocoder~` has other talents) or a dedicated formant tool is the
  answer there.
- **Rhythmically exact multi-tap echoes:** the delays here serve the loop;
  `tap.multitap~` serves the grid.

## Checkpoint

Two transposer-delay loops where the feedback re-enters the transposer, so
pitch accumulates pass after pass — +7 becomes +14 becomes +21. Constant-sum
grain envelopes keep the tenth pass as steady as the first; Hermite taps keep
it in tune; the capped, DC-blocked loop keeps it bounded forever. Intervals
are the architecture, delay is the pacing, feedback is the height — and the
morph engine turns the whole staircase into something you can bend mid-climb.
