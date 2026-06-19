# Interview Guide — capturing what only you remember

*The research recovered the **facts and dates** of 1999–2013 reasonably well (see digs `03`–`09`). What no archive holds is the **texture**: what it felt like, what was at stake, the conversations, the decisions, the people. That's perishable, and you're the only source. This guide turns the open questions into prompts.*

## How to use this
- **Answer in any form** — type prose under a prompt, dictate a voice memo, or just talk while someone records. Rough is fine; we polish later.
- **Texture over completeness.** Skip prompts that don't spark anything. A vivid two-sentence memory beats a dutiful paragraph. Concrete beats abstract — names, rooms, dates, what was on the screen, who said what.
- **★ marks a prompt that also resolves a specific research gap or a flagged date conflict** (cross-ref the checklist in `10-source-checklist.md`).
- Drop answers into `research/sources/interview-*.md` (or hand them back to me) and I'll weave them into the dossier as primary source.

---

## Act 1 — Genesis & commerce (1999–2004)

*What we have: copyright dates to 1999; Silicon Prairie Intermedia → Electrotap; Tap.Tools 1 (~2003) then Tap.Tools 2 (April 2005, 150+ externals, $65/$99). Kansas City, Missouri. The git repo holds none of this.*

1. **The very first object.** What was the first `tap.*` object you ever wrote, and why? `tap.quantize` is described in a later commit as "one of the [first] externs i ever wrote." What problem were you actually trying to solve, and for what project or piece?
2. **1999.** The copyright says 1999 but the first public Tap.Tools is years later. What existed in 1999 — what were you making, on what machine, in what context (school, gigs, a band, a job)?
3. ★ **Tap.Tools 1.** When did 1.0 actually ship, what did it cost, and what was in it? (Research can't pin the exact date/price — this is a real gap.)
4. **Silicon Prairie Intermedia.** Where did that name come from? Was it a real business or a banner for shareware? Who else, if anyone, was involved?
5. **The shareware/registration era.** You shipped registration keys and a paid installer for years. What was that experience like — did people actually pay? How much did it earn? What did the "honor system" later in 2013 feel like by comparison?
6. **Why Max, why externals.** Plenty of people patched in Max without ever writing C externals. What pushed you across that line into writing compiled objects?

## Act 2 — Norway, Jade, TTBlue, Jamoma, Hipno (2001–2009)

*What we have: Jade (dev 2001, public ~2002, v1.1 2003) → modular architecture lifted out at Trond Lossius's suggestion "over shrimp on the west coast of Norway" → BEK Bergen residency Oct 2003 → TTBlue ("TapTools Blue," SourceForge, open-sourced 2005, LGPL) → Jamoma launched March 2005 → Hipno (Cycling '74, built on Pluggo, ~$199, announced Oct 2005). Electrotap co-founded with Jesse Allison; the Teabox (ICMC 2004).*

7. **Jade.** Describe Jade in your own words — what it did, why you built it, who used it. It's barely documented now but it's the seed of everything. What did it feel like to use?
8. ★ **The shrimp meal.** This anecdote keeps surfacing — Lossius suggesting you lift the modular architecture out of Jade, over a meal of shrimp on the Norwegian coast. What actually happened? Where were you, what was said, did you know it mattered at the time?
9. **BEK & Bergen.** How did an American end up at Bergen senter for elektronisk kunst in 2003? What was that residency, who was there, and how did it change the work?
10. **"TTBlue" = "TapTools Blue."** Confirmed via the SourceForge mailing list name. Why "Blue"? What's the story of the name, and of deciding to open-source the DSP library that your commercial product depended on?
11. **The doubled life.** For years you were simultaneously author of TapTools (the externals), TTBlue/Jamoma (the library under them), *and* working in the broader Max world. How did you hold those together? Did the open-source Jamoma work ever conflict with the commercial Tap.Tools/Hipno work?
12. **Hipno — the deal.** ★ Darwin Grosse (creativesynth.com) is credited with connecting your `tap.plug.*` work to what became Hipno. How did the Cycling '74 deal actually come together? What was your relationship with Grosse and with David Zicarelli then?
13. **Hipno — the build.** Your own line (from the C74 interview) calls it "a really unfocused effort to leverage a bunch of 3rd-party Max work… into plug-ins." What was building Hipno like — with Jesse Allison and Nathan Wolek, on the Pluggo platform? The Hipnoscope UI? Crunch, deadlines, NAMM?
14. **Hipno — the end.** Cycling '74 discontinued Hipno (with Pluggo/Mode/UpMix) in May 2009 because it depended on Pluggo. How did that feel — and what did it teach you about building on someone else's platform (a lesson that echoes in the 2026 Jamoma decision)?
15. **Electrotap & the Teabox.** What was Electrotap, really? How did the company work with Jesse Allison, and what was it like shipping *hardware* (the Teabox sensor interface) as a small team?

