# The drum machine

The Roland TR-808 is the most thoroughly analyzed drum machine in the
academic literature, and the reason is charming: the whole instrument is
analog synthesis. No samples anywhere — every sound is a small circuit, and
most of them are variations on about four ideas. The `tap.808.*` family
recreates the eight voice channels circuit block by circuit block, one
external per channel, and `tap.808.seq~` supplies the machine's other half
as one sequencer row per patch cord. This chapter is the family tour: the
shared trigger contract, each voice's character and knobs, and the
calibration pass against a real unit that the numbers come from.

Companion material: each voice's reference page and help patcher, the family
overview patcher (`tap.808.maxhelp` — all eight voices sequenced off one
`phasor~`), the
[`tr808_calibration.ipynb`](https://github.com/tap/TapTools/blob/main/notebooks/tr808_calibration.ipynb)
notebook, and the
[`step_seq.ipynb`](https://github.com/tap/TapTools/blob/main/notebooks/step_seq.ipynb)
sequencer notebook. Provenance runs through the Werner–Abel–Smith papers
(DAFx-14 and companions) and the TR-808 Service Notes, read component by
component; every magic constant in the kernel headers carries its schematic
designator.

## One trigger to rule them all

On the hardware, every voice hangs off a common trigger bus: the CPU's 1 ms
pulse rides a voltage between 4 and 14 V depending on the accent circuit,
and a hotter pulse excites each circuit *harder* — more punch, slightly
different timbre — not merely louder. The family keeps that literally:
**every voice fires on a signal rising edge, and the edge's amplitude
(0..1) is the accent**, mapped onto the 4–14 V bus. `bang` and
`trigger 0.7` messages cover the scheduler side. Filter states persist
across triggers, so fast rolls interfere with the ringing tail like the
hardware — no machine-gun effect. And because the excitation is a voltage,
anything that makes an edge can play the kit: a `click~`, an envelope, a
`tap.303.seq~` gate, or the row object built for the job.

## The voices

### `tap.808.kick~` — the bridged-T with a biography

The bass drum is a damped bridged-T resonator (~49.4 Hz from the modeled
component values; Roland's chart optimistically says 56, real units measure
as low as 48) with three behaviors that make it *the* kick, all emergent
from the modeled schematic: for the first ~6 ms the envelope saturates Q43
and the resonator sits near ~129 Hz — the attack punch, which is a
*different mechanism* from the famous downward pitch "sigh" (leakage through
R161, the paper's fitted nonlinearity); and a retriggering pulse re-excites
the center node as the envelope collapses so the note doesn't step down.
Panel knobs: `decay` (seconds of ring at the top), `tone` (click at ~7 kHz
down to ~300 Hz), `level`. Paper-documented bends, stock by default:
`tuning`, `pulse`, `sigh`, `attack` — turn `sigh 0` and the pitch relaxation
disconnects, exactly as the bend does on the bench.

Calibration: against a real unit's knob-gridded sample set, the fundamental
sat within **2.4 %** at every tone/decay position and the −40 dB decay
endpoints within 6 % (72 ms → 2.36 s measured, 69 ms → 2.42 s modeled) —
**no constant needed changing**.

### `tap.808.snare~` and `tap.808.clap~` — resonators plus noise

The snare is two bridged-Ts (the late-revision ~173/336 Hz pair) with a
trigger divider and the "snappy" path — enveloped noise, band-limited around
4 kHz to the measured unit. Fundamentals calibrated within 1.2 %, including
the mode flip at tone-max. The clap channel (`@model clap|maracas`) is the
Service Notes' Figure-13 circuit: band-passed noise near 2 kHz through a VCA
driven by a three-teeth sawtooth retrigger — the "multiple hands" transient —
plus the Q70 reverberation tail. The maracas mode is the same noise voiced
short and bright.

### `tap.808.hat~` and `tap.808.cymbal~` — the metal bank

Six Schmitt-trigger square oscillators (205.3, 369.6, 304.4, 522.7 Hz plus
the two trimmer-tuned at 800 and 540, duty 47.98 %) feed two bandpass
voicings near 3.4 and 7.1 kHz. Werner et al. measured that resistor variance
puts any given unit **up to ~20 % off** those frequencies — which is why no
two 808s' cymbals sound alike, and why `seed`/`tolerance` exists: every seed
is a different unit off the line, and an `mc.` stack of cymbals decorrelates
like real hardware. The hats are **one object with two trigger inlets**
because on hardware they are one circuit with two envelope paths and a
choke — closed chokes open (the Q23/R173 path), pinned by test, and
unimplementable as separate externals. Open-hat decay spans the chart's
90–600 ms; the cymbal's two separately enveloped bands cover its 350–1200 ms
"sizzle" span. `tap.808.cowbell~` taps just the 540/800 pair into the ~860 Hz
voicing with a two-slope envelope; more cowbell is a patching decision.

### `tap.808.tom~` and `tap.808.rim~` — the resonator variations

Six sounds on two objects, as the hardware switches them: `@size low|mid|high`
× `@model tom|conga`. Congas are the tom circuit without its noise layer,
tuned differently; the toms add the D80/D81 attack pitch fall and a pink
noise layer. The rim channel is `@model rimshot|claves`: the rimshot's
~1667 + 455 Hz crack with the swing-VCA's harmonics, versus the claves' pure
~2500 Hz tick. Tunings sit within ~4 % of the measured unit.

![The bridged-T resonator circuit: an op-amp with capacitive arms, a resistive bridge, and a leg to ground, triggered through an injection resistor, with the kick's per-sample leg modulation drawn in red — and the eight voices grouped by how they use it](images/tr808/bridged-t.svg)

*Roland's universal voice circuit. Eight voices, one network — the kick earns its punch by modulating the leg per sample.*

## The calibration pass, honestly

The §7.2 calibration ran against a real TR-808 (s/n 103852) recorded from
the individual outs with **knob positions encoded in the filenames** — a
0/2.5/5/7.5/10 dial grid, 116 samples — which upgraded "sounds right" to a
quantitative per-knob-cell comparison. Identical measurements (spectral-peak
fundamental, −40 dB decay, power centroid) ran on both sides. The pitches
were already right nearly everywhere; what the pass actually changed was
*time*: tom, conga, cowbell, and clap tails roughly doubled to match the
unit, the snappy was band-limited and re-enveloped, the rimshot re-voiced
low-dominant, the cymbal's decay span corrected. Each kernel header carries
its residuals. The lesson generalizes: schematics get you the frequencies;
recordings get you the envelopes.

## The other half: `tap.808.seq~`

One row of the 16-step sequencer, as an object: feed it a phase ramp (0..1
per pattern, a `phasor~`) and it emits trigger impulses whose amplitude is
the step's accent — the family contract, straight into any voice. Twelve
rows off one phasor are the hardware's panel, sample-locked forever; the
accent row falls out of giving every row the same `accents` list. The
measured facts, from the sequencer notebook: steps land on the analytic grid
within one sample; the pinned levels are **plain 0.01** (the 4 V base — an
un-accented hit still strikes the circuit) and **accented 0.5** (the accent
knob at noon; 1.0 is the full 14 V); `swing` delays the off-16ths by exactly
swing/2 of a step; a `length 12` row against 16s is the triplet pre-scale
generalized to polymeter; and pattern slots with cycle-quantized `recall`
are the A/B-half and fill switching as one message. `pulse` widens the
impulse into a held gate when you'd rather drive `tap.adsr~` than a drum.

## When it is not the right tool

- **You want *a* kick, not *the* kick.** A sine with an envelope is cheaper
  and takes EQ more politely. This family's value is the circuit behavior —
  the attack jump, the choke, the accent-as-voltage.
- **You want your own drum sounds.** These circuits are what they are;
  `seed`, the documented bends, and the panel knobs bend them, but a
  sampler is a sampler.
- **You want 909 hats.** The 909's metal is sampled; this machine's is six
  square waves. Different instrument, different chapter, maybe someday.

## Checkpoint

Eight channels, four circuit ideas — bridged-T resonators, a shared metal
bank, noise paths, swing-VCAs — under one amplitude-as-accent trigger bus,
calibrated per knob cell against a real unit and honest about what changed
(the tails) and what didn't (the tunings). The hats choke because they share
a circuit; the cymbals decorrelate because resistors do; and the sequencer
row emits the same voltage idea the voices drink, so the whole kit runs off
one phasor ramp. The machine's two halves, both measured.
