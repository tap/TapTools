# Plan — the Radiohead family

> **Status: in progress — `tap.tapecho~`, `tap.stammer~` and `tap.fuzz~` shipped end-to-end,
> chapters included (2026-08-15); `tap.touche~`, the two diffuseurs and `tap.scrub~` shipped
> as kernels plus Max slices (2026-08-15/17), chapters still to come for those three.**
> `tap.ondes~`'s remaining pieces (the `triode` stage and the heterodyne `source`) are design.
> This is the drafting record of the
> 2026-08-15 survey ("are there Radiohead-inspired objects we should consider?"), amended the
> same day against the Eno components wave (`d4cf28a`) before any code was written. It stays
> after the objects ship, the plans-directory way; the chapters have their own drafting
> record in `PLAN-radiohead-chapters.md`. Per-object status lives in the table below.

Planning document for a family of kernels drawn from Radiohead's performed electronics:
the Ondes Martenot, the live Max/MSP mangling rigs, the tape echoes, the Kaoss-pad vocal
scrubbing, the ShredMaster-era fuzz. Kernel-first in this repo, Max wrappers and pin bumps
in TapTools-Max afterward, per the release flow.

The family's single thesis, the Eno-family way — one sentence every object serves:
**these are instruments, not processes.** Radiohead's electronics are played on stage in
real time — Jonny Greenwood performs the Ondes and his own Max patches live; the Kaoss pad
scrubs Thom's voice mid-song; the echo's regeneration is ridden like a fader. So in this
family the *performance surface* is the point: every parameter is a hand on the machine,
which makes the house no-zipper rule (per-sample ramps, allocation-free setters, safe while
audio runs) not just hygiene here but the actual feature. Where the Eno family's spine was
"degradation is the stability mechanism," this family's spine is "the control is the
instrument."

There is also a lineage claim that makes this family belong in a Max package specifically:
Greenwood's stutter rigs *are* Max patches — the band's documented live setup runs
Max/MSP. A Radiohead family in a Max package is not a tribute; it is a return.

## The family at a glance

| Object | Kernel | Recreates | Standing on | Status |
|--------|--------|-----------|-------------|--------|
| `tap.tapecho~` | `tapecho.h` | Multi-head tape echo (Copicat / Space Echo school) | `tape_loop.h` — almost pure composition | ✅ shipped 2026-08-15 (kernel, Max slice, chapters) |
| `tap.stammer~` | `stammer.h` | The live buffer-stutter rig (*Go To Sleep*, *The Gloaming*) | Original design; `tape::reel`, seeded rng | ✅ shipped 2026-08-15 (kernel, Max slice, chapters) |
| `tap.ondes~` | `ondes.h` + diffuseurs | The Ondes Martenot voice and its diffuseurs | Heterodyne source + triode nonlinearity + `garden.h` modal idiom; **not** `vco.h` — see the source hunt | planned — sources read, gate open, needs a design pass |
| `tap.fuzz~` | `fuzz.h` | Two-stage tone-stacked fuzz (the OK Computer-era dirt) | `overdrive.h` sibling; the DAFx-07 cascade | ✅ shipped 2026-08-15 (kernel, Max slice, chapters) |
| `tap.scrub~` | `scrub.h` | Kaoss-school granular scrub of live capture | `stammer::capture` (shared, not copied) + a Hann grain scheduler | ✅ shipped 2026-08-17 (kernel, notebook, Max slice) |
| `tap.metallique~` / `tap.palme~` | `diffuseur.h` | The Ondes diffuseurs as driven resonators | `garden.h`'s modal maths without its strike envelopes; `grm_comb.h`'s sustained resonance | ✅ shipped 2026-08-17 (kernel, notebook, Max slices) |

Parked (surveyed, deliberately not planned): a spectral freeze on the `stft.h` scaffold
(`tap.sustain~` and `tap.discreet~` now cover most of that ground between them), a Klatt
formant synthesizer (complete published spec — Klatt 1980, JASA — but a project of its
own), and a signal-rate bitcrush (trivial; nothing blocks it, nothing urgent about it).

