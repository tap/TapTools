# Introduction

TapTools is a collection of Max/MSP objects with roots back to 1999, rebuilt in
2026 on a portable DSP kernel library (this repository) with thin Max wrappers
(the [TapTools-Max](https://github.com/tap/TapTools-Max) package). This book is
its field guide, in the tradition of the AmbiTap and SampleRateTap books and
MuTap's *Quieting the Loop*: one chapter per object family, written for the
person patching at 11 pm, not for the person grading a DSP exam.

Each chapter makes the same promises:

- **It says what the thing is for** — and, near the end, when it is the wrong
  tool, because every tool is sometimes the wrong tool.
- **Every performance claim is measured, not remembered.** The numbers in these
  chapters come from the kernel's own test suite and from the executed
  verification notebooks in [`notebooks/`](https://github.com/tap/TapTools/tree/main/notebooks),
  which drive the *same C++ code the Max objects compile* through a C ABI. When
  a chapter says "47 dB", a notebook cell measured 47 dB, and you can re-run it.
- **Trade-offs are stated as trades.** Knobs that buy something always pay with
  something; the chapters try to name both sides.

The book is organized the way a patch is:

- **Part I — Sources**: the virtual-analog oscillator, `tap.vco~`, including
  its analog-character section and the honest Moog recipe.
- **Part II — Filters**: the morphing Simper SVF (`tap.svf~`), the transistor
  ladder (`tap.ladder~`), and the envelope filter modeled on the Snow White
  AutoWah (`tap.autowah~`) — with its hardware-calibration harness.
- **Part III — Strings, rooms, and spirals**: exact true-stereo convolution
  (`tap.convolve~`) and the two GRM Tools recreations — the tuned comb bank
  (`tap.5comb~`) and the pitch-accumulating shimmer loop (`tap.pitchaccum~`).
- **Part IV — Tape and time**: the Eno recreations — the *Discreet Music*
  two-machine tape loop (`tap.discreet~`), the *Music for Airports*
  incommensurate loop bank (`tap.airport~`), the generative event garden
  (`tap.garden~`), and the components they decompose into.
- **Part V — The machines you ride**: the Radiohead family — objects whose
  point is the performance surface rather than a setting. The multi-head tape
  echo (`tap.tapecho~`), the live buffer-stutter rig (`tap.stammer~`), the
  two-stage fuzz (`tap.fuzz~`), the granular scrub pad (`tap.scrub~`), the two
  Ondes Martenot diffuseurs as standalone driven resonators
  (`tap.metallique~`, `tap.palme~`), and the Ondes Martenot voice itself
  (`tap.ondes~`, with `tap.triode~` and `tap.touche~`).
- **Part VI — The spectral set**: the 24-band vocoder (`tap.vocoder~`), the
  per-bin spectral gate (`tap.nr~`), and the bin remapper (`tap.spectra~`).
- **Part VII — The rhythm section**: the Roland recreations — the TB-303 voice,
  its diode-ladder filter, and its sequencer (`tap.303~`, `tap.diode~`,
  `tap.303.seq~`), and the eight TR-808 voice channels with their row
  sequencer (`tap.808.*`, `tap.808.seq~`).
- **Part VIII — Staying in tune**: the pitch corrector (`tap.tune~`) and the
  detection/resynthesis machinery it stands on.
- **Part IX — The pedalboard**: the stompbox recreations — the voiced feedback
  overdrive (`tap.overdrive~`), chasing the TS-lineage feedback pedals rather
  than a waveshaping curve.
- **Part X — The machine, file by file**: the SampleRateTap-style deep dives —
  one chapter per kernel header, deriving the math, reviewing the code, and
  recording *why* each algorithm is written the way it is, alternatives and
  all. Parts I–IX are for driving the objects; Part X is for trusting them —
  or changing them.
- **Part XI — Recipes**: whole patches chasing specific sounds — the TR-808
  kits behind four decades of records, the three-oscillator Moog voice —
  with settings you can check against the reference pages and the honest
  accounting of what each ingredient buys.

More chapters land as objects mature; the utility and Jitter objects live in
their reference pages, where they belong.
