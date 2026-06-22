# Fork changelog

Changes made on top of the upstream baseline (`thestk/rtaudio` @ `e5f0774`).
See `UPSTREAM.md` for provenance.

## 2026-06-22 — CI + static-analysis foundation

### New: continuous static analysis

The original project's CI only *builds* the library across platforms. This
fork adds a static-analysis / hygiene layer, all driven by a single
reproducible script so contributors get the same results locally as in CI.

- **`scripts/static-analysis.sh`** — single source of truth for the checks
  below. Targets: `warnings`, `cppcheck`, `clang-tidy`, `sanitizers`, `all`.
- **`.github/workflows/analysis.yml`** — runs the checks on every push / PR:
  - **Compiler warnings**: builds with `-Wall -Wextra -Werror` under **both**
    `g++` and `clang++`, across all Linux backends (ALSA, PulseAudio, JACK,
    OSS). Enforcing.
  - **cppcheck**: `warning,performance,portability` with `--error-exitcode`.
    Enforcing. Justified suppressions live in
    `scripts/cppcheck-suppressions.txt`.
  - **clang-tidy**: curated `bugprone-*` / `performance-*` / select
    `modernize-*` checks (`.clang-tidy`). Advisory for now — uploads a report
    artifact; promote to enforcing via `CLANG_TIDY_STRICT=1` as findings are
    burned down.
  - **ASan + UBSan**: builds and runs the device-enumeration path headlessly
    under AddressSanitizer + UndefinedBehaviorSanitizer. Enforcing.
  - **Valgrind**: `--leak-check=full` over the probe path; gates on
    *definite* leaks. Enforcing.

### Fixed: issues surfaced by the new analysis

All of the following were caught by the checks above and are verified by them.

- **Uninitialized members** (`RtAudio.h`): `RtApiStream` left ~20 scalar
  members uninitialized in its constructor (cppcheck `uninitMemberVar`); they
  were only set later by `clearStreamInfo()`. Added C++11 in-class default
  initializers (matching `clearStreamInfo()`), plus defaults for `ConvertInfo`,
  so a freshly constructed stream is always in a well-defined state.
- **Variable-length arrays** (`RtAudio.cpp`): replaced three non-standard VLAs
  (rejected by MSVC, flagged by clang `-Wvla-cxx-extension`):
  - `char dest[MB_CUR_MAX]` → `char dest[MB_LEN_MAX]` (compile-time constant).
  - two `void *bufs[channels]` in the ALSA non-interleaved read/write paths →
    `std::vector<void *>`.
- **Format-string mismatch** (`RtAudio.cpp`, JACK): `snprintf(..., "%d", i)`
  with `unsigned int i` → `%u` (cppcheck `invalidPrintfArgType_sint`).
- **Pass-by-value** (`RtAudio.cpp`, ALSA): `probeDeviceInfo(..., std::string
  name)` → `const std::string &` (cppcheck `passedByValue`).
- **Redundant string copy** (`RtAudio.cpp`, JACK): `port = port.substr(0,
  iColon)` → `port.resize(iColon)` (cppcheck `uselessCallsSubstr`).

### Notes

- No public API or ABI changes; all fixes are internal.
- Backends not runtime-testable on Linux CI (CoreAudio, WASAPI, DirectSound,
  ASIO) are still covered by the warnings/cppcheck static checks where the
  code is reachable, and by the existing cross-compile build matrix.