## What the Eno wave changed (the amendments this plan bakes in)

The survey predated the Eno components wave by hours. Five amendments, now assumptions:

1. **`tap.tapecho~` is composition, not construction.** `tape_loop.h` already ships the
   reel (delay-line topology explicitly supported), the periodic wow/flutter transport
   citing the same tape-echo literature the survey cited, and `wear` (darken → bounded
   saturation → DC block) as an in-loop stage. The echo is a multi-head sibling of
   `discreet.h`, not a new machine.
2. **The Ondes diffuseurs inherit the garden's modal pattern.** Mode banks with published
   ratio tables (Fletcher & Rossing), doublet splitting, per-index deterministic scatter,
   per-mode decay — the scary half of `tap.ondes~` now has a shipped house idiom and a
   standalone component (`tap.chime~`) to model the shape on. *(Amended twice by the source
   hunt below: the resonator maths stands, but the diffuseurs are driven rather than struck,
   so the garden's strike envelopes do not carry over — and the voice is not a `vco.h`
   descendant at all.)*
3. **Components from day one.** The Eno components chapter's lesson — "the monoliths were
   monoliths by accident" — is prospective here: every kernel below is planned as parts
   with a thin composition, and the parts get C ABI + wrapper reachability from the start.
4. **The family randomness convention.** Anything stochastic (the stammer's dice, scrub
   jitter if any) draws from the seeded xorshift64* idiom so renders and tests reproduce
   bit-exactly and instances decorrelate by seed. The stammer inherits this wholesale.
5. **The delivery template exists.** Shared-machinery header where earned, kernels + C ABI
   + ctypes + executed notebooks, a `tools/render` listening binary (`radiohead_render`,
   the `eno_render` shape), book chapters with drafting records, full Max vertical slices.
   This plan mirrors that template rather than inventing one.

## Per-object plans

### 1. `tap.tapecho~` — the multi-head tape echo *(first; small)* — ✅ shipped

> **Shipped 2026-08-15**: `include/taptools/tapecho.h`, `tests/tapecho_test.cpp` (11
> scenarios), the C ABI + ctypes surface (`TapEcho`), the executed `notebooks/tapecho.ipynb`,
> and `tools/render/radiohead_render.cpp` with four performed scenarios; then the Max
> vertical slice in TapTools-Max (wrapper, six min-api scenarios, maxref, help patcher,
> pin bump — REVIVAL.md entry 18). What the plan predicted held: the kernel is composition
> — the null test below is *bitwise*, and `tape_loop.h` needed no changes at all to serve a
> second topology. The chapters shipped the same day (see `PLAN-radiohead-chapters.md`).
> Still to come: the on-Mac validation pass.
>
> Design decisions taken during implementation that this record should carry:
> the head layout is `span_ms` (the motor, = a ratio-1.0 head) times a per-head ratio, so
> four evenly spaced heads is the default and a three-head Copicat layout is set explicitly
> (the "preset + free" open question, resolved toward *free with an even-spacing default* —
> no spacings are claimed as measured from any unit); regeneration reaches
> `k_regen_max_driven` = 1.5 but is capped **per sample** back to 1.0 whenever drive is 0,
> since the saturator is the only thing bounding the past-unity regime.

The Watkins Copicat sits in Ed O'Brien's rig; the Space Echo school is all over the
catalog. The house delay family (`delay.h`, multitap, procrastinate) is clean digital;
this is the dirty one, and nearly all of it exists:

- One `tape::reel` in delay-line topology: advancing record head, **N read heads** at
  settable spacings (per-head level and pan, the multitap law). Motor speed is the master
  delay control, and a speed change glides as varispeed — the `discreet.h` contract:
  moving the head IS the doppler, no crossfading "digital" mode.
