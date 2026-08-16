# Plan — `tap.ondes~`, after reading the sources

> **Status: design, not yet implemented.** The source gate is closed —
> `PLAN-radiohead-family.md` §3 records what was found and how far each paper was read. This
> file is the design pass those findings forced, written before any code, because what the
> papers describe is **not the object the family plan sketched**.

## What changed, in one paragraph

The family plan assumed an Ondes Martenot voice built on `vco.h`: an oscillator with waveform
registers, switched timbres, the usual synthesis shape. The circuit paper says otherwise. The
instrument is a **heterodyne** design whose oscillators are, measured, essentially pure — about
0.03 % second-harmonic distortion even coupled to the rest of the circuit, which is precisely
the simplification Najnudel et al. use to reach real time. The character comes from what
happens *after* the difference tone: two triode stages, the intensity key, and the diffuseur.
So the kernel is a clean source into a nonlinearity into a gain law into a resonator — much
closer to `fuzz.h` and `garden.h` than to `vco.h`.

## The shape

```
ribbon/keyboard ─▶ heterodyne source ─▶ triode stages ─▶ touche ─▶ diffuseur ─▶ out
                     (near-sinusoidal)   (the timbre)   (the gain)  (the voice)
```

Four components under a thin composition, the family's habit — and unlike the tape echo's
`head` or the stammer's `slicer`, **three of these are independently useful** and should be
standalone externals from the start:

| Component | What it is | Standalone? |
|-----------|------------|-------------|
| `source` | the heterodyne difference tone, near-sinusoidal | no — it is an oscillator, `tap.vco~` territory if anyone wants one |
| `triode` | the two post-demodulator gain stages, the timbre | maybe — a triode-flavoured saturator has uses beyond this instrument |
| `touche` | the intensity-key gain law | **yes** — a measured, published expressive gain curve is useful on *anything* |
| `palme` / `metallique` | the diffuseurs as driven resonators | **yes** — run a guitar through the Palme; that is the killer feature |

`touche` and the diffuseurs are the reason to build this object even for someone who never
wants an ondes. That is worth designing for rather than discovering later, per the components
chapter's lesson.

## `touche` — fully specified, build it first

The measurement is in `PLAN-radiohead-family.md`; the design consequences:

- **Input is a position**, normalized 0..1 over the playable travel, not a force and not a
  velocity. The paper is explicit that the result does not depend on gesture speed, so a
  static curve is the finding, not a shortcut.
- **The curve is the published table, interpolated** — 45.0 / 53.3 / 61.6 / 70.0 / 78.3 / 86.6
  / 95.0 dB_SPL at 4.3 / 5.3 / 5.9 / 6.4 / 6.8 / 7.3 / 8.8 mm. Monotone cubic (PCHIP-style)
  through seven points, precomputed at `prepare()` into a table; no fitting, no analytic
  approximation, because the shape is the whole point and a straight line in dB-vs-mm is
  visibly wrong (the displacement steps are 1.0, 0.6, 0.5, 0.4, 0.5, 1.5 mm for equal dB
  steps).
- **50 dB of range** over ~4.5 mm of travel, with the bottom of the table at the noise floor.
  Below the first point the object should go to true silence rather than extrapolating.
- Expose the raw millimetre domain too, not just 0..1 — the numbers are published and someone
  will want to drive it from a real sensor.
- An optional **force** input as a second mode: dB is linear in log(force) below ~85 dB_SPL /
  1.3 N. Document it as the secondary map; displacement is primary.
- The curve is **pitch-independent** (all notes share the shape), which the tests should pin:
  the same `touche` position gives the same gain at any source frequency.

Honest limit to state in the header: the table is one instrument (No. 320) and the paper notes
variation between units can exceed 10 %.

## `triode` — where the timbre actually is

The circuit paper attributes the harmonics to two successive triode stages after the
demodulator, and their plugin exposes demodulator input gain as a harmonics control — a knob
the real instrument does not have, and a good precedent for exposing one here.

