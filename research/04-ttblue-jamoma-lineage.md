# TapTools — TTBlue / Jamoma / jamoma2 Lineage (Research Notes)

Research phase 04. Purpose: document the **DSP-library and framework lineage UNDER
TapTools** — **TTBlue → Jamoma DSP → jamoma2** — as published, external,
citable history. This is the "shoulders" that TapTools stands on (the ReadMe.txt
explicitly thanks Jamoma and ObjectiveMax). Aimed at supporting a book.

Confidence tags used below:
- **[snippet — verify]** — drawn from a WebSearch synthesized snippet, not a fetched
  primary page; treat phrasing as paraphrase until confirmed.
- **[repo-confirmed]** — confirmed from files in this local TapTools repo.
- **[fetched]** — confirmed via a successful WebFetch of a live (non-archive) page.
- **[inferred]** — reasonable inference, not directly stated.

---

## 0. Web-access status (read this first)

- **WebSearch worked well** and is the workhorse for this phase. Almost every external
  fact below comes from synthesized WebSearch snippets with cited source URLs.
- **WebFetch mostly blocked**, but **succeeded for `github.com/jamoma/jamoma2`** this
  session (see §4). Expect failure on most other hosts.
- **web.archive.org (Wayback) is HARD-BLOCKED** — not used.
- **`redmine.jamoma.org`** (the old Jamoma DSP wiki, the canonical TTBlue/JamomaDSP
  documentation) appears in search results but is likely offline; its content here is
  reconstructed from search snippets and must be re-verified later, ideally via Wayback.
- **GitHub MCP tools are scoped to `tap/taptools` only** — I could NOT pull commit
  history for `jamoma/jamoma2`, `jamoma/JamomaCore`, etc. via the API. Those repos
  should be cloned/inspected directly in a later phase for exact dates.

---

## 1. TTBlue ("TapTools Blue")

**One-line:** TTBlue was Tim Place's open-source C++ DSP library — the engine reused
both by Electrotap's tap.tools and by Jamoma's audio externals — that was later
absorbed into the Jamoma platform and renamed **Jamoma DSP / JamomaDSP**.

### Name and identity
- **"TTBlue" = "TapTools Blue."** The SourceForge mailing list is literally named
  *"ttblue-devel Mailing List for **TapTools Blue**"* — this resolves the meaning of
  the "TT" prefix and ties the library by name to TapTools.
  [snippet — verify] (sourceforge.net/p/ttblue/mailman/ttblue-devel/)
- The `tt_*` prefix is **[repo-confirmed]**: this TapTools repo vendors the headers at
  `/home/user/TapTools/source/ttblue/` (e.g. `tt_audio_base.h/.cpp`, `tt_audio_signal.h`,
  plus a large set of DSP unit-generator headers: `tt_adsr.h`, `tt_allpass.h`,
  `tt_bandpass_butterworth.h`, `tt_comb.h`, `tt_delay.h`, `tt_limiter.h`, `tt_lfo.h`,
  `tt_svf.h`, `tt_lowpass_*pole.h`, `tt_overdrive.h`, `tt_phasor.h`, `tt_noise.h`,
  `tt_rms.h`, etc.). The class-name convention (`tt_audio_base`) is the original
  pre-"TT"-CamelCase TTBlue style.

### Dates / provenance
- **Created 2003 in Bergen, Norway** by Timothy Place. **Open-sourced 2005 under the
  GNU LGPL.** [snippet — verify] (redmine.jamoma.org/projects/dsp/wiki;
  trondlossius.no/articles/909-what-happens-to-tlobjects)
- Repeated phrasing across sources: *"TTBlue was initially created by Timothy Place in
  2003 in Bergen, Norway and open sourced in 2005. It is a GNU LGPL C++ library of
  mainly DSP methods developed by Tim Place and used for tap.tools as well as several
  Jamoma externals."* [snippet — verify]

### Hosting
- **SourceForge:** `http://sourceforge.net/projects/ttblue` (project + `ttblue-devel`
  mailing list). [snippet — verify]
- Also referenced as self-hosted at **`http://electrotap.net:9004/ttblue`** (an
  Electrotap-run server/redmine/SVN-style URL). [snippet — verify] — verify whether this
  was SVN/Trac; the `:9004` port suggests a self-hosted dev server.