- `tape::wow_flutter` on the transport — periodic-only and deterministic, inherited as a
  *decision*, not an accident: the family keeps bit-exact renders; stochastic capstan
  drift stays a documented non-goal unless listening says otherwise.
- `tape::wear` in the regeneration path. The self-oscillation story must be stated in the
  header, `discreet.h`-style: with drive engaged, `swing_shape`'s 1/drive bound holds the
  loop absolutely bounded, so regeneration is *allowed* past unity into controlled
  sound-on-sound howl — the dub move, made safe by the same inversion `discreet.h` runs
  on. At drive 0 the fb cap falls back to the `delay.h` rule.

Head layout: four evenly spaced heads by default, every ratio freely settable underneath.
Tests, house patterns: an oracle measurement of wow — drive a sine through one head and read
the pitch deviation with `tap::dsp::yin` against the analytic transport excursion; and a null
test — wow 0, drive 0, regen 0 should collapse the echo to a plain multitap delay. *Both
landed, and the null is bitwise* (the interpolators do align — same family Hermite, same
fractional position, same equal-power pan law), measured in the kernel test and again across
the C ABI in the notebook. Measured at ship: per-pass generation loss within 0.2% of the
analytic wear transfer on both sides of the corner; wow 10.91 cents measured against 10.88
predicted; every past-unity drive setting plateaus under its analytic ceiling.

### 2. `tap.stammer~` — the live stutter rig *(second; the most "us")* — ✅ shipped

> **Shipped 2026-08-15**: `include/taptools/stammer.h` (the planned `capture` + `slicer` split
> under a thin `machine`), `tests/stammer_test.cpp` (9 scenarios), the C ABI + ctypes surface
> (`Stammer`), the executed `notebooks/stammer.ipynb`, and three more `radiohead_render`
> scenarios. The design decision worth carrying: the suite and the notebook both lean on a
> **pinned-dice identity** — with density 1, whole-step slices, one forward pass and no flank,
> the machine must reduce to *exactly* a one-step delay, bitwise. One identity pins the grid
> countdown, the slice origin and the playback head together, which is far stronger than
> testing three off-by-ones separately. The capture/slicer split is a component boundary for
> composition and testing only: a slicer needs a capture, so (like tapecho.h's `head`) it is
> honestly documented as not standalone-external material. Measured at ship: the identity holds
> bitwise; a seed replays bit-identically while a different seed changes 89% of samples; at
> density 0 the rng is provably untouched and the object is a bitwise bypass at any mix; and the
> material contract is measured at its premise — slices of a sustained sine are 1.000 alike by
> magnitude spectrum, slices of a plucked phrase 0.286. The Max vertical slice followed the
> same day (wrapper, five min-api scenarios, maxref, help patcher, pin bump — REVIVAL.md entry
> 19), and the chapters with it (see `PLAN-radiohead-chapters.md`). Still to come: the
> on-Mac validation pass.

The disintegrating guitar at the end of *Go To Sleep* and the mangling in *The Gloaming*
come from Greenwood's own Max patches: capture the live input, re-fire randomized slices
of it. An **original design** in the brassage tradition (Roads, *Microsound*) — no port,
no IP entanglement; the published record of the band's rig (interviews, the *From the
Basement* films) is behavioral reference only, nothing copied.

Two components and a thin composition:

- **capture** — a `tape::reel` recording the live input (shared shape with `tap.scrub~`
  below; whether it is literally one shared class is an open question).
- **slicer** — the dice and the playback: fire probability per quantum, quantized slice
  lengths (musical divisions of a settable base), repeat-count distribution, reverse
  probability, per-slice equal-power envelope width. Every continuous parameter rides a
  ramp; every random draw comes from the seeded xorshift64* so a seed is a performance you
  can replay. Dry/slice balance is the performance fader.

Tests: determinism per seed (bit-exact renders), the envelope's constant-power promise,
and a material contract stated in the header — the object is *for* transient material
(drums, struck guitar); on static pads it is just a tremolo, and the header says so.

### 3. `tap.ondes~` — the Ondes Martenot *(the flagship; gated)*

