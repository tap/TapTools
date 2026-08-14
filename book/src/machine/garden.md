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

Three decaying sine modes at the transverse-vibration ratios of a free-free
bar — 1 : 2.756 : 5.404, from the bars-and-tubular-chimes chapter of
Fletcher & Rossing's *The Physics of Musical Instruments* (f_n grows as
(2n+1)²) — each mode with its own `tr808::decay_env`, the upper two scaled
by per-event brightness and dying faster (`k_mode_haste`), as struck tubes
do. Mode levels sum to at most 1, so a chime is bounded by its velocity and
the pool bound stays arithmetic. The inharmonicity moved the pitch
contract rather than breaking it: the upper modes clear quickly, so the
YIN oracle reads each strike in its ring-down — the scale-contract
scenario measures the tail, where the fundamental is all that remains, and
still lands every off-scale plant on the scale within 20 cents. Softening
maps to the upper-mode levels, so "purer every pass" is measurable as a
Goertzel trajectory: mode two (2.756f) fades return over return by almost
exactly `soften` while the fundamental holds. Modes that would land above
0.45·sr stay silent rather than aliasing. Steals re-aim: the pool's
quietest chime gets `trigger()`ed with new targets while its envelopes and
phases free-run, so a steal glides where a reset would click; the
`decay_env` was built for exactly this non-resetting retrigger, one family
over.

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

Idle planting consumes the family RNG (`tr808::white_noise`, xorshift64*,
the seed-folding and clear-reseeds contract) — and *only* idle planting
does. That consumption discipline is load-bearing: the third leg of the
seeded triad, "with the gardener disabled the seed cannot matter at all",
is only true because a disabled gardener never touches the generator, so
two beds with different seeds run bit-identical until the first idle draw.
The suite pins all three legs, the way the tr808 voices taught: same seed
bit-exact, different seed audibly different, seed irrelevant when the
random feature is off. `step_seq.h` promises "no randomness anywhere"; this
kernel is the deliberate counterpoint, and the triad is the bridge back to
a reproducible test suite.

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

The suite measures the output, never the internals: peak-per-window ratios
for the decay staircase (0.5 ± 0.075 across four returns, then
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
from `tune.h` and applied at entry, and a gardener whose RNG discipline
makes generative behavior compatible with a bit-exact test suite. Third
costume, same inversion: the system stays bounded because everything in it
is always fading. Every claim lives twice — `garden.ipynb` executed,
`garden_test.cpp` pinned.