### What it contained / capabilities
- Object-oriented C++ DSP: *"signal generators, audio effects, filters, Fourier
  transforms and windowing functions for granular and spectral processing."*
  [snippet — verify] (the `source/ttblue/` header list above corroborates the
  generators/filters/effects part [repo-confirmed]).
- Cross-target: *"used effectively to create VST and AU plug-ins as well as external
  objects for Pure Data and Cycling '74's Max/MSP."* [snippet — verify]
- Design note (from the old DSP wiki): *"an object-oriented, reflective application
  programming interface for C++, with an emphasis on real-time signal processing …
  TTBlue typically communicates by sending messages rather than calling functions."*
  This message-passing/reflective design is the seed of what became the
  **Jamoma Foundation** runtime model. [snippet — verify] [inferred link]

### Relationship to tap.tools and to Jamoma
- TTBlue was the **shared DSP engine**: it backed Electrotap's **Tap.Tools** AND
  several Jamoma audio externals. Named Jamoma examples that were built on TTBlue:
  **`jcom.limiter~`** and **`jcom.saturation~`**. [snippet — verify]
- *"is used extensively by Electrotap's Tap.Tools and the open source Jamoma project."*
  [snippet — verify]

### Rename / absorption into "Jamoma DSP"
- **TTBlue was pulled under the Jamoma Platform umbrella and renamed "Jamoma DSP"**
  (a.k.a. JamomaDSP). *"TTBlue (also known as JamomaDSP/JamomaCore) was eventually
  pulled under the umbrella of the Jamoma Platform, rather than being an entirely
  independent project, and the name of TTBlue was changed to Jamoma DSP."*
  [snippet — verify]
- In the layered Jamoma platform, **Jamoma DSP specializes the Jamoma Foundation
  classes to provide a framework/library of unit generators** (synthesis, processing,
  analysis, filters, FFT/windowing). [snippet — verify]
