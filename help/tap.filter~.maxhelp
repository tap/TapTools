{
	"patcher" : 	{
		"fileversion" : 1,
		"rect" : [ 155.0, 44.0, 661.0, 700.0 ],
		"bglocked" : 0,
		"defrect" : [ 155.0, 44.0, 661.0, 700.0 ],
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
					"id" : "obj-94",
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
					"id" : "obj-79",
					"maxclass" : "comment",
					"numinlets" : 1,
					"numoutlets" : 0,
					"patching_rect" : [ 10.0, 8.0, 485.0, 30.0 ],
					"text" : "tap.filter~",
					"textcolor" : [ 0.2, 0.2, 0.2, 1.0 ],
					"varname" : "autohelp_top_title"
				}

			}
, 			{
				"box" : 				{
					"fontname" : "Arial",
					"fontsize" : 12.754705,
					"frgb" : [ 0.2, 0.2, 0.2, 1.0 ],
					"id" : "obj-80",
					"maxclass" : "comment",
					"numinlets" : 1,
					"numoutlets" : 0,
					"patching_rect" : [ 10.0, 36.0, 485.0, 21.0 ],
					"text" : "A unified multimode audio filter",
					"textcolor" : [ 0.2, 0.2, 0.2, 1.0 ],
					"varname" : "autohelp_top_digest"
				}

			}
, 			{
				"box" : 				{
					"fontname" : "Verdana",
					"fontsize" : 10.0,
					"frgb" : [ 0.658824, 0.658824, 0.658824, 1.0 ],
					"id" : "obj-81",
					"linecount" : 2,
					"maxclass" : "comment",
					"numinlets" : 1,
					"numoutlets" : 0,
					"patching_rect" : [ 10.0, 57.0, 509.0, 31.0 ],
					"text" : "The tap.filter~ object is a single flexible biquad whose filter type (mode) is selectable at runtime among the RBJ Audio EQ Cookbook shapes. Frequency, Q, and gain share one set of controls; frequency can also be modulated by a signal in the second inlet.",
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
					"patching_rect" : [ 230.0, 122.0, 220.0, 19.0 ],
					"text" : "Pick the filter type (mode)"
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
					"patching_rect" : [ 175.0, 175.0, 280.0, 19.0 ],
					"text" : "Cutoff / center frequency in Hz (Hz, default 1000)"
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
					"patching_rect" : [ 175.0, 222.0, 280.0, 19.0 ],
					"text" : "Q (resonance / steepness; shelf slope S for shelves)"
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
					"patching_rect" : [ 175.0, 269.0, 280.0, 19.0 ],
					"text" : "Gain in dB (peaking, lowshelf, highshelf only)"
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
					"patching_rect" : [ 175.0, 320.0, 290.0, 19.0 ],
					"text" : "A signal into the right inlet modulates the cutoff"
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
					"patching_rect" : [ 305.0, 430.0, 200.0, 31.0 ],
					"text" : "clear: reset the internal state (flush the delay memory) if the filter blows up"
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
					"patching_rect" : [ 175.0, 490.0, 220.0, 19.0 ],
					"text" : "white noise source to feed the filter"
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
					"patching_rect" : [ 40.0, 487.0, 64.0, 19.0 ],
					"text" : "tap.noise~"
				}

			}
, 			{
				"box" : 				{
					"id" : "obj-tog",
					"maxclass" : "toggle",
					"numinlets" : 1,
					"numoutlets" : 1,
					"outlettype" : [ "int" ],
					"patching_rect" : [ 40.0, 455.0, 20.0, 20.0 ]
				}

			}
, 			{
				"box" : 				{
					"id" : "obj-menu",
					"fontname" : "Verdana",
					"fontsize" : 10.0,
					"items" : [ "lowpass", ",", "highpass", ",", "bandpass", ",", "bandpass_constant_peak", ",", "notch", ",", "allpass", ",", "peaking", ",", "lowshelf", ",", "highshelf" ],
					"labelclick" : 1,
					"maxclass" : "umenu",
					"numinlets" : 1,
					"numoutlets" : 3,
					"outlettype" : [ "int", "", "" ],
					"patching_rect" : [ 40.0, 120.0, 170.0, 19.0 ],
					"types" : [  ]
				}

			}
