"""ctypes bridge to the TapTools C ABI, shared by the verification notebooks.

Loads build_capi/libtaptools_capi.{so,dylib,dll} relative to the kernel root
(kernel/ in the TapTools repo), building it first if missing (requires cmake
in PATH):

    cmake -B kernel/build_capi -S kernel/tools/capi
    cmake --build kernel/build_capi

The C ABI (kernel/tools/capi/) wraps the *same* portable DSP headers the Max
externals compile — so the notebooks exercise the real shipping code, not a
Python re-implementation. Exposed kernels: tap.convolve~'s conv_engine
(`Convolver`), tap.svf~ (`Svf`), tap.ladder~ (`Ladder`), tap.vco~ (`Vco`),
and tap.autowah~ (`Wah`). Parameter names on the kernel classes mirror each
kernel header's param_index enum.

Copyright 2003-2026 Timothy Place. New BSD License.
"""

from __future__ import annotations

import ctypes
import pathlib
import subprocess
import sys

import numpy as np

# The kernel root (this file lives in kernel/notebooks/).
KERNEL = pathlib.Path(__file__).resolve().parent.parent

# Categorical palette for the notebooks (colorblind-safe, fixed assignment
# order — never cycled). Sequential maps use viridis; diverging use RdBu_r.
PALETTE = ["#4269d0", "#efb118", "#ff725c", "#6cc5b0", "#3ca951", "#ff8ab7", "#a463f2"]

_BUILD = KERNEL / "build_capi"


def _lib_path() -> pathlib.Path:
    stem = "taptools_capi"
    names = {"linux": f"lib{stem}.so", "darwin": f"lib{stem}.dylib", "win32": f"{stem}.dll"}
    name = next(v for k, v in names.items() if sys.platform.startswith(k))
    # single-config generators put the lib at the build root; MSVC nests it under a config dir
    for cand in (_BUILD / name, _BUILD / "Release" / name, _BUILD / "Debug" / name):
        if cand.exists():
            return cand
    return _BUILD / name


def _build_lib() -> None:
    subprocess.run(["cmake", "-B", str(_BUILD), "-S", str(KERNEL / "tools" / "capi")],
                   cwd=KERNEL, check=True, capture_output=True)
    subprocess.run(["cmake", "--build", str(_BUILD), "--config", "Release", "--parallel"],
                   cwd=KERNEL, check=True, capture_output=True)


