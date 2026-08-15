# The garden that plays itself

The first two chapters of this part recirculate sound: tape that forgets,
loops that never agree. This one recirculates *decisions*. Plant a note and
it comes back every pass of the loop a step quieter and a step purer, until
it fades below hearing and retires. Plant several and they braid. Stop
planting altogether and, after a patient interval, the garden starts
planting for itself — always on the scale, never in a hurry. You do not
play this instrument so much as tend it, which is exactly the posture Eno
kept asking for: the composer as gardener, not architect. The kernel is
named for that metaphor.

What it recreates is the *principle* behind Brian Eno and Peter Chilvers'
generative apps (Bloom, 2008), as described in their published interviews
and in Eno's 1996 "Generative Music" talk: touch becomes note, note repeats
and fades, scale makes wrong notes impossible, idleness hands the piece to
the system. The principle only — no scale tables, timings, or sounds are
taken from the app, and its name is a live trademark of Opal Limited, which
is why this object is a garden and not a bloom. (As with `tap.tune~`'s
history paragraph, none of this is legal advice; the project's ship-gate is
a freedom-to-operate review.)

Companion material: the executed notebook `garden.ipynb`, which measured
every claim below, and the `eno_render` tool's `garden_played` and
`garden_idle` scenarios, the listening copies. The Max wrapper lands in the
TapTools-Max package alongside the rest of the family.

![Signal-flow diagram of tap.garden~: notes through a scale quantizer into a 64-event ring, fired at their loop positions into a 16-voice wind-chime pool that sums onto a stereo pair, with a red per-pass path multiplying velocity by decay and brightness by soften back into the ring, and a dashed seeded gardener planting into the ring](images/garden/block-diagram.svg)

*Events on a loop instead of audio on a tape — the same recirculation, one level of abstraction up.*

## Plant and return

`note(pitch, velocity)` plants: the pitch snaps to the current root and
scale *at entry*, a small wind chime is struck on the next sample,
and the event takes a seat at the loop's current position. Every pass, it
fires again at `velocity × decay`, and below `floor` it retires. The
notebook's staircase is the whole contract in one figure: a plant at 0.8
with decay 0.5 returns with its fundamental at exactly half the last, four
times over (measured ratios 0.500, 0.500, 0.500, 0.500), then silence, and
`active_events` reads zero. The whole strike fades a shade faster than its
fundamental — quieter returns are also duller, because strike hardness
couples brightness to velocity.

![A rendered waveform showing five returns of one planted note, each half the height of the last, with the measured peak levels labeled and the retirement floor marked](images/garden/staircase.svg)

*The return staircase: decay 0.5, floor 0.05, and a bloom that knows when it is finished.*

That arithmetic is also the stability story. The family's inversion —
degradation as the stabilizer — reaches its third form here: a bloom lives
exactly `ceil(log(floor/velocity) / log(decay))` passes, so the population
of live events *converges by construction* no matter how fast you plant.
And beneath the arithmetic sits a hard bound: sixteen chimes in a fixed
pool, the quietest stolen when a seventeenth is needed, its envelopes
re-aimed rather than reset so a steal glides instead of clicking.

## `soften` — returns get purer, not just quieter

The chime is four decaying mode *doublets* at the transverse ratios of the
chosen material — by default 1 : 2.756 : 5.404 : 8.933, the free-free-tube
physics in Fletcher & Rossing — with the upper modes softer, steeper in brightness
(b, b², b³), and dying roughly as f² faster, so the fourth mode is the
few-millisecond tick of clapper contact and every strike rings down to its
fundamental. Each mode pair is split a few cents, the way a real tube's
degenerate modes are, so the tail *beats* slowly instead of decaying like a
lab sine. Each pass multiplies the event's *brightness* by `soften`, and
brightness is the upper modes' level: a bloom collapses toward its
fundamental as it recedes, losing its tick first — the tape chapters'
generation loss, restated in modes instead of passbands. The notebook
measures the mode-two-to-fundamental ratio shrinking by exactly `soften`
every single return, and the pinned tests hold each piece separately: the
tick confined to the contact, the tail's beat dipping and returning, soft
strikes duller than hard ones, high tubes ringing shorter than low.

## The rack: material, flaws, and seats

`material` swaps what the tubes are made of — a mode, not a fader. At 0 the
rack is wind chimes, the free-free tube's 1 : 2.756 : 5.404 : 8.933; at 1 it
is a tuned bar, the mallet instrument's double-octave 1 : 4 : 10 : 20 (both
tables from Fletcher & Rossing). The table is read at strike time, so every
live bloom re-voices at its next return: the notebook measures the second
partial's energy moving cleanly from 2.756× to 4× when the material flips.