, 			{
				"box" : 				{
					"id" : "obj-modemsg",
					"fontname" : "Verdana",
					"fontsize" : 10.0,
					"maxclass" : "message",
					"numinlets" : 2,
					"numoutlets" : 1,
					"outlettype" : [ "" ],
					"patching_rect" : [ 40.0, 145.0, 75.0, 17.0 ],
					"text" : "mode $1"
				}

			}
, 			{
				"box" : 				{
					"id" : "obj-freqnum",
					"fontname" : "Verdana",
					"fontsize" : 10.0,
					"htextcolor" : [ 0.870588, 0.870588, 0.870588, 1.0 ],
					"maxclass" : "flonum",
					"numinlets" : 1,
					"numoutlets" : 2,
					"outlettype" : [ "float", "bang" ],
					"patching_rect" : [ 40.0, 172.0, 54.0, 19.0 ],
					"triscale" : 0.9
				}

			}
, 			{
				"box" : 				{
					"id" : "obj-freqmsg",
					"fontname" : "Verdana",
					"fontsize" : 10.0,
					"maxclass" : "message",
					"numinlets" : 2,
					"numoutlets" : 1,
					"outlettype" : [ "" ],
					"patching_rect" : [ 40.0, 195.0, 90.0, 17.0 ],
					"text" : "frequency $1"
				}

			}
, 			{
				"box" : 				{
					"id" : "obj-qnum",
					"fontname" : "Verdana",
					"fontsize" : 10.0,
					"htextcolor" : [ 0.870588, 0.870588, 0.870588, 1.0 ],
					"maxclass" : "flonum",
					"numinlets" : 1,
					"numoutlets" : 2,
					"outlettype" : [ "float", "bang" ],
					"patching_rect" : [ 40.0, 219.0, 54.0, 19.0 ],
					"triscale" : 0.9
				}

			}
, 			{
				"box" : 				{
					"id" : "obj-qmsg",
					"fontname" : "Verdana",
					"fontsize" : 10.0,
					"maxclass" : "message",
					"numinlets" : 2,
					"numoutlets" : 1,
					"outlettype" : [ "" ],
					"patching_rect" : [ 40.0, 242.0, 40.0, 17.0 ],
					"text" : "q $1"
				}

			}
, 			{
				"box" : 				{
					"id" : "obj-gainnum",
					"fontname" : "Verdana",
					"fontsize" : 10.0,
					"htextcolor" : [ 0.870588, 0.870588, 0.870588, 1.0 ],
					"maxclass" : "flonum",
					"numinlets" : 1,
					"numoutlets" : 2,
					"outlettype" : [ "float", "bang" ],
					"patching_rect" : [ 40.0, 266.0, 54.0, 19.0 ],
					"triscale" : 0.9
				}

			}
, 			{
				"box" : 				{
					"id" : "obj-gainmsg",
					"fontname" : "Verdana",
					"fontsize" : 10.0,
					"maxclass" : "message",
					"numinlets" : 2,
					"numoutlets" : 1,
					"outlettype" : [ "" ],
					"patching_rect" : [ 40.0, 289.0, 60.0, 17.0 ],
					"text" : "gain $1"
				}

			}
, 			{
				"box" : 				{
					"id" : "obj-lfonum",
					"fontname" : "Verdana",
					"fontsize" : 10.0,
					"htextcolor" : [ 0.870588, 0.870588, 0.870588, 1.0 ],
					"maxclass" : "flonum",
					"numinlets" : 1,
					"numoutlets" : 2,
					"outlettype" : [ "float", "bang" ],
					"patching_rect" : [ 40.0, 313.0, 54.0, 19.0 ],
					"triscale" : 0.9
				}

			}
, 			{
				"box" : 				{
					"id" : "obj-sig",
					"fontname" : "Verdana",
					"fontsize" : 10.0,
					"maxclass" : "newobj",
					"numinlets" : 1,
					"numoutlets" : 1,
					"outlettype" : [ "signal" ],
					"patching_rect" : [ 40.0, 337.0, 35.0, 19.0 ],
					"text" : "sig~"
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
					"patching_rect" : [ 255.0, 430.0, 40.0, 17.0 ],
					"text" : "clear"
				}

			}
