# One STFT, three effects: `fft.h`, `stft.h`, `nr.h`, `spectra.h`

The user-facing chapters for [`tap.nr~`](../nr.md) and
[`tap.spectra~`](../spectra.md) both lean on the same claim: the machinery
is *transparent* — set the effect to do nothing and the output is the input,
exactly, one FFT frame late. All the trust in these objects lives in that
claim, and it is not free: it has to be engineered into the window, the
overlap, and one normalization constant. This appendix builds the stack
bottom-up — the FFT, the scaffold, then the two small effects on top — and
proves the transparency claim rather than asserting it.

## `fft.h`: the transform, owned outright

The kernel repo's law is zero dependencies — plain C++17, standard library
only. So the FFT is in-house: an in-place iterative radix-2 Cooley–Tukey
in `fft::transform(re, im, inverse)`, about forty lines. It is also *one*
copy by design: the identical routine previously lived, byte for byte,
inside `conv_engine` (`tap.convolve~`), `tap.nr~`, and `tap.spectra~`, and
was consolidated at the kernel split so it is maintained and tested once —
`tests/fft_test.cpp` is that single test point. The two halves:

- **Bit-reversal permutation.** An iterative FFT consumes its input in
  bit-reversed index order; the first loop swaps each element `i` with its
  bit-reversed partner `j`, maintaining `j` incrementally (the carry-ripple
  idiom) rather than reversing bits per index; the `i < j` guard swaps each
  pair once.
- **Butterfly stages.** For each length `len` = 2, 4, … N, combine pairs of
  half-blocks with twiddle factors e^(∓2πik/len). The twiddle is advanced by
  a complex-multiply recurrence (`cwr`, `cwi` rotated by (`wr`, `wi`)) — one
  `cos`/`sin` per stage instead of per butterfly. A recurrence accumulates
  rounding, but in double precision over these sizes it is far inside the
  pinned tolerance.

The scaling convention is asymmetric and load-bearing: **forward is
unscaled, inverse divides by N**, so forward-then-inverse is the identity.
Every claim is pinned in `fft_test.cpp`: the forward transform matches a
naive O(N²) DFT to 10⁻⁹ for N ∈ {2, 4, 8, 16, 64, 256}; the round trip
reconstructs random complex input to 10⁻⁹ at N = 128; a unit impulse
transforms to an exactly flat unit spectrum; and a real cosine at bin 3 of
32 lands N/2 on bins 3 and 29 — fixing the sign convention (forward kernel
e^(−i…)) and the conjugate-bin layout the effects below depend on.

## `stft.h`: the scaffold, and the COLA proof

`stft` is the overlap-add machinery shared verbatim by both effects: Hann
window, fixed 4× overlap (`m_hop = m_fftsize / m_overlap`), a circular
input buffer, a circular output *accumulator*, and a per-sample pump.
`process()` takes the effect as a callable — `op(re, im, N)` mutates the
N-point spectrum in place between the forward and inverse transforms; the
only difference between `tap.nr~` and `tap.spectra~` is that lambda. The
window, built in `configure()`:

```text
m_window[k] = 0.5 − 0.5·cos(2π·k / m_fftsize)        k = 0 … N−1
```

— the *periodic* Hann (denominator N, not N−1), which is what makes the
overlap sums below exactly constant rather than rippling. The window is
applied **twice** per frame: once at analysis (`m_re[k] = inbuf·window[k]`)
and once at synthesis (`outbuf += m_re[k]·m_window[k]·m_norm`). With an
identity op, the inverse transform returns the windowed frame exactly (the
FFT round trip is the identity), so each input sample x is delivered to the
output through every frame that covers it, weighted by w² each time:

```text
y[t] ∝ x[t−N] · Σₘ w²(n − mH)          H = N/4, four frames cover each n
```

Perfect reconstruction therefore requires the shifted **window-squared**
sum to be constant — the COLA (constant overlap-add) condition for
double-windowing. For the periodic Hann at 4× overlap it is, and the
constant has a closed form. Expand w²:

```text
w²(θ) = (0.5 − 0.5·cos θ)² = 0.375 − 0.5·cos θ + 0.125·cos 2θ     θ = 2πn/N
```

A hop of N/4 advances θ by π/2. Over four hops, the cos θ terms are four
quarter-turns of a phasor — they sum to zero; the cos 2θ terms advance by π
per hop and cancel in adjacent pairs. What survives is the constant:

```text
Σₘ w²(n − mH) = 4 × 0.375 = 3/2         for every n
```

