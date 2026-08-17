# Plan — `tap.ondes~`, after reading the sources

> **Status: complete as an object — every piece has shipped.** `touche` 2026-08-15 as
> `tap.touche~`; both diffuseurs 2026-08-17 as `tap.metallique~` and `tap.palme~`; the `triode`
> and the heterodyne source 2026-08-17 as `tap.triode~` and `tap.ondes~`. The book chapters
> followed the same day — `book/src/ondes.md` (voice, triode and intensity key together) and
> `book/src/diffuseurs.md`, plus `machine/ondes.md` and `machine/diffuseur.md`. What remains is
> the waveform registers, which are still unsourced — a second hunt on 2026-08-17 failed to
> close the gate but left two concrete leads (see the last section).
> The source gate is closed —
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

## `touche` — ✅ shipped

> **Shipped 2026-08-15**: `include/taptools/touche.h`, `tests/touche_test.cpp` (11 scenarios),
> the C ABI + ctypes surface (`Touche`), the executed `notebooks/touche.ipynb`, and a
> `radiohead_render` scenario putting the curve against the two laws you would otherwise reach
> for. The seven published points come back to within 6e-5 dB.
>
> Two implementation notes worth carrying. **The normalized domain is the physical travel, not
> the measured band** — an early cut mapped 0..1 onto 4.3–8.8 mm, which put position 0 exactly
> on the first published point *and* returned silence there, contradicting the measurement. The
> paper puts playable gestures at roughly 3–9.5 mm with the measured band inside, so position
> now spans 9.5 mm and the bottom 45 % is genuinely silent (the key's first phase is bending
> before it reaches the powder bag). **And the dead zone belongs in the lookup, not the table**:
> zeroing dense-table entries below the floor put a cliff next to it, so a query landing on the
> floor lerped toward zero and read 4.2 dB low. Both were caught by the reproduce-the-table
> test, which is exactly what that test is for.
>
> Still to come for this piece: the Max vertical slice and a chapter (probably folded into an
> Ondes-family chapter once more of the instrument exists, rather than one chapter per part).

## The design (as written before implementation)

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

## `triode` — where the timbre actually is — ✅ shipped

