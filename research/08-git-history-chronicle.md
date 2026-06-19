# TapTools Git-History Chronicle

> A documented chronicle of the **complete git history** of the TapTools
> repository, mined as a primary historical source for research about TapTools
> (Max/MSP externals by Timothy Place). Every claim below is backed by a commit
> hash and date drawn from `git log --all`. Commit messages are quoted verbatim;
> these wry one-liners are the surviving narrative voice of the author.
>
> Method: `git fetch origin`; `git log --all --reverse`; per-branch logs;
> `git show --stat <sha>` for object-level file changes. Cross-referenced
> (read-only) against the author's own `REVIVAL.md` on `refs/heads/main`.
>
> Scope note: git is a **rich** source here precisely because the web is mostly
> blocked. But it is also a **bounded** source — see the closing section "What
> git cannot tell us (the pre-2014 gap)."

---

## 1. Branch-by-branch map

There are six "real" branches plus four `claude/*` working branches. The
key structural fact: **`main` (2026) and `master` (2014–2020) share history;
`legacy`, `windows`, and `taptools-min` are divergent snapshots of three
different abandoned modernization attempts; `gh-pages` is an orphan stub.**

| Branch | Span | Tip commit | Role |
|--------|------|-----------|------|
| `master` | 2014-10-31 → 2020-02-11 | `5cf68f2` "Update ReadMe.txt" | The open-source era; frozen since 2020. |
| `main` | 2026-06-13 → 2026-06-19 | `1063395` (merge PR #31) | The 2026 revival; built on `master`'s base, then "prunes the corpse." |
| `legacy` | 2014-10-31 → 2015-12-25 | `a03dc9a` "legacy ReadMe" | Preserves the 2015 jamoma2-port state as a snapshot. |
| `windows` | 2014-10-31 → 2016-03-18 | `a99aea4` "appveyor config" | The 2016 CMake + AppVeyor Windows-CI experiment. |
| `taptools-min` | 2016-10-04 → 2019-06-06 | `be8e852` "updated top-level cmake" | Cycling '74's own Min port; **deleted upstream**, preserved here. |
| `gh-pages` | 2012-11-06 (single commit) | `d560c00` | A GitHub Pages template stub — the *oldest* commit, but content-empty. |

`claude/*` working branches (2026, machine-assisted; not deep-dived here):
- `claude/taptools-revival-xtc276` — the main revival work branch (tip `e95a7db`, 2026-06-16).
- `claude/revival-md-tasks-vwwpa4` — CI fixes (tip `4732d7c`, 2026-06-17).
- `claude/taptools-min-reconciliation` — the taptools-min merge (tip `d853476`, 2026-06-17).
- `claude/revival-md-planning-y0vjdj` — polish/help-patcher/research-dossier work (tip `68f680c`, 2026-06-18).
- `claude/taptools-book-planning-6t9hzv` — the current branch (this document).

---

## 2. The unified chronological timeline

Each row is anchored to a commit hash and its author date. "Branch" is the
earliest branch the commit appears on. Significance is summarized.

| Date | Branch | Commit | What happened | Significance |
|------|--------|--------|---------------|--------------|
| 2012-11-06 | gh-pages | `d560c00` | "Create gh-pages branch via GitHub" — index.html + GitHub Pages template assets (587 lines, 9 files). | Oldest commit in the repo. **Content-empty stub** (default theme; no project content). |
| 2014-10-31 16:19 | master | `a09b3c5` | Root content commit. Removes `tap.list.join`/`tap.list.slice`; **same commit introduces the entire existing tree** (ReadMe, ~200 docs, Core + MaxSDK submodules). | The **earliest real TapTools content** in git. History before this is absent. |
| 2014-10-31 16:26–17:45 | master | `20a25ba`…`a8a9608` | **The "great pruning"** — a ~90-minute burst removing obsolete objects with explanatory messages (see §4). | The densest narrative moment in git: the author euthanizing 1999-era objects one by one. |
| 2014-11-04 → 11-06 | master | `5b5a0ac`…`07abde5` | Help/ref updates for Max 7; more retirements (`tap.loadbang`, `tap.lfo~`, `tap.quantize~`, `tap.colorspace`, `tap.path`, `tap.buildassist`, `tap.diff~`, `tap.decibels~`); "project updates L-Z." | The pruning's second wave; Max-7-era housekeeping. Last sustained 2014 activity. |
| 2015-05-15 07:19–12:02 | master/legacy/windows | `4a20802`…`c5b9ec0` | "removing submodules, will replace with subtrees"; squash-import of `max-sdk` and `Core` (Jamoma) as subtrees; "first stab at travis integration"; "faking the language so we can run CI on OSX." | **Modernization attempt #1 (part A):** vendor deps as subtrees + Travis CI. |
| 2015-07-29 | legacy/windows | `a1b2de8`…`d7c7d51` | "adding jamoma2 as submodule"; **`tap.fourpole`: "reviving this ancient object, but now building it on the jamoma2 library"**; resonance fix. | **Modernization attempt #1 (part B):** the jamoma2 rewrite. Got exactly one object (`tap.fourpole~`) before stalling. |
| 2015-12-25 | legacy/windows | `a03dc9a`, `a4c2a05` | "legacy ReadMe" (legacy branch reduced to a 2-line stub); **`tap.rotate`: "bringing it back from the dead..."** (windows branch). | Christmas-day return: a *second* jamoma2 revival (`tap.rotate`). The "comes back every few years" pattern begins. |
| 2016-03-18 | windows | `6f8bcdb`…`a99aea4` | jamoma2 updates ×3; **"initial cmake success on the mac using tap.fourpole~ as the test rabbit"** (`8c671d9`); five "appveyor config" commits. | **Modernization attempt #2:** CMake + AppVeyor Windows CI. All in a single day, then stops. |
| 2016-10-04 → 10-06 | taptools-min | `8cdb6b6`…`0d76d30` | Min API submodule + min-devkit scaffolding; initial Min ports of `tap.fft.list~`, `tap.fft.normalize~`, `tap.sift~`; "fifo moved into min"; sift~ main-thread option; inlet<>/safe-outlet experiments. | **Modernization attempt #3 (wave 1):** the Cycling '74 Min port begins. Authored as `tim@cycling74.com`. |
| 2019-01-22 → 02-24 | taptools-min | `1aa8803`…`39408a0` | min api update; **`tap.buffer.record~`: "initial min version -- dramatically better performance than the original"**; `tap.buffer.snap~` initial min version; api updates. | **Modernization attempt #3 (wave 2):** 2.5-year gap, then a burst. The "comes back every few years" pattern, again. |
| 2019-06-06 | taptools-min | `546993b`, `be8e852` | "adding min-lib"; "updated top-level cmake." | Last commit on `taptools-min`. The C74 port goes quiet. |
| 2020-02-11 | master | `5cf68f2` | "Update ReadMe.txt." | The lone touch of the open-source era's final years. Then **six years of silence.** |
| 2026-06-13 | main | `90d31c8`…`b62bba8` | The revival: inventory/plan, modern Min + C++20 CMake foundation, then ~40 object ports across Tiers 1–3 + Jitter; **`b62bba8` "Prune the corpse: remove all dead legacy trees"** (6808 files, ~1.37M lines deleted). | **Modernization attempt #4:** the full revival. Machine-assisted (authored `Claude <noreply@anthropic.com>`), directed by the author. |
| 2026-06-17 | main | `4732d7c`, `d853476` + merges | CI fixes (macOS 11 / MSVC); **"Reconcile taptools-min"** — the long-deleted C74 port is merged in and `tap.sustain~` recovered. | The four attempts converge: attempt #3's archive is salvaged into attempt #4. |
| 2026-06-17 → 06-19 | main | `4ef0c86`…`1063395` | Spectral set reinvented (`tap.vocoder~`/`tap.nr~`/`tap.spectra~`); test harness; help patchers; research dossiers. | Revival reaches "all ~48 objects ported, runtime validation pending." |

---

## 3. Per-branch detail

### 3.1 `gh-pages` (2012) — the oldest, emptiest commit
Single commit `d560c00` (2012-11-06, "Timothy Place <tim@74objects.com>"):
"Create gh-pages branch via GitHub." Contents are the stock GitHub Pages
template: `index.html` (84 lines), `params.json`, `stylesheets/`,
`javascripts/main.js`, and template images (`blacktocat.png`, etc.). **No
TapTools project content whatsoever.** Its only historical value is the date: it
proves a TapTools GitHub repo existed by November 2012, two years before any real
code was committed.

### 3.2 `master` (2014–2020) — the open-source era
The root commit `a09b3c5` (2014-10-31) simultaneously *introduces* the full
existing tree and *begins removing* from it. The introduced tree shows the shape
of TapTools as of late 2014: a `TapTools/docs/` folder of ~150 `.maxref.xml`
reference pages, a `source/tap.*/` tree of C++/Objective-C externals, `Core`
(Jamoma) and `MaxSDK` git submodules, and a 195-line `ReadMe.txt`. The branch
runs through the 2014 pruning (§4), the 2015 subtree/Travis/jamoma2 work, and
then goes dormant — its final commit is the trivial `5cf68f2` (2020-02-11)
"Update ReadMe.txt." This is the branch the 2026 revival builds on.

### 3.3 `legacy` (2015) — the preserved snapshot
The `legacy` branch carries the same history as `master`/`windows` up through
2015-07-29's `tap.fourpole~` jamoma2 work, then diverges with a single
distinctive commit: `a03dc9a` (2015-12-25) **"legacy ReadMe."** That commit
strips `ReadMe.txt` down to a 2-line stub (`-190 / +2`):

```
TAPTOOLS
By Timothy Place
http://74objects.com/

LEGACY VERSIONS

This is a dummy branch for legacy version tags.
```

So `legacy` is explicitly a **dummy branch for legacy version tags** — a frozen
marker preserving the pre-revival (jamoma2-era) state. `REVIVAL.md` confirms the
2026 work treats `legacy` as the canonical archive of the old package, restoring
help/ref assets and shared bpatchers (e.g. `tap.badge.maxpat`) from it after the
prune.

### 3.4 `windows` (2014–2016) — the CMake/AppVeyor experiment
`windows` shares history with `legacy` (the 2014 prune, the 2015 subtree/Travis
work, jamoma2). It then adds the 2015-12-25 `tap.rotate` revival (`a4c2a05`,
"bringing it back from the dead...") and the 2016-03-18 burst that *is* the
Windows experiment, all in a single day:

- `6f8bcdb`, `26992df`, `49ef2fa` — "updating jamoma2 to the latest" (×3).
- `8c671d9` — **"initial cmake success on the mac using tap.fourpole~ as the test rabbit."** This is the "test rabbit" commit: it adds a root `CMakeLists.txt`, a per-object `source/tap.fourpole~/CMakeLists.txt`, builds a `.mxo`, and renames the source `tap.fourpole~.cpp → tap.fourpole_tilde.cpp` (the `_tilde` naming convention that the 2026 revival would re-derive independently).
- `0214903` — "initial appveyor files" (`appveyor.yml`, 38 lines + `source/build.ps1`).
- `47c3d11`, `98fbcbb`, `e80d58e`, `a99aea4` — four successive "appveyor config" iterations, ending the branch.

What it reveals: Windows support was attempted via **CMake (to escape the
Mac-only Ruby/Xcode build) + AppVeyor (Windows CI)**, with `tap.fourpole~` as
the single guinea pig ("test rabbit"). The branch ends mid-iteration on the same
day it started (2016-03-18) — there is no evidence of a green Windows build or of
the experiment being merged. It stalls exactly where the jamoma2 dependency would
have to be made to build on Windows.

### 3.5 `taptools-min` (2016–2019) — Cycling '74's own Min port
This branch is an **independent history with no shared base** with the others
(per `REVIVAL.md` §8). It is **Cycling '74's official Min port** of TapTools
(`github.com/Cycling74/taptools.git`), **since deleted upstream** and preserved
here for archival. All commits are authored **"Timothy Place
<tim@cycling74.com>"** — i.e. Place during his Cycling '74 tenure. Full
timeline:

| Date | Commit | Message |
|------|--------|---------|
| 2016-10-04 | `8cdb6b6` | "Min API added as git submodule" |
| 2016-10-04 | `ccc41b3` | "initial scaffolding provided by the min-devkit script" (adds Travis+AppVeyor CMake skeleton, `HowTo-NewObject.md`, hello-world project) |
| 2016-10-04 | `f988c76` | "tap.fft.list~: intial port" *(sic)* |
| 2016-10-05 | `a65a831` | "tap.fft.normalize~: initial port from the old tap.tools" |
| 2016-10-05 | `ecbb73e` | "tap.sift~: initial port from the old tap.tools" |
| 2016-10-05 | `2564380` | "fifo moved into min" |
| 2016-10-05 | `d5ab9d1` | **"tap.sift~: new option to send output to main thread instead of scheduler thread"** |
| 2016-10-06 | `b27d8ff` | "'safe' outlet experiments" |
| 2016-10-06 | `603a687` | "inlets now use inlet<> syntax." |
| 2016-10-06 | `0d76d30` | "update for change to message for template args" |
| 2019-01-22 | `1aa8803` | "min api update" |
| 2019-01-25 | `0826fdd` | **"tap.buffer.record~: initial min version -- dramatically better performance than the original"** |
| 2019-01-31 | `0b802a7` | "updates to objects to make compatible with latest min api" |
| 2019-01-31 | `9fda3a0` | "tap.buffer.snap~: initial min-based version." |
| 2019-02-24 | `39408a0` | "min api update" |
| 2019-06-06 | `546993b` | **"adding min-lib"** |
| 2019-06-06 | `be8e852` | "updated top-level cmake" |

Key narrative points:
- The port came in **two widely-spaced waves**: an intense three-day burst in
  October 2016 (FFT + sift objects, threading and outlet/inlet API experiments),
  then a **2.5-year gap**, then a January–June 2019 wave (the buffer objects,
  min-lib). This is the same "comes back every few years" rhythm seen elsewhere.
- `0826fdd` is notable for the explicit performance claim: the Min rewrite of
  `tap.buffer.record~` is **"dramatically better performance than the
  original."** This is the strongest authorial endorsement of the Min approach in
  the whole history, and the reason the 2026 revival flagged taptools-min's
  buffer.record~ ring-buffer fade as a deferred optimization to revisit (§8 of
  REVIVAL.md).
- `d5ab9d1`'s **scheduler-thread vs. main-thread output option** for `tap.sift~`
  is a deliberate threading-correctness refinement; the 2026 revival grafted this
  (as a `high_priority` attribute) onto its own re-port.
- `546993b` "adding min-lib" is the last design move — and the one the 2026
  revival explicitly rejected ("all DSP written as plain portable C++ with no
  dependency on `min-lib`," REVIVAL.md §7).

`REVIVAL.md` §8 reconciles this branch object-by-object: the 2026 revival had
independently re-ported 6 of the 7 objects and kept its own versions (citing
bugs in the taptools-min copies — OOB writes, dropped outlets, a `wrap`-vs-clamp
bug), while recovering one genuinely unique object — **`tap.sustain~`** — from
the archive.

### 3.6 `main` (2026) — the revival
57 commits authored "Claude <noreply@anthropic.com>" (machine-assisted, directed
by Place), interleaved with merge commits and CI fixes authored by Place under
his GitHub identities (`109637+tap@users.noreply.github.com`,
`timothyaplace@gmail.com`). The arc: inventory/plan → modern CMake + min-api +
C++20 foundation → first object (`tap.change`) → Tier 1 → Tier 2 → Tier 3 →
Jitter → infrastructure → **`b62bba8` "Prune the corpse"** (deletes the dead
Jamoma `Core/`, the legacy `TapTools/` package, `source/ttblue/`, every legacy
`source/tap.*` wrapper, old SDK trees, `build.rb`, `.travis.yml` — **6,808 files,
~1,370,401 deletions**) → taptools-min reconciliation → spectral reinvention →
test harness + research dossiers. See §5 for how this caps the four attempts.

---

## 4. The "great pruning" of 2014-10-31 (object-by-object)

On Halloween 2014, between **16:19 and 17:45 CDT** (≈90 minutes), Place
methodically retired obsolete objects, one commit per object, each message a
small obituary stating *why*. This is the single densest narrative passage in the
repository. The removals (with the verbatim stated reason and what the commit
deleted):

| Time | Commit | Object(s) removed | Stated reason (verbatim) | Files deleted |
|------|--------|-------------------|--------------------------|---------------|
| 16:19 | `a09b3c5` | `tap.list.join`, `tap.list.slice` | "removing as these are now obsolete because zl is now capable of processing large lists." | (within root commit) |
| 16:26 | `20a25ba` | `tap.onepole~` | "removing as i fixed the problems with max's onepole~ being inaccurate back in 2011 (and this object actually pre-existed the addition of the onepole~ object in max 4 too)." | `.cpp` + `.yml` |
| 16:30 | `caca666` | (maxquery + default-settings) | "removing files made obsolete in recent versions of max" | `taptools.maxquery`, 2 `.maxdefaults` |
| 16:30 | `f68d4fb` | `init/` folder | "init: removing this folder/file as it just creates aliases to bogus objects" | `taptools-init.txt` |
| 16:35 | `7bcc328` | `tap.pulserouter~` | "tap.pulseroute: removing this one as it is another of jesse's objects rather than one of the original taptools." | `.cpp` (282 ln) + `.yml` |
| 16:48 | `b1d37f4` | `tap.xml.sax` | "removing as clumsy patcher-based xml processing makes no sense now that we have much better tools in js etc." | maxref + maxhelp (488 ln) + 2 java classes + maxpat |
| 16:48 | `efa14ab` | (help/ref) | "help/ref spring cleaning" | — |
| 17:01 | `81351a3` | `tap.buffer.norm~` | "removing as this is obsolete due to the 'normalize' message i added to buffer~ for max 6." | `.cpp` (131 ln) + `.yml` |
| 17:04 | `05834a7` | `tap.twopole~` | "removing as this has been replaced by the j.filter~ object in jamoma" | `.cpp` + `.yml` |
| 17:04 | `62bf53f` | `tap.fourpole~` | "removing as this has been replaced by the j.filter~ object in jamoma" | `.cpp` + `.yml` |
| 17:16 | `2955894` | (backwards-compatibility-lib) + `tap.pi` | "going on a diet... eliminating ancient cruft that from years of languish in the 'backwards-compatibility-lib' folder" | **150 files, 57,792 deletions** — incl. `TapToolsIntro.mov` (10 MB), `tap.pi.cpp`, dozens of maxref/maxhelp/abstraction files for retired objects |
| 17:20–17:23 | `4d4b4b7`, `1044772`, `ba1bdfa` | (build artifacts) | "eliminating builds of removed externals" (×3) | compiled externals |
| 17:42 | `5614815` | `tap.applescript` | **"goodbye forever. you won't be missed."** | `.m` (196 ln) + xcodeproj + maxref + maxhelp + 4 `.scpt` examples (1,143 deletions) |

Second wave (2014-11-04 to 11-06), same obituary pattern:

| Commit | Object | Stated reason (verbatim) |
|--------|--------|--------------------------|
| `565cc8c` | `tap.loadbang` | "a bad idea that is finally going away." |
| `1e524a6` | `tap.lfo~` | "the low-res LFO is dead, we aren't trying to squeek out this kind of silly extra performance on G3 processors anymore..." |
| `f8cb542` | `tap.ramp~` | "removing the obsolete build" |
| `05b146c` | `tap.quantize~` | "one of the externs i ever wrote is nevertheless pretty useless, time to stop being sentimental..." |
| `dba5251` | `tap.path` | "obsolete... ~ is now resolved natively in Max as well as Desktop:/ and a few others. The prefs path code here is no obsoleted by Apple. None of this code is cross-platform. Good bye." |
| `6663ab5` | `tap.buildassist` | "obsoleted by the Projects feature in Max 6, which will become the one true way to build applications in the future." |
| `1d12788` | `tap.colorspace` | "you should now use j.unit from jamoma for these colorspace/unit conversions. j.unit addresses the bugs and inaccuracies that were present in tap.colorspace." |
| `bf750e3` | `tap.diff~` | "no need to maintain something this simple as an external anymore -- do it with gen~." |
| `f29e81d` | `tap.decibels~` | "this object was written for Max 3.6.x -- the atodb~ and dbtoa~ objects were later introduced in Max 4, making this object mostly obsolete back in 2001. for the other couple of idiosyncratic features you can now use j.unit~ from Jamoma. Retiring..." |

**Why the pruning matters as a source:** the messages encode a precise
technical-historical record — *which* native Max feature or Jamoma object
superseded *which* TapTools object, and roughly *when* (Max 3.6, Max 4, Max 6,
2011, 2001). They reveal the objects' deep age (some "pre-existed Max 4,"
"written for Max 3.6.x," "back in 2001"), Place's role inside Cycling '74 ("i
fixed the problems with max's onepole~"; "the 'normalize' message i added to
buffer~"), and authorship distinctions ("another of jesse's objects rather than
one of the original taptools" — distinguishing Jesse Allison's contributions).
The full retired-object inventory is preserved in `REVIVAL.md` §4–5.

---

## 5. The four modernization attempts (and the recurring pattern)

TapTools' git history is, structurally, **four attempts to drag a 1999-era
codebase onto a modern toolchain**, each stalling at the same wall — the dead
Jamoma dependency and a Mac-only build — separated by multi-year gaps. The
recurring rhythm ("every few years he comes back to this") is visible in the
dates alone: 2015, 2016, 2016–2019, then 2026.

### Attempt #1 — jamoma2 port + Travis CI (2015)
- **What it tried:** Escape the dead old-Jamoma library by rewriting objects on
  **jamoma2** (the modern rewrite), vendor dependencies as git **subtrees** (`4a20802` "removing
  submodules, will replace with subtrees"; squash-imports of `max-sdk` and
  `Core`), and add **Travis CI** (`9d951691` "first stab at travis integration";
  `c5b9ec0` "faking the language so we can run CI on OSX").
- **What it achieved:** Exactly **one object** ported — `tap.fourpole~`
  (`c180cba`, 2015-07-29: "reviving this ancient object, but now building it on
  the jamoma2 library"), plus a resonance fix (`d7c7d51`). A *second* object,
  `tap.rotate`, was revived five months later (`a4c2a05`, 2015-12-25: "bringing
  it back from the dead...").
- **Why it stalled:** jamoma2 was itself young and would itself be abandoned.
  Per REVIVAL.md, the `source/jamoma2` submodule "was never even cloned" — the
  port died after 1–2 objects. Travis-on-Xcode-6.1 later rotted.

### Attempt #2 — CMake + AppVeyor Windows CI (2016)
- **What it tried:** Replace the Mac-only Ruby/Xcode build with **CMake**, and
  add **Windows CI via AppVeyor**, so cross-platform support would be built in
  rather than bolted on. All on the `windows` branch, 2016-03-18.
- **What it achieved:** "initial cmake success on the mac using tap.fourpole~ as
  the **test rabbit**" (`8c671d9`) — a working Mac CMake build for one object,
  and the `_tilde` source-naming convention. AppVeyor config scaffolding
  (`0214903` + four "appveyor config" iterations).
- **Why it stalled:** The branch ends mid-iteration on the *same day*. No
  evidence of a green Windows build; the jamoma2 dependency that #1 left dangling
  was never made to build on Windows.

### Attempt #3 — Cycling '74 Min port (2016–2019)
- **What it tried:** A clean reimplementation on Cycling '74's modern **Min**
  (C++ wrapper) framework, on its own branch/repo. Two waves (Oct 2016 FFT/sift;
  Jan–Jun 2019 buffers + min-lib).
- **What it achieved:** ~7 objects ported (`tap.fft.list~`, `tap.fft.normalize~`,
  `tap.sift~`, `tap.buffer.record~`, `tap.buffer.snap~`, plus prototypes), with
  the standout result that the Min `tap.buffer.record~` had **"dramatically
  better performance than the original"** (`0826fdd`), and threading refinements
  (sift~ scheduler/main-thread option, `d5ab9d1`).
- **Why it stalled:** Last commit 2019-06-06; the upstream Cycling '74 repo was
  later **deleted**. The work was effectively lost until the 2026 revival
  rediscovered and archived it.

### Attempt #4 — the 2026 full revival (`main`)
- **What it tried:** The decisive break: **cut Jamoma entirely**, reimplement all
  DSP as portable C++20 with **Min as a thin wrapper only** (no `min-lib`
  dependence), CMake + GitHub Actions for **macOS universal (arm64+x86_64) AND
  Windows from day one** (REVIVAL.md §6–7). Machine-assisted, directed by Place.
- **What it achieved:** All ~48 core objects across Tiers 1–3 + all 5 Jitter
  objects + infrastructure ported and compile-verified; the spectral set
  *reinvented* standalone (`tap.vocoder~`/`tap.nr~`/`tap.spectra~`); the dead
  trees finally removed ("Prune the corpse," `b62bba8`); **and the three prior
  attempts folded in** — `tap.fourpole~`/`tap.rotate` re-cut where attempt #1's
  jamoma2 source was gone, and attempt #3's `taptools-min` reconciled
  object-by-object (REVIVAL.md §8).
- **Status:** Per REVIVAL.md §9, the headline gap is **runtime validation in a
  live Max** (everything is compile/unit-tested against a mock kernel).

**The pattern, stated plainly:** each attempt picked a then-modern escape hatch
(jamoma2, CMake/AppVeyor, Min, then full standalone C++20), got one or a handful
of objects across, then stalled on the same two structural blockers — the dead
dependency and the dead build system. The 2026 attempt is the first to attack
the *dependency* itself (cut Jamoma) rather than swap one framework for another,
which is why it is also the first to finish.

---

## 6. Authorship and identity across eras

The author name/email variants in `git log --all` are themselves a historical
record of where Place was, and (in 2026) of the human/machine division of labor:

| Identity | Commits | Era / meaning |
|----------|---------|---------------|
| `Timothy Place <tim@74objects.com>` | 1 | The gh-pages stub (2012) and the 74objects (his own studio) identity. |
| `timothy place <tim@74objects.com>` (lowercase) | 45 | The 2014 pruning + 2015 subtree/Travis work. The lowercased name is the dominant open-source-era signature. |
| `+1m place <tim@74objects.com>` | 1 | `0e2f1de` (2015-05-15, "updated jamoma core and max sdk") — a garbled/aliased commit-name variant, same email. |
| `Timothy Place <tim@cycling74.com>` | 33 | The Cycling '74 employment era: the 2015–2016 jamoma2/CMake/AppVeyor work AND the entire `taptools-min` Min port (2016–2019). |
| `Claude <noreply@anthropic.com>` | 57 | The 2026 machine-assisted revival commits (the per-object ports, REVIVAL.md updates, research dossiers). |
| `Timothy Place <109637+tap@users.noreply.github.com>` | 5 | 2026 GitHub-web identity — the PR merges (#27–#31). |
| `Timothy Place <timothyaplace@gmail.com>` | 2 | 2026 personal identity — the hand-authored CI fixes and the taptools-min reconciliation commit. |

Reading the table: the `74objects.com` → `cycling74.com` → `gmail.com` email
progression tracks Place's professional moves (independent studio → Cycling '74
→ post-Cycling '74). The 2014 prune is in his own name (lowercase); the 2016–2019
Min port is on the company clock (`cycling74.com`); the 2026 revival cleanly
separates **machine-authored object work** (`Claude`) from **human-authored
direction, CI, merges, and reconciliation** (the two GitHub/gmail identities).

---

## 7. What git cannot tell us (the pre-2014 gap)

Despite being a rich source, git has a **hard floor**, and it must be stated as a
documented gap:

- **The oldest commit is `d560c00` (2012-11-06)** — but it is only the empty
  `gh-pages` template stub. It carries a *date* but no *content*.
- **The earliest real TapTools content is `a09b3c5` (2014-10-31 16:19)** — and
  even that commit *opens* with a deletion. The full pre-2014 tree was imported
  wholesale in this single root commit with **no incremental history behind it.**
- Therefore **the entire development history from ~1999 to 2013 is absent from
  git.** We know from the commit *messages* that objects date back that far —
  `tap.decibels~` "was written for Max 3.6.x … mostly obsolete back in 2001";
  `tap.onepole~` "pre-existed the addition of the onepole~ object in max 4";
  `tap.fourpole~` is "this ancient object." But the *commits* that created them
  do not exist in this repository. The pre-2014 era survives only as **fossil
  evidence inside later commit messages, retired-object docs, and the REVIVAL.md
  inventory** — not as recoverable source history.
- Consequently, git can date and explain the **retirements** of the early
  objects precisely (the 2014 pruning), but it **cannot** date their original
  authorship, show their evolution, or reveal the commercial/Hipno/TTBlue/Jamoma
  lineage that predates the GitHub import. Those threads must be sourced
  elsewhere (the companion research dossiers; archival recovery).

In short: **git is authoritative for 2014-10-31 onward and silent before it.**
Every date and quotation in this chronicle lives inside that window; the
1999–2013 origin story is the documented blind spot.

---

## Appendix: corroboration with the author's REVIVAL.md

`REVIVAL.md` (on `refs/heads/main`) is the author's own prose account of the
revival and aligns with the git facts:
- Its era table ("2013 monolithic→modular on old Jamoma; 2014 pruned + subtrees;
  2015 Travis + jamoma2, **abandoned after 1–2 objects**") matches attempts #1's
  git record exactly.
- "Only `tap.fourpole~` was ported to it [jamoma2]" matches `c180cba`; the
  jamoma2 submodule "never even cloned" matches the empty `source/jamoma2`.
- §8 documents the `taptools-min` branch as a "previously-unknown … official
  Cycling '74 effort … since deleted upstream … preserved as the `taptools-min`
  branch," with "an independent history with no shared base with `main`" —
  matching this chronicle's §3.5.
- The "Prune the corpse" step (REVIVAL.md §6 step 5 / §7) matches commit
  `b62bba8`.

Where REVIVAL.md is *interpretation*, git is *evidence*; the two agree, which is
why the timeline above can be stated with commit-level confidence.
