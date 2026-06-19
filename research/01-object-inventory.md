# TapTools — Object Inventory & Code Archaeology

Research notes for a book about TapTools, the Max/MSP external suite by Timothy Place (74 Objects LLC), built on Jamoma.
Repo: `/home/user/TapTools`, branch `claude/taptools-book-planning-6t9hzv`. Dense notes, not prose. All paths absolute.

---

## 1. Top-level orientation

- `source/` — 50 `tap.*` object directories, each containing exactly one `<name>.cpp` plus an `<name>.xcodeproj`. One external per directory. This is the *current* compiled-object set.
- `source/ttblue/` — 53 legacy header (`tt_*.h`) + 1 `.cpp` DSP library ("TTBlue", the proto-Jamoma DSP layer). Several audio objects `#include "../ttblue/tt_*.h"` directly.
- `source/jamoma2/` — git submodule (`https://github.com/jamoma/jamoma2.git`), pinned at `21035c39…` but **uninitialized in this checkout** (empty dir). The modern C++11 rewrite of Jamoma. Only ONE object currently builds on it: `tap.fourpole~`.
- `source/TTClassWrapperMax.{h,cpp}` — the heart of the architecture: an automated wrapper that exposes a Jamoma (TTBlue) class as a Max class. `Copyright © 2008 by Timothy Place`. `wrapTTClassAsMaxClass(TT("limiter"), "tap.limi~", …)` is the canonical one-liner that turns a DSP class into a Max external. Also provides `TTValueFromAtoms`/`TTAtomsFromValue` and Jitter-matrix↔TTMatrix bridges (`TTMatrixReferenceJitterMatrix`, etc.).
- `Core/` — vendored Jamoma1 "Core" (Foundation, DSP, Graph, Modular, Score, AudioGraph, Shared) as a git subtree. `Core/DSP/extensions/` holds the DSP extension libs the wrapped objects load at runtime: `AnalysisLib`, `Crossfade`, `EffectsLib`, `FilterLib`, `FFTLib`, `GeneratorLib`, `ResamplingLib`, `SpatLib`, `WindowFunctionLib`, etc. (objects call e.g. `TTLoadJamomaExtension_FilterLib()`).
- `objectivemax/` — "ObjectiveMax"/`MaxObject` framework, built first by `build.rb`. Objective-C bridge layer (older infrastructure, credited in ReadMe).
- `max-sdk/` — vendored Max SDK (subtree) providing `source/build.rb` used to compile every project in `source/`.
- `build.rb` — Ruby build orchestrator: builds Jamoma `Core/Foundation` + `Core/DSP`, then `objectivemax/MaxObject`, then `build_projects_for_dir("source")`; finally moves `tap.loader.*` into `TapTools/extensions/` and copies the readme. Static linking of Jamoma deps (no `support/` dylib folder — see commented-out block).
- `.travis.yml` — Travis CI (OSX). Commits show CI was bolted on late ("first stab at travis integration").
- `TapTools/` — the **packaged distribution** (a Max Package): `externals/` (48 `.mxo`), `extensions/tap.loader.mxo`, `docs/` (70 `.maxref.xml`), `help/` (78 `.maxhelp`), `extras/TapTools Overview.maxpat`, `patchers/abstractions` + `patchers/jamoma-modules`, plus `interfaces/`, `jsui/`, `templates/`, `misc/`. `package-info.json`: version `4b2`, min Max 6.1.3, "A potpourri of objects for Max".

### Two architectural eras (key finding)

The codebase is a **stratified archaeological site** spanning ~2000–2015:

| Stratum | Mechanism | Objects (in current `source/`) |
|---|---|---|
| **Pure Jamoma1 wrap** (`wrapTTClassAsMaxClass`) | one-line macro wrap of a TTBlue class | `tap.dcblock~`, `tap.limi~`, `tap.noise~`, `tap.pan~`, `tap.pulsesub~` |
| **Hand-written wrapper around TTBlue class** | `#include "../ttblue/tt_*.h"`, manual dsp dispatch | `tap.comb~`, `tap.svf~`, `tap.verb~`, `tap.shift~`, `tap.multitap~`, `tap.procrastinate~`, `tap.radians~`, `tap.buffer.record~`, `tap.fft.binmodulator~`, `tap.jit.colortrack` |
| **Fully hand-written DSP/control** (no DSP lib) | raw Max SDK sample loops | `tap.elixir~`, `tap.autothru~`, `tap.width~`, `tap.random~`, `tap.split~`, `tap.count~`, `tap.counter~`, `tap.zerox~`(*wraps AnalysisLib), most control objects |
| **Modern jamoma2** (C++11) | `Jamoma::LowpassFourPole`, `SampleBundle` | `tap.fourpole~` only — the lone object ported forward |

Copyright dates in headers trace the timeline: oldest are **2000** (`tap.elixir~`, `tap.fft.normalize~`, `tap.sift~`), through 2003–2008, with `tap.fourpole~` rebuilt in **2015**.

---

## 2. Object inventory — summary table (50 source objects)

Story potential: ★★★ = strong book material, ★★ = some, ★ = routine.

