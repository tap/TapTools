# The patches with names on them

The previous recipe built the three-oscillator voice. This one drives it at
four records — a blue-eyed-soul hook, the bassline that retired a bass
player, a singing art-rock lead, and the one-take modular solo that started
it all — and, along the way, answers a fair question: if "Lucky Man" was
played on a Moog *modular*, does the kit need a modular object?

The provenance rule from the part opener applies double here, because gear
folklore is a genre of its own. For each patch the chapter says what is
documented about the record and what is reconstruction. And the standing
disclaimer stands: these settings chase the *sound*; the hands, the tape,
and the mix stay on the record.

Every patch below is a delta against the wiring and tables of
[Three oscillators into a ladder](minimoog.md) — build that voice first.
Two performance tools recur, so here they are once:

- **Vibrato** is the oscillator's own now: `@vibrato` (depth in cents, so
  the musical width holds in every register), `@vibrato_rate` (Hz), and
  `@vibrato_delay` (ms) — the onset fades in through that time constant
  and re-arms on every new note, which is most of what makes a lead
  "sing." ±10 cents at 5.5 Hz with a few hundred milliseconds of delay is
  the classic setting. (This chapter's first draft had to print a
  scaling formula into the Hz-calibrated FM inlet here; that formula
  became the improvements plan's §2, and §2 became these attributes —
  the audit worked.)
- **Sequenced lines**: `tap.303.seq~` emits pitch as a MIDI-note signal and
  a gate at 1.0/2.0 — `mtof~` turns the pitch into Hz for the oscillators'
  signal inlets, and the gate drives `tap.adsr~` directly (it opens above
  0.5). The Moog voice sequenced this way is the classic
  synth-line scaffold, slides included.

## The Winwood hook — "While You See a Chance" (1980)

What's documented: Winwood played essentially everything on *Arc of a
Diver* himself, synthesizers included; accounts of the rig put Moog
monosynths at the center of it. The reconstruction: the opening hook is a
brassy, open-filter lead with a fast attack and just enough glide to round
the corners — a patch that sits between horn section and organ, which is
very much a keyboardist's lead.

Deltas from the lead patch:

| control | setting |
|---|---|
| voices | 1 and 2 only, at f, `detune` −6 / +6; retire voice 3 |
| all `smooth` | 40 ms |
| ladder | `@resonance 0.3 @drive 6 @asym 0.3` |
| filter contour | `@attack 5 @decay 500 @sustain -6 @release 400`, amount 4500 Hz, base 400 Hz |
| loudness contour | `@attack 5 @decay 200 @sustain -2 @release 250` |

The brass illusion is the filter contour's `sustain` sitting high (−6 dB):
the filter opens and *stays* open, so the tone holds its brightness through
the note instead of wah-ing. Play the hook in clean detached eighths — the
40 ms of glide only speaks when notes touch.

## The bassline that retired a bass player — "Flash Light" (1977)

What's documented, and gloriously so: Bernie Worrell built Parliament's
"Flash Light" bassline by stacking Minimoogs — the story is told with the
number three attached — playing the line keyboard-style under Bootsy
Collins' guitar. This is the patch where the previous chapter's "the stack
is most of the sound" rule gets its funk citation.

Deltas from the bass patch:

| control | setting |
|---|---|
| voices | 1 and 2 at f (`detune` −7 / +7), voice 3 at f ÷ 2, its `gain` −6 |
| all `smooth` | 35 ms |
| ladder | `@resonance 0.6 @drive 12 @asym 0.5 @comp 0.2` |
| filter contour | `@attack 1 @decay 150 @sustain -24 @release 150`, amount 2200 Hz, base 90 Hz |
| loudness contour | `@attack 1 @decay 250 @sustain -6 @release 100` |

