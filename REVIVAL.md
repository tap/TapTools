# TapTools Revival — Inventory & Plan

> Working document for bringing TapTools back to life in 2026.
> Status as of 2026-06-17. The revival work has been consolidated into `main`
> (Tiers 1–3 + infrastructure + all 5 Jitter objects; legacy preserved on the `legacy`
> branch). See §8 for the `taptools-min` reconciliation.

## 1. Where things actually stand

TapTools is a collection of **~48 shipping Max/MSP externals** (roots back to
1999; last substantive work 2014–2015; a lone README touch in 2020). The repo
is intact — nothing is lost. The "overwhelming mess" is really **three
half-finished migrations stacked on top of each other**:

| Era  | Attempt                                                | State                                   |
|------|-------------------------------------------------------|-----------------------------------------|
| 2013 | Monolithic → modular externals on **old Jamoma** (v4) | Shipped — this is the bulk              |
| 2014 | Pruned obsolete objects; vendored deps as git subtrees | Done                                    |
| 2015 | **Travis CI** + rewrite objects on **jamoma2**         | Abandoned after 1–2 objects             |

### The three real blockers

1. **Dead dependency.** 50 of 52 source objects are thin wrappers over the
   **old Jamoma** C++ library (`TTClassWrapperMax`, `TTDSPInit`, `TT(...)`).
   Jamoma is archived/dead. The 2015 escape hatch (jamoma2) is also abandoned
   *and* its submodule was never even cloned (`source/jamoma2` is empty). Only
   `tap.fourpole~` was ported to it.
2. **Dead build system.** Ruby (`build.rb`) driving Xcode projects + **Travis
   on Xcode 6.1**. Both long gone. Modern Max dev uses the **CMake-based
   `max-sdk-base`**.
3. **Dead binaries.** The checked-in `.mxo` files are `i386 + x86_64` — **no
   arm64**, so they don't run on any Apple Silicon Mac. Everything needs
   rebuilding regardless.

### Decisions locked in

- **Platforms:** macOS (Apple Silicon, universal arm64+x86_64) **and Windows**.
- **Dependency strategy:** **Cut Jamoma** — reimplement DSP standalone on the
  modern Max SDK. No dead dependency dragged along.
- **Scope:** review the full historical object set (current + retired +
  Jamoma-migrated) before committing to a per-object port list.

Only **`ttblue`** (a support lib, likely the SQLite glue for `tap.filecontainer`)
is currently Jamoma-free. Every shipping audio/utility object needs decoupling.

---

## 2. Currently in source (need Jamoma decoupling)

Effort tiers below are a **first-pass estimate** pending per-object code review.
"Cut" = reimplement the DSP/logic directly on the Max SDK.

### Tier 1 — Trivial utility / data (fast)
| Object | What it does |
|--------|--------------|
| `tap.change` | Filter out repetitions (lists/symbols/numbers) |
| `tap.route` | A more flexible `route` |
| `tap.list.index` | Construct/decompose lists by element |
| `tap.sieve` | Only allow filtered values through |
| `tap.prime` | Generate prime numbers |
| `tap.bits` | Bit operations |
| `tap.gang` | Link objects together |
| `tap.random` | Floating-point RNG |
| `tap.radians~` | Convert to/from radians |
| `tap.inquisitor` | Interrogate another object's attributes |
| `tap.biquadcalc` | Calculate biquad coefficients |

### Tier 2 — Simple MSP DSP (moderate)
| Object | What it does |
|--------|--------------|
| `tap.dcblock~` | DC offset filter |
| `tap.noise~` | Colored noise (white/pink/brown/blue/gauss) |
| `tap.pan~` | Stereo panner |
| `tap.crossfade~` | Crossfade two signals |
| `tap.split~` | Signal-rate `split` |
| `tap.autothru~` | Auto pass-through |
| `tap.count~` / `tap.counter~` | Count samples / signal transitions |
| `tap.zerox~` | Count zero crossings |
| `tap.width~` | Measure pulse width |
| `tap.sift~` | Sift samples to control rate |
| `tap.random~` | Signal-rate RNG |
| `tap.pulsesub~` | Pulse-based envelope substitution |

