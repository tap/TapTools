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

## The songbook

The famous "vocoder songs" are the best syllabus for the craft — partly
because several of them aren't vocoders, and knowing which is which
teaches more than any preset. Provenance below follows the part's rule:
documented where it's documented, labeled reconstruction where it isn't.

### "In the Air Tonight" (1981) — the ghost choir

What's documented: Phil Collins ran the verse vocal through a Roland
VP-330 — a *soft* vocoder, voiced like a string machine, mixed under a
nearly whispered dry vocal. The reconstruction: this is the anti-robot
patch. Carrier: two saws, `detune` ±4, `imperfect 0.3`, **no noise
layer** — sibilance is what you don't want here — through `tap.svf~`
(`@type lowpass @frequency 4000`) to take the glass off. Vocoder:
`q 8–12`, `response_interval 120` — wide bands and a slow mouth blur the
consonants into breath. Mix the vocoded return *under* the dry voice, a
shadow rather than a double. The dry whisper carries the words; the
vocoder carries the dread.

### "Mr. Blue Sky" / "Mr. Roboto" / "Intergalactic" — the front-and-center robot

ELO (EMS vocoders, documented), Styx, and the Beastie Boys are the
talking-synth patch played as a *lead*: bright carrier, crisp bands
(`q 20–30`), fast mouth (`response_interval 15–30`), and the melody in
the carrier's held notes while the words ride the modulator. Kraftwerk —
the genre's founders, on custom and commercial hardware across the
years — sit here too, usually with a single unison line rather than
chords: the robot speaks in monophony. Enunciate. Then enunciate more.

One label to keep straight: Zapp, Roger Troutman, and the P-Funk
talkbox records are **not vocoders** — a talkbox pipes the carrier into
the performer's actual mouth and the room mic hears real articulation.
Chasing that sound with this object gets you a cousin, not the thing.

### "Hide and Seek" (2005) — the one that isn't a vocoder

What's documented: Imogen Heap sang into a harmonizer (the DigiTech
Vocalist lineage), a keyboard choosing the chord — so every sound on the
record is her *actual voice*, pitch-shifted into harmony, breath and
formants intact. That's why it doesn't sound like a robot; there is no
carrier. Three routes, honestly ranked:

1. **The right tool — `tap.harmony~`.** This record's mechanism is
   exactly what the object does: formant-preserving voices holding a
   chord over the aligned dry voice. Its recipe — with the Bon Iver
   patches that extend the lineage — is [A choir of one](choir-of-one.md).
   (This object exists because this section's first draft had to work
   around its absence; the audit worked.)
2. **The manual fallback — a shifter stack.** Voice into parallel
   `tap.shift~` objects at chord intervals (`tap.semitone2ratio` feeds
   their ratio inlets). Keep the voicing within ±7 st — plain granular
   shifting moves formants with the pitch, and wide intervals go
   chipmunk where the formant-corrected routes don't.
3. **The vibe — the choir patch above.** Speak-sing into the choir row's
   settings with an `mc.` carrier holding the chords. It will sound like
   a vocoder doing Imogen Heap, which is its own valid sound — just
   don't mistake it for the record's mechanism.

### The plugin-era default — an Orange-school carrier

The late-90s software vocoders (the Orange Vocoder the most loved of
them) changed the *default sound* of the effect: where hardware
vocoders leaned on whatever synth was nearby, the plugins shipped with
a built-in, very bright virtual-analog carrier — so "the plugin sound"
is really a carrier voicing: wide, glossy, present. One honest line
first: that plugin is still a shipping commercial product, and the
house provenance rule applies — nothing here reverse-engineers it. What
follows is *our* bright VA carrier in that school, built from this
package's own oscillator:

```text
tap.vco~ (saw, f,   detune -6, seed 11) ──┐
tap.vco~ (saw, f,   detune +6, seed 22) ──┼─▶ +~ ─▶ tap.svf~ (highshelf) ─▶ carrier
tap.vco~ (saw, f+12, gain -6,  seed 33) ──┤
tap.noise~ (white) ─ *~ 0.08 ─────────────┘
```

All three oscillators `@shape 2 @imperfect 0.2 @jitter 2`; the octave-up
voice adds the gloss the era is remembered for; `tap.svf~ @type
highshelf @frequency 6000 @gain 4` is the sheen. Vocoder settings:
`q 25`, `response_interval 20`. Play the carrier in fifths and octaves
rather than full triads — the brightness supplies the width, and triads
in a bright carrier smear the consonant bands.

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
