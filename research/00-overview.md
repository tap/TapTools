# TapTools Book — Research Overview & Shape Options

*Synthesis of the three research digs. Start here.*

This is the cover sheet for the `research/` dossier assembled to answer one question: **what shape should a book about TapTools take?** The honest answer from the research is that the material does not force a single shape — but it strongly *favors* a few, and rules others out. This document lays out what we found, the cross-cutting themes, and 3 candidate shapes with a recommendation.

Companion files:
- [`01-object-inventory.md`](01-object-inventory.md) — code archaeology: every `tap.*` object, what it does, families, provenance, story potential.
- [`02-history-timeline.md`](02-history-timeline.md) — version-by-version history, the 2014–2020 git tail, the collaborator network, the missing pre-2014 era.
- [`03-ecosystem-context.md`](03-ecosystem-context.md) — Jamoma, Cycling '74, 74 Objects, Timothy Place's career, collaborator bios. (Web access was partial; facts are confidence-tagged.)

---

## What TapTools actually is (the one-paragraph version)

TapTools is a **~15-year personal toolbox of Max/MSP externals** (c. 1999/2000–2015) by **Timothy Place**, distributed under **74 Objects LLC**, built on the DSP library the same author was writing underneath it (**TTBlue → Jamoma → jamoma2**). It began commercial (registration keys, paid installer, the Tap.Tools 1/2/3 era under Silicon Prairie Intermedia, then Electrotap), shared DNA with the commercial **Hipno** plug-in suite, and ended open-source under BSD "on the honor system," distributed as a Max Package. The suite *shrank* over its life as Max and Jamoma absorbed its functions — and the author recorded each retirement in candid, funny commit messages.

## The three strata of source material

The research surfaced three distinct, unevenly-documented layers — this unevenness is itself the central planning fact:

| Stratum | Period | How well documented | Where it lives |
|---|---|---|---|
| **The deep history** | 1999–2013 (Tap.Tools 1–3, commercial era, Hipno, Electrotap) | **Sparsely** — the formative decade is *not in git* | `ReadMe.txt` changelog (v3.0+ only), web archives (mostly unreachable), **author memory** |
| **The codebase** | c. 2000–2015 (stratified geology of 3 coding eras) | **Richly** — 50 externals + ~24 abstractions + 70 ref pages + 78 help patchers | This repo |
| **The retirement epilogue** | 2014–2020 (git history, 52 commits) | **Vividly** — first-person commit narrative | `git log` |

**The key asymmetry:** the codebase and the 2014+ epilogue are exhaustively documented and recoverable by anyone. The first ~13 years — the part that would make this a *story* rather than a *catalog* — survive mainly in your head. Any historical book is therefore partly an act of memory-capture against a deadline, and that should drive sequencing (see Next Steps).

## Cross-cutting themes the research keeps hitting

These recur across all three digs and are the real "spine" candidates:

