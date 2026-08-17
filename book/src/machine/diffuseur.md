# Driven, not struck: `diffuseur.h`

This file exists because a plan was wrong in a useful way. The Ondes family
plan said the diffuseurs would inherit `garden.h`'s modal machinery, and
they do — mode ratios, doublet splitting, per-mode decay. What it did not
say, and what reshaped the file, is that a diffuseur is **driven**. There is
no trigger here and no `decay_env`. The input excites the body continuously
and the body rings at its own rates, which is `grm_comb.h`'s situation
rather than the chime's.

Five classes: `mode`, `plate`, `sympathetic`, `harp`, `transducer`, and two
cabinets over a shared `cabinet` base. Nothing else.

## Unit peak gain, and everything it saves

`mode` is the constant-peak-gain two-pole resonator (Steiglitz; Smith,
*Introduction to Digital Filters*): poles at radius R, zeros at ±1, and
`b0 = (1 − R²)/2`.

That choice pays three times, and it is worth spelling out because it is the
reason this file has almost no defensive code in it.

- **Peak gain is 1 at any Q.** So a bank of weighted modes is bounded by the
  sum of its weights. The plate's eight weights sum to exactly 1, which
  means the body cannot output more than its input, and there is no limiter
  anywhere in the file.
- **Changing `decay` does not change the level.** With a plain two-pole
  resonator, moving R moves the peak gain, so a decay knob is also a volume
  knob. Here it is not.
- **The zeros at ±1 are exact nulls at DC and Nyquist.** So there is no DC
  blocker on the body either. It cannot accumulate one.

None of that is novel — it is a textbook resonator used for the reason the
textbook gives — but the cumulative effect on a file that runs sixteen of
them plus twelve delay loops is large.

## The order is the argument, and it is a bitwise test

The instrument's signal reaches the transducer first, and the transducer's
motion excites the body. So the nonlinearity is **upstream** of the
resonator.

That is the central design claim of the file, so it is pinned rather than
described: a scenario builds `transducer → plate` by hand and checks that a
whole `metallique` is **bitwise identical** to it, and that the reverse
wiring — resonate, then distort — differs by 28 % of peak.

Getting that null to be actually bitwise took one fix. `cabinet::blend` is
an equal-power crossfade written with `cos`/`sin`, and `cos(π/2)` in double
precision is 6.1e-17, not 0. A wiring test that reads 6.1e-17 has not
demonstrated identity; it has demonstrated *approximate* identity, which is
the thing the test exists to distinguish from. Both ends of the blend are
now exact short-circuits: mix 0 returns the dry input bit for bit, mix 100
returns the wet.

## The transducer's bound is 2/saturation, not 1/saturation

The moving-iron model squares the drive, and `vca::swing_shape` bounds the
result at `1/saturation`. The obvious test — output stays under
`1/saturation` — failed at 1.49 against a bound of 1.25.

The test was wrong, not the code. A hard-driven squared law produces a
nearly-constant *positive* waveform: it sits up near the ceiling and dips
toward zero. Removing its DC recentres that, so the excursion below the mean
adds to the excursion above it, and the worst-case swing after the DC
blocker is up to **twice** the saturator's own bound.

The corrected bound is `2/saturation`, documented in the header, and the
scenario now asserts both sides of it — greater than `1/sat`, less than
`2/sat` — so the test still catches the saturator disappearing entirely.

The general shape of this mistake is common enough to name: **a DC blocker
after an asymmetric nonlinearity is not free.** It does not just remove an
offset; it converts an offset into headroom you have to have.

## A measurement that measured its own edges

The palme's selectivity scenario drives the board with a tone and measures
what is still ringing after the tone stops. First version: switch the tone
on, switch it off, measure the tail. It failed — 3.66× selectivity against
the 4× asserted — and the failure was real but not about the strings.

Switching a tone on and off is a step, and a step is broadband. It excites
every string on the board, so the tail contained twelve strings ringing
regardless of what frequency had been played. The measurement was reading
its own edges.

Fading the drive in and out over 250 ms removes the step. Every one of the
twelve strings then passes, with the worst at 4.4×. The same fade is what
the book figure uses, and the figure's caption says so, because a reader
reproducing it without the fade will get the wrong answer.

## Twelve strings

Widely copied hobbyist build pages describe the palme as two banks of twelve
strings. The peer-reviewed source (Wijnand, Boutin, Jossic & Maniguet, Forum
Acusticum 2023) says twelve, and `k_strings = 12` with a comment saying
which source won and why.

Their tuning is not published anywhere found, so it is a parameter rather
than a constant, and the header says that too. Guessing a tuning and
hard-coding it would have been the same category of error as the
twenty-four.

## Where recreation begins

The instruments, their dates, their excitation and their transducer type are
peer-reviewed. The modal data is not — no ondes-specific measurement of
either body exists in any of the four sources read — so the plate uses
Fletcher & Rossing's free circular plate (Rayleigh's Chladni ratios at
Poisson 0.3) and the strings use the harmonic series.

This is stated in the header at the top rather than in a limits section at
the bottom, because it changes what the object *is*: a recreation of the
general physics, not a model of Martenot's instruments. The same applies to
`asymmetry` and `saturation` — the source establishes that the moving-iron
driver is nonlinear and that Thiele–Small does not describe it, and then
does not hand over a curve. Those two coefficients are voiced by ear and
labelled as voiced by ear.

A diffuseur with both at 0 is a linear resonator and is missing a real
stage. That is a choice the caller may make, and the header says so rather
than forcing a minimum.

## Checkpoint

A textbook resonator chosen for three properties that between them remove
the limiter, the DC blocker and the decay/level coupling. A bitwise null
that pins the transducer upstream of the body, which required making an
equal-power blend exact at its endpoints. A saturator bound corrected from
`1/sat` to `2/sat` because a DC blocker after an asymmetric nonlinearity
buys headroom, not just centring. A selectivity test that had to stop
measuring its own on/off step. And a provenance line drawn where the
published sources actually stop.