### Tier 3 — Complex DSP (heavier)
| Object | What it does |
|--------|--------------|
| `tap.svf~` | State-variable filter w/ LFO modulation |
| `tap.comb~` | Comb filter with filtered feedback |
| `tap.fourpole~` | 4-pole ladder filter *(already on jamoma2 — will re-cut standalone)* |
| `tap.rotate` | (revived 2015) |
| `tap.verb~` | Reverb |
| `tap.limi~` | Look-ahead limiter |
| `tap.multitap~` | Self-contained multitap delay |
| `tap.procrastinate~` | Cascading delay effect |
| `tap.elixir~` | Gain-structure management |
| `tap.adsr~` | Envelope generator |
| `tap.shift~` | Pitch shifter |
| `tap.fft.binmodulator~` | Modulate FFT bins |
| `tap.fft.list~` | FFT analysis → list |
| `tap.fft.normalize~` | Normalize an FFT |
| `tap.buffer.peak~` | Hottest sample in a `buffer~` |
| `tap.buffer.record~` | Smooth recording to a `buffer~` |
| `tap.buffer.snap~` | Snap to zero-crossings in a `buffer~` |

### Jitter (need the Jitter SDK — separate sub-effort)
`tap.jit.ali` · `tap.jit.colortrack` · `tap.jit.kernel` · `tap.jit.proximity` · `tap.jit.sum`

### Special / infrastructure (investigate individually)
| Object | Note |
|--------|------|
| `tap.loader` | Package loader shim — **may be obsolete** under the modern Max package system; verify before porting |
| `tap.filecontainer` | Bundles files into a SQLite container (uses `ttblue`) |
| `tap.midimapper` | Map MIDI input to user-defined output |
| `tap.folder` | Filesystem create/delete/copy |
| `ttblue` | SQLite support library (already Jamoma-free) |

---

## 3. Formerly existed — retired but **documented** (real shipping objects, source removed)

These have surviving `.maxref.xml` docs and/or help patchers but no current
source. Strong candidates to **resurrect from docs + git history** if still useful:

| Object | What it did | Revive? (first take) |
|--------|-------------|----------------------|
| `tap.delay~` | Sample-accurate delay | ✅ **done** (reconstructed from docs) |
| `tap.delay` | Delay lists/symbols/numbers | ✅ **done** (reconstructed from docs) |
| `tap.sustain~` | Sample-and-loop sounds | ✅ distinctive |
| `tap.vocoder~` | 24-band vocoder | ✅ high-value |
| `tap.spectra~` | Spectral remapping | ✅ distinctive |
| `tap.nr~` | Spectral noise reduction | ✅ high-value |
| `tap.5comb~` | 5× comb filter | maybe |
| `tap.adapt~` | (audio processor) | review |
| `tap.buffer.record2~` | Smooth buffer recording (v2) | merge into `tap.buffer.record~`? |
| `tap.smooth` | Data-stream smoother | maybe (native alts exist) |
| `tap.deviate` | Randomize & "prime" input | maybe |
| `tap.semitone2ratio` | Semitones → frequency ratio | trivial; maybe fold in |
| `tap.string.sub` | String substitution | maybe |
| `tap.thru` / `tap.thru~` | Feedback utilities | maybe |
| `tap.decay_calc` | Feedback coefficient calc | maybe (pair w/ `tap.biquadcalc`) |
| `tap.jit.delay` | Matrix-stream frame delay | Jitter tier |
| `tap.jit.motion` / `motion+` / `motion2` | Video motion detection | Jitter tier |
| `tap.jit.grayscale` | Grayscale conversion | Jitter tier (native alts exist) |
| `tap.jit.pan` | Video panner | Jitter tier |
| `tap.jit.getattributes` | Jitter abstraction helper | Jitter tier |

