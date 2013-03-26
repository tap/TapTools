{
	"patcher" : 	{
		"fileversion" : 1,
		"rect" : [ 416.0, 65.0, 539.0, 705.0 ],
		"bglocked" : 0,
		"defrect" : [ 416.0, 65.0, 539.0, 705.0 ],
		"openrect" : [ 0.0, 0.0, 0.0, 0.0 ],
		"openinpresentation" : 0,
		"default_fontsize" : 10.0,
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
					"maxclass" : "comment",
					"text" : "Individual \"bits\"",
					"numoutlets" : 0,
					"fontsize" : 10.0,
					"frgb" : [ 0.333333, 0.333333, 0.333333, 1.0 ],
					"id" : "obj-88",
					"fontname" : "Verdana",
					"numinlets" : 1,
					"patching_rect" : [ 17.0, 387.0, 93.0, 19.0 ]
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "comment",
					"text" : "Poke in a \"bit\" value and a sample location in the buffer",
					"linecount" : 2,
					"numoutlets" : 0,
					"fontsize" : 10.0,
					"frgb" : [ 0.333333, 0.333333, 0.333333, 1.0 ],
					"id" : "obj-87",
					"fontname" : "Verdana",
					"numinlets" : 1,
					"patching_rect" : [ 98.0, 561.0, 158.0, 31.0 ]
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "message",
					"text" : "1.",
					"numoutlets" : 1,
					"fontsize" : 10.0,
					"hidden" : 1,
					"outlettype" : [ "" ],
					"id" : "obj-85",
					"fontname" : "Verdana",
					"numinlets" : 2,
					"patching_rect" : [ 380.0, 391.0, 35.0, 17.0 ]
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "message",
					"text" : "8.",
					"numoutlets" : 1,
					"fontsize" : 10.0,
					"hidden" : 1,
					"outlettype" : [ "" ],
					"id" : "obj-84",
					"fontname" : "Verdana",
					"numinlets" : 2,
					"patching_rect" : [ 325.0, 391.0, 35.0, 17.0 ]
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "flonum",
					"numoutlets" : 2,
					"fontsize" : 10.0,
					"outlettype" : [ "float", "bang" ],
					"id" : "obj-82",

					"fontname" : "Verdana",
					"numinlets" : 1,
					"patching_rect" : [ 380.0, 409.0, 52.0, 19.0 ]
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "flonum",
					"numoutlets" : 2,
					"fontsize" : 10.0,
					"outlettype" : [ "float", "bang" ],
					"id" : "obj-80",

					"fontname" : "Verdana",
					"numinlets" : 1,
					"patching_rect" : [ 325.0, 409.0, 52.0, 19.0 ]
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "message",
					"text" : "set helpbuf",
					"numoutlets" : 1,
					"fontsize" : 10.0,
					"outlettype" : [ "" ],
					"id" : "obj-77",
					"fontname" : "Verdana",
					"numinlets" : 2,
					"patching_rect" : [ 254.0, 433.0, 67.0, 17.0 ]
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "waveform~",
					"textcolor" : [  ],
					"numoutlets" : 6,
					"outlettype" : [ "float", "float", "float", "float", "list", "" ],
					"voffset" : 1.0,
					"id" : "obj-75",
					"buffername" : "helpbuf",
					"numinlets" : 5,
					"patching_rect" : [ 254.0, 462.0, 255.0, 126.0 ],
					"vzoom" : 8.0
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "message",
					"text" : "31 0.7",
					"numoutlets" : 1,
					"fontsize" : 10.0,
					"outlettype" : [ "" ],
					"id" : "obj-73",
					"fontname" : "Verdana",
					"numinlets" : 2,
					"patching_rect" : [ 132.0, 470.0, 44.0, 17.0 ]
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "message",
					"text" : "5 0.5",
					"numoutlets" : 1,
					"fontsize" : 10.0,
					"outlettype" : [ "" ],
					"id" : "obj-72",
					"fontname" : "Verdana",
					"numinlets" : 2,
					"patching_rect" : [ 132.0, 450.0, 38.0, 17.0 ]
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "comment",
					"text" : "trigger more than one pulse at a time by combining bits (don't go over 31)",
					"linecount" : 2,
					"numoutlets" : 0,
					"fontsize" : 10.0,
					"frgb" : [ 0.333333, 0.333333, 0.333333, 1.0 ],
					"id" : "obj-71",
					"fontname" : "Verdana",
					"numinlets" : 1,
					"patching_rect" : [ 97.0, 396.0, 202.0, 31.0 ]
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "message",
					"text" : "3 0.1",
					"numoutlets" : 1,
					"fontsize" : 10.0,
					"outlettype" : [ "" ],
					"id" : "obj-61",
					"fontname" : "Verdana",
					"numinlets" : 2,
					"patching_rect" : [ 132.0, 431.0, 38.0, 17.0 ]
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "comment",
					"text" : "Use the phasor to loop through the associated buffer x times per second.  As the triggering samples (bits) are passed, they send out a pulse to their associated outlets.  ",
					"linecount" : 3,
					"numoutlets" : 0,
					"fontsize" : 10.0,
					"frgb" : [ 0.333333, 0.333333, 0.333333, 1.0 ],
					"id" : "obj-60",
					"fontname" : "Verdana",
					"numinlets" : 1,
					"patching_rect" : [ 165.0, 125.0, 307.0, 43.0 ]
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "comment",
					"text" : "The buffer stores values that are used like bits.  [1, 2, 4, 8, 16] to address the 5 different pulse outlets.  Store a value at any location in the buffer and it will trigger the appropriate outlet.  ",
					"linecount" : 3,
					"numoutlets" : 0,
					"fontsize" : 10.0,
					"frgb" : [ 0.333333, 0.333333, 0.333333, 1.0 ],
					"id" : "obj-59",
					"fontname" : "Verdana",
					"numinlets" : 1,
					"patching_rect" : [ 90.0, 335.0, 339.0, 43.0 ]
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "newobj",
					"text" : "loadbang",
					"numoutlets" : 1,
					"fontsize" : 10.0,
					"hidden" : 1,
					"outlettype" : [ "bang" ],
					"id" : "obj-58",
					"fontname" : "Verdana",
					"numinlets" : 1,
					"patching_rect" : [ 222.0, 413.0, 57.0, 19.0 ]
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "message",
					"text" : "16 0.8",
					"numoutlets" : 1,
					"fontsize" : 10.0,
					"outlettype" : [ "" ],
					"id" : "obj-57",
					"fontname" : "Verdana",
					"numinlets" : 2,
					"patching_rect" : [ 51.0, 481.0, 44.0, 17.0 ]
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "message",
					"text" : "8 0.6",
					"numoutlets" : 1,
					"fontsize" : 10.0,
					"outlettype" : [ "" ],
					"id" : "obj-55",
					"fontname" : "Verdana",
					"numinlets" : 2,
					"patching_rect" : [ 51.0, 461.0, 38.0, 17.0 ]
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "message",
					"text" : "4 0.4",
					"numoutlets" : 1,
					"fontsize" : 10.0,
					"outlettype" : [ "" ],
					"id" : "obj-56",
					"fontname" : "Verdana",
					"numinlets" : 2,
					"patching_rect" : [ 51.0, 442.0, 38.0, 17.0 ]
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "message",
					"text" : "2 0.2",
					"numoutlets" : 1,
					"fontsize" : 10.0,
					"outlettype" : [ "" ],
					"id" : "obj-54",
					"fontname" : "Verdana",
					"numinlets" : 2,
					"patching_rect" : [ 51.0, 425.0, 38.0, 17.0 ]
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "message",
					"text" : "1 0.",
					"numoutlets" : 1,
					"fontsize" : 10.0,
					"outlettype" : [ "" ],
					"id" : "obj-53",
					"fontname" : "Verdana",
					"numinlets" : 2,
					"patching_rect" : [ 51.0, 406.0, 36.0, 17.0 ]
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "button",
					"numoutlets" : 1,
					"outlettype" : [ "bang" ],
					"id" : "obj-45",
					"numinlets" : 1,
					"patching_rect" : [ 279.0, 258.0, 20.0, 20.0 ]
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "newobj",
					"text" : "edge~",
					"numoutlets" : 2,
					"fontsize" : 10.0,
					"outlettype" : [ "bang", "bang" ],
					"id" : "obj-46",
					"fontname" : "Verdana",
					"numinlets" : 1,
					"patching_rect" : [ 279.0, 234.0, 44.0, 19.0 ]
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "button",
					"numoutlets" : 1,
					"outlettype" : [ "bang" ],
					"id" : "obj-49",
					"numinlets" : 1,
					"patching_rect" : [ 232.0, 258.0, 20.0, 20.0 ]
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "newobj",
					"text" : "edge~",
					"numoutlets" : 2,
					"fontsize" : 10.0,
					"outlettype" : [ "bang", "bang" ],
					"id" : "obj-50",
					"fontname" : "Verdana",
					"numinlets" : 1,
					"patching_rect" : [ 232.0, 234.0, 44.0, 19.0 ]
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "flonum",
					"numoutlets" : 2,
					"fontsize" : 10.0,
					"outlettype" : [ "float", "bang" ],
					"id" : "obj-42",

					"fontname" : "Verdana",
					"numinlets" : 1,
					"patching_rect" : [ 91.0, 122.0, 52.0, 19.0 ]
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "button",
					"numoutlets" : 1,
					"outlettype" : [ "bang" ],
					"id" : "obj-39",
					"numinlets" : 1,
					"patching_rect" : [ 185.0, 258.0, 20.0, 20.0 ]
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "newobj",
					"text" : "edge~",
					"numoutlets" : 2,
					"fontsize" : 10.0,
					"outlettype" : [ "bang", "bang" ],
					"id" : "obj-40",
					"fontname" : "Verdana",
					"numinlets" : 1,
					"patching_rect" : [ 185.0, 234.0, 44.0, 19.0 ]
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "button",
					"numoutlets" : 1,
					"outlettype" : [ "bang" ],
					"id" : "obj-35",
					"numinlets" : 1,
					"patching_rect" : [ 138.0, 258.0, 20.0, 20.0 ]
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "newobj",
					"text" : "edge~",
					"numoutlets" : 2,
					"fontsize" : 10.0,
					"outlettype" : [ "bang", "bang" ],
					"id" : "obj-36",
					"fontname" : "Verdana",
					"numinlets" : 1,
					"patching_rect" : [ 138.0, 234.0, 44.0, 19.0 ]
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "message",
					"text" : "clear",
					"numoutlets" : 1,
					"fontsize" : 10.0,
					"outlettype" : [ "" ],
					"id" : "obj-31",
					"fontname" : "Verdana",
					"numinlets" : 2,
					"patching_rect" : [ 148.0, 299.0, 37.0, 17.0 ]
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "newobj",
					"text" : "* 44100.",
					"numoutlets" : 1,
					"fontsize" : 10.0,
					"outlettype" : [ "float" ],
					"id" : "obj-23",
					"fontname" : "Verdana",
					"numinlets" : 2,
					"patching_rect" : [ 46.0, 542.0, 57.0, 19.0 ]
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "newobj",
					"text" : "poke~ helpbuf",
					"numoutlets" : 0,
					"fontsize" : 10.0,
					"id" : "obj-22",
					"fontname" : "Verdana",
					"numinlets" : 3,
					"patching_rect" : [ 17.0, 568.0, 85.0, 19.0 ]
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "newobj",
					"text" : "unpack 0 0.",
					"numoutlets" : 2,
					"fontsize" : 10.0,
					"outlettype" : [ "int", "float" ],
					"id" : "obj-21",
					"fontname" : "Verdana",
					"numinlets" : 1,
					"patching_rect" : [ 17.0, 516.0, 71.0, 19.0 ]
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "button",
					"numoutlets" : 1,
					"outlettype" : [ "bang" ],
					"id" : "obj-19",
					"numinlets" : 1,
					"patching_rect" : [ 92.0, 258.0, 20.0, 20.0 ]
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "newobj",
					"text" : "edge~",
					"numoutlets" : 2,
					"fontsize" : 10.0,
					"outlettype" : [ "bang", "bang" ],
					"id" : "obj-17",
					"fontname" : "Verdana",
					"numinlets" : 1,
					"patching_rect" : [ 92.0, 234.0, 44.0, 19.0 ]
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "ezdac~",
					"numoutlets" : 0,
					"id" : "obj-8",
					"numinlets" : 2,
					"patching_rect" : [ 210.0, 637.0, 45.0, 45.0 ]
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "bpatcher",
					"numoutlets" : 0,
					"id" : "obj-69",
					"name" : "tap.badge.maxpat",
					"args" : [  ],
					"numinlets" : 0,
					"patching_rect" : [ 292.0, 625.0, 225.0, 67.0 ]
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
					"patching_rect" : [ 24.0, 633.0, 54.0, 19.0 ]
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
					"patching_rect" : [ 29.0, 637.0, 100.0, 20.0 ]
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "umenu",
					"varname" : "autohelp_see_menu",
					"numoutlets" : 3,
					"fontsize" : 11.595187,
					"items" : [ "(Objects:)", ",", "phasor~", ",", "tap.pulsesub~", ",", "tap.ramp~", ",", "<separator>" ],
					"outlettype" : [ "int", "", "" ],
					"id" : "obj-67",
					"fontname" : "Arial",
					"types" : [  ],
					"numinlets" : 1,
					"patching_rect" : [ 29.0, 657.0, 130.0, 20.0 ]
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "newobj",
					"text" : "phasor~ 1.",
					"numoutlets" : 1,
					"fontsize" : 10.0,
					"outlettype" : [ "signal" ],
					"id" : "obj-3",
					"fontname" : "Verdana",
					"numinlets" : 2,
					"patching_rect" : [ 91.0, 143.0, 68.0, 19.0 ]
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "newobj",
					"text" : "buffer~ helpbuf 1000",
					"numoutlets" : 2,
					"fontsize" : 10.0,
					"outlettype" : [ "float", "bang" ],
					"id" : "obj-4",
					"fontname" : "Verdana",
					"numinlets" : 1,
					"patching_rect" : [ 148.0, 319.0, 120.0, 19.0 ]
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "comment",
					"text" : "required argument: name of a buffer",
					"numoutlets" : 0,
					"fontsize" : 10.0,
					"frgb" : [ 0.333333, 0.333333, 0.333333, 1.0 ],
					"id" : "obj-5",
					"fontname" : "Verdana",
					"numinlets" : 1,
					"patching_rect" : [ 110.0, 190.0, 205.0, 19.0 ]
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "newobj",
					"text" : "tap.pulserouter~ helpbuf 5",
					"numoutlets" : 5,
					"fontsize" : 12.0,
					"outlettype" : [ "signal", "signal", "signal", "signal", "signal" ],
					"id" : "obj-6",
					"fontname" : "Verdana",
					"numinlets" : 1,
					"patching_rect" : [ 91.0, 207.0, 207.0, 21.0 ]
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
					"patching_rect" : [ 24.0, 633.0, 140.0, 50.0 ]
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "comment",
					"varname" : "autohelp_top_title",
					"text" : "tap.pulserouter~",
					"textcolor" : [ 0.2, 0.2, 0.2, 1.0 ],
					"numoutlets" : 0,
					"fontface" : 3,
					"fontsize" : 20.871338,
					"frgb" : [ 0.2, 0.2, 0.2, 1.0 ],
					"id" : "obj-9",
					"fontname" : "Arial",
					"numinlets" : 1,
					"patching_rect" : [ 10.0, 8.0, 485.0, 30.0 ]
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "comment",
					"varname" : "autohelp_top_digest",
					"text" : "Generate pulses from a ramp and a buffer~",
					"textcolor" : [ 0.2, 0.2, 0.2, 1.0 ],
					"numoutlets" : 0,
					"fontsize" : 12.754705,
					"frgb" : [ 0.2, 0.2, 0.2, 1.0 ],
					"id" : "obj-10",
					"fontname" : "Arial",
					"numinlets" : 1,
					"patching_rect" : [ 10.0, 36.0, 485.0, 21.0 ]
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "comment",
					"varname" : "autohelp_top_description",
					"text" : "tap.pulserouter~ generates pulses for up to 5 different processes by scanning through a buffer. The buffer stores bits that are used to trigger a pulse on a specific outlet. One application for this is triggering grains in granular synthesis (e.g. in conjunction with Nathan Wolek's Granular ToolKit).",
					"linecount" : 4,
					"numoutlets" : 0,
					"fontsize" : 11.595187,
					"frgb" : [ 0.333333, 0.333333, 0.333333, 1.0 ],
					"id" : "obj-11",
					"fontname" : "Arial",
					"numinlets" : 1,
					"patching_rect" : [ 10.0, 57.0, 510.0, 60.0 ]
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "textbutton",
					"varname" : "autohelp_top_ref",
					"bgcolor" : [ 0.0, 0.0, 0.0, 0.0 ],
					"bgoveroncolor" : [ 0.0, 0.0, 0.0, 0.0 ],
					"underline" : 1,
					"textcolor" : [ 0.296, 0.38, 0.506, 1.0 ],
					"numoutlets" : 3,
					"fontface" : 3,
					"fontsize" : 12.754705,
					"spacing_x" : 0.0,
					"bgoncolor" : [ 0.0, 0.0, 0.0, 0.0 ],
					"outlettype" : [ "", "", "int" ],
					"bordercolor" : [ 0.0, 0.0, 0.0, 0.0 ],
					"spacing_y" : 0.0,
					"textoncolor" : [ 0.27, 0.35, 0.47, 1.0 ],
					"presentation_rect" : [ 0.0, 0.0, 203.275009, 14.666666 ],
					"id" : "obj-12",
					"fontlink" : 1,
					"text" : "open tap.pulserouter~ reference",
					"textovercolor" : [ 0.4, 0.5, 0.65, 1.0 ],
					"fontname" : "Arial",
					"bgovercolor" : [ 0.0, 0.0, 0.0, 0.0 ],
					"numinlets" : 1,
					"patching_rect" : [ 296.724976, 22.0, 203.275009, 14.666666 ]
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "panel",
					"varname" : "autohelp_top_panel",
					"grad2" : [ 0.9, 0.9, 0.9, 1.0 ],
					"numoutlets" : 0,
					"mode" : 1,
					"id" : "obj-13",
					"background" : 1,
					"grad1" : [ 0.88, 0.98, 0.78, 1.0 ],
					"numinlets" : 1,
					"patching_rect" : [ 5.0, 5.0, 495.0, 52.0 ]
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "message",
					"text" : "vzoom $1",
					"numoutlets" : 1,
					"fontsize" : 10.0,
					"outlettype" : [ "" ],
					"id" : "obj-78",
					"fontname" : "Verdana",
					"numinlets" : 2,
					"patching_rect" : [ 325.0, 433.0, 58.0, 17.0 ]
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "message",
					"text" : "voffset $1",
					"numoutlets" : 1,
					"fontsize" : 10.0,
					"outlettype" : [ "" ],
					"id" : "obj-81",
					"fontname" : "Verdana",
					"numinlets" : 2,
					"patching_rect" : [ 380.0, 433.0, 59.0, 17.0 ]
				}

			}
 ],
		"lines" : [ 			{
				"patchline" : 				{
					"source" : [ "obj-17", 0 ],
					"destination" : [ "obj-19", 0 ],
					"hidden" : 0,
					"midpoints" : [  ]
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-23", 0 ],
					"destination" : [ "obj-22", 1 ],
					"hidden" : 0,
					"midpoints" : [  ]
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-31", 0 ],
					"destination" : [ "obj-4", 0 ],
					"hidden" : 0,
					"midpoints" : [  ]
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-6", 0 ],
					"destination" : [ "obj-17", 0 ],
					"hidden" : 0,
					"midpoints" : [  ]
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-3", 0 ],
					"destination" : [ "obj-6", 0 ],
					"hidden" : 0,
					"midpoints" : [  ]
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-36", 0 ],
					"destination" : [ "obj-35", 0 ],
					"hidden" : 0,
					"midpoints" : [  ]
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-6", 1 ],
					"destination" : [ "obj-36", 0 ],
					"hidden" : 0,
					"midpoints" : [  ]
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-40", 0 ],
					"destination" : [ "obj-39", 0 ],
					"hidden" : 0,
					"midpoints" : [  ]
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-6", 2 ],
					"destination" : [ "obj-40", 0 ],
					"hidden" : 0,
					"midpoints" : [  ]
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-42", 0 ],
					"destination" : [ "obj-3", 0 ],
					"hidden" : 0,
					"midpoints" : [  ]
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-46", 0 ],
					"destination" : [ "obj-45", 0 ],
					"hidden" : 0,
					"midpoints" : [  ]
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-50", 0 ],
					"destination" : [ "obj-49", 0 ],
					"hidden" : 0,
					"midpoints" : [  ]
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-6", 3 ],
					"destination" : [ "obj-50", 0 ],
					"hidden" : 0,
					"midpoints" : [  ]
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-6", 4 ],
					"destination" : [ "obj-46", 0 ],
					"hidden" : 0,
					"midpoints" : [  ]
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-58", 0 ],
					"destination" : [ "obj-53", 0 ],
					"hidden" : 1,
					"midpoints" : [  ]
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-58", 0 ],
					"destination" : [ "obj-54", 0 ],
					"hidden" : 1,
					"midpoints" : [  ]
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-58", 0 ],
					"destination" : [ "obj-56", 0 ],
					"hidden" : 1,
					"midpoints" : [  ]
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-58", 0 ],
					"destination" : [ "obj-55", 0 ],
					"hidden" : 1,
					"midpoints" : [  ]
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-58", 0 ],
					"destination" : [ "obj-57", 0 ],
					"hidden" : 1,
					"midpoints" : [  ]
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-73", 0 ],
					"destination" : [ "obj-21", 0 ],
					"hidden" : 0,
					"midpoints" : [ 141.5, 507.0, 26.5, 507.0 ]
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-72", 0 ],
					"destination" : [ "obj-21", 0 ],
					"hidden" : 0,
					"midpoints" : [ 141.5, 468.0, 113.0, 468.0, 113.0, 507.0, 26.5, 507.0 ]
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-61", 0 ],
					"destination" : [ "obj-21", 0 ],
					"hidden" : 0,
					"midpoints" : [ 141.5, 447.0, 113.0, 447.0, 113.0, 507.0, 26.5, 507.0 ]
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-57", 0 ],
					"destination" : [ "obj-21", 0 ],
					"hidden" : 0,
					"midpoints" : [ 60.5, 507.0, 26.5, 507.0 ]
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-55", 0 ],
					"destination" : [ "obj-21", 0 ],
					"hidden" : 0,
					"midpoints" : [ 60.5, 477.0, 26.5, 477.0 ]
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-56", 0 ],
					"destination" : [ "obj-21", 0 ],
					"hidden" : 0,
					"midpoints" : [ 60.5, 459.0, 26.5, 459.0 ]
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-54", 0 ],
					"destination" : [ "obj-21", 0 ],
					"hidden" : 0,
					"midpoints" : [ 60.5, 441.0, 26.5, 441.0 ]
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-53", 0 ],
					"destination" : [ "obj-21", 0 ],
					"hidden" : 0,
					"midpoints" : [ 60.5, 423.0, 26.5, 423.0 ]
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-21", 1 ],
					"destination" : [ "obj-23", 0 ],
					"hidden" : 0,
					"midpoints" : [  ]
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-21", 0 ],
					"destination" : [ "obj-22", 0 ],
					"hidden" : 0,
					"midpoints" : [  ]
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-77", 0 ],
					"destination" : [ "obj-75", 0 ],
					"hidden" : 0,
					"midpoints" : [  ]
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-58", 0 ],
					"destination" : [ "obj-77", 0 ],
					"hidden" : 1,
					"midpoints" : [  ]
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-78", 0 ],
					"destination" : [ "obj-75", 0 ],
					"hidden" : 0,
					"midpoints" : [  ]
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-80", 0 ],
					"destination" : [ "obj-78", 0 ],
					"hidden" : 0,
					"midpoints" : [  ]
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-81", 0 ],
					"destination" : [ "obj-75", 0 ],
					"hidden" : 0,
					"midpoints" : [  ]
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-82", 0 ],
					"destination" : [ "obj-81", 0 ],
					"hidden" : 0,
					"midpoints" : [  ]
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-58", 0 ],
					"destination" : [ "obj-84", 0 ],
					"hidden" : 1,
					"midpoints" : [  ]
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-84", 0 ],
					"destination" : [ "obj-80", 0 ],
					"hidden" : 1,
					"midpoints" : [  ]
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-58", 0 ],
					"destination" : [ "obj-85", 0 ],
					"hidden" : 1,
					"midpoints" : [  ]
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-85", 0 ],
					"destination" : [ "obj-82", 0 ],
					"hidden" : 1,
					"midpoints" : [  ]
				}

			}
 ]
	}

}
