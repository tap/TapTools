# Summary

[Introduction](introduction.md)

# Part I — Sources

- [The oscillator and its knobs](vco.md)

# Part II — Filters

- [The filter that morphs](svf.md)
- [The transistor ladder](ladder.md)
- [The pedal that listens](autowah.md)

# Part III — Strings, rooms, and spirals

- [Borrowed rooms](convolve.md)
- [Five strings, no guitar](fivecomb.md)
- [The spiral staircase](pitchaccum.md)

# Part IV — Tape and time

- [The tape that forgets slowly](discreet.md)
- [Loops that never line up](airport.md)
- [The garden that plays itself](garden.md)
- [The same machine, in pieces](components.md)

# Part V — The spectral set

- [Making the machine talk](vocoder.md)
- [A gate for every bin](nr.md)
- [The spectrum, re-plumbed](spectra.md)

# Part VI — The rhythm section

- [The acid machine](acid.md)
- [The drum machine](drums.md)

# Part VII — Staying in tune

- [The note you meant](tune.md)

# Part VIII — The pedalboard

- [Distortion with a memory](overdrive.md)

# Part IX — The machine, file by file

- [Solving the filter on paper: svf.h](machine/svf.md)
- [The nonlinear loop: ladder.h](machine/ladder.md)
- [The master phase and its corrections: vco.h](machine/vco.md)
- [Detector, law, and a borrowed filter: autowah.h](machine/autowah.md)
- [Convolution without compromise: conv_engine.h](machine/conv.md)
- [Ring time as the truth: grm_comb.h](machine/comb.md)
- [Grains that sum to one: grm_pitchaccum.h](machine/pitchaccum.md)
- [Two banks and a multiplier: vocoder.h](machine/vocoder.md)
- [One STFT, three effects: fft.h, stft.h, nr.h, spectra.h](machine/spectral.md)
- [Seventeen, not four: diode_ladder.h](machine/diode.md)
- [The couplings are the instrument: tb303_voice.h](machine/tb303.md)
- [One network, eight voices: the tr808 headers](machine/tr808.md)
- [Time as a function of phase: step_seq.h](machine/seq.md)
- [Three ways to move a pitch: yin.h, psola.h, pvoc.h](machine/pitch.md)
- [The nearest allowed note: tune.h](machine/tune.md)
- [The clipper in the loop: overdrive.h](machine/overdrive.md)
- [Wear as the stabilizer: tape_loop.h and discreet.h](machine/tape.md)
- [Free-running heads, one shared clock: airport.h](machine/airport.md)
- [Events, not audio: garden.h](machine/garden.md)

# Part X — Recipes

- [How to read a recipe](recipes/cookbook.md)
- [One machine, four decades](recipes/808-classics.md)
- [Three oscillators into a ladder](recipes/minimoog.md)
- [The patches with names on them](recipes/moog-classics.md)
- [Move a knob while it loops](recipes/acid-line.md)
- [The ostinato machine](recipes/sequenced-modular.md)
- [The robot on the radio](recipes/robot-voice.md)
- [A choir of one](recipes/choir-of-one.md)
- [The staircase and the wash](recipes/shimmer.md)
- [Sixteenths into a listening filter](recipes/funk-filter.md)
- [A field guide to rooms](recipes/rooms.md)
- [Chords with no keyboard](recipes/comb-drones.md)