## 4. Formerly existed — retired (in git history, no current docs)

Recovered from `git log` (source was deleted). Most were intentionally retired:

`tap.applescript` · `tap.buffer.norm~` · `tap.buildassist` · `tap.colorspace` ·
`tap.decibels~` · `tap.diff~` · `tap.lfo~` · `tap.onepole~` · `tap.path` ·
`tap.pi` · `tap.pulserouter~` · `tap.quantize~` · `tap.twopole~`

## 5. Superseded / migrated (per historical changelog)

Documented as retired in favor of native Max or Jamoma — review for **repatriation**
now that Jamoma is also dormant:

| Old TapTools object | Was replaced by | Repatriate? |
|---------------------|-----------------|-------------|
| `tap.colorspace` | `j.unit` (Jamoma) | ⚠️ Jamoma dormant — candidate to bring back |
| `tap.decibels~` | `atodb~`/`dbtoa~` (native) or `j.unit~` | native covers it |
| `tap.onepole~` / `twopole~` / `fourpole~` | `tap.filter~` | `tap.filter~` itself isn't in source — see below |
| `tap.average~` | `average~` (native) | native covers it |
| `tap.degrade~` | `degrade~` (native) | native covers it |
| `tap.diff` | `gen~` | native covers it |
| `tap.path` | native path resolution | native covers it |
| `tap.buildassist` | Max Projects | obsolete |
| `tap.xml.sax` | `mxj` XmlParse | obsolete |
| `tap.svn` | (dropped) | obsolete |

> **Open thread — `tap.filter~`:** the changelog touts it as the flagship
> multichannel filter that replaced onepole/twopole/fourpole, but it is **not in
> the current source tree** and has no maxref. It likely lived in Jamoma
> (`j.filter~`?). Worth deciding whether to rebuild a unified `tap.filter~`
> standalone — it could absorb several of the individual filter objects.

> **Jamoma repatriation:** a deeper pass over the vendored `Core/` Jamoma
> modules (Foundation/DSP/AudioGraph/Modular) can identify `j.*` objects that
> originated from or pair with TapTools and are worth pulling back. Flagged as a
> follow-up survey, not part of the first build.

---

## 6. Recommended path

1. **Proof of life.** Stand up a modern **CMake + `max-sdk-base`** build and get
   **one** Tier-1 object compiling as a **universal macOS external** that loads
   in Max 9. Establishes the template every other object follows.
2. **Windows from day one.** Wire the same CMake build for Windows so we never
   bolt it on later.
3. **CI.** Replace Travis with **GitHub Actions** building both platforms.
4. **March the tiers.** Port Tier 1 → 2 → 3, then Jitter, then evaluate
   resurrecting the documented-but-retired set. Ship incrementally via GitHub
   releases / the Max Package Manager.
5. **Prune the corpse.** Once the new build proves out, remove the dead `Core/`
   Jamoma subtree, old Xcode projects, `build.rb`, Travis config, and stale
   `.mxo` binaries.

### Suggested first proof-of-life object
`tap.change` or `tap.prime` — Tier 1, no audio, minimal surface, exercises the
whole toolchain (build → package → load in Max) without DSP distractions.

---

## 7. Progress log

**Foundation decision (locked):** Build on **Min as a thin wrapper only** — Min
handles the Max plumbing (inlets/outlets, attributes, messages, the DSP perform
loop), while **all DSP is written as plain portable C++** with **no dependency on
`min-lib`**. Rationale: `min-api` + `max-sdk-base` are actively maintained (last
commit 2026-03-24) and compile clean against the current toolchain, but `min-lib`
is the under-maintained piece. Keeping DSP portable means shallow lock-in — the
wrapper is a small, swappable shim if Min ever stalls. Standard: **C++20**.
Targets: **macOS universal (arm64+x86_64) + Windows**, via CMake + GitHub Actions.

- ✅ **Modern build foundation** — root `CMakeLists.txt`, `min-api` submodule,
  GitHub Actions CI (mac+win), universal-binary verification. Replaces the
  retired Ruby/Xcode + Travis build. Dead `jamoma2` submodule removed.