| Object | Family | What it does (core idea) | Substrate | Story |
|---|---|---|---|---|
| tap.adsr~ | Audio/env | ADSR envelope gen; linear/exp/hybrid curves; control- or signal-rate trigger | TTBlue `adsr` (GeneratorLib) | ★ |
| tap.autothru~ | Audio/util | Passes inlet 1, or inlet 2 if connected; pure memcpy routing | hand-written | ★ |
| tap.biquadcalc | Audio/control | Calculates 5 biquad coeffs (9 filter types) for `biquad~` | hand-written (RBJ cookbook) | ★★ |
| tap.bits | List/util | Bit-list ↔ integer conversion via shifts | hand-written | ★ |
| tap.buffer.peak~ | Buffer | Returns sample index of peak |amplitude| in a buffer~ | hand-written, atomic lock | ★ |
| tap.buffer.record~ | Buffer | Record into buffer~ with fade-interpolated writes; loop wrap | TTBlue `onewrap` + hand fade | ★★ |
| tap.buffer.snap~ | Buffer | Snaps a ms/sample position to nearest zero-crossing (symmetric search) | hand-written | ★★ (the "HACK" comments) |
| tap.change | List/util | Repeat filter: output only when input ≠ previous (any atom type) | hand-written | ★ |
| tap.comb~ | Audio/DSP | Feedback comb filter w/ lowpass damping in feedback path; signal-rate delay mod | TTBlue `tt_comb` | ★★ (Zicarelli help) |
| tap.count~ | Audio/analysis | Counts non-zero samples; activation state machine, auto-reset/loop | hand-written | ★ |
| tap.counter~ | Audio/analysis | Counts zero-crossing edges up/down with bounds wrap | hand-written | ★★ (Pinkston idea) |
| tap.crossfade~ | Audio/DSP | Multichannel (≤32) crossfade, equal-power/linear, lookup or calc | TTBlue `crossfade` | ★ |
| tap.dcblock~ | Audio/DSP | DC-blocking highpass | Jamoma1 wrap (`dcblock`) | ★ |
| tap.elixir~ | Audio/util | 2–10-input mixer/crossfader, per-inlet slew, auto 1/N gain | hand-written, 9 unrolled perform fns | ★★ (oldest, 2000) |
| tap.fft.binmodulator~ | FFT | Per-FFT-bin LFO modulation (≤512 LFOs, one per bin) | TTBlue `tt_lfo` array | ★★★ |
| tap.fft.list~ | FFT | FFT spectral stream → Max list (clock-deferred) | hand-written | ★ |
| tap.fft.normalize~ | FFT | Normalize FFT real/imag by bin count; DC & Nyquist ×0.5 | hand-written (2000) | ★ |
| tap.filecontainer | Filesystem | SQLite-backed archive: packs files+attrs into one DB file, platform-filtered extract | hand-written + SQLite | ★★★ |
| tap.folder | Filesystem | mkdir/delete/copy/unzip folders; Mac via AppleScript, Win via SHFileOperation | hand-written, platform `#ifdef` | ★★ |
| tap.fourpole~ | Audio/DSP | Stereo 4-pole (24dB/oct) lowpass, freq + resonance | **jamoma2** `LowpassFourPole` | ★★★ (revived 2015) |
| tap.gang | List/util | "Gang" multiple objects via 4 in/4 out; qelem-deferred routing | hand-written | ★★★ (orig. "tap.hipnopp") |
| tap.inquisitor | Util/introspection | Reads attribute values of another *named* object by walking the patcher graph | hand-written (`jpatcher_api`) | ★★ |
| tap.jit.ali | Jitter | Spatial weighted interpolation: weights from a 2D "space matrix" applied to a data matrix | hand-written Jitter | ★★★ (Ali Momeni, NIME'03) |
| tap.jit.colortrack | Jitter | Tracks ≤4 color regions by HSL thresholding; outputs bounds/center/size | hand Jitter + TTBlue `onewrap` | ★★★ (1073 lines, loop-unrolled) |
| tap.jit.kernel | Jitter | Generates 2D radial-basis (Gaussian-ish) kernel matrix from center/size/weight | hand-written Jitter | ★★ |
| tap.jit.proximity | Jitter | Nearest point in a reference matrix to input (x,y) via Manhattan distance | hand-written Jitter | ★ |
| tap.jit.sum | Jitter | Sums matrix (quantity + center of mass); mode 1 = change detection | hand-written Jitter | ★★★ (built for Hipno) |
| tap.limi~ | Audio/DSP | Limiter | Jamoma1 wrap (`limiter`) | ★★ (jhno's limi~) |
| tap.list.index | List/util | [index,value] ↔ flat list; 0- or 1-based | hand-written | ★ |
| tap.loader | Infra | Boot object; posts "TapTools 4.1 | 74objects.com" banner at launch | hand-written (42 lines) | ★★ (the loader mechanism) |
| tap.midimapper | MIDI | Maps/routes MIDI events (note/cc/pressure/bend/prog/aftertouch) by match rules + channel | hand-written | ★★ |
| tap.multitap~ | Audio/DSP | Multi-tap delay (≤100 taps), per-tap time+gain arrays | TTBlue `tt_multitap` | ★ |
| tap.noise~ | Audio/gen | Noise gen: white/pink/brown/blue/gaussian | Jamoma1 wrap (`noise`) | ★★ (gaussian added late) |
| tap.pan~ | Audio/DSP | Stereo panner; equal-power/linear/sqrt; lookup/calc | Jamoma1 wrap (`panorama`) | ★ |
| tap.prime | List/util | Next prime ≥ input | wraps `TTPrime()` | ★ |
| tap.procrastinate~ | Audio/DSP | Granular delay+pitch+pan+gain; randomizes 4 params within ranges; OSC-style msgs | TTBlue `tt_procrastinate` (composite) | ★★★ (named pun) |
| tap.pulsesub~ | Audio/gen | Pulse-wave subtractive oscillator | Jamoma1 wrap (`pulsesub`) | ★ |
| tap.radians~ | Audio/util | Hz↔radians, deg↔radians conversion (signal-rate) | TTBlue `tt_audio_base` | ★ |
| tap.random | List/util | Random float in [-1,1) on bang via LCG | hand-written | ★ |
| tap.random~ | Audio/util | On each zero-crossing emit a new random value in [low,high] | hand-written | ★★ (Pinkston idea) |
| tap.rotate | Util/geometry | 3D Euler rotation of ≤128 coordinate sets (Z→Y→X) | hand-written | ★★★ (Stephan Moore; "back from the dead") |
| tap.route | List/util | Symbol/list router: matches searchstrings at positions, match→L / no-match→R; partial-match mode | hand-written | ★ |
| tap.shift~ | Audio/DSP | Time-domain (OLA/granular) pitch shifter via ratio + window | TTBlue `tt_shift` | ★★ |
| tap.sieve | List/util | Integer gate: passes value only if it equals the stored filter value | hand-written | ★ |
| tap.sift~ | Audio/util | Removes a constant value from a signal; queue non-matches → float or signal out | hand-written (2000) | ★ |
| tap.split~ | Audio/util | Range splitter: in-range→outlet1, else→outlet2, truth flag→outlet3 | hand-written | ★ |
| tap.svf~ | Audio/DSP | Stereo state-variable filter (LP/HP/BP/notch/peak) + LFO freq mod + portamento ramp | TTBlue `tt_svf`+`tt_lfo`+`tt_ramp` | ★★★ ("FARTS LIKE MAD"; sawtooth not ready) |
| tap.verb~ | Audio/DSP | Algorithmic stereo reverb (Freeverb-style combs+allpass) + downsample/limit/clip/dcblock chain | TTBlue `tt_verb` + 7 helpers | ★★★ |
| tap.width~ | Audio/analysis | Measures pulse-width (high-state duration) of a signal in ms | hand-written | ★ |
| tap.zerox~ | Audio/analysis | Zero-crossing detector/counter | Jamoma1 wrap (AnalysisLib `zerocross`) | ★ |

---

## 3. Object families

### A. Audio / DSP `~` objects (the core suite)
- **Filters:** `tap.comb~` (feedback comb + lowpass), `tap.svf~` (state-variable + LFO + portamento), `tap.fourpole~` (24dB lowpass, jamoma2), `tap.dcblock~`. Legacy lineage: `tap.onepole~/twopole~/fourpole~` → superseded by `tap.filter~`/`j.filter~` (see §4).
- **Delay/granular:** `tap.multitap~`, `tap.shift~` (pitch), `tap.procrastinate~` (granular delay+shift+pan+gain randomizer), `tap.buffer.record~`.
- **Reverb:** `tap.verb~` (the most elaborate single DSP object — orchestrates 8 TTBlue sub-modules with a downsampling pipeline).
- **Generators:** `tap.noise~`, `tap.pulsesub~`, `tap.adsr~`, `tap.random~`.
- **Mix/route/spatial:** `tap.elixir~` (mixer), `tap.crossfade~`, `tap.pan~`, `tap.autothru~`, `tap.split~`.
- **Dynamics:** `tap.limi~`.

### B. Buffer analysis (`tap.buffer.*`)
`tap.buffer.peak~` (peak index), `tap.buffer.snap~` (zero-cross snapping), `tap.buffer.record~` (faded recording). All do direct `buffer~` memory scanning with atomic `b_inuse` locking. Docs/help also reference a `tap.buffer.record2~` and historical `tap.buffer.norm~` (retired).

### C. FFT (`tap.fft.*` — designed for `pfft~` patchers)
`tap.fft.binmodulator~` (per-bin LFO bank — the standout), `tap.fft.list~` (spectrum→list), `tap.fft.normalize~` (FFT scaling). Help folder contains `blue.pfft.binmodulator2~.maxpat`, a worked pfft~ subpatch.

### D. Signal-rate analysis/counting
`tap.zerox~`, `tap.counter~`, `tap.count~`, `tap.width~`, `tap.random~`. Theme: turning audio-rate events (zero crossings, non-zero runs) into counts/triggers/randomness.

### E. Jitter (`tap.jit.*` — video/matrix)
`tap.jit.ali` (spatial interpolation), `tap.jit.colortrack` (HSL color tracking), `tap.jit.kernel` (RBF kernel), `tap.jit.proximity` (nearest-neighbor), `tap.jit.sum` (matrix stats + change detection). Docs reference many more retired/abstraction Jitter objects: `tap.jit.delay`, `tap.jit.motion(+/2)`, `tap.jit.pan`, `tap.jit.grayscale`, `tap.jit.getattributes`.

### F. List / numeric utilities
`tap.bits`, `tap.change`, `tap.list.index`, `tap.sieve`, `tap.prime`, `tap.random`, `tap.route`, `tap.gang`, `tap.rotate` (geometry).

### G. MIDI
`tap.midimapper`.

### H. Filesystem / data
`tap.filecontainer` (SQLite archive), `tap.folder` (FS ops).

### I. Introspection / infrastructure
`tap.inquisitor` (reads another object's attributes), `tap.loader` (boot/banner; built into `extensions/` not `externals/`).

---

## 4. Provenance, stories & lineages (the rich vein)

**The git history is itself the primary story source.** Commit messages read like eulogies/retirement notes the author wrote as he pruned the suite. This is unusually candid and is the single best narrative artifact in the repo.

### "Back from the dead" / revivals
- `a4c2a05` — **"tap.rotate: bringing it back from the dead…"** (and header literally: *"algorithm provided by stephan moore"*, `source/tap.rotate/tap.rotate.cpp:1-13`).
- `c180cba` — **"tap.fourpole: reviving this ancient object, but now building it on the jamoma2 library"**; `d7c7d51` fixes resonance. The only object ported to the modern jamoma2 substrate.
- `83555e1` — "adding jamoma2 as submodule" (the modernization beachhead that stalled at one object).

### Objects named after / contributed by people
- **`tap.jit.ali`** — named for **Ali Momeni**. Header: *"Based on patches by Ali Momeni for the NIME'03 conference"* (`source/tap.jit.ali/tap.jit.ali.cpp:9`). ReadMe: "Ali Momeni, for whom tap.jit.ali is named, provided interpolation algorithms, feedback, and testing." Known harmless quirk (ReadMe): posts "attempting to allocate matrix with unknown type" on first instantiation.
- **`tap.rotate`** — 3D rotation algorithm by **Stephan Moore** (in-code credit). ReadMe also credits Moore for `tap.anticlick~`.
- **`tap.random~` / `tap.counter~`** — ReadMe: "Russell Pinkston for the idea of making tap.random~ and tap.counter~."
- **`tap.limi~`** — ReadMe: "jhno for his support of the augmenting of his limi~ algorithm." (i.e. derived from John Eichenseer / jhno's `limi~`.)
- **`tap.comb~`** — ReadMe: "David Zicarelli for help with the original tap.comb~".
- ReadMe SPECIAL THANKS also names Jeremy Bernstein (tap.applescript, tap.myip, tap.windowdrag), Joshua Kit Clayton, Darwin Grosse (tap.plug.* series), Richard Dudas (filter externs, color conversion, **provided code for the list.join/list.slice externals** — note these are now *removed*, see below), Trond Lossius, Nils Peters (extensive feedback; submitted help-patcher fixes), Andrew Pask, Dave Watson & Rob Sussman (Windows port).

### Hipno provenance (commercial-product lineage)
TapTools shares DNA with **Hipno** (the Cycling '74 / Electrotap plug-in suite):
- `tap.gang` header: *"gang multiple objects together (originally called tap.hipnopp)"* (`source/tap.gang/tap.gang.cpp:9`).
- `tap.jit.sum` block comment: *"This function is basically duplicating the behavior of jit.change. It is only here because Hipno required it without the extra dependency on yet another Jitter object."* (`source/tap.jit.sum/tap.jit.sum.cpp:~379`).
- Commit `7bcc328` — "tap.pulseroute: removing this one as it is another of **jesse's objects** rather than one of the original taptools." (Jesse = collaborator; objects co-mingled from Hipno-era work.)

### Deprecated/superseded lineages (the suite shrinking as Max grew)
The recurring theme: an object dies when Max (or Jamoma) absorbs its function. Captured in commit messages:
- **Filters:** `tap.onepole~` (`20a25ba`: "removing as i fixed the problems with max's onepole~ being inaccurate back in 2011 (and this object actually pre-existed the addition of the onepole~ object in max 4 too)"), `tap.twopole~`/`tap.fourpole~` (`05834a7`/`62bf53f`: "replaced by the j.filter~ object in jamoma") — though `tap.fourpole~` was later **resurrected** on jamoma2.
- `tap.decibels~` (`f29e81d`: written for Max 3.6.x, obsoleted by Max 4's atodb~/dbtoa~ — *"Retiring…"*).
- `tap.colorspace` (`1d12788`: "you should now use j.unit from jamoma… j.unit addresses the bugs and inaccuracies that were present in tap.colorspace").
- `tap.buffer.norm~` (`81351a3`: obsoleted by the `normalize` message added to `buffer~` in Max 6).
- `tap.list.join`/`tap.list.slice` (`a09b3c5`: "obsolete because zl is now capable of processing large lists") — i.e. the *Richard Dudas* code, now gone.
- `tap.lfo~` (`1e524a6`: **"the low-res LFO is dead, we aren't trying to squeek out this kind of silly extra performance on G3 processors anymore…"**).
- `tap.loadbang` (`565cc8c`: **"a bad idea that is finally going away."**).
- `tap.applescript` (`5614815`: **"goodbye forever. you won't be missed."**).
- `tap.quantize` (`05b146c`: **"one of the [first] externs i ever wrote… nevertheless pretty useless, time to stop being sentimental…"**).
- `tap.buildassist` (obsoleted by Max 6 Projects), `tap.path` (`dba5251`: "Good bye."), `tap.diff` (`bf750e3`: "do it with gen~"), `tap.xml.sax` (clumsy patcher XML, use js), `tap.ramp~`, `init` (alias-maker for "bogus objects").

### In-code voice / jokes / honest hacks (great pull-quotes)
- `tap.svf~`: **`// THIS OBJECT FARTS LIKE MAD WITHOUT THIS`** (re: the `Z_NO_INPLACE` flag), and a sawtooth-LFO stub: *"sorry - sawtooth isn't quite ready…"*.
- `tap.buffer.snap~`: **`// THE PLUS 1 IS A HACK`** / **`// THE PLUS 2 IS A HACK`** with a comment to the effect of *"these hacks deal with some roundoff or something somewhere in here"* — an honest unresolved-precision admission.
- `tap.jit.sum`: *"THIS IS THE PART OF THE CODE THAT ACTUALLY WORKS / FOR SOME REASON IT ONLY WORKS WITH UNIPLANE DATA"* and a wry *"BECAUSE THEY WANTED TO MAKE LISTS FOR MULTIPLANE (UNLIKE ME)"*.
- `tap.jit.colortrack`: explicit *"UNROLLING THIS FOR-LOOP FOR SPEED"* and *"MAY WANT TO REPLACE SOME BRANCHING WITH FUNCTION POINTERS"* — performance-engineering commentary.
- `tap.filecontainer` & `tap.folder`: Windows `SHFileOperation()` is so unreliable the code notes *"MULTIPLE CALLS TO THIS METHOD WILL EVENTUALLY RESULT IN SUCCESS"*.
- `tap.fft.binmodulator~`: *"THIS IS DONE IN THE … JamomaDSP.dylib ROUTINE ALREADY"* — visible dependency seams.
- `tap.elixir~` ships a printed self-credit string: *"tap.elixir version 1.2, Copyright © 2001 by Timothy Place"*.

### The naming aesthetic (a book theme)
Object names are puns/metaphors, not literal: **elixir** (a magic mix), **procrastinate~** (a *delay*), **inquisitor** (introspection/interrogation), **sift~/sieve** (filtering values), **gang** (ganging objects), **snap** (snap-to-zero-crossing). Worth a section.

---

## 5. Technical substrate (for a "how it's built" chapter)

- **TTBlue → Jamoma1 → jamoma2** is the spine. TapTools is in many ways the *application layer / proving ground* for Place's DSP library work: the same author wrote both the `tt_*` DSP classes (`source/ttblue/`) and the Max wrappers. Many objects are 20–30-line shells over a reusable DSP class — the book's "Jamoma and the wrapper pattern" story.
- **`TTClassWrapperMax`** is the automation that makes a one-line external possible (`wrapTTClassAsMaxClass`). It also bridges Jamoma `TTMatrix` ↔ Jitter matrices and `TTValue` ↔ Max atoms — the seam where three frameworks (Max, Jitter, Jamoma) meet.
- **Runtime extension loading:** objects pull in only the DSP lib they need via `TTLoadJamomaExtension_*()` (FilterLib, GeneratorLib, AnalysisLib, Crossfade, EffectsLib, …). Modular by design.
- **Build:** `build.rb` (Ruby) + per-project `.xcodeproj` + `max-sdk/source/build.rb`; static linking of Jamoma so the package is self-contained (no `support/` dylibs). Travis CI on OSX (late addition).
- **Distribution form:** a Max 6 **Package** (`TapTools/package-info.json`), the then-new modular distribution mechanism — itself a milestone (ReadMe "New In TapTools 4 beta 2: Distributed as a Max Package"). Earlier the suite was a single monolithic `tap.tools` extension (TapTools 3.1 modularized it into per-object externals).
- **The `tap.loader` mechanism:** a no-box loader external placed in `extensions/` that prints the banner and (per ReadMe) "helps guarantee Max will load the correct version of an object, and not find old/conflicting versions in your searchpath" — a real packaging pain-point of the era.

---

## 6. Documentation material already in the repo (what a book can draw on)

A book has substantial existing scaffolding — every shipped object has **both** a reference page and a help patcher:
- **`TapTools/docs/*.maxref.xml`** — 70 Max reference pages (C74 `.maxref.xml` format with the full `_c74_*` XSL/CSS toolchain present). These contain authored object descriptions, inlet/outlet/attribute docs.
- **`TapTools/help/*.maxhelp`** — 78 interactive help patchers (the canonical "how to use it" demos). Includes `blue.pfft.binmodulator2~.maxpat` and a `tap.badge.maxpat`.
- **`TapTools/extras/TapTools Overview.maxpat`** — the package's Extras-menu landing patcher (presentation-mode UI, the suite's front door).
- **`TapTools/patchers/abstractions/`** — ~24 objects that ship as **`.maxpat` abstractions** rather than compiled externals: `tap.5comb~`, `tap.adapt~`, `tap.delay`, `tap.deviate`, `tap.nr~`/`tap.xnr~` (noise reduction), `tap.spectra~`, `tap.vocoder~`, `tap.sustain~`, `tap.smooth`, `tap.semitone2ratio`, `tap.string.sub`, `tap.thru`/`tap.thru~`, `tap.buffer.record2~`, and most surviving `tap.jit.*` helpers (`tap.jit.delay/motion/motion+/motion2/pan/grayscale/getattributes`). **Important:** the "object set" is larger than the 50 externals — a parallel suite of patcher-abstractions exists.
- **`TapTools/patchers/jamoma-modules/{audio,control}`** — Jamoma module wrappers, showing the Jamoma-Modular integration layer.
- **`ReadMe.txt`** — the historical (v4 beta 2, 2013) readme with a detailed multi-version CHANGE LOG and the SPECIAL THANKS roster: a ready-made timeline + credits source. Note it predates the later pruning, so it lists objects since removed (tap.myip, tap.windowdrag, tap.applescript, etc.) — useful for the "what was lost" chapter.

### Discrepancy worth flagging
The shipped `TapTools/docs` + `help` + `patchers/abstractions` describe a **larger object universe** than the 50 compiled `source/` externals. Many documented names (`tap.delay~`, `tap.decay_calc`, `tap.sustain~`, `tap.spectra~`, `tap.vocoder~`, `tap.smooth`, `tap.5comb~`, the extra `tap.jit.*`) have **no surviving `.cpp`** — they are either abstractions or were retired (per §4). The git history is the authority on which were deliberately killed vs. converted to abstractions. A book's inventory should reconcile three lists: compiled externals (50), shipped abstractions (~24), and retired-in-git objects (~20+).

---

## 7. Where the story potential is concentrated (recommendation)

**Lead objects (★★★ — build chapters around these):**
- `tap.verb~` — a complete algorithmic reverb; showcases the multi-module Jamoma composition pattern.
- `tap.svf~` — filter + LFO + portamento; carries the best in-code voice ("FARTS LIKE MAD", unfinished sawtooth).
- `tap.jit.colortrack` — 1073 lines of hand-optimized HSL color tracking; the "real-time CV before it was easy" object.
- `tap.jit.ali` — the human story (Ali Momeni, NIME'03), interpolation, harmless-error footnote.
- `tap.jit.sum` & `tap.gang` — the Hipno provenance thread (commercial → open-source migration; "jesse's objects").
- `tap.fft.binmodulator~` — 512 per-bin LFOs; vivid spectral-processing concept.
- `tap.procrastinate~` — the pun + the composite granular processor.
- `tap.filecontainer` — SQLite-in-Max archive; unusual, very un-DSP, lots of cross-platform war stories.
- `tap.fourpole~` & `tap.rotate` — the "back from the dead" pair; the jamoma2 modernization arc.

**Strong supporting (★★):** `tap.comb~` (Zicarelli), `tap.limi~` (jhno), `tap.counter~`/`tap.random~` (Pinkston), `tap.buffer.snap~` (the honest hacks), `tap.folder` (Windows FS horror), `tap.inquisitor`, `tap.midimapper`, `tap.noise~`, `tap.elixir~` (the 2000-vintage original), `tap.loader` (packaging story), `tap.biquadcalc` (RBJ cookbook lineage).

**Routine (★ — inventory, not chapters):** `tap.autothru~`, `tap.split~`, `tap.count~`, `tap.width~`, `tap.zerox~`, `tap.bits`, `tap.change`, `tap.list.index`, `tap.sieve`, `tap.prime`, `tap.random`, `tap.route`, `tap.radians~`, `tap.dcblock~`, `tap.pan~`, `tap.pulsesub~`, `tap.adsr~`, `tap.crossfade~`, `tap.shift~`, `tap.multitap~`, `tap.buffer.peak~`, `tap.fft.list~`, `tap.fft.normalize~`, `tap.sift~`, `tap.jit.kernel`, `tap.jit.proximity`.

---

## 8. One-paragraph synthesis (what the codebase reveals)

TapTools is a **15-year personal toolbox** (c. 2000–2015) by a single author who was simultaneously building the DSP library it sits on (TTBlue → Jamoma → jamoma2). It reads as stratified geology: 2000-era hand-written sample loops, a 2003–2008 middle layer of `wrapTTClassAsMaxClass` one-liners over reusable DSP classes, and a single 2015 jamoma2 outpost. The suite **shrinks over time** as Max and Jamoma absorb its functions — and the author documents each death in candid, often funny commit messages that double as a design-philosophy commentary ("the low-res LFO is dead… we aren't trying to squeek out performance on G3 processors anymore"; "a bad idea that is finally going away"). Threaded through are human credits (Momeni, Moore, Pinkston, jhno, Zicarelli, Dudas) and a commercial-product lineage (Hipno) that bled into the open-source release. The richest material is not the routine utilities but (a) the few elaborate DSP/Jitter objects (`tap.verb~`, `tap.svf~`, `tap.jit.colortrack`, `tap.fft.binmodulator~`), (b) the provenance/credit stories, and (c) the git-log retirement narrative — a rare first-person record of an artist-programmer deciding what is worth maintaining as the platform evolves underneath him.
