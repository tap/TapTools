{
	"patcher" : 	{
		"fileversion" : 1,
		"rect" : [ 326.0, 80.0, 560.0, 640.0 ],
		"bglocked" : 0,
		"defrect" : [ 326.0, 80.0, 560.0, 640.0 ],
		"openrect" : [ 0.0, 0.0, 0.0, 0.0 ],
		"openinpresentation" : 0,
		"default_fontsize" : 11.0,
		"default_fontface" : 0,
		"default_fontname" : "Verdana",
		"gridonopen" : 0,
		"gridsize" : [ 5.0, 5.0 ],
		"gridsnaponopen" : 0,
		"toolbarvisible" : 1,
		"boxanimatetime" : 200,
		"imprint" : 0,
		"enablehscroll" : 1,
		"enablevscroll" : 1,
		"devicewidth" : 0.0,
		"boxes" : [ 			{
				"box" : 				{
					"background" : 1,
					"grad1" : [ 0.88, 0.98, 0.78, 1.0 ],
					"grad2" : [ 0.9, 0.9, 0.9, 1.0 ],
					"id" : "obj-27",
					"maxclass" : "panel",
					"mode" : 1,
					"numinlets" : 1,
					"numoutlets" : 0,
					"patching_rect" : [ 5.0, 5.0, 495.0, 52.0 ],
					"varname" : "autohelp_top_panel"
				}

			}
, 			{
				"box" : 				{
					"fontface" : 3,
					"fontname" : "Arial",
					"fontsize" : 20.871338,
					"frgb" : [ 0.2, 0.2, 0.2, 1.0 ],
					"id" : "obj-23",
					"maxclass" : "comment",
					"numinlets" : 1,
					"numoutlets" : 0,
					"patching_rect" : [ 10.0, 8.0, 485.0, 30.0 ],
					"text" : "tap.sustain~",
					"textcolor" : [ 0.2, 0.2, 0.2, 1.0 ],
					"varname" : "autohelp_top_title"
				}

			}
, 			{
				"box" : 				{
					"fontname" : "Arial",
					"fontsize" : 12.754705,
					"frgb" : [ 0.2, 0.2, 0.2, 1.0 ],
					"id" : "obj-24",
					"maxclass" : "comment",
					"numinlets" : 1,
					"numoutlets" : 0,
					"patching_rect" : [ 10.0, 36.0, 485.0, 21.0 ],
					"text" : "Capture recent audio into seamless sustaining loops",
					"textcolor" : [ 0.2, 0.2, 0.2, 1.0 ],
					"varname" : "autohelp_top_digest"
				}

			}
, 			{
				"box" : 				{
					"fontname" : "Verdana",
					"fontsize" : 10.0,
					"frgb" : [ 0.658824, 0.658824, 0.658824, 1.0 ],
					"id" : "obj-25",
					"linecount" : 2,
					"maxclass" : "comment",
					"numinlets" : 1,
					"numoutlets" : 0,
					"patching_rect" : [ 10.0, 57.0, 480.0, 31.0 ],
					"text" : "tap.sustain~ continuously records its input. On a bang the most recent material is trimmed to zero-crossings and played back as a crossfaded loop so it sustains without clicks. A bank of voices is round-robined so captures overlap.",
					"varname" : "autohelp_top_description"
				}

			}
, 			{
				"box" : 				{
					"fontname" : "Verdana",
					"fontsize" : 10.0,
					"frgb" : [ 0.658824, 0.658824, 0.658824, 1.0 ],
					"id" : "obj-c1",
					"maxclass" : "comment",
					"numinlets" : 1,
					"numoutlets" : 0,
					"patching_rect" : [ 175.0, 115.0, 280.0, 19.0 ],
					"text" : "white-noise source to record and sustain"
				}

			}
, 			{
				"box" : 				{
					"fontname" : "Verdana",
					"fontsize" : 10.0,
					"frgb" : [ 0.658824, 0.658824, 0.658824, 1.0 ],
					"id" : "obj-c2",
					"maxclass" : "comment",
					"numinlets" : 1,
					"numoutlets" : 0,
					"patching_rect" : [ 175.0, 175.0, 290.0, 19.0 ],
					"text" : "BANG captures the most recent audio into a voice"
				}

			}
, 			{
				"box" : 				{
					"fontname" : "Verdana",
					"fontsize" : 10.0,
					"frgb" : [ 0.658824, 0.658824, 0.658824, 1.0 ],
					"id" : "obj-c3",
					"maxclass" : "comment",
					"numinlets" : 1,
					"numoutlets" : 0,
					"patching_rect" : [ 250.0, 230.0, 250.0, 19.0 ],
					"text" : "voices: overlapping sustaining voices (1-5)"
				}

			}
, 			{
				"box" : 				{
					"fontname" : "Verdana",
					"fontsize" : 10.0,
					"frgb" : [ 0.658824, 0.658824, 0.658824, 1.0 ],
					"id" : "obj-c4",
					"maxclass" : "comment",
					"numinlets" : 1,
					"numoutlets" : 0,
					"patching_rect" : [ 250.0, 270.0, 250.0, 19.0 ],
					"text" : "rise: per-voice fade-in time in ms"
				}

			}
, 			{
				"box" : 				{
					"fontname" : "Verdana",
					"fontsize" : 10.0,
					"frgb" : [ 0.658824, 0.658824, 0.658824, 1.0 ],
					"id" : "obj-c5",
					"maxclass" : "comment",
					"numinlets" : 1,
					"numoutlets" : 0,
					"patching_rect" : [ 250.0, 310.0, 250.0, 19.0 ],
					"text" : "fade: crossfade across loop-points in ms"
				}

			}
, 			{
				"box" : 				{
					"fontname" : "Verdana",
					"fontsize" : 10.0,
					"frgb" : [ 0.658824, 0.658824, 0.658824, 1.0 ],
					"id" : "obj-c6",
					"linecount" : 2,
					"maxclass" : "comment",
					"numinlets" : 1,
					"numoutlets" : 0,
					"patching_rect" : [ 250.0, 350.0, 250.0, 31.0 ],
					"text" : "length: maximum captured-loop length in ms (changing it restarts the recording buffer)"
				}

			}
, 			{
				"box" : 				{
					"fontname" : "Verdana",
					"fontsize" : 10.0,
					"frgb" : [ 0.658824, 0.658824, 0.658824, 1.0 ],
					"id" : "obj-c7",
					"maxclass" : "comment",
					"numinlets" : 1,
					"numoutlets" : 0,
					"patching_rect" : [ 250.0, 415.0, 250.0, 19.0 ],
					"text" : "clear: erase all captured loops (every voice)"
				}

			}
, 			{
				"box" : 				{
					"id" : "obj-tog",
					"maxclass" : "toggle",
					"numinlets" : 1,
					"numoutlets" : 1,
					"outlettype" : [ "int" ],
					"patching_rect" : [ 40.0, 95.0, 20.0, 20.0 ]
				}

			}
, 			{
				"box" : 				{
					"id" : "obj-noise",
					"fontname" : "Verdana",
					"fontsize" : 10.0,
					"maxclass" : "newobj",
					"numinlets" : 1,
					"numoutlets" : 2,
					"outlettype" : [ "signal", "" ],
					"patching_rect" : [ 40.0, 120.0, 64.0, 19.0 ],
					"text" : "tap.noise~"
				}

			}
, 			{
				"box" : 				{
					"id" : "obj-bang",
					"maxclass" : "button",
					"numinlets" : 1,
					"numoutlets" : 1,
					"outlettype" : [ "bang" ],
					"patching_rect" : [ 40.0, 170.0, 24.0, 24.0 ]
				}

			}
, 			{
				"box" : 				{
					"id" : "obj-voicesnum",
					"fontname" : "Verdana",
					"fontsize" : 10.0,
					"htextcolor" : [ 0.870588, 0.870588, 0.870588, 1.0 ],
					"maxclass" : "number",
					"numinlets" : 1,
					"numoutlets" : 2,
					"outlettype" : [ "", "bang" ],
					"patching_rect" : [ 110.0, 225.0, 54.0, 19.0 ],
					"triscale" : 0.9
				}

			}
, 			{
				"box" : 				{
					"id" : "obj-voicesmsg",
					"fontname" : "Verdana",
					"fontsize" : 10.0,
					"maxclass" : "message",
					"numinlets" : 2,
					"numoutlets" : 1,
					"outlettype" : [ "" ],
					"patching_rect" : [ 110.0, 248.0, 70.0, 17.0 ],
					"text" : "voices $1"
				}

			}
, 			{
				"box" : 				{
					"id" : "obj-risenum",
					"fontname" : "Verdana",
					"fontsize" : 10.0,
					"htextcolor" : [ 0.870588, 0.870588, 0.870588, 1.0 ],
					"maxclass" : "flonum",
					"numinlets" : 1,
					"numoutlets" : 2,
					"outlettype" : [ "float", "bang" ],
					"patching_rect" : [ 110.0, 265.0, 54.0, 19.0 ],
					"triscale" : 0.9
				}

			}
, 			{
				"box" : 				{
					"id" : "obj-risemsg",
					"fontname" : "Verdana",
					"fontsize" : 10.0,
					"maxclass" : "message",
					"numinlets" : 2,
					"numoutlets" : 1,
					"outlettype" : [ "" ],
					"patching_rect" : [ 110.0, 288.0, 60.0, 17.0 ],
					"text" : "rise $1"
				}

			}
, 			{
				"box" : 				{
					"id" : "obj-fadenum",
					"fontname" : "Verdana",
					"fontsize" : 10.0,
					"htextcolor" : [ 0.870588, 0.870588, 0.870588, 1.0 ],
					"maxclass" : "flonum",
					"numinlets" : 1,
					"numoutlets" : 2,
					"outlettype" : [ "float", "bang" ],
					"patching_rect" : [ 110.0, 305.0, 54.0, 19.0 ],
					"triscale" : 0.9
				}

			}
, 			{
				"box" : 				{
					"id" : "obj-fademsg",
					"fontname" : "Verdana",
					"fontsize" : 10.0,
					"maxclass" : "message",
					"numinlets" : 2,
					"numoutlets" : 1,
					"outlettype" : [ "" ],
					"patching_rect" : [ 110.0, 328.0, 60.0, 17.0 ],
					"text" : "fade $1"
				}

			}
, 			{
				"box" : 				{
					"id" : "obj-lengthnum",
					"fontname" : "Verdana",
					"fontsize" : 10.0,
					"htextcolor" : [ 0.870588, 0.870588, 0.870588, 1.0 ],
					"maxclass" : "flonum",
					"numinlets" : 1,
					"numoutlets" : 2,
					"outlettype" : [ "float", "bang" ],
					"patching_rect" : [ 110.0, 345.0, 54.0, 19.0 ],
					"triscale" : 0.9
				}

			}
, 			{
				"box" : 				{
					"id" : "obj-lengthmsg",
					"fontname" : "Verdana",
					"fontsize" : 10.0,
					"maxclass" : "message",
					"numinlets" : 2,
					"numoutlets" : 1,
					"outlettype" : [ "" ],
					"patching_rect" : [ 110.0, 368.0, 70.0, 17.0 ],
					"text" : "length $1"
				}

			}
, 			{
				"box" : 				{
					"id" : "obj-clear",
					"fontname" : "Verdana",
					"fontsize" : 10.0,
					"maxclass" : "message",
					"numinlets" : 2,
					"numoutlets" : 1,
					"outlettype" : [ "" ],
					"patching_rect" : [ 200.0, 415.0, 40.0, 17.0 ],
					"text" : "clear"
				}

			}
, 			{
				"box" : 				{
					"color" : [ 1.0, 0.890196, 0.090196, 1.0 ],
					"id" : "obj-sustain",
					"fontname" : "Verdana",
					"fontsize" : 10.0,
					"maxclass" : "newobj",
					"numinlets" : 1,
					"numoutlets" : 1,
					"outlettype" : [ "signal" ],
					"patching_rect" : [ 40.0, 450.0, 100.0, 19.0 ],
					"text" : "tap.sustain~"
				}

			}
, 			{
				"box" : 				{
					"bgcolor" : [ 0.529412, 0.529412, 0.529412, 1.0 ],
					"id" : "obj-scope",
					"maxclass" : "scope~",
					"numinlets" : 2,
					"numoutlets" : 0,
					"patching_rect" : [ 200.0, 480.0, 130.0, 100.0 ],
					"rounded" : 0
				}

			}
, 			{
				"box" : 				{
					"id" : "obj-dac",
					"local" : 1,
					"maxclass" : "ezdac~",
					"numinlets" : 2,
					"numoutlets" : 0,
					"patching_rect" : [ 40.0, 530.0, 45.0, 45.0 ]
				}

			}
, 			{
				"box" : 				{
					"fontname" : "Verdana",
					"fontsize" : 10.0,
					"frgb" : [ 0.658824, 0.658824, 0.658824, 1.0 ],
					"id" : "obj-c8",
					"maxclass" : "comment",
					"numinlets" : 1,
					"numoutlets" : 0,
					"patching_rect" : [ 95.0, 535.0, 100.0, 19.0 ],
					"text" : "summed output"
				}

			}
, 			{
				"box" : 				{
					"args" : [  ],
					"id" : "obj-69",
					"maxclass" : "bpatcher",
					"name" : "tap.badge.maxpat",
					"numinlets" : 0,
					"numoutlets" : 0,
					"patching_rect" : [ 330.0, 570.0, 225.0, 67.0 ]
				}

			}
 ],
		"lines" : [ 			{
				"patchline" : 				{
					"destination" : [ "obj-noise", 0 ],
					"source" : [ "obj-tog", 0 ]
				}

			}
, 			{
				"patchline" : 				{
					"destination" : [ "obj-sustain", 0 ],
					"source" : [ "obj-noise", 0 ]
				}

			}
, 			{
				"patchline" : 				{
					"destination" : [ "obj-sustain", 0 ],
					"source" : [ "obj-bang", 0 ]
				}

			}
, 			{
				"patchline" : 				{
					"destination" : [ "obj-voicesmsg", 0 ],
					"source" : [ "obj-voicesnum", 0 ]
				}

			}
, 			{
				"patchline" : 				{
					"destination" : [ "obj-sustain", 0 ],
					"source" : [ "obj-voicesmsg", 0 ]
				}

			}
, 			{
				"patchline" : 				{
					"destination" : [ "obj-risemsg", 0 ],
					"source" : [ "obj-risenum", 0 ]
				}

			}
, 			{
				"patchline" : 				{
					"destination" : [ "obj-sustain", 0 ],
					"source" : [ "obj-risemsg", 0 ]
				}

			}
, 			{
				"patchline" : 				{
					"destination" : [ "obj-fademsg", 0 ],
					"source" : [ "obj-fadenum", 0 ]
				}

			}
, 			{
				"patchline" : 				{
					"destination" : [ "obj-sustain", 0 ],
					"source" : [ "obj-fademsg", 0 ]
				}

			}
, 			{
				"patchline" : 				{
					"destination" : [ "obj-lengthmsg", 0 ],
					"source" : [ "obj-lengthnum", 0 ]
				}

			}
, 			{
				"patchline" : 				{
					"destination" : [ "obj-sustain", 0 ],
					"source" : [ "obj-lengthmsg", 0 ]
				}

			}
, 			{
				"patchline" : 				{
					"destination" : [ "obj-sustain", 0 ],
					"source" : [ "obj-clear", 0 ]
				}

			}
, 			{
				"patchline" : 				{
					"destination" : [ "obj-dac", 0 ],
					"source" : [ "obj-sustain", 0 ]
				}

			}
, 			{
				"patchline" : 				{
					"destination" : [ "obj-dac", 1 ],
					"source" : [ "obj-sustain", 0 ]
				}

			}
, 			{
				"patchline" : 				{
					"destination" : [ "obj-scope", 0 ],
					"source" : [ "obj-sustain", 0 ]
				}

			}
 ]
	}

}
