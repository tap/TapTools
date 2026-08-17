# The instrument in the corner of the room

The Ondes Martenot is not a synthesizer, and the fastest way to make it
sound like one is to patch it like one. This recipe assembles the four
objects that make up the actual instrument — voice, key, and a loudspeaker
with a body — and then spends most of its length on the part that is not
a setting at all: what your two hands do.

Everything measured here is borrowed from [the instrument that is not a
synthesizer](../ondes.md) and [loudspeakers you can
play](../diffuseurs.md), which in turn cite the circuit paper (Najnudel,
Hélie, Roze & Boutin, IEEE/ACM TASLP 28, 2020) and the intensity-key
measurement (Quartier et al., *Acta Acustica* 101(2), 2015).

## The chain

```text
[ribbon signal] ──▶ tap.ondes~ ──▶ tap.palme~ ──▶ out
[key signal]    ──▶     ▲              (or tap.metallique~)
```

Two signals in, one instrument out. That is the whole rig, and the
temptation to put things between the stages should be resisted until you
have played it as it stands — the voice and the diffuseur were designed to
be adjacent, and every stage you insert is a stage the real instrument does
not have.

## The voice

| control | setting | why |
|---|---|---|
| `ribbon` | driven by signal | semitones above A1, not Hz |
| `key` | driven by signal | 0–1 of the physical travel |
| `depth` | `1.` | equal oscillators; the full harmonic series |
| `detect` | `0.2` | the published R4 × C21, 200 µs |
| `drive` | `1.` | nominal; the harmonics are already there |
| `keyplacement` | `0` | pressure is level |
| `polarity` | `1` | |
| `power` | `0` | the 2A3 moves total harmonics by 0.003 |
| `oversample` | `4` | |
| `smooth` | `0.` | the signal inlets are not ramped anyway |

Start there and change exactly one thing at a time, because most of these
are citations rather than tastes and the object will tell you when you have
left the instrument behind.

The two that are genuinely yours: `keyplacement 1` moves the key in front
of the valves, so hard presses get dirty as well as loud — worth about 0.09
of total harmonic content at a half-press, and it is the single change that
most makes the object feel like a synthesizer rather than an ondes.
`polarity -1` flips which side of the waveform the preamplifier bends,
worth about 0.12. Try both; keep whichever suits the piece.

`depth` below 1 is the cheapest real timbre move in the object. At `0.4`
the envelope never closes and the tone thins toward a sinusoid — the
closest thing here to a "register", and it is a physical mismatch between
two oscillators rather than an invented control.

## The hands

This is the recipe.

**The ribbon is linear in semitones**, because the circuit paper's Eq. 7
makes it so. That single fact is why an ondes glissando sounds like an
ondes glissando: a hand moving at constant speed produces a constant-rate
glide, not the accelerating swoop a linear-in-Hz control gives you. So
drive `ribbon` with something that moves *linearly in semitones over time*:

```text
line~ 0. 36. 4000   ──▶ tap.ondes~ left inlet
```

Three octaves in four seconds, and it will sound even the whole way. Build
your phrases the same way — `line~` or a slow `sig~` ramp per note, never a
quantized step unless you specifically want the instrument to sound wrong.
Nothing here rounds to a semitone, and that is deliberate.

**The key is the dynamics, and it starts silent.** Roughly the bottom 45 %
of the travel makes no sound at all. That is the key's own first phase, the
elastic strip bending before it reaches the powder bag, and it is why the
instrument attacks so sharply: the whole 50 dB lives in the 4.5 mm right
after the silence. Practically:

- Drive `key` from a pedal, a fader, or a `line~` — anything continuous.
- Expect nothing below about `0.45`. If you want the note to speak the
  instant your controller leaves zero, rescale: `scale 0. 1. 0.45 1.`.
  Do that only if you want to give up the attack, because that dead travel
  is what lets you place an entrance to the millisecond.
- The curve steepens through the middle and flattens at the top. Crescendos
  therefore want a *decelerating* controller move, not a linear one — which
  is exactly the feedback a player's finger gets from the real spring.

**Play them together.** The ribbon without the key is a test tone; the key
without the ribbon is a volume pedal. The instrument is the two hands, and
`ondes_ribbon.wav` in the render set exists to demonstrate what that
sounds like when both are moving.

## The loudspeaker, which is an instrument too

`tap.palme~` is the default answer. Twelve strings on a board, and they
sustain what the voice has already stopped playing:

| control | setting |
|---|---|
| `root` | `110.` — put the lowest string under your part's key |
| `tuning` | `0` chromatic, so the board answers every note |
| `decay` | `6.` |
| `damping` | `4000.` |
| `detune` | `4.` — cents of scatter, so the board is not a chorus unit |
| `drive` / `asymmetry` / `saturation` | `1.` / `0.3` / `0.2` |
| `mix` | `60` |

`tuning 1` puts the harmonic series on the root instead: the board then
answers only what belongs to that key, which turns a chromatic line into
something that blooms on some notes and stays dry on others. Use it when
the piece really is in one key, and hear it as a compositional decision
rather than a preset.

Watch the level. Twelve resonant loops add up, and a driven board can be a
great deal louder than what went into it — `level` is there for that, and
it is the one control on these objects you will need to touch first.

`tap.metallique~` is the other cabinet: eight plate modes rather than
twelve strings, so it colours instead of harmonizing.
`@pitch 180 @decay 6 @tilt 0.8 @brightness 1. @mix 50` is the gong. Push
`drive` to 3 with `asymmetry 0.5` and the distortion happens *before* the
plate, because that is where the transducer is — a distorted waveform
ringing a gong, not a distorted gong. It is not subtle and it is the most
distinctive sound in the family.

## What each ingredient buys, in order

1. **`tap.ondes~` with both hands moving.** Everything else is optional.
   A static ribbon and a static key is a demo, not an instrument.
2. **The diffuseur.** The voice alone is thin on purpose — it is a valve
   preamplifier output, and it was never meant to be heard without a body
   after it.
3. **`depth`.** The one timbre control that costs nothing and is physical.
4. **`keyplacement` / `polarity`.** Real, measured, and yours to choose.
5. **`power`.** Measured at 0.003 of total harmonic content. Last.

## When to leave the recipe

- **You want the waveform registers.** The real instrument has switchable
  timbres — creux, gambe, nasillard and the rest. They are not here, and
  they are not here on purpose: no source obtained describes their filter
  shapes, and inventing them is the one thing these objects will not do.
  If you need those colours, put a filter after `tap.ondes~` and call it
  your filter, not Martenot's.
- **You want polyphony.** The instrument is monophonic and so is this. Two
  `tap.ondes~` in parallel is a duet, not a chord — which is how ondes
  ensembles actually worked, so it is not a bad answer.
- **You want the diffuseurs on something else.** Take them. A guitar into
  `tap.palme~` is the best argument for shipping them standalone, and
  nothing about them needs the voice in front.
- **You want `tap.triode~` on its own.** It is a published valve stage and
  it works on anything: `@tube 2 @stage 2 @drive 6` is the 2A3 power stage
  used for something it was never in this instrument for.
