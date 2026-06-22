# Handoff — RtAudio fork: CI + static-analysis (and what's next)

**Date:** 2026-06-22
**Author:** previous Claude Code session (`claude-opus-4-8`)
**For:** a new session that has `tap/rtaudio` in scope

---

## 1. Goal (original mandate)

Fork RtAudio (the real-time audio I/O C++ library) and progressively address
shortcomings: memory leaks, missing tests, compiler warnings, CI with static
analysis, platform coverage, performance.

User decisions so far:
- **Fork target:** a real GitHub fork — now created at **`tap/rtaudio`**.
- **First priority:** **CI + static analysis** (done — see §3).
- **Workflow location:** repo-root `.github/workflows/` so it runs live.
- **Landing style:** branch + PR preferred (unconfirmed; default to that).

## 2. Access constraint (why this handoff exists)

The previous session was scoped to `tap/taptools` only. It could **not**
create/fork repos (`403: Resource not accessible by integration`) nor read/push
`tap/rtaudio`. The work was therefore committed to the **TapTools** repo on
branch `claude/rtaudio-fork-audit-rmobji` under `rtaudio/`, and also exported as
a patch.

**A new session with `tap/rtaudio` in scope should push the work there.**

## 3. What's been done (the deliverable)

A continuous static-analysis layer on top of upstream `thestk/rtaudio`
@ `e5f0774b2156082ec3db998bd6b2a94b66ade8ac` (2026-02-27). Upstream CI only
*builds*; this adds real analysis. **9 files**, no public API/ABI change.

**Infrastructure added**
- `scripts/static-analysis.sh` — single source of truth (local == CI). Targets:
  `warnings`, `cppcheck`, `clang-tidy`, `sanitizers`, `all`. Env knobs:
  `RTAUDIO_APIS` (default `alsa pulse jack oss`), `CXX`, `CLANG_TIDY_STRICT`.
- `.github/workflows/analysis.yml` — jobs:
  - **warnings** (enforcing): `-Wall -Wextra -Werror`, g++ **and** clang++, all
    4 Linux backends.
  - **cppcheck** (enforcing): `warning,performance,portability`, `--error-exitcode`.
  - **clang-tidy** (advisory): uploads report artifact; promote with
    `CLANG_TIDY_STRICT=1`.
  - **sanitizers** (enforcing): ASan+UBSan build+run of the headless probe.
  - **valgrind** (enforcing): definite-leak check of the probe path.
- `.clang-tidy` — curated bugprone/performance/select-modernize checks.
- `scripts/cppcheck-suppressions.txt` — documented suppressions.

**Bugs fixed (all surfaced by the above checks)**
- `RtAudio.h`: ~20 uninitialized `RtApiStream` members + `ConvertInfo` →
  C++11 in-class initializers matching `clearStreamInfo()` (cppcheck
  `uninitMemberVar`).
- `RtAudio.cpp`: removed 3 non-standard VLAs (MSVC-incompatible, clang
  `-Wvla-cxx-extension`): `char dest[MB_CUR_MAX]`→`[MB_LEN_MAX]`; two
  `void *bufs[channels]` (ALSA non-interleaved r/w) → `std::vector<void *>`.
- `RtAudio.cpp`: JACK `snprintf("%d", unsigned)` → `%u`; ALSA `probeDeviceInfo`
  `std::string name` → `const std::string &`; JACK `port.substr(0,n)` self-copy
  → `port.resize(n)`.

**Verification status:** ALL gates verified green locally on Ubuntu 24.04 with
g++ 13.3, clang/clang-tidy 18.1.3, cppcheck 2.13.0, valgrind 3.22. Patch also
applies cleanly to a pristine `e5f0774` checkout and passes there.

## 4. How to land it in `tap/rtaudio` (new session, in scope)

The fork already has the full upstream tree; only the 9-file diff goes on top.

