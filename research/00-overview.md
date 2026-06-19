# TapTools Book — Research Overview & Shape Options

*Synthesis of the three research digs. Start here.*

This is the cover sheet for the `research/` dossier assembled to answer one question: **what shape should a book about TapTools take?** The honest answer from the research is that the material does not force a single shape — but it strongly *favors* a few, and rules others out. This document lays out what we found, the cross-cutting themes, and 3 candidate shapes with a recommendation.

Companion files:
- [`01-object-inventory.md`](01-object-inventory.md) — code archaeology: every `tap.*` object, what it does, families, provenance, story potential.
- [`02-history-timeline.md`](02-history-timeline.md) — version-by-version history, the 2014–2020 git tail, the collaborator network, the missing pre-2014 era.
- [`03-ecosystem-context.md`](03-ecosystem-context.md) — Jamoma, Cycling '74, 74 Objects, Timothy Place's career, collaborator bios. (Web access was partial; facts are confidence-tagged.)

**Archival-recovery digs (search-driven, filling the pre-2014 gap):**
- [`04-ttblue-jamoma-lineage.md`](04-ttblue-jamoma-lineage.md) — the DSP-library spine: TTBlue (= "TapTools Blue"), the Jade→Jamoma origin story, license chain, 9 citable papers.
- [`05-commercial-era-hipno.md`](05-commercial-era-hipno.md) — the commercial years: Silicon Prairie → Electrotap → 74 Objects, the registration model, Hipno, Darwin Grosse, the Teabox.
- [`06-community-forums-recovery.md`](06-community-forums-recovery.md) — community footprint: the McGill Max listserv archive, forum threads, reception, and a 26-item Wayback target list.
- [`07-revival-second-life.md`](07-revival-second-life.md) — **the 2026 revival** (act 4: resurrection); summarizes `REVIVAL.md` (authored by Tim Place, on `main`).
- [`08-git-history-chronicle.md`](08-git-history-chronicle.md) — the complete git history across all branches: the 2014 "great pruning," the four modernization attempts, the authorship arc, the pre-2014 gap.
- [`09-deep-web-recovery-round2.md`](09-deep-web-recovery-round2.md) — deeper search recovery: Jade, version-by-version reception, Hipno/Pluggo, the Teabox, interviews, current footprint.

