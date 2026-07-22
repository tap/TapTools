# Plan — the pitch-correction chapters

> **Status: drafted.** All three chapters are written and live in `src/` per the placement
> below (2026-07-22). This file remains as the drafting record, the plans-directory way.

Planning document for the *Tools on Tap* chapters covering the 2026-07 pitch-correction work
(`tap.tune~`, `taptools/tune.h`, and the DspTap primitives `yin.h` / `psola.h` / `pvoc.h`).
Three chapters: one user-facing, two machine appendices. This file is the outline to draft
from; it is not part of the built book.

Every measured claim below already exists as an executed notebook cell or a pinned test —
each section lists its evidence so the chapters keep the book's "measured, not remembered"
promise without new lab work.

## Placement in SUMMARY.md

Insert a new part before the machine part (which becomes Part VII):

```md
# Part VI — Staying in tune

- [The note you meant](tune.md)

# Part VII — The machine, file by file
  ...existing entries...
- [Three ways to move a pitch: yin.h, psola.h, pvoc.h](machine/pitch.md)
- [The nearest allowed note: tune.h](machine/tune.md)
```

The two machine entries slot after `machine/seq.md`, keeping the part's file-by-file order
roughly chronological. `machine/pitch.md` covers headers that live in the DspTap repo — the
same cross-repo situation `machine/spectral.md` already handles for `fft.h`; follow its
convention (name the repo once, link the file, treat it as part of the machine).

---

## Chapter 1 (user-facing) — *The note you meant* (`src/tune.md`)

The chapter sells one image: the distance between the note you sang and the note you meant,
and a single time constant that decides how honestly to close it. Voice: the person patching
at 11 pm; every knob a trade.

Outline:

1. **The effect, and where it came from.** Real-time monophonic correction: detect, snap,
   glide. One paragraph of history done carefully: the foundational 1998 patent expired in
   2018, so the classic pipeline is public domain — which is why the field exists; the famous
   product name is a live trademark, which is why this object is called `tap.tune~`; and
   polyphonic per-note editing remains fenced (and out of scope — this is a monophonic tool
   by design, not by omission). Keep it two sentences per fact, no legal advice.
2. **One knob that is the instrument: `speed`.** Hard snap (0 ms) is the quantize effect;
   ~20 ms is invisible repair; 200 ms only leans. Figure: the three-glide pitch-track plot.
   *Evidence: `notebooks/tune.ipynb` §1; kernel scenario "retune speed sets the glide".*
3. **Telling it what's allowed.** Key + scale, the twelve per-note enables (`notes`), and
   MIDI mode (the held-note targeting that turns the corrector into a hard harmonizer-ish
   instrument). The empty-mask / no-notes-held behavior: no target, no correction — the
   object never guesses. *Evidence: kernel scenarios for mask/MIDI targeting.*
4. **Three engines, one corrector: `backend`.** The user-level trade table (grain: cheapest,
   waveform-preserving, default; psola: voice, formants for free; pvoc: dense material,
   one-frame latency) and the latency ledger in milliseconds. All three land the same
   intonation — figure: the vibrato-voice pitch-track overlay. Point forward to the machine
   chapter for *why*. *Evidence: `tune.ipynb` §2 (all backends settle at 220.00 Hz).*
5. **Keeping the singer's mouth: `formant`.** When corrections are small it doesn't matter;
   when MIDI mode commands a fifth, it does. Figure: the spectra overlay (bump stays at 960).
   One honest sentence each on psola (needs no flag) and grain (unaffected).
   *Evidence: `tune.ipynb` §3; pvoc formant tests.*