*How to Disappear Completely*, *The National Anthem*, *Where I End and You Begin*. The
instrument decomposes on exactly the family's seams, and the decomposition is the plan:

- **The voice**: a continuous-pitch oscillator (`vco.h` groundwork) with the waveform mix
  registers, driven by two performance signals — pitch (the ribbon; continuous, no
  quantization, glide is the playing technique not a parameter) and the **touche
  d'intensité**, the pressure key: a fast nonlinear-taper VCA whose response curve is the
  expressive heart of the instrument and must come from published measurement, not vibes.
- **The diffuseurs**, as standalone resonator components usable on *any* input — the
  killer feature is running a guitar through the Palme: **Métallique** (gong plate) as a
  modal bank per Fletcher & Rossing's plates/gongs chapters in the `garden.h` doublet
  idiom; **Palme** (sympathetic strings) as a bank of tuned string resonators
  (comb/waveguide school, `grm_comb.h` experience). Principal is the dry path.

**Gate: source collection before implementation.** The provenance rule is the whole
ballgame here. If a needed number has no published source, the honest fallback is a
*recreation* voiced by ear against published recordings and documented as such — decided
per-number, in the header, when we get there.

#### Source hunt, 2026-08-15 — findings, and the gate

**All three full texts are now in hand** (supplied manually — HAL's Anubis wall blocks
automated retrieval, and Unpaywall confirms HAL is the only OA host for all three, so there is
no mirror; the papers are open access, the wall is anti-automation, and it was not circumvented).

| Source | For | Status |
|--------|-----|--------|
| Quartier, Meurisse, Colmars, Frelat, Vaiedelich, "Intensity Key of the Ondes Martenot: An Early Mechanical Haptic Device", *Acta Acustica united with Acustica* **101**(2), 421–428, 2015, doi:10.3813/AAA.918837 | the touche d'intensité | **read in full** |
| Najnudel, Hélie, Roze, Boutin, "Simulation of an ondes Martenot circuit", *IEEE/ACM TASLP* **28**, 2651–2660, 2020 (HAL hal-02920526) | the circuit | **read in full** |
| Najnudel, Hélie, Roze, "Simulation of the Ondes Martenot Ribbon-Controlled Oscillator…", *JAES* **67**(12), 961–971, 2019, doi:10.17743/jaes.2019.0040 (HAL hal-02425249) | the variable oscillator | **read in full** |
| Wijnand, Boutin, Jossic, Maniguet, Forum Acusticum 2023 | the diffuseur transducer | read in full |

**The touche d'intensité is fully specified — this subsystem's gate is open.** The key is a
rheostat: a graphite/mica powder bag compressed by the key, resistance dropping as the number
of conducting bead paths rises (the carbon-microphone principle). Quartier et al. measured
force, displacement and sound simultaneously on instrument No. 320 and give the taper as a
table, reproduced here because it *is* the specification:

| dB_SPL | mean displacement (mm) | mean finger force (N) |
|--------|------------------------|------------------------|
| 45.0 | 4.3 (s.d. 0.15) | 0.39 (s.d. 0.06) |
| 53.3 | 5.3 (s.d. 0.19) | 0.47 (s.d. 0.07) |
| 61.6 | 5.9 (s.d. 0.15) | 0.52 (s.d. 0.07) |
| 70.0 | 6.4 (s.d. 0.15) | 0.62 (s.d. 0.08) |
| 78.3 | 6.8 (s.d. 0.12) | 0.82 (s.d. 0.11) |
| 86.6 | 7.3 (s.d. 0.08) | 1.34 (s.d. 0.11) |
| 95.0 | 8.8 (s.d. 0.05) | 9.60 (s.d. 0.30) |

Five things follow directly, and together they are the `touche` component's contract:

1. **4.5 mm of travel carries the whole 50 dB** (4.3 → 8.8 mm, background to maximum);
   playable gestures span roughly 3–9.5 mm.
