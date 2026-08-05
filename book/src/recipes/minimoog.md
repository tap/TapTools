# Three oscillators into a ladder

The oscillator chapter ends with the Moog recipe's core — the three-voice
saw stack and the driven ladder — and ranks what each ingredient buys. This
recipe finishes the instrument: the two envelopes, the amplifier, the gate,
and the settings that turn one signal chain into the two patches everyone
actually means by "Moog" — the bass that walks and the lead that sings.
The model here is the classic three-oscillator monosynth voice: three
oscillators into a mixer, one four-pole ladder, one loudness contour, one
filter contour, glide on the pitch. Nothing below requires an object the
package doesn't ship.

Companion material: the [oscillator chapter](../vco.md) (the stack's
rationale and the analog section's ranges), the [ladder
chapter](../ladder.md) (every filter number below is measured in its
[notebook](https://github.com/tap/TapTools/blob/main/notebooks/ladder.ipynb)),
and the reference pages for `tap.adsr~` and `tap.vca~`.

## The voice, wired

```text
pitch (midi note) ──▶ mtof ─┬─▶ tap.vco~ (voice 1)──┐
                            ├─▶ tap.vco~ (voice 2)──┼─▶ *~ 0.36 ─▶ tap.ladder~ ─▶ tap.vca~ ─▶ out
                            └─▶ ÷2 ─▶ tap.vco~ (3)──┘                    ▲              ▲
gate (0/1 signal) ──┬───────────▶ tap.adsr~ (filter contour) ─▶ *~ amount ─▶ +~ base ──┘│
                    └───────────▶ tap.adsr~ (loudness contour) ─────────────────────────┘
```

- **Pitch** arrives as note frequencies (floats into each `tap.vco~` left
  inlet; halve for voice 3's octave-down). The oscillators' own `smooth`
  ramp is the glide knob — no portamento object exists or is needed.
- **Gate** is any signal that goes above 0.5 and back — `tap.adsr~` reads
  the gate by level, per sample. A `tap.303.seq~` gate output (1.0 plain,
  2.0 accented) drives it directly, which also gets you slides for free;
  so does a MIDI-driven 0/1 signal, or the `trigger 1` / `trigger 0`
  attribute messages for mouse-driven patching.
- **The filter contour** scales into the cutoff's signal inlet: envelope ×
  `amount` (Hz) + `base` (Hz) into `tap.ladder~`'s right inlet. The classic
  panel's "amount" knob is your `*~`.
- **The loudness contour** multiplies the ladder's output — `tap.vca~` with
  the envelope into its gain inlet keeps the option of `@circuit warm`
  saturation later.

One period-correct honesty note: the original panel's contours are
attack/decay/sustain with a release *switch* (release equals decay, on or
off). `tap.adsr~` gives the full four stages; set `release` equal to
`decay` and you have the switch's "on" position.

## The stack and the ladder

The three-voice table is the oscillator chapter's, reproduced so this page
patches alone:

| voice | frequency | `detune` | `drift` | `seed` |
|-------|-----------|----------|---------|--------|
| 1     | f         | −4       | 8       | 11     |
| 2     | f         | +5       | 8       | 22     |
| 3     | f ÷ 2     | +2       | 10      | 33     |

All three: `@shape 2` (saw), `@jitter 3 @track 2 @imperfect 0.3`, and
`smooth` per the patch below. Sum through `*~ 0.36` (≈ 1/2.8, headroom for
three voices), then `tap.ladder~` at the chapter's voicing: `@mode 0` (the
lp24 response — the mode attribute is the numeric pole-mix index)
`@resonance 0.35 @drive 9 @asym 0.45 @comp 0.25`. Keeping `comp` low
preserves the authentic passband droop; `drive 9` sits where the ladder
notebook measures the tanh stages just starting to thicken (3.5 % THD at
8 dB). Spend the character budget in the filter first — the chapter's
measurements are the argument.

## Patch one: the bass

The left hand of a decade of records: short filter contour, no vibrato,
glide short enough to read as punch rather than portamento.

| control | setting |
|---|---|
| all `tap.vco~` `smooth` | 25 ms |
| filter `tap.adsr~` | `@attack 2 @decay 220 @sustain -18 @release 220` |
| filter `amount` / `base` | 2500 Hz / 120 Hz |
| ladder `resonance` | 0.25 |
| loudness `tap.adsr~` | `@attack 2 @decay 400 @sustain -3 @release 120` |

The sound lives in the filter contour's `decay`: 220 ms is the "wah" that
articulates each note. Shorten toward 120 ms and it turns percussive;
lengthen toward 400 ms and it turns brassy. For a rounder, more
sub-friendly bass, drop `drive` to 3 and `asym` to 0.2 — the even
harmonics are lovely on a lead and muddy on a bass amp. If anything
downstream cares about DC, remember the ladder chapter's warning:
an asymmetric saturator can leave a small signal-dependent offset —
`tap.dcblock~` after the VCA is one object of insurance.

## Patch two: the lead

The singing version: longer glide, opened filter, resonance high enough to
color but under the edge, and the release switch "on."

| control | setting |
|---|---|
| all `tap.vco~` `smooth` | 80 ms |
| filter `tap.adsr~` | `@attack 15 @decay 600 @sustain -8 @release 600` |
| filter `amount` / `base` | 4000 Hz / 300 Hz |
| ladder `resonance` | 0.55 |
| loudness `tap.adsr~` | `@attack 8 @decay 300 @sustain -2 @release 350` |

Two moves push it from good to *that sound*:

- **Play the glide.** 80 ms of `smooth` means overlapping note changes
  swoop; detached ones barely bend. The keyboard articulation is the
  vibrato.
- **Lean on the octave voice.** Pull voice 3 up to `f` (unison) for the
  hollow reedy register, or leave it at `f ÷ 2` and drop voice 2's level
  for the fat fifth-less stack. The `interp`-timed preset morph (`store` /
  `recall <slot> <ms>`) can glide between these voicings mid-phrase —
  a patch element the hardware never had.

## What each ingredient buys

In order — and, per the house rule, cut from the bottom when CPU or taste
says so:

1. **The stack.** Three free-running voices at ±cents is most of the sound
   (the oscillator chapter's argument, with its measurements).
2. **The ladder.** Drive, asymmetry, and the low-`comp` droop — the
   character budget.
3. **The filter contour.** The one envelope listeners hear as "the synth's
   voice." Its `decay` is the most audible 100 ms in the patch.
4. **Glide.** Free, iconic, already in the oscillator.
5. **The loudness contour.** Keep it simple; the filter does the talking.
6. **The analog section.** `drift`/`jitter`/`imperfect` at the chapter's
   moderate settings — salt, not sauce.

## When to leave the recipe

- **You want polyphony.** This is a monosynth voice; `mc.`-wrapping the
  whole chain gives you many monosynths, and a real polysynth patch wants
  per-voice envelopes and different discipline.
- **You want the 303 instead.** The couplings that make acid are a
  different instrument — `tap.303~` refuses to be decoupled, and that
  refusal is its chapter.
- **You want clean.** Every stage here has an opinion — `tap.svf~` and
  `tap.fourpole~` are the polite siblings when the patch needs a filter,
  not a character.
