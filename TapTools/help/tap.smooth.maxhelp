{
	"patcher" : 	{
		"fileversion" : 1,
		"rect" : [ 199.0, 69.0, 434.0, 571.0 ],
		"bglocked" : 0,
		"defrect" : [ 199.0, 69.0, 434.0, 571.0 ],
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
		"boxes" : [ 			{
				"box" : 				{
					"maxclass" : "bpatcher",
					"numoutlets" : 0,
					"id" : "obj-69",
					"name" : "tap.badge.maxpat",
					"args" : [  ],
					"numinlets" : 0,
					"patching_rect" : [ 188.0, 487.0, 225.0, 67.0 ]
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "newobj",
					"text" : "autohelp",
					"numoutlets" : 0,
					"fontsize" : 10.0,
					"hidden" : 1,
					"id" : "obj-52",
					"fontname" : "Verdana",
					"numinlets" : 1,
					"patching_rect" : [ 24.0, 495.0, 54.0, 19.0 ],
					"frozen_object_attributes" : 					{
						"helpfontname" : "Verdana",
						"helpfontsize" : 10.0
					}

				}

			}
, 			{
				"box" : 				{
					"maxclass" : "comment",
					"varname" : "autohelp_see_title",
					"text" : "See Also:",
					"textcolor" : [ 0.0, 0.0, 0.0, 1.0 ],
					"numoutlets" : 0,
					"fontface" : 1,
					"fontsize" : 11.595187,
					"frgb" : [ 0.0, 0.0, 0.0, 1.0 ],
					"id" : "obj-66",
					"fontname" : "Arial",
					"numinlets" : 1,
					"patching_rect" : [ 29.0, 499.0, 100.0, 20.0 ]
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "umenu",
					"varname" : "autohelp_see_menu",
					"numoutlets" : 3,
					"fontsize" : 11.595187,
					"items" : [ "(Objects:)", ",", "onepole~", ",", "<separator>" ],
					"outlettype" : [ "int", "", "" ],
					"id" : "obj-67",
					"fontname" : "Arial",
					"types" : [  ],
					"numinlets" : 1,
					"patching_rect" : [ 29.0, 519.0, 130.0, 20.0 ]
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "newobj",
					"text" : "sel 1",
					"numoutlets" : 2,
					"fontsize" : 10.0,
					"hidden" : 1,
					"outlettype" : [ "bang", "" ],
					"id" : "obj-9",
					"fontname" : "Verdana",
					"numinlets" : 2,
					"patching_rect" : [ 48.0, 303.0, 32.0, 19.0 ]
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "newobj",
					"text" : "> 30.",
					"numoutlets" : 1,
					"fontsize" : 10.0,
					"hidden" : 1,
					"outlettype" : [ "int" ],
					"id" : "obj-10",
					"fontname" : "Verdana",
					"numinlets" : 2,
					"patching_rect" : [ 48.0, 282.0, 38.0, 19.0 ]
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "comment",
					"text" : "<-reset the generator",
					"linecount" : 2,
					"numoutlets" : 0,
					"fontsize" : 10.0,
					"frgb" : [ 0.333333, 0.333333, 0.333333, 1.0 ],
					"id" : "obj-11",
					"fontname" : "Verdana",
					"numinlets" : 1,
					"patching_rect" : [ 116.0, 231.0, 66.0, 31.0 ]
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "newobj",
					"text" : "tap.loadbang 0 0.9",
					"numoutlets" : 2,
					"fontsize" : 10.0,
					"hidden" : 1,
					"outlettype" : [ "bang", "" ],
					"id" : "obj-12",
					"fontname" : "Verdana",
					"numinlets" : 1,
					"patching_rect" : [ 128.0, 357.0, 104.0, 19.0 ]
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "comment",
					"text" : "0. is no smoothing, and 1. is perfectly smooth (no changes). The default is 0.75.",
					"linecount" : 3,
					"numoutlets" : 0,
					"fontsize" : 10.0,
					"frgb" : [ 0.333333, 0.333333, 0.333333, 1.0 ],
					"id" : "obj-13",
					"fontname" : "Verdana",
					"numinlets" : 1,
					"patching_rect" : [ 157.0, 316.0, 173.0, 43.0 ]
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "comment",
					"text" : "generate a random number between 0 and 1",
					"linecount" : 3,
					"numoutlets" : 0,
					"fontsize" : 10.0,
					"frgb" : [ 0.333333, 0.333333, 0.333333, 1.0 ],
					"id" : "obj-14",
					"fontname" : "Verdana",
					"numinlets" : 1,
					"patching_rect" : [ 95.0, 159.0, 93.0, 43.0 ]
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "comment",
					"text" : "turn on the metro",
					"numoutlets" : 0,
					"fontsize" : 10.0,
					"frgb" : [ 0.333333, 0.333333, 0.333333, 1.0 ],
					"id" : "obj-15",
					"fontname" : "Verdana",
					"numinlets" : 1,
					"patching_rect" : [ 107.0, 112.0, 98.0, 19.0 ]
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "message",
					"text" : "0",
					"numoutlets" : 1,
					"fontsize" : 10.0,
					"outlettype" : [ "" ],
					"id" : "obj-16",
					"fontname" : "Verdana",
					"numinlets" : 2,
					"patching_rect" : [ 84.0, 230.0, 17.0, 17.0 ]
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "comment",
					"text" : "adjust the smoothing coefficient",
					"numoutlets" : 0,
					"fontsize" : 12.0,
					"frgb" : [ 0.333333, 0.333333, 0.333333, 1.0 ],
					"id" : "obj-17",
					"fontname" : "Verdana",
					"numinlets" : 1,
					"patching_rect" : [ 109.0, 296.0, 228.0, 21.0 ]
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "flonum",
					"maximum" : 1.0,
					"numoutlets" : 2,
					"fontsize" : 12.0,
					"outlettype" : [ "float", "bang" ],
					"triscale" : 0.9,
					"id" : "obj-18",

					"fontname" : "Verdana",
					"minimum" : 0.0,
					"numinlets" : 1,
					"patching_rect" : [ 111.0, 317.0, 40.0, 21.0 ],
					"htextcolor" : [ 0.870588, 0.870588, 0.870588, 1.0 ]
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "newobj",
					"text" : "tap.thru",
					"numoutlets" : 1,
					"fontsize" : 10.0,
					"outlettype" : [ "float" ],
					"id" : "obj-19",
					"fontname" : "Verdana",
					"numinlets" : 1,
					"patching_rect" : [ 51.0, 208.0, 49.0, 19.0 ]
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "newobj",
					"text" : "+ 0.",
					"numoutlets" : 1,
					"fontsize" : 10.0,
					"outlettype" : [ "float" ],
					"id" : "obj-20",
					"fontname" : "Verdana",
					"numinlets" : 2,
					"patching_rect" : [ 34.0, 229.0, 30.0, 19.0 ]
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "multislider",
					"bgcolor" : [ 0.352941, 0.415686, 0.556863, 1.0 ],
					"numoutlets" : 2,
					"candicane2" : [ 0.145098, 0.203922, 0.356863, 1.0 ],
					"candicane5" : [ 0.584314, 0.827451, 0.431373, 1.0 ],
					"outlettype" : [ "", "" ],
					"candicane4" : [ 0.439216, 0.619608, 0.070588, 1.0 ],
					"slidercolor" : [ 1.0, 0.733333, 0.356863, 1.0 ],
					"peakcolor" : [ 0.498039, 0.498039, 0.498039, 1.0 ],
					"compatibility" : 1,
					"id" : "obj-21",
					"setstyle" : 3,
					"candicane6" : [ 0.733333, 0.035294, 0.788235, 1.0 ],
					"setminmax" : [ -20.0, 20.0 ],
					"candicane3" : [ 0.290196, 0.411765, 0.713726, 1.0 ],
					"numinlets" : 1,
					"patching_rect" : [ 192.0, 146.0, 199.0, 69.0 ],
					"candicane7" : [ 0.878431, 0.243137, 0.145098, 1.0 ],
					"candicane8" : [ 0.027451, 0.447059, 0.501961, 1.0 ]
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "toggle",
					"numoutlets" : 1,
					"outlettype" : [ "int" ],
					"id" : "obj-22",
					"numinlets" : 1,
					"patching_rect" : [ 34.0, 135.0, 15.0, 15.0 ]
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "newobj",
					"text" : "metro 5",
					"numoutlets" : 1,
					"fontsize" : 10.0,
					"outlettype" : [ "bang" ],
					"id" : "obj-23",
					"fontname" : "Verdana",
					"numinlets" : 2,
					"patching_rect" : [ 34.0, 154.0, 48.0, 19.0 ]
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "newobj",
					"text" : "tap.random",
					"numoutlets" : 2,
					"fontsize" : 10.0,
					"outlettype" : [ "float", "" ],
					"id" : "obj-24",
					"fontname" : "Verdana",
					"numinlets" : 1,
					"patching_rect" : [ 34.0, 175.0, 67.0, 19.0 ]
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "flonum",
					"numoutlets" : 2,
					"fontsize" : 10.0,
					"outlettype" : [ "float", "bang" ],
					"triscale" : 0.9,
					"id" : "obj-25",

					"fontname" : "Verdana",
					"numinlets" : 1,
					"patching_rect" : [ 34.0, 373.0, 53.0, 19.0 ],
					"htextcolor" : [ 0.870588, 0.870588, 0.870588, 1.0 ]
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "flonum",
					"numoutlets" : 2,
					"fontsize" : 10.0,
					"outlettype" : [ "float", "bang" ],
					"triscale" : 0.9,
					"id" : "obj-26",

					"fontname" : "Verdana",
					"numinlets" : 1,
					"patching_rect" : [ 34.0, 253.0, 49.0, 19.0 ],
					"htextcolor" : [ 0.870588, 0.870588, 0.870588, 1.0 ]
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "multislider",
					"bgcolor" : [ 0.352941, 0.415686, 0.556863, 1.0 ],
					"numoutlets" : 2,
					"candicane2" : [ 0.145098, 0.203922, 0.356863, 1.0 ],
					"candicane5" : [ 0.584314, 0.827451, 0.431373, 1.0 ],
					"outlettype" : [ "", "" ],
					"candicane4" : [ 0.439216, 0.619608, 0.070588, 1.0 ],
					"slidercolor" : [ 1.0, 0.733333, 0.356863, 1.0 ],
					"peakcolor" : [ 0.498039, 0.498039, 0.498039, 1.0 ],
					"compatibility" : 1,
					"id" : "obj-27",
					"setstyle" : 3,
					"candicane6" : [ 0.733333, 0.035294, 0.788235, 1.0 ],
					"setminmax" : [ -20.0, 20.0 ],
					"candicane3" : [ 0.290196, 0.411765, 0.713726, 1.0 ],
					"numinlets" : 1,
					"patching_rect" : [ 34.0, 395.0, 199.0, 69.0 ],
					"candicane7" : [ 0.878431, 0.243137, 0.145098, 1.0 ],
					"candicane8" : [ 0.027451, 0.447059, 0.501961, 1.0 ]
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "newobj",
					"text" : "tap.loadbang 0 1",
					"numoutlets" : 2,
					"fontsize" : 10.0,
					"outlettype" : [ "bang", "" ],
					"id" : "obj-28",
					"fontname" : "Verdana",
					"numinlets" : 1,
					"patching_rect" : [ 24.0, 111.0, 94.0, 19.0 ]
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "newobj",
					"text" : "tap.smooth",
					"numoutlets" : 1,
					"fontsize" : 14.249197,
					"outlettype" : [ "float" ],
					"id" : "obj-29",
					"fontname" : "Verdana",
					"color" : [ 1.0, 0.890196, 0.090196, 1.0 ],
					"numinlets" : 2,
					"patching_rect" : [ 34.0, 342.0, 90.0, 24.0 ]
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "panel",
					"varname" : "autohelp_see_panel",
					"bgcolor" : [ 0.85, 0.85, 0.85, 0.75 ],
					"numoutlets" : 0,
					"bordercolor" : [ 0.5, 0.5, 0.5, 0.75 ],
					"border" : 2,
					"id" : "obj-68",
					"background" : 1,
					"numinlets" : 1,
					"patching_rect" : [ 24.0, 495.0, 140.0, 50.0 ]
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "comment",
					"varname" : "autohelp_top_title",
					"text" : "tap.smooth",
					"textcolor" : [ 0.93, 0.93, 0.97, 1.0 ],
					"numoutlets" : 0,
					"fontface" : 3,
					"fontsize" : 20.871338,
					"frgb" : [ 0.93, 0.93, 0.97, 1.0 ],
					"id" : "obj-2",
					"fontname" : "Arial",
					"numinlets" : 1,
					"patching_rect" : [ 10.0, 8.0, 414.0, 30.0 ]
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "comment",
					"varname" : "autohelp_top_digest",
					"text" : "Data stream smoother",
					"textcolor" : [ 0.93, 0.93, 0.97, 1.0 ],
					"numoutlets" : 0,
					"fontsize" : 12.754705,
					"frgb" : [ 0.93, 0.93, 0.97, 1.0 ],
					"id" : "obj-3",
					"fontname" : "Arial",
					"numinlets" : 1,
					"patching_rect" : [ 10.0, 36.0, 414.0, 21.0 ]
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "comment",
					"varname" : "autohelp_top_description",
					"text" : "The tap.smooth object implements a simple 1-pole lowpass filter which is applied to the incoming data stream to massage the incoming data.",
					"linecount" : 2,
					"numoutlets" : 0,
					"fontsize" : 10.0,
					"frgb" : [ 0.333333, 0.333333, 0.333333, 1.0 ],
					"id" : "obj-4",
					"fontname" : "Verdana",
					"numinlets" : 1,
					"patching_rect" : [ 10.0, 57.0, 432.0, 31.0 ]
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "textbutton",
					"varname" : "autohelp_top_ref",
					"bgcolor" : [ 0.0, 0.0, 0.0, 0.0 ],
					"bgoveroncolor" : [ 0.0, 0.0, 0.0, 0.0 ],
					"underline" : 1,
					"textcolor" : [ 0.309, 0.395, 0.524, 1.0 ],
					"numoutlets" : 3,
					"fontface" : 3,
					"fontsize" : 12.754705,
					"spacing_x" : 0.0,
					"bgoncolor" : [ 0.0, 0.0, 0.0, 0.0 ],
					"outlettype" : [ "", "", "int" ],
					"bordercolor" : [ 0.0, 0.0, 0.0, 0.0 ],
					"spacing_y" : 0.0,
					"textoncolor" : [ 0.27, 0.35, 0.47, 1.0 ],
					"presentation_rect" : [ 0.0, 0.0, 171.718384, 14.666666 ],
					"id" : "obj-5",
					"fontlink" : 1,
					"text" : "open tap.smooth reference",
					"textovercolor" : [ 0.4, 0.5, 0.65, 1.0 ],
					"fontname" : "Arial",
					"bgovercolor" : [ 0.0, 0.0, 0.0, 0.0 ],
					"numinlets" : 1,
					"patching_rect" : [ 257.281616, 22.0, 171.718384, 14.666666 ]
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "panel",
					"varname" : "autohelp_top_panel",
					"grad2" : [ 0.85, 0.85, 0.85, 1.0 ],
					"numoutlets" : 0,
					"mode" : 1,
					"id" : "obj-6",
					"background" : 1,
					"grad1" : [ 0.27, 0.35, 0.47, 1.0 ],
					"numinlets" : 1,
					"patching_rect" : [ 5.0, 5.0, 424.0, 52.0 ]
				}

			}
 ],
		"lines" : [ 			{
				"patchline" : 				{
					"source" : [ "obj-26", 0 ],
					"destination" : [ "obj-21", 0 ],
					"hidden" : 0,
					"midpoints" : [ 43.5, 275.0, 185.0, 275.0, 185.0, 138.0, 201.5, 138.0 ]
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-18", 0 ],
					"destination" : [ "obj-29", 1 ],
					"hidden" : 0,
					"midpoints" : [  ]
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-12", 1 ],
					"destination" : [ "obj-18", 0 ],
					"hidden" : 1,
					"midpoints" : [  ]
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-9", 0 ],
					"destination" : [ "obj-16", 0 ],
					"hidden" : 1,
					"midpoints" : [  ]
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-16", 0 ],
					"destination" : [ "obj-20", 1 ],
					"hidden" : 0,
					"midpoints" : [  ]
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-19", 0 ],
					"destination" : [ "obj-20", 1 ],
					"hidden" : 0,
					"midpoints" : [  ]
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-20", 0 ],
					"destination" : [ "obj-19", 0 ],
					"hidden" : 0,
					"midpoints" : [ 43.5, 249.0, 100.0, 249.0, 100.0, 201.0, 60.5, 201.0 ]
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-10", 0 ],
					"destination" : [ "obj-9", 0 ],
					"hidden" : 1,
					"midpoints" : [  ]
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-26", 0 ],
					"destination" : [ "obj-10", 0 ],
					"hidden" : 1,
					"midpoints" : [  ]
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-25", 0 ],
					"destination" : [ "obj-27", 0 ],
					"hidden" : 0,
					"midpoints" : [  ]
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-29", 0 ],
					"destination" : [ "obj-25", 0 ],
					"hidden" : 0,
					"midpoints" : [  ]
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-26", 0 ],
					"destination" : [ "obj-29", 0 ],
					"hidden" : 0,
					"midpoints" : [  ]
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-20", 0 ],
					"destination" : [ "obj-26", 0 ],
					"hidden" : 0,
					"midpoints" : [  ]
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-24", 0 ],
					"destination" : [ "obj-20", 0 ],
					"hidden" : 0,
					"midpoints" : [  ]
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-23", 0 ],
					"destination" : [ "obj-24", 0 ],
					"hidden" : 0,
					"midpoints" : [  ]
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-22", 0 ],
					"destination" : [ "obj-23", 0 ],
					"hidden" : 0,
					"midpoints" : [  ]
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-28", 1 ],
					"destination" : [ "obj-22", 0 ],
					"hidden" : 0,
					"midpoints" : [ 108.5, 131.0, 43.0, 131.0 ]
				}

			}
 ]
	}

}
