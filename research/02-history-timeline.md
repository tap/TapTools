# TapTools — History & Timeline Dossier

Research notes for book planning. Dense, not polished. Two primary sources mined:
- `/home/user/TapTools/ReadMe.txt` — the official CHANGE LOG (versions 3.0 → 4 beta 2), SPECIAL THANKS, copyright `© 1999–2013, 74 Objects LLC`.
- The git repository (52 commits, **2014-10-31 → 2020-02-11**) — only the project's tail.

Author: Timothy Place. Vendor: 74 Objects LLC. License (by v4): New BSD (3-clause). Built atop Jamoma + ObjectiveMax. Platform by end: Mac-only, Max 6.1+.

---

## 1. The two-era problem (state this up front)

The project's own copyright spans **1999–2013**, but git only begins **2014-10-31**. There is a ~15-year pre-git history (1999–2014) that exists ONLY in:
- the prose changelog in `ReadMe.txt` (covers 3.0 → 4 beta 2),
- author memory,
- web archives / old Cycling '74 forum posts / installer archives.

Even the changelog is incomplete for the deep past: it begins at **3.0** (Max 5 era). Versions 1.x and 2.x (the Max 3 / Max 4 era, roughly 1999–2007) are **undocumented in any source in this repo**. The changelog repeatedly references that era retroactively (e.g. "written for Max 3.6.x", "introduced in Max 4 ... back in 2001", "fixed onepole~ being inaccurate back in 2011") — those asides are the only surviving fossils of the 1.x/2.x period.

Date histogram of git commits (`git log --format='%ai' | cut -c1-7 | sort | uniq -c`):
```
17  2014-10     <- the big "spring cleaning" purge (one afternoon, 31 Oct)
19  2014-11     <- continuation of purge + Max 7 help updates
10  2015-05     <- submodules→subtrees, Jamoma/SDK vendoring, Travis CI
 4  2015-07     <- jamoma2 submodule, reviving tap.fourpole on jamoma2
 1  2015-12     <- reviving tap.rotate
 1  2020-02     <- single ReadMe edit, 5 years later (project effectively dormant)
```
So the *active* git life is really **Oct 2014 – Dec 2015** (~14 months), plus one stray 2020 touch. The git era is a maintenance/pruning/modernization epilogue, not a development era.

---

## 2. Chronological timeline (table)

Dates before 2014 are from the changelog (release order; no precise dates survive except the v4b2 masthead "26 April 2013"). Dates from 2014 on are git commit dates.

