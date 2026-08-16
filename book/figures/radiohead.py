#!/usr/bin/env python3
"""Generate the measured figures for the Radiohead-family book chapters.

Drives the *shipping* kernels (tapecho.h, stammer.h, fuzz.h) through the C ABI via the
notebooks' ctypes bridge — the same rule as eno.py and the verification
notebooks: figures are measurements of the real DSP, never illustrations of
what it should do. The companion notebooks (notebooks/tapecho.ipynb,
stammer.ipynb, fuzz.ipynb) carry the same measurements with commentary; this script renders
the book-styled SVGs.

Regenerate after a kernel behavior change:

    python3 book/figures/radiohead.py   # writes book/src/images/{tapecho,stammer,fuzz}/*.svg

Colors and rcParams are eno.py's, verbatim in intent: the house categorical
hues with the amber snapped darker so pairs pass the print/CVD lightness-band
checks on a light page, and direct labels everywhere so identity never rides on
color alone.
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


def pluck_train(seconds, period=0.31):
    """The family's documented material: transients, and a phrase rather than one
    note repeating (a single note would flatter a stutter)."""
    pitches = np.array([196.0, 233.1, 261.6, 349.2, 293.7])
    t = np.arange(int(seconds * fs)) / fs
    phi = np.mod(t, period)
    hz = pitches[(t // period).astype(int) % pitches.size]
    out = np.zeros_like(t)
    for k in range(1, 6):
        out += (1.0 / k) * np.exp(-phi * (4.0 + 3.0 * k)) * np.sin(2 * np.pi * hz * k * phi)
    return 0.5 * out


def head_layout():
    """tapecho: an impulse returning once per head, on the motor's grid."""
    span = 0.4
    m = tap.TapEcho(fs, 2.0, smooth_ms=0, wow=(0, 0), flutter=(0, 0), regen=0.0,
                    drive=0.0, darken_hz=20000.0, mix=100, span_ms=span * 1000, heads=4)
    x = np.zeros(int(0.55 * fs))
    x[0] = 1.0
    left, _ = m.process(x)

    fig, ax = plt.subplots(figsize=(7.2, 2.5))
    t = np.arange(left.size) / fs * 1000.0
    ax.plot(t, left, color=BLUE, lw=1.0)
    for i in range(4):
        ms = span * (i + 1) / 4 * 1000.0
        ax.axvline(ms, color=MUTED, lw=0.8, ls=":")
        ax.text(ms + 4, 0.60, f"{(i + 1) / 4:.2f}", color=MUTED, fontsize=8.5)
    ax.text(150, 0.60, "head position, as a fraction of span", color=MUTED, fontsize=8.5)
    ax.set_xlabel("time (ms) — motor span 400 ms")
    ax.set_ylabel("output")
    ax.set_title("one impulse, four heads: each returns at span × its ratio")
    fig.savefig(out_dir("tapecho") / "head-layout.svg", bbox_inches="tight")
    plt.close(fig)


def self_oscillation():
    """tapecho: past-unity regeneration plateaus under the saturator's ceiling."""
    regen, burst = 1.4, 0.5
    drives = np.array([0.3, 0.5, 0.8, 1.2, 2.0])
    peaks = []
    for d in drives:
        m = tap.TapEcho(fs, 2.0, smooth_ms=0, wow=(0, 0), flutter=(0, 0), mix=100,
                        heads=1, ratios=[1.0], span_ms=250.0, regen=regen,
                        drive=float(d), darken_hz=6000.0)
        x = np.zeros(int(14.0 * fs))
        n = int(burst * fs)
        x[:n] = 0.5 * np.random.default_rng(7).uniform(-1, 1, n)
        y, _ = m.process(x)
        peaks.append(float(np.max(np.abs(y))))

    # The analytic ceiling on the tape is |in|max + regen/drive (the saturator bounds the
    # returned signal by 1/drive; the record head adds the direct send), scaled by the single
    # centre-panned head's cos(pi/4).
    ceiling = np.cos(np.pi / 4) * (0.5 + regen / drives)

    fig, ax = plt.subplots()
    ax.plot(drives, ceiling, "-", color=AMBER, lw=1.2, alpha=0.8)
    ax.plot(drives, peaks, "o", color=BLUE, ms=6)
    ax.text(0.55, ceiling[1] * 1.06, "ceiling: (|in|max + regen/drive)", color=AMBER)
    ax.text(0.55, peaks[1] * 0.72, "measured peak", color=BLUE)
    ax.set_xlabel("drive")
    ax.set_ylabel("peak |output|")
    ax.set_title(f"regen {regen}: self-oscillating, and bounded by the saturator")
    fig.savefig(out_dir("tapecho") / "self-oscillation.svg", bbox_inches="tight")
    plt.close(fig)


