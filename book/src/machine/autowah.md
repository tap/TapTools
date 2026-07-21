# Detector, law, and a borrowed filter: `autowah.h`

The user-facing chapter ([The pedal that listens](../autowah.md)) made three
measurable claims: the sweep law matches its design to 0.000 cents, the
follower's timing is an honest RC discharge, and sensitivity at the floor
turns the object into a *truly* fixed filter. This appendix derives each,
starting from the decision that `tap.autowah~` is mostly not a new filter.

## The composition decision: don't write a second SVF

The Snow White's core is an LM13700 OTA state-variable filter swept by an
envelope. TapTools already ships an SVF kernel whose defining property —
proved in [the SVF appendix](svf.md) — is A-stability under per-sample
cutoff modulation. An envelope-swept filter moves its cutoff *every sample by
construction*; the property the wah needs most is exactly the one `svf.h`
already guarantees by theorem. So `wah_filter` owns a
`tap::tools::svf::svf_filter` member (`m_svf`) and drives it through the
signal-rate path, `m_svf.tick(m_cutoff)` then `m_svf.process(0, x)`, once
per sample. The house rule makes this legal: objects under
`source/projects/` stay self-contained, but *inside the kernel repo* sharing
between kernels is encouraged — `autowah.h` simply `#include "svf.h"`.