The code does not hard-code 3/2. `configure()` overlap-adds `overlap`
copies of `m_window[k]²` around a circular buffer and reads the value at
`cola[m_fftsize/2]`, setting `m_norm = 1/c` — for Hann at 4×,
`m_norm = 2/3` (verified numerically: the computed sum is 1.5 to within
10⁻¹⁵ at every index, so reading the midpoint is safe). Computing it keeps
the scaffold correct for any window/overlap it might grow.

**Latency is exactly N, and here is the accounting.** The pump writes
`in[i]` into `m_inbuf[m_pos]`, reads the output from `m_outbuf[m_pos]`,
zeroes that slot, advances, and fires a frame every `m_hop` samples. When a
frame fires, its index k holds input sample x[t₀ − (N−1) + k] (t₀ the
newest sample, at k = N−1), and synthesis writes index k into
`m_outbuf[(m_pos + k) % N]`, which the pump reads k+1 samples later. Output
time minus input time:

```text
(t₀ + 1 + k) − (t₀ − N + 1 + k) = N          for every k, every frame
```

A frame cannot be transformed until it has filled — that is the whole cost,
and why `latency()` simply returns `m_fftsize`. Both test suites pin the
full contract at once: with a do-nothing effect (`nr` at `threshold 0`,
`spectra` at `remap 1`), `out[t] == in[t − N]` to within 10⁻⁹ on broadband
noise for all t ≥ 2N (the run-in covers frames that still window in
zeros). This is the "transparent machinery" sentence in both user
chapters, with its provenance attached: FFT round trip (pinned) × COLA
constant (derived) × exact-N pipeline (derived).

## `nr.h`: the gate, precisely

The spectral op is `gate()`, and its knee is short enough to quote in full
as math. Per bin k:

```text
mag  = √(re[k]² + im[k]²) · (2/N)
gain = 1                                if thr ≤ 0 or mag ≥ thr
gain = (mag / thr)^slope                if mag < thr   (slope ≤ 0 → 1)
re[k] *= gain;  im[k] *= gain
```

The 2/N puts `mag` on a sinusoid-amplitude scale (a real tone of amplitude
A puts A·N/2 in each of its two conjugate bins; ×2/N recovers A). One
honest calibration note: the frame is Hann-windowed *before* the FFT, and
the Hann's coherent gain is 1/2 — a bin-centred sine of amplitude A
actually measures `mag = A/2` (verified numerically: A = 0.8 reads 0.400),
with leakage in the adjacent bins. `threshold` is a linear amplitude on the
windowed scale; a full-scale sine sits near 0.5, not 1.0.

The knee is a **downward expander per bin**. Take logs of the gain law
below threshold:

```text
L_out − L_thr = (1 + slope) · (L_in − L_thr)         in dB
```

Every dB below the threshold becomes (1 + slope) dB below it: `slope 0` is
unity (bypass by another name — the code special-cases it), the default
`slope 2` is a 1:3 expander, and slope → ∞ approaches a hard gate. Both
`re` and `im` are scaled by the same real gain, so phase is untouched — the
gate reshapes magnitude only.

Two structural notes. First, the loop runs over **all N bins**, mirror
half included, with no symmetry bookkeeping — and needs none: the input
frame is real, so its spectrum is Hermitian, magnitudes are symmetric
(`mag[N−k] = mag[k]`), conjugate pairs get the same real gain, and
Hermitian symmetry survives the op — the inverse stays real for free.
(Hold that thought; `spectra` is not so lucky.)
Second, the per-frame independence of the gain decision is exactly where
**musical noise** comes from: a bin whose magnitude hovers near `thr` flips
between pass and heavy attenuation frame by frame, and each isolated pass
is one Hann-windowed near-sinusoid burst, milliseconds long — a chirp.
Scattered over time and frequency, chirps sound like water. That is not a
bug in the code; it is the knee's steepness meeting the frame rate, which
is why the user chapter's cure is a gentler `slope`, not a different
implementation.

`tests/nr_test.cpp` pins the three defining behaviors: gate open
(`threshold 0`) reconstructs noise to 10⁻⁹ delayed one frame; a quiet
bin-centred tone (amplitude 0.05 against `threshold 0.5`, `slope 4`) leaves
a steady-state tail under 5 % of the input RMS; a loud tone (0.8 against
0.01) passes with RMS within 2 % of the input.

## `spectra.h`: the remap, and why the mirror is not optional

The op builds a new spectrum over the lower half:

