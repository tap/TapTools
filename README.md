# TapTools kernel

[![build](https://github.com/tap/TapTools/actions/workflows/build.yml/badge.svg)](https://github.com/tap/TapTools/actions/workflows/build.yml)
[![Tap House Style](https://github.com/tap/TapTools/actions/workflows/style.yml/badge.svg)](https://github.com/tap/TapTools/actions/workflows/style.yml)
[![Docs](https://github.com/tap/TapTools/actions/workflows/docs.yml/badge.svg)](https://github.com/tap/TapTools/actions/workflows/docs.yml)

**📖 [*Tools on Tap*](https://timothy.place/TapTools/)** — the field guide to these kernels,
one measured chapter per object family — is published from this repo on every merge.

The portable DSP library behind the TapTools Max package: header-only, plain C++20, **no Max SDK,
no min-api, no Jamoma**. One self-contained header per object under `include/taptools/`, each in
its own `tap::tools::<family>` namespace. The TapTools Max externals are thin
[Min](https://github.com/Cycling74/min-api) shims over these kernels — the same split AmbiTap
uses (AmbiTap = kernel library, AmbiTap-Max = wrappers).

This is a complete standalone CMake project with its own CI.
[TapTools-Max](https://github.com/tap/TapTools-Max) pins it as its `submodules/taptools` submodule
and points `TAPTOOLS_KERNEL_DIR` there. This repo in turn pins
[DspTap](https://github.com/tap/dsptap) as `submodules/dsptap` for the shared low-level primitives
(real FFT, YIN, PSOLA, phase vocoder), consumed as the `tap::dsp` target.

## What's here

All 29 headers under `include/taptools/`, by family. The third column is the namespace (or, where a
header adds no nested namespace, the class) the kernel lives in.

**Sources, filters, and amplifiers**

| Kernel | Max object | Contents |
|---|---|---|
| `vco.h` | `tap.vco~` | Virtual-analog oscillator, polyBLEP (`tap::tools::vco`) |
| `svf.h` | `tap.svf~` | Simper/Cytomic morphing SVF (`tap::tools::svf`) |
| `ladder.h` | `tap.ladder~` | ZDF Moog-style ladder (`tap::tools::ladder`) |
| `diode_ladder.h` | `tap.diode~` | ZDF TB-303 diode ladder (`tap::tools::diode`) |
| `autowah.h` | `tap.autowah~` | Snow White-style envelope filter (`tap::tools::autowah`) |
| `overdrive.h` | `tap.overdrive~` | LGW-voiced feedback overdrive (`tap::tools::od`) |
| `vca.h` | `tap.vca~` | Voltage-controlled amplifier (`tap::tools::vca`) |
| `adsr.h` | `tap.adsr~` | Virtual-analog ADSR envelope, legacy Jamoma curves as modes (`tap::tools::adsr`) |
| `fuzz.h` | `tap.fuzz~` | Two-stage tone-stacked fuzz on the DAFx-07 cascade (`tap::tools::fuzz`) |
| `touche.h` | `tap.touche~` | The Ondes Martenot intensity key as a published gain law (`tap::tools::touche`) |
| `diffuseur.h` | `tap.metallique~`, `tap.palme~` | The Ondes diffuseurs as driven resonators (`tap::tools::diffuseur`) |
| `ondes.h` | `tap.ondes~`, `tap.triode~` | The Ondes Martenot voice: heterodyne detector and load-line triode stages (`tap::tools::ondes`) |

**Voices, drums, and sequencing**

| Kernel | Max object | Contents |
|---|---|---|
| `tb303_voice.h` | `tap.303~` | TB-303 acid-bass voice (`tap::tools::tb303`) |
| `step_seq.h` | `tap.808.seq~`, `tap.303.seq~` | Shared step-sequencer engine (`tap::tools::seq`) |
| `tr808_kick.h` | `tap.808.kick~` | TR-808 bass drum (`tap::tools::tr808`) |
| `tr808_snare.h` | `tap.808.snare~` | TR-808 snare drum (`tap::tools::tr808`) |
| `tr808_tom.h` | `tap.808.tom~` | TR-808 tom / conga (`tap::tools::tr808`) |
| `tr808_hat.h` | `tap.808.hat~` | TR-808 hi-hat (`tap::tools::tr808`) |
| `tr808_cymbal.h` | `tap.808.cymbal~` | TR-808 cymbal (`tap::tools::tr808`) |
| `tr808_clap.h` | `tap.808.clap~` | TR-808 handclap / maracas (`tap::tools::tr808`) |
| `tr808_rim.h` | `tap.808.rim~` | TR-808 rimshot / claves (`tap::tools::tr808`) |
| `tr808_cowbell.h` | `tap.808.cowbell~` | TR-808 cowbell (`tap::tools::tr808`) |
| `bridged_t.h` | *(shared)* | Bridged-T resonator — the 808's universal voice circuit (`tap::tools::tr808`) |
| `metal_bank.h` | *(shared)* | The 808 "metal bank": six square oscillators + filter voicings (`tap::tools::tr808`) |
| `swing_vca.h` | *(shared)* | Swing-type VCA, RC decay envelopes, white noise (`tap::tools::tr808`) |

**Spectral**

| Kernel | Max object | Contents |
|---|---|---|
| `stft.h` | *(shared)* | Overlap-add STFT scaffold for the spectral kernels (`tap::tools::stft`) |
| `vocoder.h` | `tap.vocoder~` | Channel vocoder (`tap::tools::vocoder`) |
| `nr.h` | `tap.nr~` | Spectral noise reduction (`tap::tools::nr`) |
| `spectra.h` | `tap.spectra~` | Spectral remapping (`tap::tools::spectra`) |
| `conv_engine.h` | `tap.convolve~` | Partitioned (UPOLS) true-stereo convolution (`tap::tools::conv_engine`) |

**Pitch and time**

| Kernel | Max object | Contents |
|---|---|---|
| `tune.h` | `tap.tune~` | Monophonic pitch correction (`tap::tools::tune`) |
| `harmonizer.h` | `tap.harmony~` | Formant-preserving multi-voice harmonizer (`tap::tools::harmony`) |
| `grm_comb.h` | `tap.5comb~` | GRM comb-bank recreation (`tap::tools::fivecomb`) |
| `grm_pitchaccum.h` | `tap.pitchaccum~` | GRM PitchAccum recreation (`tap::tools::pitchaccum`) |

**Tape and loops**

| Kernel | Max object | Contents |
|---|---|---|
| `tape_loop.h` | *(shared)* | Tape reel, wow/flutter transport, generation-loss wear (`tap::tools::tape`) |
| `discreet.h` | `tap.discreet~` | *Discreet Music* two-machine regeneration loop (`tap::tools::discreet`) |
| `airport.h` | `tap.airport~` | *Music for Airports* incommensurate loop bank (`tap::tools::airport`) |
| `garden.h` | `tap.garden~` | Generative event loop on the Bloom principle (`tap::tools::garden`) |
| `tapecho.h` | `tap.tapecho~` | Multi-head tape echo (`tap::tools::tapecho`) |
| `stammer.h` | `tap.stammer~` | Live buffer-stutter rig (`tap::tools::stammer`) |
| `scrub.h` | `tap.scrub~` | Granular scrub over live capture (`tap::tools::scrub`) |

`taptools.h` is the umbrella header that pulls in every kernel above. `stft.h`, `tune.h`,
`harmonizer.h` and `conv_engine.h` reach into `tap::dsp` (the pinned DspTap submodule) for the
real FFT and the pitch primitives; every other kernel is standard library only.

Plus, all Max-free:

- **`tests/`** — Catch2 unit tests for the kernels (fetched via FetchContent; run with `ctest`).
  Wrapper-level tests (attributes, Min plumbing) stay with the externals on the min-api harness
  in the Max package.
- **`tools/render/`** — offline WAV renderers (`diode_render`, `tb303_render`, `ladder_render`, `vco_render`, `radiohead_render`,
  `grm_comb_render`, `grm_pitchaccum_render`, `autowah_render`) for listening checks outside Max.
- **`tools/capi/`** — a small C ABI (`taptools_capi`) over a *subset* of the kernels, for the
  notebooks and other non-C++ consumers. `tools/capi/taptools_capi.h` is the authoritative list of
  what is currently reachable; extend it (and `notebooks/taptools_py.py`) alongside any kernel the
  notebooks need to measure.
- **`notebooks/`** — Jupyter verification notebooks driving the *actual shipping DSP* through the
  C ABI via ctypes (`taptools_py.py`) — never a Python re-implementation. They are committed
  **executed**; re-execute them when behavior changes. Two are hardware-calibration harnesses
  rather than pure measurements: `autowah_validation.ipynb` (its last section ingests reamped
  recordings of the real Snow White pedal) and `tr808_calibration.ipynb`.
- **`bench/`** — CPU benchmarks and the per-machine regression ratchet (see `bench/README.md`).
- **`book/`** — *Tools on Tap*, the mdBook field guide (the AmbiTap/SampleRateTap/MuTap book
  pattern): one chapter per object family, every claim measured by the notebooks/tests. Built
  and published to Pages by `.github/workflows/docs.yml`. Twenty-four user-facing chapters across
  nine parts — sources (`vco`), filters (`svf`, `ladder`, `autowah`), strings/rooms/spirals
  (`convolve`, `5comb`, `pitchaccum`), tape and time (`discreet`, `airport`, `garden`,
  `components`), the machines you ride (`tapecho`, `stammer`, `fuzz`, `scrub`, `diffuseurs`,
  `ondes`), the spectral set (`vocoder`, `nr`, `spectra`), the rhythm
  section (`acid`, `drums`), staying in tune (`tune`), the pedalboard (`overdrive`) — plus a
  recipes part of whole patches, and
  **"The machine, file by file"**: one deep-dive appendix per kernel header
  (SampleRateTap-style) deriving the math, reviewing the code, and recording why each
  algorithm is built the way it is.

## Build & test

From the repository root. The DspTap submodule must be initialized first — several kernels include
`tap/dsp/` headers and the `taptools` target links `tap::dsp`:

```sh
git submodule update --init --recursive
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
ctest --test-dir build --output-on-failure
```

CMake exports the header-only target `TapTools::taptools`; `TAPTOOLS_BUILD_{TESTS,TOOLS,BENCH}`
and `TAPTOOLS_INSTALL` default to on only when the kernel is the top-level project. Installing
provides a `find_package(TapToolsKernel)` config package.

## How the Max package consumes it

The Max package's `CMakeLists.txt` points its externals at `${TAPTOOLS_KERNEL_DIR}/include`, which
defaults to its own `submodules/taptools` — the pinned submodule of this repository, per the
AmbiTap-Max pattern. Override it with `-DTAPTOOLS_KERNEL_DIR=/path/to/taptools` to build the
externals against a sibling development checkout instead of the pin. Externals whose kernel reaches
into `tap::dsp` also link that target, which the Max package's root `CMakeLists.txt` exposes.

DSP changes land here (or in DspTap, then bump `submodules/dsptap` here); TapTools-Max then bumps
its `submodules/taptools` pin.

## Rules

- Kernels depend on the C++ standard library and, where a shared primitive already exists, on
  `tap::dsp` from the pinned DspTap submodule. Nothing Max-specific — that belongs in the wrapper.
- General-purpose signal primitives belong in **DspTap**, one level down; musical, object-level
  kernels stay here. The radix-2 FFT once duplicated between `conv_engine.h` and `tap.nr~`
  consolidated that way, into `tap::dsp::real_fft`.
- Headers are **C++20**. The consuming Max wrapper `.cpp`s and their min-api unit tests compile at
  C++20 as well, so C++20 features are fair game.
- Sharing code *between* kernels is allowed here (that's much of the point — `stft.h` is the
  overlap-add scaffold shared by the spectral kernels, and the `tr808` blocks are shared by the
  eight drum voices); the Max-side "each object is self-contained" rule applies to the wrapper
  package, not to the kernel.
- Provenance and algorithm notes live in each header's top comment. The Max package's `REVIVAL.md`
  is the roadmap, including which objects' DSP is still inline Max-side and next to extract.

## License

MIT — © 2002–2026 Timothy Place. See [`LICENSE`](LICENSE). Every kernel, test, tool and benchmark
source file carries an `SPDX-License-Identifier: MIT` banner. The pinned DspTap submodule is
separately MIT-licensed and the third-party code it vendors (Ooura FFT, CMSIS-DSP) keeps its own
license — see [`submodules/dsptap/NOTICE.md`](submodules/dsptap/NOTICE.md).

Note the consuming Max package, [TapTools-Max](https://github.com/tap/TapTools-Max), remains under
the New BSD License it has shipped under since 2002; this kernel library was relicensed to MIT to
match its DspTap and AmbiTap siblings.
