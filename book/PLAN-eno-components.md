# Plan — *The same machine, in pieces* (`src/components.md`)

Drafting record for the chapter covering the decomposition of `tap.airport~` and
`tap.garden~` into patchable components. Kept after shipping, per the house convention.

## Where it came from

Not a roadmap item. The question was asked directly: the airport and garden objects are
nice as monolithic blocks, but would breaking them into components patched together in Max
both reveal the inner structure and allow customization?

The answer that made it worth doing was not "yes, that would be nice" but a reading of the
code: **the components already existed as classes and only the monolith could reach them.**
`tape_loop.h` was already a component library; `airport.h` held an array of `loop_state`
and a summing loop; `garden.h`'s `bell` was already standalone and `bed` was four machines
wired together. And the package already ships this way everywhere else — there is no
`tap.808~`, there are eight voices and a `tap.808.seq~`. The Eno objects were the
exception, not the norm.

That reframing is the chapter's spine: this is not a redesign, it is promoting seams the
code already had.

## What shipped

Kernel: `airport::loop` extracted from `loop_bank`; `garden::rack` / `garden::ring` /
`garden::gardener` / `garden::scale_quantizer` extracted from `garden::bed`. Both
monoliths become composition and nothing else. C ABI + ctypes bindings for each component.
Max: `tap.reel~`, `tap.chime~`, `tap.bloom`, `tap.scale`, `tap.gardener`, full vertical
slice each.

## Evidence the chapter is allowed to cite

Everything in the chapter traces to a pinned scenario. No notebook cells were added — the
kernels' behaviour did not change, so the executed notebooks still stand as they are, and
the new claims are structural rather than measured-from-audio.

- `tests/airport_test.cpp` — "standalone lanes summed are the bank, bitwise" (3 lanes,
  2 s, staggered punch schedule, both pan endpoints, shaded and bypassed darken). Mutation
  check during development: a 1e-12 level nudge on one lane fails it, so it is not vacuous.
- `tests/airport_test.cpp` — "a lone lane's head is as sacred as one in the bank";
  "unprepared, a lone lane is silent and leaves the busses alone".
- `tests/garden_test.cpp` — "the bed is exactly its components wired together, bitwise"
  (20 s, gardener running, so rng consumption order is under test).
- `tests/garden_test.cpp` — "the ring's convergence theorem is exact when nothing sounds
  it" (4 triples against `ceil(log(f/v)/log(d))`).
- `tests/garden_test.cpp` — "the rack fills idle bells first, then steals the quietest";
  "the gardener touches its rng only while idling".
- `TapTools-Max/runtime-tests/patchers/tap.reel~-is-airport.maxtest.maxpat` — the same
  airport identity against the real externals in Max (on-Mac gate, not CI).

Behaviour-preservation of the extractions themselves was verified during development with
throwaway fingerprint harnesses (FNV-1a over every output sample of multi-second renders
through splices, punch-ins, mode changes, and the seeded gardener; identical before and
after). Those are **not** committed and the chapter does not cite them — the committed
evidence is that every pre-existing scenario passes unchanged, plus the null tests.

## Structure

1. The monoliths were monoliths by accident — the parts were already there. Table of the
   five objects and what each one was.
2. "The patch is the object" as a measurement, not a slogan — the two bitwise null tests.
   Why bitwise is available at all (the objects' own promises are already bitwise).
3. What patching buys: airport's four (insert on one loop, varispeed, >8 loops, tape you
   actually use), and `tap.bloom` as the most portable idea in the family.
4. Where the seams show — three honest costs.
5. Checkpoint.

## The three honest costs (the section that earns the chapter)

- **The garden's patch is not sample-accurate.** `tap.bloom`/`tap.gardener` run on Max's
  scheduler; returns land within an `@interval` tick. Stated plainly, with the consequence:
  there is deliberately no in-Max null test for the garden, because asserting a null that
  cannot hold is worse than not asserting one.
- **Voice stealing had to stay in the kernel.** The obvious `poly~` answer is wrong twice
  — round-robin stealing loses the glide-not-click promise, and `poly~` does not exist off
  Max. This is the decision the chapter should be clearest about, because it looks like
  over-engineering until you know both halves of the reason.
- **A bell reads silent until processed once**, so same-sample strikes collide onto one
  voice. Pre-existing, surfaced by the new rack test, documented rather than fixed —
  fixing it would change the sound.

## Deliberately left out

- `composite_period` has nowhere to live in a patch of independent reels (it needs all the
  lengths at once). `tap.reel~` reports `loopsamples`, which is the raw material; a
  `tap.period` utility is the obvious follow-up and is flagged in `REVIVAL.md` rather than
  quietly dropped. The chapter names the loss instead of pretending the decomposition is
  free.
- No new notebook sections. The claims here are structural (bitwise identity, exact
  arithmetic), which pinned tests carry better than measured cells.

## Voice notes

- Same as the family chapters: the person patching, not the person marketing.
- Resist "modular is better". The monoliths are the put-it-on-and-walk-away objects and the
  chapter should say so in its first three sentences, so the split reads as additive.
- Titles considered and rejected: "Taking the lid off" (cute, says nothing), "The
  decomposition" (an engineering word for a musical book), "Components" (a category, not a
  title).
