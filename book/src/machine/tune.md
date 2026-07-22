# The nearest allowed note: `tune.h`

The [pitch-primitives appendix](pitch.md) built the parts: a detector and
two shifters, each with a numeric contract. This appendix is about the
composition — `tap::tools::tune::corrector`, the object behind
`tap.tune~` — where the interesting problems are not algorithms but
*policies*: what runs when, what is allowed to allocate, what happens when
the detector reports nothing, and one measured surprise that became the
kernel's best war story. The scenarios in `tune_test.cpp` drive the class
directly, using the DspTap detector as an independent pitch oracle on the
output; `notebooks/tune.ipynb` re-measures the headline claims through the
C ABI.

## The pipeline and its clock

`process()` runs per sample; analysis runs per hop (256 samples at 48 kHz,
about 5.3 ms, scaled with the rate). Each sample: feed the detector's input
ring, maybe analyze, advance two slews (the applied correction and the
grain window), compute the ratio, resynthesize.

The geometry trick that keeps every setter real-time safe: the detector is
built at `prepare()` for the *worst case* — lags from 2 kHz down to 55 Hz —
and the user's `set_range()` merely filters results afterward, treating
out-of-range estimates as unvoiced. Changing the range never reallocates,
so it is safe mid-audio, and the price is a fixed analysis cost: with
window = τ_max = 873 samples at 48 kHz, the YIN difference function is
roughly 760k multiply-adds per analysis — an ~80 µs scalar spike every
5.3 ms, well inside a 64-sample vector's 1.3 ms budget, and the
FFT-accelerated difference function remains available behind the same
contract if an embedded target ever objects.

Analysis converts the detected period to MIDI, chooses a target
(next section), sets the correction goal in semitones, and retargets the
grain window. Unpitched frames set the correction goal to zero — the
corrector *relaxes* toward honesty — while the window holds its last value
rather than lurching toward a default.

## The period lock, told as a bug hunt

The first version of the resynthesis stage was the two-tap `tap.shift~`
engine with its grain window clamped to a sensible fixed range, minimum
5 ms. The oracle tests immediately failed — not wildly, *musically*: a
452 Hz input hard-snapped to A440 came out at 441.4 Hz, 5.4 cents sharp.
Detection was exonerated first (0.06 cents), then the applied correction
(right to five decimals). The bias lived in the shifter itself, and an
isolation experiment found the shape of it:

| grain window | measured output | error |
|---|---|---|
| exactly 2 detected periods (212.4 smp) | 439.98 Hz | −0.06 cents |
| fixed clamp (240 smp) | 441.38 Hz | +5.41 cents |
| 480 smp (≈4.52 periods) | 441.36 Hz | +5.34 cents |

The two taps ride the same phasor half a cycle apart, so they sit
window/2 samples apart in the delay line. When window/2 is an integer
number of source periods, the taps read *the same phase* of the waveform
and their crossfade is invisible — and the average retune ratio is exactly
the phasor's ratio. When it isn't, every crossfade splices a phase jump
into the output, and the jumps do not average away: they bias the pitch.
Period-locking the window is not a quality nicety; it is what makes the
ratio true.

The fix: the window targets the smallest **even multiple** of the detected
period that clears the minimum — 2 periods normally, 4 for high pitches
whose 2 periods would be under 5 ms — so the taps always sit an integer
number of periods apart. The clamp survives only as an outer bound. This is
the cleanest example in the book of a defect no assert-on-internals test
would ever catch: only an oracle — the detector listening to the *output* —
could hear 5 cents.

## Choosing the target

Scale mode: round the detected MIDI to a center note, then scan offsets
−6…+6 for enabled pitch classes, keeping the candidate nearest to the
*fractional* detected pitch (ties resolve to the smaller motion). A tritone
of search radius suffices for any non-empty mask; an empty mask returns no
target, and no target means a zero correction goal — the object never
guesses. MIDI mode is simpler and blunter: the nearest currently-held note
in absolute MIDI space, whatever the distance (clamped to ±12 semitones of
actual correction). `amount` scales the goal before the glide — a fader on
the distance itself.

## The glide

