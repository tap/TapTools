# TapTools — Ecosystem & External Context (Research Notes)

Research phase 03. Purpose: gather the wider ecosystem and external context around
**TapTools** — a suite of Max/MSP externals by **Timothy Place**, distributed by
**74 Objects LLC** (copyright 1999–2013), built on top of the **Jamoma** framework
and **ObjectiveMax** — to support planning a book.

---

## Web-access status

**Web research WAS available** in this session (WebSearch worked throughout;
WebFetch worked for some sites but was blocked/403 for others).

Caveats to keep in mind when using these notes:

- **WebSearch worked well.** Most findings below come from search-result summaries
  with cited URLs.
- **WebFetch was unreliable.** It returned HTTP 403 for Wikipedia
  (`en.wikipedia.org`), `cdm.link`, the Stetson lecture page, and the McGill-hosted
  old ReadMe. It worked for `github.com/jamoma/jamoma2`. It **could not** reach
  `web.archive.org` at all ("Claude Code is unable to fetch from web.archive.org").
- Because several full pages could not be fetched, some facts below are drawn from
  search-engine *snippets/summaries* rather than the primary page. These are marked
  **[snippet — verify]**. Facts confirmed from the local repository are marked
  **[repo-confirmed]**. Reasonable inferences are marked **[inferred]**.

A dedicated list of things to verify in a later phase is at the end.

---

## 1. Timothy Place — who he is in the field

**Summary:** Timothy Place is a real-time audio/DSP software engineer and lead
developer of Jamoma, with a ~15-year career at Cycling '74 and Ableton. He is the
author of TapTools and the principal behind its distributing entities (Silicon
Prairie Intermedia → Electrotap → 74 Objects LLC). His user/email handle is
`timothyaplace` (consistent with the GitHub account `tap` and the user context for
this session).

Career arc (chronological, assembled from multiple snippets):

- **Doctorate in electronic music**, University of Missouri–Kansas City (UMKC). Won
  UMKC's *Entrepreneur of the Year* award for founding Electrotap while a doctoral
  student. **[snippet — verify]**
- **Co-founder of Electrotap** (with Jesse Allison); co-creator of the **Teabox**
  sensor interface — a hardware/software sensor system for laptop/gestural
  electronic music. Reviewed in *Computer Music Journal* / Project MUSE. **[snippet]**
- **Lead developer / software architect of Jamoma** — described as "the lead
  developer of Jamoma, a platform for interactive arts-based research and
  performance." **[snippet]**
- **Cycling '74** — Research Engineer / member of the Max/MSP/Jitter development team
  for 15+ years. **[snippet]**
- **Ableton** — continued real-time DSP work (Cycling '74 was acquired by Ableton in
  2017; he is associated with Ableton DSP work). **[snippet — verify exact role/dates]**
- **Garmin (current, as of research date)** — Technical Lead Software Engineer,
  Audio & DSP, leading next-gen automotive audio for Garmin's Automotive OEM
  business. **[snippet]**
- Personal color: builds furniture with hand tools, climbs/mountaineers, long-distance
  cyclist. (Flavor for an author bio, if wanted.) **[snippet]**

**jamoma2** (the modern successor, see §3): a header-only C++11/14 library that Place
drove as a ground-up rewrite of the Jamoma 1 codebase. **[repo-confirmed: it is the
git submodule at `source/jamoma2` pointing to `https://github.com/jamoma/jamoma2.git`]**

Sources:
- https://www.stetson.edu (Stetson lecture announcement, 2010; "lead developer of
  Jamoma… member of the Cycling74 development team") — *fetch blocked, snippet only*
- https://www.linkedin.com/in/timothyaplace/ (Garmin / Audio & DSP)
- https://theorg.com/org/garmin/org-chart/timothy-place
- https://muse.jhu.edu/pub/6/article/188577 (Teabox review)
- https://cycling74.com/articles/an-interview-with-tim-place (primary interview — *not
  yet fetched; high-value for the book*)

---

## 2. 74 Objects LLC — the distributing entity

