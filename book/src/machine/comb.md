# Ring time as the truth: `grm_comb.h`

The [user-facing chapter](../fivecomb.md) made three claims that sound like
marketing until you do the math: that a voice *keeps its decay* as its pitch
sweeps, that `warp` stretches the partials while the fundamental *stays in
tune*, and that `phase` at 100 cancels the even harmonics — exactly. This
appendix derives all three the way the file was designed, then walks the
code-level decisions. The behavioral claims below are pinned by the kernel
scenarios in `tap.5comb_tilde_test.cpp`, which drive
`tap::tools::fivecomb::comb_bank` directly (no Max in the loop); the few
numbers outside the test suite are marked as measured on the kernel for
this chapter.

## A comb is a string

One voice is a delay of d samples fed back on itself. Ignoring the in-loop
filters for a moment, the recursion the code implements
(`y = in + fb * ap_out`, written back into the delay line) is:

```text
y[n] = x[n] + fb · y[n − d]        h[n] = δ[n] + fb·δ[n−d] + fb²·δ[n−2d] + …
```

The impulse response is echoes every d samples, each scaled by another
factor of fb — and echoes every 1/f seconds *is* a tone at f and its
harmonics: a plucked string tuned to f = fs/d. The first kernel scenario
pins the geometry: a 500 Hz voice at 48 kHz (d ≈ 96) puts its first three
echoes at one period spacing, within a couple of samples of 96/192/288.

## Ring time as the truth

The legacy object exposed fb directly. This file refuses to, and the reason
is in the impulse response above. After t seconds the signal has made t·fs/d
round trips, so the decay envelope is `level(t) = fb^(t·fs/d)`.

Reverberation's standard yardstick is RT60, the time to fall 60 dB — a
factor of 10^(−60/20) = 10⁻³. Set level(rt60) = 10⁻³ and solve:

```text
fb^(rt60 · fs / d) = 10⁻³   ⇒   fb = 10^(−3·d / (rt60·fs))
```

which is character for character the file's line in `update_derived()`:
`m_fb[v] = min(pow(10.0, −3.0·d_total/(rt60·m_sr)), k_fb_max)`.

The `res` knob maps to rt60 on a log curve before this — `rt60 =
k_rt60_min · (k_rt60_max/k_rt60_min)^(res_eff/100)`, 20 ms at res → 0⁺ up to
100 s at res 100 — because equal knob travel should mean equal *ratios* of
decay time, which a linear map to fb spectacularly is not (all the action of
a raw-feedback comb lives in the last few percent of the knob).

Now the design consequence, which is the chapter title. Because fb is
re-derived from the **current** delay every time `update_derived()` runs,
the ring time is the invariant: sweep a voice's frequency and fb is silently
re-solved to hold rt60 constant. A raw-feedback comb has it backwards — hold
fb fixed and rt60 = −3d/(fs·log₁₀ fb) is *proportional to d*, so low notes
ring 1/f longer and high notes choke. The "resonance maps to ring time"
scenario pins the calibration: inverting the log curve for rt60 = 1 s gives
res ≈ 45.93, and the measured tail drops close to the ideal 30 dB over a
half-second window (the test accepts 22–40 dB; it lands near 32, the excess
being upper partials that the interpolator and loop lowpass damp slightly
faster).

## Why the delay must be fractional

At 48 kHz a 440 Hz comb needs d = 48000/440 = 109.09 samples. Round to 109
and the voice plays 48000/109 = 440.37 Hz — about +1.4 cents. Worse than the
absolute error: each of the five voices quantizes *differently*, so the
carefully-tuned beating between voices (the point of a bank) is replaced by
whatever the rounding produced. The legacy abstraction had integer delays
and control-rate stepping; the file header names that as the main reason it
never sounded like the GRM original.

So the tap is fractional — but not linear. Reading between samples with
linear interpolation is a two-tap filter H(z) = (1−η) + η·z⁻¹ where η is the
fractional part, with magnitude:

```text
|H(ω)|² = 1 − 2η(1−η)(1 − cos ω)
```

— a lowpass whose damping depends on η (worst at η = ½, where |H| = cos(ω/2),
a null at Nyquist). Two failure modes follow. Statically, this filter sits
*inside the loop*: its droop is applied once per round trip and compounds
into the ring, so two voices with different fractional parts get different
brightness decay for free. Dynamically, a sweep cycles η through 0 → 1
repeatedly, so the loop's damping *ripples* at the sweep rate — audible
dulling and level flutter. `read_hermite()` is the 4-point, 3rd-order
Hermite (Catmull-Rom) interpolator instead: C¹-continuous, passband flat to
far higher frequency, far weaker dependence on η. Its cost is the geometry
constraint noted in the code — the youngest of its four points is one ahead
of the base, so d must exceed 2 strictly, hence `k_min_delay_samples = 2.5`
and the ceiling `f_ceil = min(k_freq_ceil_hz, m_sr / k_min_delay_samples)`.

