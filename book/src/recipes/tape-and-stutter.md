# The part that comes apart, on tape

Three objects and one posture: hands on the controls while it runs. The
stutter takes a phrase apart, the tape echo smears the pieces, and the fuzz
decides how hard the whole thing is being pushed. None of the three has a
"right" setting, which is the point of the part of the book they live in.

This recipe is a rig, not a record. What is documented about the Kid A-era
working method is that a laptop running Max sat in the signal path and got
played — the objects here are informed by *what that rig was for*, not by
anyone's patch. Nothing below is claimed to be a reconstruction of a
specific track.

Measured claims are borrowed from [four heads and a
motor](../tapecho.md), [the part that comes apart](../stammer.md) and
[the dirt with two stages](../fuzz.md).

## The chain

```text
source ──▶ tap.fuzz~ ──▶ tap.stammer~ ──▶ tap.tapecho~ ──▶ out
```

Order matters, and this order is the useful one:

- **Fuzz first**, because a stutter of a distorted signal is a stutter; a
  distortion of a stuttered signal turns every slice edge into a transient
  the clipper amplifies.
- **Echo last**, because the echo is the only object here that is supposed
  to blur. Put it before the stutter and the stutter slices the blur, which
  sounds like a mistake rather than a decision.

## The dirt

Keep it low. Two saturators in series get muddy fast, and the echo has its
own `drive`.

| control | setting |
|---|---|
| `gain` | `0.35` |
| `edge` | `0.3` |
| `asymmetry` | `0.2` |
| `bass` / `treble` / `contrast` | `0.` / `0.1` / `0.3` |
| `oversample` | `4` |
| `level` | to taste, usually negative |

`contrast` is the scoop, and a scooped source stutters better than a
mid-heavy one — the slices stop fighting the vocal or the guitar they came
from. Leave `oversample` at 4 and resist the urge to save the cycles at 2:
a hard `edge` on bright material is exactly the case where one doubling
stops being enough, and 2 measures badly above about 6 kHz.

## The stutter

| control | setting | what it does |
|---|---|---|
| `step` | `60.` ms | the grid |
| `divisions` | `1` | |
| `density` | `0.3` | how often it grabs |
| `repeats` | `4` | how long it holds |
| `reverse` | `0.2` | chance a repeat plays backwards |
| `jump` | `250.` ms | how far back a slice may reach |
| `fade` | `2.` ms | the flanks |
| `seed` | any integer | |
| `mix` | `100` | |

**The one thing to internalize: `repeats` is the hold, `density` is the
grab.** Occupancy — how much of the timeline has a slice in flight —
measures 41 % at density 0.3 / repeats 1 and **90 %** at density 0.9 /
repeats 1, but density 0.3 with repeats 6 already sits at 76 %. If the part
feels too busy, pull `repeats` before you touch `density`; you will keep
the sparseness of the *entrances* while shortening what each one does.

`seed` is a contract, not a flavour: the same seed and the same moves give
a bit-identical render, and two instances on two tracks decorrelate by seed
alone. Different seeds change 89 % of samples, so it is a real dice roll,
not a phase tweak.

At `density 0.` the object is a bitwise bypass — worth knowing, because it
means you can automate density to zero and get the dry signal back exactly,
with no crossfade artifact to work around.

**The material contract.** This object flatters a played phrase and
flatters a sustained note far too much. Slice similarity measures 1.000 on
a held sine against 0.286 on a plucked phrase: on sustained material every
slice is interchangeable, so the stutter has nothing to expose and sounds
like a tremolo. Feed it something with transients and pitch variety.

## The tape

| control | setting |
|---|---|
| `span` | `400.` ms |
| `heads` | `3` |
| `ratios` | `0.25 0.5 1.` |
| `levels` | `0.7 0.85 1.` |
| `pans` | `0.2 0.8 0.5` |
| `regen` | `0.55` (the ride starts here) |
| `darken` | `5000.` |
| `drive` | `0.4` |
| `wow` / `flutter` | `0.4 0.35` / `0.3 8.` |
| `mix` | `35` |

`span` is defined at the ratio-1.0 head, so the head at `1.` returns at
400 ms and the others at 100 and 200. Change `span` while it runs and the
whole thing glides like tape speed rather than splicing — that is the
transport, and it is the second-best gesture in this rig.

The best one is `regen`. **It goes past 1 on purpose.** Past unity the line
self-oscillates, and it stays bounded because the saturator caps what comes
back: the ceiling is `|in|max + regen/drive`, measured under that value at
every drive tried. So a ride up to `1.4` and back is a controlled build,
not a fire. Keep `drive` up while you do it — `drive 0.` removes the
saturator and the cap falls back to unity, which is the setting where a
long ride will *not* behave.

`darken` is the generation loss, and it is what makes repeats decay into a
shape instead of just getting quieter. 5 kHz is a good default; below 3 kHz
the tail turns to mud, which is sometimes what you want under a chorus.

## The gestures, in order of value

1. **Ride `regen` past unity and back**, while `mix` stays put. One hand,
   whole arrangement.
2. **Automate `density` to 0 and back.** Exact bypass, so it reads as the
   part reassembling rather than a fade.
3. **Move `span` during a held note.** Varispeed glide, not a splice.
4. **Change `seed` between takes**, never during one.
5. **`reverse` and `jump`.** Character, and cheap to overdo. Note `jump` is
   *milliseconds*, not a probability — at 250 the machine starts quoting
   material from a quarter-second before the slice it just took, which is
   where a stutter stops sounding like a stutter and starts sounding like
   an edit.

## When to leave the recipe

- **The source is sustained.** See the material contract. Put the stutter
  on the drums and leave the pad alone.
- **You want the slices in time with something.** Nothing here syncs to a
  transport; `step` is milliseconds. Drive it from your own clock if you
  need bars.
- **You want the echo to stay clean.** `drive 0.` gets you a clean line —
  but then do not ride `regen` past 1, because the cap that makes that safe
  is the saturator you just removed.
- **You want the dirt to be the point.** Then the fuzz belongs last, not
  first, and this is a different recipe: `tap.tapecho~` → `tap.fuzz~` with
  the echo's own `drive` at 0. Distorting a wash is a real sound; it is
  just not this one.
