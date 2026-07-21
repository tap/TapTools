# Grains that sum to one: `grm_pitchaccum.h`

The [user-facing chapter](../pitchaccum.md) sold the effect on one image —
+7 becomes +14 becomes +21, a staircase — and one engineering promise: "the
tenth pass is as steady as the first." The image is a topology claim and the
promise is an identity about a pair of window functions, and both are
provable. This appendix proves them with the file's own names, then walks
the pitch follower's failure mode and the ledger. The kernel scenarios in
`tap.pitchaccum_tilde_test.cpp` drive `tap::tools::pitchaccum::accum_bank`
directly and pin every measured claim; the sibling `tap.shift~` tests pin
the envelope identity to nine decimal places.

## Transposition is a moving tap

A delay tap that *moves* changes pitch. Write the read position of a tap
with delay D(n) samples behind a write head at sample n:

```text
p(n) = n − D(n)
```

The output at sample n reproduces the input's phase at time p(n), so the
output advances through the input at the rate:

```text
dp/dn = 1 − dD/dn
```

A fixed tap (dD/dn = 0) plays at unity. A tap whose delay *shrinks* by
(ratio − 1) samples per sample plays the buffer at `ratio` times real speed
— up an octave means eating the delay line at one extra sample per sample.
The code implements exactly this, inverted into phasor form: the tap's delay
is `base + window_samples · ph`, and the phasor steps

```text
m_phase += −(ratio − 1.0) / window_samples
```

so dD/dn = window_samples · dph/dn = −(ratio − 1), giving dp/dn = ratio.
(The `tt_shift` provenance of that line is flagged in the code.) The
transposition scenarios pin the result at the endpoints and the middle:
+12 puts the energy of a 440 Hz sine at 880 Hz, −12 at 220 Hz, and 0 passes
440 untouched — each dominating its reference bin by the test's margins.

One moving tap cannot run forever — the phasor wraps, and at the wrap the
tap teleports across the window: a splice. Hence the classic two-tap engine:
a second tap rides the *same* phasor at `ph_b = ph_a + 0.5` (mod 1), half a
cycle apart, so one tap is always mid-window while the other is wrapping,
and each is faded by `envelope()` so the splice happens at zero gain.

## The envelope pair: an exact partition of unity

Here is `envelope(ph, flank)`, region by region (flank ∈ (0, 0.5] is the
crossfade width as a fraction of the cycle — `m_flank` maps `xfade` 1–100%
onto (0.005, 0.5]):

```text
ph ∈ [0, flank]:          sin²( π·ph / (2·flank) )      — cos²-shaped rise
ph ∈ (flank, 0.5]:        1                             — plateau
ph ∈ (0.5, 0.5+flank]:    cos²( π·(ph−0.5) / (2·flank) )— fall
ph ∈ (0.5+flank, 1):      0
```

The claim — stated as a comment in the file, and load-bearing — is that the
taps at ph and ph + 0.5 sum to 1 **exactly, at every phase and every flank
width**. Proof by the same four regions, writing e(·) for the envelope and
using ph_b = ph_a + 0.5 mod 1:

```text
ph_a ∈ [0, flank]:        ph_b ∈ [0.5, 0.5+flank]
    e_a + e_b = sin²(π·ph_a/2·flank) + cos²(π·ph_a/2·flank) = 1
ph_a ∈ (flank, 0.5]:      ph_b ∈ (0.5+flank, 1]
    e_a + e_b = 1 + 0 = 1
ph_a ∈ (0.5, 0.5+flank]:  ph_b wraps to ph_a − 0.5 ∈ (0, flank]
    e_a + e_b = cos²(π·(ph_a−0.5)/2·flank) + sin²(π·(ph_a−0.5)/2·flank) = 1
ph_a ∈ (0.5+flank, 1):    ph_b = ph_a − 0.5 ∈ (flank, 0.5]
    e_a + e_b = 0 + 1 = 1
```

Every crossfade region pairs a sin² with the cos² *of the same argument*;
everywhere else a plateau pairs with a zero. The construction also joins
each flank to its plateau with zero slope (the derivative of sin² vanishes
at both ends), so there is no corner to click, and at flank = 0.5 the
plateau vanishes and the pair degenerates to a complementary Hann pair.

