#!/usr/bin/env python3
"""Generate the measured figures for the Eno-family book chapters.

Drives the *shipping* kernels (discreet.h, airport.h, garden.h) through the
C ABI via the notebooks' ctypes bridge — the same rule as the verification
notebooks: figures are measurements of the real DSP, never illustrations of
what it should do. The companion notebooks (notebooks/discreet.ipynb,
airport.ipynb, garden.ipynb) carry the same measurements with commentary;
this script renders the book-styled SVGs.

Regenerate after a kernel behavior change:

    python3 book/figures/eno.py     # writes book/src/images/{discreet,airport,garden}/*.svg

Colors: the house categorical hues (notebooks/taptools_py.py PALETTE), with
the amber snapped darker (#efb118 -> #b8890f) so pairs pass the print/CVD
lightness-band and separation checks on a light page. Every multi-series
figure carries direct labels, so identity never rides on color alone.
"""

import pathlib
import sys

import numpy as np
import matplotlib

matplotlib.use("Agg")
import matplotlib.pyplot as plt

sys.path.insert(0, str(pathlib.Path(__file__).resolve().parents[2] / "notebooks"))
import taptools_py as tap

IMAGES = pathlib.Path(__file__).resolve().parents[1] / "src" / "images"

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

fs = 48000.0


def out_dir(name):
    d = IMAGES / name
    d.mkdir(parents=True, exist_ok=True)
    return d


def tone(x, f):
    n = np.arange(x.size)
    return 2.0 * np.abs(np.dot(x, np.exp(-2j * np.pi * f * n / fs))) / x.size


def generation_loss():
    """discreet: per-pass two-tone decay vs. the analytic wear transfer."""
    m = tap.Discreet(fs, 8.0, smooth_ms=0, wow=(0, 0), flutter=(0, 0), mix=100,
                     input_level=1.0, loop_seconds=0.25, regen=0.9, drive=0.0,
                     darken_hz=2000.0)
    f_hi, f_lo = 6000.0, 300.0
    n_burst = int(0.1 * fs)
    tt = np.arange(n_burst) / fs
    x = np.zeros(int(1.6 * fs))
    x[:n_burst] = 0.4 * np.sin(2 * np.pi * f_hi * tt) + 0.4 * np.sin(2 * np.pi * f_lo * tt)
    y = m.process(x)

    def wear_gain(f, cutoff):
        w = 2 * np.pi * f / fs
        a = 1.0 - np.exp(-2 * np.pi * cutoff / fs)
        ejw = np.exp(-1j * w)
        lp = np.abs(a / (1 - (1 - a) * ejw))
        r, nm = 0.999, (1 + 0.999) / 2
        return lp * np.abs(nm * (1 - ejw) / (1 - r * ejw))

    loop = int(0.25 * fs)
    passes = np.arange(1, 6)
    hi = [tone(y[k * loop: k * loop + n_burst], f_hi) for k in passes]
    lo = [tone(y[k * loop: k * loop + n_burst], f_lo) for k in passes]
    pred_hi = hi[0] * (0.9 * wear_gain(f_hi, 2000.0)) ** (passes - 1)
    pred_lo = lo[0] * (0.9 * wear_gain(f_lo, 2000.0)) ** (passes - 1)

    fig, ax = plt.subplots()
    ax.semilogy(passes, pred_lo, "-", color=AMBER, lw=1.2, alpha=0.7)
    ax.semilogy(passes, lo, "s", color=AMBER, ms=6)
    ax.semilogy(passes, pred_hi, "-", color=BLUE, lw=1.2, alpha=0.7)
    ax.semilogy(passes, hi, "o", color=BLUE, ms=6)
    ax.text(2.1, lo[1] * 1.45, "300 Hz — below the corner", color=AMBER)
    ax.text(1.6, hi[1] * 0.5, "6 kHz — above it", color=BLUE)
    ax.set_xticks(passes)
    ax.set_xlabel("pass through the loop")
    ax.set_ylabel("tone level")
    ax.set_title("generation loss, measured (points) vs. regen · |H_wear| (lines)")
    fig.savefig(out_dir("discreet") / "generation-loss.svg", bbox_inches="tight")
    plt.close(fig)


def raster():
    """airport: return raster of two incommensurate loops; the lcm marked."""
    b = tap.Airport(fs, 2.0, smooth_ms=0, lengths=[0.5, 0.625], pans=[-1.0, 1.0])
    click = np.zeros(1)
    click[0] = 1.0
    for i in (0, 1):
        b.record(i, True)
        b.process(click)
        b.record(i, False)
    yl, yr = b.process(np.zeros(int(7.5 * fs)))
    hits_a = np.flatnonzero(yl > 0.5) / fs
    hits_b = np.flatnonzero(yr > 0.5) / fs

    fig, ax = plt.subplots(figsize=(7.2, 2.3))
    ax.eventplot([hits_a, hits_b, np.concatenate([hits_a, hits_b])],
                 colors=[BLUE, AMBER, MUTED], lineoffsets=[2, 1, 0], linelengths=0.75)
    for k in (1, 2):
        ax.axvline(2.5 * k, color=RED, lw=1.0, ls=":")
    ax.text(2.5, 2.72, "composite period: 2.5 s (the lcm)", color=RED, ha="center")
    ax.set_yticks([2, 1, 0])
    ax.set_yticklabels(["loop A · 0.5 s", "loop B · 0.625 s", "the sum"])
    ax.set_xlabel("time (s)")
    ax.set_ylim(-0.6, 3.0)
    ax.grid(axis="y", alpha=0)
    fig.savefig(out_dir("airport") / "raster.svg", bbox_inches="tight")
    plt.close(fig)


def staircase():
    """garden: the decay-0.5 return staircase, retiring below the floor."""
    g = tap.Garden(fs, smooth_ms=0, idle_seconds=0, spread=0, loop_seconds=0.5,
                   decay=0.5, floor=0.05, bell=(0.002, 0.05, 1.0), scale=0)
    g.note(69, 0.8)
    y, _ = g.process(int(3.5 * fs))  # spread 0: both busses identical, plot the left

    t = np.arange(y.size) / fs
    fig, ax = plt.subplots()
    ax.plot(t, y, color=BLUE, lw=0.5)
    for k in range(5):
        v = np.abs(y[int(k * 0.5 * fs): int((k + 1) * 0.5 * fs)]).max()
        ax.plot([k * 0.5, k * 0.5 + 0.22], [v, v], color=AMBER, lw=1.6)
        ax.text(k * 0.5 + 0.24, v, f"{v:.2f}", color=AMBER, va="center", fontsize=8.5)
    ax.axhline(0.05, color=RED, lw=0.9, ls=":")
    ax.text(3.44, 0.075, "floor 0.05 — retirement", color=RED, ha="right", fontsize=8.5)
    ax.set_xlabel("time (s)")
    ax.set_ylabel("output")
    ax.set_title("decay 0.5: each return half the previous peak, then the bloom retires")
    fig.savefig(out_dir("garden") / "staircase.svg", bbox_inches="tight")
    plt.close(fig)


if __name__ == "__main__":
    generation_loss()
    raster()
    staircase()
    print("wrote", *(str(p) for p in sorted(IMAGES.glob("*/generation-loss.svg"))),
          str(IMAGES / "airport" / "raster.svg"), str(IMAGES / "garden" / "staircase.svg"))