2. **The map is memoryless.** The paper states explicitly that the change in sound intensity
   depends only on displacement and the force it implies, and *not on the velocity of the
   gesture* — so a static displacement→gain curve is not a simplification, it is the finding.
3. **The taper is not linear-in-dB against displacement.** The table's dB steps are equal by
   construction (six nuances over 50 dB) and the displacement steps are not: 1.0, 0.6, 0.5,
   0.4, 0.5, 1.5 mm. Interpolate the table; do not fit a straight line.
4. **Linear in dB against log(force)** over the quasi-linear region (below ~85 dB_SPL and
   1.3 N) — Weber–Fechner, and the paper's argument for why the key feels the way it does.
   Useful if a force-sensing controller is ever the input; displacement is the primary map.
5. **Pitch-independent.** Resistance curves for different notes have the same shape, so the
   key's law separates cleanly from the oscillator — one gain curve serves the whole range.
   (Amplitude does fall ~8 dB as frequency rises, but that is the oscillator and speaker, not
   the key.)

**The voice is a heterodyne pair, not a waveform-register VCO — amendment needed.** The plan
assumed an oscillator with timbre switches built on `vco.h`. Najnudel et al. model instrument
No. 169 as **five coupled stages**: fixed-frequency oscillator (80 kHz), variable-frequency
oscillator, demodulator, preamplifier, power amplifier, coupled through transformers. Two
findings matter more than the topology:

- **The oscillators are essentially pure.** Even coupled to the rest of the circuit, oscillator
  output measures about **0.03 % second-harmonic distortion** — which is precisely the
  simplification the authors use to reach real time. So the tone does *not* come from an
  interesting oscillator waveform.
- **The timbre comes from downstream.** Harmonics are generated by the **two successive triode
  stages** after the demodulator, and then the **diffuseur** "converts the electrical waveform
  into sound and in turn modifies its spectral content". Their plugin even exposes demodulator
  input gain as a harmonics control — a knob the real instrument does not have.

So the kernel's shape should be: a clean sinusoidal source (heterodyne difference tone, and the
ribbon paper is the reference for the variable oscillator's tuning behaviour), into a
**triode-flavoured nonlinearity**, into the **touche** gain curve, into the diffuseurs. That is
much closer to `overdrive.h`/`fuzz.h` territory than to `vco.h`, and it is a different object
than the one this plan sketched. Note also that their full PHS simulation runs at 768 kHz and
their plugin consumes 85 % of a laptop CPU core — a faithful circuit solve is *not* the route
for a TapTools kernel; the documented simplifications are.

**Diffuseurs, unchanged from the earlier note:** driven, not struck — *métallique* a
motor-excited gong (1944–45, patented 1947), *palme* an electromagnet driving **12** strings on
a soundboard (1949–50), *résonance* motor-excited springs (1970s). The early transducer is a
moving-iron loudspeaker and inherently nonlinear (Thiele–Small does not apply). Widely
circulated DIY pages say the palme has 24 strings; the peer-reviewed source says 12 — prefer
the peer-reviewed number and treat hobbyist build documentation as unciteable here.

**Where the gate stands: open enough to build.** The touche is fully specified. The voice has a
published decomposition and, more usefully, a published *justification for simplifying it*. The
diffuseurs have their excitation and their transducer characterized, though the resonator mode
data still has to come from Fletcher & Rossing rather than from an ondes-specific source. The
remaining work is design, not sourcing — and the object it points at is not the one originally
sketched, so `tap.ondes~` needs a design pass against these findings before implementation.

### 4. `tap.fuzz~` — the two-stage fuzz *(small, parallel-friendly)* — ✅ shipped