The applied correction chases its goal through a one-pole with time
constant `speed` (0 = assignment, the hard snap), evaluated per sample so
the goal can move every hop while the glide stays silky. The ratio is then
`2^(applied/12)`, computed per sample; the exp2 is cheap and the
alternative — caching with edge cases — is not. The grain window rides its
own 15 ms slew toward the period-locked target, and the detected period
gets a third slew for the PSOLA backend's per-sample period input. Three
small slews, no zippers, no special cases at the joins.

## Three backends behind one seam

`set_backend()` swaps only the last stage; detector, mapper, and glide are
shared state that survives the switch. Both alternate engines are
constructed at `prepare()` (PSOLA sized to the deepest detectable period,
the phase vocoder's FFT scaled to ~21 ms at any rate), so switching
allocates nothing and is safe mid-audio; the incoming engine is cleared to
silence first — a fade-in, not a splice of stale buffers. The ledger, at
48 kHz:

| backend | resynthesis | latency |
|---|---|---|
| grain | period-locked two-tap (in-kernel) | ≈ base delay + window/2, a few ms |
| psola | `tap::dsp::psola` | 2 × 873 + 2 = 1748 samples ≈ 36 ms |
| pvoc | `tap::dsp::pvoc` (+ optional LPC formant trade) | 1024 samples ≈ 21 ms |

The backend-parametrized scenarios feed all three the same 46-cent-sharp
sawtooth and require the same landing (±6 cents at healthy level); a
switching scenario hops between engines mid-signal and requires finite
output and a correction that is still standing at the end. `set_formant()`
forwards to the phase vocoder — PSOLA preserves formants by construction
and the grain engine is waveform-preserving, so the flag deliberately
touches one path.

## Learning the key

The auto-key learner is thirteen doubles and a policy. Every voiced
analysis adds 1 to its pitch class's histogram bin; every analysis
multiplies all twelve bins by a leak chosen so the histogram forgets with
a 60-second time constant. On demand — never on a schedule — the histogram
is scored by Pearson correlation against the published Krumhansl–Kessler
major and minor profiles at all twelve rotations, and the best of the 24
becomes the estimate, with the winning correlation as confidence. A mass
guard withholds any estimate until roughly half a second of voiced material
exists, so silence cannot have an opinion.

The design decision that matters is that the learner is *advisory*:
`autokey_estimate()` reports and `autokey_apply()` adopts, but nothing in
the audio path ever re-aims the targets on its own. This is a UI-safety
argument, not modesty — a corrector that changes its own scale mid-phrase
turns a wrong estimate into a wrong *performance*, and the person at the
patch cannot undo what they never saw happen. Measured: a tonic-weighted
D-major scale scores D major at 0.95 confidence; an A harmonic-minor
melody scores A minor; reset withdraws the estimate.

## The ledger

- **Allocation discipline.** Everything sized at `prepare()`: detector
  frame and ring, both alternate backends, the grain buffer at the maximum
  window. After that the audio path allocates nothing; every setter either
  writes a double, flips a flag, or clears preallocated state.
- **Oracle-based testing.** The scenarios measure the *output's* pitch with
  the independently-certified DspTap detector — the only kind of test that
  caught the period-lock bias — and use sawtooth, not sine, wherever PSOLA
  participates, per its documented material contract.
- **The maxtest.** One assertion runs inside a real Max: unpitched DC in,
  exactly DC out — detector unvoiced, correction zero, complementary
  envelopes summing to one. It pins the whole "never guess" policy at
  unity gain in the shipping binary.
- **What the wrapper adds.** Only plumbing: attribute forwarding, the
  atomic pitch handoff to a scheduler timer for the right outlet, and
  `applykey` writing back through the *attributes* so Max's saved state
  stays the source of truth.

## Checkpoint

A per-sample corrector with a per-hop brain: worst-case geometry bought at
`prepare()` so nothing ever allocates again, unpitched input relaxing to
zero correction, and three slews smoothing every join. The war story is the
period lock — two taps half a window apart are only honest when that
half-window is an integer number of periods, and only an oracle test could
hear the 5-cent lie. Targets are chosen, never invented; the glide is one
pole and one exp2; the backends swap behind a seam that clears to silence;
and the key learner watches, scores, remembers for a minute — and speaks
only when spoken to.
