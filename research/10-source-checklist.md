# Primary-Source Checklist — for a local / unrestricted-network pass

*Everything below was surfaced via WebSearch but **could not be opened from the research environment** (WebFetch + the Wayback Machine are blocked there). This is the consolidated, de-duplicated target list from digs `06` and `09`, ordered by value. Run it from a normal browser or a machine with working `curl`/Wayback access, then drop captured text/screenshots back into `research/sources/`.*

## How to use this
- **Wayback syntax:** `https://web.archive.org/web/*/<url>` shows all snapshots of a dead page; `https://web.archive.org/web/2008/<url>` jumps to the nearest 2008 capture. The dead hosts below (`electrotap.com`, `download.74objects.com`, `74objects.com`) are **Wayback-only**.
- **Capture, don't just read:** save full page text (and screenshots of product pages / UIs) into `research/sources/` with a filename like `cdm-taptools2-2005.md` and a one-line provenance header (URL + capture date).
- **Tick the box** when captured. Note in-line if a page is gone even from Wayback.
- **Three open conflicts to resolve** as you go (flagged ⚠️ below): the Hipno announce-vs-ship date; the exact Tap.Tools 1.0 date/price; the Darwin Grosse "Art+Music+Technology" episode for Place.

---

## A. First-person / highest narrative value
- [ ] **Cycling '74 — "An Interview with Tim Place"** — `https://cycling74.com/articles/an-interview-with-tim-place` (Wayback all snapshots). *The single best narrative source: Jade/Jamoma/TTBlue/Hipno in your own words. Contains the "leverage 3rd-party Max work into plug-ins" Hipno-origin quote.*
- [ ] **74objects WordPress blog** — `https://74objects.wordpress.com/` (crawl all; esp. `2009/04/15/the-jamoma-platform/`; Wayback front page 2008–2012). *First-person dev history: TTBlue, tap.tools, Plugtastic.*
- [ ] **"The Development of Jamoma"** — `https://cycling74.com/articles/the-development-of-jamoma` (2005-09-15). *The Jade→Jamoma story, the "shrimp on the West coast of Norway" anecdote, SourceForge dates.*
- [ ] ⚠️ **Art + Music + Technology podcast — the Place/Jamoma episode** — start `https://jamoma.org/categories/Darwin%20Grosse/`, then the AMT feed (Apple Podcasts / player.fm / listennotes), search "Place"/"Jamoma". *Find episode number + date; likely a long first-person interview.*
- [ ] **LinkedIn** — `https://www.linkedin.com/in/timothyaplace/` (or Wayback snapshot). *Exact Cycling '74 / Ableton / Garmin dates + titles to firm the career timeline.*

## B. Tap.Tools — the product, version by version
- [ ] **Tap.Tools 1.0b2b ReadMe** — `http://www.music.mcgill.ca/~ich/classes/mumt306/Tap.Tools_1.0b2b/ReadMe-Tap.Tools-beta2b` (browse parent `~ich/classes/mumt306/` too). ⚠️ *The earliest object inventory + version date + contact URL — anchors the Tap.Tools 1 era.*
- [ ] **Wayback `download.74objects.com/taptools/index.html`** — the real download index: full version list, system requirements, possibly installers.
- [ ] **Wayback `electrotap.com/taptools/` across 2004–2012** — best single source for per-version pricing, object counts, changelogs, screenshots.
- [ ] **CDM — "Tap.Tools 2… Construction Kit Gets Bigger"** — `https://cdm.link/2005/04/taptools-2-maxmspjitter-construction-kit-gets-bigger/`. *Full review + comments (April 2005): object count, $65/$99 tiers, the "indispensible time-savers" verdict.*
- [ ] **GitHub ReadMe + releases** — `https://github.com/tap/TapTools/blob/master/ReadMe.txt` and `/releases`. *Authoritative changelog (already partly in-repo); confirm the v3.1 "untethered from Jamoma" + v4 "open source / Max Package" notes.*
- [ ] **C74 forum — Tap.Tools 3.2** — `https://cycling74.com/forums/tap-tools-3-2` (download URL, Max-for-Live focus).
- [ ] **C74 forum — Max 6 + TapTools 4** — `https://cycling74.com/forums/max-6-and-taptools-4` (Jamoma 0.5.7, 32/64-bit, OS X 10.7).
- [ ] **Google Group `electrotap`** — `https://groups.google.com/g/electrotap` (e.g. `…/c/F3xOabWTZ-Y` "TAP-TOOLS-PRO 3.1 @ MAX FOR LIVE"). *Confirms the "Pro" edition; official support/announce channel.*
- [ ] **jiyounkang blog — "Tap.Tools Update"** — `https://jiyounkang.com/wordpress/index.php/archives/229` (the "100 objects / v2.0 direction" note; date it).
- [ ] **CNMAT recommended-externals** — `https://cnmat.berkeley.edu/content/recommended-third-party-max-msp-jitter-softwares` + `https://github.com/CNMAT/CNMAT-MMJ-Depot/wiki/Recommended-3rd-Party-Externals`. *Placement/wording among peers.*

