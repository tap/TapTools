# Events, not audio: `garden.h`

`garden::bed` recirculates *events* where its siblings recirculate samples,
which makes it the family's odd one out mechanically and its purest member
conceptually: the wear-as-stabilizer inversion survives the abstraction jump
intact, as arithmetic. This appendix walks the machinery — the ring, the
split between planting and firing, the chime, the quantizer, the gardener —
and the two contracts that had to be designed before they could be tested.

## The event ring

Sixty-four fixed seats (`std::array`, nothing allocated at `prepare()` —
this kernel buys no tape at all), each event a pitch, a velocity, a
brightness, a position on the loop, and a plant-order sequence number. The
sequence number exists for one policy: when the garden is full, the *oldest
live* bloom yields to a new plant. The musical argument is stated in the
header — a touch must always speak (rejecting input makes an instrument
feel dead), and the oldest bloom has survived the most decay passes, so it
is the quietest thing on the table; retiring it is the least audible edit
available. The pinned scenario plants a distinctive high note, floods the
ring with sixty-four more, and requires the first note's pitch measurably
gone from the following pass.

## Fire is not plant

`note()` does *not* sound a voice. It quantizes, seats the event at the
loop's current position, and returns; the next `process()` sample finds the
event's position under the playhead and fires it. The first draft did both
— plant-and-fire in `note()` — and the loop fired it again one sample
later, a double-trigger that fell out of the design the moment firing
became the loop's exclusive job. One mechanism, two consequences: a plant
sounds one sample late (inaudible, documented), and every sounding of every
event goes through a single code path, which is what makes the return grid
a testable promise. After each fire the event blooms: velocity times
`decay`, brightness times `soften`, retire below `floor` — so a bloom lives
exactly `ceil(log(floor/velocity)/log(decay))` passes and the population
converges no matter the planting rate. That is the stability theorem, and
it is three lines of arithmetic instead of a saturator.

## The chime

Four decaying mode doublets at the transverse-vibration ratios of the
selected `material` — the free-free tube's 1 : 2.756 : 5.404 : 8.933 from
the bars-and-tubular-chimes chapter of Fletcher & Rossing's *The Physics of
Musical Instruments* (f_n grows as (2n+1)²), or the tuned bar's
double-octave 1 : 4 : 10 : 20 from the mallet-percussion chapter — each
mode a pair of sines split a fixed few cents, the doublet splitting of a
real tube's degenerate mode pairs (same source), so the tail beats slowly
instead of decaying like a lab sine. The ratio and haste tables are indexed
`[material][mode]` and read at strike time, which is the whole
implementation of the material switch: instant, allocation-free, and every
live bloom re-voices at its next return. Each mode rides
its own `tr808::decay_env`; decay times divide by ~ratio² (radiation
damping grows with frequency), which makes the fourth mode a
tens-of-milliseconds contact tick, and scale by √(440/f) per strike, so
small high tubes ring shorter than long low ones. The upper modes scale
with per-event brightness *times strike hardness* (a soft strike is a dull
strike) and progressively steeply (b, b², b³), so `soften` strips the tick
first and mode two by exactly its ratio. Mode levels sum to at most 1, so a
chime is bounded by its velocity and the pool bound stays arithmetic; modes
above 0.45·sr stay silent rather than aliasing.

The phase rule earned a refinement when the doublets arrived: a strike on a
*silent* tube zeroes its phases — fresh initial conditions, so the pair
starts aligned and its beat blooms identically at every return, which is
what keeps per-return spectral measurements deterministic — while an
audible steal keeps free-running phases and glides instead of clicking.
The inharmonicity moved the pitch contract rather than breaking it: the
upper modes clear quickly, so the YIN oracle reads each strike in its
ring-down, and the scale-contract scenario still lands every off-scale
plant on the scale within 20 cents.

## The tube is the identity

