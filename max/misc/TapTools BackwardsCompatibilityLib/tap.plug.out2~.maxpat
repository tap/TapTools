{
	"patcher" : 	{
		"rect" : [ 367.0, 232.0, 514.0, 502.0 ],
		"bgcolor" : [ 0.1, 0.1, 0.1, 1.0 ],
		"bglocked" : 0,
		"defrect" : [ 367.0, 232.0, 514.0, 502.0 ],
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
					"maxclass" : "comment",
					"text" : "Right Input",
					"fontsize" : 7.9,
					"numinlets" : 1,
					"patching_rect" : [ 118.0, 111.0, 62.0, 17.0 ],
					"numoutlets" : 0,
					"fontname" : "Geneva",
					"frgb" : [ 0.658824, 0.658824, 0.658824, 1.0 ],
					"id" : "obj-1"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "comment",
					"text" : "actually scale the signal",
					"fontsize" : 7.9,
					"numinlets" : 1,
					"patching_rect" : [ 135.0, 330.0, 135.0, 17.0 ],
					"numoutlets" : 0,
					"fontname" : "Geneva",
					"frgb" : [ 0.658824, 0.658824, 0.658824, 1.0 ],
					"id" : "obj-2"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "comment",
					"text" : "almost -1 to 1, just to be safe",
					"linecount" : 2,
					"fontsize" : 7.9,
					"numinlets" : 1,
					"patching_rect" : [ 212.0, 353.0, 78.0, 28.0 ],
					"numoutlets" : 0,
					"fontname" : "Geneva",
					"frgb" : [ 0.658824, 0.658824, 0.658824, 1.0 ],
					"id" : "obj-3"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "comment",
					"text" : "put it in range",
					"fontsize" : 7.9,
					"numinlets" : 1,
					"patching_rect" : [ 318.0, 176.0, 169.0, 17.0 ],
					"numoutlets" : 0,
					"fontname" : "Geneva",
					"frgb" : [ 0.658824, 0.658824, 0.658824, 1.0 ],
					"id" : "obj-4"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "newobj",
					"text" : "scale",
					"fontsize" : 7.9,
					"numinlets" : 6,
					"patching_rect" : [ 237.0, 174.0, 79.0, 17.0 ],
					"numoutlets" : 1,
					"fontname" : "Geneva",
					"outlettype" : [ "float" ],
					"id" : "obj-5"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "comment",
					"text" : "only use it if the attribute is set",
					"fontsize" : 7.9,
					"numinlets" : 1,
					"patching_rect" : [ 249.0, 196.0, 169.0, 17.0 ],
					"numoutlets" : 0,
					"fontname" : "Geneva",
					"frgb" : [ 0.658824, 0.658824, 0.658824, 1.0 ],
					"id" : "obj-6"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "comment",
					"text" : "find just the volume controller",
					"fontsize" : 7.9,
					"numinlets" : 1,
					"patching_rect" : [ 280.0, 155.0, 169.0, 17.0 ],
					"numoutlets" : 0,
					"fontname" : "Geneva",
					"frgb" : [ 0.658824, 0.658824, 0.658824, 1.0 ],
					"id" : "obj-7"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "newobj",
					"text" : "route 7",
					"fontsize" : 7.9,
					"numinlets" : 1,
					"patching_rect" : [ 237.0, 153.0, 42.0, 17.0 ],
					"numoutlets" : 2,
					"fontname" : "Geneva",
					"outlettype" : [ "", "" ],
					"id" : "obj-8"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "newobj",
					"text" : "r ---ctl",
					"fontsize" : 7.9,
					"numinlets" : 0,
					"patching_rect" : [ 237.0, 133.0, 44.0, 17.0 ],
					"numoutlets" : 1,
					"fontname" : "Geneva",
					"outlettype" : [ "" ],
					"id" : "obj-9"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "toggle",
					"numinlets" : 1,
					"patching_rect" : [ 199.0, 129.0, 15.0, 15.0 ],
					"numoutlets" : 1,
					"outlettype" : [ "int" ],
					"id" : "obj-10"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "newobj",
					"text" : "route midivol",
					"fontsize" : 7.9,
					"numinlets" : 1,
					"patching_rect" : [ 199.0, 108.0, 69.0, 17.0 ],
					"numoutlets" : 2,
					"fontname" : "Geneva",
					"outlettype" : [ "", "" ],
					"id" : "obj-11"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "newobj",
					"text" : "gate",
					"fontsize" : 7.9,
					"numinlets" : 2,
					"patching_rect" : [ 199.0, 195.0, 48.0, 17.0 ],
					"numoutlets" : 1,
					"fontname" : "Geneva",
					"outlettype" : [ "" ],
					"id" : "obj-12"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "newobj",
					"text" : "patcherargs 1 @midivol 1",
					"fontsize" : 7.9,
					"numinlets" : 1,
					"patching_rect" : [ 66.0, 87.0, 143.0, 17.0 ],
					"numoutlets" : 2,
					"fontname" : "Geneva",
					"outlettype" : [ "", "" ],
					"id" : "obj-13"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "newobj",
					"text" : "*~",
					"fontsize" : 7.9,
					"numinlets" : 2,
					"patching_rect" : [ 53.0, 329.0, 27.0, 17.0 ],
					"numoutlets" : 1,
					"fontname" : "Geneva",
					"outlettype" : [ "signal" ],
					"id" : "obj-14"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "newobj",
					"text" : "*~",
					"fontsize" : 7.9,
					"numinlets" : 2,
					"patching_rect" : [ 102.0, 329.0, 27.0, 17.0 ],
					"numoutlets" : 1,
					"fontname" : "Geneva",
					"outlettype" : [ "signal" ],
					"id" : "obj-15"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "message",
					"text" : "$1 20",
					"fontsize" : 7.9,
					"numinlets" : 2,
					"patching_rect" : [ 222.0, 276.0, 35.0, 15.0 ],
					"numoutlets" : 1,
					"fontname" : "Geneva",
					"outlettype" : [ "" ],
					"id" : "obj-16"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "newobj",
					"text" : "line~",
					"fontsize" : 7.9,
					"numinlets" : 2,
					"patching_rect" : [ 222.0, 296.0, 31.0, 17.0 ],
					"numoutlets" : 2,
					"fontname" : "Geneva",
					"outlettype" : [ "signal", "bang" ],
					"id" : "obj-17"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "flonum",
					"htextcolor" : [ 0.870588, 0.870588, 0.870588, 1.0 ],
					"bgcolor" : [ 0.866667, 0.866667, 0.866667, 1.0 ],
					"fontsize" : 7.9,
					"numinlets" : 1,
					"patching_rect" : [ 222.0, 249.0, 35.0, 17.0 ],
					"numoutlets" : 2,
					"fontname" : "Geneva",
					"outlettype" : [ "float", "bang" ],
					"id" : "obj-18",
					"triscale" : 0.9,
					"textcolor" : [ 0.0, 0.0, 0.0, 1.0 ]
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "newobj",
					"text" : "pp #1 fixed c5 Volume dB",
					"fontsize" : 7.9,
					"numinlets" : 1,
					"patching_rect" : [ 222.0, 224.0, 124.0, 17.0 ],
					"numoutlets" : 3,
					"fontname" : "Geneva",
					"outlettype" : [ "", "float", "" ],
					"id" : "obj-19",
					"save" : [ "#N", "pp", "$1", "fixed", "c5", "Volume", "dB", ";", "#PP", "text", "Set the Master Output Volume", ";" ]
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "toggle",
					"numinlets" : 1,
					"patching_rect" : [ 20.0, 404.0, 15.0, 15.0 ],
					"numoutlets" : 1,
					"outlettype" : [ "int" ],
					"id" : "obj-20"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "inlet",
					"numinlets" : 0,
					"patching_rect" : [ 102.0, 109.0, 15.0, 15.0 ],
					"numoutlets" : 1,
					"outlettype" : [ "" ],
					"id" : "obj-21",
					"comment" : "(signal~) right"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "newobj",
					"text" : "tap.typecheck~",
					"fontsize" : 7.9,
					"numinlets" : 1,
					"patching_rect" : [ 53.0, 60.0, 92.0, 17.0 ],
					"numoutlets" : 8,
					"fontname" : "Geneva",
					"outlettype" : [ "signal", "bang", "int", "float", "list", "", "list", "" ],
					"id" : "obj-22"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "inlet",
					"numinlets" : 0,
					"patching_rect" : [ 53.0, 42.0, 15.0, 15.0 ],
					"numoutlets" : 1,
					"outlettype" : [ "" ],
					"id" : "obj-23",
					"comment" : "(signal~) left, (toggle) dac~, (bang) initialize"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "newobj",
					"text" : "clip~ -0.9999 0.9999",
					"fontsize" : 7.9,
					"numinlets" : 3,
					"patching_rect" : [ 102.0, 373.0, 108.0, 17.0 ],
					"numoutlets" : 1,
					"fontname" : "Geneva",
					"outlettype" : [ "signal" ],
					"id" : "obj-24"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "newobj",
					"text" : "dac~",
					"fontsize" : 7.9,
					"numinlets" : 2,
					"patching_rect" : [ 53.0, 435.0, 59.0, 17.0 ],
					"numoutlets" : 0,
					"fontname" : "Geneva",
					"id" : "obj-25"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "newobj",
					"text" : "plugout~",
					"fontsize" : 7.9,
					"numinlets" : 2,
					"patching_rect" : [ 53.0, 401.0, 59.0, 17.0 ],
					"numoutlets" : 2,
					"fontname" : "Geneva",
					"outlettype" : [ "signal", "signal" ],
					"id" : "obj-26"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "comment",
					"text" : "receive our plugin's incoming controller data",
					"linecount" : 2,
					"fontsize" : 7.9,
					"numinlets" : 1,
					"patching_rect" : [ 283.0, 127.0, 136.0, 28.0 ],
					"numoutlets" : 0,
					"fontname" : "Geneva",
					"frgb" : [ 0.658824, 0.658824, 0.658824, 1.0 ],
					"id" : "obj-27"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "comment",
					"text" : "Volume parameter number is the second argument to the patch. It is \"fixed\" meaning it won't be randomized, and \"c5\" means that the slider knob is a different color (orange) than the others. Giving the dB label automagically deals with the fancy math for us.",
					"linecount" : 5,
					"fontsize" : 7.9,
					"numinlets" : 1,
					"patching_rect" : [ 265.0, 244.0, 225.0, 59.0 ],
					"numoutlets" : 0,
					"fontname" : "Geneva",
					"frgb" : [ 0.658824, 0.658824, 0.658824, 1.0 ],
					"id" : "obj-28"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "newobj",
					"text" : "clip~ -0.9999 0.9999",
					"fontsize" : 7.9,
					"numinlets" : 3,
					"patching_rect" : [ 53.0, 351.0, 108.0, 17.0 ],
					"numoutlets" : 1,
					"fontname" : "Geneva",
					"outlettype" : [ "signal" ],
					"id" : "obj-29"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "comment",
					"text" : "Left Input, control stuff",
					"fontsize" : 7.9,
					"numinlets" : 1,
					"patching_rect" : [ 71.0, 42.0, 127.0, 17.0 ],
					"numoutlets" : 0,
					"fontname" : "Geneva",
					"frgb" : [ 0.658824, 0.658824, 0.658824, 1.0 ],
					"id" : "obj-30"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "comment",
					"text" : "A handy object from Jitter so we can use attributes in a patcher",
					"linecount" : 2,
					"fontsize" : 7.9,
					"numinlets" : 1,
					"patching_rect" : [ 211.0, 78.0, 162.0, 28.0 ],
					"numoutlets" : 0,
					"fontname" : "Geneva",
					"frgb" : [ 0.658824, 0.658824, 0.658824, 1.0 ],
					"id" : "obj-31"
				}

			}
 ],
		"lines" : [ 			{
				"patchline" : 				{
					"source" : [ "obj-5", 0 ],
					"destination" : [ "obj-12", 1 ],
					"hidden" : 0
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-8", 0 ],
					"destination" : [ "obj-5", 0 ],
					"hidden" : 0
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-9", 0 ],
					"destination" : [ "obj-8", 0 ],
					"hidden" : 0
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-16", 0 ],
					"destination" : [ "obj-17", 0 ],
					"hidden" : 0
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-18", 0 ],
					"destination" : [ "obj-16", 0 ],
					"hidden" : 0
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-19", 0 ],
					"destination" : [ "obj-18", 0 ],
					"hidden" : 0
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-12", 0 ],
					"destination" : [ "obj-18", 0 ],
					"hidden" : 0,
					"color" : [ 0.6667, 0.6667, 0.6667, 1.0 ],
					"midpoints" : [ 208.5, 239.0 ]
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-18", 0 ],
					"destination" : [ "obj-19", 0 ],
					"hidden" : 0,
					"midpoints" : [ 231.5, 270.0, 219.0, 270.0, 219.0, 220.0, 231.5, 220.0 ]
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-10", 0 ],
					"destination" : [ "obj-12", 0 ],
					"hidden" : 0
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-11", 0 ],
					"destination" : [ "obj-10", 0 ],
					"hidden" : 0
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-13", 1 ],
					"destination" : [ "obj-11", 0 ],
					"hidden" : 0
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-17", 0 ],
					"destination" : [ "obj-15", 1 ],
					"hidden" : 0,
					"midpoints" : [ 231.5, 325.0, 119.5, 325.0 ]
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-26", 1 ],
					"destination" : [ "obj-25", 1 ],
					"hidden" : 0
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-24", 0 ],
					"destination" : [ "obj-26", 1 ],
					"hidden" : 0
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-15", 0 ],
					"destination" : [ "obj-24", 0 ],
					"hidden" : 0
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-21", 0 ],
					"destination" : [ "obj-15", 0 ],
					"hidden" : 0,
					"color" : [ 1.0, 0.890196, 0.090196, 1.0 ]
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-17", 0 ],
					"destination" : [ "obj-14", 1 ],
					"hidden" : 0,
					"midpoints" : [ 231.5, 321.0, 70.5, 321.0 ]
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-22", 1 ],
					"destination" : [ "obj-13", 0 ],
					"hidden" : 0
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-20", 0 ],
					"destination" : [ "obj-25", 0 ],
					"hidden" : 0,
					"color" : [ 0.6667, 0.6667, 0.6667, 1.0 ],
					"midpoints" : [ 29.0, 426.0, 62.5, 426.0 ]
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-26", 0 ],
					"destination" : [ "obj-25", 0 ],
					"hidden" : 0
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-29", 0 ],
					"destination" : [ "obj-26", 0 ],
					"hidden" : 0
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-14", 0 ],
					"destination" : [ "obj-29", 0 ],
					"hidden" : 0
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-22", 0 ],
					"destination" : [ "obj-14", 0 ],
					"hidden" : 0
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-23", 0 ],
					"destination" : [ "obj-22", 0 ],
					"hidden" : 0
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-22", 2 ],
					"destination" : [ "obj-20", 0 ],
					"hidden" : 0,
					"color" : [ 0.6667, 0.6667, 0.6667, 1.0 ],
					"midpoints" : [ 83.35714, 82.0, 29.0, 82.0 ]
				}

			}
 ]
	}

}