Composition buys something subtler than saved code: the corner-identity
argument. When the envelope is off, the wah *is* a bare SVF, and that is a
testable equation rather than a resemblance. The kernel test
("sensitivity at the floor is the cocked-wah: bit-close to a bare svf at
bias") runs the wah at sensitivity −60 dB, bias 800 Hz, resonance 0.7
against a separately constructed `svf::svf_filter` fed
`ref.process(x, 800.0)`, and requires `maxerr < 1e-12` over half a second of
signal. The wet paths are arithmetic-identical — same `tick(cutoff)` entry,
same clamps, same solve — so why 10⁻¹² and not `==`? The dry leak: at
`mix` = 100 the mix angle is θ = π/2, and while `sin(π/2)` is exactly 1.0 in
doubles, `cos(π/2)` rounds to 6.123×10⁻¹⁷, so the output carries a
6×10⁻¹⁷·x dry residue. The tolerance covers one ulp-scale cosine, nothing
else. Resonance meaning is shared the same way — the wah's 0..1 knob goes
through the SVF's own `q_from_resonance` mapping, so "resonance 0.7" means
the same Q in both objects.

## The detector: gain, rectifier, follower

The detector chain in `process()` is three lines:

```text
driven = key · m_sens_gain                           (dB → linear input gain)
rect   = |driven|            (full-wave, default)
       or max(driven, 0)     (half-wave, the traced single-diode topology)
m_env += coef · (rect − m_env)                       (one-pole follower)
```

**The sensitivity floor is a contract, not a clamp.** `update_derived` maps
the dB knob as

```text
m_sens_gain = 0                    if sens_db ≤ −60 dB
            = 10^(sens_db / 20)    otherwise
```

−60 dB is not "very quiet" (that would be gain 0.001); it is *exactly zero*.
With `m_sens_gain = 0` the rectifier output is identically 0, `m_env` decays
to 0 and stays there, `m_sweep = tanh(0) = 0`, and `map_cutoff(0)` returns
`m_bias` exactly — the test asserts `w.cutoff_hz() == 250.0` with `==`. That
is what makes the pedal's secondary "cocked wah" mode a true fixed filter
rather than an approximately-fixed one that still breathes a few cents with
the input. Factory slot 3 is that voicing as data.

**The follower coefficient is the exact RC discretization.** The analog
detector is a capacitor charged toward the rectified signal:
env′ = (rect − env)/τ. Solving that ODE exactly over one sample period
T = 1/fs gives env[n] = rect + (env[n−1] − rect)·e^(−T/τ), which rearranges
to the code's recurrence with

```text
a = 1 − e^(−1/(τ·fs))        (the file: m_attack_coef = 1 − exp(−1000/(ms·m_sr)))
```

After N = τ·fs samples of a step, the remaining error is
(1−a)^N = e^(−N/(τ·fs)) = e^(−1): the envelope reaches 63.2% in exactly τ.
This is not the cheap approximation a ≈ 1/(τ·fs); the exponential form makes
the ms parameters honest at any rate. The notebook measured it: attack set
to 2.0 ms reaches 63% in **1.94 ms**; decay set to 250 ms falls to 36.8% in
**256 ms**; and a log-domain fit of the release is a pure exponential with
τ = 252 ms and residual **σ = 0.004** — an RC discharge, like the hardware.
The attack/release asymmetry is one branch, `coef = (rect > m_env) ?
m_attack_coef : m_decay_coef` — the diode charges the cap through one
resistance and lets it bleed through another.

**Full-wave default, half-wave option.** The traced hardware detector is a
single diode: it charges only on positive half-cycles, so the envelope
droops between charges at the signal's fundamental. Full-wave rectification
charges twice per cycle and halves the gaps. The follower cannot filter this
out without also slowing the response, so the ripple rides the envelope and
frequency-modulates the cutoff — the hardware's "sweep-rate ripple." The
notebook quantified the A/B on a 110 Hz tone (decay 60 ms): settled envelope
ripple (std/mean) **0.7% full-wave vs 2.6% half-wave**. The kernel defaults
to the cleaner full-wave and keeps half-wave selectable (`set_rectifier()`)
because the flavor question is a hardware-listening question, not a math
question — it waits for the calibration pass.

## The sweep law, and where it is honest about ignorance

```text
m_sweep  = tanh(k_env_knee · m_env)                     k_env_knee = 1.5
m_cutoff = m_bias · 2^(m_sweep · m_range)               clamped to [20 Hz, min(20 kHz, 0.45·fs)]
```

Three deliberate choices:

**(a) Exponential in Hz.** Equal envelope increments move the cutoff by
equal *octaves*, which is how a sweep sounds uniform — pitch perception is
log-frequency. The honest caveat lives in the header and in
`map_cutoff()`'s own comment: the LM13700's frequency is linear in control
*current*, so the pedal's true law hinges on the BJT stage that converts the
envelope voltage into that current — plausibly exponential (a BJT's
collector current is exponential in V_BE), but not yet measured. That is why
the law is one isolated function: if calibration finds a linear V→I driver,
`map_cutoff` becomes `bias + sweep · span_hz` and nothing else in the kernel
changes.

**(b) The tanh soft knee.** Without it, hard playing would pin `sweep` at a
clamp rail — a hard corner in the control trajectory, audible as the filter
slamming its ceiling. With it, the ceiling is approached asymptotically.
The arithmetic at the defaults (bias 250 Hz, range 3.3 octaves,
sensitivity 0 dB):

```text
full-scale DC key → m_env → 1
m_sweep → tanh(1.5) = 0.905
m_cutoff → 250 · 2^(0.905 · 3.3) ≈ 1982 Hz
```

— about 2 kHz, under the asymptotic rail 250·2^3.3 ≈ 2462 Hz, which itself
matches the hardware's published 250–2500 Hz span. The unit test pins the
settled cutoff into (1800, 2100) Hz and separately drives an absurd +24 dB
sensitivity into an 8× full-scale key to confirm the cutoff saturates at the
ceiling (reaching > 99% of it) instead of running away. `range` is signed —
negative sweeps *down* from bias, a deliberate extension the pedal never had.

**(c) Measured.** The validation notebook swept the envelope range and
compared measured cutoff against the designed curve: **max error
0.000 cents**. The law in the code is the law on paper; when hardware
recordings arrive, any disagreement is a fact about the *model choice*, not
about the implementation.

## One filter, two owners: the forwarding discipline

Both kernels ship per-sample parameter ramps. Run both and every set would
be smoothed twice — lagged, and worse, *shaped* (a ramp of a ramp is not a
ramp). So `prepare()` declares a single owner:

```text
m_svf.set_smooth_ms(0.0);   // this kernel owns all smoothing; svf setters snap
```

The wah's own ramp array smooths every audible parameter; the composed SVF's
setters snap instantly to whatever the wah forwards. And forwarding is
change-gated: `update_derived` pushes `set_resonance` / `set_drive` /
`set_circuit` only when the cached values (`m_svf_resonance`, `m_svf_drive`,
`m_svf_circuit`, seeded to −1 to force the first forward) actually differ.
The reason is the SVF's two-tier update from its own appendix: those setters
dirty the *shape* tier (damping, mix weights, drive gain — a `pow` and the
mix logic). While any wah ramp is active, `update_derived` runs every
sample; forwarding unconditionally would re-run the SVF's shape update every
sample of every bias morph even though resonance never moved. Cutoff needs
no gate at all — `tick(cutoff_hz)` lands in the SVF's cutoff cache, which
recomputes the `tan` and solve constants only when the value differs.

## The circuit switch

`drive` at 0 dB runs the SVF's clean linear circuit; anything above engages
`circuit_driven` — tanh band-node limiting, 2× oversampled — as the optional
OTA-flavored color stage. The switch is a threshold in `update_derived`:

```text
circuit = (m_svf_drive > 1e-6) ? circuit_driven : circuit_clean
```

Why is switching circuits mid-stream acceptable? At the switch point drive
≈ 0 dB, so the driven circuit's input gain is 1 and tanh is near-identity at
typical band-node levels — the two circuits compute nearly the same output,
and the transition is benign. Honestly stated: *near*-identity, not
identity. At high resonance the band node runs hot, tanh visibly bends, and
the driven circuit also brings its oversampling path with it — so engaging
drive from zero on a screaming resonant setting is a small audible step. The
abuse test accepts this trade explicitly: resonance 1.0 plus max drive on
square-wave bursts must stay bounded and finite (the SVF's bounded
self-oscillation doing its job), not polite.

Output staging is the equal-power crossfade shared with `tap.crossfade~`:
θ = mix·π/200, `m_dry_gain = cos θ · g`, `m_wet_gain = sin θ · g` — the
master gain g rides both paths, so `gain` never changes the balance.

## The engineering ledger

- **Per-sample cost accounting.** Settled steady state pays: one rectify,
  one follower multiply-add, one `tanh` (knee), one `exp2` (law), the SVF
  solve, two mix multiplies. The `pow(10, ·)`/`exp` calls live only in
  `update_derived`, which runs per sample *while ramps move* and exactly
  once after they settle (`m_derived_dirty` re-arms only when
  `m_ramps_active > 0`). The real recurring cost is the SVF's cutoff-tier
  `tan`, paid whenever the envelope actually moved the cutoff — and skipped
  by the SVF's cutoff cache whenever it didn't (silence, or the cocked wah).
- **`envelope()` and `cutoff_hz()` exist for measurement.** They read
  `m_sweep` and `m_cutoff` after the fact; the C ABI's
  `taptools_wah_process(..., env_out, cutoff_out, n)` taps them per sample,
  and the validation notebook's `trace=True` path — the ground truth its
  STFT peak-trajectory extractor was proven against (0.979 log-frequency
  correlation) — is built on exactly these two accessors.
- **The preset-morph engine is the GRM pattern** (16 slots, the
  `grm_comb`/`grm_pitchaccum` house count): `store_preset` captures ramp
  *targets* (knob positions, never mid-ramp instantaneous values),
  `recall_preset(slot, seconds)` re-targets every ramp so a morph is just
  nine simultaneous ramps — re-targeting mid-morph stays continuous for
  free. The test walks a 100 ms morph and requires bias to move
  monotonically with no step larger than one ramp increment.
- **Factory slots are data, not code**: four `params` structs
  (guitar / bass / slow swell / cocked wah) in slots 0–3. Changing a voicing
  after the hardware session edits numbers, not logic; the test pins them.
- **Structural switches (`mode`, `rectifier`) are not ramped or morphed** —
  interpolating between rectifier topologies has no physical meaning.
- **Anti-denormal on the envelope** (`< 1e-15 → 0`, the `tap.comb~` idiom):
  a decaying exponential otherwise glides into denormals and multiplies its
  CPU cost during silence.
- **Sidechain by signature**: `process(x)` is `process(x, x)`; the wrapper's
  key inlet is the two-argument form. Single-channel by design — per-channel
  envelopes are the correct behavior under `mc.` wrapping.

## The calibration pass, by construction

Every open hardware question maps to one isolated switch point: the sweep
law is `map_cutoff()` (one function), the stock filter tap is `m_mode`'s
default (`mode_lowpass`, flagged in the header as inference), the detector
topology is `set_rectifier()`, the knee is `k_env_knee` (one constant). The
validation notebook is the instrument that will close them: its extractor
recovers the swept peak from wet audio alone, is already calibrated against
the kernel's own trajectories, and its last cell waits for
`snowwhite_*.wav`. When the pedal arrives, disagreements land on named
constants — not on a rewrite.

## Checkpoint

The wah is a detector and a law in front of a borrowed filter. Composing
`svf_filter` puts the per-sample-modulation stability where it is already
proven, and makes "sensitivity off equals a bare SVF" an identity checked to
10⁻¹² (the gap being one rounded cosine). The follower coefficient
1 − e^(−1/(τ·fs)) is the exact RC step — 63.2% in τ by algebra, 1.94 ms
measured for 2.0 set. The sweep law is exponential-in-Hz through a tanh knee
that turns hard playing into asymptotic approach (~2 kHz at the defaults,
under the hardware's 2.5 kHz rail) — measured at 0.000 cents against design,
and honestly provisional, isolated in one function until the real pedal
votes.
