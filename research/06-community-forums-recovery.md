# 06 — Community & Forums Recovery (TapTools / TTBlue / Jamoma)

Web-search-driven archival recovery of the TapTools community footprint in the
Max/MSP world: forums, mailing lists, and reception. Phase-1 reconnaissance —
the deliverable is both what was recovered via search **and** a map of where the
richest archival material lives for a later (Wayback/forum-logged-in) phase.

Research date: 2026-06-19. Researcher: archival recovery pass for the TapTools book.

---

## 0. Web-access status (READ FIRST)

- **WebFetch is broadly BLOCKED in this environment.** Every page fetch attempted
  returned **HTTP 403 Forbidden**, including:
  - `http://www.music.mcgill.ca/~ich/classes/mumt306/.../ReadMe-Tap.Tools-beta2b`
  - `http://max-archive.bek.no/`
  - `https://musicmoz.org/Computers/Software/Max_and_MSP/Patch_Libraries/`
  - `https://74objects.wordpress.com/2009/04/15/the-jamoma-platform/`
- **Wayback Machine (web.archive.org) is hard-blocked** (per task brief; not retried).
- **WebSearch works** and is the workhorse here. It returns synthesized snippets
  plus real URLs, and frequently surfaces page *content* even when the page itself
  cannot be fetched. **All substantive facts below are tagged `[snippet — verify]`**
  unless otherwise noted; they come from search-result synthesis, not from reading
  the source page directly. A Phase-2 session with working fetch/Wayback should
  re-confirm quotes and dates against the live/archived pages.

---

## 1. The McGill Max/MSP listserv — IDENTITY CONFIRMED (key pre-2014 primary source)

This was the brief's top priority, and it resolved cleanly.

**What it was** `[snippet — verify]`
- The long-running Max mailing list was **run by Christopher Murtagh at McGill
  University** through "a number of years in the 90s." This is the McGill listserv
  the author remembered.
- The McGill-era list predates the Cycling '74 web forums and was *the* primary
  community discussion venue during the period when **MSP was first released
  (1997–1999)** — exactly the formative era for audio externals on Max.
- That window also covers the **Netochka Nezvanova** turbulence on the list
  (the infamous "should she stay or should she go" community vote), a well-known
  episode that helps date and characterize the list culture.

**Where the archive lives now — THE FIND** `[snippet — verify]`
- The McGill list archives for **1997–1999** were restored and are hosted by
  **BEK (Bergen senter for elektronisk kunst / Bergen Electronic Arts Centre)** at:
  - **`http://max-archive.bek.no/`** — "Max Mailing List Archives." Presents
    itself as a chronology of the various Max mailing lists/forums over time and
    hosts archives of **discontinued** lists. Browsable **by month and by thread**
    (standard mailing-list-archive layout).
  - BEK's announcement/explainer page: **`https://bek.no/en/restoring-old-max-list-archives/`**
- Restoration was done **by Jon Witte**, following a discussion **on the current
  Max mailing list hosted by BEK**. (So BEK runs both the historical archive *and*
  a present-day Max list.)
- Cross-announcement on the Cycling '74 forum:
  **`https://cycling74.com/forums/old-mailing-list-archives-restored`**
  ("Old mailing list archives restored").

**Significance for the book**
- This is a genuine, publicly hosted, **searchable/quotable primary source** for
  late-90s Max community discourse — the substrate TapTools (Tap.Tools 1.0 beta,
  2002–era) grew out of. Note: the BEK archive's *confirmed* span is **1997–1999**,
  which is **earlier** than TapTools' public life (Tap.Tools 1.0b ≈ 2002+). So
  the BEK archive is most useful for *context* (the world Tim Place entered), and
  possibly for early posts by Place himself, rather than for TapTools release chatter.
- **Caveat to verify in Phase 2:** confirm whether BEK also archives *later* list
  segments (2000s) or only 1997–1999, and whether full-text search exists or only
  by-month/by-thread browsing.

