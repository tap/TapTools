# The master phase and its corrections: `vco.h`

The user-facing chapter promised an oscillator whose folded harmonics sit
~47 dB down, whose analog section is "exactly zero by default" with a seed
that works like a serial number, and whose FM survives through zero. Each
claim is a theorem about this file or a measurement of it. This appendix
derives the corrections — polyBLEP, the leaky triangle, the sync patch —
then the analog-character section, including the one place where honest
analysis contradicted intuition and the tests were written to match.

## One phase, many readings

There is a single accumulator, `m_phase` ∈ [0,1), advanced once per sample
in `step`:

```text
f_eff = base_hz · 2^(cents/1200) + fm_hz      (pitch is exponential,
dt    = f_eff / m_sr,  clamped to ±0.49        FM is linear, in Hz)
adt   = max(|dt|, 1e-8)
```

`cents` collects detune, drift, jitter, the per-unit tolerance offset, and
`track` — everything musical multiplies; only FM adds. Every waveform is a
*reading* of the same phase: sine through `sin`, saw as 2p−1, pulse as a
comparison against `pw`, triangle as an integral. The `shape` morph
crossfades adjacent readings of one phase, so it can never produce a
discontinuity the phase itself doesn't have. The problem is entirely the
discontinuities.

![The VCO as a diagram: frequency sum into the master phase accumulator, fanning to the four waveform readings and the shape crossfade](../images/vco/block-diagram.svg)

*The fan-out the chapter title promises: every waveform is a reading of the same φ.*

## The residual: deriving `poly_blep`

A naive saw jumps by −2 at the wrap; a step's spectrum falls at only
6 dB/oct, so its harmonics march past Nyquist and fold back inharmonic.
The ideal fix is a *band-limited* step — the integral of a sinc. The
polyBLEP observation: the band-limited step differs from the naive step
only near the edge, so instead of storing sinc-integral tables (minBLEP),
approximate the difference with a polynomial. Take the crudest kernel one
sample wide per side — a unit-area triangle, b(τ) = 1 − |τ| on τ ∈ [−1, 1]
with τ in samples from the discontinuity — integrate, subtract the step:

```text
s_bl(τ) = (τ+1)²/2             τ ∈ [−1, 0]
s_bl(τ) = 1 − (1−τ)²/2         τ ∈ [0, 1]

r(τ) = s_bl − s_naive:
r(τ) = (τ+1)²/2                τ ∈ [−1, 0)      (before the edge)
r(τ) = −(1−τ)²/2               τ ∈ [0, 1]       (after the edge)
```

Scale by 2 — the saw's wrap step — and these are exactly the file's
`poly_blep` branches: just after the wrap (`t < dt`, normalized t/dt = τ),
`t + t − t*t − 1` = −(1−τ)² = 2·r; just before it (`t > 1.0 − dt`,
normalized (t−1)/dt = τ ∈ [−1,0)), `t*t + t + t + 1` = (τ+1)² = 2·r.
Three properties fall out of the derivation:

- **Continuity at the window edges.** r(−1) = r(1) = 0: the correction
  fades in and out without new discontinuities of its own.
- **The midpoint property.** r(0⁻) = +½, r(0⁺) = −½: the corrected edge
  passes through the middle of the jump, as a true band-limited step does.
- **"±1 sample" precisely.** A sample lands in a branch iff its phase is
  within `dt` of the wrap, and phase moves `dt` per sample — exactly the
  last sample before and the first after each edge are touched, ever;
  the scope trace still looks like a saw.

The triangle kernel approximates the sinc (its spectrum is sinc², not a
brickwall), so suppression is finite and measurable: the notebook drives a
3951 Hz saw, whose 13th harmonic folds to 3364 Hz, and measures it at
−26.7 dB naive versus −74.2 dB with polyBLEP — **47.5 dB of suppression**.
The notebook's sample-level zoom shows the mechanism: the corrected saw
passes through +0.588 and −0.856 on its way down, where the naive saw
jumps in a single step.

## Saw, pulse, and the second BLEP

`saw_at` is the reading minus the residual: `2·bent(p,·) − 1 −
poly_blep(p, dt)` (subtracted: the wrap step is −2, `poly_blep` is
normalized for +2). `pulse_at` is ±1 with *two* edges: the rising edge at
the wrap (step +2, residual added) and the falling edge at p = pw
(step −2, residual subtracted) — the latter evaluated at `wrap01(p − pw)`,
re-centering the phase coordinate so that edge sits at zero of its own
window and the same branches apply. Calibration is pinned by measurement:
a bipolar pulse at duty d must average 2d − 1, and the notebook measures
−0.800 / −0.500 / +0.000 at 10 / 25 / 50 %; the kernel test holds the 25 %
mean within ±0.03.

## The triangle: integrate, but leak

A triangle is the integral of a square — the classic analog trick. A ±1
square at frequency f forces slope ±4f (2 units in half a period 1/(2f)),
so the per-sample increment is ±4·f/fs = ±4·dt, which is `tri_tick`'s
scaling exactly:

```text
m_tri_state = 0.999 · m_tri_state + 4.0 · adt · sq
```

giving peak ±1 with no post-normalization. The 0.999 is the honesty tax:
the BLEP-corrected square's samples do not sum to exactly zero per period
(the two edges land at different sub-sample positions, so their
corrections don't cancel — and `tri_pw` skew under `imperfect` makes the
imbalance deliberate), and a pure integrator would ramp that residue to
infinity. The leak turns the integrator into a one-pole highpass with
corner fs·(1 − 0.999)/2π ≈ 7.6 Hz at 48 kHz — far below any audible
fundamental, high enough to hold DC bounded.

Why the integrated square is *correctly* antialiased: integration
multiplies the spectrum by 1/ω, −6 dB/oct. The square's already-suppressed
alias residual was generated near Nyquist, where 1/ω is smallest, so
integrating a BLEP square *improves* the alias-to-harmonic ratio — the
correction gets cheaper exactly where the waveform gets harder, which is
why this hardware trick survives digitally intact.

## Through-zero FM

Because FM adds in Hz after the exponential pitch math, `dt` can go
negative and the phase genuinely runs backward — that is all "through
zero" means, and why the sidebands stay coherent when the modulation
swings past the carrier. Two guards make it safe. The BLEP windows use
`adt = |dt|`: a window is a *duration*, one sample each side of an edge,
whichever way the phase travels (with a 1e-8 floor so the `t /= dt`
normalization survives a frozen phase). And `dt` is clamped to ±0.49: at
|dt| ≥ 0.5 the window tests `t < dt` and `t > 1 − dt` would overlap and
every sample would be "at an edge" — the clamp keeps the effective
frequency below Nyquist, where the model means anything at all. Measured:
a 500 Hz sine under ±900 Hz of FM at a 100 Hz rate — depth past the
carrier, genuinely through zero — puts its sideband at −13.7 dB with
−158.3 dB between the lines (144.5 dB of contrast), bounded at |y| = 1.00.

## Hard sync, one-sided

A rising zero crossing on the sync input (`m_sync_prev ≤ 0`, `sync > 0`)
resets the phase. Linear interpolation locates the crossing inside the
sample:

```text
frac    = m_sync_prev / (m_sync_prev − sync)      ∈ [0, 1)
m_phase = wrap01((1.0 − frac) · dt)
```

— the phase restarts from zero at the crossing and accumulates only the
remaining fraction of the sample, so sync pitch is sub-sample accurate
(the notebook's synced slave measures periodic at 110.1 Hz against a
110 Hz master). The reset is still a discontinuity of size
`d = waveform_out_peek(p_old, …) − waveform_out_peek(wrap01(p_new), …)`,
sized on the *morphed* waveform without advancing the triangle integrator:

```text
x = 1.0 − frac        correction += d · 0.5 · x²
```

This is a first-order polynomial BLEP, honestly cruder than the saw's:
**one-sided**, because the pre-reset sample is already output when the
edge arrives — a reset cannot be predicted — and a one-sided patch can
never reproduce the full band-limited edge (the midpoint property needed
both sides). d·½x² is the triangle-kernel residual for a step landing x
into a sample; the code feeds it x = 1 − frac, the elapsed fraction
*since* the crossing, so its weighting runs opposite to the two-point post
branch (½·frac²) — at the first-order accuracy a one-sided patch can
claim, both are O(d) click reducers vanishing at one end of the window.
The header flags minBLEP tables as the wholesale upgrade; a `m_pending`
slot, read and cleared each sample but never written, is scaffolding for
the second correction sample it would need.

## The analog section, derived (2026-07)

**Two time scales of pitch noise.** `tick_drift` is sample-and-hold noise
redrawn every `m_sr / 2` samples (~2 Hz) smoothed by a one-pole with
`a = 1 − exp(−2π·0.5/m_sr)` — the exact discrete step of a 0.5 Hz lowpass.
`tick_jitter` is the same structure at ~80 Hz through ~40 Hz: the fast
companion, trembling where drift strolls. Both are depth-scaled in cents
into the pitch path. Measured: relative period spread 2.01×10⁻⁷ at jitter
0 versus 2.74×10⁻³ at 10 cents — four orders of magnitude of
micro-instability, still under the test's 0.02 ceiling.

**The bent ramp, honestly.** `imperfect` bows the saw via
`bent(p, bend) = p + bend·p·(1−p)`, `bend = 0.35·imp·m_tol_curve`. The
parabola vanishes at both endpoints, so the wrap step stays exactly 2 and
the BLEP stays correctly sized — why the bend lives *inside* the ramp
reading. Now the honest part. In x = p − ½ the saw 2p−1 = 2x is odd (a
sine series) while the parabola p(1−p) = ¼ − x² is even (a cosine series):
the bend's Fourier content is in **quadrature** with the saw's own
components. Harmonic k gains an orthogonal part of relative size bend/(πk)
that moves its *magnitude* only at second order — about 0.05 dB at k = 1
for the maximal bend of 0.35, per √(1 + (bend/πk)²): a scope-obvious shark
fin, almost no harmonic-magnitude shift. Discovered by measurement — and
the kernel test matches the truth: it asserts the waveform bow (interior
deviation > 0.03 at `imperfect 1`) and makes no harmonic-magnitude claim.

