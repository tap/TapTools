# TapTools kernel

The portable DSP library behind the TapTools Max package: header-only, plain C++20, **no Max SDK,
no min-api, no Jamoma**. One self-contained header per object under
`include/taptools/`, everything in the `taptools` namespace. The TapTools Max externals are thin
[Min](https://github.com/Cycling74/min-api) shims over these kernels — the same split AmbiTap
uses (AmbiTap = kernel library, AmbiTap-Max = wrappers).

This is a complete standalone CMake project. It lives in `kernel/` of the TapTools monorepo and
is designed to be lifted verbatim into its own repository (`TapTools` = kernel, `TapTools-Max` =
wrappers); every path below is relative to this directory, so the layout and commands are the
same either way.

## What's here

| Kernel | Max object | Contents |
|---|---|---|
| `include/taptools/autowah.h` | `tap.autowah~` | Snow White-style envelope filter (`taptools::autowah`) |
| `include/taptools/ladder.h` | `tap.ladder~` | ZDF Moog-style ladder (`taptools::ladder`) |
| `include/taptools/svf.h` | `tap.svf~` | Simper/Cytomic morphing SVF (`taptools::svf`) |
| `include/taptools/vco.h` | `tap.vco~` | Virtual-analog oscillator (`taptools::vco`) |
| `include/taptools/grm_comb.h` | `tap.5comb~` | GRM comb-bank recreation (`taptools::fivecomb`) |
| `include/taptools/grm_pitchaccum.h` | `tap.pitchaccum~` | GRM PitchAccum recreation (`taptools::pitchaccum`) |
| `include/taptools/conv_engine.h` | `tap.convolve~` | Partitioned (UPOLS) true-stereo convolution (`taptools::conv_engine`) |

Plus, all Max-free:

- **`tests/`** — Catch2 unit tests for the kernels (fetched via FetchContent; run with `ctest`).
  Wrapper-level tests (attributes, Min plumbing) stay with the externals on the min-api harness
  in the Max package.
- **`tools/render/`** — offline WAV renderers (`ladder_render`, `vco_render`, `grm_comb_render`,
  `grm_pitchaccum_render`, `autowah_render`) for listening checks outside Max.
- **`tools/capi/`** — a small C ABI (`taptools_capi`) over the kernels for non-C++ consumers:
  `conv_engine`, `svf`, `ladder`, `vco`, and `autowah`.
- **`notebooks/`** — Jupyter verification notebooks driving the *actual shipping DSP* through the
  C ABI via ctypes (`taptools_py.py`): `convolution_reverb.ipynb`, `svf.ipynb`, `ladder.ipynb`,
  `vco.ipynb`, and `autowah_validation.ipynb` (the hardware-calibration harness for the Snow
  White model — its last section ingests reamped recordings of the real pedal).
- **`bench/`** — CPU benchmarks and the per-machine regression ratchet (see `bench/README.md`).
- **`book/`** — *Tools on Tap*, the mdBook field guide (the AmbiTap/SampleRateTap/MuTap book
  pattern): one chapter per object family, every claim measured by the notebooks/tests. Built
  and published to Pages by `.github/workflows/docs.yml`. First chapter: the `tap.vco~`
  oscillator, including its analog-character section and the honest Moog recipe.

## Build & test

From this directory (as its own repo, or `-S kernel` from the monorepo root):

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
ctest --test-dir build --output-on-failure
```

CMake exports the header-only target `TapTools::taptools`; `TAPTOOLS_BUILD_{TESTS,TOOLS,BENCH}`
and `TAPTOOLS_INSTALL` default to on only when the kernel is the top-level project. Installing
provides a `find_package(TapToolsKernel)` config package.

## How the Max package consumes it

The Max package's `CMakeLists.txt` points its externals at `${TAPTOOLS_KERNEL_DIR}/include`
(`TAPTOOLS_KERNEL_DIR` defaults to the in-tree `kernel/`; override it with
`-DTAPTOOLS_KERNEL_DIR=/path/to/kernel` to build against an external checkout). Once this kernel
is its own repository, the Max package pins it as a git submodule and points
`TAPTOOLS_KERNEL_DIR` there, per the AmbiTap-Max pattern.

## Rules

- Kernels depend on the C++ standard library only. Anything Max-specific belongs in the wrapper.
- Headers are **C++20**. The consuming Max wrapper `.cpp`s and their min-api unit tests compile at
  C++20 as well, so C++20 features are fair game.
- Sharing code *between* kernels is allowed here (that's much of the point — e.g. the radix-2
  FFT duplicated between `conv_engine.h` and `tap.nr~` should consolidate here); the Max-side
  "each object is self-contained" rule applies to the wrapper package, not to the kernel.
- Provenance and algorithm notes live in each header's top comment. The Max package's `REVIVAL.md`
  is the roadmap, including which objects' DSP is still inline Max-side and next to extract.
