# A choir of one

This chapter exists because this book's own audit demanded it. The vocoder
songbook had to label "Hide and Seek" honestly — a harmonizer, not a
vocoder: pitch-shifted copies of the *actual voice*, formants intact, no
carrier anywhere — and the package had no object for that mechanism. Now it
does. `tap.harmony~` holds up to four formant-preserving voices at
intervals you set in semitones, over a dry path the kernel delays into
alignment so chords land as chords. This recipe is how to sing through it,
and its worked examples are the modern masters of the effect: Bon Iver.

The claims behind the object are measured in the executed
[verification notebook](https://github.com/tap/TapTools/blob/main/notebooks/harmonizer.ipynb)
and pinned in the kernel's test battery (`tests/harmonizer_test.cpp`):
across two octaves of voicings every interval lands within **0.04 cents**
of its equal-tempered target under the DspTap yin oracle; the dry path is
sample-aligned with the voices to a 3.7×10⁻⁸ residual — why chords land
as chords, not flams; a synthetic formant bump stays near home only when
`formant` is on (band centroid 1058 → 1154 Hz on a +7 shift, versus 1439
riding the full ratio with it off); and interval glides walk the pitch
through the middle instead of jumping. The engine per voice is the DspTap
phase vocoder — the same peak-locked shifting and LPC formant machinery
the [pitch machine chapter](../machine/pitch.md) derives.

## The instrument

```text
voice ──▶ tap.tune~ (@speed 0, key of the song) ──▶ tap.harmony~ ──▶ out
```

The corrector upstream is optional but it is the modern sound: hard-snap
the lead first and every harmonizer voice inherits the quantized pitch, so
the stack locks like a keyboard instrument instead of drifting like a
choir. Skip it and the stack breathes with your intonation — older,
warmer, more Crosby-Stills than Vernon.

Three controls do the character work:

- **`formant` on** is the entire point: an octave-down voice stays *you*,
  bigger. Off is the chipmunk-chorus bend — useful, but it stops being a
  choir.
- **`chord`** is the performance surface: `chord -12 3 7` sets three
  intervals and silences the fourth voice in one message. Wire a Max
  chord-to-intervals mapping (played notes minus the sung root) and the
  keyboard chooses the harmony live — the rig the credits of the records
  below describe.
- **`glide`** at the 10 ms default snaps chord changes; at 300–500 ms the
  stack *slides* between chords, which no group of human singers can do
  and is worth featuring, not hiding.

One honest number: latency is one FFT frame (`fftsize`, default 1024
samples ≈ 21 ms at 48 kHz), dry path included. For live monitoring that
is audible as a slight remove — performers adapt in minutes, but mix the
monitor wet so they hear the instrument, not the delay ghost.

## "Woods" (2009) — the stacked chapel

What's documented: Justin Vernon built "Woods" from many overdubbed
a cappella takes, each hard-tuned — a chapel of his own voice, later the
foundation of Kanye West's "Lost in the World." The record's mechanism is
*overdubs*, and the recipe respects that:

- The live approximation: `tap.tune~ @speed 0` → `tap.harmony~` with
  `chord 3 7 12`, `@dry 1`, all through a long dark reverb
  ([the wash](shimmer.md) settings work). One pass, four-voice chapel.
- The faithful version: record *takes* — sing each chord tone through the
  corrector alone, layer them, and use `tap.harmony~` per take only to
  widen (`chord 12` at `@level1 0.4`). Stacked takes decorrelate the way
  overdubs do; one harmonizer pass, however good, is one performance.
  The difference is the difference between a choir and a string patch.

## "715 - CRΞΞKS" (2016) — the Messina

What's documented: the *22, A Million* credits name "the Messina," the
rig Chris Messina and Vernon built to pitch-stack his live voice into
chords (the Prismizer-school effect associated with Francis and the
Lights, and heard on Chance the Rapper's gospel records). "715 - CRΞΞKS"
is that instrument a cappella: every sound is the processed voice.

```text
voice ─▶ tap.tune~ (@speed 0) ─▶ tap.harmony~ @dry 1 @formant 1 @glide 10
                                     chord -12 3 7    (verse color)
                                     chord -12 4 7    (the lift)
                                     chord -5 3 10    (the ache)
```

- `@dry 1` — the lead lives *inside* the stack, equal citizen, exactly
  what makes the sound read as one multiplied person rather than
  lead-plus-backers.
- Chords change per phrase, not per note: bind each `chord` list to a key
  or a pedal and play the harmony like slow organ stops.
- The low voice carries the weight: `-12` under a falsetto lead is the
  record's signature register trick. Keep it at full level; thin the
  upper voices (`@level3 0.7`) when the stack gets glassy.
- No reverb, or almost none — the record's intimacy is the dry stack
  right against the microphone. Resist the wash this once.

## The craft notes

1. **Feed it one voice.** The formant model and the intervals both assume
   monophonic input — the kernel's header says so, and a strummed guitar
   through a "choir" proves it right within two bars.
2. **Mind the sum.** Dry plus four unity voices is five voices;
   `tap.limi~` or a `*~ 0.5` after the object is the standing advice.
3. **Close voicings beat wide ones.** ±12 is the working span; the
   engine's contract runs to ±24 and the top octave of that range is a
   *sound effect*, not a singer.
4. **The corrector's `speed` is the era dial.** 0 ms is 2016; 40 ms is
   1970s session stack; bypassed is a folk group.

## When to leave the recipe

- **You want the robot.** No carrier here, no bands — that's the
  [vocoder](robot-voice.md), and the two chapters together are the
  voice-processing fork in the road: wear the voice over a synth, or
  multiply the voice itself.
- **You want real ensemble.** Overdub real takes; the "Woods" section's
  faithful version is the honest ceiling of one person's choir.
- **You want harmony that follows chords you *sing*.** The object holds
  intervals; it doesn't do music theory. The keyboard (or your patch's
  chord logic) is the brain — which is exactly how the famous rigs work.
