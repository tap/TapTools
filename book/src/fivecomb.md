# Five strings, no guitar

Feed a comb filter its own output and it stops being an EQ curiosity and
becomes a *string*: a resonator with a pitch, a ring time, and a temperament.
Five of them, tuned by hand, is an instrument — that was the insight of the
GRM Tools Classic "Comb Filters" plugin, and `tap.5comb~` is its recreation:
five resonant combs with per-voice tuning, masters that play the whole bank,
and the preset-morph engine that made the GRM tools feel alive. This chapter
is how to tune, ring, and morph it.

Companion material: the reference page and help patcher in the TapTools-Max
package, and the `grm_comb_render` tool in the kernel repo, which renders the
listening-check scenarios outside Max.

## What a resonant comb actually is

A delay of 1/f seconds fed back on itself resonates at f and all its
harmonics — pluck it with noise and it rings like a string tuned to f. Two
implementation details decide whether five of them sound like a chorus of
strings or like a broken flanger, and both were the reasons this object was
recreated rather than ported:

- **Fractional delays.** At 48 kHz, a 440 Hz comb needs a delay of 109.09
  samples. Round it to 109 and the comb plays 440.37 Hz — every voice lands
  on a slightly wrong, slightly *different* wrong pitch, and the beating
  between voices (the whole point of a bank) is gone. The delays here are
  Hermite-interpolated: the tuning is continuous, and sweeps glide instead of
  zippering.
- **No clipper in the loop.** The feedback path uses a DC blocker and a
  precise feedback cap, not a hard limiter — high resonance rings clean
  instead of distorting.

![Signal-flow diagram of one comb voice: input sums with feedback into a
fractional delay line, read by Hermite interpolation, with the feedback ring
running through the loop lowpass, normalized DC blocker, warp allpass, and
ring-time-derived feedback gain; a pickup tap at half the loop feeds the
output subtractor](images/comb/block-diagram.svg)

*One voice of five. The red ring is the string; the amber tap is the pluck
position.*

## The knobs, one by one

### `freq1..5` and `freq` — the tuning

Per-voice frequencies (5 Hz floor, the GRM's own) plus a master multiplier
(0..2) that transposes the whole bank — the master is the *performance*
control, gliding every voice proportionally so chords stay chords.

### `res1..5` and `res` — ring time, not feedback

Resonance is mapped to **ring time on a log curve, 20 ms to 100 s**, and the
feedback coefficient is derived from the *current* delay — so a voice keeps
its ring time as its pitch sweeps, instead of ringing longer at low notes and
choking at high ones (the raw-feedback behavior of naive combs, and of the
legacy abstraction). 50 is a decaying pluck; 80+ sustains; near 100 it is a
drone that outlives your patience.

### `lp1..5` and `lp` — the string's brightness

A one-pole lowpass inside each feedback loop: every pass around the loop gets
darker, which is exactly how real strings decay (highs first). Open it for
metallic; close it toward a few hundred Hz for felt and thump.

### `warp` — stiff strings

New to this recreation: a negative-coefficient allpass in the loop disperses
the partials — upper harmonics round-trip faster and stretch **sharp**, the
inharmonicity of a stiff piano string. The main tap is compensated at each
voice's fundamental, so the *pitch* stays put while the timbre goes
piano-ish, then bell-ish. At extreme warp × high tuning the loop can't get
shorter than the dispersion and the pitch flattens — physical, and
documented.

### `phase` — where you pluck the string

Also new: a half-loop pickup tap. At 100 the even harmonics cancel — the
sound of plucking a string exactly at its midpoint. Neutral at 0.

### `gain`, `mix`, and the morph engine

Equal-power dry/wet and output gain, plus the GRM hallmark: **sixteen preset
slots with timed interpolation**. `store 1`, retune everything, `store 2`,
then `recall 1 8000` — every frequency, resonance, and damping glides for
eight seconds through territory you never explicitly tuned. Grabbing one
parameter mid-morph overrides just that parameter. The morph is not a
transition between sounds; it *is* the sound.

## Recipes

- **The resonator chord:** tune `freq1..5` to a voicing (say 80/120/160/200/
  102 Hz — the legacy factory preset), `res` high, and feed it drums or
  speech. The input is now an excitation signal for your chord.
- **The piano that isn't:** moderate resonance, `warp 40`, `lp` around 3 kHz,
  and pluck with clicks.
- **The eight-second gesture:** two stored extremes and a long `recall` — the
  classic GRM move. Automate nothing else.

## When it is not the right tool

- **One comb, precise and plain:** `tap.comb~` is the single, cheaper unit.
- **Echoes rather than pitch:** delays long enough to hear as repeats are
  `tap.delay~` / `tap.multitap~` territory — a comb *is* a delay, but this
  one is tuned and normalized for resonance, not slapback.
- **Faithful nostalgia:** this deliberately is **not** the legacy
  `tap.5comb~` abstraction — the integer delays, linear feedback, and in-loop
  clipper it had are exactly what was retired, and the deviations are flagged
  in the reference page for the audition.

## Checkpoint

Five Hermite-tuned resonant combs with ring time on a log map (20 ms–100 s),
per-loop damping, and two ways to bend the string physics (`warp` for
stiffness, `phase` for pluck position) — under masters that transpose the
bank and a sixteen-slot morph engine that turns retuning into performance.
Strings, chords, drones, and gestures; no guitar required.
