#!/usr/bin/env python3
"""Generate the measured figures for the Radiohead-family book chapters.

Drives the *shipping* kernels (tapecho.h, stammer.h, fuzz.h, scrub.h, touche.h, diffuseur.h,
ondes.h) through the C ABI via the
notebooks' ctypes bridge — the same rule as eno.py and the verification
notebooks: figures are measurements of the real DSP, never illustrations of
what it should do. The companion notebooks (notebooks/tapecho.ipynb,
stammer.ipynb, fuzz.ipynb) carry the same measurements with commentary; this script renders
the book-styled SVGs.

Regenerate after a kernel behavior change:

    python3 book/figures/radiohead.py
    # writes book/src/images/{tapecho,stammer,fuzz,scrub,ondes,diffuseur}/*.svg

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


# ---- tap.scrub~ ------------------------------------------------------------------------------


def scrub_null():
    """scrub: the identity the whole object is built on, and the window sum behind it."""
    lag, size = 480, 96  # samples; size divides by the overlap, which is what makes it exact
    m = tap.Scrub(fs, 2000.0, smooth_ms=0, overlap=2, mix=100, level=1.0,
                  size_ms=size * 1000.0 / fs, position_ms=lag * 1000.0 / fs)
    x = pluck_train(0.7)
    y = m.process(x)
    err = float(np.max(np.abs(y[2000:] - x[2000 - lag:-lag])))

    fig, (a1, a2) = plt.subplots(1, 2, figsize=(7.2, 2.6))
    sl = slice(9000, 10200)
    ms = np.arange(sl.start, sl.stop) / fs * 1000.0
    a1.plot(ms, x[sl.start - lag:sl.stop - lag], color=AMBER, lw=3.0, alpha=0.45)
    a1.plot(ms, y[sl], color=BLUE, lw=1.0)
    pk = float(np.max(np.abs(y[sl])))
    a1.set_ylim(-1.35 * pk, 1.75 * pk)
    a1.text(ms[10], 1.48 * pk, "input, 480 samples late", color=AMBER, fontsize=8.5)
    a1.text(ms[10], 1.18 * pk, "the scrub", color=BLUE, fontsize=8.5)
    a1.set_xlabel("time (ms)")
    a1.set_title(f"held still at unity pitch: worst error {err:.1e}", fontsize=9.5)

    # Overlaps 2 and 4 both sum to exactly 1, so they are the same line: draw the second
    # dashed and label them apart, or the figure looks like one of them is missing.
    for n_ov, color, style, at in ((1, RED, "-", 1.0), (2, BLUE, "-", 1.0), (4, AMBER, "--", 14.0)):
        g = tap.Scrub(fs, 500.0, smooth_ms=0, overlap=n_ov, mix=100,
                      size_ms=480 * 1000.0 / fs, position_ms=2400 * 1000.0 / fs)
        z = g.process(np.ones(int(0.4 * fs)))[-1200:]
        a2.plot(np.arange(z.size) / fs * 1000.0, z, color=color, lw=1.4, ls=style)
        y_at = float(z[:200].mean())
        a2.text(at, y_at + (0.05 if n_ov != 4 else -0.14), f"overlap {n_ov}", color=color,
                fontsize=8.5)
    a2.set_ylim(0, 1.25)
    a2.set_xlabel("time (ms)")
    a2.set_ylabel("gain on a constant")
    a2.set_title("Hann overlap-adds flat from 2 up", fontsize=9.5)
    plt.tight_layout()
    fig.savefig(out_dir("scrub") / "null.svg", bbox_inches="tight")
    plt.close(fig)


def scrub_two_hands():
    """scrub: the pitch moves while the position does not, and what the wraps cost."""
    def measure(st, f0=311.0, half=15.0):
        g = tap.Scrub(fs, 3000.0, smooth_ms=0, overlap=2, size_ms=100.0,
                      position_ms=900.0, pitch=float(st), mix=100, level=1.0)
        n = int(fs * 3.0)
        t = np.arange(n) / fs
        y = g.process(0.5 * np.sin(2 * np.pi * f0 * t))
        ref = 0.5 * np.sin(2 * np.pi * f0 * 2 ** (st / 12) * t)

        def band(sig):
            seg = sig[len(sig) // 2:]
            mag = np.abs(np.fft.rfft(seg * np.hanning(len(seg))))
            fr = np.fft.rfftfreq(len(seg), 1 / fs)
            sel = np.abs(fr - f0 * 2 ** (st / 12)) <= half
            return np.sqrt((mag[sel] ** 2).sum()), mag[sel].max()

        b, pk = band(y)
        rb, rpk = band(ref)
        return b / rb, (pk / b) / (rpk / rb)

    semis = np.array([-12, -7, -3, 3, 7, 12, 19])
    energy, focus = zip(*[measure(s) for s in semis])

    fig, ax = plt.subplots(figsize=(7.2, 2.6))
    ax.plot(semis, np.array(energy), "o-", color=BLUE, ms=5)
    ax.plot(semis, np.array(focus), "o-", color=RED, ms=5)
    ax.axhline(1.0, color=MUTED, lw=0.8, ls=":")
    ax.text(-11, 0.70, "energy at the transposed pitch", color=BLUE, fontsize=8.5)
    ax.text(-11, 0.64, "how concentrated it is on one line", color=RED, fontsize=8.5)
    ax.set_ylim(0.6, 1.06)
    ax.set_xlabel("pitch (semitones), position held at 900 ms")
    ax.set_ylabel("fraction of a clean shifter")
    ax.set_title("the pitch moves; what the wraps cost is focus, not the note")
    fig.savefig(out_dir("scrub") / "two-hands.svg", bbox_inches="tight")
    plt.close(fig)


# ---- the Ondes Martenot ------------------------------------------------------------------------


def ondes_envelope():
    """ondes: the demodulated envelope, and the harmonics it carries before any valve."""
    det = tap.Detector(fs, frequency=220.0)
    ph = np.linspace(0, 2, 801)

    fig, (a1, a2) = plt.subplots(1, 2, figsize=(7.2, 2.7))
    for depth, color, label in ((1.0, BLUE, "depth 1 (equal oscillators)"),
                                (0.5, AMBER, "depth 0.5")):
        d = tap.Detector(fs, frequency=220.0, depth=depth)
        a1.plot(ph, d.envelope(ph % 1.0), color=color, lw=1.6)
        a1.text(0.06, d.envelope(np.array([0.5]))[0] + 0.12, label, color=color, fontsize=8.5)
    a1.set_xlabel("cycles of the note")
    a1.set_ylabel("envelope")
    a1.set_title("the envelope of two summed oscillators", fontsize=9.5)

    ideal = np.array([(4 / np.pi) / (4 * n * n - 1) for n in range(1, 7)])
    db = 20 * np.log10(ideal[1:] / ideal[0])
    a2.bar(np.arange(2, 7), db, color=BLUE, width=0.6)
    for n, v in zip(range(2, 7), db):
        a2.text(n, v - 1.6, f"{v:.1f}", ha="center", color="white", fontsize=8.5)
    a2.set_xlabel("harmonic")
    a2.set_ylabel("dB below the fundamental")
    a2.set_title("|cos| is not a sinusoid — before any valve", fontsize=9.5)
    plt.tight_layout()
    fig.savefig(out_dir("ondes") / "envelope.svg", bbox_inches="tight")
    plt.close(fig)


def ondes_tube():
    """ondes: the published valve, and the load line that makes a stage out of it."""
    vp = np.linspace(1, 260, 400)
    vbias, rk, rp = tap.OP_DEMOD
    t = tap.Triode(fs, tap.TUBE_6C5, tap.OP_DEMOD)
    vk, vp0, ip0, gain = t.bias

    fig, (a1, a2) = plt.subplots(1, 2, figsize=(7.2, 3.1))
    for vg, color in ((0.0, RED), (-2.0, AMBER), (-4.0, BLUE), (-8.0, MUTED)):
        ip = tap.tube_plate_current(tap.TUBE_6C5, vp, vg) * 1000.0
        a1.plot(vp, ip, color=color, lw=1.3)
        # Label where the curve leaves the visible box, not at its last point: the Vg 0 curve
        # is off the top of the plot by 260 V, and a label out there stretches the layout.
        inside = np.flatnonzero(ip < 13.0)
        j = int(inside[-1]) if inside.size else 0
        a1.text(float(vp[j]) + 4, float(ip[j]), f"Vg {vg:.0f}", color=color, fontsize=8,
                va="center")
    load = (vbias - vk - vp) / rp * 1000.0
    a1.plot(vp, load, color=INK, lw=1.2, ls="--")
    a1.plot([vp0], [ip0 * 1000.0], "o", color=INK, ms=6)
    a1.text(vp0 - 8, ip0 * 1000.0 + 1.6, "quiescent", color=INK, fontsize=8.5, ha="right")
    a1.set_xlim(0, 285)
    a1.set_ylim(0, 14)
    a1.set_xlabel("plate volts")
    a1.set_ylabel("plate current (mA)")
    a1.set_title(f"6C5 at the demodulator's point ({vbias:.0f} V, {rp/1000:.0f}k)", fontsize=9.5)

    gv = np.linspace(-10, 10, 501)
    for name, tube, op, color in (("6C5, demodulator", tap.TUBE_6C5, tap.OP_DEMOD, BLUE),
                                  ("6C5, preamplifier", tap.TUBE_6C5, tap.OP_PREAMP, AMBER),
                                  ("2A3, power amp", tap.TUBE_2A3, tap.OP_POWER, RED)):
        a2.plot(gv, tap.Triode(fs, tube, op).plate_swing(gv), color=color, lw=1.5)
    a2.text(-9.5, -34, "6C5, demodulator", color=BLUE, fontsize=8.5)
    a2.text(-9.5, -46, "6C5, preamplifier", color=AMBER, fontsize=8.5)
    a2.text(-9.5, -58, "2A3, power amp", color=RED, fontsize=8.5)
    a2.set_xlabel("grid volts around the bias point")
    a2.set_ylabel("plate swing (V)")
    a2.set_title("the stage inverts, and it is not symmetric", fontsize=9.5)
    plt.tight_layout()
    fig.savefig(out_dir("ondes") / "tube.svg", bbox_inches="tight")
    plt.close(fig)


def ondes_drive():
    """ondes: the harmonics are there before the drive knob does anything."""
    def run(d):
        v = tap.Ondes(fs, smooth_ms=0, ribbon=24.0, key=1.0, level=1.0, drive=float(d))
        return v.process(int(fs * 1.0)), v.frequency

    def goertzel(y, f):
        seg = y[len(y) // 2:]
        w = 2 * np.pi * f / fs
        c = 2 * np.cos(w)
        s1 = s2 = 0.0
        for val in seg:
            s1, s2 = val + c * s1 - s2, s1
        return np.sqrt(max(0.0, s1 * s1 + s2 * s2 - c * s1 * s2)) * 2 / len(seg)

    drives = np.array([0.0, 0.5, 1.0, 2.0, 4.0, 6.0, 8.0])
    thd, level = [], []
    for d in drives:
        y, f0 = run(d)
        h = np.array([goertzel(y, f0 * k) for k in range(1, 11)])
        thd.append(np.sqrt((h[1:] ** 2).sum()) / h[0])
        level.append(h[0])

    fig, ax = plt.subplots(figsize=(7.2, 2.6))
    ax.plot(drives, thd, "o-", color=BLUE, ms=5)
    ax.plot(drives, level, "o-", color=AMBER, ms=5)
    ax.axhline(thd[0], color=MUTED, lw=0.8, ls=":")
    ax.text(0.15, thd[0] - 0.075, "what the demodulator alone already makes", color=MUTED,
            fontsize=8.5)
    ax.text(4.2, 0.12, "harmonic content", color=BLUE, fontsize=8.5)
    ax.text(3.6, 0.90, "fundamental level", color=AMBER, fontsize=8.5)
    ax.set_ylim(0, 1.0)
    ax.set_xlabel("drive")
    ax.set_title("the valves add to a signal that is already rich")
    fig.savefig(out_dir("ondes") / "drive.svg", bbox_inches="tight")
    plt.close(fig)


def touche_curve():
    """ondes: the intensity key's published law, against the line nobody measured."""
    table_db = np.array([45.0, 53.3, 61.6, 70.0, 78.3, 86.6, 95.0])
    table_mm = np.array([4.3, 5.3, 5.9, 6.4, 6.8, 7.3, 8.8])
    travel = 9.5

    key = tap.Touche(fs, smooth_ms=0)
    p = np.linspace(0, 1, 2001)
    mm = p * travel
    curve = 20 * np.log10(np.where((g := np.array([key.gain_at(v) for v in p])) > 0, g, 1e-300))
    band = (mm >= table_mm[0]) & (mm <= table_mm[-1])
    frac = (mm - table_mm[0]) / (table_mm[-1] - table_mm[0])
    line = -50.0 * (1.0 - frac)

    fig, ax = plt.subplots(figsize=(7.2, 2.7))
    ax.plot(mm[band], curve[band], color=BLUE, lw=2.0)
    ax.plot(mm[band], line[band], color=AMBER, lw=1.2, ls="--")
    ax.plot(table_mm, table_db - table_db[-1], "o", color=RED, ms=6, zorder=5)
    ax.axvspan(0, table_mm[0], color=MUTED, alpha=0.10)
    ax.text(2.1, -25, "silent:\nthe key is\nstill bending", color=MUTED, fontsize=8.5, ha="center")
    ax.text(4.6, -8, "Quartier et al. 2015", color=RED, fontsize=8.5)
    ax.text(6.5, -38, "a straight line, for comparison", color=AMBER, fontsize=8.5)
    ax.set_xlim(0, travel)
    ax.set_xlabel("key displacement (mm)")
    ax.set_ylabel("gain (dB, referenced to full press)")
    ax.set_title("50 dB in 4.5 mm — and the shape is the measurement, not a fit")
    fig.savefig(out_dir("touche") / "curve.svg", bbox_inches="tight")
    plt.close(fig)