**Option A — via GitHub MCP (no local clone):**
1. `mcp__github__get_file_contents(owner=tap, repo=rtaudio, path=/)` — confirm
   default branch and that baseline == `e5f0774` (if newer, rebase the diff;
   watch `RtAudio.cpp`/`RtAudio.h`).
2. Create branch `static-analysis-ci`.
3. `mcp__github__push_files` the 9 files (contents below / from the patch) in one
   commit. Paths are **repo-root-relative** (e.g. `.github/workflows/analysis.yml`,
   `scripts/static-analysis.sh`, `RtAudio.cpp`, `RtAudio.h`, …).
4. Open PR `static-analysis-ci` → default branch.
5. Watch the Actions run; confirm gates green (consider `subscribe_pr_activity`).

**Option B — local git (if the session can push):**
```sh
git clone https://github.com/tap/rtaudio.git && cd rtaudio
git checkout -b static-analysis-ci
git apply rtaudio-static-analysis.patch   # attached to the handoff
git add -A && git commit -m "Add static-analysis CI and fix surfaced issues"
git push -u origin static-analysis-ci
```

The patch (`rtaudio-static-analysis.patch`) is the canonical source of the
change. The same content also lives in the TapTools repo on branch
`claude/rtaudio-fork-audit-rmobji` under `rtaudio/` (regenerate with
`git diff --relative=rtaudio 4f41b83 808292c`).

## 5. Reproduce / verify locally

```sh
sudo apt-get install -y g++ clang clang-tidy cppcheck valgrind \
  libasound2-dev libpulse-dev libjack-jackd2-dev
./scripts/static-analysis.sh all          # warnings, cppcheck, clang-tidy, sanitizers
# individual: ./scripts/static-analysis.sh warnings|cppcheck|clang-tidy|sanitizers
```
Notes: OSS uses the bundled `include/soundcard.h` (script adds `-Iinclude`).
Sanitizers use g++ (clang asan runtime not packaged on the dev host). Valgrind
gates on *definite* leaks only (ALSA lib globals show as "still reachable").

## 6. Backlog / next steps (priority order)

1. **clang-tidy burn-down → enforcing.** 181 advisory findings, dominated by
   `modernize-use-nullptr` (122), `bugprone-multi-level-implicit-pointer-conversion`
   (14), `performance-enum-size` (12), `modernize-use-equals-default` (9),
   `performance-unnecessary-value-param` (6), `performance-avoid-endl` (5),
   `modernize-use-override` (4). Fix, then set `CLANG_TIDY_STRICT=1`.
2. **`-Wsign-conversion` cleanup.** ~105 warnings (clang `-Wconversion`); large
   but mechanical. Gate once clean.
3. **Real test suite.** Current `tests/` are interactive demos. Add
   assertion-based tests; a **dummy/null backend** would make stream-lifecycle
   (open/start/stop/close, callback, time, error paths) testable in CI without
   hardware.
4. **Platform coverage.** CoreAudio/WASAPI/DirectSound/ASIO can't be
   runtime-tested on Linux CI. Keep them in the cross-compile build matrix
   (existing `ci.yml` MinGW + macOS); add static checks where reachable.
   Consider adding clang-tidy/cppcheck passes with the Windows/mac defines.
5. **Performance.** Audit the callback/convertBuffer path for avoidable copies
   and per-callback allocations; add a benchmark.
6. **Sync discipline.** `UPSTREAM.md` documents provenance + how to pull
   upstream and how to extract from the subtree.

## 7. Key references in the tree

- `UPSTREAM.md` — provenance, extraction, upstream-sync instructions.
- `CHANGES.fork.md` — fork changelog (what changed and why).
- `scripts/static-analysis.sh`, `.clang-tidy`, `scripts/cppcheck-suppressions.txt`.
- TapTools branch: `claude/rtaudio-fork-audit-rmobji` (commits `4f41b83`
  baseline, `808292c` the work).
