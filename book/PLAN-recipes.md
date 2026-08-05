# Plan — the Recipes part

> **Status: drafted.** The part opener and all ten recipes are written and live in
> `src/recipes/` per the placement below (2026-08-05). This file remains as the drafting
> record, the plans-directory way. The improvement findings the drafting surfaced live in
> `plans/recipes-improvements.md` in the TapTools-Max repo (the design-of-record for object
> changes); new recipes now wait on new objects.

Planning document for Part IX of *Tools on Tap*: **Recipes** — the book's third kind of
chapter. Parts I–VII say what each object is for; Part VIII says why to trust it; a recipe
puts several objects on one patch cord and chases a specific, named sound.

## Placement in SUMMARY.md

A new part after the machine part:

```md
# Part IX — Recipes

- [How to read a recipe](recipes/cookbook.md)
- [One machine, four decades](recipes/808-classics.md)
- [Three oscillators into a ladder](recipes/minimoog.md)
- [The patches with names on them](recipes/moog-classics.md)
- [Move a knob while it loops](recipes/acid-line.md)
- [The ostinato machine](recipes/sequenced-modular.md)
- [The robot on the radio](recipes/robot-voice.md)
- [The staircase and the wash](recipes/shimmer.md)
- [Sixteenths into a listening filter](recipes/funk-filter.md)
- [A field guide to rooms](recipes/rooms.md)
- [Chords with no keyboard](recipes/comb-drones.md)
```

The introduction's organization list gains a matching Part IX bullet.

## The rules a recipe is held to

Stated in the part opener (`recipes/cookbook.md`), enforced in drafting:

1. **Every knob named exists, spelled as the attribute is spelled.** Recipes were drafted
   against the wrapper sources in TapTools-Max (`source/projects/`), not from memory — that
   pass is what caught, e.g., that the snare has no `decay` (its tail rides `tone`), the
   clap's tail knob is `tail`, the sequencer programs via `hits`/`accents`/`velocities`/
   `step` (there is no `steps` message), `tap.ladder~`'s `mode`/`solver` are numeric
   indices at the Max layer, and `tap.adsr~` gates on signal level > 0.5 (so a
   `tap.808.seq~` impulse at the default levels will not open it).
2. **Settings are starting points; measurements are citations.** A recipe's knob values are
   ears' work and say so. Any *measured* number a recipe leans on (the kick's ~49 Hz
   fundamental, the ladder's THD-vs-drive walk, sequencer polymeter) is borrowed from an
   executed notebook or pinned test and cited, never re-derived — the "measured, not
   remembered" promise unchanged.
3. **Provenance stays honest.** Documented production history (who used the machine) is
   stated as such; pattern grids are labeled starting points, not transcriptions; and no
   recipe claims to *be* a record — mix, room, tape, and hands are out of the box.
4. **Ingredients are ranked.** Every recipe ends with the vco-chapter-style ordered list of
   what each element buys, so cutting from the bottom is a stated option.

## Shipped recipes

- **`recipes/808-classics.md` — One machine, four decades.** Four kits off one
  `phasor~`/`tap.808.seq~` scaffold: "Planet Rock" electro (1982), "Sexual Healing" slow
  soul (1982), Miami bass (the tuned long-decay kick as bassline), trap (half-time,
  polymeter hat-roll rows via `length 24`/`length 32`). Evidence borrowed:
  `tr808_calibration.ipynb` (kick fundamental, calibration residuals), `step_seq.ipynb`
  (grid accuracy, polymeter), the drums chapter's pinned roll/choke behavior.
- **`recipes/minimoog.md` — Three oscillators into a ladder.** Completes the oscillator
  chapter's Moog recipe into a playable monosynth voice: stack → ladder → `tap.vca~`, two
  `tap.adsr~` contours (with the period-correct release-switch note), gate/pitch plumbing,
  and bass + lead settings tables. Evidence borrowed: `vco.ipynb` and `ladder.ipynb` via
  their chapters.
- **`recipes/moog-classics.md` — The patches with names on them.** The voice above driven
  at four records as deltas from its bass/lead tables: Winwood's "While You See a Chance"
  hook, Worrell's "Flash Light" stacked-Minimoog bass, Wright's "Shine On" lead, and
  Emerson's "Lucky Man" modular solo. Carries the answer to "do we need a Moog modular
  object?" — no: a modular is routing freedom, and Max is the patch panel; the modules
  already ship (`tap.vco~`/`tap.ladder~`/`tap.adsr~`/`tap.vca~`/`tap.noise~` + the
  sequencer pair). Gear provenance is stated per patch (documented vs. reconstruction).

