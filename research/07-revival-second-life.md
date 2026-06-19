# The 2026 Revival — TapTools' Second Life (book material)

*Research note. Primary source: **`REVIVAL.md`** authored by Timothy Place, living on the **`main`** branch of `tap/taptools` (read 2026-06-19; ~37 KB working document, "Status as of 2026-06-18"). This file summarizes it as book material and should be re-synced from `main` before drafting, since it is a living document.*

> **Why this matters for the book:** the earlier digs framed TapTools as a story that *ends* in elegiac retirement — the git-log eulogies of 2014–2020 as Max absorbed each object. The revival overturns that ending. There is now a **fourth act: resurrection.** This is the single biggest change to the book's possible shape since the research began, and it is the strongest argument yet for a narrative spine (Shapes B/C).

## What the revival is

A 2026 effort to bring TapTools back to life on a modern toolchain, consolidated into `main` (the pre-revival state is preserved on the `legacy` branch). Per `REVIVAL.md`, the repo we first explored was really **three half-finished migrations stacked on each other**: the 2013 monolithic→modular move on old Jamoma, the 2014 prune + vendored subtrees, and an abandoned 2015 attempt to rewrite objects on jamoma2 (which stalled after one object — the `tap.fourpole~` we found).

### The decisive move: cut Jamoma loose
The framework the whole suite stood on — TTBlue → Jamoma → jamoma2 — is dead/dormant. The revival's locked-in decision was **not** to drag the corpse along but to **reimplement every object's DSP as portable standalone C++**, with **Min** (`min-api` + `max-sdk-base`, actively maintained) as a thin, swappable wrapper and **no dependency on `min-lib`**. Standard: C++20. Targets: **macOS universal (arm64+x86_64) + Windows**. Build: **CMake + GitHub Actions**, replacing the dead Ruby/Xcode/Travis stack. The old Jamoma `Core/`, `ttblue`, `objectivemax`, `max-sdk`, `build.rb`, and the i386/x86_64-only `.mxo` binaries have been pruned (preserved in git history).

### Scope of work completed (per the progress log)
- **All ~48 core objects** across Tiers 1–3 ported and compile-verified on the Min/Max SDK toolchain.
- **All 5 Jitter objects** ported (matrix→value via the Jitter API; the matrix→matrix `tap.jit.kernel` rebuilt on Min's `matrix_operator`, fixing an original off-by-one OOB write).
- **Lost objects reinvented from scratch** — the spectral trio `tap.vocoder~`, `tap.nr~`, `tap.spectra~` had *no surviving source* (they were `pfft~` abstractions over un-ported helpers); each was reimplemented as a self-contained external with its own STFT, unit-tested for reconstruction.
- **Objects resurrected from docs** — `tap.delay~` / `tap.delay` rebuilt from their surviving `.maxref.xml` pages (no source survived).
- **New unified object** — `tap.filter~` (RBJ multimode biquad), the standalone replacement for the old Jamoma filter flagship that had superseded `tap.onepole~/twopole~/fourpole~`.
- **Test coverage** — Catch unit tests (24/24 green) + the Cycling '74 `max-test` harness vendored for in-Max runtime validation.

### The outstanding gap
**Runtime validation in a live Max** is the headline remaining work — everything is compile- and unit-tested against a mock kernel but not yet auditioned for sound (esp. the reinvented spectral trio, `tap.verb~` oversampling, `tap.sustain~`, the filters/delays).

## The `taptools-min` subplot (a gift to the narrative)
The revival surfaced a **second, previously-unknown Min port**: `taptools-min`, an **official Cycling '74 effort** (`github.com/Cycling74/taptools.git`, 2016–2019) that was **deleted upstream** and now survives only as the `taptools-min` branch of this repo. It held 7 Min-ported objects; the revival had independently re-ported 6 of them, so a per-object diff decided winners (the revival's versions won on every contested object, usually because the C74 port carried bugs or depended on `min-lib`). Its one genuinely unique object — **`tap.sustain~`** (capture audio → trim to zero-crossings → crossfaded sustaining loop) — was rescued and rebuilt multi-voice. *Cycling '74 quietly maintained TapTools for a few years and then dropped it; the author found the abandoned port and salvaged it.* That is a chapter on its own.

## Latent bugs found in the originals (the unreliable-narrator thread)
Porting forced a close reread of 15-year-old code and turned up real bugs the originals shipped with: `tap.random~` (per-vector vs per-sample edge test), `tap.buffer.snap~` (a post-clamp loop that could never terminate), `tap.fft.normalize~` (a 0.49-biased equality that disabled DC/Nyquist halving), `tap.comb~` (undefined-when-unset feedback/decay coupling). For a book this is gold: the author auditing his younger self.

## How the revival reshapes the book's arc

The four acts now available:
1. **Genesis & commerce** (1999–2004) — Tap.Tools under Silicon Prairie/Electrotap; the registration-key era; Jade; TTBlue ("TapTools Blue").
2. **The Jamoma entanglement & Hipno** (2003–2009) — Norway/BEK, the shrimp-meal pivot, the open-source framework, the commercial plug-in suite built on Pluggo, Cycling '74.
3. **Open source & the long retirement** (2013–2020) — BSD honor system, the suite shrinking as Max grows, the git-log eulogies.
4. **Resurrection** (2016 C74 port → 2026 revival) — cutting the dead framework loose, rebuilding on modern foundations, reinventing what was lost, salvaging the abandoned port. *Present tense — the author is doing it now, and writing the book is part of the same act of return.*

This is a complete, satisfying narrative shape with a beginning, a middle, a false ending, and a true ending — and it's **self-documenting**, because `REVIVAL.md` is the author narrating act 4 in real time.

## Implications / open questions
- **The book and the revival are intertwined.** Writing the book *while* doing the revival means the final chapters can be reportage, not just memory. Decide whether the book documents the revival as it happens (a "now" thread) or treats it as a closed final chapter.
- **`REVIVAL.md` is canonical primary source** for act 4 — keep this note synced from `main`.
- **Branch ecology as artifact:** `main`, `legacy`, `taptools-min`, `windows`, `gh-pages`, and several `claude/*` working branches are themselves evidence of the revival's method (and of AI-assisted archaeology being part of the story — worth deciding whether the book acknowledges that).
- Does the revival change the audience calculus? It adds a "modernizing a legacy C++/Max codebase" angle that a practitioner/engineering audience would value.