## C. Hipno / Pluggo ecosystem
- [ ] ⚠️ **Resolve the date conflict** across these: `https://www.synthtopia.com/content/2005/10/19/cycling-74-intros-hipno-10/` (announce 2005-10-19) · `https://www.kvraudio.com/news/cycling_74_announces_hipno_2897` and `…cycling_74_releases_hipno_v1_0_over_40_plug_ins_4116` · `https://www.mixonline.com/technology/namm-news-cycling-74-releases-hipno-plug-suite-380959`. *Best current reading: announced Oct 2005, shipped ~NAMM Jan 2006 — confirm.*
- [ ] **MusicRadar Hipno review** — `https://www.musicradar.com/reviews/tech/cycling74-hipno-23402` (and `…-22035` — check if distinct). *Full review text, rating, pros/cons.*
- [ ] **AudioMasterclass Hipno blog** — `https://www.audiomasterclass.com/blog/hipno-plug-in-suite-from-cycling-74`. *The five-category breakdown + plug-in names (Deluge, Shypht, Morphulescence, Vsynth, Drunken Sailor, LoopDeeLa, SquiglyQ).*
- [ ] **Hipno 1.1 Universal Binary** — `https://sonicstate.com/news/2006/09/28/cycling-74-plug-ins-go-universal-binary/` (28 Sep 2006).
- [ ] **Hipno manual / "Getting Started" PDF** — look on C74 S3 (pattern: `cycling74.s3.amazonaws.com/download/…`, cf. `Pluggo35GettingStarted.pdf`). *The FULL 40+ plug-in list + Hipnoscope docs.*
- [ ] **The May 2009 discontinuation** — `https://cdm.link/cycling-74-ditches-plug-in-development-support-free-commercial-alternatives/` (Zicarelli quote, rationale, Max-for-Live pivot) + `https://www.kvraudio.com/news/cycling_74_discontinues_pluggo_moves_pluggo_technology_to_max_for_live_11565`.
- [ ] **C74 legacy downloads** — `https://cycling74.com/downloads/discontinued` + `https://support.cycling74.com/hc/en-us/articles/360050994953-Cyclops-Hipno-Mode-Pluggo` + `https://cycling74.s3.amazonaws.com/support/webpages/faq_pluggo.pdf`. *Installers + PluggoBus mechanics.*
- [ ] **Sound on Sound — Pluggo review** — `https://www.soundonsound.com/reviews/cycling-74-pluggo` (ecosystem context; also `…/surround-sound-stereo` for UpMix).

## D. Jade (the pre-Jamoma precursor)
- [ ] **didascalie "about Jade"** — `http://archives.didascalie.net/tiki-index.php?page=about+Jade`. *Richest Jade description (modules, matrix, address-per-variable, events). Capture verbatim + screenshots.*
- [ ] **Wayback `electrotap.com/jade/`** — the real Jade product page, module list, v1.1 changelog, screenshots.
- [ ] **dontcrack Jade** — `https://www.dontcrack.com/freeware/downloads.php/id/2774/software/Jade/` (blurb + maybe the actual download/version).