Why demand *exact*? In a one-shot shifter, an envelope pair that sums to
1 ± δ is a gain ripple of δ at the grain rate — a subtle tremolo, mildly
regrettable. But this object's entire identity is that the transposer sits
**inside a feedback loop**: the ripple multiplies onto the signal on every
pass, and after k trips the peaks have compounded to (1+δ)ᵏ while the
troughs have decayed — a pumping that grows with exactly the feedback
settings the effect is played at. That is why the original engine's window
was replaced: `tt_shift` used a fixed 256-point padded-Welch table whose
pair did not sum flat (the deviation is documented in both this file's
header and `tap.shift~`'s). The identity is pinned numerically in the
`tap.shift~` wrapper tests — same envelope construction at flank = 0.5 —
where DC at 0.5 pushed through *moving* taps at ratio 1.3 comes out equal
to the input with max error below 1e−9: no grain-rate ripple, to double
precision.

## The accumulation topology

`transposer::process()` wires the loop in this order:

```text
in ──►(+)──► delay buffer ──► two moving enveloped taps ──► y (out)
      ▲                                                     │
      └── × fb ◄── DC blocker ◄── m_fb_state (previous y) ◄─┘
```

The fed-back sample re-enters *upstream of the taps*: it is written into the
buffer and then read back by the moving, windowed taps — which is to say it
is delayed by `delay_samples` **and transposed by `ratio` again**. Every
trip multiplies the frequency by another factor of ratio: +7 st becomes
+14 becomes +21. Contrast the ordinary "feedback around a shifter" patch,
where the feedback taps the delay output and re-enters the *delay*: each
echo is re-delayed but shifted only once, and the staircase never climbs.
The topology is the effect, and the kernel test pins its two-pass
signature: a 440 Hz burst at +7 st, 300 ms delay, 70% feedback shows
Goertzel energy at 659.26 Hz (one pass) in the 0.32–0.55 s window and at
987.77 Hz (two passes — the accumulation itself) in the 0.65–1.0 s window,
each dominating the off-frequency reference bin.

## Boundedness: the loop gain really is fb

The constant-sum envelope has a second payoff. Since e_a, e_b ≥ 0 and
e_a + e_b = 1, the two-tap read is a *convex combination* of past loop
samples — with linear taps its magnitude could never exceed the buffer's
peak, and the Hermite taps can overshoot that bound only by the
interpolator's small, bounded ripple. So the per-pass gain around the loop
is genuinely `fb` — capped at `k_fb_max = 0.99` — and not fb × (envelope
ripple peak), which is the number that would have mattered with an uneven
pair. The DC blocker in each loop kills the one component granular splicing
can otherwise rectify into a ramp; its own tiny high-frequency shelf
(2/(1+R) ≈ 1.0005) is absorbed a hundred times over by the 1% headroom in
the cap — this kernel does not need the fine print that `grm_comb.h`'s
0.99999 cap does. The test drives the worst case: both voices at 99%
feedback, opposing transpositions, five seconds — output finite, peak < 50.

## Modulation: one LFO, two seeded dice

Per sample, each voice's transposition is assembled as

```text
trans_eff = m_ramp[p_trans1 + v].current + lfo + rnd
ratio     = 2^(trans_eff / 12)
```

The LFO is one global phasor (`m_lfo_phase`); voice 2 reads it at
`+ modphase/360`, so the two shadows breathe against each other at a
settable phase — 90° by default. The random component is per-voice:
`tick_random()` holds a target from a linear-congruential generator
(seeded 1111 and 2222 in the constructor) and cosine-interpolates between
held values at `randrate`, re-arming its phase when disabled so re-enabling
starts a fresh segment rather than finishing a stale one. Deterministic
seeding is a testability decision that is also a musical one — the same
patch renders the same audio — and the test takes it literally: two
identically configured banks render outputs with `maxdiff == 0.0`, bit
identical. The modulation scenario pins the spectral effect from the other
side: 1 st of 5 Hz LFO drains more than half the carrier bin's energy into
sidebands.

## The pitch follower, and the subharmonic trap

`follow` adapts the grain window to the input. The follower is deliberately
cheap: input decimated by `k_flw_decim = 8` (6 kHz at 48 kHz) into a
1024-float ring, and every `k_flw_interval = 512` input samples a normalized
autocorrelation over a `k_flw_win = 512` window:

```text
corr[lag] = Σ x[n]·x[n+lag] / √( Σ x[n]² · Σ x[n+lag]² )
```

searched over lags for 50–800 Hz. The naive readout — take the global
maximum — has a classic failure mode that the code's comment names: a
periodic signal correlates at *every multiple* of its period, so corr[2T]
and corr[3T] sit at essentially the same height as corr[T], and windowing
noise routinely pushes a multiple to the numerical top. The global argmax
then reports a subharmonic — an octave or twelfth low — and the window
snaps to twice the true period. The fix is two lines: find the global max
`best_r`, then take the **smallest** lag whose correlation clears
`accept = max(k_flw_confidence, 0.85·best_r)` — the earliest lag within 15%
of the peak. Below `k_flw_confidence = 0.6` nothing is accepted and
`period_s()` reports 0: unpitched input is ignored rather than chased.

The window goal is `k_flw_periods = 2` detected periods (clamped to the
5–200 ms window range), approached through a one-pole slew
(`m_window_eff_ms += 0.0005·(goal − eff)`, ≈ 40 ms time constant at 48 kHz)
that relaxes back to the manual `window` value when follow is off or the
gate says unpitched. Why two periods: at each grain wrap the tap jumps by
exactly the window, so a window of k whole periods makes the splice
displacement an integer number of cycles — the spliced waveform stays
phase-coherent and the grain-rate artifacts land in tune with the source
instead of at an arbitrary rate (the envelope cycle rate is
|ratio − 1|·fs/window_samples, which at W = 2T scales with the source pitch,
≈ (ratio−1)·f₀/2). And k = 2 rather than 1 keeps the window above the 5 ms
floor across most of the follower's 50–800 Hz range. The test pins both the
adaptation and the trap: a 220 Hz tone converges the effective window into
6–13 ms, bracketing two periods (9.1 ms) — a subharmonic lock would demand
18.2 ms, outside the pin — and white noise leaves the window at the manual
setting (70–100 ms around the 87 ms default).

## The engineering ledger

- **17 ramped parameters, same morph engine as `grm_comb.h`.** One ramp
  array, `m_ramps_active`, `m_derived_dirty` recomputing derived values per
  sample while moving and once at settle; `store_preset` snapshots targets,
  `recall_preset` retargets every ramp from its current value, so mid-morph
  overrides are continuous with no special case. Pinned: an 80 ms recall
  under audio shows no sample-to-sample jump above 0.3 and lands every
  parameter to 1e−9; mix 0 is an exact passthrough (< 1e−9) even with 90%
  feedback churning inside the muted wet path.
- **The modulated ratio path lives in `process()`, not `update_derived()`**
  — the code comments the split: LFO and random are inherently per-sample,
  so caching them would save nothing; the cacheable tier is delays, fb,
  voice gains, flank, and the equal-power mix.
- **Hermite taps with `k_base_delay = 3` headroom** — same 4-point
  interpolator and same ≥ 2-sample geometry constraint as `grm_comb.h`,
  here so that a voice delay of 0 ms is still legal under moving taps.
- **GRM's stereo-width fader is dropped, on purpose.** The kernel is mono
  and the wrapper is single-channel by house rule (`mc.` wraps it); the
  omission is declared in the wrapper header and the maxref. Width would
  have been the only parameter that could not live in a mono kernel.
- **The follower is a mode, not a fader** — `set_follow(bool)` sits outside
  the morphable parameter set (the file says so), because interpolating a
  boolean analysis mode over a morph is meaningless.
- **Allocation discipline.** One buffer per voice sized for the worst case
  (`(k_max_delay_ms + k_max_window_ms)` at prepare-time sr, +16), a fixed
  1024-float follower ring, and a stack-local correlation array; after
  `prepare()` the audio path allocates nothing, and the analysis cost —
  a few hundred multiplies per input sample, amortized — is paid only when
  `follow` is enabled.

## Checkpoint

A tap whose delay changes at (1 − ratio) samples per sample *is* a
transposer — dp/dn = ratio, by one derivative. Two taps on the same phasor half a cycle
apart cover each other's splices, and the cos²/sin² envelope pair sums to
one exactly, region by region, at any flank width — which in a feedback
loop is the difference between loop gain fb and loop gain fb-times-ripple
compounding every pass. The feedback re-enters *upstream of the taps*, so
every echo is transposed again: the staircase is a topology, and the test
hears both steps. The follower reads the earliest strong autocorrelation
lag, not the tallest, because the tallest is routinely a subharmonic; two
detected periods keep the splices phase-coherent. Everything else — ramps,
morph, seeds, caps — is the same discipline as the comb bank, and equally
pinned.
