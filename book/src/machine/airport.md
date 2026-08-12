# Free-running heads, one shared clock: `airport.h`

`airport::loop_bank` is structurally the smallest kernel in the family — a
fixed array of loops, a stereo sum, no feedback anywhere — and that is what
makes it interesting to read: nearly every promise it makes is *structural*,
so nearly every test on it is bitwise. This appendix walks the file in code
order and dwells on the one discipline that defines it.

## `loop_state`: the multitap idiom with a reel in each seat

The bank is `std::array<loop_state, k_max_loops>` with an active count —
`delay.h`'s multitap shape, kept deliberately: per-index setters that
silently no-op on a bad index, getters that return safe defaults, newly
activated slots arriving at their stored settings. Each seat holds a
`tape::reel` (its own worst-case buy — eight 30-second reels is ~92 MB of
double tape, the family's largest allocation, stated in the header rather
than discovered in production), a `tape::wear` used as a playback shade, a
phase, a record flag, and three ramps (level, pan, darken).

## The phase discipline

The load-bearing sentence in the header is "the phase is NEVER reset":
recording starts wherever the head is, `set_loops` activates a loop with
its head wherever it last was, a splice re-wraps the head modulo the new
length without rewinding, and only `prepare()`/`clear()` — DSP restarts —
may rewind. The reason is musical: in "2/1" the free-run *is* the piece,
and any convenience reset (snap to zero on record, realign on length
change) would quietly delete the composition. The pinned scenario earns the
promise the blunt way: it fires a setter storm mid-render — level, darken,
record, length, count — and then requires the click grid unmoved and the
head advanced by exactly the samples processed. `phase()` exists as
introspection precisely so that test could be written.

## Record semantics

`record` is a gate, not an action: while on, the input *replaces* the tape
at the integer head position, after the read — so you hear the previous
generation under the head while punching, and one Hermite support point
(two samples) of the old generation blends across the punch, which the
header files under honest limits instead of papering over with a crossfade.
No overdub-sum, because the provenance had none: each Airports phrase was
recorded once. Freeze is the strong promise — record off, and two
successive passes of the loop are required bit-identical. That promise is
only possible because of the next decision.

## The shade and its bypass

Per-loop `darken` reuses `tape::wear` with drive pinned at 0, as a *static
playback tone* — deliberately not generation loss, because a frozen loop
replays the same magnetic imprint every revolution and modeling wear on it
would be dishonest physics. At the band ceiling (the default) the stage is
bypassed entirely: not "flat enough", but not-in-the-signal-path, which is
what upgrades the freeze test and the hard-pan test (a pan of −1 adds the
loop's samples to the left bus unscaled) from tolerance checks to bitwise
facts. Engaged, the shade is the exact one-pole from grm_comb.h, and the
notebook measures a 6 kHz phrase through a 1 kHz shade at 0.169 of its
transparent twin against 0.169 predicted.

## `composite_period_seconds`

The lcm of the active loop lengths in samples, folded pairwise with a
`long long` gcd, overflow detected before each multiply and reported as
+inf. It is introspection, not DSP — but it is the piece's thesis as a
number: 24000- and 30000-sample loops report exactly 2.5 s (and the pinned
scenario also proves the rendered output repeats at 120000 samples and
does *not* repeat at 60000), while seven airport-scale lengths overflow to
infinity, which the header calls the point.

## A finding: the raster before the assertion

The lcm scenario existed as an assertion first — bitwise equality of two
2.5-second windows — and it passed, which is exactly why it was worth
plotting. The notebook's event raster (every return of loop A, loop B, and
their sum on one timeline) made the same fact *visible*: the coincidence
pattern audibly and graphically re-enters at 2.5 s and drifts everywhere
short of it. The assertion pins the promise; the raster is what convinces a
human the promise means something. The pair — one bitwise test, one
executed figure — is this library's preferred way to hold a structural
claim from both sides.

## The engineering ledger

Almost everything here is exact, so the suite asserts exactly: bit-equality
for freeze and for the lcm window, bitwise silence on the far bus for hard
pans, `phase()` continuity to 1e−9 through the setter storm, and the splice
law (0.9 of a 1 s loop re-wraps to 0.8 of a 0.5 s loop, never zero). The
one measured tolerance in the file is the shade's analytic transfer at 20%,
and the equal-power pan law needs no scenario of its own because the
multitap chapter already pinned the center at 1/√2 to 1e−12 — same code
shape, same law, cited rather than re-proven. Long-run behavior needs no
stability test at all: there is no feedback path to go wrong, which is
itself a fact the file's structure makes obvious enough not to test.

## Checkpoint

A fixed bank of reels, one sacred free-running head each; record replaces
and freeze is bitwise; splices re-wrap, never rewind; the shade bypasses to
bit-transparency at the ceiling; and the composite period is the score's
arithmetic made introspectable. The promises are structural, the tests are
bitwise, and the executed raster in `airport.ipynb` is the human-readable
proof that the structure composes.
