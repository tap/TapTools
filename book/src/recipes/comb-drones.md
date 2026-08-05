# Chords with no keyboard

The comb-bank chapter ends on "strings, chords, drones, and gestures; no
guitar required" — this recipe supplies the chords. `tap.5comb~`'s five
voices tune in Hz (`freq1..5`), which means voicings are numbers you can
keep, trade, and morph between; below is a small book of them, plus the
excitation and morph craft that turns a filter bank into an instrument.

The mechanics cited here — ring time on a log map (20 ms–100 s), Hermite
tuning, `warp`'s stretched partials, `phase`'s midpoint pluck — are
measured and explained in [five strings, no guitar](../fivecomb.md) and
[its machine chapter](../machine/comb.md).

## Voicings to keep

Tunings in Hz; MIDI equivalents in parentheses for orientation. The
`notes` message tunes a voicing in one gesture — up to five MIDI note
numbers, fractional allowed, so just-intonation intervals land exactly
(`notes 45 57 64 69.86 76` is the major glow with a true 5/4 third) — and
the Hz attributes remain for exact ratios like the bell plate.

| voicing | freq1..5 | character |
|---|---|---|
| the factory chord | 80 / 120 / 160 / 200 / 102 | the legacy preset: a root-fifth-octave stack with a rub (102 against 80) |
| the open fifth | 55 / 110 / 165 / 220 / 330 (A1, A2, E3, A3, E4) | power-chord drone; nothing to clash with any source |
| the major glow | 110 / 165 / 220 / 275 / 330 (A2, E3, A3, ~C#4, E4) | just-intonation major: 275 is a pure 5/4 third, warmer than 12-TET's 277.2 |
| the dark cluster | 65.4 / 77.8 / 98 / 130.8 / 196 (C2, D#2, G2, C3, G3) | minor with a low rub; film-cue territory |
| the bell plate | 210 / 297 / 420 / 594 / 841 | non-octave (√2 ratios): inharmonic, gong-ish before `warp` even arrives |

Masters make voicings performable: `freq` (0..2) transposes the whole
bank proportionally — chords stay chords under the glide — and `res`/`lp`
scale ring and brightness bank-wide.

## Ringing them

- **Drone:** `res1..5 85`, `lp` toward 5000. Feed it *anything* quiet and
  sustained — pink noise at low level, a field recording, your room tone.
  At `res` ≈ 100 the bank sustains essentially forever; the input stops
  being audio and becomes bowing pressure.
- **Pluck:** `res1..5` around 60–70 and excite with clicks or a sparse
  `tap.808.rim~` (`@model claves`) pattern — every tick strums the chord.
  Drums work; speech works eerily well (the chapter's "resonator chord").
- **Strings, stiffened:** `warp 40` stretches the upper partials sharp —
  piano-ish, then bell-ish — while the compensated main tap keeps the
  *pitch* put. Pair with `lp` near 3000 for the felt-hammer version.
- **The midpoint pluck:** `phase 100` cancels the even harmonics — the
  hollow, clarinet-adjacent voicing of a string plucked exactly at its
  middle. On the bell plate tuning it turns purely ceremonial.

Watch the sum: five ringing combs stack like five strings. Ride `gain`
down as `res` goes up, and `tap.limi~` on the output is cheap insurance
for the `res 100` lifestyle.

## The gesture

The bank's real instrument is the morph engine. Store the major glow in
slot 1 and the dark cluster in slot 2 — then `recall 2 8000` and every
frequency, ring time, and damping glides for eight seconds *through
tunings you never chose*, Hermite interpolation keeping the sweep
continuous instead of zippered. The chapter's advice stands: automate
nothing else. One long morph over a static source is a complete piece of
sound design; grabbing a single fader mid-morph overrides just that
parameter, which is the escape hatch when the in-between territory finds
something worth keeping.

## When to leave the recipe

- **One resonance, surgically placed:** `tap.comb~` is the single unit,
  or `tap.svf~ @type bell` when you want EQ, not a string.
- **You want echoes.** Combs long enough to hear as repeats are delays
  wearing a costume — `tap.delay~`/`tap.multitap~` are the honest tools.
- **You want more than five strings.** The count is fixed; `mc.` wrapping
  the whole bank gives you choirs of banks, at which point you are
  building a sympathetic-string instrument and should budget CPU like it.