def load() -> ctypes.CDLL:
    if not _lib_path().exists():
        print("building taptools_capi ...")
        _build_lib()
    lib = ctypes.CDLL(str(_lib_path()))

    vp = ctypes.c_void_p
    f32p = ctypes.POINTER(ctypes.c_float)
    f64p = ctypes.POINTER(ctypes.c_double)
    sigs = {
        "taptools_conv_create":         ([], vp),
        "taptools_conv_destroy":        ([vp], None),
        "taptools_conv_configure":      ([vp, ctypes.c_int, ctypes.c_int], ctypes.c_int),
        "taptools_conv_load_ir":        ([vp, f32p, f32p, f32p, f32p, ctypes.c_int, ctypes.c_double],
                                         ctypes.c_int),
        "taptools_conv_clear":          ([vp], ctypes.c_int),
        "taptools_conv_process":        ([vp, f64p, f64p, f64p, f64p, ctypes.c_int], ctypes.c_int),
        "taptools_conv_block_size":     ([vp], ctypes.c_int),
        "taptools_conv_max_partitions": ([vp], ctypes.c_int),
        "taptools_conv_has_ir":         ([vp], ctypes.c_int),
        # the four parameter-indexed kernels share one shape (see _Kernel below)
        "taptools_svf_create":          ([], vp),
        "taptools_svf_destroy":         ([vp], None),
        "taptools_svf_prepare":         ([vp, ctypes.c_double], ctypes.c_int),
        "taptools_svf_set":             ([vp, ctypes.c_int, ctypes.c_double], ctypes.c_int),
        "taptools_svf_set_mode":        ([vp, ctypes.c_int], ctypes.c_int),
        "taptools_svf_set_order":       ([vp, ctypes.c_int], ctypes.c_int),
        "taptools_svf_set_circuit":     ([vp, ctypes.c_int], ctypes.c_int),
        "taptools_svf_set_oversample":  ([vp, ctypes.c_int], ctypes.c_int),
        "taptools_svf_set_smooth_ms":   ([vp, ctypes.c_double], ctypes.c_int),
        "taptools_svf_clear":           ([vp], ctypes.c_int),
        "taptools_svf_process":         ([vp, f64p, f64p, ctypes.c_int], ctypes.c_int),
        "taptools_svf_process_mod":     ([vp, f64p, f64p, f64p, ctypes.c_int], ctypes.c_int),
        "taptools_ladder_create":         ([], vp),
        "taptools_ladder_destroy":        ([vp], None),
        "taptools_ladder_prepare":        ([vp, ctypes.c_double], ctypes.c_int),
        "taptools_ladder_set":            ([vp, ctypes.c_int, ctypes.c_double], ctypes.c_int),
        "taptools_ladder_set_mode":       ([vp, ctypes.c_int], ctypes.c_int),
        "taptools_ladder_set_solver":     ([vp, ctypes.c_int], ctypes.c_int),
        "taptools_ladder_set_oversample": ([vp, ctypes.c_int], ctypes.c_int),
        "taptools_ladder_set_smooth_ms":  ([vp, ctypes.c_double], ctypes.c_int),
        "taptools_ladder_clear":          ([vp], ctypes.c_int),
        "taptools_ladder_process":        ([vp, f64p, f64p, ctypes.c_int], ctypes.c_int),
        "taptools_ladder_process_mod":    ([vp, f64p, f64p, f64p, ctypes.c_int], ctypes.c_int),
        "taptools_vco_create":        ([], vp),
        "taptools_vco_destroy":       ([vp], None),
        "taptools_vco_prepare":       ([vp, ctypes.c_double], ctypes.c_int),
        "taptools_vco_set":           ([vp, ctypes.c_int, ctypes.c_double], ctypes.c_int),
        "taptools_vco_set_seed":      ([vp, ctypes.c_uint], ctypes.c_int),
        "taptools_vco_set_smooth_ms": ([vp, ctypes.c_double], ctypes.c_int),
        "taptools_vco_clear":         ([vp], ctypes.c_int),
        "taptools_vco_process":       ([vp, f64p, ctypes.c_int], ctypes.c_int),
        "taptools_vco_process_mod":   ([vp, f64p, f64p, f64p, ctypes.c_int], ctypes.c_int),
        "taptools_wah_create":        ([], vp),
        "taptools_wah_destroy":       ([vp], None),
        "taptools_wah_prepare":       ([vp, ctypes.c_double], ctypes.c_int),
        "taptools_wah_set":           ([vp, ctypes.c_int, ctypes.c_double], ctypes.c_int),
        "taptools_wah_set_mode":      ([vp, ctypes.c_int], ctypes.c_int),
        "taptools_wah_set_rectifier": ([vp, ctypes.c_int], ctypes.c_int),
        "taptools_wah_set_smooth_ms": ([vp, ctypes.c_double], ctypes.c_int),
        "taptools_wah_recall":        ([vp, ctypes.c_int, ctypes.c_double], ctypes.c_int),
        "taptools_wah_clear":         ([vp], ctypes.c_int),
        "taptools_wah_process":       ([vp, f64p, f64p, f64p, f64p, f64p, ctypes.c_int], ctypes.c_int),
    }
    for name, (argtypes, restype) in sigs.items():
        fn = getattr(lib, name)
        fn.argtypes = argtypes
        fn.restype = restype
    return lib


