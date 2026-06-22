#!/usr/bin/env bash
#
# static-analysis.sh — run RtAudio's static analysis / hygiene checks.
#
# This is the single source of truth used both locally and by CI
# (.github/workflows/analysis.yml). Run it from anywhere; it locates the
# repository root relative to its own path.
#
# Usage:
#   scripts/static-analysis.sh [warnings|cppcheck|clang-tidy|sanitizers|all]
#
# Default target is "all". Each target exits non-zero on failure so CI fails
# loudly, except clang-tidy which is currently advisory (see CLANG_TIDY_STRICT).
#
# Environment:
#   CXX               C++ compiler for the warnings target (default: detect g++/clang++; runs both if unset)
#   CLANG_TIDY_STRICT if "1", clang-tidy findings fail the build (default: 0, advisory)
#   RTAUDIO_APIS      space-separated Linux APIs to check (default: "alsa pulse jack oss")

set -uo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$ROOT"

RTAUDIO_APIS="${RTAUDIO_APIS:-alsa pulse jack oss}"
CLANG_TIDY_STRICT="${CLANG_TIDY_STRICT:-0}"
STD="c++11"

# Map a Linux API name to its preprocessor define.
api_define() {
  case "$1" in
    alsa)  echo "-D__LINUX_ALSA__" ;;
    pulse) echo "-D__LINUX_PULSE__" ;;
    jack)  echo "-D__UNIX_JACK__" ;;
    # OSS uses the OSSv4 <sys/soundcard.h> bundled under include/.
    oss)   echo "-D__LINUX_OSS__ -Iinclude" ;;
    *) echo "unknown API: $1" >&2; return 1 ;;
  esac
}

all_defines() {
  local d=""
  for api in $RTAUDIO_APIS; do d="$d $(api_define "$api")"; done
  echo "$d"
}

fail=0
note() { printf '\n\033[1;34m==> %s\033[0m\n' "$*"; }
err()  { printf '\033[1;31m%s\033[0m\n' "$*" >&2; fail=1; }

# ---------------------------------------------------------------------------
run_warnings() {
  note "Compiler warnings (-Wall -Wextra -Werror)"
  local compilers="${CXX:-}"
  [ -z "$compilers" ] && compilers="g++ clang++"
  local WARN="-Wall -Wextra -Werror"
  for cc in $compilers; do
    command -v "$cc" >/dev/null 2>&1 || { echo "skip: $cc not found"; continue; }
    for api in $RTAUDIO_APIS; do
      local def; def="$(api_define "$api")" || { fail=1; continue; }
      printf '  %-8s %-6s ... ' "$cc" "$api"
      if "$cc" -std=$STD $WARN -I. $def -c RtAudio.cpp -o /dev/null 2>/tmp/warn.$$.log; then
        echo "ok"
      else
        echo "FAIL"; cat /tmp/warn.$$.log; err "warnings: $cc/$api failed"
      fi
      rm -f /tmp/warn.$$.log
    done
  done
}

# ---------------------------------------------------------------------------
run_cppcheck() {
  note "cppcheck"
  command -v cppcheck >/dev/null 2>&1 || { err "cppcheck not installed"; return; }
  cppcheck --version
  if cppcheck --enable=warning,performance,portability \
      --std=$STD $(all_defines) \
      --suppressions-list=scripts/cppcheck-suppressions.txt \
      --inline-suppr --error-exitcode=2 --quiet \
      RtAudio.cpp rtaudio_c.cpp; then
    echo "cppcheck: clean"
  else
    err "cppcheck: findings above"
  fi
}

# ---------------------------------------------------------------------------
run_clang_tidy() {
  note "clang-tidy (advisory unless CLANG_TIDY_STRICT=1)"
  command -v clang-tidy >/dev/null 2>&1 || { err "clang-tidy not installed"; return; }
  clang-tidy --version | head -2
  local report=clang-tidy-report.txt
  : > "$report"
  local rc=0
  for src in RtAudio.cpp rtaudio_c.cpp; do
    echo "  analyzing $src ..."
    clang-tidy "$src" -- -std=$STD -I. $(all_defines) >>"$report" 2>/dev/null || rc=$?
  done
  local n; n=$(grep -cE 'warning:|error:' "$report" || true)
  echo "clang-tidy: $n finding(s); full report in $report"
  if [ "$CLANG_TIDY_STRICT" = "1" ] && [ "$n" -gt 0 ]; then
    err "clang-tidy: $n findings (strict mode)"
  fi
}

# ---------------------------------------------------------------------------
# Build + run the device-enumeration path under ASan/UBSan. This exercises
# construction, API probing and teardown without requiring audio hardware,
# so it runs headlessly in CI.
run_sanitizers() {
  note "AddressSanitizer + UndefinedBehaviorSanitizer (headless probe)"
  local cc="${CXX:-g++}"
  command -v "$cc" >/dev/null 2>&1 || cc=g++
  for api in $RTAUDIO_APIS; do
    local def lib bin
    def="$(api_define "$api")"
    case "$api" in
      alsa)  lib="-lasound" ;;
      pulse) lib="-lpulse -lpulse-simple" ;;
      jack)  lib="$(pkg-config --libs jack 2>/dev/null || echo -ljack)" ;;
      oss)   lib="" ;;
    esac
    bin="/tmp/rtaudio_san_${api}"
    printf '  %-6s build ... ' "$api"
    if ! "$cc" -std=$STD -g -O1 -fsanitize=address,undefined -fno-omit-frame-pointer \
        -I. $def RtAudio.cpp tests/audioprobe.cpp $lib -lpthread -o "$bin" 2>/tmp/san.$$.log; then
      echo "FAIL"; cat /tmp/san.$$.log; err "sanitizer build failed for $api"; continue
    fi
    echo "ok"
    printf '  %-6s run   ... ' "$api"
    if ASAN_OPTIONS=detect_leaks=1:exitcode=99 \
       UBSAN_OPTIONS=print_stacktrace=1:halt_on_error=1:exitcode=99 \
       "$bin" >/tmp/san.$$.out 2>&1; then
      echo "clean"
    else
      local code=$?
      # exit 99 == sanitizer error; other codes are tolerated (no hardware, etc.)
      if [ "$code" = "99" ]; then
        echo "SANITIZER ERROR"; cat /tmp/san.$$.out; err "sanitizer runtime error for $api"
      else
        echo "ok (exit $code, no sanitizer error)"
      fi
    fi
    rm -f /tmp/san.$$.log /tmp/san.$$.out "$bin"
  done
}

# ---------------------------------------------------------------------------
target="${1:-all}"
case "$target" in
  warnings)    run_warnings ;;
  cppcheck)    run_cppcheck ;;
  clang-tidy)  run_clang_tidy ;;
  sanitizers)  run_sanitizers ;;
  all)         run_warnings; run_cppcheck; run_clang_tidy; run_sanitizers ;;
  *) echo "usage: $0 [warnings|cppcheck|clang-tidy|sanitizers|all]" >&2; exit 2 ;;
esac

if [ "$fail" -ne 0 ]; then
  note "FAILED"
  exit 1
fi
note "OK"
