# Three ways to move a pitch: `yin.h`, `psola.h`, `pvoc.h`

The [user-facing chapter](../tune.md) promised that three interchangeable
engines land the same intonation. This appendix is about why that is hard:
each engine is a *claim about what a pitched sound is*, and each claim fails
somewhere specific and measurable. Two of those failures were found the good
way — as failing tests during development — and both are now pinned as
contracts rather than patched into vagueness.

These three headers live in the shared **DspTap** repository (the same home
as the real FFT that `machine/spectral.md` describes), because a pitch
detector and two shifters are not Max material or even TapTools material —
they are primitives, in the `fft.h` mold: a double-precision golden model,
a float32 embedded profile pinned against it, allocation-free `noexcept`
processing, fixed documented latency, and hot loops kept contiguous as
future Helium/HVX backend seams. Every number below is produced by the
shipping code through DspTap's C ABI in `notebooks/pitchshift.ipynb`, and
gated in `test_yin.cpp` / `test_psola.cpp` / `test_pvoc.cpp`.

## A period is a lag that explains the signal: `yin.h`

Autocorrelation says: a signal is periodic at the lag where it best matches
itself. The trouble is that a harmonic-rich signal matches itself *rather
well* at twice the true period too, and "rather well" wins often enough to
make naive autocorrelation an octave gambler. YIN (de Cheveigné & Kawahara,
2002) replaces "best match" with "smallest failure": a squared **difference**
function

```
d(τ) = Σ (x[j] − x[j+τ])²,   j over the integration window
```

then divides each lag's failure by the running mean of all failures up to
that lag — the *cumulative-mean normalization* — so `d′(0) ≡ 1` and small
lags stop being free wins. The detector takes the **first** lag whose
normalized failure dips under an absolute threshold (0.1 by default),
descends to the local minimum, and refines it with a parabolic fit over the
three surrounding values — the sub-sample step that turns an integer lag
grid into a fractional period.

The contract, measured: worst sine error 0.17 cents across 82–988 Hz
(including deliberately non-integer periods), worst sawtooth error 0.155
cents *with no octave errors* — the trap the normalization exists to
disarm. Noise and silence report unvoiced, and the threshold gates honestly
(a deliberately dirtied sine flips to unvoiced when the threshold is
tightened below its measured aperiodicity).

One honest limit, kept on purpose: *first dip under threshold* scans from
short lags to long, so on synthetic material whose fundamental is nearly
absent — a formant bump with almost no energy at f₀ — a subharmonic lag
that happens to land on an exact integer can dip deeper than the true
period's slightly-off-grid dip, and the detector follows it. The notebook
demonstrates this deliberately and measures such material with a cepstral
oracle instead. Real voices keep enough fundamental that the rule holds;
the failure is documented, not hidden.

## Finding 1 — a shifter that moves everything except the envelope: `psola.h`

TD-PSOLA's move is disarmingly physical. Put an analysis mark every period.
Cut a two-period Hann grain around each mark. To synthesize a new pitch,
lay the grains back down at a *new* spacing — period/ratio — and sum. The
windows are arranged to sum to one at the identity, grains are scaled by
1/ratio to keep the sum flat elsewhere, and synthesis marks are placed with
sub-sample precision (each grain resampled through the same 4-point Hermite
kernel the rest of the family uses) so mark rounding never becomes pitch
jitter. The file adds one real-time honesty: marks come from a free-running
period-synchronous scheduler, not glottal-epoch estimation — the standard
practical simplification — and the caller supplies the period, so detector
and shifter stay independently testable.

Then the finding, told as it happened. The first shift-accuracy test fed the
shifter a pure sine at ratio 2 and got back **0.0000** — silence, from a
correct implementation. Because that *is* what PSOLA does: re-spacing
period-synchronous grains resamples the source's **spectral envelope** at
the new harmonic spacing. A voice's envelope is wide — formants — so the
new harmonics sample it fine, which is exactly the celebrated property:
formants stay put while pitch moves. A pure sine's envelope is a single
spike at f₀, and after an octave up the new harmonic grid (2f₀, 4f₀, …)
contains nothing at f₀ — the output honestly, correctly vanishes. One
property, two faces.

The response was not to patch the algorithm into something less itself. The
shift tests were rewritten onto voice-like material (a normalized
band-limited sawtooth: ±8 cents across ratios 0.5–2.0 at healthy level),
and the pure-tone behavior got its own pinning test,
`PureToneOctaveUpThinsOut`, so that if this property ever *changes*, someone
is forced to explain why. The header now opens with the warning label:
know what PSOLA is; feed it harmonics; for pure tones use a
waveform-preserving shifter.

Latency is fixed at `2·max_period + 2` samples — the price of grains that
must be fully received before they can be laid back down.

