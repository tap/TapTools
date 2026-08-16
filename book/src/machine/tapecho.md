# Composition, not construction: `tapecho.h`

This is the shortest appendix in the book, and that is the point of it.

`tape_loop.h` was written for the Eno family — one shared header holding a
reel, a transport, and a wear stage, factored out because `discreet.h` and
`airport.h` needed the same four pieces twice. The claim implicit in
factoring it that way was that it is a *library*: machinery that a machine
nobody had written yet could be built out of. `tapecho.h` is the test of
that claim, and the result is worth recording precisely, because "we
extracted a shared header" is easy to say and rarely checked.

The result: **`tape_loop.h` needed no changes at all.** Not a new method,
not a widened clamp, not a friend declaration. A tape echo — a different
topology, a different number of read points, a different stability regime —
composed out of it exactly as shipped.

## What the file actually contains

Two classes and no DSP that was not already in the library.

`head` is a read position with three ramps: a `ratio` along the tape path, a
`level`, and a `pan`. Its `read()` takes a reel it does not own, the motor
span, and the shared transport offset, and accumulates a panned contribution
onto the stereo busses. It is a component in the airport.h sense — a piece
the monolith is made of, reachable for testing — but honestly labeled as
*not* standalone-external material: a head without a reel is not a machine,
it is an index. That distinction is worth keeping straight, because the
components chapter's lesson ("the monoliths were monoliths by accident") can
be over-applied. Some seams are real and some are arithmetic.

`machine` owns one reel in delay-line topology, one `tape::wow_flutter`, one
`tape::wear`, and four heads. Its `process()` reads the heads, applies the
regeneration cap, writes the record head, and mixes. There is nothing else
in it.

## The geometry: one motor

Each head's delay is `span_samples * ratio - offset`, where `offset` is the
transport error. Two decisions hide in that one line.

The first is that `span` is defined as the delay of a *ratio-1.0* head
rather than as "the delay time", which is what makes the motor a motor: one
multiply per head and the whole layout scales together, as a tape speed
does. The alternative — per-head absolute times — would have made a speed
change into four coordinated parameter moves and lost the doppler for free.

The second is that `offset` is subtracted once, shared by every head. That
is physically right for a single transport (one capstan error displaces the
whole tape path) and it is also the cheap answer, so it is worth saying
plainly that the per-head phase differences of a real multi-head transport
are *not* modeled. It is a documented limit, not an accident.

## The stability inversion, one step further

`machine/tape.md` derived why `discreet.h` may run regeneration at exactly
1.0: `wear`'s saturator is bounded by 1/drive, so the loop is bounded no
matter the gain. That derivation does not stop at 1.0 — nothing in it does.
So this kernel lets regeneration reach `k_regen_max_driven` (1.5), and the
tape is bounded by `|in|max + regen/drive` at any setting.

The subtlety is the boundary. That guarantee exists *only while the
saturator is engaged*, and `drive` is a ramped parameter a performer can
take to zero mid-howl. At drive 0 the wear path is exactly linear with
|H| ≤ 1, so regeneration above 1.0 would grow without bound. The kernel
therefore computes the cap **per sample** from the current drive:

```cpp
const double regen_eff = std::min(regen, (drive > 0.0) ? k_regen_max_driven
                                                       : k_regen_max_linear);
```

Not in the setter — in the audio path, because `drive` moves during
performance and a setter-time decision would be stale the moment it
mattered. The stored target keeps its high value, so pulling drive to zero
lands the loop at 1.0 and restoring drive brings the howl back. That
asymmetry between *target* and *effective* is the one piece of state in this
kernel that is not obvious from the header's public surface, which is why it
is written down twice: here, and in the file's own banner.

## Why the null test is the important one

The suite's load-bearing scenario neutralizes the tape — no transport error,
no regeneration — and asserts that a one-head echo is **bitwise**
`delay.h`'s Hermite multitap.

It is bitwise rather than approximate because nothing was reimplemented:
both paths compute `time_ms * 0.001 * sr` the same way, both clamp at the
same 2.5-sample Hermite floor, both evaluate the same polynomial at the same
fractional position, and both apply `(pan + 1) * 0.25 * π` to the same
`k_pi`. The multiply by a ratio of exactly 1.0 and the subtraction of an
offset of exactly 0.0 are both exact in IEEE-754, so the arithmetic does not
merely agree — it is the same arithmetic.

That is what makes the test meaningful. An approximate null test would pass
just as happily over a second implementation that happened to be close. A
bitwise one only passes if the shared code is genuinely shared, which is the
proposition on trial. The notebook runs the same comparison across the C ABI
so the claim also holds at the boundary the externals cross.

One consequence worth knowing when reading the test: at pan 0 the two busses
are *not* bit-identical to each other, because `cos(π/4)` and `sin(π/4)`
differ by one ulp in IEEE-754 doubles. That is inherited from `delay.h`'s
pan law, it is the same in both objects, and it is exactly why the null test
compares each bus against its counterpart rather than comparing left to
right.

## Checkpoint

Two classes, no new DSP, and a shared header that did not move. The motor
geometry buys varispeed with one multiply per head; the regeneration cap
lives in the audio path because the thing it depends on is performed; and
the null test is bitwise because being bitwise is the only version of that
test that proves anything.