## The second wave (shipped 2026-08-05)

All seven backlog recipes landed in one pass, each drafted against a fresh wrapper-source
sweep (the audit found and fixed a shipped-chapter drift along the way: the pitchaccum
chapter's `pitch1`/`feedback1` spellings are actually `trans1`/`fb1`, feedback on a 0–99
scale — corrected in `src/pitchaccum.md`):

- **`recipes/acid-line.md` — Move a knob while it loops.** The Phuture method: a 16-step
  line (grids + the `pitches`/`gates`/`accents`/`slides` lane messages), the knob rides,
  accent runs into the measured ×1.94 C13 bloom, `tap.overdrive~` after.
- **`recipes/sequenced-modular.md` — The ostinato machine.** Berlin school / "I Feel
  Love": `tap.303.seq~` → `mtof~` → external slew → the Moog voice; transpose as harmony;
  the honest wrinkle that the vco's signal inlet bypasses `smooth` (→ improvements plan).
- **`recipes/robot-voice.md` — The robot on the radio.** Carrier casting (saw pair + 10 %
  noise as the sibilance budget), the three settings rows (talk/choir/rhythm-transfer),
  the left-inlet-is-modulator debugging fact. Extended with the songbook: "In the Air
  Tonight"'s VP-330 ghost choir, the front-and-center robots (ELO/Styx/Beasties/
  Kraftwerk), the talkbox distinction, "Hide and Seek" honestly labeled a harmonizer
  (with the `tap.shift~` + `tap.semitone2ratio` stack as the closer route), and an
  Orange-school plugin-era carrier — built as our own VA voicing, with the house rule
  against reverse-engineering shipping products stated in print.
- **`recipes/shimmer.md` — The staircase and the wash.** The full Eno-school chain:
  pitchaccum spiral (+12/+7) into `tap.verb~` or a convolved church; damping as the
  make-or-break; descent, micro-halo, and morph-gesture variants.
- **`recipes/funk-filter.md` — Sixteenths into a listening filter.** Clav chop, bass
  quack (factory slot 2), cocked wah (slot 4), the sidechain and envelope-outlet patch
  points; honest Mu-Tron distancing per the autowah chapter.
- **`recipes/rooms.md` — A field guide to rooms.** IR curation for `tap.convolve~`:
  shopping list, sixty-second audition drill, placement (`predelay` first, `blocksize` by
  role, `set` for performance swaps).
- **`recipes/comb-drones.md` — Chords with no keyboard.** Five `tap.5comb~` voicings as a
  keepable table (factory, open fifth, just major, dark cluster, √2 bell plate), ringing
  techniques, the eight-second morph gesture.

**The audit's first shipped object (2026-08-05):** the songbook's "Hide and Seek" finding
("the mechanism is a formant-corrected harmonizer and no object provides it") became
`tap.harmony~` — kernel `taptools/harmonizer.h` on the DspTap pvoc/LPC substrate, seven
oracle-based test scenarios, capi + bridge, wrapper with reference page and help patcher —
and **`recipes/choir-of-one.md`** documents it: the instrument (corrector → harmonizer),
the Bon Iver worked examples ("Woods" as stacked chapel with the overdub-honesty note;
"715 - CRΞΞKS" as the Messina-school live stack), craft notes, and the robot-vs-choir fork
back to the vocoder chapter. The executed verification notebook shipped the same day
(`notebooks/harmonizer.ipynb` — 0.04-cent interval accuracy, 3.7e-8 dry-alignment
residual, the formant-centroid measurement — cited by the recipe). A proper object
chapter (Part VI territory) remains future work.

Other new recipes wait on new objects (or on the improvements plan landing — e.g. the vco
performance section would simplify the Moog chapters' vibrato plumbing).

## Notes for drafting

- Titles follow the book's voice: image first, no object name in the title.
- Pattern grids are `text` code blocks, 16 columns with a step-number header; `X` accented,
  `x` plain, `.` rest — the two-level scheme matches the sequencer's `plain`/`accented`
  hardware model, which is the idiom recipes should teach first (per-step `velocities` is
  the documented escape hatch).
- Tempo math is stated once in the scaffold section (`phasor~` frequency = BPM ÷ 240 for a
  16-step bar) and reused.
- Figure candidates (not yet drawn): the shared drum-kit scaffold as a proper signal-flow
  SVG; the Moog voice wiring diagram in the house block-diagram style (`book/figures/`).
- If a wrapper surface changes (e.g. `tap.ladder~` ever grows symbolic `mode` values), the
  recipes are the pages most likely to silently rot — re-check them against the wrappers
  when bumping the TapTools-Max side.