**Bridge artifacts (to unlock what this environment can't reach):**
- [`10-source-checklist.md`](10-source-checklist.md) — consolidated, prioritized primary-source checklist for a **local / unrestricted-network** pass (Wayback + fetch), with what to capture and which gaps each item closes.
- [`11-interview-guide.md`](11-interview-guide.md) — structured prompts for the **pre-2014 texture only Tim remembers** (the perishable memory-capture; 30 prompts across the four acts).

> **Note on web access:** the Wayback Machine and most direct page-fetches are blocked from this environment; **WebSearch** carried the recovery and surfaced page *content* via snippets. Search-derived facts are confidence-tagged in each file. A future Wayback-enabled or forum-logged-in pass should work the target list in `06-`.

---

## What TapTools actually is (the one-paragraph version)

TapTools is a **~25-year arc of Max/MSP externals** (1999–present) by **Timothy Place**, distributed under **74 Objects LLC**, built on the DSP library the same author was writing underneath it — **TTBlue (literally "TapTools Blue") → Jamoma → jamoma2**. It began commercial (registration keys, paid installer, the Tap.Tools 1/2/3 era under Silicon Prairie Intermedia, then Electrotap), entwined with the **Jamoma** open-source framework born in Norway (lifted from Place's "Jade" environment at Trond Lossius's suggestion "over a meal of shrimp," BEK Bergen Oct 2003, launched March 2005), and spun off the commercial **Hipno** plug-in suite (Cycling '74, NAMM 2005, built on Pluggo). It went open-source under BSD "on the honor system" (2013), then **shrank** over 2014–2020 as Max and Jamoma absorbed its functions — the author recording each retirement in candid, funny commit-message eulogies. **And then, in 2026, he brought it back from the dead** — cutting the now-dead Jamoma loose, rebuilding all ~48 objects on the modern Max SDK, and reinventing the lost ones (see [`07-revival-second-life.md`](07-revival-second-life.md)).

## The three strata of source material

The research surfaced **four** distinct, unevenly-documented layers — this unevenness is itself the central planning fact:

| Stratum | Period | How well documented | Where it lives |
|---|---|---|---|
| **The deep history** | 1999–2013 (Tap.Tools 1–3, commercial era, Hipno, Electrotap, Jamoma birth) | **Partly recovered** — not in git, but search recovered names/dates/anecdotes | `ReadMe.txt` changelog (v3.0+); digs `04`–`06`; Wayback target list (un-mined); **author memory** |
| **The codebase** | c. 2000–2015 (stratified geology of 3 coding eras) | **Richly** — 50 externals + ~24 abstractions + 70 ref pages + 78 help patchers | This repo (`legacy` branch) |
| **The retirement epilogue** | 2014–2020 (git history, 52 commits) | **Vividly** — first-person commit narrative | `git log` |
| **The revival (act 4)** | 2016 (C74 `taptools-min`) → 2026 (full rebuild) | **Exhaustively + in real time** | `main` branch + **`REVIVAL.md`** (author-written); dig `07` |

**The key asymmetry (now improved):** the codebase, the 2014+ epilogue, and the 2026 revival are exhaustively documented. The first ~13 years — the part that makes this a *story* rather than a *catalog* — the search digs partially recovered (the Jade/shrimp/BEK origin, the Hipno deal, business lineage, dates), but the *texture* (what it felt like, the negotiations, the Kansas City years) still lives mainly in your head. Memory-capture remains the perishable task; it's now scoped by the open-questions lists, not a blank page.

## Cross-cutting themes the research keeps hitting

These recur across all three digs and are the real "spine" candidates:

1. **"Max ate my object" — and then the maker un-ate them.** The lifecycle engine of the whole suite: TapTools fills a gap in Max/MSP, thrives, then is retired when Cycling '74 ships the feature natively (`average~`, `degrade~`, `atodb~`/`dbtoa~`, `buffer~ normalize`, `zl`, `gen~`, Max 6 Projects). Then in 2026 the framework *it* stood on (Jamoma) is the thing that dies, and the author rebuilds the suite without it. An arc about building tools on a platform that's chasing you — with a redemptive turn.
2. **The author and the substrate are the same person.** TapTools is the application/proving-ground layer for Place's own DSP library (TTBlue→Jamoma→jamoma2). The "one-line external" (`wrapTTClassAsMaxClass`) is the punchline of years of library design. Few toolkits let you tell both stories at once.
3. **Consolidate, then un-consolidate.** Merged all externals into one monolithic extension in 3.0; re-split into per-object binaries one version later in 3.1. A clean, teachable architecture-decision story.
4. **Commercial → open-source → hobby.** Registration keys and paid installers → BSD on the honor system → volunteer maintenance. The Hipno lineage ("originally called tap.hipnopp", "jesse's objects") is the commercial DNA bleeding into the free release.
5. **A first-person retirement narrative — with a sequel.** The git log reads like eulogies the author wrote while pruning: *"the low-res LFO is dead… we aren't trying to squeek out performance on G3 processors anymore," "a bad idea that is finally going away," "goodbye forever. you won't be missed," "bringing it back from the dead."* And now `REVIVAL.md` is the author narrating the resurrection in real time. This first-person voice across *both* the death and the rebirth is the book's most distinctive, hardest-to-replicate asset.
6. **Resurrection & the unreliable narrator (NEW).** The 2026 port forced a close reread of 15-year-old code and found real bugs the originals shipped (`tap.random~`, `tap.buffer.snap~`, `tap.fft.normalize~`, `tap.comb~`); lost objects (`tap.vocoder~`, `tap.nr~`, `tap.spectra~`) had to be *reinvented* from docs because no source survived; and a deleted Cycling '74 port (`taptools-min`) was salvaged. The author auditing his younger self is a rich seam (dig `07`).
7. **The naming aesthetic.** Objects are puns/metaphors, not labels: *elixir* (a mix), *procrastinate~* (a delay), *inquisitor* (introspection), *sift~/sieve* (filtering), *gang*, *snap*. A whole minor-key chapter sits here.
8. **The human network.** Momeni (tap.jit.ali, NIME'03), Stephan Moore (tap.rotate), Pinkston (tap.random~/counter~), jhno (limi~), Zicarelli (tap.comb~), Dudas (the now-deleted list externals), Grosse (the `tap.plug.*`→Hipno connector), Lossius (the Jamoma pivot) — TapTools sits at the crossing of Cycling '74, the Jamoma project, and academic computer music.

## What the material rules in and out

- **Ruled IN by abundance:** descriptive/technical content (every object is documented and source-available), the technical-substrate story (Jamoma/wrapper), and the 2014+ retirement narrative.
- **Ruled IN but at-risk:** the deep historical narrative (1999–2013) — possible, but only if memory is captured; the repo alone can't supply it.
- **Ruled OUT / weak:** a pure exhaustive reference manual — it would mostly duplicate the existing `.maxref` pages and help patchers (which already do that job well, and ~half the objects are "routine"). The book's value is *not* in re-documenting `tap.split~`.

---

## Three candidate shapes

### Shape A — "Fieldbook" (descriptive / algorithmic, object-led)
A curated tour of ~15–20 of the most interesting objects, each a short chapter: the problem it solved, the algorithm/DSP idea, the implementation, and the story behind it. Skips the routine utilities.
- **Best supported by:** the codebase dig — `tap.verb~`, `tap.svf~`, `tap.jit.colortrack`, `tap.fft.binmodulator~`, `tap.procrastinate~`, `tap.filecontainer`, `tap.jit.ali`.
- **Strength:** lowest-risk; the material is fully in-repo and recoverable today. Useful to practitioners.
- **Weakness:** risks being a catalog; the connective tissue (why these existed, the arc) has to be supplied or it's just annotated source.

### Shape B — "Stratified" (historical / narrative, time-led)
The story of a 25-year toolkit and its maker, told chronologically across the **four acts**: the commercial Tap.Tools/Electrotap years; the Jamoma entanglement & Hipno; the open-source turn and long retirement; and the 2026 resurrection. Objects appear as *evidence*, not as the organizing principle.
- **Best supported by:** themes #1–8; the history + recovery digs; the commit-log voice; `REVIVAL.md`.
- **Strength:** uses the single most distinctive asset (the first-person voice across both death and rebirth + your dual role as author-of-tool-and-library). Now has a *complete* arc — beginning, middle, false ending, true ending.
- **Weakness:** still wants pre-2014 *texture* from memory, but the recovery digs have de-risked the facts/dates.

### Shape C — "Hybrid: a memoir in objects"  ★ recommended
Chronological four-act spine (Shape B) with deep-dive "object chapters" (Shape A) interleaved as set pieces. E.g. the Hipno chapter carries `tap.gang`/`tap.jit.sum`; the Jamoma chapter carries the wrapper pattern and `tap.verb~`; the retirement chapter is the literal git-log pruning; the **revival chapters pair each rebuild/reinvention with its original** (the author meeting his younger self, object by object).
- **Best supported by:** all seven digs together.
- **Strength:** plays to every strength found; flexible; degrades gracefully where deep history is thin (lean on object + revival chapters). The revival supplies a present-tense engine and a natural ending.
- **Weakness:** more to structure; needs an editorial hand to keep the modes from fighting.

### Shape D — "Resurrection / modernization" (engineering-led, NEW option)
A focused book on *bringing a 25-year-old C++/Max codebase back to life*: cutting a dead framework dependency, porting to Min/CMake/CI, reinventing lost DSP, reconciling a rival port — with the history as backstory. Closest to what `REVIVAL.md` already documents.
- **Best supported by:** dig `07` + the codebase digs.
- **Strength:** maximally concrete and recoverable (it's happening now); strong appeal to a software-engineering / audio-dev audience; least dependent on memory.
- **Weakness:** narrower audience; risks losing the human/historical richness that makes the larger story special.

**Recommendation:** aim for **C (four-act memoir in objects)**. The revival changed the calculus: the narrative is no longer at-risk-of-thinness at the end (it has a vivid, self-documenting act 4), and the recovery digs de-risked the beginning. C now plays to *every* strength. If a tighter, faster book is wanted, **D** is the strong fallback and could even be written *first* (it's the most recoverable) and later expanded into C.

---

## Suggested next steps (in order)

1. **React to the shapes** (A / B / **C** / D — or a blend).
2. **Memory-capture pass — still the perishable task, now scoped not blank.** The recovery digs supplied the facts; what's left is *texture*. I can turn the consolidated open-questions lists (across digs `03`–`07`: exact Tap.Tools 1.0 date/price, the Hipno negotiation as you lived it, your C74/Ableton dates, the Kansas City years, the identities of "jhno"/"jesse", the shrimp-meal scene) into a **structured interview guide** you answer in prose or voice — primary source for acts 1–2.
3. **Mine the Wayback target list (phase-2 research) from a less-restricted environment.** The Wayback Machine is blocked *here*; dig `06` produced a prioritized 26-item list (74objects.com / electrotap.com / SourceForge TTBlue snapshots, dead forum threads, the Cycling '74 Tim Place interview). Best run by you locally or from a different network.
4. **Reconcile the object universe** — but note dig `07`/`REVIVAL.md` has *already* largely done this (compiled / abstractions / retired / reinvented / resurrected lists, with a per-object port log). A clean appendix-ready index is now mostly a consolidation job.
5. **Pick the pilot chapter to feel the voice.** Strong candidates given the revival: a **paired "then/now" chapter** (e.g. `tap.verb~` original vs. the 2026 Moorer-style rebuild), the **git-log retirement chapter**, or the **`taptools-min` salvage** (Cycling '74 quietly maintaining and dropping it).
6. **Decide how the book relates to the live revival.** It's happening now; the book can document act 4 as reportage. Worth an explicit call (see dig `07` open questions), including whether the AI-assisted archaeology/rebuild is itself part of the story.

*Nothing here commits us. The dossier is the evidence; the shape is yours to call.*
