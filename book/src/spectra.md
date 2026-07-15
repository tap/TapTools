# The spectrum, re-plumbed

Every process so far in this book treats the spectrum with respect: filters
shade it, gates prune it, vocoders dress it up. `tap.spectra~` re-plumbs it.
Each output bin *k* is filled from input bin round(k · `remap`) — the
spectrum's contents redistributed by a rule with no acoustic justification
whatsoever. It is the one object in this book whose purpose is to sound like
nothing in nature, and it is honest about it: the reference page has called
it an "ultra-non-linear effect" since 2002. This chapter is what the rule
does, why the results are inharmonic almost everywhere, and how to drive an
effect whose sweet spots are narrow and strange.

Companion material: the reference page and help patcher in the TapTools-Max
package; the kernel's Catch suite pins the two anchor behaviors below.

## The rule, and its two pinned anchors

Inside the object's own STFT (the same Hann/4×-overlap engine as `tap.nr~`),
the lower half of the output spectrum is assembled by reading input bins at
`k · remap`, and the upper half is mirrored to keep the spectrum Hermitian —
so the output is always real, whatever violence the remap did. Two behaviors
are pinned by test:

- **`remap 1` is the identity**: the output reconstructs the input exactly,
  delayed by one FFT frame. Transparent machinery, like its sibling.
- **`remap 2` moves input bin 2k to output bin k** — the spectrum compressed
  toward the bottom: content from twice the frequency lands at half.

## Why almost everything comes out inharmonic

Pitch shifting scales frequencies *continuously*; this remaps **bin
indices**, quantized to round(k · remap). A harmonic series at f, 2f, 3f…
survives integer remaps in recognizable form — `remap 2` folds a harmonic
spectrum roughly an octave down — but at `remap 1.37` the partials land on a
grid nature never drew: some merge, some vanish, spacings go irrational-ish.
The result reads as bells, metal, ghosts of the input. That in-between space
is the instrument. Sweep `remap` slowly across 1.0 and you can hear the sound
leave reality and come back.

Two practical corollaries:

- **`remap` just above or below 1** (0.9–1.1) is the subtle zone — a
  detuned, phasey shadow of the input, cheaper than it sounds.
- **`remap` well below 1** stretches the low spectrum upward across the
  output (each output bin reads a *lower* input bin), thinning the top;
  well above 1 compresses everything into the bass and discards the input's
  top octaves entirely. Loud, dark, and blunt — usually wants a fresh
  brightness source afterwards.

## The knobs

There is really one, plus the frame:

### `remap` — the rule

Continuous. Identity at 1; integer values are the quasi-musical landmarks;
everything between is the inharmonic wilderness. Automate it slowly — the
per-frame quantization means fast sweeps step audibly, which is either the
problem or the point.

### FFT size — the grain of the grid

Bigger frames put the bins closer together, so the remap grid is finer: less
quantization grit, smoother inharmonicity, more latency (one frame, as
always) and more transient smearing. Smaller frames make the remap chunkier
and more overtly digital. Unlike `tap.nr~`, where the frame is a fidelity
question, here it is a *flavor* question.

## Recipes

- **Bell foundry:** harmonic material (a `tap.vco~` saw, a piano) at
  `remap 1.3–1.6`, into a long reverb. Instant inharmonic percussion.
- **The shadow voice:** speech at `remap 0.95`, mixed subtly under the dry —
  a wrongness the ear notices before the mind does.
- **The corridor:** automate `remap` 1.0 → 2.0 over a minute under a
  sustained chord — a slow departure from consonance that lands, at exactly
  2, somewhere almost stable again.
- **Stacked plumbing:** two in series at `remap` a and b is a remap at a·b
  with two layers of quantization grit — the grit is the reason to do it.

## When it is not the right tool

- **Musical transposition.** The remap is spectral plumbing, not pitch
  shifting: use `tap.shift~` for clean intervals, `tap.pitchaccum~` for the
  spiral.
- **Harmonizing or formant work.** Nothing here knows what a formant is; the
  rule moves bins, not vowels.
- **Subtle timbre correction.** Even at its gentlest this object is a
  character effect; EQ-shaped intentions belong with `tap.filter~` or
  `tap.svf~`'s EQ modes.

## Checkpoint

One rule — output bin k reads input bin round(k · remap), Hermitian-mirrored —
inside a transparent STFT: identity at 1 (pinned), octave-fold at 2 (pinned),
and an inharmonic wilderness everywhere between. The FFT size sets the grain
of the grid, the sweet spots are narrow, and that is the appeal: this is the
book's one unapologetic reality-distortion tool. Use it where nature's
spectra have gotten boring.