_LIB = load()


def _f32(a) -> np.ndarray:
    return np.ascontiguousarray(a, dtype=np.float32)


def _f64(a) -> np.ndarray:
    return np.ascontiguousarray(a, dtype=np.float64)


def _p32(a: np.ndarray):
    return a.ctypes.data_as(ctypes.POINTER(ctypes.c_float))


def _p64(a: np.ndarray):
    return a.ctypes.data_as(ctypes.POINTER(ctypes.c_double))


def _check(code: int, who: str) -> None:
    if code is not None and code < 0:
        raise RuntimeError(f"{who} failed")


class Convolver:
    """A live tap.convolve~ conv_engine — true-stereo uniformly-partitioned overlap-save.

    >>> c = Convolver(blocksize=256, max_partitions=64)
    >>> c.load_ir(ll=ir, rr=ir)          # any of ll/lr/rl/rr; missing paths are silent
    >>> outL, outR = c.process(inL, inR) # wet (fully convolved) stereo output
    """

    def __init__(self, blocksize: int, max_partitions: int):
        self._h = _LIB.taptools_conv_create()
        if not self._h:
            raise RuntimeError("taptools_conv_create failed")
        _check(_LIB.taptools_conv_configure(self._h, int(blocksize), int(max_partitions)), "configure")

    def load_ir(self, ll=None, lr=None, rl=None, rr=None, scale: float = 1.0) -> None:
        """Publish the four true-stereo IR paths (LL, LR, RL, RR). Missing paths are silent; the
        arrays may differ in length (shorter ones are zero-padded to the longest)."""
        arrs = [ll, lr, rl, rr]
        length = max((len(a) for a in arrs if a is not None), default=0)
        self._ir_keep = []  # keep the float buffers alive across the C call
        ptrs = []
        for a in arrs:
            if a is None:
                ptrs.append(None)
            else:
                f = _f32(np.pad(np.asarray(a, dtype=np.float32), (0, length - len(a))))
                self._ir_keep.append(f)
                ptrs.append(_p32(f))
        _check(_LIB.taptools_conv_load_ir(self._h, ptrs[0], ptrs[1], ptrs[2], ptrs[3],
                                          int(length), float(scale)), "load_ir")

    def process(self, inL, inR) -> tuple[np.ndarray, np.ndarray]:
        inL, inR = _f64(inL), _f64(inR)
        n = len(inL)
        outL, outR = np.empty(n), np.empty(n)
        _check(_LIB.taptools_conv_process(self._h, _p64(inL), _p64(inR), _p64(outL), _p64(outR), n),
               "process")
        return outL, outR

    def clear(self) -> None:
        _check(_LIB.taptools_conv_clear(self._h), "clear")

    @property
    def block_size(self) -> int:
        return _LIB.taptools_conv_block_size(self._h)

    @property
    def max_partitions(self) -> int:
        return _LIB.taptools_conv_max_partitions(self._h)

    @property
    def has_ir(self) -> bool:
        return bool(_LIB.taptools_conv_has_ir(self._h))

    def __del__(self):
        h = getattr(self, "_h", None)
        if h:
            _LIB.taptools_conv_destroy(h)
            self._h = None