- A standalone GitHub mirror exists: **`RWelsh/JamomaDSP`** ("Audio and Digital Signal
  Processing library build on the Jamoma Foundation"). [snippet — verify]
  (github.com/RWelsh/JamomaDSP)
- **"TTBlueMeetingInGenoa"** is a page on the old DSP wiki — implies a TTBlue
  developer meeting in **Genoa/Genova** (plausibly aligned with NIME 2008 in Genova).
  [snippet — verify] (redmine.jamoma.org/projects/dsp/wiki/TTBlueMeetingInGenoa) —
  **OPEN QUESTION**, see §7.
- A **`ChangeLog`** page also existed on the DSP wiki — a prime Wayback target for
  exact TTBlue→JamomaDSP version dates.
  (redmine.jamoma.org/projects/dsp/wiki/ChangeLog) [snippet — verify]

### Note on TapTools' own DSP in this repo
[repo-confirmed] This repo also carries `Core/DSP/` (the JamomaDSP framework) and the
full Jamoma layered stack under `Core/` (see §3). So TapTools v4 effectively ships
*both* the legacy `source/ttblue/` headers and the modern JamomaCore `Core/`
frameworks — a direct artifact of the lineage. (Detailed local cross-check is the
job of phase 03; here the focus is the external/published history.)

---

## 2. The Jade → Jamoma origin story

**Canonical primary source:** the Cycling '74 article **"The Development of Jamoma"**
(now at `https://cycling74.com/articles/the-development-of-jamoma`; originally dated
**2005-09-15**, URL pattern `cycling74.com/2005/09/15/the-development-of-jamoma`).
[snippet — verify on live page]

### The narrative (assembled from multiple snippets)
1. **Jade.** Jamoma grew out of **Jade**, a *"realtime performance management
   environment"* / *"realtime performance management environment called Jade, written
   by Timothy Place using Cycling '74 Max."* Jade's purpose was to reduce the
   complexity and instability of large Max patches and to aid Place's own
   compositional work. [snippet — verify]
   - **Jade timeline:** initial development **early 2001**; **public release end of
     2002**; significant update to **version 1.1 in mid-2003**. [snippet — verify]
2. **The shrimp meal (2003).** *"In 2003 the modular architecture was lifted out of
   Jade **at the recommendation of Trond Lossius over a meal of shrimp on the West
   coast of Norway.** … It was at this time that the great benefit of having modules
   directly within Max was realized — and Jamoma was born."* [snippet — verify]
   (Bergen / Norway's west coast; Lossius was at BEK.)
3. **BEK, October 2003.** Development *"started during his time at BEK in October
   2003."* Trond Lossius (BEK – Bergen senter for elektronisk kunst) joined Place to
   build the new open-source Max/MSP/Jitter framework. [snippet — verify]
   (bek.no/en/jamoma-web-site/, bek.no/en/jamoma-2/, bek.no/en/kronologi/chronology/)
4. **Open-source launch, 2005.** *"In 2005 the work began in earnest"* — Jamoma was
   started as an Open Source project hosted on **SourceForge**. The author/initial
   author identifies as Place. The book's lead (formal open-source launch **March
   2005**) is consistent but not directly quoted in snippets — **verify the exact
   March 2005 date** on the live Cycling '74 article or BEK chronology. [snippet — verify]

### Places & institutions
- **BEK — Bergen senter for elektronisk kunst** (Bergen Center for Electronic Arts),
  Bergen, Norway. Trond Lossius' home institution; the cradle of Jamoma. BEK keeps a
  chronology and Jamoma pages (bek.no). [snippet — verify]
- Later international support: Germany, USA, France, Norway (see §3 / §4).

---

## 3. Jamoma development arc, 2005–~2017 (the layered platform)

### Layered architecture (JamomaCore frameworks)
[repo-confirmed] This repo's `Core/` contains the canonical layer directories:
`Foundation/`, `DSP/`, `Graph/`, `AudioGraph/`, `Modular/`, `Score/` (plus `Shared/`).
`Core/README.md` header reads **"JamomaCore … Jamoma Frameworks for Audio and Control
Structure."** `Core/License.txt` is the **New BSD License** ("copyrighted by the
contributors to Jamoma").

Layer roles (from published descriptions [snippet — verify]):
- **Foundation** — low-level C++ support: base classes, the object model, dynamic
  binding / introspection / reflection, and the messaging/communication system.
- **DSP** (the former TTBlue) — specializes Foundation to provide unit generators for
  audio synthesis/processing/analysis.
- **Graph** — networks Foundation objects into graph structures; basic *asynchronous*
  processing model for nodes.
- **AudioGraph** — extends/specializes Graph to network DSP objects into dynamic
  multi-channel audio-processing graphs (well suited to HOA / Ambisonics / Wave Field
  Synthesis / mic arrays).
- **Modular** — the Max-facing layer: structured development & control of "modules"
  (the `jcom.*` modules), presets, mapping, cueing, MVC separation.
- **Score** — time/sequencing layer; connects to the French i-score / OSSIA lineage
  (see §4).
- (Also referenced historically: **Jamoma Graphics** for screen graphics.)

### Version timeline (Modular, the user-facing product)
- **0.2 / 0.3** — early public releases; "quite mature despite low version number."
  (0.3.1 announced via jamoma.org blog 2006-06-25.) [snippet — verify]
- **0.4** — major update; *"rewrite of the entire core architecture … now coded in
  C"*; faster/clearer. [snippet — verify]
- **0.5** — major rewrite for **Max 5**; originally planned as a Max4→Max5 port but
  became a large overhaul (performance, Windows stability, ease of use). Announced on
  cycling74 forums ("Jamoma 0.5 Released"). [snippet — verify]
- **0.6 (alpha)** — enables **MVC separation** in Max via custom externals + patching
  guidelines; download page `jamoma.org/download/0.6/`. [snippet — verify]

### International team expansion → French Score / OSSIA branch
- The **Score** work connects to **i-score** (later **OSSIA score**), driven by a
  French cluster: **GMEA** (Centre National de Création Musicale, Albi-Tarn) and
  **Blue Yeti**, with **LaBRI**, ENJMIN, RSF as ANR-OSSIA partners. [snippet — verify]
- **ANR OSSIA** ("Open Scenario System for Interactive Application"), coordinated by
  GMEA. Participants incl. artists **Pascal Baltazar**, Renaud Rubiano, **Julien
  Rabin**; engineers/researchers **Théo de la Hogue**, Clément Bossut, Jaime Chao,
  Jean-Michaël Célerier, Nicolas Vuaille, etc. [snippet — verify] (ossia.io/project.html,
  blueyeti.fr/ossia-score/)
- **Jamoma ↔ i-score integration:** *"some services of i-score were improved by
  implementing the Jamoma Modular framework into libIscore."* i-score → **OSSIA Score
  1.0** released **December 2017**, free/open-source (ossia.io). [snippet — verify]
- Jamoma described as *"an international open source project involving Germany, the
  United States, France, and Norway."* [snippet — verify]

### Move to jamoma2 / wind-down
- **jamoma2** is the modern header-only rewrite (see §4 for details).
- **Activity tail-off (no formal EOL announcement found):** last-update signals from
  search — **JamomaCore ~Feb 1, 2017**; **JamomaMax ~Aug 15, 2016**; **JamomaPureData
  ~Apr 12, 2016**; the umbrella `jamoma/Jamoma` repo is described as *"a historical
  relic."* jamoma2's last GitHub activity ~**May 2017** ([fetched], see §4).
  [snippet — verify] — net: the project went **dormant ~2016–2017** rather than being
  formally retired. **Verify exact final release/tag locally.**

---

## 4. jamoma2 (the modern rewrite)

[fetched] from `https://github.com/jamoma/jamoma2`:
- **Tagline:** *"A header-only C++ library for building dynamic and reflexive systems
  with an emphasis on audio and media."*
- **License: MIT.** (A clean break from JamomaCore's New BSD / TTBlue's LGPL —
  noteworthy for the book's licensing thread: LGPL → BSD → MIT.)
- **C++ standard: C++11 / C++14.** Rationale (paraphrased from repo): Jamoma1's
  codebase grew over ~10 years; the language tooling (C++11/14) and best practices
  shifted dramatically, motivating a from-scratch, header-only redesign that also
  fixes thread-safety / race-condition problems in the v1 architecture.
- **Repo stats at fetch time:** ~403 commits on `master`, ~30 stars, ~6 forks; uses
  Travis CI (`.travis.yml`). Last activity ~**May 2017** [snippet — verify on exact date].
- A `STYLE.md` exists (coding-style guide) — useful primary source on the rewrite's
  engineering philosophy. (github.com/jamoma/jamoma2/blob/master/STYLE.md)

[repo-confirmed] jamoma2 is wired into THIS repo as a git submodule:
`.gitmodules` → `[submodule "source/jamoma2"]  path = source/jamoma2  url =
https://github.com/jamoma/jamoma2.git`. **Note:** the submodule was **not initialized
locally** this session (git log fell through to the TapTools repo), so jamoma2's own
commit dates/authors must be pulled later (`git submodule update --init`, or clone).

**Authorship:** specific jamoma2 contributor names were not surfaced by search; based
on the platform team, expect **Timothy Place, Trond Lossius, Nathan Wolek** (Wolek has
a Cycling '74 profile and is a long-time Jamoma DSP contributor). [inferred — verify
via repo contributors]

---

## 5. Key people

| Person | Role in the lineage | Notes / source |
|---|---|---|
| **Timothy Place** | Author of Jade, TTBlue, TapTools; Jamoma lead/architect; principal of jamoma2 | GitHub `tap`; later Cycling '74 / Ableton / Garmin. [snippet — verify] |
| **Trond Lossius** | Co-founder of Jamoma; "shrimp meal" instigator; BEK (Bergen); spatial-audio work | BEK; trondlossius.no; github `lossius`. [snippet — verify] |
| **Nils Peters** | DSP/AudioGraph, spatial audio, SpatDIF, automated testing | nilspeters.info; AudioLabs Erlangen. [snippet — verify] |
| **Alexander Refsum Jensenius** | Modular/control, NIME workshop, composite-parameters paper | arj.no; Univ. of Oslo. [snippet — verify] |
| **Nathan Wolek** | Jamoma DSP contributor; granular/DSP | cycling74.com "A Few Minutes with Nathan Wolek." [snippet — verify] |
| **Pascal Baltazar** | French Modular/Score; ANR-OSSIA artist-coordinator | baltazars.org; ossia.io. [snippet — verify] |
| **Théo de la Hogue** | Jamoma Score / i-score R&D coordination; ANR-OSSIA | blueyeti.fr; ossia.io. [snippet — verify] |
| **Julien Rabin** | French Modular/Score contributor; ANR-OSSIA | ossia.io. [snippet — verify] |
| **Dave Watson** | Windows support; NIME 2008 workshop co-leader | TapTools ReadMe thanks "Dave Watson" for Windows help [repo-confirmed]. |

(Institutions: **BEK**, Bergen / **GMEA**, Albi / **Blue Yeti** / **LaBRI**, Bordeaux /
**IRCAM** (research visits) / **Cycling '74**.)

---

## 6. Academic / citable sources (full trail)

> All confirmed to exist via search; PDFs largely on `jamoma.org/publications/` and
> `quod.lib.umich.edu` (ICMC proceedings). Master list page:
> **https://www.jamoma.org/publications/**

1. **Place, T. & Lossius, T. (2006).** *Jamoma: A Modular Standard for Structuring
   Patches in Max.* Proc. ICMC 2006, New Orleans, USA (Nov 6–11, 2006).
   - PDF: https://jamoma.org/publications/attachments/jamoma-icmc2006.pdf
   - Proceedings: https://quod.lib.umich.edu/i/icmc/bbp2372.2006.032/1
   - dblp: https://dblp.org/db/conf/icmc/icmc2006.html  | ResearchGate /242142499
   - **The foundational paper.** [snippet — verify metadata]

2. **Place, T., Lossius, T., Jensenius, A. R., & Peters, N. (2008).** *Flexible Control
   of Composite Parameters in Max/MSP.* Proc. ICMC 2008, Belfast, UK (pp. 233–236).
   - Proceedings: https://quod.lib.umich.edu/i/icmc/bbp2372.2008.132/1
   - dblp: https://dblp.org/db/conf/icmc/icmc2008.html
   [snippet — verify]

3. **Jensenius, A. R., Place, T., Lossius, T. (with Baltazar, P. & Watson, D.) (2008).**
   *Jamoma Workshop.* NIME 2008, Genova, Italy.
   - PDF: https://www.nime.org/web_archive/2008/documents/NIME2008-Jamoma-workshop.pdf
   - Also academia.edu /2823154. [snippet — verify]

4. **Place, T., Lossius, T., Jensenius, A. R., Peters, N., & Baltazar, P. (2008).**
   *Addressing Classes by Differentiating Values and Properties in OSC.* NIME 2008,
   Genova, Italy. (OSC namespace / node value-vs-property addressing.) [snippet — verify]

5. **Place, T., Lossius, T., & Peters, N. (2010a).** *A Flexible and Dynamic C++
   Framework and Library for Digital Audio Signal Processing.* Proc. ICMC 2010,
   New York, USA (Jun 1–5, 2010).
   - PDF: https://jamoma.org/publications/attachments/jamoma-icmc2010.pdf
   - Proceedings: https://quod.lib.umich.edu/i/icmc/bbp2372.2010.031/7
   - **This is the Jamoma Foundation + DSP (ex-TTBlue) framework paper** — describes
     the reflective C++ API, polymorphic typing, dynamic binding, introspection.
     [snippet — verify]

6. **Place, T., Lossius, T., & Peters, N. (2010b).** *The Jamoma Audio Graph Layer.*
   Proc. DAFx-10 (13th Int. Conf. on Digital Audio Effects), Graz, Austria (Sep 6–10).
   - PDF: https://www.jamoma.org/publications/attachments/jamoma-audiograph-DAFx.pdf
   - Announced: https://trondlossius.no/articles/2010-09-08-dafx-paper-on-jamoma-audio-graph.html
   [snippet — verify]

7. **Peters, N., Lossius, T., & Place, T. (2012).** *An Automated Testing Suite for
   Computer Music Environments.* Proc. SMC 2012 (9th Sound and Music Computing Conf.),
   Copenhagen (Jul 12–14, 2012). *(Best-paper nominee.)*
   - PDF: https://jamoma.org/publications/attachments/smc2012-testing.pdf
   - ResearchGate /263008262. [snippet — verify]

8. **(Spatialization, SMC 2009)** *A Stratified Approach for Sound Spatialization.*
   (Jamoma authors.) PDF:
   https://www.jamoma.org/publications/attachments/Spatialization-SMC09.pdf
   [snippet — verify authors/year]

9. **(Related) SpatDIF** — Peters, N., Lossius, T., & Schacher, J. — *The Spatial
   Sound Description Interchange Format: Principles, Specification, and Examples,*
   Computer Music Journal (MIT Press). Plus original SpatDIF proposal, ICMC 2008,
   Belfast. Jamoma blog: https://www.jamoma.org/blog/2013-05-1-spatdif-paper/
   [snippet — verify]

### Secondary / context URLs worth keeping
- Cycling '74: *The Development of Jamoma* — https://cycling74.com/articles/the-development-of-jamoma
- Cycling '74: *An Interview With Tim Place* — https://cycling74.com/articles/an-interview-with-tim-place
- Cycling '74: project page — https://cycling74.com/projects/jamoma
- 74Objects blog: *The Jamoma Platform* (2009-04-15) — https://74objects.wordpress.com/2009/04/15/the-jamoma-platform/
- Jamoma C++ library page — https://www.jamoma.org/cplusplus/  ; Core — https://www.jamoma.org/core/
- Jamoma API intro — https://api.jamoma.org/
- Jamoma team — https://www.jamoma.org/about/team/  (and https://jamoma.org/team/)
- BEK: Jamoma — https://bek.no/en/jamoma-2/ ; Jamoma Web Site — https://bek.no/en/jamoma-web-site/ ; Chronology — https://bek.no/en/kronologi/chronology/
- GitHub orgs/repos: https://github.com/jamoma ; jamoma2 — https://github.com/jamoma/jamoma2 ; JamomaCore — https://github.com/jamoma/JamomaCore ; Jamoma umbrella — https://github.com/jamoma/Jamoma ; RWelsh/JamomaDSP — https://github.com/RWelsh/JamomaDSP
- Old Redmine (likely offline): http://redmine.jamoma.org/projects/dsp/wiki ; .../ChangeLog ; .../TTBlueMeetingInGenoa ; news http://redmine.jamoma.org/projects/jamoma/news
- TTBlue SourceForge devel list — https://sourceforge.net/p/ttblue/mailman/ttblue-devel/
- French Score/OSSIA — https://ossia.io/project.html ; http://www.blueyeti.fr/ossia-score/

---

## 7. Open questions / to verify locally or via Wayback later

1. **Exact "March 2005" Jamoma open-source launch date** — confirm on the live
   Cycling '74 article (and confirm the article's own publish date 2005-09-15 and
   author byline). Wayback `cycling74.com/2005/09/15/the-development-of-jamoma`.
2. **TTBlue precise dates** — exact 2003 creation month and 2005 LGPL open-source date;
   SourceForge project registration date. Wayback `sourceforge.net/projects/ttblue`
   and the **DSP wiki `ChangeLog`** (redmine.jamoma.org/projects/dsp/wiki/ChangeLog).
3. **TTBlue → "Jamoma DSP" rename date** — when exactly was the library renamed/folded
   in? (DSP wiki ChangeLog is the best lead.)
4. **"TTBlueMeetingInGenoa"** — what/when was this meeting? Hypothesis: a dev meetup
   tied to NIME 2008 in Genova; confirm via the wiki page (Wayback).
5. **`electrotap.net:9004/ttblue`** — was this SVN/Trac? Relationship to the SourceForge
   project (mirror? successor?).
6. **jamoma2 commit history / contributors / first+last commit dates** — run
   `git submodule update --init source/jamoma2` (or clone jamoma/jamoma2) since GitHub
   MCP is scoped to `tap/taptools` only. Confirm MIT license file and authorship
   (Place / Lossius / Wolek?).
7. **Formal wind-down** — is there any explicit EOL/"project is dormant" statement, or
   only the dormant-by-inactivity signal (~2016–2017 last commits)? Check
   `jamoma/Jamoma` readme-old.md and the Redmine news feeds.
8. **Jade specifics** — was Jade ever released open-source? Where (download archive)?
   Confirm the 2001 / end-2002 / mid-2003-v1.1 dates against a primary Jade page.
9. **License chain for the book** — confirm the thread **TTBlue = LGPL → JamomaCore =
   New BSD [repo-confirmed] → jamoma2 = MIT [fetched]**; note TapTools itself ships
   under **New BSD (BSD-3-Clause)** per ReadMe.txt [repo-confirmed].
10. **SMC 2009 spatialization paper** — confirm exact author list/year.