## Act 3 — Open source & the long retirement (2010–2020)

*What we have: v3.1 decoupled from Jamoma ("completely untethered"), a Pro edition for Max-for-Live; v4 (2013) re-coupled to Jamoma 0.5.7, dropped the registration key/installer, went New BSD "on the honor system," became a Max Package; then the 2014-10-31 "great pruning" and a trickle to 2020. You joined Cycling '74 as a research engineer in this era.*

16. **Going free.** Why drop the registration keys and installer in 2013 and move to BSD + donations? Was it principle, exhaustion, market reality, or your changing role?
17. **Joining Cycling '74.** When and how did you join C74, and what did it mean to now work *inside* the company whose platform you'd been extending (and built Hipno with)? Did working there change how you felt about TapTools?
18. **The great pruning (Halloween 2014).** In about 90 minutes you deleted ~a dozen objects, one wry obituary each ("goodbye forever. you won't be missed."; "a bad idea that is finally going away."; "the low-res LFO is dead…"). What was that session — a deliberate cleanup, a mood, an evening? Did it feel like loss, relief, or housekeeping?
19. **"jesse's objects."** A commit removes `tap.pulseroute` as "another of jesse's objects rather than one of the original taptools." How did Jesse Allison's work get co-mingled into TapTools, and how did you think about what was "really" TapTools vs. Hipno-era shared code?
20. **Watching Max absorb you.** Object after object died because Cycling '74 shipped the feature natively (`average~`, `atodb~`, `buffer~ normalize`, `zl`, `gen~`). What's it like to build tools on a platform that keeps making your tools redundant — *especially once you worked there?*

## Act 4 — Resurrection & the present (2015–2026)

*What we have: four modernization runs — 2015 jamoma2 (one object), 2016 CMake/AppVeyor Windows attempt, 2016–2019 Cycling '74's own `taptools-min` Min port (then deleted), and the 2026 full revival that finally cuts Jamoma loose. You're now at Garmin (since Oct 2020).*

21. **Coming back, again and again.** You've returned to revive TapTools roughly every few years for a decade — 2015, 2016, 2019, 2026. What keeps pulling you back? What did each attempt run out of (time, tooling, motivation, a dead dependency)?
22. **Cycling '74's `taptools-min`.** C74 quietly maintained a Min port of TapTools (2016–2019) and then deleted it. Were you involved? How did it feel to find it abandoned and salvage it into the 2026 revival?
23. **The Jamoma decision.** The 2026 revival's defining choice is to *cut Jamoma entirely* and reimplement DSP standalone — abandoning the library lineage you spent years building. What's it like to deliberately leave behind TTBlue/Jamoma? Closure, or a small grief?
24. **Meeting your younger self.** Porting forced a close reread of 15-year-old code — and turned up real bugs you shipped (`tap.random~`, `tap.buffer.snap~`, `tap.fft.normalize~`, `tap.comb~`) and honest hacks ("THIS OBJECT FARTS LIKE MAD…", "THE PLUS 1 IS A HACK"). What's it like to audit your younger self? Any object you were proud or embarrassed to reread?
25. ★ **The actual question.** You've said the revival is an experiment, not a commitment — you're trying to figure out if reviving TapTools has value, and in what form. In your gut: who is it *for* now? Yourself? A few old users? A historical record? A teaching artifact? What would make it feel worth it?

## Cross-cutting — people, voice, feeling

26. **The credits roster.** Walk through the people in the "special thanks": ★ who are **"jhno"** (the `limi~` algorithm) and **Stephan Moore** (tap.rotate) and **Russell Pinkston** (tap.random~/counter~)? What's a memory of each that explains why their name is in the code?
27. **Ali Momeni.** `tap.jit.ali` is named for him, from his NIME'03 patches. What's the story — the conference, the collaboration, why an object carries his name?
28. **The naming aesthetic.** The puns are deliberate — *elixir*, *procrastinate~*, *inquisitor*, *sift~/sieve*, *gang*, *snap*. Where did that sensibility come from? A favorite naming you're still pleased with?
29. **The commit-message voice.** The git log is unusually candid and funny. Were you writing for anyone, or just to yourself? Did you know you were leaving a record?
30. **One artifact.** Is there a box, a hard drive, an old laptop, an email account, a folder of installers, screenshots, or correspondence from the Silicon Prairie / Electrotap / 74 Objects years that still exists? (Even a directory listing would be a major primary-source recovery the web can't give us.)