**Where the spectral work is actually done.** The reset corner rounds
through a one-pole whose cutoff closes from ~22 kHz toward ~8 kHz
(`fc = min(22000 − 14000·imp, 0.45·m_sr)`, coefficient cached against
`m_round_imp`): measured, the saw's 40th harmonic (17.6 kHz) is 6.4 dB
quieter at `imperfect 1`; the test requires > 4 dB. The triangle skews via
`tri_pw = clamp(0.5 + imp·m_tol_tri·0.01, 0.05, 0.95)` — duty asymmetry in
the integrated square is even harmonics — measured: triangle h2 rises from
−185.5 dB (numerically absent) to −34.1 dB at `imperfect 0.8`. The sine
reads a mildly bent phase (`bent(p, 0.5·bend)`), the pulse width takes a
static offset up to ±1.5 %, the whole unit a pitch offset up to ±2 cents
(`m_tol_cents`).

**Which unit you own.** The tolerances come from a *separate* stream:
`compute_tolerances` hashes the seed (`m_seed * 2654435761u + 12345u`)
into its own local LCG, never touching the runtime `m_rng`. The contract:
`clear()` resets `m_rng = m_seed` and all noise state but does not re-roll
tolerances — resetting the oscillator must never change which unit off the
production line you own; only `set_seed` re-rolls, because changing the
serial number *is* changing the unit. Every tolerance is scaled by
`imperfect` at use, yielding the contract the section rests on: **at
`imperfect 0`, every seed is bit-identical to the ideal oscillator.** The
kernel test renders seeds 7 and 8 and requires `ya == yb` — exact equality
over 24000 samples — and the notebook confirms it; conversely, with
`drift 20` seeds 7 and 8 diverge by up to 0.183 (measured) while the same
seed renders bit-identically, and the test pins that at `imperfect 0.6`
different seeds are audibly different units.

**`track`.**

```text
cents += track · log₂(base_hz / 440.0)
```

A V/oct converter's calibration error grows linearly in octaves from its
trim point; this is that line, exact at 440 Hz *by construction*
(log₂ 1 = 0). Measured at `track 5`: −15.0 / −10.0 / −5.0 / +0.0 / +5.0 /
+10.0 / +15.0 cents across −3…+3 octaves; the test holds the trim point
under 1 cent, ±3 octaves within 2 cents of ±15.

## The engineering ledger

- **Determinism is structural.** All randomness flows from one 32-bit LCG
  (1664525 / 1013904223) seeded by `m_seed` (0 remapped to 1); no wall
  clock, no `std::random` — renders, tests, and mc. stacks reproduce
  bit-for-bit, and the jitter test pins same-seed bit-identity.
- **Off means exactly off.** `tick_drift`/`tick_jitter` return before
  consuming the RNG at zero depth — a default-configured oscillator never
  advances `m_rng`, so *different seeds render identically* until a
  stochastic feature is engaged. The corner-rounding pole is gated on
  `imp > 0.0`, its state primed while bypassed (`m_round_lp = y`) so
  engaging `imperfect` mid-note is click-free.
- **The triangle integrator ticks only when the morph needs it.**
  `waveform_out` short-circuits the crossfade endpoints (`a <= 0.0`), and
  `tri_tick` is stateful — skipping it when unused is a cost saving *and*
  a correctness rule (parking at pure saw must not silently integrate);
  sync sizing uses `waveform_out_peek` for the same reason.
- **Clamps with reasons:** `dt` at ±0.49 (window overlap / Nyquist), `adt`
  floored at 1e-8 (division in `poly_blep`), `tri_pw` in [0.05, 0.95] and
  `pw` in [0.01, 0.99] (an edge pair must stay two distinct windows).
- **The house frame:** per-sample linear ramps with an active-count fast
  path, 16 preset slots morphable over time, allocation-free processing,
  setters safe while audio runs — the same bones as `ladder.h` and
  `svf.h`, so the wrapper stays a shim.

## Checkpoint

One master phase; every waveform is a reading of it, and every reading's
discontinuity gets the residual of a triangle-kernel band-limited step —
two samples per edge, continuous at its window boundaries, a measured
47.5 dB of alias suppression at the folded 13th harmonic. The triangle
integrates the corrected square (slope ±4f, hence `4·adt`) with a 0.999
leak; FM adds in Hz so the phase can run backward, `adt` keeping the
windows directionless; sync resets with sub-sample accuracy and patches
the step one-sidedly because resets can't be predicted — minBLEP is the
flagged upgrade. The analog section is derived noise at two time scales, a
quadrature-honest bent ramp, a rounding pole, a skewed duty, and a
calibration line exact at A440 — all drawn from a tolerance stream
`clear()` never touches, all scaled by `imperfect`, and all provably
absent at zero: the ideal oscillator is a test-pinned invariant, not a
default setting.