**Summary:** 74 Objects LLC was Timothy Place's company/label for distributing his
Max software (TapTools) and frameworks (ObjectiveMax), based in **Kansas City, MO**.
The name riffs on "Cycling '74" / Max-object culture ("74 objects" = objects for the
'74 platform). The copyright line in the TapTools ReadMe reads
**"Copyright © 1999–2013, 74 Objects LLC."** **[repo-confirmed]**

Lineage of the TapTools-distributing business (reconstructed):

1. **Silicon Prairie Intermedia** — published the earliest Tap.Tools (MSP version
   1.0 beta 2b, dated **13 August 2002**). **[snippet — verify]**
2. **Electrotap** — the company (Tim Place + Jesse Allison) that marketed Tap.Tools 2
   and 3 and the Teabox. ObjectiveMax's ReadMe states it "is used in commercial
   products such as TapTools 3, **marketed by Electrotap**." **[repo-confirmed for the
   Electrotap/ObjectiveMax link]**
3. **74 Objects LLC** — the entity named in the copyright and on the download site.
   ObjectiveMax ReadMe header: *"Copyright © 2007–2009 by 74 Objects LLC,
   http://74objects.com/."* **[repo-confirmed]**

Distribution/web presence:
- TapTools was sold/downloaded from **`http://download.74objects.com/taptools/index.html`**.
  As of research, the site is **defunct** ("the electrotap website no longer exists").
  **[snippet — verify; web.archive.org could not be fetched this session]**
- The advisory framing — "74 Objects… offers advisory options to customers in the
  field of real-time media in the arts" — appears in one snippet. **[snippet — verify]**

Sources:
- Local repo: `ReadMe.txt` (copyright line), `objectivemax/ReadMe.txt` (74objects.com URL,
  Electrotap link), `Core/License.txt` (BSD / Jamoma)
- https://github.com/electrotap/Teabox (Electrotap org on GitHub; file
  `_decoders/max4live/74Objects.xcconfig` ties the two names together)
- https://cdm.link/2005/04/taptools-2-maxmspjitter-construction-kit-gets-bigger/
  (*fetch blocked; snippet confirms "150+ externals", reverb/pitch/vocoder, 2005*)

---

## 3. Jamoma — the framework TapTools sits on

**Summary:** Jamoma is an **open-source framework for modular development in
Max/MSP/Jitter**. Its core idea is "the module" — a standardized wrapper pairing an
algorithm patcher with a GUI patcher, communicating via **Open Sound Control (OSC)**,
with built-in preset, mapping, and cueing infrastructure. It lets users build complex
audio/video patches quickly and is widely used in academic/interactive-arts contexts.
Licensed under **BSD** (the repo's `Core/License.txt` is the "New BSD License,"
attributed to "the contributors to Jamoma"). **[repo-confirmed + snippet]**

History & key people:
- Foundational paper: **T. Place & T. Lossius, "Jamoma: A Modular Standard for
  Structuring Patches in Max," ICMC 2006.** **[snippet — strong]**
- International OSS project (BSD). Contributors named across sources include:
  **Timothy Place** (Cycling '74, US), **Trond Lossius** (BEK, Bergen, Norway),
  **Nils Peters** (CNMAT, UC Berkeley), **Alexander Refsum Jensenius** (University of
  Oslo), **Pascal Baltazar** (composer, France), **Théo de la Hogue** (GMEA, France),
  **Julien Rabin** (GMEA, France). **[snippet]**
- Layered architecture papers exist (e.g., "The Jamoma Audio Graph Layer," DAFx;
  "A Stratified Approach for Sound Spatialization," SMC09). Jamoma's `Core` is split
  into layers — **Foundation, DSP, Graph, AudioGraph, Modular, Score** — which is
  mirrored in this repo's `Core/` subdirectories. **[repo-confirmed: those folders
  exist with their own License.txt files]**