**Related "where did the list go" forum threads (Cycling '74) to mine later** `[snippet — verify]`
- `https://cycling74.com/forums/ot-mailing-list-question` — "[OT] Mailing list question"
- `https://cycling74.com/forums/ot-list-archive` — "[OT] list archive"
- These likely contain pointers/history of the McGill → BEK → Cycling '74-forum migration.

**Other McGill connection (separate from the listserv) — academic adoption**
- TapTools shows up in **McGill music-tech course materials**: the path
  `http://www.music.mcgill.ca/~ich/classes/mumt306/Tap.Tools_1.0b2b/ReadMe-Tap.Tools-beta2b`
  is the **MUMT 306** class directory (Ichiro Fujinaga's course; `~ich`), which
  hosted a copy of **Tap.Tools 1.0 beta 2b** with its ReadMe. This is concrete
  evidence of **TapTools being used as university courseware** in the Max/MSP
  pedagogy world. (Page is 403 here; recover ReadMe text in Phase 2 — it likely
  lists the early object set and Place's contact/URL.)

---

## 2. Cycling '74 forums — discoverable TapTools/TTBlue threads

Cannot fetch pages (403), but search reliably surfaced thread titles, URLs, and
gist. **All thread URLs verified to exist via search; content gist is
`[snippet — verify]`.** Catalog of recovered threads:

**TapTools release / availability / version threads**
- `https://cycling74.com/forums/tap-tools-3-2` — **"Tap.Tools 3.2"**. Gist: the
  RC and the release were the same build; **Intel-only**. Users ask about Windows.
- `https://cycling74.com/forums/tap-tool-in-4-6` — **"tap.tool in 4.6"**. Gist:
  installing tap objects in **Max/MSP 4.6**; a user notes the `.shlb` external had
  to be moved to the **`library/CFMsupport`** folder to load (classic Mac
  Carbon/CFM detail — useful period texture).
- `https://cycling74.com/forums/taptools-for-max-6-1-x64` — **"TapTools for Max 6.1 x64?"**
  — later-era compatibility question (64-bit Max 6.1).
- `https://cycling74.com/forums/where-can-i-find-tap-tools` — **"where can I find tap tools?"**
- `https://cycling74.com/forums/about-tap-tools` — **"about tap-tools"**
- `https://cycling74.com/forums/tap-tools-include-on-windows` — **"Tap.Tools include on windows ?"** (Misc forum)

**Bug / usage threads**
- `https://cycling74.com/forums/bug-in-tap-tools-tap-pan` — **"Bug in tap.tools (tap.pan~)"**
  — concrete object-level discussion of `tap.pan~`.

**External-development threads (the C++ / TTBlue angle the author flagged)**
- `https://cycling74.com/forums/max-externals-in-c` — **"MAX externals in C++"** (Dev forum).
  Gist `[snippet — verify]`: TTBlue described as a **GNU LGPL C++ library of mainly
  DSP methods by Tim Place**, used for tap.tools and several Jamoma externals;
  ttblue and Jamoma both contain examples of how to build Max externals from it.
- `https://cycling74.com/forums/building-a-filter` — **"Building a Filter"**.
  Gist `[snippet — verify]`: advice to **check out the active branch of TTBlue
  via SVN** (with Xcode + MaxMSP SDK) as a worked filter example. (Confirms TTBlue
  was distributed via **Subversion** — an archival target: find the SVN host.)

**Reception signal (weak but present)** `[snippet — verify]`
- Search surfaced phrases like **"excellent tap-tools"** and thanks directed to
  Tim re: a **TapTools 4 beta 2**, plus the candid note that **"In the case of
  Tap.Tools, those resources are extremely limited"** (i.e., late-life support was
  thin). These are fragments — Phase 2 should pull the surrounding posts for quotable
  reception.

**The Tim Place interview (primary, high value)**
- `https://cycling74.com/articles/an-interview-with-tim-place` — Cycling '74
  editorial interview with Place covering his developer/artist work (Electrotap,
  Jade, Max for Live, etc.). **Top-priority fetch for Phase 2** — likely the single
  best first-person source on motivation/history. (403 here.)

---

## 3. Other community venues & reception (era map)

**MusicMoz (Open Directory–style catalog)** `[snippet — verify]`
- `https://musicmoz.org/Computers/Software/Max_and_MSP/Patch_Libraries/` — the
  "Patch Libraries" category the author mentioned. Parent:
  `https://musicmoz.org/Computers/Software/Max_and_MSP/`.
- How TapTools was catalogued (from snippets): **Tim Place's "Tap.Tools," a build
  for Max/MSP/Jitter**, listed alongside **"Jade," an interactive performance
  environment** ("Tim Place – Interactive Music"). Catalog framing: Electrotap's
  bundle of Max objects; **US$65–99 license** (the $99 tier allowing building
  collectives/standalones/plug-ins). Exact catalog blurb needs Phase-2 fetch.

**CDM (Create Digital Music) — press coverage** `[snippet — verify]`
- **`https://cdm.link/2005/04/taptools-2-maxmspjitter-construction-kit-gets-bigger/`**
  (also resolves at `https://cdm.link/taptools-2-maxmspjitter-construction-kit-gets-bigger/`)
  — **"Tap.Tools 2: Max/MSP/Jitter Construction Kit Gets Bigger,"** dated
  **April 2005**. This is a real contemporaneous press hit. Snippet content:
  Tap.Tools 2.0 = **"over 150 externals"** for audio effects (reverb, pitch shift,
  dynamics, vocoder, delays, envelope substitution), filtering (LFO-driven FFT
  filters), and **Jitter graphics incl. motion tracking**; plus ADSR generators,
  buffer/loop processing, XML utilities, MIDI mapping, RNGs, AppleScript loading
  (Mac), and reusable UI for envelopes/parameters. Framed as time-saving
  pre-built building blocks. **Fetch full article in Phase 2** (good quotable review).
- Related CDM roundups likely mentioning it:
  - `https://cdm.link/maxmsp-resource-roundup-computer-music-special-updated/`
    — "Max/MSP Resource Roundup" (check for TapTools listing).
  - `https://cdm.link/tag/maxmspjitter/` — CDM Max/MSP/Jitter tag index.

**Pure Data world (TTBlue's Pd usability)** `[snippet — verify]`
- TTBlue is cross-platform C++ DSP; the cross-Pd/Max angle in the ecosystem runs
  through **flext** (grrrr.org / Thomas Grill): `https://github.com/grrrr/flext`,
  `https://grrrr.org/research/software/flext/`, `https://puredata.info/community/pdwiki/flext`.
  Searches did **not** surface a direct Pd-list thread naming TTBlue by name —
  flext is the documented Pd↔Max bridge mechanism, and TTBlue is independently
  documented (74objects) as usable to build **Pd** externals + VST/AU plug-ins.
  **Phase-2 target:** search the Pd mailing-list archive (`lists.puredata.info`)
  directly for "TTBlue"/"Tim Place"/"Plugtastic."

**KVR Audio** `[snippet — verify]`
- No clean dedicated Electrotap/Tap.Tools announcement thread surfaced; KVR results
  were generic Max/MSP dev threads. Low priority unless Phase-2 forum search finds one.

**Academic / NIME / ICMC (reception & influence — strong)** `[snippet — verify]`
- The lineage from TTBlue → **Jamoma** is heavily documented academically, which is
  the strongest "influence" evidence:
  - **Place & Lossius, "Jamoma: A Modular Standard for Structuring Patches in Max,"
    ICMC 2006** — PDF: `https://jamoma.org/publications/attachments/jamoma-icmc2006.pdf`;
    Semantic Scholar / Academia / ResearchGate copies also exist.
  - **NIME 2008 Jamoma workshop** (Jensenius, Place, Lossius):
    `https://www.nime.org/web_archive/2008/documents/NIME2008-Jamoma-workshop.pdf`
  - **Place, Lossius, Jensenius, Peters, Baltazar, "Addressing classes by
    differentiating values and properties in OSC," NIME 2008** (Genova).
  - Jamoma publications hub: `https://www.jamoma.org/publications/`.
- **Electrotap Teabox** hardware got a peer review: **Project MUSE,
  "Electrotap Teabox Sensor Interface (review)"** —
  `https://muse.jhu.edu/article/188577/summary` (shows Electrotap's broader academic footprint).

**Jade (sibling product, co-catalogued with TapTools)** `[snippet — verify]`
- `http://archives.didascalie.net/tiki-index.php?page=about+Jade` — "about Jade."
- `https://www.dontcrack.com/freeware/downloads.php/id/2774/software/Jade/` — Jade download mirror.

**74objects blog (primary, first-person history)** `[snippet — verify]`
- `https://74objects.wordpress.com/2009/04/15/the-jamoma-platform/` — "The Jamoma
  Platform" (2009). Tim Place's blog documenting Jamoma/TTBlue/tap.tools lineage.
  403 here — **Phase-2 fetch.**

---

## 4. Reception / influence — synthesis

`[snippet — verify]` throughout:
- **Commercial standing:** Tap.Tools was a paid, professional **"construction kit"**
  (US$65–99), not a hobby patch dump — pitched as optimized building blocks for
  serious Max/MSP/Jitter work. v2.0 (2005) crossed **150+ externals**.
- **Press:** CDM covered it positively (2005). MusicMoz catalogued it in the
  canonical "Patch Libraries" directory of the era.
- **Pedagogy:** adopted as courseware (McGill MUMT 306 directory hosted Tap.Tools 1.0b2b).
- **Technical influence (the durable legacy):** TTBlue — **created by Place in 2003
  in Bergen, open-sourced 2005** — became the **Jamoma DSP layer** ("also known as
  TTBlue for historical reasons"), the C++ engine behind Jamoma's signal processing,
  and was reusable for **Pd externals and VST/AU plug-ins**. The academic Jamoma
  papers (ICMC 2006, NIME 2008) are the clearest evidence the work influenced the
  broader modular-Max research community.
- **Late-life:** Electrotap's website went offline (~2019 per snippets); support
  became thin; TapTools is now **open source on GitHub** (`https://github.com/tap/TapTools`,
  account `https://github.com/tap`). Community threads shifted from "buy/use" to
  "where do I find it / does it run on x64."
- **Biographical anchor:** Place — co-founded **Electrotap** (with **Jesse Allison**),
  Graduate Research Fellow at **UMKC**, ~15 yrs at **Cycling '74**, runs **74 Objects LLC**,
  later **Ableton** and now **Garmin** (Audio & DSP technical lead). Sources:
  Cycling '74 interview, LinkedIn (`https://www.linkedin.com/in/timothyaplace/`),
  74objects blog.

---

## 5. ARCHIVAL TARGET LIST FOR PHASE 2 (prioritized)

For a session with working WebFetch and/or Wayback (`web.archive.org/web/*/<url>`)
and/or a logged-in Cycling '74 forum account. Ordered by value × recoverability.

### Tier 1 — primary sources, highest value
1. **`https://cycling74.com/articles/an-interview-with-tim-place`** — first-person
   interview; likely the best single narrative source. Fetch + Wayback all snapshots.
2. **`http://max-archive.bek.no/`** (and `https://bek.no/en/restoring-old-max-list-archives/`)
   — the restored **McGill (Murtagh) Max list, 1997–1999**. Map its structure
   (by-month / by-thread), test for full-text search, and grep for **"Place"**,
   **"tap"**, **"Bergen"**. Confirm whether later years are also archived.
3. **`https://74objects.wordpress.com/`** — crawl the whole blog, esp.
   `2009/04/15/the-jamoma-platform/`; Wayback the front page across 2008–2012 for
   posts on TTBlue/tap.tools/Plugtastic. First-person dev history.
4. **`http://www.music.mcgill.ca/~ich/classes/mumt306/Tap.Tools_1.0b2b/ReadMe-Tap.Tools-beta2b`**
   — recover the **Tap.Tools 1.0b2b ReadMe** (early object list, contact URL,
   version date). Also browse the parent `~ich/classes/mumt306/` dir for more.

### Tier 2 — press & catalog (quotable reception)
5. **`https://cdm.link/2005/04/taptools-2-maxmspjitter-construction-kit-gets-bigger/`**
   — full CDM article + comments (April 2005). Quotable contemporaneous review.
6. **`https://cdm.link/maxmsp-resource-roundup-computer-music-special-updated/`**
   and `https://cdm.link/tag/maxmspjitter/` — additional CDM mentions.
7. **`https://musicmoz.org/Computers/Software/Max_and_MSP/Patch_Libraries/`** —
   capture the exact catalog blurb for **Tap.Tools** and **Jade** (and neighbors,
   for context on how it sat among peers). Wayback for older catalog states.

### Tier 3 — Cycling '74 forum threads (fetch full text / archive)
8. `https://cycling74.com/forums/max-externals-in-c` — "MAX externals in C++" (TTBlue/C++ dev).
9. `https://cycling74.com/forums/building-a-filter` — "Building a Filter" (TTBlue SVN example).
10. `https://cycling74.com/forums/tap-tools-3-2` — "Tap.Tools 3.2" (release/Intel-only).
11. `https://cycling74.com/forums/tap-tool-in-4-6` — "tap.tool in 4.6" (CFM install detail).
12. `https://cycling74.com/forums/bug-in-tap-tools-tap-pan` — "Bug in tap.tools (tap.pan~)".
13. `https://cycling74.com/forums/taptools-for-max-6-1-x64` — "TapTools for Max 6.1 x64?".
14. `https://cycling74.com/forums/about-tap-tools` — "about tap-tools".
15. `https://cycling74.com/forums/where-can-i-find-tap-tools` — "where can I find tap tools?".
16. `https://cycling74.com/forums/tap-tools-include-on-windows` — "Tap.Tools include on windows ?".
17. `https://cycling74.com/forums/old-mailing-list-archives-restored` — list-migration history.
18. `https://cycling74.com/forums/ot-mailing-list-question` and
    `https://cycling74.com/forums/ot-list-archive` — list/archive provenance.

### Tier 4 — ecosystem / influence / adjacent
19. **Jamoma papers:** `https://jamoma.org/publications/attachments/jamoma-icmc2006.pdf`,
    `https://www.nime.org/web_archive/2008/documents/NIME2008-Jamoma-workshop.pdf`,
    `https://www.jamoma.org/publications/`.
20. **Jamoma DSP wiki/changelog (TTBlue lineage):** `http://redmine.jamoma.org/projects/dsp/wiki`
    and `.../wiki/ChangeLog` — explicit "DSP layer aka TTBlue" docs + dated history.
21. **Pd mailing-list archive** `lists.puredata.info` (and `puredata.info`) — search
    "TTBlue", "Tim Place", "Plugtastic", "tap.tools".
22. **Electrotap site (dead, Wayback-only):** target `electrotap.com` and the
    download host **`download.74objects.com/taptools/`** in Wayback — recover the
    real product pages, version list, pricing, and download archive.
23. **Project MUSE Teabox review:** `https://muse.jhu.edu/article/188577/summary`.
24. **Jade pages:** `http://archives.didascalie.net/tiki-index.php?page=about+Jade`;
    `https://www.dontcrack.com/freeware/downloads.php/id/2774/software/Jade/`.
25. **TTBlue source provenance:** find the original **SVN repo** referenced in the
    "Building a Filter" thread (pre-GitHub TTBlue host) — likely on a Jamoma or
    SourceForge/Berlios-style server; check Wayback.
26. **Google Groups `electrotap`** group: `https://groups.google.com/g/electrotap`
    (threads like `.../c/F3xOabWTZ-Y` "TAP-TOOLS-PRO 3.1 @ MAX FOR LIVE",
    `.../c/Hozbv_HVFbw` "Tap-Tools for MAX For Live") — official support/announce
    channel; fetchable user-community primary source.

### Verification to-dos for Phase 2
- Confirm the BEK archive's exact year span and whether full-text search exists.
- Confirm Christopher Murtagh as list admin and the McGill→BEK chain from the BEK
  page itself (currently snippet-only).
- Pull exact CDM date/quotes and the MusicMoz catalog blurb verbatim.
- Recover Tap.Tools ReadMe(s) for an authoritative early object inventory.

---

### Source URLs cited (all surfaced via WebSearch; pages not fetched — 403)
- http://max-archive.bek.no/
- https://bek.no/en/restoring-old-max-list-archives/
- https://cycling74.com/forums/old-mailing-list-archives-restored
- https://cycling74.com/forums/ot-mailing-list-question
- https://cycling74.com/forums/ot-list-archive
- http://www.music.mcgill.ca/~ich/classes/mumt306/Tap.Tools_1.0b2b/ReadMe-Tap.Tools-beta2b
- https://cycling74.com/articles/an-interview-with-tim-place
- https://cycling74.com/forums/max-externals-in-c
- https://cycling74.com/forums/building-a-filter
- https://cycling74.com/forums/tap-tools-3-2
- https://cycling74.com/forums/tap-tool-in-4-6
- https://cycling74.com/forums/bug-in-tap-tools-tap-pan
- https://cycling74.com/forums/taptools-for-max-6-1-x64
- https://cycling74.com/forums/about-tap-tools
- https://cycling74.com/forums/where-can-i-find-tap-tools
- https://cycling74.com/forums/tap-tools-include-on-windows
- https://musicmoz.org/Computers/Software/Max_and_MSP/Patch_Libraries/
- https://musicmoz.org/Computers/Software/Max_and_MSP/
- https://cdm.link/2005/04/taptools-2-maxmspjitter-construction-kit-gets-bigger/
- https://cdm.link/maxmsp-resource-roundup-computer-music-special-updated/
- https://cdm.link/tag/maxmspjitter/
- https://74objects.wordpress.com/2009/04/15/the-jamoma-platform/
- https://jamoma.org/publications/attachments/jamoma-icmc2006.pdf
- https://www.nime.org/web_archive/2008/documents/NIME2008-Jamoma-workshop.pdf
- https://www.jamoma.org/publications/
- http://redmine.jamoma.org/projects/dsp/wiki
- https://github.com/tap/TapTools
- https://github.com/tap
- https://github.com/grrrr/flext
- https://grrrr.org/research/software/flext/
- https://puredata.info/community/pdwiki/flext
- https://muse.jhu.edu/article/188577/summary
- http://archives.didascalie.net/tiki-index.php?page=about+Jade
- https://www.dontcrack.com/freeware/downloads.php/id/2774/software/Jade/
- https://groups.google.com/g/electrotap/c/F3xOabWTZ-Y
- https://groups.google.com/g/electrotap/c/Hozbv_HVFbw
- https://www.linkedin.com/in/timothyaplace/
