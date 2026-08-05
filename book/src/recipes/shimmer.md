# The staircase and the wash

Shimmer has a documented birthplace: Brian Eno and Daniel Lanois in the
early eighties, feeding a pitch shifter and a reverb into each other until
a guitar came out sounding like weather. The pitchaccum chapter tells the
half of the story that lives inside one object — the transposer-delay loop
where every pass climbs again — and its recipes sketch the pairing. This
recipe is the whole patch: the spiral, the wash, and the mix decisions
that keep ten seconds of accumulated fifths from eating a track.

Measured claims are borrowed from [the spiral staircase](../pitchaccum.md)
(the +7-becomes-+14 accumulation, the constant-sum grain envelopes, the
0.99 feedback cap) and [borrowed rooms](../convolve.md).

## The chain

```text
source ──▶ tap.pitchaccum~ ──▶ reverb (tap.verb~ or tap.convolve~) ──▶ return
   └────────────────── dry path ───────────────────────────────────────▶ out
```

Run it as a send: the source stays dry and full-size in the mix, and the
shimmer return comes up underneath it like backlight. On the send,
`tap.pitchaccum~` at `mix 100` (its own dry path stays home) and the
reverb wet-only.

## The spiral

| control | setting |
|---|---|
| `trans1` / `delay1` / `fb1` / `gain1` | +12 st / 400 ms / 75 / 50 |
| `trans2` / `delay2` / `fb2` / `gain2` | +7 st / 650 ms / 60 / 50 |
| `xfade` | 60 — smooth flanks, soft attacks |
| `modfreq` / `moddepth` / `modphase` | 0.3 Hz / 0.1 st / 90° |
| `follow` | off for chords and pads; on for monophonic lines |

The two shadows are doing different jobs: the octave climbs politely
(+12, +24, +36 — always consonant), while the fifth *rotates* the harmony
(+7, +14, +21 — a fifth, then a ninth, then a #11) and is where the
Eno-school mystery comes from. Pull `fb2` down toward 40 when the source
is already harmonically rich; push `fb1` toward 90 for the endless
version — the loop is capped and DC-blocked, so "too long" is an
aesthetic problem, not a stability one. The touch of modulation
(`moddepth 0.1`, with `modphase 90` breathing the shadows against each
other) keeps a long spiral from sounding cloned — depth stays subtle or
the climb turns seasick.

## The wash

Either reverb works; they fail differently:

- **`tap.verb~`** (the designed tail): `@mix 100 @decay 8 @damping 4000
  @lowpass 8000 @delay 60 @modfreq 0.2 @moddepth 0.3`. The damping matters
  more than the length — shimmer's accumulated highs need somewhere soft
  to land, and 4 kHz of loop damping is the difference between glow and
  glass dust.
- **`tap.convolve~`** (the borrowed room): a long church at `@mix 100
  @predelay 20`, and pick the IR by its top end — audition the tail alone
  and reject anything that rings metallic up high, because the spiral
  will find it. (The [field guide to rooms](rooms.md) has the audition
  drill.)

Order matters and is worth an experiment: spiral → reverb (above) washes
the staircase — the classic. Reverb → spiral transposes the *wash itself*
and is wilder and less controllable; the historical chains did both,
depending on the record.

## Variants

- **The descent:** `trans1 -5`, `trans2 -12`, long delays, feedback ~50 —
  the staircase into the basement. Darker damping (2–3 kHz); the low
  accumulation muddies fast, so shorter reverb.
- **The micro-halo:** `trans1 +0.15`, `trans2 -0.15`, delays 60/90 ms,
  feedback ~50, `xfade` wide, modest reverb — no spiral at all, an
  expensive-sounding widener that flatters pads.
- **The gesture:** store the halo in slot 1 and the full +12/+7 spiral in
  slot 2, then `recall 2 8000` as the chorus lands — the morph engine
  glides every parameter, and the *bloom* is the production moment.

## When to leave the recipe

- **The mix is dense.** Shimmer is backlight; on a busy arrangement it
  reads as mud. It earns its keep on sparse sources — one guitar, one
  voice, one held pad.
- **You want rhythmic echoes climbing in pitch.** The delays here serve
  the loop, not the grid; that patch is a tempo-synced delay into
  `tap.shift~`, built by hand.
- **You want the pitch to stay put.** Then it's just reverb — go straight
  to the [field guide](rooms.md).
