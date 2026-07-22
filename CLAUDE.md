# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## What this is

**TapTools** (this repo, `tap/taptools`) — the portable DSP kernel library behind the
[TapTools-Max](https://github.com/tap/TapTools-Max) package of Max/MSP externals: one
self-contained header per object under `include/taptools/`, namespace `tap::tools`, plain C++20
with the standard library only — no Max, no Min, no frameworks. The AmbiTap / AmbiTap-Max split:
DSP lives and is tested *here*; the Max repo is thin wrappers plus a submodule pin of this repo.
Shared low-level primitives (real FFT, YIN, PSOLA, phase vocoder) live one level further down in
the [DspTap](https://github.com/tap/dsptap) submodule (`submodules/dsptap`, target `tap::dsp`) —
general-purpose signal primitives go there; musical/object-level kernels stay here.

## Layout

- `include/taptools/` — one header per kernel (e.g. `tune.h`, `svf.h`, `grm_pitchaccum.h`);
  `stft.h` is the shared overlap-add scaffold for the spectral kernels.
- `submodules/dsptap/` — `tap::dsp` (fft/yin/psola/pvoc + vendored Ooura/CMSIS), pinned.
- `tests/` — Catch2 (FetchContent), pure C++, no Max/min-api/mock kernel; runs via `ctest`.
- `tools/capi/` + `notebooks/` — the verification layer: a C ABI over the same kernel headers the
  externals compile, a ctypes bridge (`notebooks/taptools_py.py`), and *executed* verification
  notebooks. `tools/` also holds render tools (e.g. `tr808_render`).
- `book/` — *Tools on Tap* (mdbook), the field guide; `bench/` — benchmarks.

## Kernel conventions (load-bearing)

- **Shape:** a class in `tap::tools::<name>` with `prepare(sr)` (all allocation happens here),
  per-sample `process(...)`, and setters that are allocation-free and safe while audio runs.
  Worst-case geometry is bought at `prepare()` so runtime parameter changes never reallocate.
- **No zippers:** parameters ride per-sample linear ramps or one-pole slews (see `grm_comb.h` /
  `grm_pitchaccum.h` for the param_index + ramp house pattern).
- **Provenance is documented.** Faithful ports keep magic constants and cite where they came from;
  recreations cite papers/schematics. New DSP is implemented from published literature only —
  an IP policy, not a citation habit.
- **Honest limits in the header.** Known failure modes and material contracts (e.g. PSOLA wants
  harmonic input) are stated where the next reader will look.

## Style

`STYLE.md` + `.clang-format` + `.clang-tidy` are the canonical Tap House copies (synced from
taphouse; CI drift-checks them — never hand-edit). Run `pre-commit install` once per clone; on
Claude Code web the checked-in SessionStart hook does it automatically. clang-tidy compiles with a
clang front end — treat it as a second compiler, not a linter.

## Build & test

```sh
git submodule update --init --recursive
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
ctest --test-dir build --output-on-failure
```

Tests are Catch2 `SCENARIO`s named for the promise they pin. House patterns worth preserving:
**oracle-based measurement** (drive the kernel with synthesized material and measure the *output*
with an independently certified detector — `tap::dsp::yin` — rather than asserting internals; this
is how the tune kernel's 5-cent period-lock bias was caught), and **material contracts** (test
PSOLA-family code with sawtooth, not sine, per its documented contract).

## Verification layer

The notebooks drive the shipping C++ through `tools/capi` via ctypes — never Python
re-implementations. They are committed executed; re-execute when behavior changes. When adding a
kernel that notebooks (or other bindings) should reach, extend `tools/capi/taptools_capi.{h,cpp}`
and `notebooks/taptools_py.py` alongside it.

## The book

`book/` is *Tools on Tap* (mdbook, `src/SUMMARY.md`, `create-missing = false` so SUMMARY entries
must exist). One user-facing chapter per object family plus "The machine, file by file"
appendices. The standing promise: **every performance claim is measured, not remembered** — cite
the executed notebook cell or pinned test that carries each number. Chapter plans live as
`book/PLAN-*.md` drafting records and stay after the chapters ship.

## Release flow

DSP changes land here (or in dsptap, then bump `submodules/dsptap` here); TapTools-Max then bumps
its `submodules/taptools` pin. After a PR merges via rebase/squash, repoint any open consumer pins
at the identical tree on `main` so they stay reachable after branch cleanup.