6. **Letting it find the key: `autokey`.** Learning-only by design — the estimate never
   flips your targets mid-phrase; `getkey` asks, `applykey` adopts. The ~1-minute memory and
   the half-second warm-up, stated as behavior ("it forgets old sections about as fast as
   you move on from them"). *Evidence: kernel autokey scenarios (D major at 0.95).*
7. **The right outlet.** `pitch <midi> <hz>` every `interval` ms — the free tuner display;
   drive a number box, a scope, or your own harmonizer logic.
8. **When it is the wrong tool.** Chords (monophonic detector — and the polyphonic fence);
   drums and speech consonants (unpitched input passes with a mild grain coloration — say
   so); octave-plus creative shifts (reach for `tap.shift~`/`tap.pitchaccum~` instead).
9. **Companion material.** Help patcher, the maxtest, both notebooks, the machine chapters.

## Chapter 2 (machine) — *Three ways to move a pitch: `yin.h`, `psola.h`, `pvoc.h`*
(`src/machine/pitch.md`)

The appendix earns the two findings. Structure it as three short essays sharing one moral:
each algorithm is a claim about what a pitched sound *is*, and each claim fails somewhere
measurable.

1. **A period is a lag that explains the signal: `yin.h`.** The difference function, the
   cumulative-mean normalization (why τ=0 stops being a trap), the absolute threshold, the
   parabolic sub-sample refinement. The contract numbers (worst sine error 0.17 cents; saw
   octave-clean). The honest limit: first-dip-below-threshold on missing-fundamental
   material picks subharmonics — shown deliberately with the notebook's formant-bump
   synthetic, and why real voices don't trigger it. *Evidence: `test_yin.cpp`,
   `pitchshift.ipynb` §1 (+ the §4 oracle footnote).*
2. **Finding 1 — a shifter that moves everything except the envelope: `psola.h`.** Grains,
   marks, the 1/ratio window sum, sub-sample Hermite placement. Then the finding, told as it
   happened: the octave-up sine "failure" (0.0000 peak) that is the algorithm working as
   published — PSOLA resamples the spectral envelope, which is one property wearing two
   faces (formant preservation on voice, silence on the tone the envelope abandoned). The
   pinned `PureToneOctaveUpThinsOut` test as the moral: document the property, don't patch
   it. *Evidence: `test_psola.cpp`, `pitchshift.ipynb` §2.*
3. **Finding 2 — the naive phase vocoder loses half its level: `pvoc.h`.** The textbook
   `round(k·ratio)` remap measured (0.14–0.46 of unit level at fractional ratios) and the
   two structural reasons; Laroche–Dolson peak-region rigid translation; and the subtle bug
   worth a whole section: the integer shift's modulator is frame-relative, so ψ must
   accumulate the FULL `f·(r−1)` — subtracting the integer part desynchronizes the
   overlap-add. End on the contracts: ~0.95 level everywhere, sub-cent accuracy, identity at
   ratio 1 exact to 7.8e-16. *Evidence: `test_pvoc.cpp`, `pitchshift.ipynb` §3.*
4. **The envelope as a filter: LPC formant preservation.** Autocorrelation method,
   Levinson–Durbin in double, |A| via one FFT of the prediction polynomial, the
   `env(target)/env(source)` per-bin trade, the boost clamp, and the identity invariance
   (exactly unity when nothing moves). *Evidence: pvoc formant tests, `pitchshift.ipynb` §4.*
5. **The house pattern.** One paragraph: golden double model, float32 profile, fixed
   contracts, hot loops as future MVE/HVX backend seams — the `fft.h` inheritance.

## Chapter 3 (machine) — *The nearest allowed note: `tune.h`* (`src/machine/tune.md`)

The composition appendix: how detector, mapper, glide, and three resynthesis engines share
one allocation-free object.

1. **The pipeline and its clock.** Per-sample process, per-hop (~5.3 ms) analysis, the
   worst-case-geometry-at-prepare trick that keeps every setter real-time safe, and the
   analysis cost spike stated in numbers (and why it fits a 64-sample vector budget).
2. **The period lock, told as a bug hunt.** The 5-cent bias the oracle tests caught, the
   isolation experiment (window = exact two periods → −0.06 cents; fixed-ms clamp → +5.4),
   and the even-multiple rule that keeps the taps an integer number of periods apart. This
   is the chapter's centerpiece — it is the cleanest example in the book of a *musical*
   defect that only an oracle-based test could pin. *Evidence: the tune_test chromatic
   scenarios; the debug numbers are in the git history of this branch.*
3. **Choosing the target.** Semitone geometry of the mask search (±6 always suffices),
   MIDI-mode nearest-held-note, the correction clamp, `amount` as a fader on the distance.
4. **The glide.** One-pole on applied semitones, snap at zero, unpitched relaxation toward
   no-correction — and why the *window* holds while correction relaxes.
5. **Three backends behind one seam.** What switching clears and why (silence, not splice);
   the per-backend period plumbing (psola's slewed period input); the latency ledger.
6. **Learning the key.** The leaky histogram (60 s memory), the mass guard, Pearson vs the
   Krumhansl–Kessler profiles, and the design decision that it never acts on its own —
   stated as a UI-safety argument, not modesty. *Evidence: autokey scenarios.*
7. **The ledger.** The test strategy paragraph the machine chapters share: the DspTap yin as
   oracle, saw-not-sine for PSOLA fairness, notebook cross-checks, and what the maxtest pins
   in a real Max.

---

## Notes for drafting

- Chapter titles follow the book's voice (image first, filename in the machine part).
  Alternates considered for ch. 1: "Staying in tune", "The snap and the glide".
- The IP paragraph in ch. 1 stays engineering-orientation prose, mirroring REVIVAL.md §12;
  the ship-gate (attorney freedom-to-operate pass) belongs there, one sentence, no drama.
- Figures come straight from the two executed notebooks; regenerate rather than screenshot.
- Cross-repo linking: `machine/pitch.md` names the DspTap repo once at the top (the
  `machine/spectral.md` precedent) and links files on GitHub thereafter.