| Date / Version | Event | Significance |
|---|---|---|
| 1999 | Project begins (per copyright "© 1999") | Origin. Max 3 / early Max 4 era. No repo artifacts. |
| ~2001 | (retro-referenced) Max 4 ships `atodb~`/`dbtoa~`, `onepole~` | TapTools objects like `tap.decibels~` made "mostly obsolete back in 2001" — shows TapTools filling gaps before Max had natives. |
| **3.0** (Max 5 era) | Major rewrite for Max 5 | Full ref pages, filebrowser/metadata, advanced inspector integration; 64-bit *internal* processing; multichannel MSP objects; Java/JS objects rewritten as native externs (kills Java dependency, important for Windows); **all externals merged into a SINGLE monolithic extension** loaded at launch; TapTools Builder added (Extras menu); new objects `tap.filter~`, `tap.filecontainer`, `tap.folder`, `tap.svn`. |
| 3.0.1 | Maintenance | Dropped external dep on MaxObject framework (linked internally); buffer-object perf; `tap.inquisitor` added; object descriptions in browser (needs Max 5.0.6+); Nils Peters credited with help-patcher fixes. |
| 3.0.2 | Compatibility | Jamoma 0.5 compatibility — shows tight Jamoma coupling. |
| **3.1** | **Re-modularization + platform pruning** | REVERSES 3.0's monolith: "rather than a monolithic 'tap.tools' extension, each object is defined as a separate Max external binary." Reasons cited: standalone building, binary/library conflict resilience, Max-For-Live support. **Untethered from Jamoma** ("resolves all dependency issues"). **Dropped PPC**. **Dropped Windows**. `tap.svn` dropped. Deprecated: `tap.myip`, `tap.typecheck`, `tap.fourpole~/twopole~/onepole~` (→ use `tap.filter~`), `tap.average~`. |
| 3.2 | M4L fixes | Uninstall script added; more Max-for-Live fixes; `tap.filter~` gains many filter types. (Note: changelog jumps 3.1 → 3.2, skipping 3.3–3.5.) |
| **3.6** | **64-bit audio for Max 6** | Audio objects updated to Max 6's "new native 64-bit audio samples". Dropped ancient `tap.average~`, `tap.degrade~` (→ use Max natives `average~`, `degrade~`). Deprecated `tap.xml.sax` (→ Max 6's mxj XmlParse). Pattern: Max gaining natives → TapTools shedding redundant objects. |
| 3.6.1 | Bugfix | Fixed loading when Jamoma not installed. |
| 3.6.2 | Gatekeeper | Installer **code-signed for Apple Gatekeeper** — macOS security-era marker (~2012). `tap.noise~` gaussian mode. |
| 3.6.3 / 3.6.4 | Bugfix | M4L fixes; resolve conflict with Jamoma 0.5.6 co-install. |
| **4 beta 1** | **OPEN-SOURCED** | "All code is now open source and available via GitHub." **Registration number no longer required** → honor-system + donations. **Installer removed.** **Jamoma deps no longer bundled** (user must install Jamoma). Commercial → honor-system transition. |
| **4 beta 2** — 26 Apr 2013 | Max 6.1 + Package | Max 6.1 compat; distributed as a **Max Package** (not installer); Jamoma deps delivered via the package mechanism; beta-1 bugfixes. This is the last documented release in `ReadMe.txt`. |
| **2014-10-31** | Git history BEGINS | First commit `a09b3c5`. Start of a one-afternoon mass purge of obsolete objects. |
| 2014-10-31 | The "diet" / "spring cleaning" | 17 commits in one day removing objects Max made redundant (see §3). `2955894` "going on a diet... eliminating ancient cruft ... 'backwards-compatibility-lib'"; `efa14ab` "help/ref spring cleaning". |
| 2014-11-04 → 11-06 | Max 7 help updates + more removals | 19 commits: help patchers "update for Max 7" (so the team was tracking Max 7's 2014 release); more retirements; "project updates L-Z". |
| 2015-05-15 | Build-system rework | `4a20802` "removing submodules, will replace with subtrees"; vendored `max-sdk` and Jamoma `Core/` as git subtrees; first **Travis CI** integration (`9d95169` "first stab at travis integration"). |
| 2015-07-29 | jamoma2 arrives | `83555e1` adds **jamoma2** as a submodule; `c180cba` **"tap.fourpole: reviving this ancient object, but now building it on the jamoma2 library."** Revival, not just pruning. |
| 2015-12-25 | Christmas revival | `a4c2a05` **"tap.rotate: bringing it back from the dead..."** (committed on Christmas Day). |
| 2020-02-11 | Last commit | `5cf68f2` "Update ReadMe.txt" (one-line deletion). Project effectively dormant after 2015; this is a lone late touch. |

---

## 3. Technical evolution (themes)

**Monolith ↔ per-object swing.** The single most interesting architectural fact: TapTools went *monolithic* in **3.0** ("All external objects re-structured into a single extension loaded by Max at launch") and then *re-modularized* in **3.1** ("each object is defined as a separate Max external binary"). The reversal was driven by standalone-building friction, library/binary-conflict fragility, and Max-For-Live needs. Good book material: a concrete case where a "consolidate everything" decision was undone within one minor version by real-world deployment pain (esp. M4L).

**The "Max ate my object" pattern.** TapTools' core lifecycle theme: objects exist to fill gaps in Max/MSP, then get retired as Cycling '74 ships native equivalents. Documented explicitly across the changelog and git:
- `tap.average~`/`tap.degrade~` → native `average~`/`degrade~` (3.6).
- `tap.decibels~` → `atodb~`/`dbtoa~` "introduced in Max 4 ... back in 2001" (`f29e81d`).
- `tap.onepole~` → fixed in Max's `onepole~` "back in 2011" (`20a25ba`).
- `tap.buffer.norm~` → `normalize` message added to `buffer~` for Max 6 (`81351a3`).
- `tap.list.join`/`tap.list.slice` → `zl` now handles large lists (`a09b3c5`).
- `tap.xml.sax` → "better tools in js etc." (`b1d37f4`).
- `tap.path`, `tap.buildassist` → Max natively resolves `~`/`Desktop:/`; Projects feature replaces buildassist (`dba5251`, `6663ab5`).
- `tap.diff` → "do it with gen~" (`bf750e3`).
- `tap.loadbang` → "a bad idea that is finally going away" (`565cc8c`).
- `tap.lfo~` low-res mode → "we aren't trying to squeek out this kind of silly extra performance on G3 processors anymore" (`1e524a6`).

**Jamoma coupling → decoupling → jamoma2 re-coupling.** Three phases:
1. Tight coupling (3.0.2 "compatibility with Jamoma 0.5"; bundled Jamoma DSP dylib).
2. Decoupling (3.1 "completely untethered from Jamoma, resolving all dependency issues"; 4b1 stops bundling Jamoma so user installs it; 4b2 delivers via package mechanism).
3. Re-coupling on the next-gen library (2015: jamoma2 submodule; `tap.fourpole`/`tap.twopole`/`tap.fourpole~` superseded by `j.filter~`; `tap.colorspace` → `j.unit`). Several externals were explicitly *handed off* to Jamoma rather than maintained in TapTools.

**Audio engine evolution.** "Many audio objects ... 64-bit processing internally" (3.0) → full "native 64-bit audio samples in Max 6" (3.6). Multichannel processing introduced piecemeal (`tap.allpass~` in 3.1; `tap.dcblock~`, `tap.filter~`, `tap.overdrive~`, `tap.crossfade` earlier).

**Platform attrition.** PPC dropped (3.1), Windows dropped (3.1, despite earlier heavy Windows effort — Dave Watson & Rob Sussman thanked for the Windows port), Java dependency eliminated (3.0, partly *because* Java wasn't standard on Windows). End state: Mac-only, Intel-only, Max 6.1+.

**Build/CI modernization (git era).** `build.rb` Ruby build driver; Travis CI on OSX with `xcode61` image, `language: objective-c` faked "so we can run CI on OSX" (`c5b9ec0`); submodules→subtrees for max-sdk and Jamoma Core, later jamoma2 as submodule (`.gitmodules`: `source/jamoma2 → github.com/jamoma/jamoma2.git`).

**Distribution mechanics.** Installer (commercial era) → Max Package (4b2) → raw GitHub checkout. `tap.buildassist`/TapTools Builder (custom standalone tooling) obsoleted by Max 6's Projects feature.

**Surviving externals (git HEAD, `source/`):** ~50 objects remained after the purge, incl. `tap.adsr~`, `tap.comb~`, `tap.filter`-family (`tap.svf~`, `tap.fourpole~`, `tap.biquadcalc`), `tap.buffer.*~`, `tap.fft.*~`, `tap.jit.*` (ali, colortrack, kernel, proximity, sum), `tap.noise~`, `tap.verb~`, `tap.limi~`, `tap.elixir~`, etc. The `tap.jit.*` and FFT clusters show TapTools reached into Jitter (video) and spectral processing, not just MSP. (`ls source/` for full list.)

---

## 4. Business-model evolution

A clean commercial → open-source arc, all in the 4.x transition:
- **Commercial era (pre-4b1):** registration numbers / authorization required; `tap.applescript` was "copy protected" (3.0 notes it being *un*-copy-protected as a feature); paid installer; demo expiry.
- **4 beta 1 (open-source pivot):** BSD-licensed, on GitHub; "no longer required that you enter a registration number"; "TapTools now operates on the **honor system** — if you derive benefit ... please support it with a donation"; installer dropped.
- **4 beta 2:** Max Package distribution finalizes the shift away from installers/registration.
- **Git era:** purely volunteer/honor-system; sporadic, personal, hobby-cadence maintenance (one big cleanup, then occasional "back from the dead" revivals). The wry, sentimental commit voice ("you won't be missed", "time to stop being sentimental") reflects a solo author tending a long-running personal project past its commercial life.

Vendor identity: **74 Objects LLC** (tim@74objects.com). Note author's email shifts between `tim@74objects.com` and `tim@cycling74.com` across commits — Timothy Place was at/associated with Cycling '74 (makers of Max) during the git era, which contextualizes the deep insider knowledge of upcoming Max native features driving the prunes.

---

## 5. The social network (SPECIAL THANKS map)

The thanks list (`ReadMe.txt` lines 33–47) reads like a who's-who of the Max/MSP + computer-music + Jamoma communities. Reading each contributor and what their presence signals:

| Person | Role / identity (from context + known) | Involvement in TapTools | What it signals |
|---|---|---|---|
| **David Zicarelli** | Founder/owner of Cycling '74, the company behind Max/MSP | Helped with original `tap.comb~`, `tap.buildassist`, "many others" | Direct line to the *creator of the platform* — TapTools was inner-circle, not fringe. |
| **Joshua Kit Clayton** | Long-time Cycling '74 engineer (Jitter/MSP) | "help with numerous objects" | Deep Cycling '74 engineering support. |
| **Jeremy Bernstein** | Max developer (later Cycling '74), known for cross-platform/Max externals | `tap.applescript`, `tap.myip`, `tap.windowdrag` | The system-integration / OS-glue objects. |
| **Ali Momeni** | Composer/artist, academic (computer music, media art) | `tap.jit.ali` *named for him*; provided interpolation algorithms, feedback, testing | An object literally named after a collaborator — Jitter/video interpolation; ties to academic media-arts. |
| **Trond Lossius** | Norwegian sound artist; a core **Jamoma** developer | "various contributions and feedback" | The Jamoma connection in human form — explains the deep Jamoma coupling. |
| **Nils Peters** | Spatial-audio researcher (later Qualcomm/audio standards) | "extensive feedback"; submitted help-patcher fixes (`tap.jit.ali`, `tap.jit.delay`) — credited in 3.0.1 changelog too | Active code-level contributor, spatial-audio expertise. |
| **Richard Dudas** | Composer; ex-IRCAM/Cycling '74 (co-authored MSP objects) | Filter/color/buffer feedback; **provided code + permission for `list.join`/`list.slice`** externals | A genuine code donor; IRCAM lineage. |
| **Darwin Grosse** | Max community figure, educator/podcaster (Art+Music+Technology), Cycling '74 | Suggested the `tap.plug.*` series | Community/education side; idea source. |
| **Stephan Moore** | Sound artist / audio engineer | `tap.anticlick~` | |
| **Russell Pinkston** | Composer, UT Austin electronic music professor | Idea for `tap.random~` and `tap.counter~` | Academic-pedagogy origin for some objects. |
| **jhno (John Eichenseer)** | Electronic musician | Permission to augment his `limi~` limiter algorithm; help with `tap.windowdrag` | `tap.limi~` derives from his work. |
| **Dave Watson & Rob Sussman** | (Windows devs) | "tremendous amounts of help ... Windows version" | The now-abandoned Windows port had serious effort behind it. |
| **Andrew Pask** | Cycling '74 engineer | testing/feedback | More Cycling '74 inner circle. |

**What the network says:** TapTools sat at the intersection of (a) **Cycling '74 itself** (Zicarelli, Clayton, Bernstein, Pask, Grosse, + author's own cycling74.com email) and (b) the **Jamoma open-source DSP project** (Lossius, Peters), with (c) **academic computer-music** roots (Momeni, Pinkston, Dudas, Moore). This is why TapTools could both anticipate Max's native features (insider knowledge) and lean on Jamoma's DSP library (collaborator overlap). It was a bridge artifact between the commercial platform and the academic/open-source Max ecosystem.

---

## 6. Notable commit messages (the author's voice)

Worth quoting in the book — wry, sentimental, personal. All from `timothy place`:
- `5614815` (2014-10-31) **"tap.applescript: goodbye forever. you won't be missed."**
- `2955894` "going on a diet... eliminating ancient cruft that from years of languish in the 'backwards-compatibility-lib' folder"
- `565cc8c` "tap.loadbang: a bad idea that is finally going away."
- `f05b146c`/`05b146c`... `f8045b1` region; esp. `05b146c` "tap.quantize: one of the externs i ever wrote is nevertheless pretty useless, time to stop being sentimental..."
- `1e524a6` "tap.lfo~: the low-res LFO is dead, we aren't trying to squeek out this kind of silly extra performance on G3 processors anymore..."
- `dba5251` "tap.path: ... None of this code is cross-platform. Good bye."
- `c180cba` (2015-07-29) "tap.fourpole: reviving this ancient object, but now building it on the jamoma2 library"
- `a4c2a05` (2015-12-25) **"tap.rotate: bringing it back from the dead..."**
- `f68d4fb` "init: removing this folder/file as it just creates aliases to bogus objects"
- `7bcc328` "tap.pulseroute: removing this one as it is another of **jesse's** objects rather than one of the original taptools." — note an unnamed-in-thanks contributor "jesse" (possibly Jesse Kriss / Jesse?) authored some objects; a thread worth chasing.

The voice arc: **purge** (Oct–Nov 2014, brusque/sentimental goodbyes) → **modernize** (May 2015, build/CI) → **revive** (Jul/Dec 2015, "back from the dead"). The 2020 lone commit reads as a tombstone.

---

## 7. The missing pre-2014 history (research gaps to fill for the book)

NOT in this repo; would need external sourcing:
1. **1999–2007 (v1.x / v2.x, Max 3 & 4):** entirely undocumented. No changelog entries, no git. Only oblique retro-references ("written for Max 3.6.x", "back in 2001"). Need: Wayback Machine captures of the old TapTools / 74objects / electrotap site, old Cycling '74 forum threads, original installer archives, author's own files/memory.
2. **Precise release dates** for 3.0 → 4b1: changelog gives version order but only ONE date (4b2 = 26 Apr 2013). Need download-page archives or release announcements to date 3.0, 3.1, 3.6, 4b1.
3. **3.3–3.5:** the changelog skips from 3.2 to 3.6 — were these internal/unreleased? Verify.
4. **The Windows port lifecycle:** thanked heavily (Watson/Sussman) then dropped in 3.1 — when added, why dropped (PPC/Intel transition? maintenance cost?). Not in git.
5. **Commercial details:** pricing, registration/authorization system, demo mechanism, distributor — all pre-git. Only inferable from "registration number" / "copy protected" / "installer" mentions.
6. **The "ObjectiveMax" and "electrotap" branding:** `objectivemax/` dir + "ObjectiveMax" in thanks suggest a related/earlier framework; relationship to TapTools and to the "electrotap" name needs external confirmation.
7. **"jesse"** — the unnamed contributor in commit `7bcc328` (objects `tap.pulseroute`, `tap.pulsesub~`?) — identify and place in the network.

---

## Source citations
- Changelog, thanks, copyright, system reqs: `/home/user/TapTools/ReadMe.txt` (lines 31–47 thanks; 56–189 changelog; 193–194 copyright).
- Git history: `git log --reverse --format='%h | %ai | %an | %s'` — 52 commits, `a09b3c5` (2014-10-31) … `5cf68f2` (2020-02-11).
- Build/CI: `/home/user/TapTools/.travis.yml`, `/home/user/TapTools/build.rb`, `/home/user/TapTools/.gitmodules`.
- Surviving externals: `/home/user/TapTools/source/` (~50 `tap.*` dirs).
- Package layout: `/home/user/TapTools/TapTools/` (docs, externals, extensions, help, jsui, patchers, templates).
