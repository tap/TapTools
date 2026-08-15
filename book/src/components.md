# The same machine, in pieces

The two chapters before this one describe instruments you switch on and
walk away from. `tap.airport~` turns seven loops; `tap.garden~` tends
itself. That is the right shape for what they do, and neither is going
anywhere.

But both were monoliths by accident rather than by design. Open
`airport.h` and there was never a loop bank doing loop-bank things — there
was an array of eight identical lanes and a summing loop. Open `garden.h`
and there was a quantizer, an event ring, a chime rack, and a seeded
gardener, wired together by a class that did nothing else. The parts were
already there. Nothing outside the monolith could reach one.

So they were promoted. The lanes and the parts are objects now, and the
block diagrams at the top of the last two chapters are patchable:

| Object | What it is | Was |
|---|---|---|
| `tap.reel~` | one free-running tape loop | a lane of `tap.airport~` |
| `tap.chime~` | the sixteen-bell wind-chime rack | the voice pool of `tap.garden~` |
| `tap.bloom` | the event ring — plant, return, fade, retire | the recirculation of `tap.garden~` |
| `tap.scale` | snap a pitch to a root and scale | the entry quantizer |
| `tap.gardener` | the idle wind, seeded | the self-seeding half |

The monoliths remain exactly what they were. This is additive: the same
kernel classes, reached two ways.

## "The patch is the object" is a measurement, not a slogan

It would be easy to say that three `tap.reel~` summed are a
`tap.airport~` and leave it there. The house rule is that claims of that
kind get measured, so this one is pinned in CI like any performance
number.

The scenario *"standalone lanes summed are the bank, bitwise"* in
`tests/airport_test.cpp` configures a three-lane bank and three standalone
lanes identically — incommensurate lengths, both exact pan endpoints and
one interior pan, one shaded darken corner and one bypassed — drives both
through the same staggered punch-in schedule for two seconds, and requires
the two stereo outputs to be equal **to the bit**, not to a tolerance.
Nudging one lane's level by 1e-12 fails it.

The garden's version, *"the bed is exactly its components wired
together, bitwise"* in `tests/garden_test.cpp`, does the same across
twenty seconds with the seeded gardener running — which puts the order of
random draws under test too, since that is the part a careless split moves
without anyone noticing.

Bitwise is available here because the objects' own promises are already
bitwise: transparent playback of a frozen loop, exact pan endpoints, a
darken stage that is genuinely bypassed at the band ceiling. A
decomposition can be held to the same standard the object is.

## What you get for patching it

For the airport, four things the monolith cannot give you:

- **An insert on one loop.** A filter, a reverse, a `tap.discreet~` for
  tape breath on one phrase and not the others. Inside the bank every
  loop gets the same treatment, which is to say none.
- **A varispeed on one reel** — the bank has one shared clock by
  construction.
- **More than eight loops.** Eight was a number, not a principle.
- **Tape you actually use.** The bank buys all eight worst-case reels at
  DSP start whether you use them or not: about 92 MB of double tape at
  the 30-second default. Three `tap.reel~` buy three, about 11 MB each.

For the garden, the interesting one is `tap.bloom`. Separated from the
chime it turns out to be the most portable idea in the family, because it
recirculates *notes* and has no opinion about what sounds them. Point it
at `makenote`, at a sampler, at MIDI out, and Eno's principle — a touch
becomes a note, the note returns a little quieter each pass until it is
gone — drives an instrument that has nothing to do with wind chimes.

Splitting also made two promises directly testable that were previously
only reachable through audio. The ring's arithmetic is now countable with
no envelope tail in the way: *"the ring's convergence theorem is exact
when nothing sounds it"* checks four different velocity/decay/floor
triples against `ceil(log(floor/velocity)/log(decay))` exactly. And the
rack's allocator can be watched directly — *"the rack fills idle bells
first, then steals the quietest"* fills the pool, strikes a seventeenth
tube, and measures that the faint tube lost its partial while a loud one
kept its own.

## Where the seams show

Three honest costs, none of them hidden.

**The garden's patch is not sample-accurate.** `tap.bloom` and
`tap.gardener` run on Max's scheduler rather than the audio clock, so a
return lands within an `@interval` tick — a millisecond by default —
instead of exactly on the sample. Inside `tap.garden~` the same ring is
sample-accurate. At loop lengths measured in seconds nobody will hear the
difference, but it is a difference, and it is why the garden's null test
lives in the kernel where both sides can share one clock, and why there is
deliberately no in-Max null test for it. Asserting a null that cannot hold
would be worse than not asserting one.

**Voice stealing had to stay in the kernel.** The obvious Max answer to a
sixteen-voice rack is one voice in a `poly~`. That answer is wrong twice:
`poly~` steals round-robin, which loses the whole point — this rack steals
the *quietest* bell and re-aims it, so its phases keep free-running and its
seat glides rather than clicking — and `poly~` does not exist off Max,
while the kernel is meant to run anywhere. So `tap.chime~` is the whole
rack, and its polyphony is its own.

**A bell reads silent until it has been processed once.** The allocator
asks each bell for its level, and a bell that has been struck but not yet
processed still reports zero. Strikes issued in the same sample therefore
land on the same voice instead of spreading across the pool. Inside the
bed this only happens when two blooms share a loop position; it is
pre-existing behaviour, and it is documented in the rack scenario rather
than fixed, because fixing it would change how the object sounds.

One thing the airport decomposition genuinely loses: `composite_period`,
the report of when the whole system realigns, needs every length at once
and so has nowhere to live in a patch of independent reels. `tap.reel~`
answers `loopsamples` with its own length in samples, which is the raw
material for that arithmetic, but the lcm itself is still the monolith's.

## Checkpoint

Five objects, no new DSP: the same kernel classes the monoliths hold, given
names and inlets. Three `tap.reel~` summed are a `tap.airport~` bitwise;
`tap.gardener` into `tap.scale` into `tap.bloom` into `tap.chime~` is a
`tap.garden~` bitwise, gardener and all. Both identities are pinned
scenarios in `tests/airport_test.cpp` and `tests/garden_test.cpp`, which
CI runs on every push, and the airport's is checked again against the real
externals loaded in Max by
`runtime-tests/patchers/tap.reel~-is-airport.maxtest.maxpat`. The
monoliths still do what they did — every scenario that pinned them before
the split passes unchanged after it.
