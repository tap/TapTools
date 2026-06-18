# TapTools runtime tests (max-test)

The Catch unit tests under `source/projects/*/...test.cpp` run against a **mock**
Max kernel — they verify pure logic but never load a real object into Max. This
directory adds the missing half: **runtime tests that run inside Max itself**,
using Cycling '74's [`max-test`](https://github.com/Cycling74/max-test) harness
(MIT-licensed, vendored here as a submodule). This is how we finally close the
"needs runtime validation in Max" gap that `REVIVAL.md` keeps flagging — and the
only way to validate the DSP objects' actual audio behavior.

```
runtime-tests/
├── max-test/                 # the harness (git submodule → Cycling74/max-test)
├── patchers/                 # our *.maxtest.maxpat tests (one per object)
├── max-test-config.json      # OSC ports for the Ruby automation runner
└── make_maxtest.py           # generator that emits starter patchers
```

## What a test patcher is

A max-test patcher is an ordinary Max patcher whose filename ends in
`.maxtest.maxpat` and which:

1. **starts itself** (a `loadbang`),
2. contains one or more **`test.assert <name>`** objects (the checks),
3. **terminates itself** when done (a bang into **`test.terminate`**).

Helper objects the harness provides: `test.sample~` (sample a signal — use
instead of `snapshot~`), `test.equals` (tolerant float compare),
`test.string.equals` (multi-line, line-ending-tolerant string compare). See
`max-test/patchers/` for the upstream reference examples (the audio example
`2087-bitxor~.maxtest.maxpat` is the topology our generator mirrors).

## First-time setup (one-time, on your Mac)

The harness needs the **full Max application** installed — it cannot run in CI or
in a headless container. After cloning TapTools:

```sh
git submodule update --init --recursive          # pulls in runtime-tests/max-test
cp runtime-tests/max-test-config.json runtime-tests/max-test/misc/max-test-config.json
```

So Max can find both the test patchers and the `test.*` objects, add these to the
Max file preferences search path (Options → File Preferences), or symlink them
into your Max packages folder:

- `runtime-tests/max-test`   (provides the `test.*` objects)
- `runtime-tests/patchers`   (our tests)
- the TapTools package root  (provides the `tap.*` objects under test)

## Running

**Manually:** just open any `runtime-tests/patchers/*.maxtest.maxpat` in Max and
look at the `test.assert` objects — green/​red shows pass/fail live.

**Automated (all patchers, logged):** the harness ships a Ruby runner that drives
Max over OSC/UDP and writes results to a SQLite DB.

```sh
cd runtime-tests/max-test/ruby
ruby test.rb "/Applications/Max.app"     # point at your Max 9 install
```

It launches Max, waits for the file database, runs every `*.maxtest.maxpat` it can
find in the search path, and summarizes pass/fail. Results land in a SQLite file in
`runtime-tests/max-test/` (open with `sqlite3`).

## Authoring more tests

`make_maxtest.py` generates structurally-valid starter patchers (modeled on the
upstream working example) for both control-rate and signal-rate objects:

```sh
python3 runtime-tests/make_maxtest.py     # regenerates the bundled examples
```

> ⚠️ The generator runs headless (no Max), so a freshly generated patcher is a
> faithful **starting point**, not a guaranteed-passing test. Open each new
> patcher in Max once to confirm the assert/terminate timing, tweak expected
> values against real output, then commit it. The two bundled examples
> (`tap.prime`, `tap.radians~`) cover the two topologies; copy/extend them (or
> call `audio_test()` / `control_test()` from the script) for the rest — the DSP
> objects (filters, delays, reverb, the spectral trio, sustain~) are the priority.

## CI

Running these in GitHub Actions would require a licensed Max install on the
runner, which the SDK CI does not currently provision — so runtime tests stay a
**local, on-Mac** gate for now (the Catch unit tests + the universal-binary `lipo`
check remain the automated CI gates). Wiring a self-hosted macOS runner with Max
installed is the path to automating this later; tracked as a follow-up.
