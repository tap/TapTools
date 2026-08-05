# The ostinato machine

Two documented lineages share one patch. The Berlin school — Tangerine
Dream's *Phaedra* (1974) above all — put a Moog modular's step sequencer on
stage and let a filtered ostinato run for twenty minutes while hands moved
the cutoff. Three years later Giorgio Moroder and Donna Summer's "I Feel
Love" built an entire hit from a sequenced Moog modular bassline. The
Recipes part's Moog chapter argued you already own the modular — Max is the
patch panel; this recipe is that argument cashed in: the sequencer pair
driving the three-oscillator voice.

## The scaffold

```text
phasor~ (BPM/240) ──▶ tap.303.seq~ ──pitch──▶ mtof~ ──▶ slide~ ─┬─▶ tap.vco~ ──┐
                                  │                             └─▶ tap.vco~ ──┼─▶ *~ ─▶ tap.ladder~ ─▶ tap.vca~
                                  └──gate──┬─▶ tap.adsr~ (filter) ─▶ *~ amount ─▶ +~ base ──▶ ▲ (cutoff)
                                           └─▶ tap.adsr~ (loudness) ────────────────────────────▶ ▲ (gain)
```

- `tap.303.seq~`'s pitch outlet is a MIDI-note signal; `mtof~` turns it
  into Hz for the oscillators' signal inlets.
- Its gate outlet (1.0 plain, 2.0 accented) drives both `tap.adsr~`
  contours directly — the envelope opens above 0.5.
- **One honest wrinkle:** `tap.vco~`'s frequency *signal* inlet bypasses
  the `smooth` ramp by design ("you are the smoothing") — so sequenced
  pitch steps land as hard steps, and slide flags in the pattern won't
  glide on their own. Put a one-pole slew (Max's `slide~`, or `rampsmooth~`)
  between `mtof~` and the oscillators; the 303's ~60 ms RC is the reference
  feel. Short slew = articulation, long slew = the Berlin swoop.
- The oscillator stack, ladder voicing, and envelope tables come from
  [Three oscillators into a ladder](minimoog.md) — the bass patch is the
  right starting point. One voice instead of three is period-correct for
  the sequenced genre and cheaper; add the stack when the line is the whole
  arrangement.

## The line

The genre's cell is small and the sequencer's phase math does the rest
(one bar per phasor cycle; a `length 8` row divides it into eighths —
polymeter as arithmetic, per [the sequencer chapter](../machine/seq.md)).

The octave bounce, "I Feel Love"-school — `length 8`, every step gated:

```text
step:    1  2  3  4  5  6  7  8
pitch:   33 45 33 45 33 45 33 45
```

```text
pitches 33 45 33 45 33 45 33 45
gates   1 1 1 1 1 1 1 1
```

The Berlin cell — `length 16`, a contour that repeats but doesn't resolve:

```text
pitches 33 33 40 36 33 43 36 40 33 33 40 36 31 43 36 38
gates   1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1
```

Then the two moves that carry twenty minutes:

1. **Transpose, sparsely.** `transpose 0 → 3 → 5 → 0` at phrase boundaries
   is the harmonic language — one message, and the armed pattern semantics
   keep everything on the grid.
2. **Ride the filter, slowly.** The loudness contour stays short and
   percussive; your hand (or a very slow LFO into the `+~ base`) opens the
   ladder over minutes, not bars. The ostinato doesn't change; the light
   on it does.

## Settings that read as the genre

| control | setting |
|---|---|
| loudness `tap.adsr~` | `@attack 2 @decay 180 @sustain -12 @release 120` |
| filter `tap.adsr~` | `@attack 2 @decay 160 @sustain -20 @release 160`, amount 1800 Hz, base 150 Hz |
| ladder | `@mode 0 @resonance 0.3 @drive 6` |
| slew (`slide~`) | short; raise it only for deliberate swoops |
| seq | `@swing 0` — the genre is a grid, and the delay does the humanizing |

Two period tricks worth their lines: pan alternate notes (a `length 8` row
of accents driving `tap.pan~` recreates the famous ping-pong doubling), and
put an eighth-note delay after the voice — the echo, not the sequencer, is
where these records' motion lives. Note `tap.delay~` is a plain
integer-sample delay (no feedback, no interpolation), so use it for the
single slap and patch feedback around it, or reach for Max's delay objects
for modulated regeneration.

Glue: an 808 closed-hat row in 16ths from the drum scaffold, mixed low.
Accents land in this scaffold too: turn up `tap.adsr~`'s `velocity`
sensitivity and the sequencer's 2.0-amplitude accented gates hit the
envelopes harder — the loudness contour for punch, the filter contour for
the quack, or both.

## When to leave the recipe

- **You want the 303's couplings** — slides that bloom, accents that
  squelch. That's the [acid recipe](acid-line.md); this scaffold trades the
  couplings for a filter and envelopes you choose.
- **You want generative movement.** This sequencer is deliberately
  deterministic; probability and ratchets are future emitters, and
  randomness belongs to objects that own a `seed`.
- **The line wants to be a song.** Sixty-four steps is the ceiling; past
  that you're composing, and a piano roll is kinder than sixty-four `step`
  messages.