, 			{
				"box" : 				{
					"color" : [ 1.0, 0.890196, 0.090196, 1.0 ],
					"id" : "obj-filter",
					"fontname" : "Verdana",
					"fontsize" : 10.0,
					"maxclass" : "newobj",
					"numinlets" : 2,
					"numoutlets" : 1,
					"outlettype" : [ "signal" ],
					"patching_rect" : [ 40.0, 525.0, 90.0, 19.0 ],
					"text" : "tap.filter~"
				}

			}
, 			{
				"box" : 				{
					"bgcolor" : [ 0.529412, 0.529412, 0.529412, 1.0 ],
					"id" : "obj-scope",
					"maxclass" : "scope~",
					"numinlets" : 2,
					"numoutlets" : 0,
					"patching_rect" : [ 200.0, 560.0, 130.0, 100.0 ],
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
					"patching_rect" : [ 40.0, 610.0, 45.0, 45.0 ]
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
					"patching_rect" : [ 95.0, 615.0, 200.0, 19.0 ],
					"text" : "filtered output to your speakers"
				}

			}
, 			{
				"box" : 				{
					"args" : [  ],
					"id" : "obj-75",
					"maxclass" : "bpatcher",
					"name" : "tap.badge.maxpat",
					"numinlets" : 0,
					"numoutlets" : 0,
					"patching_rect" : [ 360.0, 600.0, 225.0, 67.0 ]
				}

			}
 ],
		"lines" : [ 			{
				"patchline" : 				{
					"destination" : [ "obj-modemsg", 0 ],
					"source" : [ "obj-menu", 1 ]
				}

			}
, 			{
				"patchline" : 				{
					"destination" : [ "obj-filter", 0 ],
					"source" : [ "obj-modemsg", 0 ]
				}

			}
, 			{
				"patchline" : 				{
					"destination" : [ "obj-freqmsg", 0 ],
					"source" : [ "obj-freqnum", 0 ]
				}

			}
, 			{
				"patchline" : 				{
					"destination" : [ "obj-filter", 0 ],
					"source" : [ "obj-freqmsg", 0 ]
				}

			}
, 			{
				"patchline" : 				{
					"destination" : [ "obj-qmsg", 0 ],
					"source" : [ "obj-qnum", 0 ]
				}

			}
, 			{
				"patchline" : 				{
					"destination" : [ "obj-filter", 0 ],
					"source" : [ "obj-qmsg", 0 ]
				}

			}
, 			{
				"patchline" : 				{
					"destination" : [ "obj-gainmsg", 0 ],
					"source" : [ "obj-gainnum", 0 ]
				}

			}
, 			{
				"patchline" : 				{
					"destination" : [ "obj-filter", 0 ],
					"source" : [ "obj-gainmsg", 0 ]
				}

			}
, 			{
				"patchline" : 				{
					"destination" : [ "obj-sig", 0 ],
					"source" : [ "obj-lfonum", 0 ]
				}

			}
, 			{
				"patchline" : 				{
					"destination" : [ "obj-filter", 1 ],
					"source" : [ "obj-sig", 0 ]
				}

			}
, 			{
				"patchline" : 				{
					"destination" : [ "obj-filter", 0 ],
					"source" : [ "obj-clear", 0 ]
				}

			}
, 			{
				"patchline" : 				{
					"destination" : [ "obj-noise", 0 ],
					"source" : [ "obj-tog", 0 ]
				}

			}
, 			{
				"patchline" : 				{
					"destination" : [ "obj-filter", 0 ],
					"source" : [ "obj-noise", 0 ]
				}

			}
, 			{
				"patchline" : 				{
					"destination" : [ "obj-dac", 0 ],
					"source" : [ "obj-filter", 0 ]
				}

			}
, 			{
				"patchline" : 				{
					"destination" : [ "obj-dac", 1 ],
					"source" : [ "obj-filter", 0 ]
				}

			}
, 			{
				"patchline" : 				{
					"destination" : [ "obj-scope", 0 ],
					"source" : [ "obj-filter", 0 ]
				}

			}
 ]
	}

}