Two more properties hang off each pitch, and neither touches the rng. A
tube's upper modes sit up to ±3 cents off the ideal ratios — the
fundamental stays true, because a maker tunes the fundamental — and the
tube keeps a fixed seat on the stereo rack, `spread` scaling how far off
center. Both are drawn by `tube_unit`, a stateless xorshift64* hash keyed
by (fundamental-in-centihertz, index) — the `metal_bank.h` per-index idiom,
index 0 the seat, 1..3 the mode scatter. Stateless is the load-bearing
word: the gardener's seeded generator is never consumed, so the seed-triad
contract survives intact, the rack is identical in every instance, and
every return of a bloom rings from the same place with the same flaws.
The pinned scenarios measure the scatter by scanning a Goertzel probe
across the second mode (±0.25-cent steps resolve it), and the seat by
left/right energy share: deterministic per pitch, different across pitches,
bounded by the constants. The seat itself follows the phase rule — pan
gains snap on a silent tube and slew ~10 ms through an audible steal — and
the equal-power law is the √((1∓p)/2) form, so `spread 0` makes the busses
*bitwise* identical (also pinned).

## Quantize at entry

The scale machinery is `tune.h`'s 12-bit pitch-class mask idiom — the
`make_mask` builder, the nearest-allowed search that never travels more
than a tritone — *copied with citation, not included*, because `tune.h`
reaches into `tap::dsp` for its detector and a garden should not link a
pitch tracker to hold five scale presets. The masks themselves are plain
public-domain scale theory, deliberately not any app's preset list.
Quantizing at entry (rather than at fire) is the semantic choice: a scale
change re-pitches nothing already planted, which keeps running gardens
stable under live tinkering and makes the contract easy to state.

## The gardener and the seed

The gardener is a wind model: once the idle threshold passes, strikes
arrive on a calm/gust cycle driven by a small state machine — a gust
catches 1 to 5 neighboring tubes (`gust` sizes it) with 30–280 ms between
strikes, the clapper walking a few semitones per swing, and the following
calm stretches with the gust just spent so the average rate stays near one
strike per pass at any setting. Idle planting consumes the family RNG
(`tr808::white_noise`, xorshift64*, the seed-folding and clear-reseeds
contract) — and *only* idle planting does. That consumption discipline is
load-bearing: the third leg of the seeded triad, "with the gardener
disabled the seed cannot matter at all", is only true because a disabled
gardener never touches the generator. The suite pins all three legs, plus
the wind itself: at gust 1 some strikes tumble inside a gust, at gust 0
single strikes never come closer than the minimum calm (the scenario sets
`decay 0` so only the gardener's own strikes are counted — planted seeds
recirculate, and returns are not wind). `step_seq.h` promises "no
randomness anywhere"; this kernel is the deliberate counterpoint, and the
triad is the bridge back to a reproducible test suite.

## A finding: envelopes never reach zero

The return-grid scenario was first written the obvious way — the percussive
test bell surely dies between returns, so the first nonzero sample after
silence is the onset. It failed, instructively: `decay_env`'s exponential
tail crosses the 1e−12 hard-zero more than half a second after a "20 ms"
decay, so there *is* no silence between returns, only −200 dB of not-quite.
The fix was to stop pretending: an instant-attack bell, an amplitude
threshold scaled to the expected return velocity, and a grid claim of
"within 8 samples" — a sixth of a millisecond — with the comment explaining
that a threshold on a sine sits a few samples into the cycle. The lesson is
general for this library: exponential envelopes make "silence" a tolerance,
and tests that assume literal zeros between notes are wrong even when they
pass.

## The engineering ledger

The suite measures the output, never the internals: fundamental ratios
for the decay staircase (0.5 ± 0.05 across four returns — the fundamental,
because hardness makes whole-strike peaks fade faster than velocity, then
`active_events() == 0` and the render below 1e−6), a strictly-decreasing
Goertzel sideband for softening, YIN for the scale contract, the seeded
triad rendered three times over, and structural bounds exercised at their
edges — sixty-five plants against sixty-four seats, thirty-two notes
against sixteen bells, finiteness and the `k_voices` amplitude bound under
sustained stealing. The two introspection counts (`active_events`,
`active_voices`) exist, as `phase()` does next door, so those scenarios
could be written against public surface.

## Checkpoint

A fixed ring of events fired by a loop counter into a fixed pool of modal
wind chimes: plant and fire kept strictly apart, wear as per-pass arithmetic
(decay, soften, floor) with convergence as its theorem, scale masks copied
from `tune.h` and applied at entry, tube identity (material voicing, mode
scatter, stereo seat) as stateless hashes so nothing generative leaks into
the audio path, and a gardener whose RNG discipline makes generative
behavior compatible with a bit-exact test suite. Third
costume, same inversion: the system stays bounded because everything in it
is always fading. Every claim lives twice — `garden.ipynb` executed,
`garden_test.cpp` pinned.