This is `fuzz.h` territory and should reuse its thinking rather than its code: a
conditioning filter, an asymmetric static curve (triodes are strongly asymmetric — even
harmonics are the point), an equalization filter, and oversampling. Two stages, cascaded, with
the gain-staging lesson from `fuzz.h` applied from the start: **the small-signal slope of the
curve family compounds**, so the drive floor must sit low enough that stage two is not
saturated at zero.

Open question: whether to model the triode with a published grid-conduction curve or to reuse
the tanh family with an asymmetry bias. The former is more honest to the instrument; the
latter is already in the house and measured. Decide with a listening comparison, and document
whichever loses.

## `source` — cheap, and deliberately so

A heterodyne pair whose difference tone is the note. Given the measured 0.03 % distortion, the
kernel should synthesize the difference tone **directly** as a sinusoid rather than simulating
two RF oscillators and a demodulator: same output, a fraction of the cost, and the paper is
the citation for why that is legitimate. The ribbon paper is the reference for how the
variable oscillator's frequency responds to the ribbon, which matters for glide feel.

State plainly in the header that this is a *documented simplification of a published model*,
not a circuit solve — and note the number that justifies it. Najnudel et al.'s full
port-Hamiltonian simulation runs at 768 kHz and their plugin consumes 85 % of a laptop core;
that is the road not taken, and the header should say so, so nobody assumes the simple path
was chosen out of ignorance.

## The diffuseurs — driven, not struck

The correction that matters most for reusing `garden.h`'s idiom. Wijnand et al.:

- **Métallique** (1944–45, patented 1947) — a gong excited by a **motor**.
- **Palme** (1949–50) — an electromagnet driving **12** metal strings on a soundboard.
- **Résonance** (1970s) — motor-excited metal springs.

So the mode banks carry over from `garden.h` but the excitation does not: these are
continuously driven resonators, not struck ones. No `decay_env` per mode; instead the input
signal drives the bank and the modes ring at their own decay rates — closer to `grm_comb.h`'s
sustained resonance than to the chime's strike envelopes. Mode ratios still come from Fletcher
& Rossing (plates/gongs for the métallique, strings for the palme), because there is no
ondes-specific modal measurement in any of the four sources.

The transducer is its own stage: early diffuseurs used a **moving-iron loudspeaker** whose
operating principle is *inherently nonlinear* — the Forum Acusticum paper's whole point is that
Thiele–Small does not apply, and it quantifies the nonlinearity on a heritage instrument. A
diffuseur that is only a resonator is missing a documented stage. Whether to model it is a
scope decision; **not** modelling it should be a stated limit rather than an omission.

Note for the header: hobbyist build pages give the palme 24 strings (two sets of 12). The
peer-reviewed source says 12. Prefer 12 and say why.

## Order of work

1. **`touche`** — fully specified, small, independently useful, and it can ship as
   `tap.touche~` before the rest of the instrument exists. Do this first.
2. **The diffuseurs** — also independently useful, and the modal machinery is familiar.
   `tap.palme~` and `tap.metallique~`.
3. **`triode`** — needs a listening comparison to settle the curve question.
4. **`source`** and the composition — last, because it is the cheapest piece and the one most
   constrained by the others.

That order deliberately front-loads the parts that are useful on their own, so the object
delivers value before the flagship is finished.

## What is still unsourced

- Modal data for either diffuseur specific to the instrument — falling back to Fletcher &
  Rossing for the general plate/string physics, which is a recreation rather than a model, and
  must be labelled as such.
- The waveform-register filter shapes. The circuit paper covers five stages but not the timbre
  registers in detail; Leipp (*Bulletin du GAM* n°60, 1972) and Laurendeau's monograph are the
  next places to look, neither yet obtained. If they do not settle it, the registers are a
  recreation voiced by ear and the header says so.