The rubber is the filter contour: near-instant attack, short decay, and a
`sustain` low enough (−24 dB) that every note is a squelch that immediately
ducks. `resonance 0.6` puts a vowel on the squelch; `drive 12` into the
tanh stages is the fat (the ladder chapter measures 16.5 % THD up there —
that's the point). Play staccato sixteenths with octave pops; let the 35 ms
glide smear only the connected passing notes. If the low end blurs, this is
the one patch where `comp` earns its raise: 0.2 keeps some droop-era
character while returning enough passband to anchor the root.

## The singing lead — "Shine On You Crazy Diamond" (1975)

What's documented: Richard Wright's rig in the *Wish You Were Here*
sessions included a Minimoog, and the singing synth lead lines in "Shine
On" are credited to it. The reconstruction: a nearly clean patch — this
lead's beauty is restraint, a barely-driven filter, and vibrato that
arrives late.

Deltas from the lead patch:

| control | setting |
|---|---|
| voices | 1 and 2 at f, `detune` −2 / +2 — a shimmer, not a chorus |
| all `smooth` | 15 ms |
| ladder | `@resonance 0.15 @drive 3 @asym 0.2` |
| filter contour | `@attack 30 @decay 900 @sustain -10 @release 700`, amount 3000 Hz, base 250 Hz |
| loudness contour | `@attack 8 @decay 300 @sustain -2 @release 500` |

Then spend all your effort on the vibrato: `@vibrato 10 @vibrato_rate 5.5
@vibrato_delay 400` — ten cents, arriving late, re-arming on each new
note so held phrase-endings bloom while passing notes stay plain. The patch is deliberately close to the ideal oscillator —
`imperfect 0.2`, drift at the polite end — because the expressive load is
carried by the hands, and everything the analog section adds here it adds
to sustained exposed notes.

## The one-take solo — "Lucky Man" (1970), and the modular question

What's documented: Keith Emerson's solo on "Lucky Man" was played on his
Moog modular system and famously kept from an improvised take — one of the
first Moog solos on a rock record, and for a generation of listeners the
first synthesizer they ever heard. The sound: a huge unison lead whose
actual melodic content is mostly *portamento* — sweeps and dives across
octaves, the glide circuit played as the instrument.

Deltas from the lead patch:

| control | setting |
|---|---|
| voices | all three; voice 3 up at f (unison), `detune` −5 / +4 / +7 |
| all `smooth` | 280 ms |
| ladder | `@resonance 0.2 @drive 8 @asym 0.4` |
| filter contour | `@attack 10 @decay 800 @sustain -4 @release 600`, amount 5000 Hz, base 800 Hz |
| loudness contour | `@attack 10 @decay 300 @sustain -1 @release 400` |

At 280 ms of `smooth`, pitch is a place you *travel to*: hold a note, strike
one two octaves up, and the voice draws the line between them. That is the
solo. The filter stays essentially open (`sustain` −4 dB) because the
record's drama is in pitch, not timbre.

So — does the kit need a Moog modular object? **No, because you are
holding one.** A modular synthesizer is oscillators, filters, envelopes,
and amplifiers with *no fixed routing*; the panel of patch cords is the
product. In this package the modules are `tap.vco~`, `tap.ladder~`,
`tap.svf~`, `tap.adsr~`, `tap.vca~`, `tap.noise~`, and the sequencer pair —
and Max itself is the patch panel, with the routing freedom no hardwired
monosynth voice (and no single "modular object") could offer. Everything
Emerson's system did on that solo — voices summed to one filter, one
loudness contour, glide on the pitch source — is the previous chapter's
wiring diagram; what the modular *added* was the freedom to have wired it
otherwise, and that freedom is the patching environment you are already
in. The one genuinely modular idiom worth calling out is the sequenced
line: `tap.303.seq~` → `mtof~` → the stack, gate → `tap.adsr~`, is the
Moog-sequencer scaffold of the Berlin school and "I Feel Love"-era disco —
no new object required, slides included.

## What separates the four

The instructive part of putting these side by side: the signal chain never
changed. What moved:

1. **The filter contour's `sustain`.** High and it's brass (Winwood), open
   and it's drama (Emerson), low and it's rubber (Worrell). One attribute
   spans the genre map.
2. **`smooth`.** 15 ms is articulation, 40 ms is rounding, 280 ms is the
   melody itself.
3. **`drive` and `resonance`.** The funk patch is the only one leaning
   hard on both — and it's the one imitating three stacked instruments.
4. **The hands.** Delayed vibrato, staccato versus legato, when *not* to
   play — the parts of the record the recipe honestly can't ship.

## When to leave the recipe

- **You want the record's whole arrangement.** The hook was never alone:
  Winwood's is doubled, Worrell's sits under a live band, Wright's floats
  on tape-delayed guitars. The patch is the voice, not the mix.
- **You want polysynth-era sounds.** Prophets and Oberheims are a
  different architecture — per-voice envelopes on real polyphony — and
  imitating them with `mc.` stacks of this voice flatters neither.
- **You want the sequenced-modular genre.** Start from the scaffold above,
  but that recipe deserves its own chapter — it lives in the plan file's
  backlog with "I Feel Love" written on it.