# ---- the diffuseurs --------------------------------------------------------------------------


def diffuseur_plate():
    """metallique: where the modes are, and what the body does to a sweep."""
    ratios = np.array([1.000, 1.730, 2.328, 3.910, 4.110, 6.300, 6.710, 7.340])
    gong = tap.Metallique(fs, pitch_hz=180.0, decay=6.0, brightness=1.0,
                          drive=1.0, asymmetry=0.0, saturation=0.0, mix=100.0)
    hz, weight = gong.modes()

    probe = np.geomspace(60.0, 2000.0, 160)
    resp = []
    for f in probe:
        g = tap.Metallique(fs, pitch_hz=180.0, decay=1.2, brightness=1.0,
                           drive=1.0, asymmetry=0.0, saturation=0.0, mix=100.0)
        n = int(fs * 1.0)
        y = g.process(np.sin(2 * np.pi * f * np.arange(n) / fs))
        resp.append(float(np.abs(y[int(fs * 0.75):]).max()))
    resp = np.array(resp)

    fig, (a1, a2) = plt.subplots(1, 2, figsize=(7.2, 2.7))
    a1.vlines(hz, 0, weight, color=BLUE, lw=2.5)
    a1.plot(hz, weight, "o", color=BLUE, ms=5)
    for i, (f, w, r) in enumerate(zip(hz, weight, ratios)):
        a1.annotate(f"{r:.3f}", (f, w), textcoords="offset points",
                    xytext=(0, 6 if i % 2 == 0 else 17), ha="center", fontsize=7.5, color=MUTED)
    a1.set_ylim(0, 0.40)
    a1.set_xlabel("frequency (Hz)")
    a1.set_ylabel("weight")
    a1.set_title(f"eight plate modes, weights summing to {weight.sum():.0f}", fontsize=9.5)

    a2.semilogx(probe, 20 * np.log10(resp), color=BLUE, lw=1.4)
    for f in hz:
        a2.axvline(f, color=MUTED, lw=0.7, ls=":")
    a2.set_xlabel("drive frequency (Hz)")
    a2.set_ylabel("peak out (dB, unit input)")
    a2.set_title("driven, not struck: the body answers a sweep", fontsize=9.5)
    plt.tight_layout()
    fig.savefig(out_dir("diffuseur") / "plate.svg", bbox_inches="tight")
    plt.close(fig)


