# Two hands on a live buffer

`tap.scrub~` is the object in this library that most needs a controller
attached before it means anything. Everything below assumes one: an XY pad,
two faders, a trackpad, a phone sending OSC — anything that gives you two
continuous values at once. The recipe is mostly about what to put on each
axis and why.

Measured claims are borrowed from [two hands on the same
tape](../scrub.md).

## The chain

```text
source ──┬─▶ tap.scrub~ ──▶ tap.palme~ ──▶ out
         └────── (its own dry path, via mix) ──────▶
```

The scrub records what passes through it, so it goes *in* the signal path
rather than on a send — there is nothing to send it that it is not already
hearing. Its `mix` is the dry/wet, and at `mix 0` it is the input bit for
bit, which means you can leave it in the chain permanently and have it be
audibly absent until you touch it.

## The pad

| control | setting |
|---|---|
| `maxhistory` | `4.` s (object argument; bought at DSP start) |
| `position` | **X axis**, 0–1500 ms |
| `pitch` | **Y axis**, −12 to +12 st |
| `drift` | `0.` |
| `size` | `80.` ms |
| `overlap` | `2` |
| `spray` | `0.` |
| `mix` | `100` while playing |
| `smooth` | `0.` if driving by signal, `20.` if by messages |

**X is position, Y is pitch, and the whole object exists because those are
independent.** On tape they would be the same axis — moving the head *is*
the pitch change. Here you can rake back through the last second and a half
at the pitch you started at, or hold still and transpose, or do both at
once in different directions. Spend the first five minutes doing each
separately; the object does not become obvious until you have felt that
they do not interact.

**`size` is the texture control.** 80 ms is a granular pad. Down at 20 ms
it turns metallic and starts pitching itself at the grain rate; up at
200 ms it stops being granular and becomes a soft varispeed. Sizes that
divide evenly by `overlap` have an exactly flat window sum — 80 with
overlap 2 does — which matters when you want the still position to be
clean.

**`overlap 1` is a texture, not a mistake.** It leaves gaps between grains:
a gated, chopped version of the same gesture. Worth a switch on the
controller.

## The honest bit about pitch

Transposing here warbles, and it is measured rather than apologized for:
98.8 % of a perfect shifter's energy lands within ±15 Hz of the transposed
pitch (worst case 91.7 %), so the *note* is right — what the object loses
is concentration, 92.0 % as focused as a clean shift and 75.0 % at worst.
Audibly that is a warble, and it is the classic single-delay-line
pitch-shifting artifact rather than anything peculiar to this kernel.

Two ways to work with it:

- **Lean in.** `spray 30.` trades the narrow comb for a broadband smear.
  On sustained material this reads as a texture rather than a fault, and it
  is the better answer for pads.
- **Stay out of its way.** Keep the Y axis to ±7 and let the position do
  the work. Small intervals warble least and the object is a scrub pad
  first.

If you need a clean shift, this is the wrong object — `tap.shift~` is built
for it. (`tap.pitchaccum~` is built for shimmer rather than transparency,
and has [an open issue](https://github.com/tap/TapTools/issues/33) about
where its line actually sits.)

## Freeze, which is the other half

`freeze 1` stops the recorder. The playhead keeps going, so the position
now addresses fixed tape and the grains loop the same window — and you can
still scrub, transpose, drift and spray through it. Nothing is going into
the input any more, which is the point: it is a hold you can perform.

A sequence that works on stage:

1. Play the phrase through at `mix 0`. Nothing happens; the tape fills.
2. `mix 100`, `freeze 1`. The last few seconds are now the instrument.
3. Drag X slowly. This is the scrub.
4. `drift -0.3`. The playhead walks backwards on its own while you keep
   your hand free for Y.
5. `spray 40.`, `size 200.`. It stops being a phrase and becomes a pad.
6. `freeze 0`, `mix 0`. The room comes back.

Step 6 is exact — `mix 0` is bitwise passthrough — so the return is clean
however far out step 5 went.

## The one constraint

A grain born `position` behind the live edge and playing at rate *r*
reaches `position − size·(r−1)` behind it by its end. Transpose *up* with
the position near the live edge and the grain's tail runs off the front of
the tape into the oldest material — a seam.

Nothing clamps it, because clamping would silently bend the pitch to keep
the grain in bounds, which is a worse failure than the seam. Practically:
**keep the position at least `size·(rate−1)` back**. At `size 80` and one
octave up that is 80 ms. Setting the X axis to start at 100 ms rather than
0 makes the whole problem disappear, and this is why the table above says
0–1500 rather than 0–1500 starting at zero.

## What each ingredient buys, in order

1. **A controller with two continuous axes.** Without it this is a delay.
2. **`freeze`.** The half of the object you can build a performance on.
3. **`size`.** The texture, and the only control that changes what kind of
   thing you are playing.
4. **A diffuseur after it.** `tap.palme~ @mix 40` sustains what the scrub
   chops; the strings fill the gaps that `overlap 1` opens.
5. **`spray`.** Trades one artifact for another. Real, and last.

## When to leave the recipe

- **You want it in time.** Nothing here syncs. That is `tap.stammer~`, on
  the same tape — literally the same `capture` code — and the two are
  meant to be swapped between rather than combined.
- **You want a clean delay.** `tap.delay~` costs a fraction as much and
  windows nothing.
- **You want the position to feel like a jog wheel.** Put `drift` on a
  spring-loaded control and leave `position` alone: drift is velocity where
  position is location, and for wheel-like gestures velocity is the right
  variable.