## Finding 2 — the naive phase vocoder loses half its level: `pvoc.h`

The textbook pitch shifter looks like four honest lines: STFT with Hann
windows at 4× overlap; per-bin instantaneous frequency from the
frame-to-frame phase increment; remap each analysis bin `k` to synthesis
bin `round(k·ratio)`; accumulate each synthesis bin's phase at its scaled
frequency and inverse-transform. It is in tutorials everywhere. Measured on
a unit sine, it delivers **0.14–0.46 of the input level** at fractional
ratios — more than half the signal simply gone — and its "identity" at
ratio 1 is a sine of the right frequency with the wrong waveform.

Two structural reasons. First, a single partial does not live in one bin;
it lives in a Hann mainlobe *pattern* across four-ish bins, and
`round(k·ratio)` scatters that pattern (220 Hz × 1.5 lands on bins
{5, 6, 8, 9} — nothing at the true target, 7.04). Second, free-running
per-bin phase accumulators destroy the phase relationships across the lobe,
and the overlap-add — which is a resampling filter with real opinions —
partially cancels what remains.

The shipping design is Laroche–Dolson peak-region shifting. Find the
spectral peaks (local maxima over ±2 bins, gated 80 dB below the frame's
strongest bin so the noise floor cannot claim regions). Split the spectrum
into regions around them. Translate each region **rigidly** by an integer
bin offset — the lobe pattern survives intact, phase relationships and
all — and rotate the whole region by a single accumulated per-hop phase ψ.

And here is the bug that cost an afternoon and earned its own comment
block: ψ must accumulate the **full** per-hop frequency difference,
`ψ += 2π·hop·f·(r−1)/N`, not the sub-bin residual left after the integer
shift. An integer bin shift is implemented, in effect, by a modulator
`e^(2πi·shift·n/N)` — but `n` is the *frame-relative* sample index, so that
modulator restarts every frame and contributes nothing to frame-to-frame
phase advance. Subtract the shift from ψ (the "obvious" refinement) and
every frame disagrees with the last about where the shifted partial's phase
should be; the overlap-add quietly shreds the signal. With ψ carrying the
full difference, the measured contracts land: sub-cent frequency accuracy
at every tested ratio, ~0.95 level everywhere, and — because at ratio 1
every shift and every ψ increment is exactly zero and the analysis phases
pass straight through — **exact** waveform identity, one frame late, to
7.8 × 10⁻¹⁶.

### The envelope as a filter: LPC formant preservation

`set_formant(true)` adds the classic source-filter correction. Per analysis
frame: autocorrelate the windowed time frame to lag 48, run Levinson–Durbin
(always in double — an order-48 recursion in float32 is not a place to
economize), and evaluate the prediction polynomial's magnitude over all
bins with one extra FFT of its coefficients — the same transform engine,
one more call. That gives a spectral envelope `E(k) = 1/|A(e^jωk)|`, and
every relocated bin trades envelopes: content moving from bin `k` to bin
`j` is scaled by `E(j)/E(k)` (clamped to ±24 dB so a near-zero envelope
cannot mint gain). The excitation moves; the envelope stays. Measured: a
synthetic 800 Hz formant on a shifted-up-a-fifth voice stays at 800 Hz
with the flag on (band-energy ratio 62:1) and dutifully chipmunks to
1200 Hz with it off. At ratio 1 the correction is `E(k)/E(k)` — exactly
unity — so the identity contract survives the feature untouched. The
method is implemented from the published literature only, which in this
corner of DSP is a policy statement, not just a citation habit.

## The house pattern

All three files repeat the `fft.h` discipline because it keeps paying:
`basic_*<Sample>` templates with `double` as the golden model and `float`
as the embedded profile, cross-precision agreement pinned by tests;
geometry fixed at construction and every buffer allocated there;
`noexcept`, allocation-free processing; latency as a number in the header,
not a vibe; and the expensive inner loops (YIN's difference function above
all) written as plain contiguous arithmetic so a Helium or HVX backend can
slot in behind the same contract with the scalar build remaining the
oracle.

## Checkpoint

A detector that measures failure-to-match instead of match, normalized so
short lags stop cheating, refined below the sample grid — sub-cent, octave-
safe, honest about the one synthetic that fools its first-dip rule. A
grain shifter whose deepest property — resampling the spectral envelope —
is both its celebrated feature and its pure-tone failure, pinned from both
faces. A phase vocoder that works *because* peaks move as rigid families
with one phase register each, carrying the full frequency difference —
since the integer shift's modulator restarts with every frame. And an LPC
envelope trade that lets the excitation move while the mouth stays. Three
claims about what a pitched sound is; three sets of receipts.