def diffuseur_selectivity():
    """palme: twelve strings, and what they answer."""
    kw = dict(root_hz=110.0, tuning=0, decay=6.0, damping=4000.0, detune=0.0,
              drive=1.0, asymmetry=0.0, saturation=0.0, mix=100.0, level=1.0, smooth_ms=0.0)

    def ring(hz, drive_s=2.0, tail_s=1.0, fade_s=0.25):
        # The drive is faded: switching a tone on and off is a step, and a step excites every
        # string on the board, so without the fades this measures its own edges.
        p = tap.Palme(fs, **kw)
        n_on = int(fs * drive_s)
        n = n_on + int(fs * tail_s)
        t = np.arange(n) / fs
        g = np.zeros(n)
        f = int(fs * fade_s)
        g[:n_on] = 1.0
        g[:f] = 0.5 - 0.5 * np.cos(np.pi * np.arange(f) / f)
        g[n_on - f:n_on] = 0.5 - 0.5 * np.cos(np.pi * np.arange(f, 0, -1) / f)
        y = p.process(g * 0.3 * np.sin(2 * np.pi * hz * t))
        tail = y[n_on + int(fs * 0.2):]
        return float(np.sqrt(np.mean(tail ** 2)))

    probe = np.geomspace(100.0, 440.0, 150)
    tails = np.array([ring(f) for f in probe])
    strings, _ = tap.Palme(fs, **kw).strings()

    fig, ax = plt.subplots(figsize=(7.2, 2.7))
    ax.semilogx(probe, 20 * np.log10(tails / tails.max()), color=BLUE, lw=1.4)
    for f in strings:
        ax.axvline(f, color=MUTED, lw=0.7, ls=":")
    ax.text(103, -68, "dotted: the twelve strings", color=MUTED, fontsize=8.5)
    ax.set_xticks([100, 150, 200, 300, 440])
    ax.set_xticklabels(["100", "150", "200", "300", "440"])
    ax.set_xticks([], minor=True)  # the log minor labels collide with 440
    ax.set_xlabel("drive frequency (Hz)")
    ax.set_ylabel("ring left after the drive stops (dB)")
    ax.set_title("the palme answers what it is tuned to, and little else")
    fig.savefig(out_dir("diffuseur") / "selectivity.svg", bbox_inches="tight")
    plt.close(fig)


if __name__ == "__main__":
    head_layout()
    self_oscillation()
    occupancy()
    material()
    fuzz_curve()
    fuzz_gain_and_bite()
    scrub_null()
    scrub_two_hands()
    ondes_envelope()
    ondes_tube()
    ondes_drive()
    touche_curve()
    diffuseur_plate()
    diffuseur_selectivity()
    print("wrote the Radiohead-family figures")
