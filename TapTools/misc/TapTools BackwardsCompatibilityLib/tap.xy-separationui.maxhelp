{
	"patcher" : 	{
		"rect" : [ 77.0, 61.0, 547.0, 577.0 ],
		"bgcolor" : [ 0.1, 0.1, 0.1, 1.0 ],
		"bglocked" : 0,
		"defrect" : [ 77.0, 61.0, 547.0, 577.0 ],
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
					"text" : "Example",
					"fontsize" : 10.533333,
					"numinlets" : 1,
					"patching_rect" : [ 130.0, 345.0, 253.0, 21.0 ],
					"numoutlets" : 0,
					"fontname" : "Arial",
					"frgb" : [ 0.658824, 0.658824, 0.658824, 1.0 ],
					"id" : "obj-1"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "panel",
					"bgcolor" : [ 0.501961, 0.54902, 0.733333, 1.0 ],
					"numinlets" : 1,
					"patching_rect" : [ 128.0, 343.0, 402.0, 20.0 ],
					"numoutlets" : 0,
					"rounded" : 0,
					"id" : "obj-2"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "jsui",
					"numinlets" : 1,
					"patching_rect" : [ 284.0, 369.0, 240.0, 159.0 ],
					"numoutlets" : 2,
					"outlettype" : [ "", "" ],
					"id" : "obj-3",
					"jsarguments" : [  ],
					"border" : 0,
					"filename" : "tap.xy-separationui.js"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "comment",
					"text" : "Outlet 2: x position of 2 balls based on base # (x) ± seperation value (y) scaled between 0:1",
					"linecount" : 4,
					"fontsize" : 7.9,
					"numinlets" : 1,
					"patching_rect" : [ 145.0, 409.0, 129.0, 49.0 ],
					"numoutlets" : 0,
					"fontname" : "Arial",
					"frgb" : [ 0.658824, 0.658824, 0.658824, 1.0 ],
					"id" : "obj-4"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "comment",
					"text" : "Outlet 1: x, y position of central ball between 0:1",
					"linecount" : 2,
					"fontsize" : 7.9,
					"numinlets" : 1,
					"patching_rect" : [ 12.0, 409.0, 129.0, 28.0 ],
					"numoutlets" : 0,
					"fontname" : "Arial",
					"frgb" : [ 0.658824, 0.658824, 0.658824, 1.0 ],
					"id" : "obj-5"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "comment",
					"text" : "(brgb 255 255 255 255) = white",
					"fontsize" : 7.9,
					"numinlets" : 1,
					"patching_rect" : [ 302.0, 215.0, 171.0, 17.0 ],
					"numoutlets" : 0,
					"fontname" : "Arial",
					"frgb" : [ 0.658824, 0.658824, 0.658824, 1.0 ],
					"id" : "obj-6"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "comment",
					"text" : "Set sets a specific position w\/o sending out the values (set x y)",
					"linecount" : 2,
					"fontsize" : 7.9,
					"numinlets" : 1,
					"patching_rect" : [ 132.0, 273.0, 149.0, 28.0 ],
					"numoutlets" : 0,
					"fontname" : "Arial",
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
					"patching_rect" : [ 188.0, 158.0, 48.0, 17.0 ],
					"triangle" : 0,
					"transparent" : 1,
					"numoutlets" : 2,
					"fontname" : "Arial",
					"outlettype" : [ "float", "bang" ],
					"id" : "obj-8",
					"triscale" : 0.9,
					"textcolor" : [ 0.0, 0.0, 0.0, 1.0 ]
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "flonum",
					"htextcolor" : [ 0.870588, 0.870588, 0.870588, 1.0 ],
					"bgcolor" : [ 0.866667, 0.866667, 0.866667, 1.0 ],
					"fontsize" : 7.9,
					"numinlets" : 1,
					"patching_rect" : [ 138.0, 158.0, 48.0, 17.0 ],
					"triangle" : 0,
					"transparent" : 1,
					"numoutlets" : 2,
					"fontname" : "Arial",
					"outlettype" : [ "float", "bang" ],
					"id" : "obj-9",
					"triscale" : 0.9,
					"textcolor" : [ 0.0, 0.0, 0.0, 1.0 ]
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "newobj",
					"text" : "pak 0. 0.",
					"fontsize" : 7.9,
					"numinlets" : 2,
					"patching_rect" : [ 138.0, 180.0, 60.0, 17.0 ],
					"numoutlets" : 1,
					"fontname" : "Arial",
					"outlettype" : [ "" ],
					"id" : "obj-10"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "flonum",
					"htextcolor" : [ 0.870588, 0.870588, 0.870588, 1.0 ],
					"bgcolor" : [ 0.866667, 0.866667, 0.866667, 1.0 ],
					"fontsize" : 7.9,
					"numinlets" : 1,
					"patching_rect" : [ 210.0, 233.0, 48.0, 17.0 ],
					"triangle" : 0,
					"transparent" : 1,
					"numoutlets" : 2,
					"fontname" : "Arial",
					"outlettype" : [ "float", "bang" ],
					"id" : "obj-11",
					"triscale" : 0.9,
					"textcolor" : [ 0.0, 0.0, 0.0, 1.0 ]
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "flonum",
					"htextcolor" : [ 0.870588, 0.870588, 0.870588, 1.0 ],
					"bgcolor" : [ 0.866667, 0.866667, 0.866667, 1.0 ],
					"fontsize" : 7.9,
					"numinlets" : 1,
					"patching_rect" : [ 160.0, 233.0, 48.0, 17.0 ],
					"triangle" : 0,
					"transparent" : 1,
					"numoutlets" : 2,
					"fontname" : "Arial",
					"outlettype" : [ "float", "bang" ],
					"id" : "obj-12",
					"triscale" : 0.9,
					"textcolor" : [ 0.0, 0.0, 0.0, 1.0 ]
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "newobj",
					"text" : "pak set 0. 0.",
					"fontsize" : 7.9,
					"numinlets" : 3,
					"patching_rect" : [ 133.0, 254.0, 65.0, 17.0 ],
					"numoutlets" : 1,
					"fontname" : "Arial",
					"outlettype" : [ "" ],
					"id" : "obj-13"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "newobj",
					"text" : "unpack 0. 0.",
					"fontsize" : 7.9,
					"numinlets" : 1,
					"patching_rect" : [ 194.0, 386.0, 63.0, 17.0 ],
					"numoutlets" : 2,
					"fontname" : "Arial",
					"outlettype" : [ "float", "float" ],
					"id" : "obj-14"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "flonum",
					"htextcolor" : [ 0.870588, 0.870588, 0.870588, 1.0 ],
					"bgcolor" : [ 0.866667, 0.866667, 0.866667, 1.0 ],
					"fontsize" : 7.9,
					"numinlets" : 1,
					"patching_rect" : [ 210.0, 496.0, 48.0, 17.0 ],
					"triangle" : 0,
					"transparent" : 1,
					"numoutlets" : 2,
					"fontname" : "Arial",
					"outlettype" : [ "float", "bang" ],
					"id" : "obj-15",
					"triscale" : 0.9,
					"textcolor" : [ 0.0, 0.0, 0.0, 1.0 ]
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "comment",
					"text" : "red",
					"fontsize" : 10.533333,
					"numinlets" : 1,
					"patching_rect" : [ 143.0, 494.0, 41.0, 21.0 ],
					"numoutlets" : 0,
					"fontname" : "Arial",
					"frgb" : [ 0.658824, 0.658824, 0.658824, 1.0 ],
					"id" : "obj-16"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "comment",
					"text" : "yellow",
					"fontsize" : 10.533333,
					"numinlets" : 1,
					"patching_rect" : [ 143.0, 477.0, 42.0, 21.0 ],
					"numoutlets" : 0,
					"fontname" : "Arial",
					"frgb" : [ 0.658824, 0.658824, 0.658824, 1.0 ],
					"id" : "obj-17"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "slider",
					"bgcolor" : [ 1.0, 1.0, 1.0, 1.0 ],
					"bordercolor" : [ 0.5, 0.5, 0.5, 1.0 ],
					"orientation" : 2,
					"numinlets" : 1,
					"patching_rect" : [ 305.0, 173.0, 13.0, 41.0 ],
					"numoutlets" : 1,
					"knobcolor" : [ 0.2, 0.2, 0.2, 1.0 ],
					"mult" : 2.0,
					"outlettype" : [ "" ],
					"id" : "obj-18"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "comment",
					"text" : "Color to Set:",
					"fontsize" : 7.9,
					"numinlets" : 1,
					"patching_rect" : [ 301.0, 159.0, 63.0, 17.0 ],
					"numoutlets" : 0,
					"fontname" : "Arial",
					"frgb" : [ 0.658824, 0.658824, 0.658824, 1.0 ],
					"id" : "obj-19"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "comment",
					"text" : "Alpha",
					"fontsize" : 7.9,
					"numinlets" : 1,
					"patching_rect" : [ 457.0, 215.0, 34.0, 17.0 ],
					"numoutlets" : 0,
					"fontname" : "Arial",
					"frgb" : [ 0.658824, 0.658824, 0.658824, 1.0 ],
					"id" : "obj-20"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "comment",
					"text" : "B",
					"fontsize" : 7.9,
					"numinlets" : 1,
					"patching_rect" : [ 476.0, 200.0, 16.0, 17.0 ],
					"numoutlets" : 0,
					"fontname" : "Arial",
					"frgb" : [ 0.658824, 0.658824, 0.658824, 1.0 ],
					"id" : "obj-21"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "comment",
					"text" : "G",
					"fontsize" : 7.9,
					"numinlets" : 1,
					"patching_rect" : [ 476.0, 186.0, 16.0, 17.0 ],
					"numoutlets" : 0,
					"fontname" : "Arial",
					"frgb" : [ 0.658824, 0.658824, 0.658824, 1.0 ],
					"id" : "obj-22"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "comment",
					"text" : "R",
					"fontsize" : 7.9,
					"numinlets" : 1,
					"patching_rect" : [ 476.0, 171.0, 16.0, 17.0 ],
					"numoutlets" : 0,
					"fontname" : "Arial",
					"frgb" : [ 0.658824, 0.658824, 0.658824, 1.0 ],
					"id" : "obj-23"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "newobj",
					"text" : "prepend set",
					"fontsize" : 7.9,
					"numinlets" : 1,
					"patching_rect" : [ 394.0, 234.0, 61.0, 17.0 ],
					"numoutlets" : 1,
					"fontname" : "Arial",
					"outlettype" : [ "" ],
					"hidden" : 1,
					"id" : "obj-24"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "umenu",
					"bgcolor" : [ 1.0, 1.0, 1.0, 1.0 ],
					"togcolor" : [ 0.55, 0.55, 0.55, 1.0 ],
					"fontsize" : 7.9,
					"arrowbgcolor" : [ 0.86, 0.86, 0.86, 1.0 ],
					"numinlets" : 1,
					"items" : [ "brgb", ",", "srgb1", ",", "srgb2" ],
					"arrowlink" : 1,
					"patching_rect" : [ 366.0, 156.0, 66.0, 17.0 ],
					"discolor" : [ 0.43, 0.43, 0.43, 1.0 ],
					"numoutlets" : 3,
					"types" : [  ],
					"pattrmode" : 1,
					"framecolor" : [ 0.5, 0.5, 0.5, 1.0 ],
					"fontname" : "Arial",
					"outlettype" : [ "int", "", "" ],
					"arrowcolor" : [ 0.22, 0.22, 0.22, 1.0 ],
					"id" : "obj-25"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "comment",
					"text" : "y seperation",
					"fontsize" : 10.533333,
					"numinlets" : 1,
					"patching_rect" : [ 167.0, 459.0, 81.0, 21.0 ],
					"numoutlets" : 0,
					"fontname" : "Arial",
					"frgb" : [ 0.658824, 0.658824, 0.658824, 1.0 ],
					"id" : "obj-26"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "comment",
					"text" : "y",
					"fontsize" : 10.533333,
					"numinlets" : 1,
					"patching_rect" : [ 7.0, 492.0, 23.0, 21.0 ],
					"numoutlets" : 0,
					"fontname" : "Arial",
					"frgb" : [ 0.658824, 0.658824, 0.658824, 1.0 ],
					"id" : "obj-27"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "newobj",
					"text" : "unpack 1 1 1",
					"fontsize" : 7.9,
					"numinlets" : 1,
					"patching_rect" : [ 317.0, 220.0, 65.0, 17.0 ],
					"numoutlets" : 3,
					"fontname" : "Arial",
					"outlettype" : [ "int", "int", "int" ],
					"hidden" : 1,
					"id" : "obj-28"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "number",
					"htextcolor" : [ 0.870588, 0.870588, 0.870588, 1.0 ],
					"bgcolor" : [ 0.866667, 0.866667, 0.866667, 1.0 ],
					"fontsize" : 7.9,
					"numinlets" : 1,
					"patching_rect" : [ 491.0, 198.0, 35.0, 17.0 ],
					"triangle" : 0,
					"transparent" : 1,
					"numoutlets" : 2,
					"fontname" : "Arial",
					"outlettype" : [ "int", "bang" ],
					"maximum" : 255,
					"id" : "obj-29",
					"triscale" : 0.9,
					"minimum" : 0,
					"textcolor" : [ 0.0, 0.0, 0.0, 1.0 ]
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "number",
					"htextcolor" : [ 0.870588, 0.870588, 0.870588, 1.0 ],
					"bgcolor" : [ 0.866667, 0.866667, 0.866667, 1.0 ],
					"fontsize" : 7.9,
					"numinlets" : 1,
					"patching_rect" : [ 491.0, 184.0, 35.0, 17.0 ],
					"triangle" : 0,
					"transparent" : 1,
					"numoutlets" : 2,
					"fontname" : "Arial",
					"outlettype" : [ "int", "bang" ],
					"maximum" : 255,
					"id" : "obj-30",
					"triscale" : 0.9,
					"minimum" : 0,
					"textcolor" : [ 0.0, 0.0, 0.0, 1.0 ]
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "number",
					"htextcolor" : [ 0.870588, 0.870588, 0.870588, 1.0 ],
					"bgcolor" : [ 0.866667, 0.866667, 0.866667, 1.0 ],
					"fontsize" : 7.9,
					"numinlets" : 1,
					"patching_rect" : [ 491.0, 169.0, 35.0, 17.0 ],
					"triangle" : 0,
					"transparent" : 1,
					"numoutlets" : 2,
					"fontname" : "Arial",
					"outlettype" : [ "int", "bang" ],
					"maximum" : 255,
					"id" : "obj-31",
					"triscale" : 0.9,
					"minimum" : 0,
					"textcolor" : [ 0.0, 0.0, 0.0, 1.0 ]
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "message",
					"text" : "saturation $1",
					"fontsize" : 7.9,
					"numinlets" : 2,
					"patching_rect" : [ 317.0, 159.0, 70.0, 15.0 ],
					"numoutlets" : 1,
					"fontname" : "Arial",
					"outlettype" : [ "" ],
					"hidden" : 1,
					"id" : "obj-32"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "newobj",
					"text" : "tap.pack 4 @triggers 1 1 1 1",
					"fontsize" : 7.9,
					"numinlets" : 4,
					"patching_rect" : [ 405.0, 256.0, 139.0, 17.0 ],
					"numoutlets" : 2,
					"fontname" : "Arial",
					"outlettype" : [ "", "" ],
					"hidden" : 1,
					"id" : "obj-33"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "newobj",
					"text" : "prepend brgb",
					"fontsize" : 7.9,
					"numinlets" : 1,
					"patching_rect" : [ 394.0, 279.0, 65.0, 17.0 ],
					"numoutlets" : 1,
					"fontname" : "Arial",
					"outlettype" : [ "" ],
					"hidden" : 1,
					"id" : "obj-34"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "number",
					"htextcolor" : [ 0.870588, 0.870588, 0.870588, 1.0 ],
					"bgcolor" : [ 0.866667, 0.866667, 0.866667, 1.0 ],
					"fontsize" : 7.9,
					"numinlets" : 1,
					"patching_rect" : [ 491.0, 212.0, 35.0, 17.0 ],
					"triangle" : 0,
					"transparent" : 1,
					"numoutlets" : 2,
					"fontname" : "Arial",
					"outlettype" : [ "int", "bang" ],
					"maximum" : 255,
					"id" : "obj-35",
					"triscale" : 0.9,
					"minimum" : 0,
					"textcolor" : [ 0.0, 0.0, 0.0, 1.0 ]
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "swatch",
					"numinlets" : 3,
					"patching_rect" : [ 317.0, 173.0, 157.0, 41.0 ],
					"numoutlets" : 2,
					"outlettype" : [ "", "float" ],
					"id" : "obj-36",
					"compatibility" : 1
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "comment",
					"text" : "position",
					"fontsize" : 10.533333,
					"numinlets" : 1,
					"patching_rect" : [ 27.0, 460.0, 58.0, 21.0 ],
					"numoutlets" : 0,
					"fontname" : "Arial",
					"frgb" : [ 0.658824, 0.658824, 0.658824, 1.0 ],
					"id" : "obj-37"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "comment",
					"text" : "x",
					"fontsize" : 10.533333,
					"numinlets" : 1,
					"patching_rect" : [ 7.0, 477.0, 23.0, 21.0 ],
					"numoutlets" : 0,
					"fontname" : "Arial",
					"frgb" : [ 0.658824, 0.658824, 0.658824, 1.0 ],
					"id" : "obj-38"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "flonum",
					"htextcolor" : [ 0.870588, 0.870588, 0.870588, 1.0 ],
					"bgcolor" : [ 0.866667, 0.866667, 0.866667, 1.0 ],
					"fontsize" : 7.9,
					"numinlets" : 1,
					"patching_rect" : [ 210.0, 478.0, 48.0, 17.0 ],
					"triangle" : 0,
					"transparent" : 1,
					"numoutlets" : 2,
					"fontname" : "Arial",
					"outlettype" : [ "float", "bang" ],
					"id" : "obj-39",
					"triscale" : 0.9,
					"textcolor" : [ 0.0, 0.0, 0.0, 1.0 ]
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "flonum",
					"htextcolor" : [ 0.870588, 0.870588, 0.870588, 1.0 ],
					"bgcolor" : [ 0.866667, 0.866667, 0.866667, 1.0 ],
					"fontsize" : 7.9,
					"numinlets" : 1,
					"patching_rect" : [ 26.0, 494.0, 48.0, 17.0 ],
					"triangle" : 0,
					"transparent" : 1,
					"numoutlets" : 2,
					"fontname" : "Arial",
					"outlettype" : [ "float", "bang" ],
					"id" : "obj-40",
					"triscale" : 0.9,
					"textcolor" : [ 0.0, 0.0, 0.0, 1.0 ]
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "flonum",
					"htextcolor" : [ 0.870588, 0.870588, 0.870588, 1.0 ],
					"bgcolor" : [ 0.866667, 0.866667, 0.866667, 1.0 ],
					"fontsize" : 7.9,
					"numinlets" : 1,
					"patching_rect" : [ 26.0, 477.0, 48.0, 17.0 ],
					"triangle" : 0,
					"transparent" : 1,
					"numoutlets" : 2,
					"fontname" : "Arial",
					"outlettype" : [ "float", "bang" ],
					"id" : "obj-41",
					"triscale" : 0.9,
					"textcolor" : [ 0.0, 0.0, 0.0, 1.0 ]
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "newobj",
					"text" : "unpack 0. 0.",
					"fontsize" : 7.9,
					"numinlets" : 1,
					"patching_rect" : [ 17.0, 388.0, 63.0, 17.0 ],
					"numoutlets" : 2,
					"fontname" : "Arial",
					"outlettype" : [ "float", "float" ],
					"id" : "obj-42"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "comment",
					"text" : "Radius of the satellites [default 0.1]",
					"fontsize" : 7.9,
					"numinlets" : 1,
					"patching_rect" : [ 227.0, 308.0, 303.0, 17.0 ],
					"numoutlets" : 0,
					"fontname" : "Arial",
					"frgb" : [ 0.658824, 0.658824, 0.658824, 1.0 ],
					"id" : "obj-43"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "message",
					"text" : "radius $1",
					"fontsize" : 7.9,
					"numinlets" : 2,
					"patching_rect" : [ 152.0, 321.0, 52.0, 15.0 ],
					"numoutlets" : 1,
					"fontname" : "Arial",
					"outlettype" : [ "" ],
					"id" : "obj-44"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "flonum",
					"htextcolor" : [ 0.870588, 0.870588, 0.870588, 1.0 ],
					"bgcolor" : [ 0.866667, 0.866667, 0.866667, 1.0 ],
					"fontsize" : 7.9,
					"numinlets" : 1,
					"patching_rect" : [ 152.0, 304.0, 48.0, 17.0 ],
					"triangle" : 0,
					"transparent" : 1,
					"numoutlets" : 2,
					"fontname" : "Arial",
					"outlettype" : [ "float", "bang" ],
					"id" : "obj-45",
					"triscale" : 0.9,
					"textcolor" : [ 0.0, 0.0, 0.0, 1.0 ]
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "comment",
					"text" : "Control the seperation of 2 positions",
					"fontsize" : 10.533333,
					"numinlets" : 1,
					"patching_rect" : [ 133.0, 34.0, 309.0, 21.0 ],
					"numoutlets" : 0,
					"fontname" : "Arial",
					"frgb" : [ 0.87451, 0.87451, 0.87451, 1.0 ],
					"id" : "obj-46",
					"textcolor" : [ 0.87451, 0.87451, 0.87451, 1.0 ]
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "comment",
					"text" : "tap.xy-separationui",
					"fontsize" : 21.066666,
					"numinlets" : 1,
					"patching_rect" : [ 133.0, 5.0, 309.0, 35.0 ],
					"numoutlets" : 0,
					"fontname" : "Arial",
					"frgb" : [ 0.87451, 0.87451, 0.87451, 1.0 ],
					"id" : "obj-47",
					"textcolor" : [ 0.87451, 0.87451, 0.87451, 1.0 ]
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "message",
					"text" : "0.1",
					"fontsize" : 7.9,
					"numinlets" : 2,
					"patching_rect" : [ 466.0, 97.0, 21.0, 15.0 ],
					"numoutlets" : 1,
					"fontname" : "Arial",
					"outlettype" : [ "" ],
					"hidden" : 1,
					"id" : "obj-48"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "message",
					"text" : "20000",
					"fontsize" : 7.9,
					"numinlets" : 2,
					"patching_rect" : [ 426.0, 97.0, 39.0, 15.0 ],
					"numoutlets" : 1,
					"fontname" : "Arial",
					"outlettype" : [ "" ],
					"hidden" : 1,
					"id" : "obj-49"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "message",
					"text" : "3.5",
					"fontsize" : 7.9,
					"numinlets" : 2,
					"patching_rect" : [ 404.0, 97.0, 21.0, 15.0 ],
					"numoutlets" : 1,
					"fontname" : "Arial",
					"outlettype" : [ "" ],
					"hidden" : 1,
					"id" : "obj-50"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "message",
					"text" : "100",
					"fontsize" : 7.9,
					"numinlets" : 2,
					"patching_rect" : [ 380.0, 97.0, 24.0, 15.0 ],
					"numoutlets" : 1,
					"fontname" : "Arial",
					"outlettype" : [ "" ],
					"hidden" : 1,
					"id" : "obj-51"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "newobj",
					"text" : "loadbang",
					"fontsize" : 7.9,
					"numinlets" : 1,
					"patching_rect" : [ 441.0, 55.0, 53.0, 17.0 ],
					"numoutlets" : 1,
					"fontname" : "Arial",
					"outlettype" : [ "bang" ],
					"hidden" : 1,
					"id" : "obj-52"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "comment",
					"text" : "List sets a specific position for the central ball (x y)",
					"linecount" : 2,
					"fontsize" : 7.9,
					"numinlets" : 1,
					"patching_rect" : [ 135.0, 199.0, 129.0, 28.0 ],
					"numoutlets" : 0,
					"fontname" : "Arial",
					"frgb" : [ 0.658824, 0.658824, 0.658824, 1.0 ],
					"id" : "obj-53"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "comment",
					"text" : "Attributes",
					"fontsize" : 10.533333,
					"numinlets" : 1,
					"patching_rect" : [ 130.0, 135.0, 253.0, 21.0 ],
					"numoutlets" : 0,
					"fontname" : "Arial",
					"frgb" : [ 0.658824, 0.658824, 0.658824, 1.0 ],
					"id" : "obj-54"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "panel",
					"bgcolor" : [ 0.501961, 0.54902, 0.733333, 1.0 ],
					"numinlets" : 1,
					"patching_rect" : [ 128.0, 133.0, 402.0, 20.0 ],
					"numoutlets" : 0,
					"rounded" : 0,
					"id" : "obj-55"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "panel",
					"bgcolor" : [ 0.807843, 0.807843, 0.807843, 1.0 ],
					"numinlets" : 1,
					"patching_rect" : [ 128.0, 302.0, 402.0, 36.0 ],
					"numoutlets" : 0,
					"rounded" : 0,
					"id" : "obj-56"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "panel",
					"bgcolor" : [ 0.501961, 0.54902, 0.733333, 1.0 ],
					"numinlets" : 1,
					"patching_rect" : [ 5.0, 495.0, 270.0, 17.0 ],
					"numoutlets" : 0,
					"rounded" : 0,
					"id" : "obj-57"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "panel",
					"bgcolor" : [ 0.541176, 0.639216, 0.752941, 1.0 ],
					"numinlets" : 1,
					"patching_rect" : [ 5.0, 478.0, 270.0, 17.0 ],
					"numoutlets" : 0,
					"rounded" : 0,
					"id" : "obj-58"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "panel",
					"bgcolor" : [ 0.501961, 0.54902, 0.733333, 1.0 ],
					"numinlets" : 1,
					"patching_rect" : [ 5.0, 461.0, 270.0, 17.0 ],
					"numoutlets" : 0,
					"rounded" : 0,
					"id" : "obj-59"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "panel",
					"bgcolor" : [ 0.501961, 0.54902, 0.733333, 1.0 ],
					"numinlets" : 1,
					"patching_rect" : [ 491.0, 153.0, 39.0, 74.0 ],
					"numoutlets" : 0,
					"rounded" : 0,
					"id" : "obj-60"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "panel",
					"bgcolor" : [ 0.807843, 0.807843, 0.807843, 1.0 ],
					"numinlets" : 1,
					"patching_rect" : [ 128.0, 154.0, 170.0, 73.0 ],
					"numoutlets" : 0,
					"rounded" : 0,
					"id" : "obj-61"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "panel",
					"bgcolor" : [ 0.807843, 0.807843, 0.807843, 1.0 ],
					"numinlets" : 1,
					"patching_rect" : [ 299.0, 154.0, 231.0, 73.0 ],
					"numoutlets" : 0,
					"rounded" : 0,
					"id" : "obj-62"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "panel",
					"bgcolor" : [ 0.807843, 0.807843, 0.807843, 1.0 ],
					"numinlets" : 1,
					"patching_rect" : [ 128.0, 228.0, 402.0, 73.0 ],
					"numoutlets" : 0,
					"rounded" : 0,
					"id" : "obj-63"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "panel",
					"bgcolor" : [ 0.807843, 0.807843, 0.807843, 1.0 ],
					"numinlets" : 1,
					"patching_rect" : [ 2.0, 364.0, 528.0, 170.0 ],
					"numoutlets" : 0,
					"rounded" : 0,
					"id" : "obj-64"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "comment",
					"text" : "by jesse allison",
					"fontsize" : 12.288889,
					"background" : 1,
					"numinlets" : 1,
					"patching_rect" : [ 133.0, 87.0, 248.0, 23.0 ],
					"numoutlets" : 0,
					"fontname" : "Arial",
					"frgb" : [ 0.658824, 0.658824, 0.658824, 1.0 ],
					"id" : "obj-65"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "fpic",
					"background" : 1,
					"numinlets" : 1,
					"pic" : "ttblue_icon.bmp",
					"patching_rect" : [ -1.0, -1.0, 130.0, 130.0 ],
					"numoutlets" : 0,
					"id" : "obj-66"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "newobj",
					"text" : "b 1",
					"fontsize" : 7.9,
					"background" : 1,
					"numinlets" : 1,
					"patching_rect" : [ 262.0, 60.0, 22.0, 17.0 ],
					"numoutlets" : 1,
					"fontname" : "Arial",
					"outlettype" : [ "bang" ],
					"hidden" : 1,
					"id" : "obj-67"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "message",
					"text" : ";\rjitter launch_browser www.electrotap.com",
					"linecount" : 3,
					"fontsize" : 7.9,
					"background" : 1,
					"numinlets" : 2,
					"patching_rect" : [ 262.0, 86.0, 113.0, 36.0 ],
					"numoutlets" : 1,
					"fontname" : "Arial",
					"outlettype" : [ "" ],
					"hidden" : 1,
					"id" : "obj-68"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "umenu",
					"bgcolor" : [ 0.866667, 0.866667, 0.866667, 1.0 ],
					"menumode" : 2,
					"labelclick" : 1,
					"fontsize" : 10.533333,
					"background" : 1,
					"numinlets" : 1,
					"items" : "electrotap.com",
					"arrowlink" : 1,
					"patching_rect" : [ 133.0, 106.0, 126.0, 21.0 ],
					"numoutlets" : 3,
					"types" : [  ],
					"fontname" : "Arial",
					"outlettype" : [ "int", "", "" ],
					"id" : "obj-69",
					"textcolor" : [ 0.0, 0.0, 0.0, 1.0 ]
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "panel",
					"bgcolor" : [ 0.388235, 0.388235, 0.388235, 1.0 ],
					"background" : 1,
					"numinlets" : 1,
					"patching_rect" : [ 128.0, 0.0, 402.0, 128.0 ],
					"numoutlets" : 0,
					"rounded" : 0,
					"id" : "obj-70"
				}

			}
 ],
		"lines" : [ 			{
				"patchline" : 				{
					"source" : [ "obj-35", 0 ],
					"destination" : [ "obj-33", 3 ],
					"hidden" : 1
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-28", 2 ],
					"destination" : [ "obj-33", 2 ],
					"hidden" : 1
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-28", 2 ],
					"destination" : [ "obj-29", 0 ],
					"hidden" : 1
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-28", 1 ],
					"destination" : [ "obj-30", 0 ],
					"hidden" : 1
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-28", 0 ],
					"destination" : [ "obj-31", 0 ],
					"hidden" : 1
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-52", 0 ],
					"destination" : [ "obj-48", 0 ],
					"hidden" : 1
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-28", 1 ],
					"destination" : [ "obj-33", 1 ],
					"hidden" : 1
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-52", 0 ],
					"destination" : [ "obj-49", 0 ],
					"hidden" : 1
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-28", 0 ],
					"destination" : [ "obj-33", 0 ],
					"hidden" : 1
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-52", 0 ],
					"destination" : [ "obj-50", 0 ],
					"hidden" : 1
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-24", 0 ],
					"destination" : [ "obj-34", 0 ],
					"hidden" : 1
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-33", 0 ],
					"destination" : [ "obj-34", 0 ],
					"hidden" : 1
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-25", 1 ],
					"destination" : [ "obj-24", 0 ],
					"hidden" : 1
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-52", 0 ],
					"destination" : [ "obj-51", 0 ],
					"hidden" : 1
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-36", 0 ],
					"destination" : [ "obj-28", 0 ],
					"hidden" : 1
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-32", 0 ],
					"destination" : [ "obj-36", 0 ],
					"hidden" : 1
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-18", 0 ],
					"destination" : [ "obj-32", 0 ],
					"hidden" : 1
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-13", 0 ],
					"destination" : [ "obj-3", 0 ],
					"hidden" : 1
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-10", 0 ],
					"destination" : [ "obj-3", 0 ],
					"hidden" : 1
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-34", 0 ],
					"destination" : [ "obj-3", 0 ],
					"hidden" : 1
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-44", 0 ],
					"destination" : [ "obj-3", 0 ],
					"hidden" : 1
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-67", 0 ],
					"destination" : [ "obj-68", 0 ],
					"hidden" : 1
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-69", 0 ],
					"destination" : [ "obj-67", 0 ],
					"hidden" : 1
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-14", 1 ],
					"destination" : [ "obj-15", 0 ],
					"hidden" : 1
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-14", 0 ],
					"destination" : [ "obj-39", 0 ],
					"hidden" : 1
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-3", 1 ],
					"destination" : [ "obj-14", 0 ],
					"hidden" : 1
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-8", 0 ],
					"destination" : [ "obj-10", 1 ],
					"hidden" : 0
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-11", 0 ],
					"destination" : [ "obj-13", 2 ],
					"hidden" : 0
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-12", 0 ],
					"destination" : [ "obj-13", 1 ],
					"hidden" : 0
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-45", 0 ],
					"destination" : [ "obj-44", 0 ],
					"hidden" : 0
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-9", 0 ],
					"destination" : [ "obj-10", 0 ],
					"hidden" : 0
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-42", 1 ],
					"destination" : [ "obj-40", 0 ],
					"hidden" : 1
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-42", 0 ],
					"destination" : [ "obj-41", 0 ],
					"hidden" : 1
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-3", 0 ],
					"destination" : [ "obj-42", 0 ],
					"hidden" : 1
				}

			}
 ]
	}

}
