# Dice you can replay: `stammer.h`

Most kernels in this library are hard to get wrong quietly: a filter with a
bad coefficient sounds bad. A stutter is not like that. It has three
interacting integer clocks — a grid countdown, a slice origin, and a
playback head — and if any one of them is off by a sample the object still
sounds *fine*. It stutters. It grooves. It is just wrong in a way no amount
of listening will surface.

So the interesting content of this appendix is not the DSP, which is a
buffer and some dice. It is how you pin three clocks at once.

## The pinned-dice identity

Every random draw in the kernel has a setting at which its outcome is
forced. Fire probability 1 always fires. `divisions` 1 always picks the
whole step. `repeats` 1 always plays one pass. `reverse` 0 never reverses.
`jump` 0 never reaches back. `fade` 0 leaves the material alone.

Set all six and the machine becomes deterministic *regardless of the seed* —
and what it must then be is not a vague "sensible output" but a specific,
checkable thing: **exactly a one-step delay**. At each grid point it grabs
precisely the step that just went past and plays it once, so

```
y[i] == x[i - step + 1]
```

for every sample after the first grid point, bitwise.

That single assertion is worth more than three separate off-by-one tests,
because it fails if the grid countdown fires a sample early, if the origin
arithmetic reaches one sample too far back, if the playback head starts at
the wrong index, *or* if any two of those are wrong in ways that would
cancel in a looser test. It is also cheap to reason about, which matters:
a test you cannot re-derive on a whiteboard is a test you will eventually
delete instead of fixing.

The `+ 1` in that expression is not a fudge. The write head holds the *next*
write position, so after recording sample `i` the newest available sample
sits at position `i`, and a slice of length `step` grabbed at grid point
`k·step` reads positions `k·step + 1 - step` upward. Getting that constant
right by derivation rather than by nudging until the test passed is the
whole discipline; a test you tune to the implementation pins nothing.

Two smaller identities sit alongside it. With `reverse` 1 the same grab
reads end-first, so `y[k·step + j] == x[k·step - j]` — the mirror of the
first, which catches a reversed-index off-by-one that the forward test
cannot see. And with `repeats` above 1, every output sample must be *either*
a fresh grab or a bit-exact copy of the block one slice-length earlier;
nothing else is legal, because a slice in flight is never interrupted. That
invariant covers the repeat machinery without needing to know how many
passes the dice chose.

## The draw order is part of the ABI

`maybe_fire()` draws in a fixed order: fire, division, repeat count,
reach-back, then the first reverse coin. Each subsequent repeat draws its
own reverse coin as it starts.

That order is not an implementation detail — it is what "a seed is a
performance" means. Reordering two draws, or adding a draw in the middle,
silently changes every render anyone has ever made with a given seed.
`garden.h` established the same discipline for the gardener; this file
inherits it, and the comment above the function says so in as many words so
the next person to add a parameter knows to append rather than insert.

The disabled case is the sharp end of it. At `density` 0 the function
returns *before* drawing anything:

```cpp
if (m_density <= 0.0) {
    return; // the dice are never rolled, so the seed provably cannot matter
}
```

The lazier version — draw, then compare against 0 and fail — behaves
identically to the ear, and would be indistinguishable in almost any test.
It would also consume one number per grid point, so the seed *would* matter:
switch density off and on again, and the stream is somewhere else. The
garden made this a family contract, and it is pinned here by a test that
runs two different seeds at density 0 and requires bit-identical output.

## Reading from the ring, and what it costs

A slice does not copy its material. It stores an origin and reads from the
capture ring as it plays.

The alternative — memcpy the slice into a private buffer at fire time —
would be more obviously correct, and it is what a first draft wants to do.
It was rejected because it is a burst copy in the audio thread: half a
second of slice is 24,000 doubles moved inside one `process()` call, a spike
that does nothing for 47,999 other samples. Reading from the ring costs
nothing extra.

The price is a real failure mode, so the header states it: if a repeat train
outlives the buffered history — `repeats · length + jump` beyond
`max_history_ms` — its tail reads fresher material as the write head laps
the origin. The kernel clamps the slice length against the bought capacity
so it can never read *outside* the buffer, but it does not and cannot
prevent a long train from being overtaken. Sizing the history is the
caller's job, and the object argument exists for exactly that.

## The envelope, and why the dip stays

Flanks are raised sine, computed to be exactly 0 at both edges and exactly 1
across the plateau, clamped per slice to half the slice so the two flanks
never overlap. Repeats are sequential, not overlapped, so each junction dips
to zero rather than crossfading.

Leaving it that way was a decision. An equal-power crossfade between
consecutive passes is easy from here — the pieces are already in the family —
and it would smooth exactly the articulation that makes a stutter read as
rhythm. The dip is the transient the ear locks onto. So the file documents
it as intentional, and the test asserts the exact edges, so nobody later
"fixes" the dip and quietly turns the object into a tremolo.

## Reuse, and the component that is not a component

The capture is a `tape::reel` in delay-line topology — the same class the
tape echo uses, the same class `airport.h` runs as a true loop. Reads here
are always at integer positions, so the family's Hermite read reduces to an
exact sample fetch; that is slightly more arithmetic than an integer index
would need, and it is kept anyway because it is one code path and because a
rate-varying sibling (`tap.scrub~`, planned) needs precisely this.

The randomness is `tr808::white_noise`, the family's seeded xorshift64*,
reached through `swing_vca.h`. Nothing new was written for it.

Which leaves the split: `capture` and `slicer` are separate classes under a
thin `machine`, per the family's components-first habit. But a `slicer`
needs a `capture` to mean anything, so — like `tapecho.h`'s `head`, and
unlike `airport.h`'s `loop` — it is documented as a component for
composition and testing rather than a candidate for its own external. The
components chapter's lesson is that seams often already exist and only the
monolith can reach them. The corollary, which is easier to forget, is that
not every class boundary is a seam.

## Checkpoint

Three clocks, pinned by one identity: force every die and the machine must
be exactly a one-step delay, bitwise. A fixed draw order, because that is
what makes a seed a contract, and an early return at density 0 so a disabled
generator provably cannot consume its stream. Ring reads instead of a burst
copy, with the failure mode written down rather than papered over. And an
envelope dip that is deliberate, tested, and therefore safe from being
helpfully removed.
