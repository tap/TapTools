{
	"patcher" : 	{
		"fileversion" : 1,
		"rect" : [ 118.0, 62.0, 527.0, 644.0 ],
		"bglocked" : 0,
		"defrect" : [ 118.0, 62.0, 527.0, 644.0 ],
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
					"patching_rect" : [ 260.0, 569.0, 225.0, 67.0 ]
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
					"patching_rect" : [ 52.0, 577.0, 54.0, 19.0 ],
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
					"numoutlets" : 0,
					"fontface" : 1,
					"fontsize" : 11.595187,
					"frgb" : [ 0.658824, 0.658824, 0.658824, 1.0 ],
					"id" : "obj-66",
					"fontname" : "Arial",
					"numinlets" : 1,
					"patching_rect" : [ 57.0, 581.0, 100.0, 20.0 ]
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "umenu",
					"varname" : "autohelp_see_menu",
					"numoutlets" : 3,
					"fontsize" : 11.595187,
					"items" : [ "(Objects:)", ",", "tap.jit.motion", ",", "<separator>" ],
					"outlettype" : [ "int", "", "" ],
					"id" : "obj-67",
					"fontname" : "Arial",
					"types" : [  ],
					"numinlets" : 1,
					"patching_rect" : [ 57.0, 601.0, 130.0, 20.0 ]
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "comment",
					"text" : "An example using 4 regions in 2 x 2 grid:",
					"numoutlets" : 0,
					"fontsize" : 12.0,
					"frgb" : [ 0.658824, 0.658824, 0.658824, 1.0 ],
					"id" : "obj-8",
					"fontname" : "Verdana",
					"numinlets" : 1,
					"patching_rect" : [ 75.0, 177.0, 344.0, 21.0 ]
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "jit.fpsgui",
					"numoutlets" : 2,
					"fontsize" : 10.0,
					"outlettype" : [ "", "" ],
					"id" : "obj-10",
					"fontname" : "Verdana",
					"numinlets" : 1,
					"patching_rect" : [ 185.0, 203.0, 60.0, 35.0 ]
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "newobj",
					"text" : "tap.loadbang 0 1",
					"numoutlets" : 2,
					"fontsize" : 10.0,
					"hidden" : 1,
					"outlettype" : [ "bang", "" ],
					"id" : "obj-11",
					"fontname" : "Verdana",
					"numinlets" : 1,
					"patching_rect" : [ 280.0, 207.0, 94.0, 19.0 ]
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "newobj",
					"text" : "tap.jit.grayscale",
					"numoutlets" : 1,
					"fontsize" : 10.0,
					"outlettype" : [ "jit_matrix" ],
					"id" : "obj-12",
					"fontname" : "Verdana",
					"numinlets" : 1,
					"patching_rect" : [ 70.0, 217.0, 89.0, 19.0 ]
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "newobj",
					"text" : "tap.jit.source",
					"numoutlets" : 1,
					"fontsize" : 10.0,
					"outlettype" : [ "jit_matrix" ],
					"id" : "obj-13",
					"fontname" : "Verdana",
					"numinlets" : 1,
					"patching_rect" : [ 70.0, 198.0, 75.0, 19.0 ]
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "newobj",
					"text" : "gate",
					"numoutlets" : 1,
					"fontsize" : 10.0,
					"hidden" : 1,
					"outlettype" : [ "" ],
					"id" : "obj-14",
					"fontname" : "Verdana",
					"numinlets" : 2,
					"patching_rect" : [ 406.0, 445.0, 31.0, 19.0 ]
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "toggle",
					"numoutlets" : 1,
					"outlettype" : [ "int" ],
					"id" : "obj-15",
					"numinlets" : 1,
					"patching_rect" : [ 426.0, 445.0, 15.0, 15.0 ]
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "flonum",
					"numoutlets" : 2,
					"fontsize" : 10.0,
					"outlettype" : [ "float", "bang" ],
					"triscale" : 0.9,
					"id" : "obj-16",

					"fontname" : "Verdana",
					"numinlets" : 1,
					"patching_rect" : [ 349.0, 445.0, 57.0, 19.0 ],
					"htextcolor" : [ 0.870588, 0.870588, 0.870588, 1.0 ]
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "jit.pwindow",
					"numoutlets" : 2,
					"outlettype" : [ "", "" ],
					"depthbuffer" : 0,
					"id" : "obj-17",
					"numinlets" : 1,
					"patching_rect" : [ 264.0, 460.0, 80.0, 60.0 ]
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "multislider",
					"bgcolor" : [ 1.0, 0.945098, 0.490196, 1.0 ],
					"numoutlets" : 2,
					"candicane2" : [ 0.145098, 0.203922, 0.356863, 1.0 ],
					"candicane5" : [ 0.584314, 0.827451, 0.431373, 1.0 ],
					"outlettype" : [ "", "" ],
					"candicane4" : [ 0.439216, 0.619608, 0.070588, 1.0 ],
					"slidercolor" : [ 0.286275, 0.121569, 0.529412, 1.0 ],
					"peakcolor" : [ 0.498039, 0.498039, 0.498039, 1.0 ],
					"compatibility" : 1,
					"id" : "obj-18",
					"setstyle" : 3,
					"candicane6" : [ 0.733333, 0.035294, 0.788235, 1.0 ],
					"setminmax" : [ 0.0, 1.0 ],
					"candicane3" : [ 0.290196, 0.411765, 0.713726, 1.0 ],
					"numinlets" : 1,
					"patching_rect" : [ 349.0, 461.0, 92.0, 61.0 ],
					"candicane7" : [ 0.878431, 0.243137, 0.145098, 1.0 ],
					"candicane8" : [ 0.027451, 0.447059, 0.501961, 1.0 ]
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "newobj",
					"text" : "tap.jit.motion+",
					"numoutlets" : 4,
					"fontsize" : 10.0,
					"outlettype" : [ "jit_matrix", "float", "float", "float" ],
					"id" : "obj-19",
					"fontname" : "Verdana",
					"numinlets" : 1,
					"patching_rect" : [ 265.0, 424.0, 85.0, 19.0 ]
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "panel",
					"rounded" : 15,
					"numoutlets" : 0,
					"border" : 1,
					"id" : "obj-20",
					"numinlets" : 1,
					"patching_rect" : [ 257.0, 415.0, 192.0, 116.0 ],
					"shadow" : 1
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "newobj",
					"text" : "gate",
					"numoutlets" : 1,
					"fontsize" : 10.0,
					"hidden" : 1,
					"outlettype" : [ "" ],
					"id" : "obj-21",
					"fontname" : "Verdana",
					"numinlets" : 2,
					"patching_rect" : [ 210.0, 445.0, 31.0, 19.0 ]
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "toggle",
					"numoutlets" : 1,
					"outlettype" : [ "int" ],
					"id" : "obj-22",
					"numinlets" : 1,
					"patching_rect" : [ 230.0, 445.0, 15.0, 15.0 ]
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "flonum",
					"numoutlets" : 2,
					"fontsize" : 10.0,
					"outlettype" : [ "float", "bang" ],
					"triscale" : 0.9,
					"id" : "obj-23",

					"fontname" : "Verdana",
					"numinlets" : 1,
					"patching_rect" : [ 153.0, 445.0, 57.0, 19.0 ],
					"htextcolor" : [ 0.870588, 0.870588, 0.870588, 1.0 ]
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "jit.pwindow",
					"numoutlets" : 2,
					"outlettype" : [ "", "" ],
					"depthbuffer" : 0,
					"id" : "obj-24",
					"numinlets" : 1,
					"patching_rect" : [ 68.0, 460.0, 80.0, 60.0 ]
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "multislider",
					"bgcolor" : [ 1.0, 0.945098, 0.490196, 1.0 ],
					"numoutlets" : 2,
					"candicane2" : [ 0.145098, 0.203922, 0.356863, 1.0 ],
					"candicane5" : [ 0.584314, 0.827451, 0.431373, 1.0 ],
					"outlettype" : [ "", "" ],
					"candicane4" : [ 0.439216, 0.619608, 0.070588, 1.0 ],
					"slidercolor" : [ 0.286275, 0.121569, 0.529412, 1.0 ],
					"peakcolor" : [ 0.498039, 0.498039, 0.498039, 1.0 ],
					"compatibility" : 1,
					"id" : "obj-25",
					"setstyle" : 3,
					"candicane6" : [ 0.733333, 0.035294, 0.788235, 1.0 ],
					"setminmax" : [ 0.0, 1.0 ],
					"candicane3" : [ 0.290196, 0.411765, 0.713726, 1.0 ],
					"numinlets" : 1,
					"patching_rect" : [ 153.0, 461.0, 92.0, 61.0 ],
					"candicane7" : [ 0.878431, 0.243137, 0.145098, 1.0 ],
					"candicane8" : [ 0.027451, 0.447059, 0.501961, 1.0 ]
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "newobj",
					"text" : "tap.jit.motion+",
					"numoutlets" : 4,
					"fontsize" : 10.0,
					"outlettype" : [ "jit_matrix", "float", "float", "float" ],
					"id" : "obj-26",
					"fontname" : "Verdana",
					"numinlets" : 1,
					"patching_rect" : [ 69.0, 424.0, 85.0, 19.0 ]
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "panel",
					"rounded" : 15,
					"numoutlets" : 0,
					"border" : 1,
					"id" : "obj-27",
					"numinlets" : 1,
					"patching_rect" : [ 61.0, 415.0, 192.0, 116.0 ],
					"shadow" : 1
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "newobj",
					"text" : "gate",
					"numoutlets" : 1,
					"fontsize" : 10.0,
					"hidden" : 1,
					"outlettype" : [ "" ],
					"id" : "obj-28",
					"fontname" : "Verdana",
					"numinlets" : 2,
					"patching_rect" : [ 407.0, 308.0, 31.0, 19.0 ]
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "toggle",
					"numoutlets" : 1,
					"outlettype" : [ "int" ],
					"id" : "obj-29",
					"numinlets" : 1,
					"patching_rect" : [ 427.0, 308.0, 15.0, 15.0 ]
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "flonum",
					"numoutlets" : 2,
					"fontsize" : 10.0,
					"outlettype" : [ "float", "bang" ],
					"triscale" : 0.9,
					"id" : "obj-30",

					"fontname" : "Verdana",
					"numinlets" : 1,
					"patching_rect" : [ 350.0, 308.0, 57.0, 19.0 ],
					"htextcolor" : [ 0.870588, 0.870588, 0.870588, 1.0 ]
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "jit.pwindow",
					"numoutlets" : 2,
					"outlettype" : [ "", "" ],
					"depthbuffer" : 0,
					"id" : "obj-31",
					"numinlets" : 1,
					"patching_rect" : [ 265.0, 323.0, 80.0, 60.0 ]
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "multislider",
					"bgcolor" : [ 1.0, 0.945098, 0.490196, 1.0 ],
					"numoutlets" : 2,
					"candicane2" : [ 0.145098, 0.203922, 0.356863, 1.0 ],
					"candicane5" : [ 0.584314, 0.827451, 0.431373, 1.0 ],
					"outlettype" : [ "", "" ],
					"candicane4" : [ 0.439216, 0.619608, 0.070588, 1.0 ],
					"slidercolor" : [ 0.286275, 0.121569, 0.529412, 1.0 ],
					"peakcolor" : [ 0.498039, 0.498039, 0.498039, 1.0 ],
					"compatibility" : 1,
					"id" : "obj-32",
					"setstyle" : 3,
					"candicane6" : [ 0.733333, 0.035294, 0.788235, 1.0 ],
					"setminmax" : [ 0.0, 1.0 ],
					"candicane3" : [ 0.290196, 0.411765, 0.713726, 1.0 ],
					"numinlets" : 1,
					"patching_rect" : [ 350.0, 324.0, 92.0, 61.0 ],
					"candicane7" : [ 0.878431, 0.243137, 0.145098, 1.0 ],
					"candicane8" : [ 0.027451, 0.447059, 0.501961, 1.0 ]
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "newobj",
					"text" : "tap.jit.motion+",
					"numoutlets" : 4,
					"fontsize" : 10.0,
					"outlettype" : [ "jit_matrix", "float", "float", "float" ],
					"id" : "obj-33",
					"fontname" : "Verdana",
					"numinlets" : 1,
					"patching_rect" : [ 266.0, 287.0, 85.0, 19.0 ]
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "panel",
					"rounded" : 15,
					"numoutlets" : 0,
					"border" : 1,
					"id" : "obj-34",
					"numinlets" : 1,
					"patching_rect" : [ 258.0, 278.0, 192.0, 116.0 ],
					"shadow" : 1
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "newobj",
					"text" : "gate",
					"numoutlets" : 1,
					"fontsize" : 10.0,
					"hidden" : 1,
					"outlettype" : [ "" ],
					"id" : "obj-35",
					"fontname" : "Verdana",
					"numinlets" : 2,
					"patching_rect" : [ 211.0, 308.0, 31.0, 19.0 ]
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "toggle",
					"numoutlets" : 1,
					"outlettype" : [ "int" ],
					"id" : "obj-36",
					"numinlets" : 1,
					"patching_rect" : [ 231.0, 308.0, 15.0, 15.0 ]
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "flonum",
					"numoutlets" : 2,
					"fontsize" : 10.0,
					"outlettype" : [ "float", "bang" ],
					"triscale" : 0.9,
					"id" : "obj-37",

					"fontname" : "Verdana",
					"numinlets" : 1,
					"patching_rect" : [ 154.0, 308.0, 57.0, 19.0 ],
					"htextcolor" : [ 0.870588, 0.870588, 0.870588, 1.0 ]
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "jit.pwindow",
					"numoutlets" : 2,
					"outlettype" : [ "", "" ],
					"depthbuffer" : 0,
					"id" : "obj-38",
					"numinlets" : 1,
					"patching_rect" : [ 69.0, 323.0, 80.0, 60.0 ]
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "multislider",
					"bgcolor" : [ 1.0, 0.945098, 0.490196, 1.0 ],
					"numoutlets" : 2,
					"candicane2" : [ 0.145098, 0.203922, 0.356863, 1.0 ],
					"candicane5" : [ 0.584314, 0.827451, 0.431373, 1.0 ],
					"outlettype" : [ "", "" ],
					"candicane4" : [ 0.439216, 0.619608, 0.070588, 1.0 ],
					"slidercolor" : [ 0.286275, 0.121569, 0.529412, 1.0 ],
					"peakcolor" : [ 0.498039, 0.498039, 0.498039, 1.0 ],
					"compatibility" : 1,
					"id" : "obj-39",
					"setstyle" : 3,
					"candicane6" : [ 0.733333, 0.035294, 0.788235, 1.0 ],
					"setminmax" : [ 0.0, 1.0 ],
					"candicane3" : [ 0.290196, 0.411765, 0.713726, 1.0 ],
					"numinlets" : 1,
					"patching_rect" : [ 154.0, 324.0, 92.0, 61.0 ],
					"candicane7" : [ 0.878431, 0.243137, 0.145098, 1.0 ],
					"candicane8" : [ 0.027451, 0.447059, 0.501961, 1.0 ]
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "newobj",
					"text" : "tap.jit.motion+",
					"numoutlets" : 4,
					"fontsize" : 10.0,
					"outlettype" : [ "jit_matrix", "float", "float", "float" ],
					"id" : "obj-40",
					"fontname" : "Verdana",
					"numinlets" : 1,
					"patching_rect" : [ 70.0, 287.0, 85.0, 19.0 ]
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "newobj",
					"text" : "jit.scissors @rows 2 @columns 2",
					"numoutlets" : 5,
					"fontsize" : 10.0,
					"outlettype" : [ "jit_matrix", "jit_matrix", "jit_matrix", "jit_matrix", "" ],
					"id" : "obj-41",
					"fontname" : "Verdana",
					"numinlets" : 1,
					"patching_rect" : [ 70.0, 237.0, 349.0, 19.0 ]
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "panel",
					"rounded" : 15,
					"numoutlets" : 0,
					"border" : 1,
					"id" : "obj-42",
					"numinlets" : 1,
					"patching_rect" : [ 62.0, 278.0, 192.0, 116.0 ],
					"shadow" : 1
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
					"patching_rect" : [ 52.0, 577.0, 140.0, 50.0 ]
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "comment",
					"varname" : "autohelp_top_title",
					"text" : "tap.jit.motion+",
					"textcolor" : [ 0.93, 0.93, 0.97, 1.0 ],
					"numoutlets" : 0,
					"fontface" : 3,
					"fontsize" : 20.871338,
					"frgb" : [ 0.93, 0.93, 0.97, 1.0 ],
					"id" : "obj-2",
					"fontname" : "Arial",
					"numinlets" : 1,
					"patching_rect" : [ 10.0, 8.0, 485.0, 30.0 ]
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "comment",
					"varname" : "autohelp_top_digest",
					"text" : "Video motion detection analysis",
					"textcolor" : [ 0.93, 0.93, 0.97, 1.0 ],
					"numoutlets" : 0,
					"fontsize" : 12.754705,
					"frgb" : [ 0.93, 0.93, 0.97, 1.0 ],
					"id" : "obj-3",
					"fontname" : "Arial",
					"numinlets" : 1,
					"patching_rect" : [ 10.0, 36.0, 485.0, 21.0 ]
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "comment",
					"varname" : "autohelp_top_description",
					"text" : "This object is an extension of the tap.jit.motion object. See the tap.jit.motion help patcher for more details of how this works. The additions include displays and finding the bounding regions of the motion.",
					"linecount" : 3,
					"numoutlets" : 0,
					"fontsize" : 10.0,
					"frgb" : [ 0.658824, 0.658824, 0.658824, 1.0 ],
					"id" : "obj-4",
					"fontname" : "Verdana",
					"numinlets" : 1,
					"patching_rect" : [ 10.0, 57.0, 480.0, 43.0 ]
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "textbutton",
					"varname" : "autohelp_top_ref",
					"bgcolor" : [ 0.0, 0.0, 0.0, 0.0 ],
					"bgoveroncolor" : [ 0.0, 0.0, 0.0, 0.0 ],
					"underline" : 1,
					"textcolor" : [ 0.27, 0.35, 0.47, 1.0 ],
					"numoutlets" : 3,
					"fontface" : 3,
					"fontsize" : 12.754705,
					"spacing_x" : 0.0,
					"bgoncolor" : [ 0.0, 0.0, 0.0, 0.0 ],
					"outlettype" : [ "", "", "int" ],
					"bordercolor" : [ 0.0, 0.0, 0.0, 0.0 ],
					"spacing_y" : 0.0,
					"textoncolor" : [ 0.27, 0.35, 0.47, 1.0 ],
					"presentation_rect" : [ 0.0, 0.0, 190.495392, 14.666666 ],
					"id" : "obj-5",
					"fontlink" : 1,
					"text" : "open tap.jit.motion+ reference",
					"textovercolor" : [ 0.4, 0.5, 0.65, 1.0 ],
					"fontname" : "Arial",
					"bgovercolor" : [ 0.0, 0.0, 0.0, 0.0 ],
					"numinlets" : 1,
					"patching_rect" : [ 309.504608, 22.0, 190.495392, 14.666666 ]
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
					"patching_rect" : [ 5.0, 5.0, 495.0, 52.0 ]
				}

			}
 ],
		"lines" : [ 			{
				"patchline" : 				{
					"source" : [ "obj-41", 2 ],
					"destination" : [ "obj-26", 0 ],
					"hidden" : 0,
					"midpoints" : [ 244.5, 270.0, 452.0, 270.0, 452.0, 407.0, 78.5, 407.0 ]
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-26", 0 ],
					"destination" : [ "obj-24", 0 ],
					"hidden" : 0,
					"midpoints" : [  ]
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-13", 0 ],
					"destination" : [ "obj-12", 0 ],
					"hidden" : 0,
					"midpoints" : [  ]
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-12", 0 ],
					"destination" : [ "obj-41", 0 ],
					"hidden" : 0,
					"midpoints" : [  ]
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-41", 0 ],
					"destination" : [ "obj-40", 0 ],
					"hidden" : 0,
					"midpoints" : [  ]
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-40", 0 ],
					"destination" : [ "obj-38", 0 ],
					"hidden" : 0,
					"midpoints" : [  ]
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-26", 1 ],
					"destination" : [ "obj-23", 0 ],
					"hidden" : 0,
					"midpoints" : [  ]
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-21", 0 ],
					"destination" : [ "obj-25", 0 ],
					"hidden" : 1,
					"midpoints" : [  ]
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-40", 1 ],
					"destination" : [ "obj-37", 0 ],
					"hidden" : 0,
					"midpoints" : [  ]
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-35", 0 ],
					"destination" : [ "obj-39", 0 ],
					"hidden" : 1,
					"midpoints" : [  ]
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-12", 0 ],
					"destination" : [ "obj-10", 0 ],
					"hidden" : 1,
					"midpoints" : [  ]
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-22", 0 ],
					"destination" : [ "obj-21", 0 ],
					"hidden" : 1,
					"midpoints" : [  ]
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-36", 0 ],
					"destination" : [ "obj-35", 0 ],
					"hidden" : 1,
					"midpoints" : [  ]
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-23", 0 ],
					"destination" : [ "obj-21", 1 ],
					"hidden" : 1,
					"midpoints" : [  ]
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-37", 0 ],
					"destination" : [ "obj-35", 1 ],
					"hidden" : 1,
					"midpoints" : [  ]
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-11", 1 ],
					"destination" : [ "obj-22", 0 ],
					"hidden" : 1,
					"midpoints" : [  ]
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-11", 1 ],
					"destination" : [ "obj-36", 0 ],
					"hidden" : 1,
					"midpoints" : [  ]
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-41", 3 ],
					"destination" : [ "obj-19", 0 ],
					"hidden" : 0,
					"midpoints" : [ 327.0, 266.0, 456.0, 266.0, 456.0, 410.0, 274.5, 410.0 ]
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-19", 0 ],
					"destination" : [ "obj-17", 0 ],
					"hidden" : 0,
					"midpoints" : [  ]
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-41", 1 ],
					"destination" : [ "obj-33", 0 ],
					"hidden" : 0,
					"midpoints" : [ 162.0, 274.0, 275.5, 274.0 ]
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-33", 0 ],
					"destination" : [ "obj-31", 0 ],
					"hidden" : 0,
					"midpoints" : [  ]
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-19", 1 ],
					"destination" : [ "obj-16", 0 ],
					"hidden" : 0,
					"midpoints" : [  ]
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-14", 0 ],
					"destination" : [ "obj-18", 0 ],
					"hidden" : 1,
					"midpoints" : [  ]
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-33", 1 ],
					"destination" : [ "obj-30", 0 ],
					"hidden" : 0,
					"midpoints" : [  ]
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-28", 0 ],
					"destination" : [ "obj-32", 0 ],
					"hidden" : 1,
					"midpoints" : [  ]
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-15", 0 ],
					"destination" : [ "obj-14", 0 ],
					"hidden" : 1,
					"midpoints" : [  ]
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-29", 0 ],
					"destination" : [ "obj-28", 0 ],
					"hidden" : 1,
					"midpoints" : [  ]
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-16", 0 ],
					"destination" : [ "obj-14", 1 ],
					"hidden" : 1,
					"midpoints" : [  ]
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-30", 0 ],
					"destination" : [ "obj-28", 1 ],
					"hidden" : 1,
					"midpoints" : [  ]
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-11", 1 ],
					"destination" : [ "obj-15", 0 ],
					"hidden" : 1,
					"midpoints" : [  ]
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-11", 1 ],
					"destination" : [ "obj-29", 0 ],
					"hidden" : 1,
					"midpoints" : [  ]
				}

			}
 ]
	}

}
