#!/usr/bin/env python3
"""Generate the measured figures for the overdrive book chapters.

Drives the *shipping* kernel (overdrive.h) through the C ABI via the notebooks'
ctypes bridge — the same rule as the verification notebooks: figures are
measurements of the real DSP, never illustrations of what it should do. The
companion notebook (notebooks/overdrive.ipynb) carries the same measurements
with commentary; this script renders the book-styled SVGs.

Regenerate after a kernel behavior change:

    python3 book/figures/overdrive.py     # writes book/src/images/overdrive/*.svg

Colors: the house categorical hues (notebooks/taptools_py.py PALETTE), with the
amber snapped darker (#efb118 -> #b8890f) so the triple passes the print/CVD
lightness-band and separation checks on a light page. Every multi-series figure
carries direct labels, so identity never rides on color alone.
"""

import pathlib
import sys

import numpy as np
import matplotlib

matplotlib.use("Agg")
import matplotlib.pyplot as plt

sys.path.insert(0, str(pathlib.Path(__file__).resolve().parents[2] / "notebooks"))
import taptools_py as tap

OUT = pathlib.Path(__file__).resolve().parents[1] / "src" / "images" / "overdrive"
OUT.mkdir(parents=True, exist_ok=True)

BLUE, AMBER, RED = "#4269d0", "#b8890f", "#ff725c"
INK, MUTED = "#1a1a1a", "#666666"

plt.rcParams.update({
    "figure.dpi": 96, "figure.figsize": (7.2, 3.1),
    "svg.fonttype": "none", "font.family": "sans-serif", "font.size": 9.5,
    "axes.grid": True, "grid.alpha": 0.22, "grid.linewidth": 0.5,
    "axes.spines.top": False, "axes.spines.right": False,
    "axes.edgecolor": MUTED, "axes.labelcolor": INK,
    "xtick.color": MUTED, "ytick.color": MUTED,
    "axes.titlesize": 10, "axes.titlecolor": INK,
    "lines.linewidth": 2.0, "legend.frameon": False, "legend.fontsize": 8.5,
})

fs = 48000


def tone(f, amp, seconds):
    t = np.arange(int(seconds * fs)) / fs
    return amp * np.sin(2 * np.pi * f * t)


def level(y, f):
    n = len(y)
    t = np.arange(n) / fs
    win = np.hanning(n)
    return 2.0 * np.abs(np.dot(y * win, np.exp(-2j * np.pi * f * t))) / win.sum()


def gain_db(f, amp=5e-4, settle=0.25, meas=0.5, **params):
    o = tap.Overdrive(fs, **params)
    x = tone(f, amp, settle + meas)
    y = o.process(x)
    n0 = int(settle * fs)
    return 20 * np.log10(level(y[n0:], f) / level(x[n0:], f))


def spectrum_db(y, ref):
    win = np.hanning(len(y))
    mag = np.abs(np.fft.rfft(y * win)) / (win.sum() / 2)
    freqs = np.fft.rfftfreq(len(y), 1 / fs)
    return freqs, 20 * np.log10(np.maximum(mag / ref, 1e-9))


def save(fig, name):
    path = OUT / name
    fig.savefig(path, bbox_inches="tight")
    plt.close(fig)
    print(f"wrote {path}")


# -- 1. the headline: gain tilt grows with drive ---------------------------------------

freqs = np.geomspace(30, 12000, 22)
fig, ax = plt.subplots()
for drive, color in ((0.0, BLUE), (0.5, AMBER), (0.9, RED)):
    g = np.array([gain_db(f, drive=drive) for f in freqs])
    ax.semilogx(freqs, g, color=color, label=f"drive {drive}")
    ax.annotate(f"drive {drive}", (freqs[-1] * 1.06, g[-1]), color=color,
                fontsize=8.5, va="center")
ax.set(xlabel="frequency (Hz)", ylabel="small-signal gain (dB)", xlim=(28, 26000),
       title="The loop's tilt grows with drive: bass pinned, mids take the gain")
ax.legend(loc="upper left")
save(fig, "tilt.svg")

# -- 2. the transfer never flattens ----------------------------------------------------

amps = np.geomspace(0.05, 1.0, 12)
fig, ax = plt.subplots(figsize=(5.4, 3.1))
for drive, color in ((0.0, BLUE), (0.5, AMBER), (1.0, RED)):
    peaks = []
    for amp in amps:
        o = tap.Overdrive(fs, drive=drive)
        y = o.process(tone(500, amp, 0.75))
        peaks.append(np.abs(y[int(0.25 * fs):]).max())
    ax.plot(amps, peaks, "o-", ms=3.5, color=color, label=f"drive {drive}")
    ax.annotate(f"drive {drive}", (amps[-1] * 1.03, peaks[-1]), color=color,
                fontsize=8.5, va="center")
ax.set(xlabel="input peak", ylabel="output peak", xlim=(0, 1.18),
       title="Peak transfer at 500 Hz: rising at every drive, never a plateau")
