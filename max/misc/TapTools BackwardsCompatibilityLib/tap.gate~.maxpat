{
	"patcher" : 	{
		"rect" : [ 193.0, 110.0, 415.0, 498.0 ],
		"bgcolor" : [ 0.1, 0.1, 0.1, 1.0 ],
		"bglocked" : 0,
		"defrect" : [ 193.0, 110.0, 415.0, 498.0 ],
		"openrect" : [ 0.0, 0.0, 0.0, 0.0 ],
		"openinpresentation" : 0,
		"default_fontsize" : 9.0,
		"default_fontface" : 0,
		"default_fontname" : "Verdana",
		"gridonopen" : 0,
		"gridsize" : [ 10.0, 5.0 ],
		"gridsnaponopen" : 0,
		"toolbarvisible" : 1,
		"boxanimatetime" : 200,
		"imprint" : 0,
		"metadata" : [  ],
		"boxes" : [ 			{
				"box" : 				{
					"maxclass" : "newobj",
					"text" : "tap.typecheck~",
					"fontsize" : 7.9,
					"numinlets" : 1,
					"patching_rect" : [ 29.0, 86.0, 92.0, 17.0 ],
					"numoutlets" : 8,
					"fontname" : "Geneva",
					"outlettype" : [ "signal", "bang", "int", "float", "list", "", "list", "" ],
					"id" : "obj-1"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "newobj",
					"text" : "* 0.5",
					"fontsize" : 7.9,
					"numinlets" : 2,
					"patching_rect" : [ 203.0, 188.0, 33.0, 17.0 ],
					"numoutlets" : 1,
					"fontname" : "Geneva",
					"outlettype" : [ "float" ],
					"id" : "obj-2"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "newobj",
					"text" : "rampsmooth~ 10000 500",
					"fontsize" : 7.9,
					"numinlets" : 3,
					"patching_rect" : [ 52.0, 366.0, 123.0, 17.0 ],
					"numoutlets" : 1,
					"fontname" : "Geneva",
					"outlettype" : [ "signal" ],
					"id" : "obj-3"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "newobj",
					"text" : "sig~ 100",
					"fontsize" : 7.9,
					"numinlets" : 1,
					"patching_rect" : [ 329.0, 237.0, 48.0, 17.0 ],
					"numoutlets" : 1,
					"fontname" : "Geneva",
					"outlettype" : [ "signal" ],
					"id" : "obj-4"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "newobj",
					"text" : "sig~ 0",
					"fontsize" : 7.9,
					"numinlets" : 1,
					"patching_rect" : [ 203.0, 236.0, 35.0, 17.0 ],
					"numoutlets" : 1,
					"fontname" : "Geneva",
					"outlettype" : [ "signal" ],
					"id" : "obj-5"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "newobj",
					"text" : "tap.split~ 0. 100.",
					"fontsize" : 7.9,
					"numinlets" : 3,
					"patching_rect" : [ 52.0, 275.0, 313.0, 17.0 ],
					"numoutlets" : 4,
					"fontname" : "Geneva",
					"outlettype" : [ "signal", "signal", "signal", "" ],
					"id" : "obj-6"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "comment",
					"text" : "threshold in dB",
					"fontsize" : 10.533333,
					"numinlets" : 1,
					"patching_rect" : [ 260.0, 102.0, 97.0, 21.0 ],
					"numoutlets" : 0,
					"fontname" : "Geneva",
					"frgb" : [ 0.658824, 0.658824, 0.658824, 1.0 ],
					"id" : "obj-7"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "flonum",
					"htextcolor" : [ 0.870588, 0.870588, 0.870588, 1.0 ],
					"bgcolor" : [ 0.866667, 0.866667, 0.866667, 1.0 ],
					"fontsize" : 7.9,
					"numinlets" : 1,
					"patching_rect" : [ 203.0, 102.0, 55.0, 17.0 ],
					"numoutlets" : 2,
					"fontname" : "Geneva",
					"outlettype" : [ "float", "bang" ],
					"id" : "obj-8",
					"triscale" : 0.9,
					"textcolor" : [ 0.0, 0.0, 0.0, 1.0 ]
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "newobj",
					"text" : "expr pow(10.\\,$f1\/20.)",
					"fontsize" : 7.9,
					"numinlets" : 1,
					"patching_rect" : [ 203.0, 160.0, 120.0, 17.0 ],
					"numoutlets" : 1,
					"fontname" : "Geneva",
					"outlettype" : [ "" ],
					"id" : "obj-9"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "comment",
					"text" : "(float) threshold(dB)",
					"linecount" : 2,
					"fontsize" : 10.533333,
					"numinlets" : 1,
					"patching_rect" : [ 203.0, 29.0, 92.0, 35.0 ],
					"numoutlets" : 0,
					"fontname" : "Geneva",
					"frgb" : [ 0.658824, 0.658824, 0.658824, 1.0 ],
					"id" : "obj-10"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "inlet",
					"numinlets" : 0,
					"patching_rect" : [ 203.0, 63.0, 18.0, 18.0 ],
					"numoutlets" : 1,
					"outlettype" : [ "" ],
					"id" : "obj-11",
					"comment" : "(float) Threshold in dB"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "comment",
					"text" : "threshold",
					"fontsize" : 10.533333,
					"numinlets" : 1,
					"patching_rect" : [ 260.0, 214.0, 63.0, 21.0 ],
					"numoutlets" : 0,
					"fontname" : "Geneva",
					"frgb" : [ 0.658824, 0.658824, 0.658824, 1.0 ],
					"id" : "obj-12"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "flonum",
					"htextcolor" : [ 0.870588, 0.870588, 0.870588, 1.0 ],
					"bgcolor" : [ 0.866667, 0.866667, 0.866667, 1.0 ],
					"fontsize" : 7.9,
					"numinlets" : 1,
					"patching_rect" : [ 203.0, 214.0, 55.0, 17.0 ],
					"numoutlets" : 2,
					"fontname" : "Geneva",
					"outlettype" : [ "float", "bang" ],
					"id" : "obj-13",
					"triscale" : 0.9,
					"textcolor" : [ 0.0, 0.0, 0.0, 1.0 ]
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "comment",
					"text" : "smoothing filter",
					"fontsize" : 10.533333,
					"numinlets" : 1,
					"patching_rect" : [ 176.0, 365.0, 107.0, 21.0 ],
					"numoutlets" : 0,
					"fontname" : "Geneva",
					"frgb" : [ 0.658824, 0.658824, 0.658824, 1.0 ],
					"id" : "obj-14"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "comment",
					"text" : "envelope detection",
					"linecount" : 2,
					"fontsize" : 10.533333,
					"numinlets" : 1,
					"patching_rect" : [ 81.0, 187.0, 64.0, 35.0 ],
					"numoutlets" : 0,
					"fontname" : "Geneva",
					"frgb" : [ 0.658824, 0.658824, 0.658824, 1.0 ],
					"id" : "obj-15"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "newobj",
					"text" : "tap.auto_thru~",
					"fontsize" : 7.9,
					"numinlets" : 2,
					"patching_rect" : [ 52.0, 148.0, 75.0, 17.0 ],
					"numoutlets" : 2,
					"fontname" : "Geneva",
					"outlettype" : [ "signal", "" ],
					"id" : "obj-16"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "comment",
					"text" : "(signal) key",
					"linecount" : 2,
					"fontsize" : 10.533333,
					"numinlets" : 1,
					"patching_rect" : [ 117.0, 29.0, 49.0, 35.0 ],
					"numoutlets" : 0,
					"fontname" : "Geneva",
					"frgb" : [ 0.658824, 0.658824, 0.658824, 1.0 ],
					"id" : "obj-17"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "inlet",
					"numinlets" : 0,
					"patching_rect" : [ 117.0, 63.0, 18.0, 18.0 ],
					"numoutlets" : 1,
					"outlettype" : [ "" ],
					"id" : "obj-18",
					"comment" : "(signal) External Key"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "comment",
					"text" : "(signal) input",
					"linecount" : 2,
					"fontsize" : 10.533333,
					"numinlets" : 1,
					"patching_rect" : [ 21.0, 28.0, 51.0, 35.0 ],
					"numoutlets" : 0,
					"fontname" : "Geneva",
					"frgb" : [ 0.658824, 0.658824, 0.658824, 1.0 ],
					"id" : "obj-19"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "inlet",
					"numinlets" : 0,
					"patching_rect" : [ 29.0, 63.0, 18.0, 18.0 ],
					"numoutlets" : 1,
					"outlettype" : [ "" ],
					"id" : "obj-20",
					"comment" : "(signal) Input"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "comment",
					"text" : "(signal) output",
					"fontsize" : 10.533333,
					"numinlets" : 1,
					"patching_rect" : [ 47.0, 423.0, 98.0, 21.0 ],
					"numoutlets" : 0,
					"fontname" : "Geneva",
					"frgb" : [ 0.658824, 0.658824, 0.658824, 1.0 ],
					"id" : "obj-21"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "newobj",
					"text" : "average~ 1000 rms",
					"fontsize" : 7.9,
					"numinlets" : 1,
					"patching_rect" : [ 52.0, 169.0, 98.0, 17.0 ],
					"numoutlets" : 1,
					"fontname" : "Geneva",
					"outlettype" : [ "signal" ],
					"id" : "obj-22"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "outlet",
					"numinlets" : 1,
					"patching_rect" : [ 29.0, 422.0, 17.0, 17.0 ],
					"numoutlets" : 0,
					"id" : "obj-23",
					"comment" : "(signal) Output"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "newobj",
					"text" : "*~",
					"fontsize" : 10.533333,
					"numinlets" : 2,
					"patching_rect" : [ 29.0, 393.0, 33.0, 21.0 ],
					"numoutlets" : 1,
					"fontname" : "Geneva",
					"outlettype" : [ "signal" ],
					"id" : "obj-24"
				}

			}
 ],
		"lines" : [ 			{
				"patchline" : 				{
					"source" : [ "obj-4", 0 ],
					"destination" : [ "obj-6", 2 ],
					"hidden" : 0
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-5", 0 ],
					"destination" : [ "obj-6", 1 ],
					"hidden" : 0
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-13", 0 ],
					"destination" : [ "obj-5", 0 ],
					"hidden" : 0
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-2", 0 ],
					"destination" : [ "obj-13", 0 ],
					"hidden" : 0
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-9", 0 ],
					"destination" : [ "obj-2", 0 ],
					"hidden" : 0
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-8", 0 ],
					"destination" : [ "obj-9", 0 ],
					"hidden" : 0
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-11", 0 ],
					"destination" : [ "obj-8", 0 ],
					"hidden" : 0
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-18", 0 ],
					"destination" : [ "obj-16", 1 ],
					"hidden" : 0
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-3", 0 ],
					"destination" : [ "obj-24", 1 ],
					"hidden" : 0
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-6", 2 ],
					"destination" : [ "obj-3", 0 ],
					"hidden" : 0
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-22", 0 ],
					"destination" : [ "obj-6", 0 ],
					"hidden" : 0
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-16", 0 ],
					"destination" : [ "obj-22", 0 ],
					"hidden" : 0
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-1", 0 ],
					"destination" : [ "obj-16", 0 ],
					"hidden" : 0
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-24", 0 ],
					"destination" : [ "obj-23", 0 ],
					"hidden" : 0
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-1", 0 ],
					"destination" : [ "obj-24", 0 ],
					"hidden" : 0
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-20", 0 ],
					"destination" : [ "obj-1", 0 ],
					"hidden" : 0
				}

			}
 ]
	}

}
