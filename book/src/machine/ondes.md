# A citation, an identity, and a sign: `ondes.h`

Three classes — `triode`, `detector`, `voice` — and three things worth
recording about how they got here. One stage turned out to need no design
decisions at all. One approximation turned out to be an exact identity. And
one sign error made a distortion knob run backwards.

The circuit is Najnudel, Hélie, Roze & Boutin, "Simulation of an ondes
Martenot circuit", IEEE/ACM TASLP **28**, 2651–2660, 2020, modelling
instrument No. 169 as five port-Hamiltonian stages. This file is **not**
that: their full solve runs at 768 kHz and their plugin costs 85 % of a
laptop core. What it takes from them is their own published reductions plus
their published component values, and the header says which is which.

## The tube is a citation, not a design

The plan framed the valve stage as a choice: a published grid-conduction
curve, or the tanh family with an asymmetry bias voiced by ear. It is
neither, and finding that out took nothing more than reading the paper
properly.

The paper names a tube model — the **enhanced Norman Koren** model (Koren,
*Glass Audio* 8(5), 1996, with Cohen & Hélie's grid-current branch, AES 129,
2010) — writes out its three equations, and publishes parameter sets in
Table II **fitted to the actual valves in ondes No. 169**, together with
each stage's supply voltage, cathode resistor and plate load.

So there was nothing to voice. `k_6f5`, `k_6c5`, `k_2a3`, `k_op_demod`,
`k_op_preamp` and `k_op_power` are Table II transcribed, and the header says
they are the citation.

A stage is then the static solution of `ipc(vpc, vgc) = (Vbias − Vk −
vpc)/Rp` on the load line, with cathode bias `Vk = Rk·Ipc` found at the
quiescent point. That is a **memoryless nonlinearity** in exactly the
DAFx-07 sense, which matters for a practical reason: tabulating it is not an
approximation of the model, it *is* the model. The table is rebuilt on a
tube or operating-point change and read with linear interpolation, so the
audio path costs a lookup rather than a root find.

The published points bias sanely — the 6C5 demodulator lands at Vk 2.70 V,
Vp 86.5 V, Ip 2.70 mA, gain 4.86 — which is its own small confirmation that
the transcription is right.

## The sign that made the drive knob run backwards

The stage must **invert**, as a real common-cathode stage does, and this is
load-bearing rather than cosmetic. The valve's asymmetry acts on whichever
side of the waveform reaches its grid. An early cut normalized the output by
the *signed* small-signal gain, which quietly un-inverted the stage, so the
curve's lopsidedness landed on the wrong half of the waveform.

The symptom was unambiguous once measured: turning `drive` **up** reduced
total harmonic content. A distortion control that gets cleaner as you push
it is not a subtle bug, but it is only visible in a sweep — at any single
setting the object sounded like a valve.

Two changes fixed it. The curve is now the true (inverting) plate swing, and
normalization is by the gain's *magnitude*. And `voice::core` applies the
demodulator's own grid-leak inversion explicitly — a growing envelope drives
that grid toward cutoff — so the two inversions put the demodulator's plate
in phase with the envelope while the curve has meanwhile acted on the
underside. `drive` now sweeps harmonic content 0.221 → 0.344, monotonically.

The gain-staging lesson from `fuzz.h` was applied here from the start rather
than learned again: each stage is normalized by its own small-signal gain,
so drive changes the distortion and not the level.

## The detector is an identity, not a simplification

The plan's instruction for this stage was "synthesize the difference tone
directly as a sinusoid", and catching that as a mistake is the most valuable
thing this build did.

The paper's 0.03 % distortion figure and its licence to replace oscillators
with a sinewave generator apply to the **oscillators**. The demodulator is
not a mixer handing you a difference tone; it is an envelope detector, and
the envelope of `cos(Φ) + cos(Φ − φ)` is `2|cos(φ/2)|`, whose Fourier series
puts H2 at −14.0 dB, H3 at −21.3 dB and H4 at −26.4 dB. Synthesizing a
sinusoid would have discarded the instrument's largest single source of
harmonics before any of the modelled stages ran.

What replaces the carrier is better than a simplification. For amplitudes 1
and `depth`, the envelope is exactly

```
sqrt(1 + depth² + 2·depth·cos(2π f t))
```

so the 80 kHz carrier drops out of the arithmetic rather than being
approximated away. Running the published RC detector on that closed form —
instant attack through the diode, 200 µs decay through R4·C21 — reproduces a
full heterodyne-plus-diode-plus-RC simulation to **within 0.10 dB on every
harmonic** at every pitch tried (`ondes.ipynb` §2).

There is one systematic difference, and it is worth knowing it is systematic
rather than noise: the closed form sits a uniform **3.0–3.2 % high**,
because a follower chasing real carrier half-cycles never quite reaches the
peak between them. On a synthesizer with a level control, that is a
constant.

The detector's characteristic pitch dependence comes along free, out of the
same 200 µs: H2 runs −14.0 dB at A2 to −19.3 dB at A6, and the level falls
2.0 dB across those five octaves.