And the tube is the identity. Each pitch is a physical tube whose
imperfections are properties of the tube, not the strike: its upper modes
sit a fixed few cents off the ideal ratios (bounded by ±3 cents — the
fundamental stays true, because a maker tunes the fundamental and the
overtones land where the metal puts them), and it hangs at a fixed seat on
the stereo rack, width set by `spread` (0 collapses to center mono, bitwise
identical busses). Both draws come from a stateless hash of the pitch, so
the rack is the same rack in every instance and every return of a bloom
rings from the same place with the same flaws — the notebook's seat chart
is a bar per pitch, and the seed triad below is untouched because no
generator is ever consumed for it.

## The scale contract

`root` and `scale` (chromatic, major, minor, and both pentatonics — plain
public-domain scale theory) define where plants may land, and quantization
happens at entry: the notebook plants all thirteen chromatic pitches from
60 to 72 into a C major-pentatonic garden and the YIN oracle reads every
sounded note on {C, D, E, G, A}. Wrong notes are not discouraged; they are
unrepresentable, which is most of why instruments in this family feel
effortless to strangers. Because quantization is at entry, changing the
scale re-pitches nothing already planted — the field changes for future
seeds only.

## The gardener

`idle_seconds` is the patience: that long after your last plant, the wind
picks up. The gardener strikes on a calm/gust cycle — `gust` at 0 is a
still day, single strikes spaced about one per pass; raise it and strikes
arrive in flurries of up to five neighboring tubes within a fraction of a
second, with longer calms between, the average rate holding. The
randomness is the family's
seeded xorshift64* with the full tr808 contract, pinned as a triad: same
seed, bit-identical garden; different seed, a different garden; gardener
disabled (`idle_seconds 0`), the seed cannot matter at all, because the
generator is never consumed. This is the library's first randomized event
source — `step_seq.h` proudly promises "no randomness anywhere" — and the
seed contract is what lets a generative instrument live in a test suite
that demands reproducibility.

## Recipes

- **The lobby:** defaults, `@idle 30. @level 0.4`, plant four or five
  notes, walk away. The garden holds the room indefinitely, bounded.
- **The music box:** `@decay 0.5 @soften 0.7 @idle 0 @bell 0.005 0.8 1.` —
  no gardener, fast decay: each phrase you play unwinds itself to silence
  in a few passes, a wind-up toy running down.
- **The endless install:** `@scale minorpentatonic @root 2 @idle 3.
  @gust 0.6 @seed 2008 @level 0.35`, never touch it again. Same seed next
  year, same garden — gusts and all.
- **The still day:** `@gust 0 @idle 10.` — no flurries, one unhurried
  strike at a time, the original music-box gardener.
- **The marimba loft:** `@material 1 @spread 1. @decay 0.7 @soften 0.8` —
  tuned bars instead of tubes, the rack thrown wide: drier, woodier blooms
  that each speak from their own place in the image.
- **Duet:** `@idle 6.` and stay at the keyboard — every silence longer
  than six seconds, the gardener answers you; every plant of yours resets
  its patience.

## The same machine, in pieces

The four machines inside this one — the entry quantizer, the event ring,
the chime rack, the seeded gardener — are objects too: `tap.scale`,
`tap.bloom`, `tap.chime~`, `tap.gardener`. Chained, they are this object
bitwise, gardener and all (pinned in `tests/garden_test.cpp`). The one
worth reaching for on its own is `tap.bloom`: separated from the chime it
recirculates *notes* and has no opinion about what sounds them, so the
principle will drive a sampler or MIDI out just as happily. One difference
to know before you patch it — out here the ring runs on Max's scheduler
rather than the audio clock, so returns land within a millisecond of the
grid instead of exactly on it. See
[The same machine, in pieces](components.md).

## When it is not the right tool

- **Melodies with wrong notes in them.** Quantization is always on;
  chromatic passing tones survive only in `@scale chromatic`, and
  micro-tonal pitches not at all. This is a fence, and it is the product.
- **Rhythm.** Events return on the loop grid, exactly, forever — no swing,
  no humanization. For patterns as *rhythm*, `tap.808.seq~` is the
  machine.
- **Any other timbre.** Two materials, one chime family, on purpose. It is
  an instrument, not a polysynth; for synthesis as a playground, patch
  oscillators.
- **A stereo panner.** The image is a rack of fixed seats keyed by pitch —
  there is no per-strike pan and no motion. For placement as a *parameter*,
  pan the object's output.

## Checkpoint

Notes become events; events recirculate on a loop, quieter by `decay` and
purer by `soften` each pass, retiring below `floor`; a sixteen-chime pool
bounds the sound and a sixty-four-seat ring bounds the score, oldest bloom
yielding first. Two materials share the rack, every tube keeps its own
flaws and its own stereo seat, the scale makes wrong notes unrepresentable,
and a seeded gardener keeps the piece alive exactly as long as you neglect
it. Every
number above lives twice: as an executed cell in `garden.ipynb` and as a
pinned scenario in `tests/garden_test.cpp`, which CI runs on every push.
