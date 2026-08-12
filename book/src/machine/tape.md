# Wear as the stabilizer: `tape_loop.h` and `discreet.h`

Every regenerating loop in this library before these files made the same
promise the same way: the loop is strictly contractive because feedback is
capped below one (`delay.h`'s `k_fb_max = 0.99`, the comb bank's calibrated
ring time). `tape_loop.h` and `discreet.h` exist to make the opposite
promise — regeneration at exactly 1.0, bounded anyway — and this appendix is
the derivation of why that is allowed.

## A shared header, by the house rule

The family needed the same four pieces twice (`discreet.h` and `airport.h`
are both tape machines), and the reuse rule sorted them cleanly. Classes
with state went into a shared header the way `swing_vca.h` was created for
the drum family: `tape::reel`, `tape::wow_flutter`, `tape::wear`, and a
`tape::ramp` that is a cited copy of `delay.h`'s anti-zipper unit. Few-line
expressions stayed copies-with-citation, as ever: the Hermite polynomial
inside `reel` is *the same read as delay.h*, line for line, and says so; the
saturator is not copied at all but included — `vca::swing_shape`, the shared
swing-type stage, with the reason on the include line.

## `reel`: one wrap, two topologies

A reel is position-addressed circular storage whose reads and writes wrap
modulo a *settable loop length*, not the buffer size. That one decision lets
the same class serve both kernels. `discreet.h` runs it as a delay line:
loop length equals capacity, an integer write head advances forever (wrapped
into range each sample — a bare `long` head would overflow LLP64's 32-bit
`long` in half a day of audio), and the play head trails it by the loop
span. `airport.h` runs it as a true loop: length set per piece, one
free-running head, positions handed in raw because the reel does all modular
arithmetic itself. A length change is deliberately a *splice* — content
kept, positions re-wrapped — because that is what cutting tape does.

## `wow_flutter`: periodic on purpose

The transport error is two sines — slow-deep wow, fast-shallow flutter —
returning a read-position offset in samples, phases zeroed at `prepare()`.
The periodic term is the dominant one in the tape-echo literature
(Arnardóttir, Abel, Smith, AES 2008), but the deeper reason the stochastic
term is a documented non-goal is testability: the wow promise is pinned by
predicting peak pitch deviation in closed form (`depth · 2π · rate`, so 2 ms
at 0.5 Hz ⇒ ±10.9 cents) and measuring it with the YIN oracle — 10.9
measured — and that oracle test only exists because two renders are
bit-identical. Determinism was a design force here, not an afterthought.

## `wear`: the boundedness argument

One pass of generation loss is three stages in fixed order: an exact
one-pole darkening lowpass (`1 − e^(−2πf_c/sr)`, the grm_comb.h map), the
shared saturator `swing_shape(v, d) = tanh(d·v)/d`, and the normalized DC
blocker. Each carries one clause of the proof:

- `tanh` is bounded, so for any drive `d > 0` the wear output can never
  exceed `1/d` — whatever the loop has accumulated. That is BIBO stability
  at regen 1.0, unconditionally, from the saturator alone.
- The DC blocker (pole 0.999, peak gain normalized to exactly 1 — the
  normalization grm_comb.h earned the hard way, chasing a +0.2 dB/s swell)
  kills the one frequency the lowpass would happily sustain forever with an
  offset attached.
- The lowpass is strictly contractive above its corner and asymptotically
  transparent below it — which is not a leak in the proof but the musical
  contract: at drive 0 and regen 1.0 the sub-corner band sustains
  indefinitely, cleanly. The header calls this the Frippertronics contract
  and states it rather than hiding it.

So where `delay.h` proves stability by gain, this family proves it by
*shape*: each pass survives because it is degraded. The pinned test drives
regen 1.0 for ten seconds of ring and asserts non-growth — never decay,
because decay would betray the contract just as surely as growth.

## The doppler decision

`discreet::machine` gives `loop_seconds` an ordinary ramp and does nothing
else, because nothing else is needed: moving a fractional read head *is*
tape-speed doppler. A 0.5 → 0.75 s glide over half a second reads back an
octave down mid-move (measured: 220 Hz, then re-lock within five cents) with
no discontinuity, since position is continuous even where its slope is not.
The rejected alternative — crossfading between two taps — would have hidden
the machine, and hiding the machine is the one thing this kernel is for.
The wow offset is clamped so the read can never cross the record head; at
absurd depths on short loops the transport flattens against the clamp
rather than wrapping, which the header files under honest limits.

## A finding: the arithmetic agreed

The per-pass wear transfer is fully analytic — `regen · |H_lp| · |H_dc|` on
the unit circle — so the notebook measured it the direct way: a two-tone
burst (300 Hz under the corner, 6 kHz over it) recirculated at drive 0, each
generation's tones read by Goertzel. Measured per-pass ratios: 0.292 and
0.890. Predicted: 0.292 and 0.890. Three decimals of agreement between a
rendering kernel and a formula derived independently in the test is the
cheapest kind of confidence this library knows how to buy, and both the test
(with 15% and 5% tolerance bands it never needs) and the executed notebook
carry the measurement.

## The engineering ledger

The suite leans on four instruments. Analytic transfers wherever the path
is linear (the per-pass darkening scenario asserts against the exact
formula, both tones, both directions — highs die faster *and* lows barely
fade, so the test cannot pass vacuously). Two-window RMS for long-run
claims, inherited from the comb bank's swell story: regen 1.0 rings ten
seconds and the late window may not exceed the early one. The YIN oracle
for anything with a pitch: wow depth in cents against the closed form, the
doppler glide and its re-lock. And bitwise assertions where the law is
exact: mix endpoints, the first echo returning as literally the recorded
impulse, two wow renders identical to the bit. The DC-step scenario checks
the blocker's actual job — a held offset at regen 1.0 does not accumulate
and the tail's mean returns below 0.02 — rather than a decay the contract
never promised.

## Checkpoint

One shared header, four blocks: a reel that wraps at the loop, a transport
that is two deterministic sines, a wear stage whose `tanh` bound *is* the
stability proof, and a cited copy of the house ramp. `discreet.h` composes
them into the two-machine loop where regeneration legally reaches 1.0,
loop moves are doppler because read heads are physical, and every claim is
carried twice — `discreet.ipynb` executed, `discreet_test.cpp` pinned.