And a bonus nobody planned: because the closed form is parameterized by the
two oscillator amplitudes, **oscillator balance becomes a physical timbre
control**. `depth` is a real mismatch between two real oscillators, not an
invented knob.

## Three measurements that lied, and what they were doing

All three were committed to a notebook or a header before being caught.

**Too few periods.** The first measurement of the detector's harmonics at
low pitch used a window holding about 2.75 periods of the fundamental.
Spectral leakage at that resolution dominated everything, and it produced a
confident, wrong claim in the header: "−9.8 dB at A2, level falls 9.7 dB".
Redone with 60 cycles, the real answer is −14.0 dB and 2.0 dB. Both numbers
were in a shipped header before the recheck.

**Probing where the answer is exactly zero.** The aliasing scenario probed
half-integer harmonics of a tone that was exactly periodic in the analysis
window. Those bins are analytically zero, so it measured −281 dB and passed
triumphantly. Fixed by computing the actual fold frequencies for a tone at
2637 Hz — deliberately not a submultiple of 48 kHz — and skipping folds that
land near real harmonics. This is the same family of error `fuzz.h` records
under "choosing a tone that divides the sample rate", committed again in a
different disguise.

**Stopping the sweep at the first plateau.** The header initially claimed
"4× is the knee, then flat". The notebook's own more careful run — settled
state, 131072-point Hann — showed 8× continuing to improve in the top
octave. Corrected to "never worse", with the full table in the header, the
test comment and the notebook.

## The evidence that closed an open question in `fuzz.h`

`fuzz.h` measured its oversampling sequence going the wrong way — 4× worse
than 2× — and had left an untested hypothesis behind: that the culprit is
*imaging*, since zero-stuffing by N leaves N−1 images for one filter to
suppress, and residual images entering a nonlinearity intermodulate into
products that are not harmonics of the input.

This file runs the **same** 8th-order Butterworth chain around a comparably
hard nonlinearity, and its sequence never reverses:

| tone | 1× | 2× | 4× | 8× |
|------|----|----|----|----|
| 587 Hz | −79.3 | −91.2 | −104.5 | −103.8 |
| 1175 Hz | −65.8 | −77.2 | −90.6 | −92.5 |
| 1760 Hz | −57.6 | −70.9 | −81.1 | −82.2 |
| 2637 Hz | −51.1 | −61.4 | −71.8 | −83.8 |
| 3520 Hz | −45.4 | −56.8 | −67.0 | −74.2 |

The difference between the two files is exactly the hypothesis: this object
is a **source**. Nothing is zero-stuffed on the way up — the detector simply
runs fast — so there are no images at all.

That was evidence, not proof — the nonlinearities differ too, and one
confounded comparison does not settle a question. But it was the first
evidence either way, and it pointed somewhere specific enough to act on.

**Acting on it settled it.** `fuzz.h` now cascades one 2× stage per doubling
instead of zero-stuffing by N once, each stage filtering at a corner that
never tightens however deep the cascade goes. Its reversal is gone — worst
step-up past 2× is a ratio of 1.017 — and its 4× and 8× improved by two to
four orders of magnitude, for about 5 % more CPU. This file needed no change,
having no upsampler to fix.

Worth naming the shape of it, because it is not the usual one: the evidence
that resolved a two-wave-old open question in one file came from **building a
different file that happened to differ in exactly the right variable**. It
was not designed as an experiment. It was noticed, written down in both
headers as evidence rather than proof, and left where the next person would
trip over it.

## A wrapper test that found a kernel bug

`tap.ondes~`'s Min-level test asserts something a patcher would otherwise
file as a bug report: with the key at rest, the object is *exactly* silent.
It failed.

`voice::set_smooth_ms` set the voice's own ramps but never forwarded to
`touche::key`, which keeps its own slew. So a key sitting at zero with
`@smooth 0` still sounded for 20 ms after every parameter touch.

This is the two-layer split working the way it is supposed to. The kernel
suite tests DSP promises; the wrapper suite tests what a patcher will
actually observe, and those are not the same set. The fix landed in the
kernel with its own scenario, not in the wrapper.

## What this file will not do

The real instrument has switchable **waveform registers**. Their filter
shapes are in none of the sources obtained. Adding them from imagination is
the one thing this file is careful not to do, and the omission is stated in
the header, the object description and the reference page rather than left
as a gap someone might charitably fill later.

Two controls *are* choices — where the intensity key sits (`keyplacement`)
and the coupling transformer's winding sense (`polarity`) — because the
paper's five stages do not settle either. Both are labelled as choices, and
both were measured to confirm they are audible ones: about 0.09 and 0.12 of
total harmonic content respectively.

## Checkpoint

A stage that required no design because the paper published the model and
its fitted parameters. A detector that is exact rather than approximate, and
cheaper than the thing it replaces. One sign error that inverted the meaning
of a distortion knob and was invisible at any single setting. Three
measurements that lied in three different ways, all recorded. The evidence that closed `fuzz.h`'s
oversampler question, from a file that happened to differ in exactly the
right variable and was not built as an experiment. And a wrapper test that found a kernel bug,
which is the split doing its job.
