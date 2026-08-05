# Plan — the Recipes part

> **Status: drafted.** The part opener and the first three recipes are written and live in
> `src/recipes/` per the placement below (2026-08-05). This file remains as the drafting
> record, the plans-directory way, and carries the backlog of future recipes.

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

## Backlog — future recipes, roughly in order of pull

- **The acid line.** `tap.303.seq~` patterns that exploit the measured couplings: accent
  runs into the C13 bloom (the ×1.94 wow), slide chains, `envmod`/`decay` interplay.
  Mostly written already in spirit across the acid chapter; the recipe is patterns.
- **The robot voice.** `tap.vocoder~` driven properly: carrier choice (saw stack vs.
  noise blend), band count trades, the sibilance path.
- **Shimmer.** `tap.pitchaccum~` + `tap.verb~`/`tap.convolve~` — the accumulating-fifths
  pad, with the honest feedback-headroom accounting.
- **The funk envelope filter.** `tap.autowah~` against the calibrated hardware curves;
  clav and bass settings.
- **Borrowed rooms, curated.** A short IR field guide for `tap.convolve~` — what to load,
  true-stereo vs. mono-in, pre-delay by trimming.
- **The five-string drone.** `tap.5comb~` tunings as chord recipes.
- **The sequenced modular.** Berlin school and "I Feel Love": `tap.303.seq~` → `mtof~` →
  the vco stack, gate → `tap.adsr~` — the scaffold is sketched at the top of
  `moog-classics.md`; the recipe is patterns, ostinato transposition, and the hat-groove
  glue.

Each lands as one file in `src/recipes/` plus a SUMMARY line; no renumbering needed.

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
