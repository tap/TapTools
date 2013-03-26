{
	"patcher" : 	{
		"rect" : [ 287.0, 108.0, 558.0, 599.0 ],
		"bgcolor" : [ 0.1, 0.1, 0.1, 1.0 ],
		"bglocked" : 0,
		"defrect" : [ 287.0, 108.0, 558.0, 599.0 ],
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
					"text" : "Y",
					"fontsize" : 7.9,
					"numinlets" : 1,
					"patching_rect" : [ 424.0, 294.0, 100.0, 17.0 ],
					"numoutlets" : 0,
					"fontname" : "Arial",
					"frgb" : [ 0.658824, 0.658824, 0.658824, 1.0 ],
					"id" : "obj-1"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "comment",
					"text" : "There are two inlets for x & y position. Send a number in an inlet to set & output the value, a list sets both x & y, and a number preceeded by the set message sets the value without output",
					"linecount" : 3,
					"fontsize" : 7.9,
					"numinlets" : 1,
					"patching_rect" : [ 135.0, 239.0, 327.0, 38.0 ],
					"numoutlets" : 0,
					"fontname" : "Arial",
					"frgb" : [ 0.658824, 0.658824, 0.658824, 1.0 ],
					"id" : "obj-2"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "flonum",
					"htextcolor" : [ 0.870588, 0.870588, 0.870588, 1.0 ],
					"bgcolor" : [ 0.866667, 0.866667, 0.866667, 1.0 ],
					"fontsize" : 7.9,
					"numinlets" : 1,
					"patching_rect" : [ 226.0, 278.0, 35.0, 17.0 ],
					"numoutlets" : 2,
					"fontname" : "Arial",
					"outlettype" : [ "float", "bang" ],
					"id" : "obj-3",
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
					"patching_rect" : [ 177.0, 278.0, 35.0, 17.0 ],
					"numoutlets" : 2,
					"fontname" : "Arial",
					"outlettype" : [ "float", "bang" ],
					"id" : "obj-4",
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
					"patching_rect" : [ 141.0, 278.0, 35.0, 17.0 ],
					"numoutlets" : 2,
					"fontname" : "Arial",
					"outlettype" : [ "float", "bang" ],
					"id" : "obj-5",
					"triscale" : 0.9,
					"textcolor" : [ 0.0, 0.0, 0.0, 1.0 ]
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "message",
					"text" : "set $1",
					"fontsize" : 7.9,
					"numinlets" : 2,
					"patching_rect" : [ 226.0, 297.0, 38.0, 15.0 ],
					"numoutlets" : 1,
					"fontname" : "Arial",
					"outlettype" : [ "" ],
					"id" : "obj-6"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "newobj",
					"text" : "pak set 0. 0.",
					"fontsize" : 7.9,
					"numinlets" : 3,
					"patching_rect" : [ 135.0, 297.0, 65.0, 17.0 ],
					"numoutlets" : 1,
					"fontname" : "Arial",
					"outlettype" : [ "" ],
					"id" : "obj-7"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "comment",
					"text" : "(brgb 255 255 255 255) = white",
					"fontsize" : 7.9,
					"numinlets" : 1,
					"patching_rect" : [ 302.0, 216.0, 171.0, 17.0 ],
					"numoutlets" : 0,
					"fontname" : "Arial",
					"frgb" : [ 0.658824, 0.658824, 0.658824, 1.0 ],
					"id" : "obj-8"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "slider",
					"bgcolor" : [ 1.0, 1.0, 1.0, 1.0 ],
					"bordercolor" : [ 0.5, 0.5, 0.5, 1.0 ],
					"orientation" : 2,
					"numinlets" : 1,
					"patching_rect" : [ 305.0, 174.0, 13.0, 41.0 ],
					"numoutlets" : 1,
					"knobcolor" : [ 0.2, 0.2, 0.2, 1.0 ],
					"mult" : 2.0,
					"outlettype" : [ "" ],
					"id" : "obj-9"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "comment",
					"text" : "Color to Set:",
					"fontsize" : 7.9,
					"numinlets" : 1,
					"patching_rect" : [ 301.0, 160.0, 63.0, 17.0 ],
					"numoutlets" : 0,
					"fontname" : "Arial",
					"frgb" : [ 0.658824, 0.658824, 0.658824, 1.0 ],
					"id" : "obj-10"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "comment",
					"text" : "Alpha",
					"fontsize" : 7.9,
					"numinlets" : 1,
					"patching_rect" : [ 457.0, 216.0, 34.0, 17.0 ],
					"numoutlets" : 0,
					"fontname" : "Arial",
					"frgb" : [ 0.658824, 0.658824, 0.658824, 1.0 ],
					"id" : "obj-11"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "comment",
					"text" : "B",
					"fontsize" : 7.9,
					"numinlets" : 1,
					"patching_rect" : [ 476.0, 201.0, 16.0, 17.0 ],
					"numoutlets" : 0,
					"fontname" : "Arial",
					"frgb" : [ 0.658824, 0.658824, 0.658824, 1.0 ],
					"id" : "obj-12"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "comment",
					"text" : "G",
					"fontsize" : 7.9,
					"numinlets" : 1,
					"patching_rect" : [ 476.0, 187.0, 16.0, 17.0 ],
					"numoutlets" : 0,
					"fontname" : "Arial",
					"frgb" : [ 0.658824, 0.658824, 0.658824, 1.0 ],
					"id" : "obj-13"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "comment",
					"text" : "R",
					"fontsize" : 7.9,
					"numinlets" : 1,
					"patching_rect" : [ 476.0, 172.0, 16.0, 17.0 ],
					"numoutlets" : 0,
					"fontname" : "Arial",
					"frgb" : [ 0.658824, 0.658824, 0.658824, 1.0 ],
					"id" : "obj-14"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "newobj",
					"text" : "unpack 1 1 1",
					"fontsize" : 7.9,
					"numinlets" : 1,
					"patching_rect" : [ 317.0, 221.0, 65.0, 17.0 ],
					"numoutlets" : 3,
					"fontname" : "Arial",
					"outlettype" : [ "int", "int", "int" ],
					"hidden" : 1,
					"id" : "obj-15"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "number",
					"htextcolor" : [ 0.870588, 0.870588, 0.870588, 1.0 ],
					"bgcolor" : [ 0.866667, 0.866667, 0.866667, 1.0 ],
					"fontsize" : 7.9,
					"numinlets" : 1,
					"patching_rect" : [ 491.0, 199.0, 35.0, 17.0 ],
					"triangle" : 0,
					"transparent" : 1,
					"numoutlets" : 2,
					"fontname" : "Arial",
					"outlettype" : [ "int", "bang" ],
					"maximum" : 255,
					"id" : "obj-16",
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
					"patching_rect" : [ 491.0, 185.0, 35.0, 17.0 ],
					"triangle" : 0,
					"transparent" : 1,
					"numoutlets" : 2,
					"fontname" : "Arial",
					"outlettype" : [ "int", "bang" ],
					"maximum" : 255,
					"id" : "obj-17",
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
					"patching_rect" : [ 491.0, 170.0, 35.0, 17.0 ],
					"triangle" : 0,
					"transparent" : 1,
					"numoutlets" : 2,
					"fontname" : "Arial",
					"outlettype" : [ "int", "bang" ],
					"maximum" : 255,
					"id" : "obj-18",
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
					"patching_rect" : [ 317.0, 160.0, 70.0, 15.0 ],
					"numoutlets" : 1,
					"fontname" : "Arial",
					"outlettype" : [ "" ],
					"hidden" : 1,
					"id" : "obj-19"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "newobj",
					"text" : "tap.pack 4 @triggers 1 1 1 1",
					"fontsize" : 7.9,
					"numinlets" : 4,
					"patching_rect" : [ 317.0, 242.0, 139.0, 17.0 ],
					"numoutlets" : 2,
					"fontname" : "Arial",
					"outlettype" : [ "", "" ],
					"hidden" : 1,
					"id" : "obj-20"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "newobj",
					"text" : "prepend brgb",
					"fontsize" : 7.9,
					"numinlets" : 1,
					"patching_rect" : [ 317.0, 264.0, 65.0, 17.0 ],
					"numoutlets" : 1,
					"fontname" : "Arial",
					"outlettype" : [ "" ],
					"hidden" : 1,
					"id" : "obj-21"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "number",
					"htextcolor" : [ 0.870588, 0.870588, 0.870588, 1.0 ],
					"bgcolor" : [ 0.866667, 0.866667, 0.866667, 1.0 ],
					"fontsize" : 7.9,
					"numinlets" : 1,
					"patching_rect" : [ 491.0, 213.0, 35.0, 17.0 ],
					"triangle" : 0,
					"transparent" : 1,
					"numoutlets" : 2,
					"fontname" : "Arial",
					"outlettype" : [ "int", "bang" ],
					"maximum" : 255,
					"id" : "obj-22",
					"triscale" : 0.9,
					"minimum" : 0,
					"textcolor" : [ 0.0, 0.0, 0.0, 1.0 ]
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "swatch",
					"numinlets" : 3,
					"patching_rect" : [ 317.0, 174.0, 157.0, 41.0 ],
					"numoutlets" : 2,
					"outlettype" : [ "", "float" ],
					"id" : "obj-23",
					"compatibility" : 1
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "panel",
					"bgcolor" : [ 0.501961, 0.54902, 0.733333, 1.0 ],
					"numinlets" : 1,
					"patching_rect" : [ 491.0, 154.0, 39.0, 81.0 ],
					"numoutlets" : 0,
					"rounded" : 0,
					"id" : "obj-24"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "panel",
					"bgcolor" : [ 0.807843, 0.807843, 0.807843, 1.0 ],
					"numinlets" : 1,
					"patching_rect" : [ 299.0, 154.0, 231.0, 81.0 ],
					"numoutlets" : 0,
					"rounded" : 0,
					"id" : "obj-25"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "comment",
					"text" : "X & Y Multiplier",
					"fontsize" : 7.9,
					"numinlets" : 1,
					"patching_rect" : [ 194.0, 180.0, 96.0, 17.0 ],
					"numoutlets" : 0,
					"fontname" : "Arial",
					"frgb" : [ 0.658824, 0.658824, 0.658824, 1.0 ],
					"id" : "obj-26"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "flonum",
					"htextcolor" : [ 0.870588, 0.870588, 0.870588, 1.0 ],
					"bgcolor" : [ 0.866667, 0.866667, 0.866667, 1.0 ],
					"fontsize" : 7.9,
					"numinlets" : 1,
					"patching_rect" : [ 170.0, 158.0, 35.0, 17.0 ],
					"numoutlets" : 2,
					"fontname" : "Arial",
					"outlettype" : [ "float", "bang" ],
					"id" : "obj-27",
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
					"patching_rect" : [ 132.0, 158.0, 35.0, 17.0 ],
					"numoutlets" : 2,
					"fontname" : "Arial",
					"outlettype" : [ "float", "bang" ],
					"id" : "obj-28",
					"triscale" : 0.9,
					"textcolor" : [ 0.0, 0.0, 0.0, 1.0 ]
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "newobj",
					"text" : "pak 1. 1.",
					"fontsize" : 7.9,
					"numinlets" : 2,
					"patching_rect" : [ 132.0, 177.0, 48.0, 17.0 ],
					"numoutlets" : 1,
					"fontname" : "Arial",
					"outlettype" : [ "" ],
					"id" : "obj-29"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "message",
					"text" : "multiplier $1 $2",
					"fontsize" : 7.9,
					"numinlets" : 2,
					"patching_rect" : [ 132.0, 198.0, 83.0, 15.0 ],
					"numoutlets" : 1,
					"fontname" : "Arial",
					"outlettype" : [ "" ],
					"id" : "obj-30"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "flonum",
					"htextcolor" : [ 0.870588, 0.870588, 0.870588, 1.0 ],
					"bgcolor" : [ 0.866667, 0.866667, 0.866667, 1.0 ],
					"fontsize" : 7.9,
					"numinlets" : 1,
					"patching_rect" : [ 342.0, 294.0, 35.0, 17.0 ],
					"numoutlets" : 2,
					"fontname" : "Arial",
					"outlettype" : [ "float", "bang" ],
					"id" : "obj-31",
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
					"patching_rect" : [ 385.0, 294.0, 35.0, 17.0 ],
					"numoutlets" : 2,
					"fontname" : "Arial",
					"outlettype" : [ "float", "bang" ],
					"id" : "obj-32",
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
					"patching_rect" : [ 453.0, 475.0, 56.0, 17.0 ],
					"numoutlets" : 2,
					"fontname" : "Arial",
					"outlettype" : [ "float", "bang" ],
					"id" : "obj-33",
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
					"patching_rect" : [ 313.0, 475.0, 56.0, 17.0 ],
					"numoutlets" : 2,
					"fontname" : "Arial",
					"outlettype" : [ "float", "bang" ],
					"id" : "obj-34",
					"triscale" : 0.9,
					"textcolor" : [ 0.0, 0.0, 0.0, 1.0 ]
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "message",
					"text" : "compile",
					"fontsize" : 7.9,
					"numinlets" : 2,
					"patching_rect" : [ 274.0, 297.0, 43.0, 15.0 ],
					"numoutlets" : 1,
					"fontname" : "Arial",
					"outlettype" : [ "" ],
					"id" : "obj-35"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "jsui",
					"numinlets" : 2,
					"patching_rect" : [ 313.0, 319.0, 150.0, 150.0 ],
					"numoutlets" : 2,
					"outlettype" : [ "", "" ],
					"id" : "obj-36",
					"jsarguments" : [  ],
					"filename" : "tap.xy-faderui.js"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "comment",
					"text" : "jsui two-dimensional controller",
					"fontsize" : 10.533333,
					"numinlets" : 1,
					"patching_rect" : [ 133.0, 34.0, 309.0, 21.0 ],
					"numoutlets" : 0,
					"fontname" : "Arial",
					"frgb" : [ 0.87451, 0.87451, 0.87451, 1.0 ],
					"id" : "obj-37",
					"textcolor" : [ 0.87451, 0.87451, 0.87451, 1.0 ]
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "comment",
					"text" : "tap.xy-faderui",
					"fontsize" : 21.066666,
					"numinlets" : 1,
					"patching_rect" : [ 133.0, 5.0, 309.0, 35.0 ],
					"numoutlets" : 0,
					"fontname" : "Arial",
					"frgb" : [ 0.87451, 0.87451, 0.87451, 1.0 ],
					"id" : "obj-38",
					"textcolor" : [ 0.87451, 0.87451, 0.87451, 1.0 ]
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "message",
					"text" : "set 1.",
					"fontsize" : 7.9,
					"numinlets" : 2,
					"patching_rect" : [ 466.0, 97.0, 35.0, 15.0 ],
					"numoutlets" : 1,
					"fontname" : "Arial",
					"outlettype" : [ "" ],
					"hidden" : 1,
					"id" : "obj-39"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "message",
					"text" : "set -6",
					"fontsize" : 7.9,
					"numinlets" : 2,
					"patching_rect" : [ 426.0, 97.0, 35.0, 15.0 ],
					"numoutlets" : 1,
					"fontname" : "Arial",
					"outlettype" : [ "" ],
					"hidden" : 1,
					"id" : "obj-40"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "message",
					"text" : "set 100",
					"fontsize" : 7.9,
					"numinlets" : 2,
					"patching_rect" : [ 404.0, 97.0, 44.0, 15.0 ],
					"numoutlets" : 1,
					"fontname" : "Arial",
					"outlettype" : [ "" ],
					"hidden" : 1,
					"id" : "obj-41"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "message",
					"text" : "set 50",
					"fontsize" : 7.9,
					"numinlets" : 2,
					"patching_rect" : [ 380.0, 97.0, 38.0, 15.0 ],
					"numoutlets" : 1,
					"fontname" : "Arial",
					"outlettype" : [ "" ],
					"hidden" : 1,
					"id" : "obj-42"
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
					"id" : "obj-43"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "comment",
					"text" : "Shift-click for finer precision",
					"fontsize" : 7.9,
					"numinlets" : 1,
					"patching_rect" : [ 155.0, 367.0, 176.0, 17.0 ],
					"numoutlets" : 0,
					"fontname" : "Arial",
					"frgb" : [ 0.658824, 0.658824, 0.658824, 1.0 ],
					"id" : "obj-44"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "comment",
					"text" : "Attributes",
					"fontsize" : 10.533333,
					"numinlets" : 1,
					"patching_rect" : [ 130.0, 134.0, 253.0, 21.0 ],
					"numoutlets" : 0,
					"fontname" : "Arial",
					"frgb" : [ 0.658824, 0.658824, 0.658824, 1.0 ],
					"id" : "obj-45"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "panel",
					"bgcolor" : [ 0.501961, 0.54902, 0.733333, 1.0 ],
					"numinlets" : 1,
					"patching_rect" : [ 128.0, 132.0, 402.0, 20.0 ],
					"numoutlets" : 0,
					"rounded" : 0,
					"id" : "obj-46"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "panel",
					"bgcolor" : [ 0.807843, 0.807843, 0.807843, 1.0 ],
					"numinlets" : 1,
					"patching_rect" : [ 128.0, 154.0, 169.0, 81.0 ],
					"numoutlets" : 0,
					"rounded" : 0,
					"id" : "obj-47"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "comment",
					"text" : "X",
					"fontsize" : 7.9,
					"numinlets" : 1,
					"patching_rect" : [ 325.0, 294.0, 100.0, 17.0 ],
					"numoutlets" : 0,
					"fontname" : "Arial",
					"frgb" : [ 0.658824, 0.658824, 0.658824, 1.0 ],
					"id" : "obj-48"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "panel",
					"bgcolor" : [ 0.807843, 0.807843, 0.807843, 1.0 ],
					"numinlets" : 1,
					"patching_rect" : [ 128.0, 236.0, 402.0, 81.0 ],
					"numoutlets" : 0,
					"rounded" : 0,
					"id" : "obj-49"
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
					"id" : "obj-50"
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
					"id" : "obj-51"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "message",
					"text" : ";\rmax launch_browser www.electrotap.com",
					"linecount" : 3,
					"fontsize" : 7.9,
					"background" : 1,
					"numinlets" : 2,
					"patching_rect" : [ 262.0, 86.0, 108.0, 36.0 ],
					"numoutlets" : 1,
					"fontname" : "Arial",
					"outlettype" : [ "" ],
					"hidden" : 1,
					"id" : "obj-52"
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
					"id" : "obj-53",
					"textcolor" : [ 0.0, 0.0, 0.0, 1.0 ]
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "comment",
					"text" : "by jesse allison",
					"fontsize" : 12.288889,
					"background" : 1,
					"numinlets" : 1,
					"patching_rect" : [ 133.0, 87.0, 125.0, 23.0 ],
					"numoutlets" : 0,
					"fontname" : "Arial",
					"frgb" : [ 0.658824, 0.658824, 0.658824, 1.0 ],
					"id" : "obj-54"
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
					"id" : "obj-55"
				}

			}
 ],
		"lines" : [ 			{
				"patchline" : 				{
					"source" : [ "obj-15", 2 ],
					"destination" : [ "obj-16", 0 ],
					"hidden" : 1
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-15", 1 ],
					"destination" : [ "obj-17", 0 ],
					"hidden" : 1
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-15", 0 ],
					"destination" : [ "obj-18", 0 ],
					"hidden" : 1
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-43", 0 ],
					"destination" : [ "obj-39", 0 ],
					"hidden" : 1
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-36", 1 ],
					"destination" : [ "obj-33", 0 ],
					"hidden" : 0
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-32", 0 ],
					"destination" : [ "obj-36", 1 ],
					"hidden" : 0
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-6", 0 ],
					"destination" : [ "obj-36", 1 ],
					"hidden" : 1
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-22", 0 ],
					"destination" : [ "obj-20", 3 ],
					"hidden" : 1
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-43", 0 ],
					"destination" : [ "obj-40", 0 ],
					"hidden" : 1
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-43", 0 ],
					"destination" : [ "obj-41", 0 ],
					"hidden" : 1
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-15", 2 ],
					"destination" : [ "obj-20", 2 ],
					"hidden" : 1
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-43", 0 ],
					"destination" : [ "obj-42", 0 ],
					"hidden" : 1
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-15", 1 ],
					"destination" : [ "obj-20", 1 ],
					"hidden" : 1
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-20", 0 ],
					"destination" : [ "obj-21", 0 ],
					"hidden" : 1
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-15", 0 ],
					"destination" : [ "obj-20", 0 ],
					"hidden" : 1
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-23", 0 ],
					"destination" : [ "obj-15", 0 ],
					"hidden" : 1
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-19", 0 ],
					"destination" : [ "obj-23", 0 ],
					"hidden" : 1
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-9", 0 ],
					"destination" : [ "obj-19", 0 ],
					"hidden" : 1
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-36", 0 ],
					"destination" : [ "obj-34", 0 ],
					"hidden" : 0
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-30", 0 ],
					"destination" : [ "obj-36", 0 ],
					"hidden" : 1
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-35", 0 ],
					"destination" : [ "obj-36", 0 ],
					"hidden" : 0
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-31", 0 ],
					"destination" : [ "obj-36", 0 ],
					"hidden" : 0
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-21", 0 ],
					"destination" : [ "obj-36", 0 ],
					"hidden" : 1
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-7", 0 ],
					"destination" : [ "obj-36", 0 ],
					"hidden" : 1
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-51", 0 ],
					"destination" : [ "obj-52", 0 ],
					"hidden" : 1
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-53", 0 ],
					"destination" : [ "obj-51", 0 ],
					"hidden" : 1
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-3", 0 ],
					"destination" : [ "obj-6", 0 ],
					"hidden" : 0
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-4", 0 ],
					"destination" : [ "obj-7", 2 ],
					"hidden" : 0
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-27", 0 ],
					"destination" : [ "obj-29", 1 ],
					"hidden" : 0
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-39", 0 ],
					"destination" : [ "obj-27", 0 ],
					"hidden" : 1
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-5", 0 ],
					"destination" : [ "obj-7", 1 ],
					"hidden" : 0
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-29", 0 ],
					"destination" : [ "obj-30", 0 ],
					"hidden" : 0
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-28", 0 ],
					"destination" : [ "obj-29", 0 ],
					"hidden" : 0
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-39", 0 ],
					"destination" : [ "obj-28", 0 ],
					"hidden" : 1
				}

			}
 ]
	}

}
