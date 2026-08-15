# Plan — the Radiohead family

> **Status: in progress — `tap.tapecho~`'s kernel has landed (2026-08-15); the rest is
> plan.** This is the drafting record of the 2026-08-15 survey ("are there
> Radiohead-inspired objects we should consider?"), amended the same day against the Eno
> components wave (`d4cf28a`) before any code was written. It stays after the objects ship,
> the plans-directory way; per-chapter drafting records will follow separately when the book
> chapters are drafted. Per-object status lives in the table below.

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
| `tap.tapecho~` | `tapecho.h` | Multi-head tape echo (Copicat / Space Echo school) | `tape_loop.h` — almost pure composition | ✅ kernel shipped 2026-08-15; Max wrapper + chapter pending |
| `tap.stammer~` | `stammer.h` | The live buffer-stutter rig (*Go To Sleep*, *The Gloaming*) | Original design; `tape::reel`, seeded rng | planned |
| `tap.ondes~` | `ondes.h` + diffuseurs | The Ondes Martenot voice and its diffuseurs | `garden.h` modal idiom, `vco.h`/`vca.h` | planned — gated on source collection |
| `tap.fuzz~` (name open) | `fuzz.h` | ShredMaster-school two-stage fuzz | `overdrive.h` sibling, published schematic | planned |
| `tap.scrub~` | `scrub.h` | Kaoss-school granular scrub of live capture | `tape::reel` + the `grm_pitchaccum.h` grain engine | planned |

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
   standalone component (`tap.chime~`) to model the shape on.
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

### 1. `tap.tapecho~` — the multi-head tape echo *(first; small)* — ✅ kernel shipped

> **Shipped 2026-08-15**: `include/taptools/tapecho.h`, `tests/tapecho_test.cpp` (11
> scenarios), the C ABI + ctypes surface (`TapEcho`), the executed `notebooks/tapecho.ipynb`,
> and `tools/render/radiohead_render.cpp` with four performed scenarios. What the plan
> predicted held: the kernel is composition — the null test below is *bitwise*. Still to
> come: the Max wrapper (`tap.tapecho~` vertical slice) in TapTools-Max, and the book
> chapter. Design decisions taken during implementation that this record should carry:
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

### 2. `tap.stammer~` — the live stutter rig *(second; the most "us")*

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
ballgame here. Candidate sources to verify and pin: the published organological studies of
the instrument (there is published work specifically measuring the touche d'intensité's
response — locate and verify it), Fletcher & Rossing for both diffuseurs, and period
technical descriptions of the waveform registers. If a needed number has no published
source, the honest fallback is a *recreation* voiced by ear against published recordings
and documented as such — decided per-number, in the header, when we get there.

### 4. `tap.fuzz~` — the ShredMaster school *(small, parallel-friendly)*

The OK Computer-era dirt (*Paranoid Android*, *My Iron Lung*). A circuit-informed
recreation from the widely published schematic — cascaded clipping stages plus its
characteristic tone stack — with the diode-clipper modeling literature (the Yeh/Abel/Smith
DAFx line) behind the solver choices. Direct sibling of `overdrive.h`; the TR-808 kernels
are the house precedent for schematic-based recreation. **Naming is an open question**
(below): the garden precedent (Bloom → garden) says don't ship a live trademark;
"ShredMaster" is Marshall's mark, and the header will cite it as provenance either way.

### 5. `tap.scrub~` — the Kaoss school *(after stammer; shares its capture)*

Live *Everything In Its Right Place*: the voice sampled on the fly and scrubbed, reversed,
smeared from a pad. A continuously recording `tape::reel` with a **performable granular
playhead**: position and speed as signals (the pad is two axes — that is the Max-side
mapping story), Hermite grains from the `grm_pitchaccum.h` engine, overlap and grain size
exposed. Distinct from `tap.reel~` (free-running loop, no performable head) and from
`tap.pitchaccum~` (transposition, not scrubbing); shares machinery with both, and the plan
is to make that sharing literal, not copied.

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
- **Book**: a family part (working titles — *The machine as a band member*; per-object
  chapters like *The tape with three heads*, *The patch that stutters*, *Waves and
  wire*) plus machine appendices, each with its own PLAN drafting record when drafted,
  every number citing an executed cell or pinned test.

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
   resonance, establishes the family's capture + seeded-performance conventions.
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
- **One capture component or two.** Stammer and scrub both record the live input into a
  reel; whether that is one shared class with two heads of use, or two thin wrappers over
  `tape::reel`, is a design call to make when the stammer lands.
- **Diffuseur delivery.** Ship the resonators inside `tap.ondes~` only, or as standalone
  externals (`tap.palme~` / `tap.metallique~`) from day one? The components chapter's
  lesson leans standalone-from-day-one.
- **Stochastic transport, ever?** The family inherits periodic-only wow/flutter. If the
  echo's listening checks say the grot is missing, the amendment is a *family* decision
  (it breaks bit-exact renders) — flagged now so it is never a quiet local hack.
