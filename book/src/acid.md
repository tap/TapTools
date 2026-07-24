# The acid machine

The Roland TB-303 was designed to imitate a bass guitar, failed completely,
and accidentally defined thirty years of dance music. What makes it
unmistakable is not any one block — a saw into a lowpass is every synth ever
made — but the *coupling*: accent drives the filter and the amplifier through
shared circuitry with memory across notes, slide is a gate that refuses to
let go, and the envelopes are fixed RC discharge curves with exactly one knob
between them. `tap.303~` is a circuit-informed model of that whole tangle;
`tap.diode~` is its filter as a standalone object; `tap.303.seq~` is the
other half of the instrument. This chapter is what each control trades, and
what the measurements say the model actually delivers.

Companion material: the reference pages and help patchers in the TapTools-Max
package, and two executed verification notebooks —
[`tb303.ipynb`](https://github.com/tap/TapTools/blob/main/notebooks/tb303.ipynb)
for the voice and
[`step_seq.ipynb`](https://github.com/tap/TapTools/blob/main/notebooks/step_seq.ipynb)
for the sequencer — every number below is a measurement from one of them or
from the kernel test suite. Provenance runs through Tim Stinchcombe's filter
analysis, Robin Whittle's Devil Fish documentation, the x0xb0x schematics,
and Robin Schmidt's Open303, whose measured calibrations several constants
adopt verbatim.

## What the hardware is, in one paragraph

One saw-core oscillator (the "square" is the saw through a transistor
shaper, not a clean pulse), into a four-stage **diode**-ladder filter — not
the Moog transistor ladder; the diode ladder's stages load each other, which
is why its resonance is broader, less pure, and entirely its own — then a
one-transistor amplifier. Two envelopes, both decay-only RC discharges: the
Main Envelope sweeps the cutoff (the `envmod` knob decides how much), the
VCA envelope is fixed. Accent makes the Main Envelope hotter *and* faster,
routes it into the VCA, and charges a capacitor (C13) through the resonance
pot — and because C13 doesn't fully discharge between closely spaced accents,
**runs of accented notes bloom**, the famous wow. Slide holds the gate across
the step boundary while the pitch CV glides through a ~60 ms RC. Everything
about a note — pitch, gate, accent, slide — comes from the sequencer, not
the panel. That is why this is three objects, not one.

## The filter first: `tap.diode~`

The panel says "18 dB/oct"; the circuit is four poles whose asymptotic slope
is 24 dB/oct with a shallower region near cutoff — Stinchcombe untangled
this, and the kernel reproduces his published transfer function to
**0.028 dB**. Two behaviors are load-bearing and easy to get wrong:

- **The resonance feedback runs through a 150 Hz high-pass**, so resonance
  thins as the cutoff drops — low notes squelch, they don't ring. Pinned by
  test: the ring-down Q falls with cutoff.
- **A stock 303 never quite self-oscillates**, and neither does this filter
  at stock settings. That emerged from the modeled feedback high-pass rather
  than being programmed in, and it's documented as a trait, not a defect.
  (Push `resonance` past 1.0 — the bend range runs to 1.5 — and it will sing
  for you anyway.)

Like `tap.ladder~` it has a `solver` choice: `fast` (default) or `exact`,
which iterates the re-linearized solve to convergence on the true nonlinear
loop. Measured across a matrix out to
resonance 1.4 and +24 dB drive — beyond anything the hardware can reach —
the two differ by at most **−44.9 dBr**, at 1.6–3.3× the CPU. The exact
solver is there for the suspicious; the fast one is there for the patch.
`oversample` (1/2/4, default 2) and a signal-rate cutoff in the right inlet
round out the `tap.ladder~` surface.

## The voice: `tap.303~`, knob by knob

The attributes mirror the seven-knob panel; the calibrations are Open303's
measured laws.

![Signal-flow diagram of the 303 voice: pitch through slide into oscillator,
shaper, coupling highpass, diode ladder, and VCA, with the accent bus
fanning to the envelope, the C13 sweep capacitor, and the VCA, and the
envelope-driven cutoff CV feeding the ladder](images/tb303/block-diagram.svg)

*The blocks are ordinary; the red and amber wires are the 303. Accent
touches three destinations at once, and C13 remembers across notes — the
couplings are the instrument.*

- **`waveform`** — `saw` or `square`. The square is the hardware's shaped
  saw: `−tanh(10^(36.9/20)·saw + 4.37)`, Open303's measured constants
  verbatim — rounded and notched, audibly not a 50 % pulse.
- **`cutoff`** — the knob in Hz. Stock travel is the measured 302–2394 Hz;
  the attribute range (100–5000) is a flagged bend beyond the panel.
- **`resonance`** — 0..1 is stock; up to 1.5 is the bend.
- **`envmod`** — how much Main Envelope reaches the cutoff, with the
  hardware's measured law: 2/3 of the sweep goes above the knob position,
  1/3 below, and the "gimmick" offset shifts the resting point down as you
  turn it up. The knobs *feel* right because the interaction is modeled, not
  just the ranges.
- **`decay`** — Main Envelope decay, 200 ms–2 s. On an accented note the
  hardware ignores this knob and runs at ~200 ms; so does the model
  (adjustable via the `accdecay` bend, 50–2000 ms).
- **`accent`** — how hard accented notes hit: louder *and* punchier (the
  envelope routing), and quackier (the C13 sweep, scaled by the resonance
  knob). The wow is measured: over a run of closely spaced accents the
  cutoff peak builds by **×1.94**, and decays back within ×0.998 once the
  accents stop. Consecutive accents at high resonance are the entire genre.
- **`tuning`**, **`gain`** — cents and dB. Plumbing.

The envelopes carry the schematic's fixed interrelations: MEG attack ~3 ms,
VCA attack ~3 ms with a measured ~1.23 s decay chopped at gate-off, 50 ms when accented.
None of these have knobs on the hardware, so none of them have knobs here —
except through the documented Devil-Fish-style bends (`slide` 10–500 ms,
`attack` 0.3–30 ms, `accdecay`, and `drive` ±24 dB into the ladder, where
the diodes compress: +24 dB of gain buys only 9.2× of RMS). All stock at
their defaults.

Phase 2 added **`vca clean|warm`**: the one-transistor class-A stage as a
slope-normalized biased saturator, in the hardware's signal order. The
distortion tracks the envelope — measured 5.4 % difference signal on quiet
notes, 11.5 % on hot accents — so `warm` thickens exactly where the hardware
does. `clean` (default) is bit-identical to phase 1.

House machinery throughout: `seed`/`tolerance` per-unit component spread (an
`mc.` stack of 303s with different seeds detunes and drifts like a wall of
real units), 16 preset-morph slots with factory acid in 1–8
(squelch, sub, screamer, rubber, knock, bloom, overdriven, glass), and
per-sample ramps on every parameter.

## The note interface, and why slide is free

`tap.303~` is TapTools' first pitched instrument, and its inlets are the
package-wide melodic contract: **pitch as a MIDI note number signal** in the
left inlet, **gate with amplitude-as-accent** in the right — 1.0 is a plain
note, 2.0 fully accented (depth = amplitude − 1). Slide needs no input at
all: *a pitch change while the gate is held is a slide* — legato, no
envelope retrigger, the ~60 ms RC glide — which is exactly the hardware's
own definition. A `note <pitch> [accent] [slide]` message covers patching
without signals.

## The other half: `tap.303.seq~`

Half the 303's sound is sequencer behavior, so the sequencer emits the
voice's contract verbatim: a pitch signal and a gate signal, clocked by a
phase ramp (0..1 per pattern, a `phasor~`). Per step: pitch, gate/rest,
accent, slide. The measured facts, from the sequencer notebook:

- Steps land on the analytic grid within **one sample**; the gate opens at
  the step start and closes at **0.5** of the step (Open303's
  `stepLength`).
- A slid step is approached with the gate *held*: 16 gated steps with 3
  slide flags produce exactly **13 note-ons** — the other three arrive
  legato, pitch stepping on the boundary sample, and the voice glides.
- Accented steps gate at 2.0; `transpose` shifts live, like the hardware's
  transpose mode without the mode; `swing` and pattern slots with
  cycle-quantized `recall` are shared with the drum rows (next chapter).

The 1981 pitch-mode/time-mode data entry is deliberately not recreated. You
keep the data model; you lose the part everyone hated.

## When it is not the right tool

- **You want a generic bass synth.** `tap.vco~` + `tap.svf~` +
  `tap.adsr~` give you ADSRs, waveform variety, and a filter that behaves.
  This object's value *is* its refusal to decouple.
- **You want the filter without the biography** — `tap.diode~` alone, or
  `tap.ladder~` if you want the Moog character instead of the 303's.
- **You want polyphony.** It's a monosynth; `mc.` gives you many monosynths,
  which is not the same thing as a polysynth and shouldn't be.

## Checkpoint

A diode ladder that matches the published analysis to 0.028 dB and won't
self-oscillate until you bend it; a voice whose envelopes, accent path, and
C13 memory come from the schematic, with the wow measured at ×1.94 across an
accent run; slide as pure gate-hold, so legato falls out of the note
contract; and a sequencer that emits that contract sample-accurately. The
coupling is the instrument — and every claim above has an executed notebook
cell behind it.
