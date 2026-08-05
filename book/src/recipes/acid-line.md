# Move a knob while it loops

The documented origin story of acid house is an instruction manual for this
recipe: in Chicago around 1985–87, Phuture (DJ Pierre, Spanky, Herk) let a
secondhand TB-303 loop a pattern and *turned the knobs while it played* —
"Acid Tracks" is twelve minutes of that. The lesson generalizes: an acid
line is not a melody with a sound; it is a **loop plus a hand**. The
pattern's job is to give the couplings something to chew on — accents for
the bloom, slides for the vowels — and the performance is `cutoff`,
`resonance`, and `envmod` moving in real time.

Everything measured here is borrowed from [the acid machine
chapter](../acid.md) and its notebooks
([`tb303.ipynb`](https://github.com/tap/TapTools/blob/main/notebooks/tb303.ipynb),
[`step_seq.ipynb`](https://github.com/tap/TapTools/blob/main/notebooks/step_seq.ipynb)).

## The scaffold

```text
phasor~ (BPM/240) ──▶ tap.303.seq~ ──pitch──▶ tap.303~ ──▶ out
                                  └──gate───▶   (right inlet)
```

One bar of 16 steps per phasor cycle (BPM ÷ 240, the drum scaffold's math);
~125 BPM → `phasor~ 0.5208`. The sequencer's pitch and gate outlets are the
voice's own contract — accents ride the gate at 2.0, slides are pitch
changes under a held gate, so the voice's ~60 ms RC does the glide.

## A line to start from

Program per step (`step <n> <pitch> [accent] [slide]`, `rest <n>`) or per
lane. A serviceable opener in A — and, as everywhere in this part, a
starting point, not a transcription:

```text
step:    1  2  3  4  5  6  7  8  9  10 11 12 13 14 15 16
pitch:   33 33 45 33 33 36 33 31 33 33 45 47 33 33 31 36
gate:    x  x  x  x  .  x  x  x  x  x  x  x  .  x  x  x
accent:  A  .  .  A  .  .  A  .  .  A  .  .  .  .  A  .
slide:   .  .  .  .  .  .  .  S  .  .  .  S  .  .  .  .
```

```text
pitches 33 33 45 33 33 36 33 31 33 33 45 47 33 33 31 36
gates   1 1 1 1 0 1 1 1 1 1 1 1 0 1 1 1
accents 1 0 0 1 0 0 1 0 0 1 0 0 0 0 1 0
slides  0 0 0 0 0 0 0 1 0 0 0 1 0 0 0 0
```

The ingredients that make it acid rather than bass: the octave jumps
(33 → 45), at least one slide *into* a note (the flag sits on the target
step), rests that let the filter close, and accents placed where the groove
leans — not where the melody peaks.

## The voice

`recall 1` is the factory "squelch" and a fine start. Explicitly:

```text
@waveform saw @cutoff 500 @resonance 0.9 @envmod 0.7 @decay 300 @accent 0.8
```

Then the moves, in the order a set builds:

1. **Ride `cutoff`.** 300 → 2000 Hz over eight bars and back. This is the
   genre. Remember the modeled `envmod` law: 2/3 of the envelope's sweep
   sits above the knob, 1/3 below, and the resting point shifts as you turn
   it — the knobs interact like the hardware because the interaction is
   modeled.
2. **Stack the accents.** Runs of accented notes at high `resonance` are
   the wow: the C13 capacitor doesn't fully discharge between close
   accents, and the measured cutoff-peak bloom across a run is **×1.94**.
   Put three accents in a row somewhere and listen to the third one open.
3. **Raise `resonance` into the break.** Past 1.0 is the documented bend
   territory — a stock 303 never self-oscillates, and neither does this
   filter until you push it there deliberately.
4. **`waveform square` for the hollow verse, saw for the drop.**
5. **`vca warm`** thickens exactly where the hardware does — measured 5.4 %
   difference signal on quiet notes, 11.5 % on hot accents.
6. **Transpose, don't re-program:** `transpose -12` on the sequencer for
   the sub-drop, `+5`/`+7` for the question-answer sections. It shifts
   live, like the hardware's transpose mode without the mode.

## After the voice

Acid techno's other instrument is the distortion pedal: `tap.overdrive~`
after the voice, driven hard, is the documented lineage (a 303 into a
screaming feedback overdrive is half the harder end of the genre). Keep
`mute` in reach on the sequencer for breakdowns — it drops the gate but the
clock keeps running, so the line re-enters exactly in place.

## When to leave the recipe

- **You're programming melodies.** If the line only sounds right without
  slides or accents, it isn't an acid line yet — or it wants the generic
  bass rig (`tap.vco~` + `tap.svf~` + `tap.adsr~`) instead of this voice's
  refusals.
- **You want the filter alone** — `tap.diode~` gives you the 303's ladder
  on any source, squelch and all, without the biography.
- **You want hands-free evolution.** The 303 rewards a hand on a knob; if
  the patch must run itself, store extremes in the voice's preset slots and
  ride timed `recall` morphs instead — a different instrument, honestly.