```text
src      = lround(k · m_remap)                k = 0 … N/2
m_ore[k] = re[src], m_oim[k] = im[src]        if 0 ≤ src ≤ N/2, else 0
```

then forces DC and Nyquist real (`m_oim[0] = m_oim[half] = 0`), mirrors —
`m_ore[N−k] = m_ore[k]`, `m_oim[N−k] = −m_oim[k]` for 0 < k < half — and
copies the scratch back over `re`/`im`.

The mirror is provable necessity, not tidiness. A real signal's DFT
satisfies `X[N−k] = conj(X[k])`, and only Hermitian spectra invert to real
signals. The remap fills the lower half by an arbitrary rule and touches
nothing above Nyquist — the upper half still holds the *input's* bins, so
the assembled spectrum is not Hermitian for any remap ≠ 1, and its inverse
transform is genuinely complex. And the scaffold's synthesis reads `m_re`
only — the imaginary part of the inverse is **discarded**. Keeping
Re(IDFT(Y)) is algebraically inverse-transforming the Hermitian *average*
½(Y[k] + conj(Y[N−k])): without the mirror, the delivered effect would be
an uncontrolled blend of the remapped lower half and the untouched upper
half, half the intended signal shunted silently into the discarded
imaginary part. The mirror makes the spectrum Hermitian *by construction*,
so the inverse is exactly real and "keep the real part" loses nothing. DC
and Nyquist are their own mirror images (k = N−k), so conjugate symmetry
forces them real — hence the two explicit zeroes.

(The remap cannot run in place — output bin k may read a bin already
overwritten — hence the `m_ore`/`m_oim` scratch, allocated in
`configure()`.) And the energy honesty: a remap is not a permutation, so Parseval is
deliberately broken. For `remap < 1`, `lround(k · remap)` is non-strictly
increasing — several output bins read the *same* input bin, duplicating
its energy. For `remap > 1` it strides — input bins are skipped, and every
output bin above N/(2·remap) reads beyond Nyquist and is zeroed,
discarding the input's top octaves. Neither direction conserves energy,
and neither is meant to: the reference page has called this an
"ultra-non-linear effect" since 2002; the kernel implements the rule, not
a transform.

`tests/spectra_test.cpp` pins the two anchors: `remap 1` reconstructs noise
to 10⁻⁹ delayed one frame (the identity copies the lower half of an
already-Hermitian spectrum, and the mirror rebuilds the upper half it
started with); `remap 2` moves a tone at input bin 16 to output bin 8 —
`lround(8 · 2) = 16` — verified by FFT-ing a frame-aligned slice of the
steady-state output and requiring the peak at bin 8.

## The engineering ledger

- **The effect is a template parameter, not a base class.** `stft::process`
  takes `SpectralOp&&` and calls it once per hop; each effect passes a
  capturing lambda. No virtual dispatch in the audio path, full inlining.
- **Why the vocoder is not the third client.** `tap.vocoder~` is
  time-domain on purpose — zero latency, no frame, `prepare(sr)` instead of
  `configure(fftsize)` — see [its own chapter](vocoder.md). The spectral
  set accepts latency as a cost model; the vocoder's whole point is not
  paying it.
- **Allocation at `configure()` only.** Window, in/out rings, FFT scratch,
  and (for `spectra`) the remap scratch are all sized there; `process()`
  is allocation-free. `reset()` flushes the running buffers without
  reallocating or touching the window — commented in the code as safe from
  a message handler, which is exactly how the wrappers use it.
- **`fftsize` is the one shared dial**, and the scaffold makes its price
  explicit: resolution (bin spacing sr/N), smearing (a per-bin decision
  spreads over a whole frame), and latency (`latency()` returns N so the
  wrapper can report a true number to the host).
- **One FFT, tested once.** The round-trip and DFT-reference pins in
  `fft_test.cpp` are what let this chapter treat "forward then inverse is
  the identity" as a premise everywhere above.

## Checkpoint

The stack is three honest layers. The FFT is forty owned lines with an
asymmetric scaling convention, pinned against a naive DFT. The scaffold
windows twice, so reconstruction needs the shifted w² sum to be constant —
for periodic Hann at 4× overlap it is exactly 3/2, measured rather than
assumed, and the pipeline delays every sample by exactly N. On top, each
effect is one spectral op: `nr` a per-bin 1:(1+slope) downward expander
whose real gain preserves Hermitian symmetry for free; `spectra` a
bin-index rule violent enough that reality — a real output — must be
restored by explicit mirror. Transparency at the neutral setting is the
theorem the whole stack exists to satisfy; both suites pin it at 10⁻⁹.
