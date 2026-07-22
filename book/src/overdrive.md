# Distortion with a memory

Every distortion plugin can bend a transfer curve. `tap.overdrive~` is built
on the observation that the pedals people actually love — the Tube Screamer
lineage, and specifically the Mad Professor Little Green Wonder that served as
this object's listening reference — don't apply one curve to the whole
spectrum. Their clipper lives inside an op-amp's feedback loop with
frequency-dependent parts around it, and that loop is most of the sound: bass
sees less gain and stays tight, mids break up first, and the knee never quite
flattens because the clean signal always rides through. A memoryless
waveshaper — including both modes of the Jamoma-era `tap.overdrive~` this
object succeeds — structurally cannot do any of that. This one can, because
the shaper sits inside a lowpass feedback loop: distortion with a memory.

Companion material: the reference page and help patcher in the TapTools-Max
package, and the [verification notebook](https://github.com/tap/TapTools/blob/main/notebooks/overdrive.ipynb),
where every number below is an executed, plotted measurement of the shipping
kernel.

## What the loop buys

The claim worth leading with, because no static curve can make it: the
object's small-signal gain **tilts with frequency, and the tilt grows with
drive**. Measured between 80 Hz and 4 kHz, the tilt is +5 dB at `drive 0`
(just the voicing EQ), +16.3 dB at `drive 0.5`, +17.2 dB at `drive 0.9`. Low
frequencies are pinned near-clean by the feedback while mids and highs take
the full drive gain — so a low E stays articulate under the same setting that
saturates the pick attack. That is the Tube Screamer "tightness" in one plot.

The second structural trait: the transfer **never flattens**. A unity clean
path is summed around the clipper — the non-inverting op-amp topology — so
however hard the shaped part saturates, output keeps rising with input
(measured strictly monotonic at every drive setting). The old sine-shaper
mode's hard ±1 plateau, a large part of what read as "digital," is gone by
construction.

## The knobs, one by one

### `drive` — 0 to 1, edge-of-breakup to saturated

Normalized, like every musical parameter on this object, with the perceptual
mapping done inside (the knob sweeps the clipper's gain from +6 to +46 dB,
with a level compensation tracking it). `drive 0` is a pedal's gain knob at
full counterclockwise — still warm, not bit-clean; `bypass` is the clean
switch. The normalized range maps directly onto MIDI/OSC controllers, and
onto Q15/Q31 fixed-point for the embedded ports this kernel is written to
survive.

### `body` — the signature voicing control

The LGW's defining knob, reproduced as linear pre/post EQ around the clipper
(that's what it is in the pedal — voicing, not nonlinearity). Toward −1,
fuller lows reach the clipper and the top gets a slight shelf lift; toward
+1, the lows thin and tighten and an upper-mid bell pushes forward — centered
at 1150 Hz, deliberately above the classic TS hump. Measured at the extremes:
100 Hz moves by 10 dB, the 1150 Hz push adds 4 dB, the counterclockwise
treble lift is +2.5 dB at 8 kHz. The exact centers and gains are by-ear
placeholders pending the in-Max voicing pass against LGW demos — the shape of
the control is final, the seasoning isn't.

### `asymmetry` — the even harmonics the old object couldn't make

Both Jamoma modes were odd functions: odd harmonics only, the entire "warmth"
vocabulary absent. `asymmetry` biases the clipper: at 0 the path is exactly
symmetric (measured H2 at −151 dB — the numerical floor), and raising it
brings the even series up smoothly (H2 at −26 dB by `asymmetry 0.6`). The
default sits at 0.15, a small nonzero warmth chosen by ear. Asymmetric
clipping generates DC, so a DC blocker sits permanently after the clipper —
measured output mean under full drive, full asymmetry: 10⁻¹⁰. (The original
TTOverdrive contained a DC blocker whose output was computed and then
discarded; this one is load-bearing.)

### `oversample` — 1, 2, 4, or 8; default 4

Clipping makes harmonics; harmonics past Nyquist fold back as inharmonic
junk. At 1× a hard-driven 5 kHz tone puts its folded seventh harmonic at
−22 dB relative to the fundamental — clearly audible garbage at 12993 Hz. At
the default 4× the same component measures −36 dB, with the true harmonics
unchanged. Turn it down to 1× only when CPU matters more than the top octave,
or when you *want* the fizz.

### `preamp`, `output`, `smooth`, `bypass`, `mute`

Input and makeup gain in dB (±24) — the only unit-bearing parameters, because
gains are the one place real units belong. Everything ramps click-free over
`smooth` milliseconds (default 20).

## Where it sits in a patch

Mono by design; wrap it in `mc.` for multichannel like the rest of the
package. It takes line-level signals as happily as guitar DI — the drive
mapping is normalized to full-scale digital, not to pickup output. For the
LGW move, start at `drive 0.4, body -0.3, asymmetry 0.15` and ride `body`
against the source's low end. For a clean boost that just thickens, `drive 0`
with `asymmetry 0.3`. For fuzz territory this is the wrong object on
purpose — the loop keeps pulling it back toward articulation.

Every claim above is pinned twice: as an executed measurement in the
notebook, and as a hard assertion in the kernel's Catch2 suite
(`tests/overdrive_test.cpp`), which CI runs on every push. The math behind
the loop — including why it had to be solved zero-delay, and what happens if
you don't — is in the machine chapter:
[The clipper in the loop](machine/overdrive.md).