## E. TTBlue / Jamoma (the DSP spine)
- [ ] **Jamoma DSP wiki + ChangeLog** — `http://redmine.jamoma.org/projects/dsp/wiki` and `…/wiki/ChangeLog`. *Explicit "DSP layer aka TTBlue" docs + dated rename history.* (Was ECONNREFUSED in research env — try Wayback.)
- [ ] **TTBlue SourceForge** — find the project page + `ttblue-devel` mailing list ("TapTools Blue") + the original **SVN repo** referenced in the C74 "Building a Filter" thread. *Pre-GitHub TTBlue host, dates, license.*
- [ ] **Jamoma papers (full text)** — `https://jamoma.org/publications/attachments/jamoma-icmc2006.pdf` · `https://www.nime.org/web_archive/2008/documents/NIME2008-Jamoma-workshop.pdf` · `https://www.jamoma.org/publications/`. *Citable primary sources (were 403 in research env).*
- [ ] **C74 forum — externals in C++ / Building a Filter** — `https://cycling74.com/forums/max-externals-in-c` + `https://cycling74.com/forums/building-a-filter`. *TTBlue/C++ dev context + the SVN example.*

## F. Teabox / Electrotap (hardware + company)
- [ ] **ICMC 2004 Teabox paper** — `https://quod.lib.umich.edu/i/icmc/bbp2372.2004.082/` (Allison & Place): architecture, S/PDIF multiplexing, sensor specs.
- [ ] **CMJ 29(3) Teabox review (Barry Moon)** — `http://www.computermusicjournal.org/reviews/29-3/moon-teabox.html` / `https://direct.mit.edu/comj/article/29/3/104/93983/` / `https://muse.jhu.edu/article/188577`. *$425 price, specs, verdict.*
- [ ] **Trond Lossius Teabox note** — `https://trondlossius.no/articles/2004-07-19-teabox.html` (contemporaneous user account, 2004-07-19).
- [ ] **Teabox open-source** — `https://github.com/electrotap/Teabox` (+ `/releases`, `_decoders/max4live/`). *Design credits & history in the README.*
- [ ] **Wayback `electrotap.com` front page + `/teabox/`** — company "about," product line, the Place+Allison founding story, the KC-MO address.

## G. Community / forums / mailing lists
- [ ] **McGill→BEK Max list archive** — `http://max-archive.bek.no/` + `https://bek.no/en/restoring-old-max-list-archives/`. *Confirm the 1997–1999 span (so the book correctly says it predates Tap.Tools); note Murtagh (admin) + Jon Witte (archivist). Test full-text search; grep "Place"/"tap"/"Bergen" anyway.*
- [ ] **C74 forum — "where can I find tap tools" / "about tap-tools"** — `https://cycling74.com/forums/where-can-i-find-tap-tools` + `…/about-tap-tools` + `…/taptools-for-max-6-1-x64` + `…/tap-tools-include-on-windows` + `…/bug-in-tap-tools-tap-pan`. *Reception + the recurring "where do I get this?" demand signal.*
- [ ] **C74 forum — list-archive provenance** — `https://cycling74.com/forums/old-mailing-list-archives-restored` + `…/ot-mailing-list-question` + `…/ot-list-archive`.
- [ ] **Pd mailing-list archive** — `lists.puredata.info` / `puredata.info` — search "TTBlue", "Tim Place", "Plugtastic", "tap.tools" (TTBlue was Pd-usable via flext).

## H. Current footprint ("is it still used?")
- [ ] **Wayback `electrotap.com/taptools/` + `musicmoz`** — `https://musicmoz.org/Computers/Software/Max_and_MSP/Patch_Libraries/` (verbatim catalog blurb for Tap.Tools + Jade).
- [ ] **Third-party externals collections** — `https://github.com/v7b1/max-thirdParty_externals` — check whether `tap.*` objects are preserved/bundled (footprint evidence).
- [ ] **CDM Max resource roundups** — `https://cdm.link/maxmspjitter-resource-roundup-computer-music-special-updated/` + `https://cdm.link/tag/maxmspjitter/`.

---

## Verification to-dos (resolve while capturing)
- ⚠️ Hipno announce-vs-ship date (Oct 2005 announce / ~Jan 2006 ship?).
- ⚠️ Exact Tap.Tools 1.0 release date + original price.
- ⚠️ The Darwin Grosse "Art + Music + Technology" episode number/date for Place.
- Confirm Christopher Murtagh as McGill list admin + the McGill→BEK chain from the BEK page itself.
- Pull exact CDM date/quotes + the MusicMoz blurb verbatim.
- Find the pre-GitHub TTBlue SVN/SourceForge host and its first-commit date.

> When captures land in `research/sources/`, I can re-synthesize digs `03`–`09` and the `00-overview` with the upgraded, page-confirmed facts (replacing the `[snippet — verify]` tags with citations).