def convolve_stereo(inL, inR, ll=None, lr=None, rl=None, rr=None, *, blocksize: int = 256,
                    scale: float = 1.0) -> tuple[np.ndarray, np.ndarray]:
    """One-shot helper: run inL/inR through a fresh engine with the given IR paths, sized to hold
    the whole IR. Returns the wet stereo output (latency = blocksize samples)."""
    length = max((len(a) for a in (ll, lr, rl, rr) if a is not None), default=0)
    max_parts = max(1, (length + blocksize - 1) // blocksize)
    c = Convolver(blocksize, max_parts)
    c.load_ir(ll=ll, lr=lr, rl=rl, rr=rr, scale=scale)
    return c.process(inL, inR)


class _Kernel:
    """Base for the parameter-indexed kernels (Svf/Ladder/Vco/Wah): a live C-ABI handle with
    keyword parameter setters mirroring the kernel header's param_index enum.

    Subclasses define PREFIX (the C-ABI function prefix), PARAMS (name -> index), and MODES /
    structural setter names. `set(smooth_ms=0)` is applied by default so notebook measurements
    are exact unless a test wants the ramps."""

    PREFIX: str = ""
    PARAMS: dict[str, int] = {}

    def __init__(self, sr: float = 48000.0, smooth_ms: float = 0.0, **params):
        self._c = {name: getattr(_LIB, f"{self.PREFIX}_{name}", None)
                   for name in ("create", "destroy", "prepare", "set", "set_smooth_ms", "clear")}
        self._h = self._c["create"]()
        if not self._h:
            raise RuntimeError(f"{self.PREFIX}_create failed")
        _check(self._c["prepare"](self._h, float(sr)), "prepare")
        self.sr = float(sr)
        _check(self._c["set_smooth_ms"](self._h, float(smooth_ms)), "set_smooth_ms")
        self.set(**params)

    def set(self, **params) -> "_Kernel":
        """Set ramped parameters by name (e.g. svf.set(frequency=1000, resonance=0.5)) and any
        structural setter exposed by the subclass (mode=..., order=..., ...)."""
        for name, value in params.items():
            if name in self.PARAMS:
                _check(self._c["set"](self._h, self.PARAMS[name], float(value)), name)
            else:
                fn = getattr(_LIB, f"{self.PREFIX}_set_{name}", None)
                if fn is None:
                    raise AttributeError(f"{type(self).__name__} has no parameter '{name}'")
                arg = float(value) if name == "smooth_ms" else int(value)
                _check(fn(self._h, arg), name)
        return self

    def clear(self) -> None:
        _check(self._c["clear"](self._h), "clear")

    def __del__(self):
        h = getattr(self, "_h", None)
        if h:
            self._c["destroy"](h)
            self._h = None


class Svf(_Kernel):
    """tap.svf~'s Simper/Cytomic morphing SVF (taptools::svf::svf_filter, mono).

    >>> f = Svf(48000, frequency=1000, resonance=0.5, mode=Svf.LOWPASS, order=4)
    >>> y = f.process(x)                      # block process
    >>> y = f.process(x, cutoff_hz=sweep)     # per-sample signal-rate cutoff
    """

    PREFIX = "taptools_svf"
    PARAMS = {"frequency": 0, "resonance": 1, "morph": 2, "gain": 3, "drive": 4}
    LOWPASS, HIGHPASS, BANDPASS, NOTCH, PEAK, ALLPASS, MORPH, BELL, LOWSHELF, HIGHSHELF = range(10)
    CLEAN, DRIVEN = 0, 1

    def process(self, x, cutoff_hz=None) -> np.ndarray:
        x = _f64(x)
        out = np.empty(len(x))
        if cutoff_hz is None:
            _check(_LIB.taptools_svf_process(self._h, _p64(x), _p64(out), len(x)), "process")
        else:
            c = _f64(cutoff_hz)
            _check(_LIB.taptools_svf_process_mod(self._h, _p64(x), _p64(c), _p64(out), len(x)),
                   "process_mod")
        return out


class Ladder(_Kernel):
    """tap.ladder~'s ZDF transistor ladder (taptools::ladder::ladder_filter).

    >>> f = Ladder(48000, frequency=800, resonance=0.7, mode=Ladder.LP24)
    >>> y = f.process(x)                      # block process
    >>> y = f.process(x, cutoff_hz=sweep)     # per-sample signal-rate cutoff
    """

    PREFIX = "taptools_ladder"
    PARAMS = {"gain": 0, "frequency": 1, "resonance": 2, "drive": 3, "comp": 4, "asym": 5}
    LP24, LP12, BP12, BP24, HP12, HP24 = range(6)
    FAST, EXACT = 0, 1

    def process(self, x, cutoff_hz=None) -> np.ndarray:
        x = _f64(x)
        out = np.empty(len(x))
        if cutoff_hz is None:
            _check(_LIB.taptools_ladder_process(self._h, _p64(x), _p64(out), len(x)), "process")
        else:
            c = _f64(cutoff_hz)
            _check(_LIB.taptools_ladder_process_mod(self._h, _p64(x), _p64(c), _p64(out), len(x)),
                   "process_mod")
        return out


class Vco(_Kernel):
    """tap.vco~'s virtual-analog oscillator (taptools::vco::vco_osc).

    >>> o = Vco(48000, frequency=110, shape=Vco.SAW)
    >>> y = o.process(4800)                                  # n samples
    >>> y = o.process(fm_hz=fm)                              # through-zero FM (Hz)
    >>> y = o.process(sync=master)                           # hard sync input
    """

    PREFIX = "taptools_vco"
    PARAMS = {"gain": 0, "frequency": 1, "shape": 2, "pw": 3, "drift": 4, "detune": 5}
    SINE, TRIANGLE, SAW, PULSE = range(4)

    def process(self, n: int | None = None, fm_hz=None, sync=None) -> np.ndarray:
        if fm_hz is None and sync is None:
            out = np.empty(int(n))
            _check(_LIB.taptools_vco_process(self._h, _p64(out), len(out)), "process")
            return out
        length = len(fm_hz) if fm_hz is not None else len(sync)
        if n is not None and int(n) != length:
            raise ValueError("n disagrees with the modulation array length")
        fm_arr = _f64(fm_hz) if fm_hz is not None else None
        sy_arr = _f64(sync) if sync is not None else None
        out = np.empty(length)
        _check(_LIB.taptools_vco_process_mod(self._h,
                                             _p64(fm_arr) if fm_arr is not None else None,
                                             _p64(sy_arr) if sy_arr is not None else None,
                                             _p64(out), length), "process_mod")
        return out

    def seed(self, seed: int) -> "Vco":
        _check(_LIB.taptools_vco_set_seed(self._h, int(seed)), "set_seed")
        return self


class Wah(_Kernel):
    """tap.autowah~'s Snow White-style envelope filter (taptools::autowah::wah_filter).

    >>> w = Wah(48000, sensitivity=0, decay=250)
    >>> y = w.process(x)                          # envelope tracks the input (the pedal)
    >>> y = w.process(x, key=other)               # sidechain
    >>> y, env, cutoff = w.process(x, trace=True) # + per-sample control trajectories
    """

    PREFIX = "taptools_wah"
    PARAMS = {"sensitivity": 0, "attack": 1, "decay": 2, "bias": 3, "range": 4,
              "resonance": 5, "drive": 6, "gain": 7, "mix": 8}
    LOWPASS, BANDPASS = 0, 1
    FULLWAVE, HALFWAVE = 0, 1

    def process(self, x, key=None, trace: bool = False):
        x = _f64(x)
        n = len(x)
        out = np.empty(n)
        key_arr = _f64(key) if key is not None else None
        env = np.empty(n) if trace else None
        cut = np.empty(n) if trace else None
        _check(_LIB.taptools_wah_process(self._h, _p64(x),
                                         _p64(key_arr) if key_arr is not None else None,
                                         _p64(out),
                                         _p64(env) if trace else None,
                                         _p64(cut) if trace else None, n), "process")
        return (out, env, cut) if trace else out

    def recall(self, slot: int, seconds: float = 0.0) -> "Wah":
        """Morph to a preset slot (0-based; 0-3 = factory guitar/bass/swell/cocked)."""
        _check(_LIB.taptools_wah_recall(self._h, int(slot), float(seconds)), "recall")
        return self