- **jamoma2** = modern rewrite. Per the GitHub repo (fetched): *"a header-only C++
  library for building dynamic and reflexive systems with an emphasis on audio and
  media."* Motivation for the rewrite: after ~10 years, Jamoma 1 had thread-safety
  problems, race conditions/deadlocks/memory leaks in the AudioGraph, hard-to-maintain
  legacy code, and the desire to use modern C++11/14. Some components were judged
  "dead-ends" needing redesign. **License: MIT** (note: differs from Jamoma 1's BSD).
  ~403 commits on master, Travis CI. **[fetch-confirmed]**

**How TapTools relates to Jamoma:** TapTools is *built on top of* Jamoma (and
ObjectiveMax), not part of it. The ReadMe's "Special Thanks" says *"TapTools stands on
the shoulders of some excellent open source software projects, including Jamoma and
ObjectiveMax."* Several TapTools objects depend on Jamoma libraries (e.g. the change
log references `JamomaDSP.dylib`), and the TapTools 4 package bundles "requisite Jamoma
dependencies, where applicable." Historically there were version-conflict pains between
TapTools and Jamoma installs (change log notes fixes for "conflicts between TapTools 3.6
and Jamoma 0.5.6"). The repo vendors jamoma2 as a submodule. **[repo-confirmed]**

Sources:
- https://www.semanticscholar.org/paper/Jamoma:-A-Modular-Standard-for-Structuring-Patches-Place-Lossius/2d50ea07a395fa5a1b9ad8cfd3e58adb04933a9f
- https://www.nime.org/web_archive/2008/documents/NIME2008-Jamoma-workshop.pdf (Jensenius, Place, Lossius — Jamoma Workshop)
- https://www.jamoma.org/publications/attachments/jamoma-audiograph-DAFx.pdf
- https://jamoma.org/blog/archive/ ; http://redmine.jamoma.org/news (project news — *not fetched*)
- https://github.com/jamoma/jamoma2 ; https://github.com/jamoma/Jamoma ; https://github.com/jamoma/JamomaMax
- Local repo: `Core/*/License.txt`, `.gitmodules`

---

## 4. Cycling '74 / Max/MSP — the platform & third-party-externals culture

**Summary:** TapTools lived inside the **Max/MSP/Jitter** ecosystem and its culture of
third-party "externals." Key facts:

- **Max** originated at **IRCAM**, created by **Miller Puckette** (late 1980s). It was
  commercialized — first via **Opcode Systems** — then **Opcode sold the Max brand in
  1997 to Cycling '74**, a company **founded in 1997 by David Zicarelli**, who had been
  central to Max's development since the late 1980s. **[snippet — strong/consistent]**
- **MSP** (audio/signal processing, ~1997) and **Jitter** (matrix/video/3D, 2002)
  extended Max. Jitter was co-authored by **Joshua Kit Clayton** and **Jeremy Bernstein**.
  **[snippet]**
- **Max for Live**: Cycling '74 developed Max for Live jointly with **Ableton**.
- **Ableton acquired Cycling '74 in June 2017.** **[snippet — strong/consistent]**
- **Externals culture:** Max is extensible via compiled C "externals." A robust
  third-party ecosystem grew up around it (TapTools, Jamoma, litterpro, CNMAT objects,
  etc.), distributed commercially and as open source, discussed heavily on the
  Cycling '74 forums. TapTools (150+ externals) and the Jamoma modules are exemplars of
  this culture. The ReadMe's "T" key-command (type "T" to spawn an object box prefilled
  with "tap.") shows how deeply TapTools integrated into the Max patching workflow.
  **[repo-confirmed for the workflow detail]**

The repo also vendors the official **`max-sdk`** (Cycling '74's C SDK) and
**`objectivemax`** — Place's own framework for writing Max externals in **Objective-C**
(wrapping them automatically as Max externals, giving access to Apple's Cocoa APIs;
"Max 5 ready," requires Max 5.0.8 / macOS 10.6). **[repo-confirmed]**

Sources:
- https://en.wikipedia.org/wiki/Cycling_'74 (*fetch blocked; snippets consistent*)
- https://en.wikipedia.org/wiki/Max_(software) (*fetch blocked*)
- https://cycling74.com/company
- https://www.ableton.com/en/blog/ableton-cycling-74-new-partnership/
- https://www.synthtopia.com/content/2017/06/06/ableton-acquires-cycling-74-creators-of-max/
- https://cdm.link/exclusive-ableton-acquires-max-maker-cycling-74-inside-the-deal/
- Local repo: `max-sdk/`, `objectivemax/ReadMe.txt`

---

## 5. Named collaborators in TapTools' credits

From the local `ReadMe.txt` "Special Thanks" list **[repo-confirmed for who is credited
and for what object]**, with field identification from web search:

| Name | Credited in TapTools for | Who they are (web) |
|---|---|---|
| **David Zicarelli** | tap.comb~, tap.buildassist, "many others" | Founder of **Cycling '74** (1997); central Max developer since late 1980s. The most senior figure in the Max world. **[snippet — strong]** |
| **Joshua Kit Clayton** ("Kit Clayton") | "numerous objects" | Cycling '74 programmer; co-developer of Max/MSP MIDI/audio and **Jitter**; also a noted SF electronic/glitch/dub musician (stage name *Kit Clayton*). **[snippet — strong]** |
| **Jeremy Bernstein** | tap.applescript, tap.myip, tap.windowdrag | Cycling '74 developer since ~2000; co-developer of Max/MSP/Jitter; later associated with Max for Live workshops. **[snippet]** |
| **Ali Momeni** | tap.jit.ali named for him; interpolation algorithms, testing | Composer/educator; PhD from **UC Berkeley CNMAT**; physics+music at Swarthmore; works in Max/Pd/microcontrollers; professor (CMU; now Brown Arts). **[snippet]** |
| **Trond Lossius** | "various contributions and feedback" | Bergen-based sound/installation artist, **BEK** (Bergen Center for Electronic Arts); **co-author of the foundational Jamoma paper**; spatial-audio researcher (SpatDIF). **[snippet — strong]** |
| **Nils Peters** | "extensive feedback and assistance"; submitted help-patcher fixes | Spatial-audio researcher, **CNMAT (UC Berkeley)**; co-author of **SpatDIF**; Jamoma contributor. **[snippet — strong]** |
| **Richard Dudas** | filter externs, color conversion, buffer binding; **provided code for list.join / list.slice** | Composer/researcher; long associated with **IRCAM** and Max development; later professor (Hanyang University, Korea). **[inferred from field; verify]** |
| **Darwin Grosse** | tap.plug.* series suggestions | Long-time Cycling '74 figure (Director of Education/community); host of the **"Art + Music + Technology"** podcast. **[snippet — Momeni was a podcast guest; verify Grosse role]** |
| **Russell Pinkston** | idea for tap.random~ and tap.counter~ | Professor of composition & director of electronic music studios, **University of Texas at Austin**; computer-music researcher (real-time synthesis/DSP, Csound). **[snippet — strong]** |

