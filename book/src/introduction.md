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

Chapters land as objects mature. First up: the virtual-analog oscillator,
`tap.vco~` — including its analog-character section and the honest recipe for
the "three oscillators into a ladder" sound everyone actually wants it for.
