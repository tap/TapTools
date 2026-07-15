# Two banks and a multiplier: `vocoder.h`

The [user-facing chapter](../vocoder.md) made three flat promises about
`tap.vocoder~`: a silent carrier is silence, gain is exactly linear, and a
silent modulator decays away at the follower rate. It could afford to,
because none of those is a tuning outcome — each one is a structural fact
about a very small graph. This appendix draws the graph, proves the facts,
and then walks the three numerical choices (band placement, filter type,
follower coefficient) that make the graph sound like a vocoder.

One honesty note up front. The original `tap.vocoder~` source did not
survive the revival; `vocoder.h` is *reconstructed from the reference
documentation* — "a basic 24-band vocoder" with `q` and `response_interval`
attributes. The topology below is the classic channel vocoder that
documentation describes, and the tests pin its structural behavior; there is
no lost binary to bit-compare against, and this chapter never pretends
otherwise.

## The graph: a bilinear form in 24 subbands

A channel vocoder is subband multiplication. Split both signals with the
*same* filter bank, measure the modulator's level per band, scale each
carrier band by that level, sum:

```text
band i:   m_i = B_i(modulator)          the modulator through bandpass i
          env_i ← follower(|m_i|)        its envelope
          c_i = B_i(carrier)             the carrier through the identical bandpass
output:   y = gain · Σᵢ c_i · env_i
```

That is `bank::process()` verbatim — the loop body computes `m`, `rect`,
`m_env[i]`, `c`, and accumulates `c * m_env[i]`, and the return line applies
`m_gain` once to the sum. Three contracts follow from the shape alone, and
`tests/vocoder_test.cpp` pins each one:

- **Silent carrier ⇒ exactly silence.** Every summand carries a factor
  `c_i`. A biquad is linear with zero state at rest, so a zero carrier gives
  `c_i ≡ 0` for all i, and the sum is identically zero no matter what the
  modulator (and hence the envelopes) does. The test drives a 220 Hz
  modulator against a zero carrier for 8000 samples and requires peak
  < 10⁻¹², but the true bound is exact: `0.0 * m_env[i]` is 0.0.
- **Gain is exactly linear.** The multiply `c_i · env_i` is the only
  nonlinearity in the graph, and it is *bilinear* — linear in the carrier
  with the envelopes held fixed, linear in the envelopes with the carrier
  held fixed. `m_gain` sits outside all of it, a scalar on the finished sum,
  and nothing upstream reads it. Two banks fed identical inputs with gains
  1 and 2 must differ by exactly a factor of 2, float for float; the test
  requires `|yb − 2·ya| < 10⁻¹²` across 8000 samples.
- **Silent modulator ⇒ output decays at the follower rate.** With the
  modulator silenced, `rect = 0` and each envelope obeys
  `env ← m_env_coef · env` — a geometric decay with the follower's time
  constant. The output is bounded by `Σ|c_i|·env_i`, so it decays with the
  envelopes even while the carrier keeps playing. The test warms the bank
  up, silences the modulator for one second at 48 kHz (≈ 50 time constants
  at the 20 ms default — a decay of e⁻⁵⁰), and requires the late output
  under 10⁻⁴ of the warmed level.

The fourth pinned property, determinism (two identical runs compare equal
with `==`), is the repo-wide claim that the kernel is pure state-machine
arithmetic: no randomness, no time, no allocation in the audio path.

## Where the bands sit

Twenty-four bands span 50 Hz to 12 kHz, log-spaced. `band_frequency(i)`
computes:

```text
f_i = k_fmin · (k_fmax / k_fmin)^(i / (k_bands − 1))     i = 0 … 23
    = 50 · 240^(i/23)
```

