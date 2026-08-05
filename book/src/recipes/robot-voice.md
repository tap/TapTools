# The robot on the radio

The vocoder chapter closes on "the casting is everything," and this recipe
is the casting call. The sound has a documented pedigree — Bell Labs
speech-compression research became, in musicians' hands, Kraftwerk's robot
choirs and ELO's talking skies, and the machine has never left the radio
since — but the records disagree on gear and agree on craft: a bright,
busy carrier, an articulate modulator, and somebody enunciating like they
mean it. All three are patching decisions.

Everything structural below is pinned by the kernel's tests and explained
in [the vocoder chapter](../vocoder.md): 24 bands, 50 Hz–12 kHz, everything
you hear is carrier.

## The carrier, built properly

The eternal failure is a dull carrier — high bands with nothing in them,
consonants gone. Build it in three layers:

```text
tap.vco~ (saw, f)        ──┐
tap.vco~ (saw, f, +7 c)  ──┼─▶ +~ ──▶ tap.vocoder~ right inlet
tap.noise~ (white) ─ *~ 0.1┘
```

- **Two saws, a few cents apart** (`@shape 2`, `detune` ±4–7): harmonics
  to the top of the range, and the beating keeps long vowels alive. Use
  the [Moog recipe's](minimoog.md) stack values; skip the octave-down
  voice — vocoded speech reads clearest with the energy above the
  fundamental.
- **A tenth of white noise** (`tap.noise~ @mode white` through `*~ 0.1`):
  this is the *s* and *t* budget. The object has no unvoiced/sibilance
  path of its own, so the noise rides the carrier full-time and the
  modulator's high-band envelopes gate it into consonants exactly when
  needed.
- **Pitch is the performance.** The vocoder never changes the carrier's
  pitch, so the carrier's notes are the melody. Held chords (an `mc.`
  stack of carriers) make the robot a choir; a single line makes it a
  lead vocalist.

## The modulator, cast against type

Articulation beats fidelity — the chapter's measured point is that band
envelopes carry everything, so contrast between bands is what you feed it.
A cheap dynamic mic is fine; compression helps; and over-enunciating
helps more than any knob. Keep the modulator out of the mix — the machine
uses it, nobody should hear it.

## The three settings

| patch | `q` | `response_interval` | the craft |
|---|---|---|---|
| the talking synth | 20 (default) | 30 | speak in rhythm; consonants land like drum hits |
| the choir | 10–15 | 250 | sing sustained vowels; the carrier chord is the harmony |
| rhythm transfer | 25–40 | 10–20 | drum loop as modulator; any sustained pad as carrier |

`q` trades crispness against smoothness (narrow separates consonants,
wide blends vowels); `response_interval` is the mouth's speed — attack and
release in one knob. `gain` is linear makeup, and you will need some: a
band-multiplied signal lands quieter than either input.

Two wiring facts that account for most dead patches: the **modulator is
the left inlet** (a synth weakly filtered by your voice means the cables
are backwards), and a silent carrier is silence no matter how loudly you
speak — pinned by test, and the fastest debugging question in vocoding.

## When to leave the recipe

- **You want tuned speech, not a played carrier** — the corrector
  (`tap.tune~`) moves the voice itself; the vocoder wears the voice over
  something else. Different identity theft.
- **You want formant-shifted or gender-shifted voice** — that's spectral
  surgery, not band gating; the corridor starts at `tap.spectra~`.
- **You want intelligibility above all.** Twenty-four analog-style bands
  are a voice, not a spectrograph; if every syllable must survive, dry
  speech mixed under the vocoded double is the radio trick that always
  works.
