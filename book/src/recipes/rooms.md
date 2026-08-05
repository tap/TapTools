# A field guide to rooms

The convolution chapter makes one promise that changes how you shop:
`tap.convolve~` is *exact* — measured to 10⁻¹² against direct convolution —
so the engine contributes no character at all. Everything the effect sounds
like is the impulse response you load. That turns "how do I get a good
reverb?" into "how do I find, judge, and place a good room?" — a curation
problem, and this recipe is the field guide.

Companion material: [the convolution chapter](../convolve.md) and its
[verification notebook](https://github.com/tap/TapTools/blob/main/notebooks/convolution_reverb.ipynb);
every measured number cited below lives in one of them.

## The shopping list

An IR is a recording of a space answering a click, and the internet holds
decades of them — university acoustics archives, church-recording projects,
hardware-unit captures released by their communities. What to bring home,
by job:

- **A church or concert hall (2–5 s).** The default "make it beautiful"
  space. Long tails flatter sustained, sparse material and drown busy
  mixes — the classic trade.
- **A plate.** Not a room at all — a steel sheet's dense, fast-building
  wash. The vocal reverb of half the records you know; sits in a mix better
  than any hall because it has no early-reflection "walls" to argue with
  the stereo image.
- **A spring.** The lo-fi twang of amp reverb; gloriously wrong on drums.
- **A small real room (0.3–0.8 s).** The most useful and least glamorous
  purchase: drums and guitars recorded dry come alive with a believable
  space that reads as "a room," not "an effect."
- **Not a room.** The chapter's point stands in practice: any filter you
  can record is loadable. A guitar body IR makes a piezo pickup sound like
  wood; a vowel is a formant filter; a single click is a delay.

Prefer **4-channel captures** when offered: the engine runs the full
true-stereo matrix (LL/LR/RL/RR), and the cross-feed paths are where
"being in the room" lives — measured in the notebook at exact path gains
with zero leakage. A 2-channel IR runs as dual mono (no cross-feed); a
mono IR is the same room on both sides.

## Judging a room in sixty seconds

Load it into the `buffer~`, then:

1. **Send a click through and listen to the tail alone** (`mix` fully
   wet). You are auditioning the IR itself — the engine adds nothing. A
   good tail decays smoothly darker; a flutter or a metallic ring here
   will be on everything you send.
2. **Check the onset.** Silence before the direct sound is pre-delay baked
   into the capture — trim it in an editor or accept it, but *know* it's
   there, because it stacks with the `predelay` you set and the engine's
   own `blocksize` samples of latency.
3. **A/B at matched loudness.** `normalize 1` is on by default and is
   energy-based, so a quiet cathedral capture and a hot plate land at
   comparable levels — judge the room, not the gain staging.

## Placing the room in a patch

- **Send, don't insert.** One `tap.convolve~` fed by a send bus serves the
  whole patch, glues sources into one space, and keeps the option of
  riding the send. Keep `mix 100` (wet-only) on a send; use `mix` as an
  insert dry/wet only on a single source.
- **`predelay` before you EQ.** 10–30 ms separates the dry attack from the
  wash and buys clarity for free — the chapter's advice, and the first
  knob to reach for when a reverb "swallows" a vocal.
- **`blocksize` by role.** Live input through the reverb: 64–128 (1.3–2.7
  ms at 48 kHz — the measured cost is exactly `blocksize` samples). Mix-bus
  send: 512–2048, the CPU-cheap end, where the latency reads as a little
  extra pre-delay you set once and forget.
- **Swap rooms as a performance move.** IR swaps are atomic and click-free
  (measured RMS across the swap: 21.9 → 22.1) — load verse-room and
  chorus-hall into two `buffer~` objects and rebind with `set <buffer-name>`
  on the downbeat. (The buffer is the only way in: the object binds the
  `buffer~` named by its first argument, and re-loading a file into that
  buffer re-transforms the IR automatically.)

## When to leave the recipe

- **You want to *design* the tail** — decay and damping knobs, modulation,
  gated endings. A static IR can't; `tap.verb~` is the algorithmic sibling
  built for exactly that.
- **You want shimmer.** The wash is only half of it — the spiral half is
  `tap.pitchaccum~`, and that pairing has its own recipe in this part.
- **You want zero latency.** `blocksize` samples, full stop; at 64 that's
  small, not zero.