def occupancy():
    """stammer: repeats, not density, is what holds the machine busy."""
    step_ms, seconds = 60.0, 6.0
    runs = [
        ("density 0.3, repeats 1", dict(density=0.3, repeats=1), BLUE),
        ("density 0.3, repeats 6", dict(density=0.3, repeats=6), AMBER),
        ("density 0.9, repeats 1", dict(density=0.9, repeats=1), RED),
        ("density 0.9, repeats 6", dict(density=0.9, repeats=6), INK),
    ]
    src = pluck_train(seconds)

    fig, ax = plt.subplots(figsize=(7.2, 2.6))
    for row, (label, params, color) in enumerate(runs):
        m = tap.Stammer(fs, 4000.0, smooth_ms=0, mix=100, step_ms=step_ms,
                        divisions=1, seed=7, **params)
        flags = np.zeros(src.size, dtype=bool)
        for i, v in enumerate(src):
            m.process(np.array([v]))
            flags[i] = m.playing
        # Draw the busy stretches as spans, not a per-sample fill: at 48 kHz a fill_between
        # over six seconds emits a path with 288k vertices per row (a 117 MB SVG, measured).
        edges = np.flatnonzero(np.diff(flags.astype(np.int8)))
        bounds = np.concatenate(([0], edges + 1, [flags.size]))
        spans = [(lo / fs, (hi - lo) / fs)
                 for lo, hi in zip(bounds[:-1], bounds[1:]) if flags[lo]]
        ax.broken_barh(spans, (row, 0.78), facecolors=color, linewidth=0)
        ax.text(seconds + 0.05, row + 0.3, f"{100.0 * flags.mean():.0f}% busy",
                color=color, fontsize=8.5)
    ax.set_yticks([r + 0.39 for r in range(len(runs))])
    ax.set_yticklabels([r[0] for r in runs], fontsize=8.5)
    ax.set_xlim(0, seconds + 1.0)
    ax.set_xlabel("time (s)")
    ax.set_title("when a slice is in flight — repeats is the hold, not density")
    fig.savefig(out_dir("stammer") / "occupancy.svg", bbox_inches="tight")
    plt.close(fig)


def material():
    """stammer: the material contract, measured at its premise."""
    t = np.arange(int(3.0 * fs)) / fs
    materials = [("sustained sine", 0.5 * np.sin(2 * np.pi * 196.0 * t), AMBER),
                 ("plucked phrase", pluck_train(3.0), BLUE)]

    def similarity(x, win_ms=60.0, n=96, seed=0):
        rng = np.random.default_rng(seed)
        w = int(win_ms * 0.001 * fs)
        window = np.hanning(w)
        specs = []
        for start in rng.integers(0, x.size - w, n):
            s = np.abs(np.fft.rfft(x[start:start + w] * window))
            norm = np.linalg.norm(s)
            if norm > 0:
                specs.append(s / norm)
        specs = np.array(specs)
        gram = specs @ specs.T
        return float(gram[np.triu_indices(specs.shape[0], 1)].mean())

    scores = [similarity(x) for _, x, _ in materials]

    fig, ax = plt.subplots(figsize=(7.2, 2.2))
    ypos = np.arange(len(materials))
    for y, (label, _, color), score in zip(ypos, materials, scores):
        ax.barh(y, score, height=0.5, color=color)
        ax.text(score + 0.015, y, f"{score:.3f}", color=color, va="center", fontsize=9)
    ax.set_yticks(ypos)
    ax.set_yticklabels([m[0] for m in materials])
    ax.set_xlim(0, 1.12)
    ax.set_xlabel("how alike two arbitrary slices are (1 = interchangeable)")
    ax.set_title("the material contract: re-ordering interchangeable things does nothing")
    ax.grid(axis="y", visible=False)
    fig.savefig(out_dir("stammer") / "material.svg", bbox_inches="tight")
    plt.close(fig)