ax.legend(loc="upper left")
save(fig, "transfer.svg")

# -- 3. even harmonics vs asymmetry ----------------------------------------------------

f0 = 220.5
fig, axes = plt.subplots(1, 2, figsize=(7.2, 3.1), sharey=True)
for ax, asym in ((axes[0], 0.0), (axes[1], 0.6)):
    o = tap.Overdrive(fs, drive=0.6, asymmetry=asym)
    y = o.process(tone(f0, 0.5, 1.5))[int(0.5 * fs):]
    fr, db = spectrum_db(y, ref=level(y, f0))
    ax.plot(fr, db, color=BLUE, lw=0.9)
    for h in (2, 4):  # flag the even harmonics
        ax.annotate(f"H{h}", (h * f0, 20 * np.log10(max(level(y, h * f0) / level(y, f0), 1e-8)) + 6),
                    color=RED, fontsize=8.5, ha="center")
    ax.set(xlim=(0, 2000), ylim=(-100, 8), xlabel="frequency (Hz)",
           title=f"asymmetry {asym}")
axes[0].set_ylabel("level re fundamental (dB)")
fig.suptitle("Asymmetry turns on the even series (220.5 Hz, drive 0.6)", fontsize=10, color=INK, y=1.04)
save(fig, "harmonics.svg")

# -- 4. body voicing -------------------------------------------------------------------

freqs_b = np.geomspace(25, 16000, 24)
fig, ax = plt.subplots()
# The curves converge at the top of the band, so the direct labels sit at the
# low end where the three are far apart (~15 dB of separation at 40 Hz).
for body, color, label in ((-1.0, BLUE, "body −1 (full lows)"),
                           (0.0, AMBER, "body 0"),
                           (1.0, RED, "body +1 (tight, mid push)")):
    g = np.array([gain_db(f, drive=0.3, body=body) for f in freqs_b])
    ax.semilogx(freqs_b, g, color=color, label=label)
    ax.annotate(label, (freqs_b[0] * 1.15, g[0]), color=color, fontsize=8.5, va="bottom")
ax.set(xlabel="frequency (Hz)", ylabel="small-signal gain (dB)", xlim=(23, 17000),
       title="body: the voicing control (drive 0.3)")
ax.legend(loc="lower right")
save(fig, "body.svg")

# -- 5. alias floor: 1x vs the default 4x (machine chapter) ----------------------------

f0a = 5001.0
alias_f = fs - 7 * f0a
fig, ax = plt.subplots()
# 1x behind, 4x on top with slight transparency: every red peak visible above
# the blue mass is alias energy the default 4x removes.
for os_, color, alpha in ((1, RED, 1.0), (4, BLUE, 0.88)):
    o = tap.Overdrive(fs, drive=0.9, asymmetry=0.2, oversample=os_)
    y = o.process(tone(f0a, 0.6, 1.5))[int(0.5 * fs):]
    fr, db = spectrum_db(y, ref=level(y, f0a))
    ax.plot(fr, db, lw=0.8, color=color, alpha=alpha, label=f"oversample {os_}×")
ax.axvline(alias_f, color=MUTED, lw=0.8, ls="--")
ax.annotate("folded H7\n(12993 Hz)", (alias_f, 2), color=MUTED, fontsize=8.5,
            ha="center", va="bottom")
ax.set(xlim=(0, fs / 2), ylim=(-110, 14), xlabel="frequency (Hz)",
       ylabel="level re fundamental (dB)",
       title="5001 Hz at drive 0.9: the alias floor at 1× vs the default 4×")
ax.legend(loc="upper right")
save(fig, "alias.svg")

# -- 6. the shaper vs its rejected alternatives (machine chapter) ----------------------

u = np.linspace(-4, 4, 400)
fig, ax = plt.subplots(figsize=(5.4, 3.3))
curves = (
    (u / np.sqrt(1 + u * u), BLUE, "u/√(1+u²)  (chosen)", 2.4),
    (np.tanh(u), AMBER, "tanh  (libm cost)", -1.5),
    (np.clip(u, -1, 1), RED, "hard clip  (aliases)", 1.2),
)
for y, color, label, lx in curves:
    ax.plot(u, y, color=color, lw=2.0, label=label)
ax.annotate("u/√(1+u²)", (2.5, 2.5 / np.sqrt(1 + 2.5**2) - 0.16), color=BLUE, fontsize=8.5)
ax.annotate("tanh", (-2.4, np.tanh(-2.4) - 0.18), color=AMBER, fontsize=8.5, ha="right")
ax.annotate("hard clip", (1.35, 1.06), color=RED, fontsize=8.5)
ax.set(xlabel="input u", ylabel="shape(u)", ylim=(-1.35, 1.35),
       title="The static curve: smooth, monotonic, asymptotic — no plateau, no seam")
ax.legend(loc="upper left")
save(fig, "shape.svg")

print("done")