## The feedback chain, in order

Per sample, the loop path in `comb_voice::process()` is:

```text
delayed = read_hermite(d_read)  →  one-pole lowpass  →  DC blocker
        →  warp allpass  →  × fb  →  + in  →  write
```

**The lowpass** is the string's brightness decay: every round trip gets a
little darker, highs first, like a real string. Its coefficient is exact —
`m_lp_a[v] = 1 − e^(−2π·fc/m_sr)` — placing the −3 dB corner at fc by
construction (the one-step discretization of an RC section). The in-file
comment flags the deviation: `tap.comb~` used `hz·2/sr`, which is not even
the small-argument limit of the exact map (that would be 2π·fc/fs) — its
actual corner lands near fc/π, a factor-of-three tuning error on a *labeled*
frequency knob. Faithful porting stops where the parameter lies about its
units.

**The DC blocker** (`y = norm·(x − x1) + R·y1`, R = `k_dc_block_r` = 0.999,
norm = `k_dc_block_norm` = (1+R)/2, ~7 Hz corner at 48 kHz) replaces the
legacy `tap.comb~` hard ±1 autoclip — the file's most consequential
retirement. The clipper existed to stop runaway; but a clipper in a resonant
loop *is* a distortion stage, and at high resonance — precisely where the
GRM sound lives — the legacy object audibly distorted. The modern argument:
cap fb below unity (`k_fb_max` = 0.99999), kill the loop's DC transmission
(the blocker's zero at z = 1), and the linear loop contracts — no limiter
needed, so res 100 rings clean.

The `norm` factor is this chapter's own contribution, and the story is worth
a paragraph. The raw blocker `(1 − z⁻¹)/(1 − R·z⁻¹)` is not passive: its
magnitude peaks at 2/(1+R) ≈ 1.0005 toward Nyquist. While proving the
contraction claim for the first edition of this chapter, the measurement
came back *false* in one corner: with the loop lowpass wide open the product
|H_lp·H_dc| crossed unity near 450 Hz, and a voice tuned there at res 100 —
where fb saturates at the cap — measurably swelled at ~+0.2 dB per second.
The fix is the normalization: scaling the blocker by (1+R)/2 pins its peak
gain at exactly 1 (the zero at DC is untouched), so with the allpass at unit
magnitude and the one-pole lowpass ≤ 1, the loop gain is bounded by fb alone
and **fb < 1 now really is the airtight inequality**. A kernel test pins the
formerly-failing corner: 450 Hz, res 100, `lp` at 20 kHz, twelve seconds of
ring-out, decaying window over window.

The normalization has a side effect the file also pays for: the blocker now
slightly attenuates each voice's *fundamental* (a few parts in 10⁴ at mid
frequencies, more for very low voices), which — uncompensated — would shave
the top off long ring times. So `update_derived` divides the RT60-derived fb
by `dc_block_gain(ω₀)`, the blocker's magnitude at the voice's fundamental —
the same pay-the-fundamental-back philosophy as the warp compensation below,
and clamped to `k_fb_max` so the contraction bound survives. The second new
kernel scenario pins the payoff: an impulse-excited voice at res 50 measures
its RT60 within 10 % of the map's 1.41 s target.

## `warp`: dispersion, and paying the fundamental back

**The allpass** is the modern GRM Comb's character control. `warp` sets

```text
m_ap_c = −k_warp_coef_max · warp/100        (c ∈ [−0.85, 0])
```

and inserts H(z) = (c + z⁻¹)/(1 + c·z⁻¹) into the loop — unit magnitude
everywhere (it cannot alter the decay), pure phase. At c = 0 it degenerates
to z⁻¹, an honest one-sample delay: warp 0 is *exactly* the harmonic Classic
comb. Its phase delay in samples is what `allpass_phase_delay()` computes:

```text
τ(ω) = [ atan2(sin ω, c + cos ω) − atan2(c·sin ω, 1 + c·cos ω) ] / ω

τ(0) = (1 − c)/(1 + c)      (the DC limit the w < 1e−9 branch returns)
```

For negative c, τ falls monotonically with frequency — at c = −0.85, from
(1.85/0.15) ≈ 12.3 samples at DC down to 1 sample at Nyquist. A partial's
resonant frequency is set by its total round-trip time d_read + τ(ω), so
upper partials, seeing a *shorter* loop, land *sharp* of the harmonic
series — the stretched partials of a stiff piano string, exactly the
physics that motivates the control.