> **Shipped 2026-08-17** as `tap.triode~`, in `include/taptools/ondes.h`.
>
> **The open question resolved itself on a closer read, and in the best possible direction.** The
> plan framed it as a choice between "a published grid-conduction curve" and "the tanh family with
> an asymmetry bias". It is neither, because the circuit paper does not merely *mention* a tube
> model — it names one (the **enhanced Norman Koren** model: Koren, *Glass Audio* 8(5), 1996, with
> Cohen & Hélie's grid-current extension, AES 129, 2010), writes out its equations, and publishes
> parameter sets **fitted to the actual valves in ondes No. 169** in its Table II, alongside the
> supply voltages and cathode resistors of every stage. So there was nothing to voice by ear and
> nothing to choose: the whole stage is a citation.
>
> A stage is then the static solution of `ipc(vpc, vgc) = (Vbias − Vk − vpc)/Rp` on its load line
> — a memoryless nonlinearity in the DAFx-07 sense, so tabulating it is not an approximation of
> the model, it *is* the model. The published operating points bias sanely (6C5 demodulator:
> Vk 2.70 V, Vp 86.5 V, Ip 2.70 mA, gain 4.86) and the curve is strongly asymmetric — a 2.17:1
> ratio between the two directions at ±4 units — which is where a triode's even harmonics live.
>
> Two things the build added to the plan. **The stage must invert**, and the sign is load-bearing
> rather than cosmetic: the tube's asymmetry acts on whichever side of the waveform actually
> reaches its grid, and a stage that quietly un-inverted itself applied the curve to the wrong
> side. An early cut did exactly that, and its drive knob *reduced* harmonics as it was turned up.
> **And the demodulator's grid-leak detection inverts too** — a growing envelope drives that grid
> toward cutoff — so the two inversions put the demodulator's plate in phase with the envelope
> while its curve has meanwhile acted on the underside.
>
> The gain-staging lesson from `fuzz.h` was applied from the start and held: output is normalized
> by each stage's own small-signal gain, so `drive` sweeps total harmonic content 0.221 → 0.344
> monotonically without the level running away.

## `source` — cheap, and deliberately so — ✅ shipped, and the plan was wrong about how

> **Shipped 2026-08-17** as `detector` inside `include/taptools/ondes.h`, and composed into
> `tap.ondes~`.
>
> **The plan's central instruction here was a mistake, and catching it is the most valuable thing
> this build did.** "Synthesize the difference tone **directly** as a sinusoid" would have thrown
> away the instrument's single largest source of harmonics. The paper's 0.03 % figure and its
> "replace with a sinewave generator" licence apply to the **oscillators**, not to the
> demodulator — and the demodulator is not a mixer that hands you a difference tone. It is an
> envelope detector, and the envelope of `cos(Φ) + cos(Φ − φ)` is `2|cos(φ/2)|`, whose Fourier
> series puts H2 at −14.0 dB, H3 at −21.3 dB and H4 at −26.4 dB. All of that exists before any
> valve touches the signal.
>
> **What replaces the carrier is better than a simplification: it is an identity.** For amplitudes
> 1 and `depth` the envelope is exactly `sqrt(1 + depth² + 2·depth·cos φ)`, so the 80 kHz carrier
> drops out of the arithmetic rather than being approximated away. Running the published RC
> detector (200 µs, from R4·C21) on that closed form reproduces the full heterodyne-plus-diode-
> plus-RC simulation to **within 0.10 dB on every harmonic at every pitch tried**, with one
> systematic difference — a uniform 3.0–3.2 % level offset, because a follower chasing real
> carrier half-cycles never quite reaches the peak between them. The detector's characteristic
> pitch dependence comes along free: H2 runs −14.0 dB at A2 to −19.3 dB at A6 and the level falls
> 2.0 dB across those five octaves, all out of the same 200 µs.
>
> A bonus the plan did not anticipate: because the closed form is parameterized by the two
> oscillator amplitudes, **oscillator balance becomes a real physical timbre control**. At `depth`
> 1 the envelope closes and the series is full; below that it never closes and the harmonics thin
> out. That is a mismatch between two real oscillators, not an invented knob.
>
> The ribbon law is the circuit paper's Eq. 7, and it is simple: `f = A1 · 2^(d/12·d0)` with
> A1 = 55 Hz. **The ribbon is linear in semitones**, which is exactly why an ondes glissando
> sounds the way it does, and why `set_ribbon` takes semitones rather than Hz.

## The diffuseurs — driven, not struck — ✅ shipped

> **Shipped 2026-08-17**: `include/taptools/diffuseur.h`, `tests/diffuseur_test.cpp` (18
> scenarios), the C ABI + ctypes surface (`Metallique`, `Palme`, plus the bare `Plate` and
> `Transducer` components), the executed `notebooks/diffuseur.ipynb`, the `metallique_stages`
> and `palme_halo` render scenarios, and both Max vertical slices.
>
> Everything below survived contact with the code. The one thing the design pass did not say,
> and the build made explicit, is that **the order is a claim worth a null test**: the
> transducer drives the body, so the cabinet must be exactly `transducer -> plate`, bitwise,
> and the reversed wiring must measurably differ (it does — 28 % of peak). Two smaller findings:
> the modal bank needs no limiter and no DC blocker at all, because Steiglitz's
> constant-peak-gain resonator has unit peak gain at any Q and its zeros at ±1 null DC and
> Nyquist exactly; and the transducer's output bound is **2/saturation, not 1/saturation**,
> because taking the DC out of a hard-driven squared law doubles the worst-case swing.
>
> The transducer question the plan left open — model the nonlinearity or state its absence —
> was answered by modelling it: the squared law is defensible from the moving-iron principle
> alone, and it is measured against its own prediction (second harmonic at exactly
> asymmetry × amplitude / 2, to 1.4e-4, with nothing at the third). The bounding saturator
> after it is labelled what it is: a modelling necessity, not a measured stage.
>
> Still open, and stated in the header rather than hidden: no radiation or cabinet model, no
> soundboard resonance, and no string stiffness (a real steel string's partials stretch sharp;
> a delay loop's are exactly harmonic).

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
2. ~~**The diffuseurs**~~ — ✅ shipped 2026-08-17 as `tap.metallique~` and `tap.palme~`.
3. ~~**`triode`**~~ — ✅ shipped 2026-08-17 as `tap.triode~`. No listening comparison was needed:
   the circuit paper publishes the tube model *and* parameter sets fitted to the instrument's own
   valves, so there was no curve to choose.
4. ~~**`source`** and the composition~~ — ✅ shipped 2026-08-17 as `tap.ondes~`.

That order deliberately front-loads the parts that are useful on their own, so the object
delivers value before the flagship is finished.

## What is still unsourced

- Modal data for either diffuseur specific to the instrument — falling back to Fletcher &
  Rossing for the general plate/string physics, which is a recreation rather than a model, and
  must be labelled as such.
- The waveform-register filter shapes. **This is the only thing standing between `tap.ondes~`
  and a complete instrument**, and the header states its absence rather than filling it with
  invention. A second source hunt ran 2026-08-17 and **did not close the gate**; what it found
  is below, so the next attempt starts further along rather than repeating it.

  *Confirmed to exist, not obtainable from here:*
  - **Leipp, "Les ondes Martenot", *Bulletin du GAM* n°60, April 1972.** The citation is real —
    it appears in an academic bibliography, and the Catgut Acoustical Society Library holds 45
    GAM issues (1963–1978) as a physical archive. Nothing is digitized anywhere reachable. This
    needs a library request, not a search.
  - **Laurendeau, *Maurice Martenot, luthier de l'électronique* (1990).** A print monograph; not
    obtained.

  *Ruled out as sources for this:*
  - The TASLP circuit paper, read in full: five stages, and the registers are not among them.
  - Its companion, "Simulation of the Ondes Martenot **Ribbon-Controlled Oscillator**"
    (HAL hal-02425249) — the title is the scope.

  *New lead, and the best one: the patents.* Martenot's 1928 "Perfectionnements aux instruments
  de musique électriques" and **FR 841.128 (February 1939)**. Patents are exactly the right class
  of source here — published, long expired, schematic-bearing, and IP-clean in a way a blog never
  is. Google Patents and Espacenet both refused to serve this environment (503 / access denied),
  so they remain unread. **Try these first next time, from a machine that can reach them.**

  *And the trap to avoid.* Hobbyist and encyclopedic descriptions of the register *waveforms* are
  abundant and broadly consistent — creux as a peak-limited triangle, gambe as a pulse at roughly
  35/65 duty, nasillard as a very narrow pulse, octaviant as an added octave, petit gambe as
  gambe lowpassed, souffle as noise, feutré as a softening filter. Every one of those is a blog,
  a forum, a retailer's history page, a performer's site or a replica manual; none is published
  literature and **none gives a filter shape, a corner, or a component value**. Implementing from
  them would break the published-literature-only policy *and* the promise the header makes, to
  buy a register set that would be guesswork wearing the instrument's vocabulary. Not done, on
  purpose.
- **Where the intensity key sits in the chain.** The paper's five stages do not include it, so
  `tap.ondes~` offers both readings as a switch (`key_placement`): after the valves it is a clean
  output law, before them the dirt comes up with the pressure. Measured, the difference is real
  (THD 0.311 against 0.222 at a half-press with drive 6), so it is a choice worth exposing rather
  than a detail to guess at.
- **The winding sense of the transformer between the two triode stages.** It decides which side of
  the waveform the preamplifier's asymmetry acts on, and it is audible — THD 0.274 against 0.394 —
  so it is a switch too (`polarity`).
