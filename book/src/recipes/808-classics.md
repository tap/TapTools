# One machine, four decades

The TR-808 sold poorly, was discontinued in 1983, and then spent forty years
becoming the most influential drum machine ever built — not by being
realistic, but by being *itself* in four different genres' hands. This
recipe visits four of those hands: the 1982 electro of "Planet Rock," the
same year's slow soul of "Sexual Healing," the tuned-kick boom of Miami
bass, and the half-time rolls of trap. Same eight circuits every time; what
changes is the pattern, the accents, and which knob someone dared to turn
all the way up.

One honesty note before the first grid: **these patterns are starting
points, not transcriptions.** Where a record's production story is
documented, the recipe says so; the grids themselves are the versions ears
agree get you into the neighborhood, and your ears finish the trip. The
voice knobs, on the other hand, are exact — every attribute below is
spelled as the shipping object spells it, and the calibration numbers
behind the voices live in the [drum machine chapter](../drums.md) and the
[`tr808_calibration.ipynb`](https://github.com/tap/TapTools/blob/main/notebooks/tr808_calibration.ipynb)
notebook.

## The scaffold every recipe shares

One `phasor~` is the transport; every `tap.808.seq~` row reads it; every
row's output cable is a voice's trigger input. The phasor's frequency for a
16-step bar of 4/4 is **BPM ÷ 240** (four beats per cycle, four sixteenths
per beat). Rows fed the same ramp are sample-locked forever — that is the
sequencer's phase-derived design (see [its machine
chapter](../machine/seq.md)), and it is why nothing below mentions sync.

```text
phasor~ (BPM/240)
   ├── tap.808.seq~  ──▶ tap.808.kick~   ──┐
   ├── tap.808.seq~  ──▶ tap.808.snare~  ──┤
   ├── tap.808.seq~  ──▶ tap.808.hat~    ──┼──▶ +~ ──▶ tap.limi~
   ├── tap.808.seq~  ──▶ (open) hat inlet 2┤
   └── tap.808.seq~  ──▶ tap.808.cowbell~──┘
```

Program a row with two lists: `hits` (which of the 16 steps sound, 1/0 per
step) and `accents` (which sounding steps lean, 1/0 per step). An accented
step emits the row's `accented` level (default 0.5), a plain step emits
`plain` (default 0.01) — those defaults are the hardware's accent knob at
noon, and they matter more than they look, because a voice's trigger
amplitude is a *voltage on the 4–14 V bus*: an accented hit is punchier and
differently voiced, not merely louder. Raise `accented` toward 1.0 when a
groove should hit like the accent knob cranked. Single steps tweak with
`step <n> <velocity>` (1-based), and each row's 16 slots (`store`/`recall`)
hold your fills.

In the grids below, `X` is an accented hit, `x` a plain one, `.` a rest.

## 1982, the Bronx via Düsseldorf: the "Planet Rock" kit

The documented part: Afrika Bambaataa and producer Arthur Baker built
"Planet Rock" on a rented TR-808, borrowing Kraftwerk's melodies, and its
kit — dry kick, clap-snare backbeat, offbeat cowbell — became *the* electro
sound. The orchestra stabs were a sampler's; everything percussive is the
machine's.

```text
step:    1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16
kick:    X . . . . . . x . .  x  .  .  .  .  .
snare:   . . . . X . . . . .  .  .  X  .  .  .
clap:    . . . . X . . . . .  .  .  X  .  .  .
closed:  x . x . x . x . x .  x  .  x  .  x  .
open:    . . . . . . . . . .  .  .  .  .  x  .
cowbell: . . x . . . x . . .  x  .  .  .  x  .
```

- Tempo ≈ 129 BPM → `phasor~ 0.5375`.
- `tap.808.kick~`: `@decay 0.35 @tone 0.55` — the electro kick is short and
  clicky, not the boom (that comes later in this chapter).
- `tap.808.snare~`: `@tone 0.6 @snappy 0.7`; layer `tap.808.clap~` on the
  same backbeat row — the clap-plus-snare composite is half the sound.
- `tap.808.cowbell~` on the offbeats, `@level 0.6`. The drum machine
  chapter's line stands: more cowbell is a patching decision.
- Hats: closed 8ths; the open hat answering just before the bar turns.
- Fill: `store 1` the main pattern, program the classic descending-tom fill
  (`tap.808.tom~`, `@size high` → `mid` → `low` on three rows) into slot 2,
  and `recall 2` a bar before the phrase ends — `quantize cycle` (the
  default) swaps it exactly on the downbeat.

## 1982, Ostend: the "Sexual Healing" slow jam

The documented part: Marvin Gaye programmed the TR-808 himself for
"Sexual Healing," and it became one of the first major hits carried by the
machine — proof in the same year as "Planet Rock" that the same circuits
could whisper. The kit is soft, sparse, and riding the plain/accent
distinction rather than density.

```text
step:    1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16
kick:    X . . . . . . x . .  .  .  .  .  .  .
snare:   . . . . x . . . . .  .  .  x  .  .  .
closed:  x . x . x . x . x .  x  .  x  .  x  .
open:    . . . . . . x . . .  .  .  .  .  x  .
claves:  . . x . . . . . . .  x  .  .  .  .  .
```

- Tempo ≈ 94 BPM → `phasor~ 0.3917`.
- `tap.808.rim~ @model claves` — the high tick is the hook of the kit.
  `@level 0.5` keeps it a seasoning.