Other credited (not in the orchestrator's named list, but in the ReadMe — worth noting):
- **Stephan Moore** — tap.anticlick~. **[repo-confirmed]**
- **"jhno"** — augmenting his **limi~** algorithm, and tap.windowdrag. ("jhno" is the
  alias of **John Eichenseer**, an electronic musician/Max developer. **[inferred — verify]**)
- **Dave Watson & Rob Sussman** — Windows port help. **[repo-confirmed]**
- **Andrew Pask** — testing/feedback. (Andrew Pask = Cycling '74 figure. **[inferred]**)

Sources:
- Local repo `ReadMe.txt` (the credits themselves)
- https://en.wikipedia.org/wiki/Kit_Clayton ; https://grayarea.org/community-entry/kit-clayton/
- https://en.wikipedia.org/wiki/Russell_Pinkston ; https://www.ravellorecords.com/artists/russell-pinkston/
- https://alimomeni.net/ali ; https://arts.brown.edu/people/ali-momeni
- https://trondlossius.no/text/ ; https://bek.no/
- https://www.cnmat.berkeley.edu/ (Nils Peters, SpatDIF)
- https://artmusictech.libsyn.com/ (Darwin Grosse podcast)

---

## 6. TapTools / 74objects web & archive presence

What exists (confirmed or strongly indicated):

- **GitHub:** `https://github.com/tap/TapTools` ("TapTools for Max and Jamoma") — the
  open-source home; releases at `/tap/TapTools/releases`. The `tap` account is Timothy
  Place. **[snippet — strong; matches local repo origin]**
- **Cycling '74 forums:** extensive threads (Tap.Tools 2/3.2, "TapTools for Max 6.1
  x64?", "MAX 6 and Taptools 4", "where can I find tap tools?", compatibility with
  litterpro, etc.) — a rich primary record of *user reception and the support
  lifecycle*. **[snippet — strong]**
- **CDM (Create Digital Music):** 2005 article on Tap.Tools 2 ("Max/MSP/Jitter
  Construction Kit Gets Bigger") — press coverage. URL:
  `https://cdm.link/2005/04/taptools-2-maxmspjitter-construction-kit-gets-bigger/`.
  **[fetch blocked; snippet only]**
- **Academic footprint (Jamoma, not TapTools per se):** ICMC 2006, NIME 2008 workshop,
  DAFx, SMC09, SpatDIF papers — Jamoma is well cited; TapTools itself appears less in
  academic literature and more in practitioner/forum contexts. **[inferred]**
- **Old distribution site:** `download.74objects.com/taptools/` — **now defunct.**
- **`74objects.com`** — referenced as live in the ObjectiveMax ReadMe (2007–09 era);
  current status unverified this session.
- **Internet Archive:** **could not be checked** — WebFetch is blocked from
  `web.archive.org` in this environment. (A later phase should snapshot-check
  `74objects.com`, `download.74objects.com`, `electrotap.com`.)
- **Old ReadMe mirror:** an early Tap.Tools 1.0b2b ReadMe is mirrored on a McGill
  course page (`music.mcgill.ca/~ich/classes/mumt306/`), dated ~Aug 2002, attributing
  publication to **Silicon Prairie Intermedia**. **[snippet; fetch blocked]**

Sources:
- https://github.com/tap/TapTools ; https://github.com/tap
- https://cycling74.com/forums/where-can-i-find-tap-tools (and sibling forum threads)
- https://cdm.link/2005/04/taptools-2-maxmspjitter-construction-kit-gets-bigger/
- http://www.music.mcgill.ca/~ich/classes/mumt306/Tap.Tools_1.0b2b/ReadMe-Tap.Tools-beta2b

---

## Confirmed vs. inferred vs. unverifiable — quick triage

**Confirmed (repo or fetched primary):**
- TapTools v4 beta 2, dated 26 April 2013; New BSD License; copyright 1999–2013,
  74 Objects LLC; built on Jamoma + ObjectiveMax; the full credits list. (repo)
- ObjectiveMax = Place's Objective-C-for-Max framework, © 2007–2009 74 Objects LLC,
  used in "TapTools 3, marketed by Electrotap," BSD-licensed. (repo)
- Jamoma Core layered structure (Foundation/DSP/Graph/AudioGraph/Modular/Score). (repo)
- jamoma2 = header-only C++11/14 rewrite, MIT-licensed, rewrite motivated by Jamoma 1's
  thread-safety/legacy issues. (fetched GitHub)
- Cycling '74 founded 1997 by David Zicarelli; Max from IRCAM/Puckette; Ableton acquired
  Cycling '74 in 2017. (consistent across multiple snippets)

**Inferred (field knowledge / single weak source):**
- Richard Dudas's IRCAM/Hanyang affiliation; "jhno" = John Eichenseer; Andrew Pask =
  Cycling '74; Darwin Grosse's exact Cycling '74 title.
- The Silicon Prairie Intermedia → Electrotap → 74 Objects business lineage (timeline
  plausible but assembled from snippets).

**Unverifiable this session:**
- Anything requiring `web.archive.org` (fetch blocked).
- Full text of: Wikipedia (Cycling '74, Max, Kit Clayton, Russell Pinkston), the CDM
  2005 article, the Stetson lecture bio, the Cycling '74 "Interview with Tim Place,"
  the McGill old ReadMe — all returned 403 to WebFetch.

---

## Open questions / things to verify in a later phase

1. **Internet Archive snapshots** of `74objects.com`, `download.74objects.com/taptools/`,
   and `electrotap.com` — capture the product pages, pricing, and the original
   marketing copy (use a tool that can reach web.archive.org, or `curl`).
2. **The Cycling '74 "Interview with Tim Place"** (`cycling74.com/articles/an-interview-with-tim-place`)
   — likely the single richest first-person source for the book; fetch in full.
3. **Exact business lineage & dates:** When did Silicon Prairie Intermedia operate?
   When was Electrotap formed, and when 74 Objects LLC? Was 74 Objects the successor to
   Electrotap or a parallel entity? Jesse Allison's role.
4. **Tap.Tools version timeline & pricing:** confirm release dates for v1 (2002), v2
   (2005), v3, v3.6, v4 beta 1 (open-sourced), v4 beta 2 (2013), and the "registration
   number → honor-system donation" transition.
5. **Richard Dudas** — confirm IRCAM history and current affiliation; nail down the
   list.join/list.slice code-license provenance (he gave code + permission per ReadMe).
6. **"jhno"** — confirm identity (John Eichenseer) and the `limi~` algorithm history.
7. **Darwin Grosse** and **Andrew Pask** — confirm exact roles at Cycling '74.
8. **Jamoma history precision:** founding year (~2003–2005?), version-to-Max-version
   mapping, the redmine/jamoma.org blog timeline, and when active development wound down
   in favor of jamoma2.
9. **CDM 2005 article** full text — useful contemporaneous press framing/quotes.
10. **Teabox / Electrotap** — the *Computer Music Journal* review (Project MUSE) for a
    citable academic mention of Place's hardware work.
11. **Academic citation count** for the Jamoma ICMC 2006 paper (establishes Jamoma's,
    and by extension TapTools', standing in the research community).
12. **Stephan Moore, Dave Watson, Rob Sussman** — minor collaborators; identify if the
    book wants full credit coverage.

---

*Compiled by research phase 03. All "[snippet]" facts derive from WebSearch result
summaries (URLs cited inline) and should be upgraded to primary-source confirmation
where they matter to the book's claims.*