Left there, the fundamental would sharpen too. The compensation is one line:
`m_d_read[v] = max(d_total − ap_tau, k_min_delay_samples)`, where
`ap_tau = allpass_phase_delay(m_ap_c, 2π·f/m_sr)` is evaluated **at the
voice's fundamental**. The main tap is shortened by precisely the phase
delay the allpass adds at that frequency, so the fundamental's round trip is
d_total again — pitch stays put while the overtones stretch. The `max` is
the documented physical limit: at extreme warp × high tuning, d_total − τ
falls below the interpolator's 2.5-sample floor, the loop cannot get shorter
than the dispersion, and the pitch flattens — physical, and flagged in the
maxref. The warp scenario pins the endpoints: warp 0 echoes at one period;
warp 100 stays bounded, still resonates, and its tail correlates < 0.5 with
the harmonic tail (a genuinely different spectrum, not a filter tilt).

## `phase`: plucking the string at its midpoint

The output tap is `out = y − pickup·read_linear(d_half)` with
`d_half = d_total/2`. Consider loop content at harmonic n of the voice —
frequency n·f, i.e. n·fs/d. Delaying it by d/2 samples shifts its phase by

```text
Δφ = 2π · (n·f/fs) · (d/2) = n·π
```

Even n: Δφ is a multiple of 2π, the delayed copy equals the original, and
the subtraction (at pickup = 1) cancels it *exactly*. Odd n: Δφ = π, the
copy is inverted, and subtraction doubles it. Pickup therefore sweeps
continuously from the full series to odd-harmonics-only — plucking a string
at its midpoint, where the even modes have a node. The test uses a 1 kHz
voice at 48 kHz so the half tap lands on exactly 24 samples; Goertzel
measures the 2f-to-f power ratio collapsing below 5% of its phase-0 value.

## The parameter engine

All 22 parameters (`k_num_params`: gain, mix, three masters, warp, phase,
freq/res/lp × 5) ride identical per-sample linear ramps. The bank keeps one
count, `m_ramps_active`, and one flag, `m_derived_dirty`; `process()`
advances only live ramps and recomputes the derived values (taps, fb,
coefficients, mix gains) *per sample while anything moves, once when
everything has settled* — the same two-tier idea as `svf.h`'s coefficient
cache, so steady state pays nothing for the smoothness.

The 16-slot preset morph is the same machinery pointed at all 22 targets at
once: `store_preset()` snapshots the ramp **targets** (knob positions, not
mid-ramp values); `recall_preset(slot, seconds)` calls `ramp_to()` on every
parameter over n = seconds·m_sr samples. Because `ramp_to()` always
retargets *from `r.current`*, a recall issued mid-morph — or a single slider
grabbed mid-recall — is continuous by construction; no special case exists.
The scenarios pin it: a 50 ms recall under a running sine produces no
sample-to-sample jump above the click threshold and lands every parameter
exactly on the preset; a mid-morph `set_freq()` reaches its own value while
the other 21 keep morphing.

## The engineering ledger

- **`k_fb_max = 0.99999`, applied after the rt60 solve.** The cap makes
  "res 100 = longest possible resonance" a bounded statement; the rt60 math
  would happily request fb ≥ 1 for rt60 → ∞ (and `res_master` can push
  res_eff to 200).
- **Anti-denormal guard** (`|x| < 1e−15 → 0`, the `tap.comb~` idiom) on
  every recursive state — a comb ringing into silence otherwise decays into
  denormal territory and multiplies its CPU cost at the quietest moment.
- **Wet 1/5 normalization, a deliberate deviation.** `k_wet_norm = 0.2`
  scales the five-voice sum; the legacy abstraction wired five `tap.comb~`
  objects straight into the output gain — a hot sum, +14 dB at full wet.
  The mix scenarios pin both endpoints: mix 0 is an exact passthrough, and
  mix 100 with all resonance off passes at unity to 1e−9.
- **Equal-power mix**: `m_dry_gain = cos θ · g`, `m_wet_gain = sin θ · g ·
  k_wet_norm`, θ = mix·π/200 — matching `tap.crossfade~`.
- **The pickup tap is linear-interpolated** (`read_linear`), not Hermite —
  deliberate asymmetry: it is a feed-*forward* output tap, so its droop is
  applied once, not compounded per round trip; the argument that
  disqualified linear for `d_read` doesn't apply.
- **Allocation only in `prepare()`**: one `ceil(sr/k_freq_floor_hz)+8`
  buffer per voice (the 5 Hz floor honors the legacy 200 ms buffer). Every
  setter is a clamp plus a ramp retarget — safe while audio runs.

## Checkpoint

A feedback comb is a string, and the file's one structural opinion is that
the string's *decay time* — not its feedback coefficient — is the musical
truth: fb = 10^(−3·d/(rt60·fs)), re-solved from the current delay so pitch
sweeps preserve the ring. Hermite taps make the tuning real, the exact
one-pole makes the damping knob honest, and the DC blocker retires the
clipper by making high resonance a solved inequality (fine print stated)
instead of a distortion stage. Warp is pure phase — dispersion paid back to
the fundamental through `allpass_phase_delay` — and phase is pure geometry:
half a loop is nπ, evens cancel, odds double. The rest is 22 ramps and one
dirty flag, and the tests hold every claim.