> **Shipped 2026-08-15**: `include/taptools/fuzz.h` (`stage` + `tone` under a thin `pedal`),
> `tests/fuzz_test.cpp` (9 scenarios), and the C ABI + ctypes surface (`Fuzz`). The name
> question closed: **`tap.fuzz~`**, generic, per the trademark posture below.
>
> The method is Yeh, Abel & Smith's DAFx-07 *simplified cascade* — conditioning filter →
> memoryless nonlinearity → equalization filter, twice — which is a stronger footing than
> the plan assumed: it supplies the architecture, the justification for a static curve
> (the diode limiter's exact ODE is a lowpass whose pole moves with voltage), the curve
> family (tanh), and the reason `asymmetry` exists (a real op-amp stage clips
> asymmetrically, producing the even harmonics an odd-only model cannot).
>
> **Two defects found by measurement, both worth carrying:**
> 1. *Gain staging.* The first cut had the second stage fully clipped at `gain` 0 — the
>    knob did nothing above about 0.2 — because the tanh family's small-signal slope is
>    `knee/tanh(knee)` (~3 at the original knee) and that multiplied into a fixed ×2.2.
>    Retuned; the harmonic ratio now sweeps 0.010 → 0.358 across the knob.
> 2. *The house oversampler is not steep enough here — and steepening it was not the whole
>    story.* With the 4th-order Butterworth that `tap.ladder~` / `overdrive.h` use, alias
>    energy at 4× came out worse than at 2× (1.7e-2 vs 2.8e-3). Eighth order improves 4× by
>    ~6× but does **not** make the sequence monotone: measured, 1×/2×/4×/8× run
>    1.2e-1 / 2.7e-5 / 7.4e-4 / 1.8e-3, so 2× is best and is now the default. An earlier
>    draft of this record claimed 8th order "restored monotonicity" — it does not, and the
>    notebook plot is the correction. The cause is open: the obvious suspect (ill-conditioned
>    biquads at low normalized cutoffs) was tested and **ruled out** by an impulse-response
>    check; the untested hypothesis is imaging, which would point at cascaded 2× resampling
>    as the real fix. Whether `overdrive.h` is owed the 8th-order change is a separate live
>    question needing its own measurement.
>
> Two test-design errors were also caught and are recorded in the suite itself, since both
> are easy to repeat: an alias test whose tone divided the sample rate (every fold lands on
> a harmonic and is invisible), and probe frequencies close enough to the fundamental to
> measure window leakage rather than aliasing. Still to come: the notebook, a render
> scenario, the Max vertical slice, and the chapter.

The OK Computer-era dirt (*Paranoid Android*, *My Iron Lung*). A circuit-informed
recreation from the widely published schematic — cascaded clipping stages plus its
characteristic tone stack — with the diode-clipper modeling literature (the Yeh/Abel/Smith
DAFx line) behind the solver choices. Direct sibling of `overdrive.h`; the TR-808 kernels
are the house precedent for schematic-based recreation. **Naming is an open question**
(below): the garden precedent (Bloom → garden) says don't ship a live trademark;
"ShredMaster" is Marshall's mark, and the header will cite it as provenance either way.

### 5. `tap.scrub~` — the Kaoss school *(after stammer; shares its capture)* — ✅ shipped

> **Shipped 2026-08-17**: `include/taptools/scrub.h` (`head` + `machine`),
> `tests/scrub_test.cpp` (13 scenarios), the C ABI + ctypes surface (`Scrub`), the executed
> `notebooks/scrub.ipynb`, two `radiohead_render` scenarios, and the Max slice.
>
> **The sharing is literal**, as planned: the tape is `stammer::capture` itself, not a second
> copy of it, and the only addition the stutter needed was `read_frac` — the fractional Hermite
> read its ±1-rate slices never used. The plan's "position and speed as signals" became
> **position and pitch**, which is the stronger claim: on tape, moving the head *is* the pitch
> change, and decoupling them is what makes this an instrument rather than a varispeed.
>
> **The load-bearing null**: Hann overlap-adds to exactly 1 at hop = size/2, so held still at
> unity pitch with no spray the scrub is the input delayed, to 4.4e-16.
>
> **One real defect, found by measurement and worth carrying.** The first cut anchored every
> grain at the position. Origins then advance at the *write head's* speed while each grain plays
> at `rate`, so the transposition applies only inside a grain, the average read rate returns to
> 1, and a steady tone comes out at its **original** pitch with a comb of grain-rate sidebands.
> The pitch knob did nothing but add texture — and no other test on the page could see it. The
> fix is a phase-continuous read head wrapped back toward the position only after it has
> wandered ±1.5 grains, a bound chosen by sweep (band energy retained 0.933 / 0.958 / 0.965 /
> 0.990 / 0.993 at ±0.5 / ±1 / ±2 / ±3 / ±4 grains; flat past 3, and every extra grain is a
> grain of position error).
>
> **And a measurement warning.** A single-bin probe reads the fixed kernel as badly broken: the
> wraps spread the transposed partial into a comb a few Hz wide, and a rectangular-window
> Goertzel on one line saw 0.02 where the band figure was 0.43. Measured properly, 98.8 % of a
> perfect shifter's energy lands within ±15 Hz of the transposed pitch (worst 91.7 %); what the
> wraps cost is concentration — 92.0 % as focused as a clean shift, 75.0 % at worst. Measure the
> band, not the bin.

### 6. The diffuseurs — `tap.metallique~` and `tap.palme~` — ✅ shipped

> **Shipped 2026-08-17**: `include/taptools/diffuseur.h` (`mode`, `plate`, `sympathetic`,
> `harp`, `transducer`, and the two cabinets over a shared `cabinet` base),
> `tests/diffuseur_test.cpp` (18 scenarios), the C ABI + ctypes surface (`Metallique`, `Palme`,
> and the bare `Plate` / `Transducer` components), the executed `notebooks/diffuseur.ipynb`,
> two `radiohead_render` scenarios, and both Max slices. Design record in `PLAN-ondes.md`.
>
> Three things the build settled. **The order is the argument**: the transducer drives the body,
> so the nonlinearity is upstream of the resonator, and a null test pins that the cabinet is
> exactly `transducer -> body` (bitwise) while the reverse wiring differs by 28 % of peak.
> **Unit peak gain per mode** (Steiglitz's constant-peak-gain resonator) plus weights that sum
> to exactly 1 makes the body bounded by its input with no limiter and no DC blocker — the
> zeros at ±1 handle DC and Nyquist. **The transducer's bound is 2/saturation, not
> 1/saturation**: a hard-driven squared law is a nearly-constant positive waveform, and removing
> its DC doubles the worst-case swing (measured 1.49 against the naive 1.25).
>
> Honest about what it is: the bodies are **recreations of the general physics** (Fletcher &
> Rossing's free circular plate, and the harmonic series), because no ondes-specific modal
> measurement exists in any of the four sources; the string tuning is a design choice; and both
> nonlinear coefficients are voiced by ear, since the source establishes *that* the moving-iron
> driver is nonlinear without handing over a curve.

## Cross-cutting commitments

- **Kernel-first, components-first.** Every object lands as kernel parts + composition in
  this repo with C ABI and ctypes bindings extended alongside (`tools/capi`,
  `notebooks/taptools_py.py`), executed notebooks for every measured claim, then the Max
  vertical slices (wrapper, `docs/` maxref, `help/` patcher, runtime maxtest) and the
  submodule pin bump in TapTools-Max.
- **`radiohead_render`** in `tools/render`: minutes-long listening checks per object, the
  `eno_render` shape — the echo into self-oscillation and back, a seeded stammer
  performance, the diffuseurs rung by a struck string.
- **Oracle-based measurement** where a promise is audible: yin on the echo's wow, yin on
  the ondes ribbon glide, envelope-power measurement on the stammer slices.
- **Book**: a family part — shipped as **Part V, *The machines you ride*** (the title the
  family thesis earned), with *Four heads and a motor* and *The part that comes apart* plus
  their machine appendices; drafting record in `PLAN-radiohead-chapters.md`. Later objects
  join the same part. Every number cites an executed cell or pinned test.

## Provenance and naming (the IP posture, applied)

- **Published-literature-only for new DSP**, per the house policy: the echo stands on the
  tape-echo modeling literature already cited in `tape_loop.h`; the fuzz on a published
  schematic plus the clipper-modeling literature; the diffuseurs on Fletcher & Rossing;
  the ondes voice is gated on locating its published measurements. The stammer and scrub
  are original designs in the granular literature's tradition.
- **Nothing is copied from anyone's patch, preset, or firmware.** Greenwood's Max rigs
  are known from published interviews and broadcast films; they inform *what the object
  is for*, never what the code says. Same posture as `garden.h` toward Bloom.
- **Trademark care in names**, the Bloom → garden precedent: no `kaoss` (Korg), no
  `shredmaster` (Marshall), no song-title names that read as endorsement. "Ondes" is the
  generic French word and the instrument's common name; "tapecho", "stammer", "scrub",
  "fuzz" are generic English.

## Order, and why

1. **`tap.tapecho~`** — smallest distance from shipped code, and it stress-tests
   `tape_loop.h` as the library the components chapter claims it is. ✅ *Kernel done; the
   stress test passed — the shared machinery needed no changes to serve a second topology.*
2. **`tap.stammer~`** — original design (no sourcing gate), highest Max-lineage
   resonance, establishes the family's capture + seeded-performance conventions. ✅ *Done; both
   conventions are now in place for `tap.scrub~` to inherit.*
3. **`tap.ondes~`** — the flagship; source collection starts immediately (it
   parallelizes with 1–2), implementation begins when the gate clears.
4. **`tap.fuzz~`** — small and independent; slots into any gap.
5. **`tap.scrub~`** — after the stammer, so the capture component is designed once with
   both consumers in view.

## Open questions

- **The fuzz's name.** `tap.fuzz~` (generic, safe, dull) vs `tap.shred~` (generic English
  word, but adjacent to the mark) vs something from the family's own metaphor. Decide
  before the vertical slice; the kernel header name can follow.
- ~~**Echo head layout.**~~ Resolved at ship: free ratios with a nominal even-spacing
  default (0.25 / 0.5 / 0.75 / 1.0), rather than a named-machine preset — no head spacings
  are claimed as measured from any unit, and a Copicat-style three is two lines to set.
- ~~**One capture component or two.**~~ Resolved at the scrub's ship: **one**. `scrub.h`
  includes `stammer.h` and uses `stammer::capture` directly; the only change the stutter needed
  was a fractional read it does not itself call.
- **`tap.pitchaccum~` has the same warble, and worse.** Measured on the same sweep the scrub was
  measured on (5 fundamentals × 7 intervals, band energy retained around the transposed pitch):
  the scrub returns mean 0.988 / worst 0.917, `tap.pitchaccum~` returns mean 0.908 / **worst
  0.004** — a near-total cancellation at 311 Hz up 19 semitones, where its ratio is exactly 3
  and the two taps land a half-window apart. That is a real finding about a shipped object,
  recorded rather than acted on: fixing it is its own job, with its own tests and its own
  consumers, and it should not ride along on an unrelated kernel.
- **Chapters for the three newest objects.** `tap.touche~`, the diffuseurs and `tap.scrub~` have
  kernels, notebooks and Max slices but no book chapters yet. The touche's belongs inside an
  Ondes-family chapter once the `triode` and `source` exist; the diffuseurs' probably with it;
  the scrub's belongs beside the stammer in Part V, since they share a tape.
- **Diffuseur delivery.** Ship the resonators inside `tap.ondes~` only, or as standalone
  externals (`tap.palme~` / `tap.metallique~`) from day one? The components chapter's
  lesson leans standalone-from-day-one.
- **Stochastic transport, ever?** The family inherits periodic-only wow/flutter. If the
  echo's listening checks say the grot is missing, the amendment is a *family* decision
  (it breaks bit-exact renders) — flagged now so it is never a quiet local hack.
