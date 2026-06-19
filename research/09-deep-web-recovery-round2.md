# Deep Web-Search Archival Recovery — Round 2

Research notes for the TapTools book project (Max/MSP externals by Timothy Place;
DSP library TTBlue→Jamoma; commercial plug-in Hipno; company lineage Silicon
Prairie Intermedia→Electrotap→74 Objects LLC).

Date compiled: 2026-06-19. Branch: `claude/taptools-book-planning-6t9hzv`.
This is the SECOND, deeper pass; it goes deeper on threads left thin in round 1
(files 00–07) and does not re-derive round-1 basics.

---

## 0. Web-access status (READ FIRST)

- **WebFetch is fully blocked in this environment** — every external URL (academic
  PDFs, cycling74.com, archive.org, muse.jhu.edu, etc.) returns HTTP 403. No page
  was actually opened.
- **The only working web tool is WebSearch**, which returns synthesized snippets +
  source URLs. It frequently surfaces page *content* (quotes, specs, dates) even
  when the page itself can't be loaded. That is the entire basis of what follows.
- **Confidence tags used below:**
  - `[snippet — verify]` = taken from a WebSearch synthesis; plausible but the
    underlying page was not opened. Treat as a lead, not a citation, until checked.
  - `[corroborated]` = the same fact appeared in ≥2 independent searches/sources.
  - `[conflict]` = sources disagree; flagged explicitly.
  - `[inferred]` = my reasoning from the snippets, not a direct statement.
- Anything needing the real page text is added to the **Wayback / primary-source
  target list** at the end (§9), expanding round 1's list.

---

## 1. Jade — Place's pre-Jamoma realtime performance-management environment

### What it was
- Jade was a **realtime performance management environment written by Timothy Place
  in Cycling '74 Max** — "a flexible, relatively easy-to-use environment for
  composition and performance." [corroborated; dontcrack + jamoma]
- Usage model: you **configure your environment by choosing modules** — inputs,
  effects, and a **matrix for connecting modules** — though **scripting was
  required**. You could pick a pre-existing Jade module or **build your own**.
  [snippet — verify; archives.didascalie.net "about Jade"]
- **Each variable of each module has an address** that can be changed live during a
  show via **events**. Events can carry **time notions** and **link event actions
  to physical actions** (e.g., pressing the space bar). [snippet — verify;
  didascalie "about Jade"] — This address-per-parameter + event-automation design
  is recognizably the seed of Jamoma's later parameter/OSC-address model. [inferred]
- It was an application that **hosts modules and allows script-based automation** of
  them. [snippet — verify; cycling74 "The Development of Jamoma"]

### Dates / chronology
- **Initial development began early 2001**, as a means to aid Place's own
  compositional work. [corroborated; "Development of Jamoma" + interview snippet]
- **Released to the public at the end of 2002.** [corroborated]
- **Significant update to v1.1 in mid-2003.** [corroborated]
- (These exactly match the dev-2001 / public-end-2002 / v1.1-mid-2003 framing in
  the task brief, now independently confirmed via search.)

### Why it mattered / how it led to Jamoma
- "**The great benefit of having modules directly within Max was realized in
  mid-2003, and Jamoma was born.**" [snippet — verify; "Development of Jamoma"]
- "**In 2003 the modular architecture was lifted out of Jade at the recommendation
  of Trond Lossius over a meal of shrimp on the West coast of Norway. In 2005 the
  work began in earnest.**" [corroborated — this anecdote appears verbatim in
  multiple search syntheses; likely from a Jamoma history page or ICMC2006 paper]
