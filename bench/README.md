# Benchmarks

CPU benchmarks for TapTools DSP kernels, starting with `tap.svf~` (`svf_bench`, built from
`kernel/bench/svf_bench.cpp` by the standalone kernel project; the binary lands in
`<build>/bench/`).

The harness times the **kernel only** (portable C++, no Max) over a matrix of configurations —
circuit (clean/driven) × order (2/4/8) × oversampling (1/2/4) × static vs signal-rate-modulated
cutoff — on deterministic noise, reporting the best-of-N ns/sample. A checksum keeps the
optimizer honest and doubles as a smoke check that a speedup didn't change the output wildly.

`diode_bench` does the same for `tap.diode~`. `oversampling_bench` times the six kernels that run
a nonlinear core between the shared Butterworth filters (`ladder`, `diode`, `svf` driven,
`overdrive`, `fuzz`, `ondes`) at each of their oversampling factors over 20 s of audio, and adds
the share of one core that real time at 48 kHz takes; `--case NAME` runs one case alone, for an
instruction counter such as callgrind. It has no committed baseline. The three benches that run
these kernels set flush-to-zero first, as a host does (`tools/support/flush_to_zero.h`): the
Butterworth stage asserts it in a Debug build.

## The ratchet

1. Build and run (from the repo root):

   ```sh
   cmake -S kernel -B build-kernel -DCMAKE_BUILD_TYPE=Release
   cmake --build build-kernel --target svf_bench
   build-kernel/bench/svf_bench            # human table
   build-kernel/bench/svf_bench --json     # machine output
   ```

2. Compare against the committed baseline **for the same machine**:

   ```sh
   build-kernel/bench/svf_bench --json > /tmp/current.json
   python3 kernel/bench/compare.py /tmp/current.json kernel/bench/baselines/<machine>.json
   ```

   Nonzero exit if any case regressed more than 5% (`--max-regress` to adjust).
3. When an optimization lands, re-record the baseline:
   `build-kernel/bench/svf_bench --json > kernel/bench/baselines/<machine>.json` and commit it.
   The ratchet only tightens — a regression has to be justified explicitly.

Absolute numbers are machine-specific, so baselines live per-machine under `baselines/`.
`linux-x64-gcc-container.json` is a development reference (cloud container, Xeon 2.8 GHz, GCC
-O3); the authoritative baseline for release decisions should be recorded on the maintainer's
Mac (arm64), since macOS/arm64 is the primary shipping target. Correctness is gated separately
by the test suites (`ctest --test-dir build-kernel` for the kernel, `ctest --test-dir build` for
the wrappers) — every optimization must keep them green.