so adjacent centres sit at a constant *ratio* of 240^(1/23) ≈ 1.269 — about
0.344 octave, a hair over four semitones, per band. Log spacing is the only
defensible choice for this machine, twice over: the ear judges musical width
by ratio, not by hertz, so equal-ratio bands devote equal perceptual width
to each channel; and speech puts its identity (formants, the envelope the
vocoder exists to capture) in the low kilohertz while its *detail*
(fricatives) rides above — a linear spacing would waste twenty bands above
6 kHz and cram every vowel into two. The span itself brackets speech: 50 Hz
is below any voice fundamental, 12 kHz is above any formant that matters,
and `recalc_filters()` clamps each centre at `0.45 · m_sr` so the top bands
stay well below Nyquist at low sample rates rather than folding.

## The filter: constant peak, unconditional stability

Each band is an RBJ Audio-EQ-Cookbook bandpass, the **constant 0 dB-peak**
variant, computed in `recalc_filters()`:

```text
w0 = 2π · fc / sr        alpha = sin(w0) / (2·q)        a0 = 1 + alpha

b0 =  alpha / a0         a1 = (−2·cos w0) / a0
b1 =  0
b2 = −alpha / a0         a2 = (1 − alpha) / a0
```

"Constant 0 dB peak" is a normalization claim: the gain at the centre
frequency is exactly 1, *for any Q*. It is worth proving, because the whole
level architecture rests on it. Evaluate the transfer function at
z = e^(jw0):

```text
H(z) = alpha·(1 − z⁻²) / [(1 + alpha) − 2·cos w0 · z⁻¹ + (1 − alpha)·z⁻²]

denominator at z = e^(jw0):
  [1 − 2·cos w0 · e^(−jw0) + e^(−2jw0)] + alpha·(1 − e^(−2jw0))
  = e^(−jw0)·(e^(jw0) − 2·cos w0 + e^(−jw0)) + alpha·(1 − e^(−2jw0))
  = e^(−jw0)·(2·cos w0 − 2·cos w0) + alpha·(1 − e^(−2jw0))
  = alpha·(1 − e^(−2jw0))                    = the numerator exactly
```

so H(e^(jw0)) = 1 identically. Why it matters here: `env_i` is supposed to
measure *the signal's* level in band i, and each carrier band is supposed to
be scaled by that measurement and nothing else. With the constant-peak
variant, changing `q` changes bandwidth only — the on-centre gain of all 48
filters stays pinned at unity, so the `q` knob narrows or overlaps the bands
without re-balancing the reconstructed spectrum or re-calibrating the
envelope levels. The cookbook's other bandpass (constant *skirt* gain) has
peak gain Q; with the default q = 20 that would be +26 dB per band, scaling
with the knob — every `q` move would also be a 24-band gain move.

Stability is likewise unconditional. A biquad is stable iff its
coefficients sit in the stability triangle, |a2| < 1 and |a1| < 1 + a2.
Here a2 = (1 − alpha)/(1 + alpha), which lies in (−1, 1) whenever
alpha > 0 — and alpha = sin(w0)/(2q) is positive for any q > 0 and any
0 < fc < Nyquist; the second condition, 2|cos w0|/(1 + alpha) <
1 + (1 − alpha)/(1 + alpha) = 2/(1 + alpha), reduces to |cos w0| < 1, true
on the same range. The code enforces the preconditions rather than
assuming them: q is floored at 0.001 and fc clamped to 0.45·sr, so no
attribute value and no sample rate can produce an unstable band. The
sections run as Direct Form I (`biquad::process` keeps `x1, x2, y1, y2`) —
at these moderate Qs and double precision, the plainest form is the
honest one.

## The follower: one coefficient, symmetric by construction

Each band's envelope is a one-pole lowpass over the full-wave rectified
band signal:

```text
rect     = |m_i|
env_i    ← m_env_coef · env_i + (1 − m_env_coef) · rect
```

with the coefficient computed in `recalc_envelope()` from the
`response_interval` attribute:

```text
tau        = response_ms / 1000                      (ms → seconds)
m_env_coef = exp(−1 / (tau · sr))
```