- Jamoma was **started as an open-source project hosted on SourceForge**, then
  activity **died off until revived in March 2005**. [snippet — verify; "Development
  of Jamoma"] — Note the two-date framing: idea 2003, real start/revival 2005.
- Marketing context: **Electrotap "markets Tim's ever-popular Tap.Tools externals …
  and the Jade software, in addition to hardware."** [snippet — verify] So Jade was
  positioned as a *product* alongside Tap.Tools and Teabox, even though free/OSS.

### Distribution
- Jade was distributed **free / open-source**. [corroborated]
- Listed on **dontcrack.com freeware** (id 2774). [confirmed URL exists]

**Still thin / needs the page:** screenshots, reviews, the actual module list, the
v1.1 changelog, and any CDM/forum announcement of Jade's public release. The
didascalie.net "about Jade" page is the single richest recoverable description —
prioritized in §9.

---

## 2. Tap.Tools — version-by-version reception

### Tap.Tools 1.x (≈2003–2004)
- Pre-2.0 marketing: "**over 100 objects** for Max, MSP, and Jitter, including
  effects processors, plugin-building utilities, video-analysis utilities, and
  much more." [snippet — verify; JyK blog "Tap.Tools Update"]
- A **v1.5** is referenced on shareware mirrors ("Tap.Tools by Electrotap v.1.5",
  sharewarejunction / softwaresea). [snippet — verify] — useful as a version-point
  even though those are junk download mirrors.
- McGill MUMT306 hosts a **"Tap.Tools_1.0b2b"** beta with source (.c files:
  `tap.avg~.c`, `tap.typecheck~.c`, `tap.quantize~.c`, etc.) — confirms a very
  early **1.0 beta 2b** existed and that early objects were plain C (not yet the
  TTBlue C++ era). [corroborated; music.mcgill.ca/~ich/classes/mumt306/]

### Tap.Tools 2 (April 2005) — the best-documented release
- **CDM Create Digital Music** review: *"Tap.Tools 2: Max/MSP/Jitter Construction
  Kit Gets Bigger,"* dated **April 2005** (URL: cdm.link/2005/04/…). [corroborated]
- Contents per CDM: "**over 150 externals**" for: audio effects (reverb, pitch
  shift, dynamics, vocoder, delays, envelope substitution); audio filtering
  (incl. **LFO-driven FFT filters**); Jitter graphics incl. **motion tracking**;
  **ADSR envelope generators**; buffer processing for loop record/playback; **XML
  file utilities**; MIDI mapping; random-number generators; **AppleScript loading
  (Mac)**; reusable interfaces for envelope generation and parameter control.
  [snippet — verify]
- **Pricing tiers: US$65–99.** At the **$99** tier you could "**build your own
  collectives, standalones, and plug-ins, and distribute your creations.**"
  [snippet — verify] — i.e., a developer/redistribution license.
- CDM verdict: "**an indispensible collection of time-savers at a really low
  price.**" [snippet — verify]
- Tap.Tools 2 was also later made **free** for Max 4.6 (per forum snippet). [snippet
  — verify]

### Tap.Tools 3 (Max 5 era) and 3.1 / 3.2
- v3 feature set (announcement-style): new **"TapTools Builder"** in the Extras menu
  to ease building standalones; **many audio objects now 64-bit internally** with
  **multichannel** processing; **full reference pages integrated with Max 5's
  searchable docs**, filebrowser/metadata, and the Max 5 inspector. [snippet —
  verify; cycling74 "Tap.Tools 3.2" thread + ReadMe]
- `tap.crossfade` became **multichannel** (first arg = number of channels). New
  object **`tap.inquisitor`** reads attribute values of another named object.
  [snippet — verify]
- **v3.1: installs became "completely untethered from Jamoma,"** resolving
  dependency issues; objects modularized as **separate Max external binaries**
  rather than one monolithic extension. [snippet — verify] — significant: at this
  point TapTools temporarily *decoupled* from Jamoma (it re-coupled at v4; see §below).
- **"TAP-TOOLS-PRO 3.1 @ MAX FOR LIVE"** thread on the electrotap Google Group —
  confirms a **"Pro"** edition / Max-for-Live targeting at 3.1. [confirmed URL;
  groups.google.com/g/electrotap/c/F3xOabWTZ-Y]
- **v3.2: "released into the wild," main focus = working well in Max for Live.**
  Download host: **`http://download.74objects.com/taptools/index.html`**. [snippet —
  verify; cycling74 "Tap-Tools 3.2" forum]

### Tap.Tools 4 (Max 6 / 6.1 era)
- **TapTools 4 beta 1: all code became open source on GitHub; no registration
  number required for authorization.** [snippet — verify; GitHub ReadMe]
- **TapTools 4 beta 2: distributed as a Max Package**, with **Jamoma dependencies
  provided via the package mechanism**; Max 6.1; **both 32- and 64-bit**; available
  via the **Package Downloader** or download.74objects.com; **requires OS X 10.7+**.
  [corroborated; "MAX 6 and Taptools 4" forum]
- **Required Jamoma 0.5.7** (which itself needs OS X 10.7+). [snippet — verify] — so
  v4 RE-coupled to Jamoma after the v3.1 decoupling. [inferred]
- Forum threads show ongoing user struggles: *"TapTools for Max 6.1 x64?"*,
  *"Max6.1 — problems within TapTools and LitterPro,"* *"Tap.Tools on max6.1.3 and
  osX.6.8?"* — i.e., the Jamoma dependency caused real install pain. [confirmed URLs]

### Catalogs / directory listings
- Listed in **MusicMoz → Computers/Software/Max_and_MSP/Patch_Libraries**. [confirmed]
- Recommended on **CNMAT's "Recommended Third Party Max-MSP-Jitter Softwares"** list
  ("Tim Place's Tap.Tools is listed among CNMAT's recommended 3rd party externals").
  [snippet — verify; cnmat.berkeley.edu + CNMAT-MMJ-Depot wiki]

**Conflict / open:** "100 objects" (1.x) vs "150 externals" (2.0) — both plausible
as the count grew; pin exact numbers from the ReadMes. No KVR or Sound on Sound
review of Tap.Tools surfaced (the SoS/KVR "tap" hits were tap-tempo, not Tap.Tools)
— Tap.Tools press appears to be **CDM-centric**, not the mainstream plugin press.

---

## 3. Hipno — deeper

### What it was / who made it
- **Hipno = a suite of "over forty" effect and instrument plug-ins** for **VST, AU,
  RTAS**, Mac OS X (Windows XP version followed). **Designed by Electrotap,
  published by Cycling '74.** [corroborated across KVR, MusicRadar, mixonline,
  Sweetwater, audiomasterclass]
- Mix of **granular, spectral, and filter/delay-based** plug-ins, plus instruments
  and modulators, with the signature **Hipnoscope UI**. [corroborated]
- Plug-ins grouped into **five categories: granular effects, spectral effects,
  delay/filter effects, instruments, and modulator/bus effects.** [snippet — verify;
  MusicRadar/audiomasterclass]

### The plug-in names (partial list, recoverable)
- **Deluge** — "a quartet of granular processing engines wrapped into one glittering
  package."
- **Technishypht** — "high-quality floating-formant spectral pitch-shifter, driven
  by a 16-step sequencer … able to sync to the host."
- **Shypht** — "pitch-shifting effects system enlivened by pitch quantization,
  filtering, and feedback looping."
- **Morphulescence** — "a cascaded bank of LFO-modulated morphing filters."
- **Vsynth** — "uses a live video feed as the spectral source for synth."
- Also named: **Drunken Sailor, LoopDeeLa, SquiglyQ.**
[all snippet — verify; audiomasterclass + KVR]
- A distinctive feature: **processors that use live video input as a control
  source** (Jitter-powered) — repeatedly highlighted as the most innovative bit.
  [corroborated]