def fuzz_curve():
    """fuzz: the one clipping family, and its knee as a character control."""
    x = np.linspace(-3, 3, 1201)
    fig, ax = plt.subplots(figsize=(7.2, 2.8))
    for k, color, label in [(0.5, INK, "0.5"), (1.6, BLUE, "1.6 — first stage"),
                            (2.0, AMBER, "2.0 — second stage, edge 0"), (12.0, RED, "12 — edge 1")]:
        ax.plot(x, np.tanh(k * x) / np.tanh(k), color=color, lw=1.4)
        ax.text(3.05, np.tanh(k * 3.0) / np.tanh(k), label, color=color, fontsize=8.5, va="center")
    ax.plot(x, x, color="0.75", lw=0.8, ls="--")
    ax.set_xlim(-3, 4.4); ax.set_ylim(-1.6, 1.6)
    ax.set_xlabel("input"); ax.set_ylabel("output")
    ax.set_title("one curve, tanh(kx)/tanh(k): full scale in is full scale out at every knee")
    fig.savefig(out_dir("fuzz") / "curve.svg", bbox_inches="tight")
    plt.close(fig)


def fuzz_gain_and_bite():
    """fuzz: the gain knob's real sweep, and asymmetry as the even-harmonic control."""
    f0 = 220.0
    t = np.arange(int(0.4 * fs)) / fs
    x = 0.3 * np.sin(2 * np.pi * f0 * t)

    def spec(y):
        seg = y[int(0.5 * y.size):]
        w = np.hanning(seg.size)
        return (np.fft.rfftfreq(seg.size, 1 / fs), np.abs(np.fft.rfft(seg * w)) * 2.0 / w.sum())

    def at(f, fr, m):
        return float(m[np.argmin(np.abs(fr - f))])

    def make(**kw):
        base = dict(smooth_ms=0, bass=0.0, treble=0.0, contrast=0.0, asymmetry=0.0, level_db=0.0)
        base.update(kw)
        return tap.Fuzz(fs, **base)

    knob = np.linspace(0, 1, 11)
    harm = []
    for g in knob:
        fr, m = spec(make(gain=g).process(x))
        harm.append(np.sqrt(sum(at(f0 * k, fr, m) ** 2 for k in range(2, 9))) / at(f0, fr, m))

    even = []
    for a in knob:
        fr, m = spec(make(gain=0.8, asymmetry=a).process(x))
        e = np.sqrt(sum(at(f0 * k, fr, m) ** 2 for k in (2, 4, 6, 8)))
        o = np.sqrt(sum(at(f0 * k, fr, m) ** 2 for k in (3, 5, 7)))
        even.append(e / o)

    fig, (a1, a2) = plt.subplots(1, 2, figsize=(7.2, 2.7))
    a1.plot(knob, harm, color=BLUE, marker="o", ms=4)
    a1.set_xlabel("gain"); a1.set_ylabel("harmonics / fundamental")
    a1.set_title("the gain knob", fontsize=9.5)
    a2.plot(knob, even, color=RED, marker="o", ms=4)
    a2.set_xlabel("asymmetry"); a2.set_ylabel("even / odd energy")
    a2.set_title("asymmetry buys the even harmonics", fontsize=9.5)
    plt.tight_layout()
    fig.savefig(out_dir("fuzz") / "gain-and-bite.svg", bbox_inches="tight")
    plt.close(fig)


if __name__ == "__main__":
    head_layout()
    self_oscillation()
    occupancy()
    material()
    fuzz_curve()
    fuzz_gain_and_bite()
    print("wrote the Radiohead-family figures")