- ✅ **`tap.change`** (Tier 1) — first object; proof of life. CI green; macOS
  binary verified universal.
- ✅ **`tap.dcblock~`** (Tier 2, DSP) — first full **vertical slice**: object +
  reference page (`docs/`) + help patcher (`help/`) + package layout. DSP is
  portable C++ (faithful to Jamoma's R=0.9997); `bypass`/`mute`/`clear`
  preserved.
- ✅ **Tier-1 batch** — `tap.prime`, `tap.sieve`, `tap.list.index`, `tap.bits`,
  each with object + reference page + help patcher. `tap.prime` ports Jamoma's
  exact `TTPrime` stepping; `tap.list.index` improves on the original (correctly
  handles symbol-leading lists). Ported reference pages + help patchers from the
  legacy package.
- ✅ **DSP batch** — `tap.random` (control RNG, faithful LCG), `tap.crossfade~`
  and `tap.pan~` (equal-power / linear / square-root curves computed directly
  from Jamoma's "calculate" formulas, position drivable by a signal inlet or the
  `position` attribute). Self-contained — no `min-lib`, no shared lookup-table
  global. New Min patterns exercised: `samples<N>` multi-output, attribute
  `setter` caching, `has_signal_connection()`.
- ✅ **`tap.noise~`** — white / pink / brown / blue / gaussian noise generator,
  faithful port of Jamoma's TTNoise colouring filters. First generator
  (`sample_operator<0,1>`); gaussian via std normal distribution.
- ✅ **Tier-1 completion** — `tap.route` (flexible router; searchstring is now a
  single `symbol` attribute since Min only specializes `vector<int>`/`vector<number>`
  attributes, not `vector<symbol>`), `tap.gang` (4-in/4-out deferred fan-out via
  per-outlet `queue`, with change-detection breaking feedback loops),
  `tap.radians~` (hz/radians/degrees converter; signal + float outlet, sample rate
  from `samplerate()`), `tap.inquisitor` (queries another object's attributes via
  the Min `patcher`/`box` patcher API — no Jamoma), `tap.biquadcalc` (RBJ Audio EQ
  Cookbook coefficient calculator, control-rate). Each with reference page + help
  patcher ported from the legacy package.
- ✅ **Tier-2 DSP batch** — `tap.split~` (range router, `sample_operator<3,3>`,
  limits drivable by signal or float/attribute), `tap.autothru~` (auto
  pass-through via `has_signal_connection()`), `tap.width~` (pulse-width meter in
  ms), `tap.count~` (gated sample counter), `tap.counter~` (signal-transition
  counter), `tap.zerox~` (zero-crossing counter — faithful port of Jamoma's
  TTZerocross, `sample_operator<1,2>`), `tap.random~` (signal-triggered S&H RNG;
  the original's per-vector edge test is fixed to per-sample so every zero→non-zero
  transition fires). All DSP is portable C++ — no `min-lib`, no Jamoma. New Min
  patterns exercised: `queue` deferral, the `dspsetup` message, the `patcher` API,
  and mixed signal + non-signal outlets. **Every object in this batch was verified
  to compile cleanly with the CMake/Min toolchain.**
- ✅ **`tap.adsr~`** (Tier 3) — attack/decay/sustain/release envelope generator,
  faithful port of Jamoma's TTAdsr (linear / exponential / hybrid curves),
  triggered by the `trigger` attribute or a signal crossing 0.5. Defaults to
  hybrid (the original's true audible default). Unblocks `tap.pulsesub~`.
- ✅ **`tap.sift~`** — both original modes via a `vector_operator` whose single
  outlet is created (signal vs control) to match the mode argument at
  instantiation; the float-dump mode uses an SPSC ring buffer drained on the main
  thread by a `queue`. (Resolved the deferral noted below.)
- ✅ **Tier-3 batch (filters)** — `tap.rotate` (3D coordinate rotation, pure data
  object), `tap.svf~` (stereo Chamberlin state-variable filter + vector-rate LFO +
  portamento ramp — the ttblue tt_svf/tt_lfo/tt_ramp trio folded into one
  `vector_operator`), `tap.comb~` (IIR comb with a lowpass in the feedback loop,
  faithful port of tt_comb). All DSP is portable C++; the LFO is computed directly
  from a phase accumulator (equivalent to the original wavetable). Compile-verified
  against the toolchain; **audio behavior still needs runtime validation in Max.**
- ✅ **`tap.pulsesub~`** (Tier 2, was deferred) — phasor → duty-cycle offset → ADSR
  → multiply, as one `sample_operator`. Resolved now that the ADSR exists.
- ✅ **Tier-3 batch (delays / dynamics / filters)** —
  `tap.multitap~` (circular-buffer multitap delay, per-tap delay/gain via vector
  attributes), `tap.limi~` (stereo look-ahead limiter w/ DC blocker + pre/post gain,
  faithful port of tt_limiter), `tap.fourpole~` (4-pole Moog ladder — **reimplemented**,
  since the 2015 original's jamoma2 `LowpassFourPole` source is not in the repo).
- ✅ **Tier-3 batch (buffers)** — `tap.buffer.peak~`, `tap.buffer.snap~`,
  `tap.buffer.record~`, all rebuilt on Min's `buffer_reference`/`buffer_lock`
  (binding, `set`, notifications, and `dirty()` handled by the framework).
  `snap~`/`record~` write/read in the audio thread via `buffer_lock<true>`.
**Tier-3 batch (mixers / FFT / pitch):** `tap.elixir~` (variable-inlet 2–10
equal-gain crossfade mixer with per-inlet slew; dynamic inlets),
`tap.fft.binmodulator~` (per-bin LFO modulation inside pfft~), and `tap.shift~`
(two-grain Welch-windowed delay-line pitch shifter — the exact 256-point padded
Welch table is embedded and mirrored to 512). Compile-verified; audio behavior
needs runtime validation in Max.

**Infrastructure objects:** ✅ `tap.midimapper` (MIDI-message remapper, pure
control logic) and ✅ `tap.folder` (filesystem make/delete/copy/move —
**modernized** onto portable `std::filesystem`, replacing the original's
AppleScript + Win32-shell code; `unzip` dropped, no portable std support). Their
reference pages/help were restored from git history after the prune.
- ⏸ `tap.loader` — a package-loader shim; **obsolete** under the modern Max package
  system (externals auto-load), so intentionally not ported.
- ⏸ `tap.filecontainer` — bundles files into a SQLite container; deferred (needs a
  SQLite integration, which the Min package doesn't currently provide).

**Remaining frontiers (each its own sub-effort):**
- **Jitter family — ✅ 5 of 5 done.** The matrix→value (analysis) objects are plain
  Min objects that read a named `jit.matrix` through the Jitter API (`c74::max`) — ✅
  `tap.jit.sum` (sum all cells), ✅ `tap.jit.proximity` (nearest 2D point), ✅
  `tap.jit.ali` (Ali Momeni proximity-weighted interpolation), ✅ `tap.jit.colortrack`
  (4-tracker HSL colour tracker — full RGB→HSL, hue-wrap, bounds/centroid/size, all 36
  attributes preserved). The matrix→matrix object uses the MOP path — ✅ `tap.jit.kernel`
  (radial-kernel **generator** built on Min's `matrix_operator<>` as a no-input MOP; the
  original's off-by-one out-of-bounds column write is fixed by the proper MOP cell
  routing). All five compile against the toolchain (JitterAPI links on the mac/win CI;
  the object compile is verified on Linux/GCC too). **Runtime validation in Max still
  pending** for the two new ones.
- **Resurrection candidates** — the two classic delays are now done: ✅ `tap.delay~`
  (sample-accurate audio delay line, circular buffer, ms delay drivable by the right
  inlet or `@delay`, `clear`/`dspsetup`) and ✅ `tap.delay` (control-rate single-pending
  message delay via Min `timer`, faithful to the original `delay`-based abstraction) —
  both reconstructed from their surviving maxref docs (no source survived). Still open:
  the **spectral set** — but a closer look (the surviving `.maxpat` abstractions on
  `legacy`) shows these are **not** standalone DSP externals to "rebuild from docs."
  They are `pfft~`/patcher **abstractions over a chain of other un-ported objects**, so
  each is really a mini sub-project:
    - `tap.spectra~` → a `pfft~ tap.spectra.pfft` subpatcher (the actual spectral
      remapping) plus `tap.typecheck~`. The subpatcher uses `fftin~`/`fftout~` +
      `tap.scale~` to reorder bins. Closest to restorable — but needs `tap.scale~` and
      `tap.typecheck~`, neither of which is ported.
    - ✅ `tap.nr~` — **reinvented** as a self-contained external (originally wrapped
      `pfft~ tap.xnr~`; no surviving source). Runs its **own STFT** — an in-house radix-2
      FFT with a Hann window at 4× overlap and COLA-normalised overlap-add — so no `pfft~`
      host is needed. Each frame's bins below `threshold` are attenuated with a `slope`
      soft-knee. **Unit-tested for perfect reconstruction** (gate open ⇒ output equals
      input delayed by one FFT frame, &lt;1e-6) and for below-threshold attenuation.
      Latency = one FFT frame. maxref rewritten; help patcher ported from the legacy
      abstraction (needs runtime rework). Audio quality still needs Max validation.
    - ✅ `tap.vocoder~` — **reinvented** as a self-contained standalone external (the
      original was a real external; the abstraction just added smoothing/gain around it).
      A bank of 24 log-spaced (50 Hz–12 kHz) RBJ constant-0 dB-peak bandpass biquads
      analyses the modulator (left inlet); a per-band one-pole envelope follower (period
      = `response_interval` ms) shapes the matching carrier band (right inlet); bands are
      summed to the output. Honours the documented `q` and `response_interval` attributes
      (made `number`, not the original's odd `symbol` registration) plus a practical
      `gain`. DSP smoke-tested (silence→silence; a band tone passes). Help patcher ported
      from the legacy abstraction — **needs runtime rework in Max** for the standalone
      object. Audio quality still needs runtime validation.
  > **Decision (author, 2026-06-17): reinvent the lost spectral DSP** rather than defer.
  > `tap.vocoder~` and `tap.nr~` done. Only `tap.spectra~` remains — the patcher-restore
  > path (needs the small `tap.scale~`/`tap.typecheck~` support objects revived).
  (`tap.sustain~` was recovered from the `taptools-min` archive — see §8.)
  > **Doc cleanup flagged:** the legacy `tap.delay.maxref.xml` carries copy-pasted
  > filter boilerplate attributes (`clip`/`coefficients`/`gain`) that don't belong to a
  > delay; ported verbatim but **not** implemented. Trim the maxref when convenient.
- **`tap.filter~`** — the open question of building a unified standalone multichannel
  filter that could absorb several individual filter objects.
- **`tap.verb~` oversampling** and SQLite for `tap.filecontainer` — minor polish.

**Latent-bug fixes made along the way (all noted in-file):** `tap.random~`
per-vector→per-sample edge test; `tap.buffer.snap~` post-clamp loop that could
never terminate; `tap.fft.normalize~` 0.49-biased equality that disabled the
DC/Nyquist halving; `tap.comb~` undefined-when-unset feedback/decay coupling.

**Tier 3 is complete.** The final two — the heaviest objects — are done:
- ✅ `tap.procrastinate~` — four `tap.shift~`-style voices chained in cascade, each
  with a randomized pitch ratio / delay / gain / equal-power pan drawn from
  configurable ranges (regenerated on bang). Embeds the padded-Welch window.
- ✅ `tap.verb~` — stereo Moorer-style reverb reconstructed from tt_verb: an 18-tap
  early-reflection pattern → six LFO-modulated comb filters (damping lowpass in each
  feedback loop) → Schroeder allpass → output lowpass → equal-power dry/wet mix →
  gain, with DC-block and clip stages. Two prime-"deviated" cores give the stereo
  image. *Not yet included from the original wrapper:* the optional look-ahead
  limiter and the internal oversampling (downsample/upsample) — a follow-up polish.

All ~48 core objects across Tiers 1–3 are now ported and compile-verified against
the Min/Max SDK toolchain. **Runtime validation in Max remains the outstanding
step** for the DSP objects (the filters, delays, pitch shifters, and reverb in
particular).

**Established Min patterns now available for the rest of Tier 3:** dynamically
created outlets + `vector_operator` (for variable I/O like the buffer/FFT
objects), SPSC ring buffer + `queue` for audio→control hand-off, per-vector
coefficient updates inside `vector_operator`, and `buffer_reference`/`buffer_lock`
for buffer~ access.

**Convention (tilde objects):** MSP objects whose Max name ends in `~` must have
their **project folder and `.cpp` named with `_tilde`** (e.g.
`source/projects/tap.dcblock_tilde/tap.dcblock_tilde.cpp`). The SDK maps the
project name back to `~` for the output binary, so the object still loads in Max
as `tap.dcblock~`, and its `docs/` + `help/` files keep the `~` name. This
applies to every remaining tilde object (`tap.noise~`, `tap.svf~`, …).

**Package layout:** the repo root is now the Min-DevKit-style package
(`externals/`, `docs/`, `help/`, generated `package-info.json`).

**Jitter completion + delay resurrection + test coverage (2026-06-17, batch 2):**
- ✅ **Jitter family complete (5/5).** `tap.jit.colortrack` (full 4-tracker HSL port —
  integer RGB→HSL, hue-wheel wrap, bounding-box/centroid/size, all 36 attributes,
  dumpout outlet; `ttblue`'s `onewrap` ported inline) and `tap.jit.kernel` (radial-kernel
  generator on Min's `matrix_operator<>` as a no-matrix-input MOP; the original's
  off-by-one column write — which wrote to `x = -1` out of bounds — is eliminated by
  per-cell MOP routing). Docs + help ported from `legacy` for both.
- ✅ **`tap.delay~` / `tap.delay` resurrected** from their maxref docs (no source
  survived) — see the frontiers note above for the implemented surface and the
  flagged maxref-boilerplate cleanup.
- ✅ **Unit-test coverage expanded** from 1 object to 10. New Catch tests with real
  input→output assertions (not just instantiation) for `tap.dcblock~` (filter
  recurrence, bypass/mute/clear), `tap.radians~` (all four conversions), `tap.zerox~`
  (per-sample trigger + normalized count), `tap.prime` (sequence + `next_prime`),
  `tap.sieve`, `tap.bits` (pack/explode round-trips), `tap.list.index`, `tap.change`,
  and `tap.biquadcalc` (RBJ coefficients vs. independently-computed references). `ctest`
  is green: **10/10 passing.**
- **Toolchain note:** `tap.crossfade~` and `tap.pan~` (pre-existing) don't compile under
  Linux/**GCC** (a `-Wchanges-meaning` + enum-`operator==` quirk in the `shapes`/`modes`
  enum pattern); they build fine under the CI clang/MSVC toolchains. Worth a small
  GCC-clean pass if local Linux builds are ever wanted, but not a CI blocker.

**Corpse pruned (step 5 done):** the dead trees have been removed now that all
objects are migrated and the build is self-contained on `min-api` — gone are the
old Jamoma `Core/`, the legacy `TapTools/` package (its docs/help were already
copied into `docs/`+`help/`, and its `.mxo` binaries were i386/x86_64-only),
`source/ttblue/`, every legacy `source/tap.*` wrapper, the old `max-sdk/` and
`objectivemax/` SDK/framework trees, plus `build.rb` and `.travis.yml`. All of it
is preserved in git history if any algorithm needs to be referenced again. The
working tree now contains only the modern package: `CMakeLists.txt`, `source/min-api`
(submodule), `source/projects/`, `docs/`, `help/`, `package-info.json.in`, and the
GitHub Actions CI.

---

## 8. The `taptools-min` reconciliation (2026-06-17)

A second, previously-unknown Min port surfaced: **`taptools-min`** — an official
Cycling '74 effort (`github.com/Cycling74/taptools.git`, 2016–2019), since
**deleted upstream**. The only surviving copy is now preserved as the
**`taptools-min` branch** of this repo (pushed for archival; an independent history
with no shared base with `main`). It contained 7 Min-ported objects plus prototypes.
This revival had already independently re-ported **6 of the 7**, so a per-object
diff decided which implementation wins:

| Object | Outcome |
|--------|---------|
| `tap.fft.list~` | **Keep revival** — taptools-min had an out-of-bounds write and dropped the dumpout outlet. |
| `tap.fft.normalize~` | **Keep revival** — both math-correct; revival adds a divide-by-zero guard. |
| `tap.elixir~` | **Keep revival** — taptools-min was broken (wrong gain model, OOB for >2 channels) *and* depended on `min-lib`. |
| `tap.buffer.snap~` | **Keep revival** — taptools-min had a `wrap`-vs-clamp bug and dropped multichannel / `mode` / `channel` / dumpout. |
| `tap.sift~` | **Keep revival + graft** — added the `high_priority` attribute and a scheduler-thread `timer` delivery path (faithful to the original's clock-based delivery), keeping the main-thread `queue` as the low-priority option. |
| `tap.buffer.record~` | **Keep revival; optimization deferred** — taptools-min's power-of-two ring-buffer fade is faster for large fades, but bit-equivalence to the current (correct, faithful) shift-based fade was not cleanly established and the win is marginal at typical fade sizes. Not worth risking a working recorder; revisit with a proper equivalence/perf test if large-fade performance becomes a need. |

**New from `taptools-min`:**
- ✅ **`tap.sustain~`** — the one genuinely unique object (capture recent audio →
  trim to zero-crossings → crossfaded forward/backward sustaining loop). Ported onto
  the current toolchain (C++20 `std::numbers` instead of MSVC-fragile `M_PI_2`;
  initialized/guarded voice state so it is safe to instantiate before the first
  capture). **EXPERIMENTAL** — the 2019 source was an in-development single-voice
  prototype (multi-voice / `rise` still stubbed). Reference page ported; no help
  patcher yet. Provenance noted in-file; relicensed to the package's New BSD with
  attribution (originally Cycling '74 / MIT — flag for the author if stricter notice
  retention is wanted).
- 📋 **`tap.sustain1~`…`tap.sustain4~` redesign** (+ `tap.sustain.voice.maxpat`,
  `tap.sustain.analyze/calc/find.js`) — a later Max-abstraction reimagining of
  sustain. Left in the `taptools-min` archive branch as design input for a future
  proper `tap.sustain~`.
- 📋 **`tap.autowah~`, `tap.selfconvolve~`** — help-patcher-only prototypes (no C++
  source ever existed); ideas for future objects, in the archive branch.

**Test harness adopted:** the Catch-based `min-object-unittest.cmake` is now wired
up (first real test: `tap.sift_tilde_test.cpp`, guarding the new `high_priority`
default), and CI runs `ctest` on macOS. taptools-min's own test files were empty
stubs; this is the first actual coverage. Pattern established for the rest.

**Platform floor raised:** minimum is now **macOS 11** (tracking Max 9; also
required by `std::filesystem` in `tap.folder`) and **`max_version_min` 9.0**. macOS
11 is set globally via a `-mmacosx-version-min=11.0` compile/link flag in the root
`CMakeLists.txt`, since Min's `min-pretarget.cmake` force-pins the deployment target
to 10.11 and CMake has no per-target deployment property.
