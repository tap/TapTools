# Borrowed rooms

Every room you have ever heard is a filter: clap once, and what comes back —
the impulse response — is everything the room will ever do to any sound.
Convolution reverb plays your signal *through* that recording. `tap.convolve~`
does it in true stereo against an impulse response held in a `buffer~`, using
the standard engine of the genre (uniformly-partitioned overlap-save FFT
convolution), and its defining property is worth stating up front: **it is
exact**. Not "high quality" — exact. This chapter is what that buys, what it
costs, and how to drive the two knobs that actually matter.

Companion material: the reference page and help patcher in the TapTools-Max
package, and the [verification notebook](https://github.com/tap/TapTools/blob/main/notebooks/convolution_reverb.ipynb) —
the first notebook in this repo, and the template for all the others.

## Exact, measured

The engine splits the IR into `blocksize`-sample partitions, transforms each
once, and multiply-accumulates in the frequency domain over a delay line of
past input spectra. The bookkeeping is intricate; the result is not. Measured
against a direct time-domain convolution of the same float32 IR: maximum
difference **3×10⁻¹²** — double-precision noise. Change the block size and
the output doesn't change either (64/256/1024 agree within 2×10⁻¹²,
latency-removed). An impulse through the engine reconstructs the IR to
5.5×10⁻¹⁴, and a synthetic 0.60 s-RT60 reverb measures back at 0.599 s. There
is no "character" in this engine to audition; the character is entirely in
the IR you load.

## The wiring, and the one real cost

Stereo in, stereo out, IR from a named `buffer~`. The cost: **latency of
exactly `blocksize` samples** — verified for every block size — on top of
your I/O latency. That is the entire quality/latency dial:

- **`blocksize`** small (64–128): tight enough for live input; more CPU per
  sample (more, smaller FFT batches).
- **`blocksize`** large (512–2048): cheapest; latency grows to match. For a
  send/return reverb on a mix bus, nobody hears 21 ms of pre-delay you didn't
  ask for — except you, so use `predelay` deliberately instead.

`maxsize` reserves capacity (both are locked in while DSP runs and apply on
restart). The rest of the surface mirrors `tap.verb~` so the two reverbs read
as siblings: `mix`, `gain`, `predelay`, `normalize` (energy-based, so quiet
and hot IRs land at comparable levels), `bypass`, `mute`.

## True stereo, by channel count

A stereo room isn't two mono rooms: sound from the left source arrives at the
right ear too. The engine runs the full 2×2 matrix — LL, LR, RL, RR — and the
`buffer~`'s channel count selects the topology:

- **4+ channels:** true stereo, all four paths (measured: a signal sent only
  left emerges on the right at exactly the cross-feed path's gain, 0.600
  expected, 0.600 measured, with zero leakage where paths are silent).
- **2 channels:** dual mono — L and R convolved separately, no cross-feed.
- **1 channel:** the same mono room on both sides.

## Loading rooms while the music plays

IRs are analysed off the audio thread and published atomically into a
double-buffered slot: swapping IRs mid-performance neither clicks nor drops
(measured RMS across the swap instant: 21.9 before, 22.1 just after), and one
block later the output is **bit-identical** to an engine that had the new IR
from the start. Load rooms like presets; the engine doesn't flinch.

## Recipes

- **The honest room:** a measured IR (church, plate, spring — the internet is
  full of them), `@mix 25 @normalize 1`, and resist the urge to EQ the IR
  itself before trying `predelay` — 10–30 ms of it buys clarity for free.
- **Not a reverb at all:** an IR is any filter. A single click is a delay; a
  strummed guitar body is a body simulator; a vowel is a formant filter.
  Convolution doesn't know it's supposed to make reverb.
- **True-stereo width:** record or synthesize the four paths with a genuinely
  different LR/RL from LL/RR — the cross-feed is where "being in the room"
  lives.

## When it is not the right tool

- **You want to *design* the reverb** — decay knobs, damping, modulation,
  gated tails. A static IR can't do any of that; `tap.verb~` (the algorithmic
  Moorer reverb, with its own oversampling and limiter) can.
- **Zero-latency insert on a live path.** The engine costs `blocksize`
  samples, full stop. At 64 that's 1.3 ms — small, not zero.
- **Time-varying convolution.** Swaps are click-free but discrete; the engine
  doesn't interpolate between rooms.

## Checkpoint

Partitioned convolution is exact linear convolution — measured to 10⁻¹² —
with one honest cost, `blocksize` samples of latency, and one honest dial,
block size against CPU. True stereo comes from the buffer's channel count;
IR swaps are atomic and dropout-free; and everything the effect *sounds like*
is the impulse response you feed it. Borrow better rooms.
