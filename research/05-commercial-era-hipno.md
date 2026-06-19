# 05 — The Commercial Era: Tap.Tools, Electrotap, and Hipno

> Archival recovery notes for the TapTools book. Focus: the **pre-2014 commercial
> history** that is NOT in the git repo — the business-name lineage, the Hipno
> deal with Cycling '74, Darwin Grosse's role, the Electrotap/Teabox hardware, and
> Tim Place's career arc.
>
> Author primary-source leads (Timothy Place) were used to seed the searches and
> are marked where corroborated.

---

## 0. Web-access status (READ FIRST)

- **WebFetch is broadly BLOCKED.** Every live-page fetch attempted returned HTTP
  403 Forbidden: `cycling74.com/articles/*`, `74objects.wordpress.com/*`, and
  even the academic host `music.mcgill.ca`. Do **not** rely on WebFetch for this
  topic.
- **Wayback Machine (web.archive.org) is HARD-BLOCKED** — not attempted.
- **WebSearch is the workhorse** and was productive: rich synthesized snippets
  plus source URLs. Most facts below come from WebSearch snippets; these are
  tagged **[snippet — verify]**.
- **One channel worked better than the web:** the GitHub MCP tool successfully
  read first-party files from `github.com/tap/TapTools` (`ReadMe.txt`,
  `REVIVAL.md`). These are authored by Tim Place and are **HIGH confidence**;
  tagged **[repo — first-party]**. (Note: `electrotap/Teabox` was NOT accessible
  via MCP in this session — "repository not configured" — so Teabox repo contents
  are search-only.)

Confidence tags used: **[repo — first-party]** (highest), **[snippet — verify]**
(WebSearch synthesis, corroborate against the actual page later), **[author]**
(stated by Tim Place in the task brief, treat as a lead to confirm).

---

## 1. Tap.Tools — the commercial product and its business-name lineage

Tap.Tools (styled **Tap.Tools** in the commercial era; **TapTools** in the later
open-source era) is a collection of Max/MSP (later Max/MSP/Jitter) externals and
abstractions by Timothy Place. Roots trace to **1999**; the commercial life ran
roughly **2002–2013**, after which it became free/open-source.

### 1.1 Business-name lineage (the through-line)

The same product passed through three legal/brand identities — this is the spine
of the commercial story:

| Era | Entity | Evidence |
|-----|--------|----------|
| ~1999–2002 | **Silicon Prairie Intermedia** | Tap.Tools 1.0 beta ReadMe: "software, source code, and documentation is copyright © 1999–2002 by Silicon Prairie Intermedia." Distributed from **www.sp-intermedia.com/tap/**. **[snippet — verify]** |
| ~2003–2009ish | **Electrotap** (Electrotap, LLC) | "Electrotap is a company formed by Tim Place and Jesse Allison." Based **Parkville / Kansas City, Missouri**. Marketed Tap.Tools, the Jade software, and the Teabox hardware. **[snippet — verify]** |
| through 2013 | **74 Objects LLC** (Kansas City, MO) | The GitHub `ReadMe.txt` closes with: "Copyright © 1999–2013 / 74 Objects LLC." Downloads were hosted at **download.74objects.com/taptools/** (now defunct). **[repo — first-party]** + **[snippet — verify]** |

- **74 Objects LLC self-description:** "a small company focusing on design and
  architecture for interactive systems in music and the arts" — Tim Place's
  vehicle; he taught full-semester courses on these subjects at UMKC and the
  Kansas City Art Institute. (Blog: `74objects.wordpress.com`, live but
  403-to-fetch.) **[snippet — verify]**
- The brand name "74 Objects" is itself a nod to **Cycling '74** (the "74").

### 1.2 The registration / installer model (commercial mechanics)

Confirmed first-party from the change-log in `ReadMe.txt` **[repo — first-party]**:

- Tap.Tools shipped with an **installer** and required a **registration number**
  for authorization. The repo's "New In TapTools 4 beta 1" entry documents the
  exact moment this ended:
  > "It is no longer required that you enter a registration number for
  > authorization (TapTools now operates on the honor system — if you derive
  > benefit from TapTools then please support it with a donation). An installer is
  > no longer provided."
- So: **paid + registration-key + installer through 3.6.x; free/BSD/no-key from
  v4 beta 1 (2013).** v4 beta 2 is dated **26 April 2013** and is governed by the
  **New BSD (3-Clause) License**.
- **Pro vs. standard license tiers existed:** the change log repeatedly notes
  features gated behind a "**Pro license**" — e.g. building **standalones**
  required a Pro license; `tap.applescript` was made to "work in standalones
  without a 'pro' license" as a v3.0 selling point. This matches the author's
  note of a **two-tier price (≈ US$65 standard / US$99 Pro)**, where the higher
  tier allowed building/distributing your own collectives, standalones, and
  plug-ins. **[author]** + corroborated by **[snippet — verify]** ("US$65–99
  license, $99 allowing users to build their own collectives, standalones, and
  plug-ins").

### 1.3 Versions and what each added

- **Tap.Tools 1 (beta ~2002)** — Silicon Prairie Intermedia. MSP-focused audio
  externals + help patches. (Beta 2b ReadMe survives, hosted on McGill's MUMT306
  course page.) **[snippet — verify]**
- **Tap.Tools 2 (April 2005)** — covered by **CDM** ("Tap.Tools 2: Max/MSP/Jitter
  Construction Kit Gets Bigger", `cdm.link/2005/04/...`). **Over 150 externals.**
  Added reverb, pitch shift, dynamics, vocoder, new delays, **envelope
  substitution**, audio filtering (incl. LFO-driven FFT filters), ADSR
  generators, buffer/loop processing, XML utilities, MIDI mapping, RNGs,
  AppleScript loading (Mac), and reusable UI for envelope/parameter control. CDM:
  "an indispensible collection of time-savers at a really low price." **[snippet
  — verify]**
- **Tap.Tools 3 (2008-era, Max 5)** — major modernization (from `ReadMe.txt`
  change log, **[repo — first-party]**): full Max 5 reference-page/file-browser/
  inspector integration; many objects to 64-bit internal processing; multichannel
  filters; **TapTools Builder** for standalones (Pro). New objects incl.
  `tap.filter~`, `tap.filecontainer`, `tap.folder`, `tap.svn`. Java/JS objects
  rewritten as native (removing the Java dependency on Windows).
  - **3.1** — "completely untethered from Jamoma"; **monolithic extension →
    per-object external binaries**; dropped **PPC** and **Windows** support.
  - **3.2** — Uninstall script; Max-for-Live fixes; many new `tap.filter~` types.
  - **3.6 / 3.6.x** — Max 6 native 64-bit audio; installer **code-signed for
    Apple Gatekeeper**; M4L fixes. (Last commercial line.)
- **Tap.Tools 4 (2013)** — open-sourced (New BSD), no key, no installer, Max 6.1,
  distributed as a **Max Package**. **[repo — first-party]**

### 1.4 The Jamoma entanglement (technical lineage)

- Tap.Tools became the seedbed for **Jamoma**. Tim Place is credited as a
  **Software Architect** on Jamoma; "Jamoma was originally born out of the modular
  construction of a realtime performance management environment called **Jade**,
  written by Timothy Place using Cycling '74 Max." **[snippet — verify]**
- In the v4 era, ~50 of 52 Tap.Tools source objects were thin C++ wrappers over
  the **old Jamoma DSP library** (`TTClassWrapperMax`, `TTDSPInit`). Only
  **`ttblue`** (a small support lib) was Jamoma-free. **[repo — first-party]** —
  this is the "TTBlue" the brief asks about: it's the SQLite/glue support library
  (the broader low-level C++ DSP foundation that grew into Jamoma was historically
  called **TTBlue / Jamoma DSP**).
- **A second, Cycling '74–internal port existed:** `Cycling74/taptools` ("**taptools-min**",
  2016–2019), an official C74 Min-based reimplementation, **since deleted
  upstream**; only surviving copy is archived as a branch of `tap/TapTools`. It
  had 7 Min-ported objects + prototypes. **[repo — first-party]** — a notable
  fact: Cycling '74 itself maintained Tap.Tools for a stretch.

**Sources (§1):**
- https://github.com/tap/TapTools (ReadMe.txt, REVIVAL.md — first-party)
- http://www.music.mcgill.ca/~ich/classes/mumt306/Tap.Tools_1.0b2b/ReadMe-Tap.Tools-beta2b
- https://cdm.link/2005/04/taptools-2-maxmspjitter-construction-kit-gets-bigger/
- https://74objects.wordpress.com/  (and /hipno/)
- https://cycling74.com/forums/tap-tools-3-2
- https://cycling74.com/forums/where-can-i-find-tap-tools
- https://www.jamoma.org/about/team/

---

## 2. Hipno — the Cycling '74 commercial plug-in suite

Hipno was the commercial high-water mark: a **Cycling '74-published plug-in suite
designed by Electrotap**, technically descended from Tap.Tools and built on
Cycling '74's **Pluggo** plug-in technology.

### 2.1 Facts table

| Field | Value | Confidence |
|-------|-------|-----------|
| Product | Hipno (HIPNO) v1.0 | confirmed |
| Publisher | **Cycling '74** | confirmed |
| Designed/built by | **Electrotap** — Timothy Place with **Jesse Allison** and **Nathan Wolek** | [snippet — verify] |
| Announced | **NAMM, ~January 20, 2005** (KVR "Cycling '74 announces Hipno") | [snippet — verify] |
| Shipped (Mac) | **Q1 2005 / Summer 2005** (KVR "releases v1.0"; 74objects says "Summer of 2005") | [snippet — verify] |
| Windows (XP) version | **later in 2005**; Win **v1.1.1 updated 22 Nov 2006** | [snippet — verify] |
| Price | **US$199 MSRP** | confirmed (multiple) |
| Plug-in count | "over 40" — variously **42** (74objects) / **43** (MusicRadar) | [snippet — verify] |
| Formats | **VST, Audio Units (Mac only), RTAS** | confirmed |
| Underlying tech | Built on **Pluggo** technology | [snippet — verify] |
| Signature UI | **Hipnoscope** | confirmed |
| Last version | **v1.0.4** (Mac); Win **v1.1.1** | [snippet — verify] |
| Discontinued | **May 15, 2009** (with Pluggo/Mode/UpMix) | confirmed |

### 2.2 What was in it

A mix of **granular, spectral, and filter/delay** processors and instruments,
plus a distinctive set of **live-video-as-control-source** plug-ins and a new set
of **modulators**. Named examples from the announcements **[snippet — verify]**:

- **Morphulescence** — cascaded bank of LFO-modulated morphing filters.
- **Vsynth** — uses a **live video feed as the spectral source** for a synth.
- **Deluge** — a quartet of granular processing engines.
- **Shypht** — pitch-shifting system with pitch quantization, filtering, feedback
  looping.
- (MusicRadar notes roughly **half of the video set are video-controlled**, incl.
  a granular audio looper and a synth version of "Sfylter.")

### 2.3 The Hipnoscope UI

"Many of the Hipno plug-ins offer the unique **Hipnoscope** interface to create,
control, and explore complex preset **morphs and interpolations** with a flick of
the wrist." Per the Tim Place interview, the Hipnoscope grew directly out of his
prior work on **(a)** video as a control source, **(b)** the **Teabox** sensor
interface (with Jesse Allison), and **(c)** **Jitter-based preset interpolation**.
The core design problem he set for Hipno: *how plug-ins are controlled in realtime
from within the plug-ins themselves.* **[snippet — verify]**

### 2.4 How Hipno relates to Tap.Tools / TTBlue (the key origin quote)

The single most important primary quote, from the Cycling '74 "**An Interview With
Tim Place**" article (page 403-blocked, but the text surfaced via WebSearch
snippet — **verify exact wording later**):

> "Hipno actually started out life as a really unfocused effort to leverage a
> bunch of 3rd-party Max work (**Tap.Tools**, GTK, etc.) into plug-ins. After an
> initial surge, the artist in me couldn't handle the diffuseness of this as a
> vision, so in going back to square one I started pulling things apart and
> reflecting on a vision for the package."

So technically: **Hipno = Tap.Tools DSP/ideas, refocused and wrapped as Pluggo
plug-ins with the Hipnoscope control layer.** **[snippet — verify]**

### 2.5 Why it was discontinued

- On **May 15, 2009**, Cycling '74 **discontinued sales of all its pre-built
  Max-based plug-in packages** — **Pluggo, Mode, Hipno, and UpMix** — stating it
  was "simply not cost-effective to support three different plug-in specifications
  on two different platforms, particularly given the increasing absence of
  standardization of host platforms." Pluggo-inspired devices were folded into
  **Max for Live** (released **23 Nov 2009**). **[snippet — verify]**
- Because Hipno was **built on Pluggo**, killing Pluggo killed Hipno. 74objects
  states plainly: "Hipno was discontinued due to the underlying technology on
  which it relied (Pluggo) being discontinued." **[snippet — verify]**
- Today Hipno appears on Cycling '74's **discontinued/legacy products** list (no
  longer for sale, no support; legacy authorization page exists).

**Sources (§2):**
- https://www.kvraudio.com/news/cycling_74_announces_hipno_2897
- https://www.kvraudio.com/news/cycling_74_releases_hipno_v1_0_over_40_plug_ins_4116
- https://www.kvraudio.com/news/cycling_74_updates_pluggo_v3_5_4_mode_v1_2_4_and_hipno_v1_0_4_4999
- https://www.mixonline.com/technology/namm-news-cycling-74-releases-hipno-plug-suite-380959
- https://www.synthtopia.com/content/2005/10/19/cycling-74-intros-hipno-10/
- https://rekkerd.org/cycling-74-has-announced-the-release-of-hipno-10/
- https://www.audiomasterclass.com/blog/hipno-plug-in-suite-from-cycling-74
- https://www.musicradar.com/reviews/tech/cycling74-hipno-22035 (and .../cycling74-hipno-23402)
- https://www.sweetwater.com/store/detail/Hipno--cycling-74-hipno
- https://www.hitsquad.com/smm/programs/Hipno/  (Win v1.1.1, 22 Nov 2006)
- https://74objects.wordpress.com/hipno/
- https://cycling74.com/articles/an-interview-with-tim-place  (403; snippet only)
- https://support.cycling74.com/hc/en-us/articles/360050994953-Cyclops-Hipno-Mode-Pluggo
- https://cycling74.com/downloads/discontinued
- https://www.kvraudio.com/news/cycling_74_discontinues_pluggo_moves_pluggo_technology_to_max_for_live_11565  (15 May 2009)
- https://cdm.link/cycling-74-ditches-plug-in-development-support-free-commercial-alternatives/

---

## 3. Darwin Grosse & creativesynth.com — the connector

Darwin Grosse (Dec 26, 1959 – June 5, 2022) is the figure who connected the
Tap.Tools world to Cycling '74 and became a central C74 personality.

- **The connection role:** The author's account — Grosse (of **creativesynth.com**,
  ~2002) **spearheaded the recommendation** that became Hipno — is consistent with
  the documentary record, though no single page states it verbatim **[author —
  verify]**. Independent corroboration of his closeness to Tap.Tools: the
  Tap.Tools `ReadMe.txt` **SPECIAL THANKS** credits **"Darwin Grosse for various
  suggestions including the tap.plug.* series."** **[repo — first-party]** — i.e.,
  Grosse was directly involved in the *plug-in*-oriented Tap.Tools objects, the
  exact lineage that became Hipno.
- **creativesynth.com (~2002):** described as "an online publication [Grosse] ran
  that people actually read" — a respected synth/Max writing outlet of the era.
  (Predecessor in spirit to his later **20objects.com / 20 Objects** and the
  podcast.) **[snippet — verify]**
- **Cycling '74 career:** **Director of Education & Customer Services** at Cycling
  '74; "found his greatest professional fulfillment working with his colleagues at
  Cycling '74 for the past **20 years**." Also **Professor of Digital Media Arts**
  (sources variously say University of Denver / University of Colorado Denver) and
  a magazine author. **[snippet — verify]**
- **"Art + Music + Technology" podcast:** launched **2013** (Jan 2014 per some
  trackers), ran ~**9 years / 380+ interviews**, ended **May 2022** just before his
  death. The de facto oral-history archive of this whole scene — a likely place to
  find Tim Place in his own words. (A specific **Tim Place episode** is highly
  plausible but I could **not pin an episode number** via search — to-do.)
- **Death:** passed away **June 5, 2022**, in Northfield, MN, age 62. Cycling '74
  published a tribute ("Working with Darwin").

**Sources (§3):**
- https://www.matrixsynth.com/2022/06/darwin-grosse-of-cycling-74-20-objects.html
- https://cassiel.com/2022/06/10/darwin-grosse-1959-2022/
- https://www.legacy.com/us/obituaries/northfieldnews/name/darwin-grosse-obituary?id=35233027
- https://cycling74.com/articles/working-with-darwin  (403; snippet only)
- https://cycling74.com/forums/darwin-grosse-appreciation-post
- https://www.synthtopia.com/content/2022/05/20/art-music-technology-podcast-ends-after-9-years-380-interviews/
- https://github.com/tap/TapTools (ReadMe.txt thanks — first-party)

---

## 4. Electrotap & the Teabox sensor interface (the hardware)

- **Electrotap, LLC** — formed by **Tim Place and Jesse Allison**, based
  **Parkville / Kansas City, Missouri**. Self-described as developing, building,
  and distributing software and hardware tools for innovative music and art.
  Product line: **Tap.Tools** (Max software), **Jade** (the performance
  environment that seeded Jamoma), the **Teabox** (hardware), and — as a designer
  for Cycling '74 — **Hipno**. Active at least ~2003–2009; later folded as Place's
  vehicle became **74 Objects LLC**. **[snippet — verify]**
- **Teabox (model T100A)** — a **sensor-to-computer interface**. Its distinguishing
  trick: instead of converting sensor voltages to MIDI (like budget rivals), it
  **digitizes them to digital audio (S/PDIF)**. Specs from reviews **[snippet —
  verify]**:
  - **24 inputs**: 8 continuous analog inputs at **12-bit** resolution + 16
    digital on/off inputs.
  - Transmits to the computer over a **digital-audio (S/PDIF / optical TOSLINK)**
    cable → very **low latency**, high resolution, immune to EMI, no USB-reliability
    problems for long-running installations.
  - Accepts Electrotap sensors plus **I-CubeX/"I-Cube Toaster"** and other sensors;
    connectors via **XLR, 1/4" TRS, 3-pin terminal header, or RJ (telephone)** jacks.
  - **Price: US$425.** Named after an early prototype housed literally in a **tea
    box**.
- **Documentation trail:** introduced in a paper **"Teabox: A Sensor Data
  Interface System," Allison & Place, ICMC 2004**; reviewed in **Computer Music
  Journal 29(3), 2005** (review by Moon) and on Project MUSE. Open-source files
  live at **github.com/electrotap/Teabox** (NOT MCP-accessible this session).
  **[snippet — verify]**
- **Jesse Allison** later became a professor (Louisiana State University, music
  technology) — a thread worth confirming for the book **[author — verify]**.
- **Nathan Wolek** (Hipno co-contributor) is a faculty member at **Stetson
  University** — granular-synthesis specialist; his involvement explains Hipno's
  strong granular content. **[snippet — verify]**

**Sources (§4):**
- https://quod.lib.umich.edu/i/icmc/bbp2372.2004.082/  (Teabox, ICMC 2004)
- https://muse.jhu.edu/article/188577/summary  (CMJ review)
- http://www.computermusicjournal.org/reviews/29-3/moon-teabox.html
- https://direct.mit.edu/comj/article/29/3/104/93983/Electrotap-Teabox-Sensor-Interface
- http://trondlossius.no/articles/280-teabox
- https://github.com/electrotap/Teabox
- https://www.stetson.edu/other/faculty/nathan-wolek.php

---

## 5. Timothy Place — career arc

A consistent self-summary appears across his bios: **"15 years designing and
creating real-time audio signal processing software for musicians, artists, and
researchers with Cycling '74 and Ableton."** **[snippet — verify]**

Timeline (assembled; exact start/end years still to be confirmed against LinkedIn):

1. **Education / academia** — PhD in **electronic music at the University of
   Missouri–Kansas City (UMKC)**. Received **UMKC "Entrepreneur of the Year"** for
   founding **Electrotap** while a doctoral student. Taught full-semester courses
   at UMKC and the **Kansas City Art Institute**. **[snippet — verify]** + **[author]**
2. **Silicon Prairie Intermedia → Electrotap → 74 Objects LLC** — his own
   companies (see §1), spanning ~1999–2013, producing Tap.Tools, Jade/Jamoma, the
   Teabox, and (for C74) Hipno.
3. **Cycling '74** — joined as a **Research Engineer** (the brief notes a period
   working *for* C74; exact years to confirm — plausibly ~2008 onward). Authored
   Jamoma/architecture work; the `Cycling74/taptools` (taptools-min) effort
   2016–2019 sits in this institutional orbit. **[snippet — verify]**
4. **Ableton** — moved to Ableton (Berlin) as part of the same "C74 and Ableton"
   15-year span (the Max-for-Live relationship makes the C74→Ableton hop natural).
   Years to confirm. **[snippet — verify]**
5. **Garmin** — since **October 2020**, **Technical Lead Software Engineer, Audio
   & DSP**, leading next-generation automotive audio systems. **[snippet — verify]**
6. **Open-source steward** — kept TapTools alive (BSD, 2013) and is now (per the
   repo's `REVIVAL.md`, dated **2026-06-18**) actively **reviving TapTools** for
   Apple Silicon + Windows on the modern Min/Max SDK, cutting the dead Jamoma
   dependency. **[repo — first-party]**

Personal color (from bios): builds furniture with hand tools, climbs/mountaineers,
long-distance cyclist. **[snippet — verify]**

**Sources (§5):**
- https://www.linkedin.com/in/timothyaplace/  (and /in/timothy-place-1a406b9/)
- https://theorg.com/org/garmin/org-chart/timothy-place
- https://cycling74.com/articles/an-interview-with-tim-place  (403; snippet only)
- https://www.jamoma.org/about/team/
- https://github.com/tap/TapTools (REVIVAL.md — first-party)
- https://en.wikipedia.org/wiki/Cycling_'74

---

## 6. Open questions / to verify later (when Wayback is reachable)

1. **Exact Tap.Tools 1.0 release date and price.** Confirm the 2002 ship and the
   first registration/installer model. (Wayback `sp-intermedia.com/tap/` and early
   `electrotap.com`.) **[snippet — verify]**
2. **Tap.Tools price points over time** — confirm the **$65 / $99 (std vs Pro)**
   split and whether it changed across v2/v3. (Wayback `download.74objects.com`,
   `electrotap.com`.)
3. **Hipno plug-in count** — 42 vs 43 (resolve; possibly Mac vs Win, or
   "instruments + effects" counting). Get the full **named plug-in list**.
4. **Hipno exact ship date** vs NAMM announce date (announce ~Jan 20 2005;
   "Summer 2005" ship per 74objects — pin the actual store date).
5. **The Darwin Grosse → Hipno causation**, in a citable form. Best bet: a quote
   in the C74 "An Interview With Tim Place," the "Working with Darwin" tribute, or
   an **Art + Music + Technology** episode. Find the **Tim Place AMT episode #**.
6. **creativesynth.com** — recover an actual snapshot (~2002) to describe what it
   published and Grosse's editorial role.
7. **Tim Place employment dates** — precise C74 start/end and Ableton start/end
   (LinkedIn was not directly fetchable; snippets only).
8. **Electrotap founding & dissolution dates**, and **Jesse Allison's** role/exit
   (and his LSU professorship timeline).
9. **The full text of the C74 "An Interview With Tim Place"** — currently 403;
   reconstruct from cache. It is the richest single primary source for the
   Tap.Tools→Hipno technical narrative.
10. **`Cycling74/taptools` (taptools-min, 2016–2019)** — confirm why C74 took
    Tap.Tools in-house and then deleted it; who worked on it. (First-party signal
    in `REVIVAL.md`; upstream repo is gone.)
11. **Teabox sales window and units** — production run, when discontinued, and the
    relationship to **I-CubeX**.
