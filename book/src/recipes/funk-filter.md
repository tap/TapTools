# Sixteenths into a listening filter

The envelope filter earned its place in funk on documented records — the
Mu-Tron-era clavinets and basses of the seventies, Stevie Wonder's
"Higher Ground" chief among them — and the autowah chapter is honest about
what this object is instead: a model of the Snow White AutoWah, a
different, throatier circuit. You are not summoning a Mu-Tron; you are
plugging into a very good pedal that listens the same way. The funk is in
what it listens *to* — which makes this the one recipe where the settings
table is half the story and your right hand is the other half.

Measured behavior cited below (the sweep law exact to the design, the RC
release, the 250 Hz → ~2.5 kHz hardware span) lives in
[the pedal that listens](../autowah.md) and its
[validation notebook](https://github.com/tap/TapTools/blob/main/notebooks/autowah_validation.ipynb).

## How to think about the knobs

Two of them are calibration, one is the personality:

- **`sensitivity`** matches the pedal to your source's level and your
  touch. Tune it so your *normal* hits open the filter halfway and your
  hard hits open it fully — the tanh knee compresses beyond that instead
  of slamming. Too high and everything pins; too low and the filter
  ignores you.
- **`bias` and `range`** set where the sweep lives: resting frequency and
  octaves above it. The defaults (250 Hz, 3.3 octaves) *are* the hardware.
- **`decay`** is the personality: how fast the filter falls back. Tens of
  ms is a wah articulation on every note — the funk setting. Hundreds is
  a swell that rides phrasing.

## The patches

| patch | settings |
|---|---|
| the clav chop | `@sensitivity 3 @attack 2 @decay 80 @bias 250 @range 3.3 @resonance 0.7 @mix 100` |
| the bass quack | `recall 2`, then `@decay 150 @resonance 0.6` |
| the slow swell | `recall 3`, or `@decay 900 @range 2.5` on pads |
| the cocked wah | `recall 4` — `sensitivity` at −60 is the envelope off; park `bias` at 800–1200 Hz |

- **The clav chop** wants sixteenth-note playing with deliberate dynamic
  contrast — the filter turns your accents into vowels. `mode 1`
  (bandpass, the circuit's other tap) is quackier and noticeably quieter;
  make it up with `gain`.
- **The bass quack** starts from the factory bass voicing (slot 2 —
  lower bias, tighter range, the GB pedal's instrument switch as a
  preset). Fingers, not pick, and let notes ring — the release is a real
  RC discharge (measured: a pure exponential, σ = 0.004) and it *sounds*
  like circuitry when you leave it room.
- **The cocked wah** is the secret mode: a fixed resonant filter with
  `bias` as a manual sweep — the parked-pedal midrange honk, and slot 4
  ships it.
- **`direction 1`** sweeps *down* from bias — the extension the pedal
  never had; reverse-envelope funk on a clean chop is startling.

## The two patch points nobody uses enough

- **The sidechain (right inlet):** one sound wahs another. Kick →
  sidechain, pad → filter is the classic; a `tap.808.seq~` row (through
  `@pulse` widened impulses) makes the filter *sequenced* while the pad
  sustains — an envelope filter with a drummer's timing.
- **The envelope outlet (right outlet, 0..1 signal):** the detector as a
  free modulation source. Scale it into `tap.vco~`'s FM inlet, a
  `tap.vca~` gain, or a second filter — one performance, many
  destinations. (In `bypass` the outlet goes to zero, so tap it from a
  live instance.)

## When to leave the recipe

- **Your source has no dynamics.** A static pad through an auto-wah is a
  static filter — feed the sidechain something rhythmic, or use
  `tap.svf~` with an LFO and own the motion yourself.
- **You want the filter on a knob.** That's the cocked wah until you want
  *morphing* responses — then `tap.svf~`'s `morph` is the tool.
- **You want the exact Mu-Tron quack.** Raise `resonance`, try `mode 1`,
  and know the chapter's warning stands: you're modding a Snow White. The
  hardware A/B pass — the notebook cell waiting for the real pedal — will
  tell us precisely how far the model is from *its own* hardware, not
  from someone else's.