1. **"Max ate my object."** The lifecycle engine of the whole suite: TapTools fills a gap in Max/MSP, thrives, then is retired when Cycling '74 ships the feature natively (`average~`, `degrade~`, `atodb~`/`dbtoa~`, `buffer~ normalize`, `zl`, `gen~`, Max 6 Projects). A genuine arc about building tools on a platform that's chasing you.
2. **The author and the substrate are the same person.** TapTools is the application/proving-ground layer for Place's own DSP library (TTBlue→Jamoma→jamoma2). The "one-line external" (`wrapTTClassAsMaxClass`) is the punchline of years of library design. Few toolkits let you tell both stories at once.
3. **Consolidate, then un-consolidate.** Merged all externals into one monolithic extension in 3.0; re-split into per-object binaries one version later in 3.1. A clean, teachable architecture-decision story.
4. **Commercial → open-source → hobby.** Registration keys and paid installers → BSD on the honor system → volunteer maintenance. The Hipno lineage ("originally called tap.hipnopp", "jesse's objects") is the commercial DNA bleeding into the free release.
5. **A first-person retirement narrative.** The git log reads like eulogies the author wrote while pruning: *"the low-res LFO is dead… we aren't trying to squeek out performance on G3 processors anymore," "a bad idea that is finally going away," "goodbye forever. you won't be missed," "bringing it back from the dead."* This voice is the book's most distinctive, hardest-to-replicate asset.
6. **The naming aesthetic.** Objects are puns/metaphors, not labels: *elixir* (a mix), *procrastinate~* (a delay), *inquisitor* (introspection), *sift~/sieve* (filtering), *gang*, *snap*. A whole minor-key chapter sits here.
7. **The human network.** Momeni (tap.jit.ali, NIME'03), Stephan Moore (tap.rotate), Pinkston (tap.random~/counter~), jhno (limi~), Zicarelli (tap.comb~), Dudas (the now-deleted list externals) — TapTools sits at the crossing of Cycling '74, the Jamoma project, and academic computer music.

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

### Shape B — "Stratified" (historical / narrative, time-led)  ★ recommended
The story of a 15-year toolkit and its maker, told chronologically: the commercial Tap.Tools/Electrotap/Hipno years, the Jamoma entanglement, the open-source turn, and the long retirement as Max grew up underneath it. Objects appear as *evidence* in the narrative, not as the organizing principle.
- **Best supported by:** themes #1–5 above; the history dig; the commit-log voice.
- **Strength:** uses the single most distinctive asset (the first-person retirement narrative + your dual role as author-of-tool-and-library). Tells a story no reference manual can. Has a real arc with a beginning, middle, and a poignant end.
- **Weakness:** **depends on the at-risk pre-2014 memory.** This is the shape that most needs you as a primary source, soonest.

### Shape C — "Hybrid: a memoir in objects"  (recommended fallback / merge)
Chronological spine (Shape B) with deep-dive "object chapters" (Shape A) interleaved as set pieces. E.g. the Hipno chapter carries `tap.gang`/`tap.jit.sum`; the Jamoma chapter carries the wrapper pattern and `tap.verb~`; the closing chapter is the literal git-log pruning, object by object.
- **Best supported by:** all three digs together.
- **Strength:** plays to every strength we found; flexible; degrades gracefully if some memory is unrecoverable (lean harder on the object chapters where history is thin).
- **Weakness:** more to structure; needs an editorial hand to keep the two modes from fighting.

**Recommendation:** aim for **C (with a B backbone)**. The research says the *narrative* is what makes this more than a manual, but the *objects* are where the recoverable, concrete, verifiable material is. C lets the strong codebase material carry the weight wherever the deep history turns out to be thin — and we won't know how thin until we test your memory against the timeline.

---

## Suggested next steps (in order)

1. **React to the shapes.** Pick a direction (or push back — maybe it's something we haven't named, e.g. a pedagogical "how Max externals are built" book using TapTools as the worked example).
2. **Memory-capture pass — do this early regardless of shape.** The pre-2014 history is the perishable asset. Even for Shape A, the *why* behind the objects lives only with you. Concretely: I can turn the `02-` timeline and the `03-` open-questions list (12 items: business dates, pricing, the dropped Windows port, the 3.3–3.5 version gap, the identities of "jhno"/"jesse", etc.) into an **interview guide** — a structured set of prompts you answer in prose or voice, which becomes primary source.
3. **Close the web gaps (phase 2 research).** `web.archive.org` was blocked this session; the 74objects.com / electrotap.com snapshots and the Cycling '74 "Interview with Tim Place" are the highest-value un-fetched sources. Retry from a less-restricted network, or you may have local copies.
4. **Reconcile the object universe.** The repo describes *three* object lists — 50 compiled externals, ~24 shipped abstractions, ~20+ retired-in-git. A definitive annotated index reconciling them is a useful artifact for any shape (and a likely appendix).
5. **Pick the pilot chapter.** Whichever shape we choose, draft one chapter to feel the voice. Strong candidates: `tap.verb~` (technical set-piece) or the git-log retirement chapter (narrative set-piece).

*Nothing here commits us. The dossier is the evidence; the shape is yours to call.*
