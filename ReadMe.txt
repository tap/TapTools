Tap.Tools
Tap.Tools is a collection of objects for Max, MSP, and Jitter available from Electrotap @ http://electrotap.com .
_________________________________________________________________

Tap.Tools, including modules for Jamoma...

These modules require Tap.Tools version 3.0 or higher.  
Tap.Tools 3.0 is the first version that provides support for Max 5.


Modules from Jade
Prior to Jamoma, Electrotap distributed an environment called Jade (from which Jamoma is derived).  Jade included a number of modules in addition to the hosting environment.  A list of modules from Jade is provided below with the newer modules to assist in transitioning projects from Jade to Jamoma.


Jade Thesaurus
Here is a list of old Jade modules, and the new modules by which they have been superceeded.

Jamoma Modules:
	- bufplay.mod					> jmod.samplePlayer~
	- crossfade.mod 					> jmod.crossfade~
	- degrade.mod and degradeM.mod 	> jmod.degrade~
	- delay.mod						> jmod.delay~
	- delayfeed.mod 					> jmod.echo~
	- filter.mod and filterM.mod 			> jmod.filter~
	- filtercascade.mod and multi_filter.mod	> jmod.equalizer~
	- input.mod and input2.mod 			> jmod.input~
	- k.tick.mod 						> jmod.qmetro
	- normalizer.mod 					> jmod.limiter~
	- output.mod 						> jmod.output~
	- scopeM.mod and analysis.mod		> jmod.scope~
	- video.avg4.mod					> jmod.avg4%
	- video.edge.mod 					> jmod.edge%
	- video.emboss.mod				> jmod.emboss%
	- video.fluoride.mod				> jmod.fluoride%
	- video.input.mod 					> jmod.input%
	- video.mblur.mod					> jmod.mblur%
	- video.op.mod 					> jmod.op%
	- video.player.mod					> jmod.input% and jmod.output%
	- video.wake.mod 					> jmod.wake%

Tap.Tools Modules:
	- anticlick.mod					> tap.jmod.anticlick~
	- lfo.mod						> tap.jmod.yalfo~
	- pitch_shift.mod and harmonizer.mod 	> tap.jmod.harmonizer~
	- procrastinateMS.mod				> tap.jmod.procrastinate~
	- reverb.mod and reverb_lite.mod 		> tap.jmod.reverb~
	- spectral_warp.mod				> tap.jmod.warp~
	- sustainM.mod and sustainMS.mod	> tap.jmod.grabloop~

	* brush.mod						> jmod.brush~
	* chunkmaker.mod					> jmod.chunk~
	* granular.mod
	* granular-lite.mod
	* teabox.mod						> jmod.teabox~

* This module has not yet been ported to Jamoma

Known Issues
(none)

Feature Requests
- add a 'speed' parameter to tap.jmod.grabloop~