### The Hipnoscope UI (in depth)
- An **interpolator/morphing interface**: morph between **up to eight presets
  ("snapshots")** at once. [snippet — verify]
- Mechanic: assign each snapshot a **color**, position/resize that color blob in the
  scope; presets appear as **colored spots on a circle**; Hipno interpolates between
  them (shown as shades). You **drag an on-screen "puck"** (or map modulation) to get
  a **weighted mix** of all snapshots under the puck — "swim through sound settings,"
  "explore spaces of presets as terrains." [snippet — verify; MusicRadar]
- Live use: control all parameters simultaneously with the mouse; constrain
  exploration to a **predesigned "interpolation space"** for complex, repeatable
  transitions. [snippet — verify]
- (This is the same lineage of "preset-interpolator" UI later seen in tools like
  Oli Larkin's pMix; useful comparison point.) [inferred]

### Control: two layers
1. **Hipnoscope Interpolator** (above).
2. **Modulators** that operate **between plug-ins on Hipno's own signal bus, the
   PluggoBus.** [corroborated]

### Pluggo dependency (ties to §4)
- Hipno was **built on Pluggo technology** and used the **PluggoBus** — a virtual
  audio bus (**eight independent channels**) to route audio between plug-ins
  independently of the host DAW. Signals go in via a **"PluggoBus Send"** plug-in.
  [snippet — verify] — i.e., Hipno required the Pluggo runtime to function.

### Pricing & dates
- **SRP US$199.** [snippet — verify; KVR]
- **[conflict] on dates:** one synthesis said "available Q1 2005"; others place
  the timeline as **announced ~Oct 2005 (Synthtopia "Cycling '74 Intros Hipno 1.0,"
  2005/10/19; AES announcement via mixonline alongside UpMix & Octirama)**, with
  **v1.0 release covered as "NAMM News" (Mix Online, NAMM is January)** → i.e.
  **announced late 2005, shipped at/around NAMM Jan 2006.** [conflict — resolve via
  the dated pages] Best current reading: **announced Oct 2005, released v1.0 Jan
  2006.** [inferred]
- **Version history (from KVR update headlines):** v1.0 → **v1.0.4** (updated
  alongside Pluggo 3.5.4 / Mode 1.2.4) → **v1.1** (Universal Binary for Intel Macs,
  **28 Sep 2006**, sonicstate) → **v1.1.1** (alongside Pluggo 3.6.1 / Mode 1.3.1 /
  UpMix 1.1.1). A **"Hipno 1.04 VST RTAS WiN"** also circulates on warez mirrors
  (magesy), confirming the Windows build. [corroborated]

### Reception
- **MusicRadar** (two review URLs exist: …/cycling74-hipno-22035 and …-23402,
  possibly print vs web): positive-for-the-adventurous. Quote: *"the plug-ins often
  sound bizarre and produce unpredictable results, but that's exactly why
  adventurous types will love this collection. You won't find such a broad sonic
  landscape anywhere else at this price."* Framed it as **cerebral / IDM-ambient-
  sound-art** territory, "not bread-and-butter." Praised the **video-control
  "boffinry"** as genuinely innovative. [snippet — verify]
- **audiomasterclass** "HIPNO Plug-In Suite from Cycling '74" — descriptive review.
- **[note — factual error in one snippet]:** one search synthesis called Electrotap
  **"Kansas-based"** — Electrotap was registered in **Kansas City, MISSOURI** (see
  §5/§6); some press said "Kansas-based" loosely. Flag when quoting.

### Discontinuation (May 2009) — see §4 for the full context
- Discontinued together with **Pluggo, Mode, and UpMix** in **May 2009**; now in
  Cycling '74's **"Downloads of Discontinued Products"** with legacy-authorization
  PDFs. [corroborated]

**Still thin:** full 40+ plug-in list; KVR *user* star-ratings; the Computer Music
magazine print review; the actual Hipno manual. Targets in §9.

---

## 4. Pluggo & the Cycling '74 plug-in ecosystem (context for Hipno)

### What Pluggo was
- A **run-time system that turns Max/MSP patches into plug-ins** — effectively a
  **VST wrapper for Max patches**, plus **AU** and **RTAS** wrappers, on **Windows
  XP and Mac OS X.** [corroborated; SoS + cycling74 docs]
- Shipped as a **collection of 100+ effects, instruments, and "modulation"
  plug-ins**, all authored in Max/MSP. [corroborated]
- "**Pluggo plug-ins can work together behind the back of your sequencer**" — via the
  **PluggoBus** (8-channel inter-plugin audio bus) and **Pluggo Sync** (host tempo
  sync beyond standard VST/AU). [snippet — verify]
- The **free Pluggo Runtime** let anyone *run* plug-ins built with Max/MSP → so a
  developer could build a Pluggo plug-in in Max/MSP and **deploy it for free
  anywhere** (Logic, FL Studio, etc.). [corroborated; CDM]

### The Pluggo family (all Pluggo-tech)
- **Pluggo** — the base 100+ effect/instrument collection.
- **Hipno** — granular/spectral/video creative suite (Electrotap).
- **Mode** — a bundle of plug-ins (KVR "Cycling '74 Mode announced"); part of the
  same family. [confirmed URL]
- **UpMix** — **surround / stereo-to-5.1 upmixing suite**; utilities **Rotator,
  ReRoute, FoldDown, ReBalance, LFE-6chan**; **UpMix installer placed plug-ins in
  the Pluggo folder structure** (i.e., Pluggo-dependent). [snippet — verify;
  mixonline AES announce + SoS "Surround Sound From Stereo"]
- Also in the legacy family: **Cyclops** (Eric Singer; video→MIDI), **Radial**,
  **Octirama** — all on the discontinued/legacy support pages. [snippet — verify]

### The May 2009 discontinuation
- **Date: announced ~May 15, 2009** (CDM "Cycling '74 Ditches Plug-in Development
  Support; Free + Commercial Alternatives," 2009-05-15). [corroborated]
- **David Zicarelli** announced Cycling '74 would **discontinue sales of all
  prebuilt Max-based plug-in packages — Pluggo, Mode, Hipno, UpMix — effective
  immediately**, with **no further development** on the packages or their supporting
  technology. [corroborated]
- Stated rationale: **not cost-effective to support three plug-in specs (VST/AU/RTAS)
  on two platforms**, amid declining host standardization. [snippet — verify]
- Strategic pivot: Pluggo technology / highlights **moved into Max for Live** (the
  Ableton collaboration), with Pluggo-inspired devices free for M4L users and
  discounts arranged with Ableton. **CDM's critique:** Max for Live is a *paid*,
  Live-only path, whereas Pluggo let you deploy free plug-ins into *any* host — a
  real loss of openness. [corroborated]
- Today: **cycling74.com/downloads/discontinued** hosts installers for owners;
  **support.cycling74.com** has "Cyclops, Hipno, Mode, Pluggo" + "Legacy Product
  Authorization" articles and a **faq_pluggo.pdf** on S3. [confirmed URLs]

---

## 5. Teabox & Electrotap hardware

### The Teabox
- **Teabox = a sensor-data interface** by Electrotap (Tim Place). Its defining
  trick: instead of converting sensor voltages to **MIDI** (like cheap rivals), it
  **converts them to digital audio and ships them over an S/PDIF cable.** [corroborated]
- **Specs:** digitizes **up to 24 inputs — 8 continuous analog inputs at 12-bit
  resolution + 16 digital on/off (toggle) inputs.** Sensor data is **multiplexed
  onto the digital-audio (S/PDIF) line and demultiplexed in software** on the
  computer. Benefits cited: **robust, very low latency, high resolution.**
  [corroborated; ICMC2004 paper text via quod.lib.umich.edu + ResearchGate]
- **Software:** ships with **Mac + Windows decoder objects for Max/MSP**, plus
  **standalone bridge apps that re-emit the data as MIDI or OpenSound Control** to
  other applications. [snippet — verify]
- **Model/price:** **"T100A Teabox Sensor Interface, US$425."** [snippet — verify; CMJ review]

### Authorship / papers
- **ICMC 2004 paper: "Teabox: A Sensor Data Interface System," by Jesse T. Allison
  and Timothy A. Place.** [corroborated; quod.lib.umich.edu ICMC archive +
  ResearchGate + Academia.edu] → confirms **Place + Jesse Allison** as co-authors/
  co-designers (answering the task's "Place + Jesse Allison?" question — **yes**).
- **Computer Music Journal review: CMJ Vol. 29, No. 3 (2005), pp. 104–106, by
  Barry Moon.** [corroborated; direct.mit.edu/comj/29/3/104 + muse.jhu.edu/188577 +
  computermusicjournal.org/reviews/29-3/moon-teabox.html] — three independent
  pointers to the same review; author **Barry Moon** is a new, confirmed detail.
- **Open-sourced later:** **github.com/electrotap/Teabox** — "open source easy-to-use
  hardware/software sensor systems," with releases and a `_decoders/max4live/`
  folder (`74Objects.xcconfig`) — shows the Teabox decoders were eventually rebuilt
  for **Max for Live** under the **74 Objects** banner. [confirmed URL]
- **Trond Lossius** wrote a Teabox article (trondlossius.no/articles/…2004-07-19),
  dated **19 July 2004** — useful as a contemporaneous user account. [confirmed URL]

### Electrotap the company
- **"Electrotap is a company formed by Tim Place and Jesse Allison."** "Electrotap
  L.L.C., based in **Kansas City, Missouri**, develops, builds, and distributes
  software and hardware tools to create innovative music and art." [snippet —
  verify] — confirms **Jesse Allison as co-founder** and **Kansas City, MO** HQ.
- Product line under Electrotap: **Tap.Tools** (Max/MSP/Jitter externals), **Jade**
  (free performance environment), **Teabox** (hardware), and **Hipno** (designed by
  Electrotap, published by Cycling '74). [corroborated]
- Later there was an **"Electrotap CyberMonday Sale"** (cycling74 Misc forum) and an
  **electrotap Google Group** used for support/announcements. [confirmed URLs]
- Note the brand evolution: **Electrotap → 74 Objects LLC** (download host
  `download.74objects.com`, GitHub `74Objects.xcconfig`, the "74objects" WordPress).
  Round 1 covered the Silicon Prairie Intermedia → Electrotap → 74 Objects chain.

**Still thin:** Teabox enclosure photos, the full sensor-kit/accessory list,
production run / how many sold, and other Electrotap hardware (was Teabox the only
shipping hardware product? Search suggests **yes, Teabox was the hardware product**,
but unconfirmed). Targets in §9.

---

## 6. Timothy Place — interviews, talks & career timeline

### The Cycling '74 interview
- **"An Interview With Tim Place"** exists at **cycling74.com/articles/an-interview-
  with-tim-place** — described as covering "his work as a developer and artist,
  charting the numerous development projects." [confirmed URL; full text not
  recoverable via search — high-priority fetch target]
- Related C74 articles: **"The Development of Jamoma"** (cycling74.com/articles/the-
  development-of-jamoma) and the **"Jamoma" project page** (cycling74.com/projects/
  jamoma). [confirmed URLs] — these carry the Jade/Jamoma history quoted in §1.

### Darwin Grosse "Art + Music + Technology" podcast
- The podcast (**Art + Music + Technology**, by **Darwin Grosse**) interviews people
  at the art/music/tech intersection; **Jamoma's site has a "Darwin Grosse"
  category** (jamoma.org/categories/Darwin%20Grosse/), strongly implying a Jamoma/
  Place episode was cross-posted. [confirmed URL]
- Search **did not return a definitive episode number/date for Tim Place.** One
  synthesis loosely associated a **2016** timeframe but that was not reliably tied
  to a Place episode. [conflict/unverifiable] — **Do not assert an episode number
  without checking.** Target in §9: enumerate the AMT episode list and grep for
  "Place" / "Jamoma."
- (For scale/context: the series ran to ~200+ episodes, e.g. "Podcast 188 | Markus
  Reuter," before Grosse stopped for health reasons.) [snippet — verify]

### Career timeline (confirmed via LinkedIn/TheOrg/ZoomInfo snippets)
- **Education / award:** received the **Entrepreneur of the Year award from the
  University of Missouri–Kansas City (UMKC)** for founding **Electrotap** "while
  working on his doctorate in electronic music." [corroborated — appears in multiple
  syntheses] → ties Electrotap's founding to his **UMKC doctoral** period.
- **Cycling '74:** **Research Engineer for 15+ years**; also **Software Architect on
  the open-source Jamoma project.** [corroborated]
- **Ableton:** "spent 15 years … with **Cycling '74 and Ableton**." **Ableton
  acquired Cycling '74 in June 2017** [corroborated; Wikipedia], so his Ableton
  affiliation flows from that acquisition (he continued in audio/DSP through the
  C74→Ableton period). Exact Ableton title/dates not separately pinned. [inferred]
- **Garmin:** **Technical Lead Software Engineer, Audio & DSP, since October 2020**,
  leading **next-gen automotive audio for Garmin's Automotive OEM business** —
  "moving to the automotive industry." [corroborated; LinkedIn/TheOrg]
- LinkedIn handles: **linkedin.com/in/timothyaplace/** (matches the user's own
  email/identity) and a second **/in/timothy-place-1a406b9/**. GitHub: **github.com/
  tap** (TapTools) and **github.com/electrotap** (Teabox). X/Twitter: **@timothyplace**.

**Career summary (best current reconstruction):**
~2001 Jade dev begins (UMKC doctoral work) → 2003 Electrotap (w/ Jesse Allison),
TTBlue, Teabox → 2003–2006 Tap.Tools/Teabox/Hipno commercial era → Jamoma
architect (2005–) → **Cycling '74 Research Engineer (~15 yrs)** → **Ableton** (post-
June-2017 acquisition) → **Garmin, Oct 2020–present.** [inferred from corroborated pieces]

---

## 7. The McGill Max listserv / mailing-list archives

- **max-archive.bek.no** hosts the restored **Max mailing-list archive for
  1997–1999** (Bergen Senter for Elektronisk Kunst). [corroborated]
- Provenance: **"The Max list was run by Christopher Murtagh at McGill University
  for a number of years in the 90s," with "Jon Witte helping maintain an archive
  for 1997–1999."** [corroborated; bek.no "Restoring old Max list archives" +
  cycling74 "Old mailing list archives restored"] — confirms round 1's Murtagh/
  McGill→BEK identification, and adds **Jon Witte** as the archivist.
- **Key limitation for our topic:** the restored span is **1997–1999** — i.e.,
  **before Jade (2001), TTBlue (2003), and Tap.Tools (2003)**. So **direct Tap.Tools
  / TTBlue / 74objects mentions are NOT expected in the BEK archive** (the
  technology postdates it). Searches for those terms inside the archive returned
  nothing topical. [corroborated/inferred]
- **Implication:** the listserv is valuable for *Max community context circa
  late-90s* (the world Place entered), not for Tap.Tools primary sources. Tap.Tools-
  era discussion lives instead on the **cycling74.com forums** and the **electrotap
  Google Group** (the real primary community channels — round 1 catalogued many).
- **Other lists:** no Pd-list or other mailing-list hits for Tap.Tools/Place
  surfaced via search worth recording. (A `chuck-users` Princeton hyperkitty hit was
  a false positive.) [inferred]

---

## 8. Current footprint — "is it still used?" (2018–2026)

- **Status of the product/site:** multiple syntheses state **"the electrotap website
  doesn't exist anymore"** and that the **download.74objects.com/taptools/** host
  "may no longer be active." [snippet — verify] The living artifact is the **GitHub
  repo `tap/TapTools`** (with a **Releases** page), but it is **"not been updated in
  a while"** and **"may not work for newer versions of Max."** [snippet — verify]
- **Still recommended on curated lists:** Tap.Tools appears on **CNMAT's recommended
  third-party externals** list and in the **CNMAT-MMJ-Depot wiki**, and in
  **MusicMoz**. [corroborated] — i.e., it retains "canonical externals library"
  status in reference lists even if effectively unmaintained.
- **Legacy-externals preservation:** **github.com/v7b1/max-thirdParty_externals** —
  "a collection of legacy third party externals for MaxMSP, developed by various
  people" — is the kind of community archive where TapTools-era objects get
  preserved. Worth checking whether tap.* objects are bundled there. [confirmed URL]
- **People still asking for it:** the cycling74 thread **"where can I find tap
  tools?"** is the canonical "where do I download this now" thread; round 1 also
  logged "about tap tools," "Tap.Tools include on windows?", "TapTools for Max 6.1
  x64?". These persist as the recurring user questions. [confirmed URLs]
- **Naming-collision noise:** "tap tools" now overwhelmingly returns **hardware
  tapping tools** and an Android app "Tap Tools" — a real discoverability problem
  for the historical software. [observed] Use **"tap.tools" / "Tap.Tools" / "TapTools
  Max"** for clean results.
- **Academic/teaching use:** **McGill MUMT306** historically used Tap.Tools (source
  files still hosted on music.mcgill.ca under `~ich/classes/mumt306/`); current
  MUMT306 (Prof. Gary Scavone) syllabus emphasizes Max/Pd/Arduino/C++ generally —
  **no evidence Tap.Tools is still assigned**, but its presence on McGill servers is
  a durable footprint. [corroborated for historical; current use unconfirmed]
- **No fresh (2021–2026) Reddit/forum endorsements** surfaced specifically — search
  could not confirm active community recommendation in that window (only the older
  CNMAT/MusicMoz canonical listings persist). [unverifiable via search — would need
  a direct r/MaxMSP search locally]

---

## 9. Wayback / primary-source target list (EXPANDED — for a local fetch pass)

Run these locally (WebFetch/Wayback) since they're 403 here. Grouped by thread;
**bold = highest priority.** Builds on round 1's list (items 1–26 in file 06).

### Jade
1. **`http://archives.didascalie.net/tiki-index.php?page=about+Jade`** — the richest
   Jade description (modules, matrix, address-per-variable, events, space-bar
   triggers). Capture verbatim + any screenshots.
2. `https://www.dontcrack.com/freeware/downloads.php/id/2774/software/Jade/` — Jade
   blurb + (maybe) the actual download / version.
3. **Wayback `electrotap.com/jade/`** (and `/taptools/`, `/teabox/`, `/hipno/`) —
   recover the real Jade product page, module list, v1.1 changelog, screenshots.
4. `https://cycling74.com/articles/the-development-of-jamoma` — Jade→Jamoma history,
   the "shrimp on the West coast of Norway" anecdote, SourceForge dates.

### Tap.Tools
5. **`https://cdm.link/2005/04/taptools-2-maxmspjitter-construction-kit-gets-bigger/`**
   — full Tap.Tools 2 review: exact object count, the $65/$99 tiers, the verdict quote.
6. `https://jiyounkang.com/wordpress/index.php/archives/229` — "Tap.Tools Update"
   blog (the "100 objects / v2.0 direction" note); date it.
7. **`https://github.com/tap/TapTools/blob/master/ReadMe.txt`** and `/releases` —
   authoritative version history, object inventory, the v3.1 "untethered from
   Jamoma" and v4 "open source / Max Package" notes.
8. `https://cycling74.com/forums/tap-tools-3-2` — the 3.2 announcement (download URL,
   Max-for-Live focus).
9. `https://cycling74.com/forums/max-6-and-taptools-4` — TapTools 4 beta details
   (Jamoma 0.5.7, 32/64-bit, OS X 10.7).
10. `https://groups.google.com/g/electrotap/c/F3xOabWTZ-Y` — "TAP-TOOLS-PRO 3.1 @
    MAX FOR LIVE" (confirms a "Pro" edition).
11. **Wayback `http://download.74objects.com/taptools/index.html`** — the real
    download index: full version list, system requirements, possibly the installers.
12. `https://cnmat.berkeley.edu/content/recommended-third-party-max-msp-jitter-softwares`
    + `https://github.com/CNMAT/CNMAT-MMJ-Depot/wiki/Recommended-3rd-Party-Externals`
    — confirm Tap.Tools' wording/placement.

### Hipno / Pluggo ecosystem
13. **`https://www.musicradar.com/reviews/tech/cycling74-hipno-23402`** AND
    **`…/cycling74-hipno-22035`** — full review text, rating, pros/cons (two URLs —
    check if distinct).
14. `https://www.audiomasterclass.com/blog/hipno-plug-in-suite-from-cycling-74` —
    the five-category breakdown + plug-in names (Deluge, Shypht, Morphulescence,
    Vsynth, Drunken Sailor, LoopDeeLa, SquiglyQ).
15. `https://www.kvraudio.com/news/cycling_74_releases_hipno_v1_0_over_40_plug_ins_4116`
    and `…announces_hipno_2897` — official announcement, price, full plug-in list,
    **resolve the 2005-vs-2006 date conflict**.
16. `https://www.synthtopia.com/content/2005/10/19/cycling-74-intros-hipno-10/` —
    dated **2005-10-19** intro (anchors the announce date).
17. `https://www.mixonline.com/technology/namm-news-cycling-74-releases-hipno-plug-suite-380959`
    and `…aes-products-…-381588` — NAMM/AES dates (release vs announce).
18. `https://sonicstate.com/news/2006/09/28/cycling-74-plug-ins-go-universal-binary/`
    — Hipno 1.1 / Universal Binary, **28 Sep 2006**.
19. **`https://cdm.link/cycling-74-ditches-plug-in-development-support-free-commercial-alternatives/`**
    — the **2009-05-15** discontinuation analysis (Zicarelli quote, rationale, M4L pivot).
20. `https://www.kvraudio.com/news/cycling_74_discontinues_pluggo_moves_pluggo_technology_to_max_for_live_11565`
    — discontinuation announcement.
21. `https://cycling74.com/downloads/discontinued` and
    `https://support.cycling74.com/hc/en-us/articles/360050994953-Cyclops-Hipno-Mode-Pluggo`
    + `https://cycling74.s3.amazonaws.com/support/webpages/faq_pluggo.pdf` — legacy
    downloads/installers + Pluggo FAQ (PluggoBus mechanics, version list).
22. `https://www.soundonsound.com/reviews/cycling-74-pluggo` — SoS Pluggo review
    (ecosystem context). Also `…/surround-sound-stereo` for UpMix.
23. **Hipno manual / "Getting Started"** PDF (look on cycling74 S3 like the Pluggo
    ones: `cycling74.s3.amazonaws.com/download/…`) — for the FULL 40+ plug-in list +
    Hipnoscope docs. (`Pluggo35GettingStarted.pdf` confirms the S3 pattern exists.)

### Teabox / Electrotap
24. **`https://quod.lib.umich.edu/i/icmc/bbp2372.2004.082/`** — ICMC 2004 Teabox
    paper full text (Allison & Place): architecture, S/PDIF multiplexing, sensor specs.
25. **`http://www.computermusicjournal.org/reviews/29-3/moon-teabox.html`** (and
    `https://direct.mit.edu/comj/article/29/3/104/93983/` / `muse.jhu.edu/article/188577`)
    — Barry Moon's CMJ 29(3) review: the $425 price, specs, verdict.
26. `https://trondlossius.no/articles/2004-07-19-teabox.html` — contemporaneous
    (2004-07-19) user account.
27. `https://github.com/electrotap/Teabox` (+ `/releases`, `_decoders/max4live/`) —
    open-sourced hardware/decoders; check the README for design credits & history.
28. **Wayback `electrotap.com` front page + `/teabox/`** — company "about," product
    line, the Place+Allison founding story, any other hardware, the KC-MO address.

### Tim Place / interviews
29. **`https://cycling74.com/articles/an-interview-with-tim-place`** — full interview
    (career arc, Jade/Jamoma/TTBlue in his own words). HIGH priority.
30. **Art + Music + Technology episode index** — find the **Tim Place / Jamoma
    episode** (number, date). Start at `https://jamoma.org/categories/Darwin%20Grosse/`,
    then the AMT feed (player.fm/listennotes/Apple Podcasts) — search "Place"/"Jamoma".
31. `https://www.linkedin.com/in/timothyaplace/` — exact C74/Ableton/Garmin dates &
    titles (to firm up the timeline; needs login, but Wayback may have a snapshot).

### Mailing lists / community
32. `http://max-archive.bek.no/` + `https://bek.no/en/restoring-old-max-list-archives/`
    — confirm 1997–1999 span (so the book can correctly say the listserv predates
    Tap.Tools); note Murtagh + Jon Witte.
33. `https://github.com/v7b1/max-thirdParty_externals` — check whether tap.* objects
    are preserved/bundled there (current-footprint evidence).

### Catalog / footprint
34. **Wayback snapshots of `electrotap.com/taptools/`** across years (2004–2012) —
    best single source for per-version pricing, object counts, and changelogs.
35. `https://musicmoz.org/Computers/Software/Max_and_MSP/Patch_Libraries/` — the
    MusicMoz catalog blurb (verbatim).

---

## 10. New/upgraded facts vs. Round 1 (quick diff for the book)

- **Jade**: address-per-variable + event automation + matrix; "shrimp on the West
  coast of Norway" Lossius anecdote (2003 idea / 2005 real start); SourceForge
  hosting; activity died then revived March 2005. [new detail]
- **Tap.Tools 2**: confirmed **April 2005**, **150+ externals**, **$65–$99 tiers**,
  CDM "indispensible … time-savers" verdict. [new specifics]
- **Tap.Tools 3.1**: **decoupled from Jamoma** ("completely untethered"), modularized
  into separate binaries; a **"PRO"** edition existed. **v4 re-coupled** to Jamoma
  0.5.7 and went **open-source on GitHub**. [new — the coupling/decoupling arc]
- **Hipno**: 5 categories; named plug-ins (Deluge/Technishypht/Shypht/Morphulescence/
  Vsynth/Drunken Sailor/LoopDeeLa/SquiglyQ); **Hipnoscope = 8-snapshot color/puck
  interpolator**; **$199**; built on **PluggoBus (8 ch)**; version chain 1.0→1.0.4→
  1.1(UB, 28-Sep-2006)→1.1.1; **MusicRadar verdict** captured. [new detail]
- **Discontinuation**: precise — **~May 15 2009**, **David Zicarelli**, Pluggo+Mode+
  Hipno+UpMix together, 3-spec/2-platform rationale, Max-for-Live pivot, CDM's
  openness critique. [new specifics]
- **Teabox**: **ICMC 2004 paper by Jesse T. Allison + Timothy A. Place**; **CMJ
  29(3) review by Barry Moon**; **$425 (T100A)**; 8×12-bit + 16 toggles over S/PDIF;
  MIDI/OSC bridge apps; later **open-sourced (github.com/electrotap/Teabox)** with a
  Max-for-Live decoder. [new — author names, price, reviewer]
- **Place career**: **UMKC Entrepreneur-of-the-Year for founding Electrotap during
  his electronic-music doctorate**; **C74 Research Engineer ~15 yrs**; **Ableton via
  the June-2017 acquisition**; **Garmin automotive audio since Oct 2020**. [firmed up]
- **Electrotap**: co-founded by **Tim Place + Jesse Allison**, **Kansas City,
  Missouri**. [confirmed co-founder + location]
- **Listserv caveat**: BEK archive is **1997–1999 only** → predates Tap.Tools; it's
  context, not a Tap.Tools primary source. [important correction/clarification]

---

### All source URLs surfaced this round (via WebSearch; none opened — 403)
- http://archives.didascalie.net/tiki-index.php?page=about+Jade
- https://www.dontcrack.com/freeware/downloads.php/id/2774/software/Jade/
- https://cycling74.com/articles/the-development-of-jamoma
- https://cycling74.com/articles/an-interview-with-tim-place
- https://cycling74.com/projects/jamoma
- https://www.jamoma.org/about/team/ ; https://jamoma.org/blog/archive/
- https://jamoma.org/categories/Darwin%20Grosse/
- https://jamoma.org/publications/attachments/jamoma-icmc2006.pdf
- https://cdm.link/2005/04/taptools-2-maxmspjitter-construction-kit-gets-bigger/
- https://cdm.link/tag/pluggo/
- https://cdm.link/cycling-74-ditches-plug-in-development-support-free-commercial-alternatives/
- https://jiyounkang.com/wordpress/index.php/archives/229
- https://github.com/tap/TapTools ; …/releases ; …/blob/master/ReadMe.txt
- https://cycling74.com/forums/tap-tools-3-2
- https://cycling74.com/forums/max-6-and-taptools-4
- https://cycling74.com/forums/taptools-for-max-6-1-x64
- https://cycling74.com/forums/max6-1-problems-within-taptools-and-litterpro
- https://cycling74.com/forums/tap-tools-on-max6-1-3-and-osx-6-8
- https://cycling74.com/forums/where-can-i-find-tap-tools
- https://groups.google.com/g/electrotap/c/F3xOabWTZ-Y
- http://download.74objects.com/taptools/index.html
- https://cnmat.berkeley.edu/content/recommended-third-party-max-msp-jitter-softwares
- https://github.com/CNMAT/CNMAT-MMJ-Depot/wiki/Recommended-3rd-Party-Externals
- https://musicmoz.org/Computers/Software/Max_and_MSP/Patch_Libraries/
- https://github.com/v7b1/max-thirdParty_externals
- https://www.kvraudio.com/news/cycling_74_announces_hipno_2897
- https://www.kvraudio.com/news/cycling_74_releases_hipno_v1_0_over_40_plug_ins_4116
- https://www.kvraudio.com/news/cycling_74_updates_pluggo_v3_5_4_mode_v1_2_4_and_hipno_v1_0_4_4999
- https://www.kvraudio.com/news/cycling_updates_pluggo_to_v3_6_1_mode_v1_3_1_hipno_v1_1_1_upmix_v1_1_1_6275
- https://www.kvraudio.com/news/cycling_74_updates_pluggo_mode_hipno_and_max_msp_4391
- https://www.kvraudio.com/news/cycling_74_discontinues_pluggo_moves_pluggo_technology_to_max_for_live_11565
- https://www.kvraudio.com/news/cycling_74_mode_announced_904
- https://www.musicradar.com/reviews/tech/cycling74-hipno-23402
- https://www.musicradar.com/reviews/tech/cycling74-hipno-22035
- https://www.audiomasterclass.com/blog/hipno-plug-in-suite-from-cycling-74
- https://www.synthtopia.com/content/2005/10/19/cycling-74-intros-hipno-10/
- https://www.synthtopia.com/content/tag/hipno/
- https://www.mixonline.com/technology/namm-news-cycling-74-releases-hipno-plug-suite-380959
- https://www.mixonline.com/technology/aes-products-cycling-74-announces-upmix-software-package-hipno-plug-ins-and-octirama-381588
- https://www.mixonline.com/technology/cycling-74-releases-plug-products-intel-based-macs-382403
- https://sonicstate.com/news/2006/09/28/cycling-74-plug-ins-go-universal-binary/
- https://www.sweetwater.com/store/detail/Hipno--cycling-74-hipno
- https://www.hitsquad.com/smm/programs/Hipno/
- https://www.soundonsound.com/reviews/cycling-74-pluggo
- https://www.soundonsound.com/reviews/surround-sound-stereo
- https://cycling74.com/downloads/discontinued
- https://support.cycling74.com/hc/en-us/articles/360050994953-Cyclops-Hipno-Mode-Pluggo
- https://support.cycling74.com/hc/en-us/articles/360051132273-Legacy-Product-Authorization
- https://cycling74.s3.amazonaws.com/support/webpages/faq_pluggo.pdf
- https://cycling74.s3.amazonaws.com/download/Pluggo35GettingStarted.pdf
- https://www.gearjunkies.com/2009/05/cycling74-pluggo-technology-moves-to-max-for-live/
- https://quod.lib.umich.edu/i/icmc/bbp2372.2004.082/
- https://www.academia.edu/99276641/Teabox_A_Sensor_Data_Interface_System
- https://www.researchgate.net/publication/221165041_Teabox_A_Sensor_Data_Interface_System
- https://direct.mit.edu/comj/article/29/3/104/93983/Electrotap-Teabox-Sensor-Interface
- http://www.computermusicjournal.org/reviews/29-3/moon-teabox.html
- https://muse.jhu.edu/article/188577/summary
- https://trondlossius.no/articles/2004-07-19-teabox.html ; http://trondlossius.no/articles/280-teabox
- https://github.com/electrotap/Teabox ; …/releases
- https://www.linkedin.com/in/timothyaplace/ ; https://www.linkedin.com/in/timothy-place-1a406b9/
- https://theorg.com/org/garmin/org-chart/timothy-place
- https://en.wikipedia.org/wiki/Cycling_'74
- http://max-archive.bek.no/ ; https://bek.no/en/restoring-old-max-list-archives/
- https://cycling74.com/forums/old-mailing-list-archives-restored
- https://www.music.mcgill.ca/~gary/306/ ; http://www.music.mcgill.ca/~ich/classes/mumt306/
- http://tap-tools.sharewarejunction.com/ ; http://www.softwaresea.com/Windows/download-Tap-Tools-10147496.htm (junk mirrors — version evidence only)