That is the exact one-sample step of a continuous first-order lag with time
constant τ: the discrete pole e^(−T/τ) with T = 1/sr. So the documented
"analysis period" is a time constant, precisely — after `response_interval`
milliseconds of silence an envelope has decayed to 1/e of its value, and
after a step up it has covered 1 − 1/e of the distance. Note what the code
does *not* have: separate attack and release. One coefficient serves both
directions, which is what the legacy surface documents (a single
`response_interval`) and is why the user chapter calls the knob "the
vocoder's attack *and* release." The 10⁻⁴ floor on `response_ms` keeps the
exponent finite; at the 20 ms default and 48 kHz, `m_env_coef` ≈ 0.99896.

## Why time-domain, when the siblings went spectral

`tap.nr~` and `tap.spectra~` (next chapter) are STFT machines. The vocoder
deliberately is not, for three compounding reasons:

- **Zero algorithmic latency.** The spectral scaffold costs exactly one FFT
  frame of delay by construction; this graph's output at sample t depends
  only on inputs up to t. A vocoder is played live against its carrier —
  latency is a musical defect here in a way it is not for noise reduction.
- **It is cheap.** 48 biquads (5 multiplies + 4 adds each in DF I) plus 24
  follower updates and 24 multiply-accumulates — on the order of three
  hundred flops per sample, no transform, no windowing, no frame buffers.
- **It is faithful.** The original `tap.vocoder~` was a real time-domain
  external; the pfft~-hosted abstraction that wrapped it in some patches
  only added smoothing and gain around it. Rebuilding it as an FFT effect
  would have been reconstructing a different object. So `vocoder.h` follows
  the `svf.h`/`ladder.h` idiom — `prepare(samplerate)` then per-sample
  `process()` — not the `configure(fftsize)` scaffold of the spectral set.

## The engineering ledger

- **`prepare()` recomputes everything.** It calls `recalc_filters()` (24
  coefficient sets, each written into both `m_mod[i]` and `m_car[i]` — the
  banks are identical by construction, one computation assigned twice) and
  `recalc_envelope()`. `set_q` re-runs only the filters, `set_response_ms`
  only the envelope coefficient, `set_gain` is a bare store — each setter
  pays for exactly what it moves, the small-scale version of `svf.h`'s
  two-tier update.
- **Setters are allocation-free and audio-safe.** All state is in fixed
  `std::array`s sized by `k_bands`; there is no allocation anywhere in the
  class, so the Min wrapper can forward attribute changes from the message
  thread while the perform loop runs.
- **The legacy surface is honored, with one documented fix.** `q` and
  `response_interval` keep their documented names, meanings, and defaults
  (20 and 20 ms). The original registered both attributes as `symbol`; the
  wrapper (`tap.vocoder_tilde.cpp`) registers them as `number`, which is
  what they actually are — a Q value and a millisecond time — and says so
  in its header. `gain` is a small, admitted addition for level staging,
  since a band-multiplied signal lands quieter than either input.
- **Both banks clear together.** `clear()` zeroes all 48 biquad states and
  the envelope array — the whole graph's memory, nothing else, so a `clear`
  message can never leave a stale envelope gating a fresh carrier.
- **What is deliberately absent:** per-band gain trims, separate
  attack/release, a noise-driven "unvoiced" band — all classic vocoder
  extensions, all outside the documented surface being reconstructed. The
  reference page promised a *basic* 24-band vocoder; the file implements
  exactly that and stops.

## Checkpoint

The vocoder is a bilinear form: two identical 24-band banks and one
multiply per band. Everything the tests pin — silence in, silence out;
exact gain linearity; follower-rate release — is a consequence of that
shape, not of tuning. The numerics are three choices: log spacing (equal
ratio per band, matched to hearing and to speech), the constant-peak RBJ
bandpass (band level measures the signal, not the Q, and stability is a
theorem with the clamps in place), and the exact one-pole coefficient
e^(−1/(τ·sr)) (the documented period is an honest time constant, symmetric
in both directions). Time-domain because latency, cost, and history all
point the same way.