- `tap.808.kick~`: `@decay 0.6 @tone 0.35` — rounder than electro, still
  polite.
- `tap.808.snare~`: `@snappy 0.35 @tone 0.4` — more drum, less noise.
- Leave the sequencer's `plain` level at its 0.01 default and place accents
  *sparingly*; at this tempo the difference between a 4 V hit and a
  half-accented one is the entire feel.
- A touch of `@swing 0.15` on the hat row loosens the grid the way a human
  thumb on the start button did.

## Late eighties, Miami: the kick is the bassline

Miami bass turned the kick's `decay` knob to the top and discovered the
808's bass drum is a *tuned instrument* — a bridged-T resonator whose
fundamental sits near 49 Hz (measured within 2.4 % of a real unit across
the knob grid; see the calibration pass in the drum machine chapter). Turn
`decay` up and it rings for seconds; give two copies two `tuning` ratios
and you have a two-note bassline.

```text
step:            1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16
kick A (root):   X . . . . . . . . .  X  .  .  .  .  .
kick B (fourth): . . . . . . X . . .  .  .  .  X  .  .
snare:           . . . . X . . . . .  .  .  X  .  .  .
closed:          x . x x x . x x x .  x  x  x  .  x  x
```

- Tempo ≈ 126 BPM → `phasor~ 0.525`.
- Two `tap.808.kick~` objects, two rows. `tuning` is a ratio of the stock
  fundamental, so **target Hz ÷ 49 ≈ your setting**: kick A `@tuning 1.0`
  (G1, where stock already sits), kick B `@tuning 1.33` (≈ C2, the fourth).
  Both `@decay 1.0` — the whole genre is that knob at the top.
- `@tone 0.2` keeps the click out of the way of the ring; `@attack 0.5`
  softens the punch mechanism if the notes should bloom instead of hit.
- `tap.808.snare~ @snappy 0.8 @drive 6` — the swing-VCA drive is the crack
  that cuts through the sub.
- Watch the sum: two ringing kicks stack. `tap.limi~` on the bus is the
  modern answer; riding `level` per voice is the period one.

## The 2010s: trap, and the arithmetic of rolls

Trap keeps Miami's tuned, sustained kick and moves the snare to beat 3 —
the half-time frame — then spends all its rhythmic budget on hi-hat
subdivision games. Those games are where this sequencer's phase-derived
design pays off: rows of *different lengths* off one phasor divide the same
bar differently, so a 32nd-note roll row and a 16th-triplet roll row are
just `length 32` and `length 24` — polymeter as arithmetic, measured in the
[sequencer notebook](https://github.com/tap/TapTools/blob/main/notebooks/step_seq.ipynb).

```text
step:              1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16
kick:              X . . . . . . x . .  x  .  .  .  .  .
snare:             . . . . . . . . X .  .  .  .  .  .  .
closed (len 16):   x . x . x . x . x .  x  .  .  .  .  .
roll    (len 32):  steps 25–32 hit, plain    (32nds on beat 4)
triplet (len 24):  steps 19–24 hit, plain    (16th triplets, beats 3–4)
```

- Tempo ≈ 140 BPM → `phasor~ 0.5833`.
- Tune the kick to the song's key with the ratio table: E1 ≈ `@tuning
  0.83`, F1 ≈ `0.88`, G1 = `1.0`, A1 ≈ `1.12`. `@decay 1.0 @tone 0.15`, and
  keep `sigh` at its default 1.0 — the pitch relaxation *is* the 808-bass
  glide everyone samples.
- The roll rows: program hits only on their last steps (as above), leave
  them muted (`@mute 1`), and unmute for the bar that needs the roll — or
  keep separate patterns in slots and `recall`. Fast rolls do not
  machine-gun: the voices' filter states persist across triggers, so a roll
  interferes with the ringing tail like hardware (pinned by the family's
  tests; see the drum machine chapter).
- Alternate hat voicing per unit: `@seed` is which 808 you own, and
  `@tolerance 0.3` puts the metal bank's oscillators off-grid the way
  resistor variance really does (Werner et al. measured up to ~20 % — the
  chapter has the numbers). Two hat objects, two seeds, panned, is a stereo
  kit for free.

## What each ingredient buys

1. **The pattern and its accents.** Four decades of genre difference above
   is mostly the grids. The accent flags are not dynamics polish — they are
   the hardware's second voicing per drum. Spend your time here.
2. **`decay` and `tuning` on the kick.** One knob separates electro from
   Miami; one ratio puts the kick in the song's key.
3. **The composite backbeat.** Clap + snare on one row (electro), or snare
   `drive` (Miami, trap) — the backbeat carries the genre signature after
   the kick.
4. **Polymeter rows for rolls.** Two extra rows, two `length` values, and
   the trap chapter of the machine's biography writes itself.
5. **`seed`/`tolerance` on the metal.** Seasoning, in the salt sense:
   invisible until you A/B two units.

## When to leave the recipe

- **You want *those records*, exactly.** Mix, tape, room, and a human on
  the start button are not in the box; at some point the honest tool is the
  actual sample.
- **You want velocity-per-step expression.** The `velocities` list gives a
  row continuous 0..1 levels — but note it trades away the two-level
  hardware model; the accent bus is the 808's own idiom.
- **You want 909, LinnDrum, or DMX.** Different circuits, different
  machines — this family models one instrument, and its refusal to be
  generic is the point.
