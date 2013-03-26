{
	"patcher" : 	{
		"fileversion" : 1,
		"rect" : [ 264.0, 75.0, 511.0, 638.0 ],
		"bglocked" : 0,
		"defrect" : [ 264.0, 75.0, 511.0, 638.0 ],
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
					"maxclass" : "comment",
					"text" : "The script method specifies a complete script as a message rather than reading in a file.  Then click bang to execute it.",
					"linecount" : 4,
					"fontsize" : 10.0,
					"numinlets" : 1,
					"frgb" : [ 0.658824, 0.658824, 0.658824, 1.0 ],
					"numoutlets" : 0,
					"patching_rect" : [ 268.0, 111.0, 161.0, 55.0 ],
					"id" : "obj-2",
					"fontname" : "Verdana"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "comment",
					"text" : "NOTE: The 'set' method is no longer supported in TapTools 3.  It has been replaced by the script method.",
					"linecount" : 3,
					"fontsize" : 10.0,
					"numinlets" : 1,
					"frgb" : [ 0.658824, 0.658824, 0.658824, 1.0 ],
					"numoutlets" : 0,
					"patching_rect" : [ 278.0, 262.0, 195.0, 43.0 ],
					"id" : "obj-1",
					"fontname" : "Verdana"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "message",
					"text" : "script beep 3",
					"fontsize" : 10.0,
					"numinlets" : 2,
					"numoutlets" : 1,
					"patching_rect" : [ 259.0, 171.0, 75.0, 17.0 ],
					"outlettype" : [ "" ],
					"id" : "obj-14",
					"fontname" : "Verdana"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "bpatcher",
					"args" : [  ],
					"numinlets" : 0,
					"numoutlets" : 0,
					"patching_rect" : [ 241.0, 546.0, 225.0, 67.0 ],
					"id" : "obj-69",
					"name" : "tap.badge.maxpat"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "newobj",
					"text" : "autohelp",
					"fontsize" : 10.0,
					"hidden" : 1,
					"numinlets" : 1,
					"numoutlets" : 0,
					"patching_rect" : [ 15.0, 554.0, 54.0, 19.0 ],
					"id" : "obj-52",
					"fontname" : "Verdana",
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
					"fontface" : 1,
					"fontsize" : 11.595187,
					"numinlets" : 1,
					"frgb" : [ 0.0, 0.0, 0.0, 1.0 ],
					"numoutlets" : 0,
					"patching_rect" : [ 20.0, 558.0, 100.0, 20.0 ],
					"id" : "obj-22",
					"fontname" : "Arial",
					"textcolor" : [ 0.0, 0.0, 0.0, 1.0 ]
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "umenu",
					"varname" : "autohelp_see_menu",
					"fontsize" : 11.595187,
					"numinlets" : 1,
					"items" : [ "(Objects:)", ",", "shell", ",", "<separator>" ],
					"numoutlets" : 3,
					"types" : [  ],
					"patching_rect" : [ 20.0, 578.0, 130.0, 20.0 ],
					"outlettype" : [ "int", "", "" ],
					"id" : "obj-23",
					"fontname" : "Arial"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "newobj",
					"text" : "p \"Common Errors\"",
					"fontsize" : 10.0,
					"numinlets" : 0,
					"numoutlets" : 0,
					"patching_rect" : [ 279.0, 524.0, 116.0, 19.0 ],
					"id" : "obj-24",
					"fontname" : "Verdana",
					"patcher" : 					{
						"fileversion" : 1,
						"rect" : [ 351.0, 278.0, 306.0, 137.0 ],
						"bglocked" : 0,
						"defrect" : [ 351.0, 278.0, 306.0, 137.0 ],
						"openrect" : [ 0.0, 0.0, 0.0, 0.0 ],
						"openinpresentation" : 0,
						"default_fontsize" : 9.0,
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
						"boxes" : [ 							{
								"box" : 								{
									"maxclass" : "comment",
									"text" : "-39: End of File Error;\r-1752: File doesn't seem to belong to AppleScript;\r-1753: Script Error;\r",
									"linecount" : 3,
									"fontsize" : 10.0,
									"numinlets" : 1,
									"frgb" : [ 0.658824, 0.658824, 0.658824, 1.0 ],
									"numoutlets" : 0,
									"patching_rect" : [ 13.0, 42.0, 263.0, 43.0 ],
									"id" : "obj-1",
									"fontname" : "Verdana"
								}

							}
, 							{
								"box" : 								{
									"maxclass" : "comment",
									"text" : "Some common errors:",
									"fontsize" : 12.0,
									"numinlets" : 1,
									"frgb" : [ 0.658824, 0.658824, 0.658824, 1.0 ],
									"numoutlets" : 0,
									"patching_rect" : [ 11.0, 20.0, 166.0, 21.0 ],
									"id" : "obj-2",
									"fontname" : "Verdana"
								}

							}
, 							{
								"box" : 								{
									"maxclass" : "panel",
									"numinlets" : 1,
									"border" : 1,
									"numoutlets" : 0,
									"patching_rect" : [ 7.0, 15.0, 278.0, 86.0 ],
									"id" : "obj-3",
									"rounded" : 15
								}

							}
 ],
						"lines" : [  ]
					}
,
					"saved_object_attributes" : 					{
						"globalpatchername" : "",
						"fontface" : 0,
						"fontsize" : 9.0,
						"default_fontface" : 0,
						"default_fontname" : "Verdana",
						"fontname" : "Verdana",
						"default_fontsize" : 9.0
					}

				}

			}
, 			{
				"box" : 				{
					"maxclass" : "comment",
					"text" : "You will probably need to edit this last script to get it to work properly on your machine because of the file path.",
					"linecount" : 3,
					"fontsize" : 10.0,
					"numinlets" : 1,
					"frgb" : [ 0.658824, 0.658824, 0.658824, 1.0 ],
					"numoutlets" : 0,
					"patching_rect" : [ 17.0, 382.0, 219.0, 43.0 ],
					"id" : "obj-25",
					"fontname" : "Verdana"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "comment",
					"text" : "read a file (file name optional)",
					"linecount" : 2,
					"fontsize" : 10.0,
					"numinlets" : 1,
					"frgb" : [ 0.658824, 0.658824, 0.658824, 1.0 ],
					"numoutlets" : 0,
					"patching_rect" : [ 52.0, 192.0, 100.0, 31.0 ],
					"id" : "obj-26",
					"fontname" : "Verdana"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "message",
					"text" : "read",
					"fontsize" : 10.0,
					"numinlets" : 2,
					"numoutlets" : 1,
					"patching_rect" : [ 16.0, 197.0, 34.0, 17.0 ],
					"outlettype" : [ "" ],
					"id" : "obj-27",
					"fontname" : "Verdana"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "message",
					"text" : "read tap.example.moveFile.scpt",
					"linecount" : 2,
					"fontsize" : 10.0,
					"numinlets" : 2,
					"numoutlets" : 1,
					"patching_rect" : [ 16.0, 326.0, 146.0, 29.0 ],
					"outlettype" : [ "" ],
					"id" : "obj-28",
					"fontname" : "Verdana"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "message",
					"text" : "read tap.example.website.scpt",
					"linecount" : 2,
					"fontsize" : 10.0,
					"numinlets" : 2,
					"numoutlets" : 1,
					"patching_rect" : [ 16.0, 277.0, 145.0, 29.0 ],
					"outlettype" : [ "" ],
					"id" : "obj-29",
					"fontname" : "Verdana"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "comment",
					"text" : "bang runs the script",
					"linecount" : 2,
					"fontsize" : 10.0,
					"numinlets" : 1,
					"frgb" : [ 0.658824, 0.658824, 0.658824, 1.0 ],
					"numoutlets" : 0,
					"patching_rect" : [ 27.0, 155.0, 65.0, 31.0 ],
					"id" : "obj-30",
					"fontname" : "Verdana"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "button",
					"numinlets" : 1,
					"numoutlets" : 1,
					"patching_rect" : [ 5.0, 158.0, 21.0, 21.0 ],
					"outlettype" : [ "bang" ],
					"id" : "obj-31"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "comment",
					"text" : "second outlet notifies when done reading a script file",
					"fontsize" : 10.0,
					"numinlets" : 1,
					"frgb" : [ 0.658824, 0.658824, 0.658824, 1.0 ],
					"numoutlets" : 0,
					"patching_rect" : [ 175.0, 465.0, 291.0, 19.0 ],
					"id" : "obj-32",
					"fontname" : "Verdana"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "newobj",
					"text" : "print",
					"fontsize" : 10.0,
					"numinlets" : 1,
					"numoutlets" : 0,
					"patching_rect" : [ 135.0, 465.0, 38.0, 19.0 ],
					"id" : "obj-33",
					"fontname" : "Verdana"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "message",
					"text" : "read tap.example.dialog.scpt",
					"linecount" : 2,
					"fontsize" : 10.0,
					"numinlets" : 2,
					"numoutlets" : 1,
					"patching_rect" : [ 16.0, 227.0, 144.0, 29.0 ],
					"outlettype" : [ "" ],
					"id" : "obj-34",
					"fontname" : "Verdana"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "newobj",
					"text" : "tap.applescript tap.example.dialog.scpt",
					"fontsize" : 10.0,
					"numinlets" : 1,
					"numoutlets" : 3,
					"patching_rect" : [ 5.0, 426.0, 220.0, 19.0 ],
					"outlettype" : [ "", "", "" ],
					"id" : "obj-36",
					"fontname" : "Verdana"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "comment",
					"text" : "data returned from the script",
					"fontsize" : 10.0,
					"numinlets" : 1,
					"frgb" : [ 0.658824, 0.658824, 0.658824, 1.0 ],
					"numoutlets" : 0,
					"patching_rect" : [ 5.0, 526.0, 190.0, 19.0 ],
					"id" : "obj-37",
					"fontname" : "Verdana"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "comment",
					"text" : "Optional argument sets default script file to read",
					"linecount" : 2,
					"fontsize" : 10.0,
					"numinlets" : 1,
					"frgb" : [ 0.658824, 0.658824, 0.658824, 1.0 ],
					"numoutlets" : 0,
					"patching_rect" : [ 230.0, 425.0, 150.0, 31.0 ],
					"id" : "obj-38",
					"fontname" : "Verdana"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "message",
					"fontsize" : 10.0,
					"numinlets" : 2,
					"numoutlets" : 1,
					"patching_rect" : [ 5.0, 506.0, 175.0, 17.0 ],
					"outlettype" : [ "" ],
					"id" : "obj-39",
					"fontname" : "Verdana"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "comment",
					"varname" : "autohelp_top_title",
					"text" : "tap.applescript",
					"fontface" : 3,
					"fontsize" : 20.871338,
					"numinlets" : 1,
					"frgb" : [ 0.93, 0.93, 0.97, 1.0 ],
					"numoutlets" : 0,
					"patching_rect" : [ 10.0, 8.0, 485.0, 30.0 ],
					"id" : "obj-17",
					"fontname" : "Arial",
					"textcolor" : [ 0.93, 0.93, 0.97, 1.0 ]
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "comment",
					"varname" : "autohelp_top_digest",
					"text" : "Run AppleScripts within Max",
					"fontsize" : 12.754705,
					"numinlets" : 1,
					"frgb" : [ 0.93, 0.93, 0.97, 1.0 ],
					"numoutlets" : 0,
					"patching_rect" : [ 10.0, 36.0, 485.0, 21.0 ],
					"id" : "obj-18",
					"fontname" : "Arial",
					"textcolor" : [ 0.93, 0.93, 0.97, 1.0 ]
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "comment",
					"varname" : "autohelp_top_description",
					"text" : "The tap.applescript object compiles and runs AppleScripts in Max. The script can be passed in dynamically or read from a file.",
					"linecount" : 2,
					"fontsize" : 10.0,
					"numinlets" : 1,
					"frgb" : [ 0.658824, 0.658824, 0.658824, 1.0 ],
					"numoutlets" : 0,
					"patching_rect" : [ 10.0, 57.0, 457.0, 31.0 ],
					"id" : "obj-19",
					"fontname" : "Verdana"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "textbutton",
					"varname" : "autohelp_top_ref",
					"bgovercolor" : [ 0.0, 0.0, 0.0, 0.0 ],
					"fontface" : 3,
					"fontsize" : 12.754705,
					"bgoveroncolor" : [ 0.0, 0.0, 0.0, 0.0 ],
					"numinlets" : 1,
					"presentation_rect" : [ 0.0, 0.0, 193.709, 14.666666 ],
					"spacing_x" : 0.0,
					"bgoncolor" : [ 0.0, 0.0, 0.0, 0.0 ],
					"numoutlets" : 3,
					"bgcolor" : [ 0.0, 0.0, 0.0, 0.0 ],
					"text" : "open tap.applescript reference",
					"spacing_y" : 0.0,
					"bordercolor" : [ 0.0, 0.0, 0.0, 0.0 ],
					"textoncolor" : [ 0.27, 0.35, 0.47, 1.0 ],
					"patching_rect" : [ 306.291016, 22.0, 193.709, 14.666666 ],
					"outlettype" : [ "", "", "int" ],
					"id" : "obj-20",
					"fontname" : "Arial",
					"underline" : 1,
					"fontlink" : 1,
					"textcolor" : [ 0.4, 0.5, 0.65, 1.0 ],
					"textovercolor" : [ 0.4, 0.5, 0.65, 1.0 ]
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "panel",
					"varname" : "autohelp_top_panel",
					"grad1" : [ 0.27, 0.35, 0.47, 1.0 ],
					"grad2" : [ 0.85, 0.85, 0.85, 1.0 ],
					"numinlets" : 1,
					"mode" : 1,
					"numoutlets" : 0,
					"patching_rect" : [ 5.0, 5.0, 495.0, 52.0 ],
					"id" : "obj-21",
					"background" : 1
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "panel",
					"varname" : "autohelp_see_panel",
					"numinlets" : 1,
					"border" : 2,
					"numoutlets" : 0,
					"bgcolor" : [ 0.85, 0.85, 0.85, 0.75 ],
					"bordercolor" : [ 0.5, 0.5, 0.5, 0.75 ],
					"patching_rect" : [ 15.0, 554.0, 140.0, 50.0 ],
					"id" : "obj-40",
					"background" : 1
				}

			}
 ],
		"lines" : [ 			{
				"patchline" : 				{
					"source" : [ "obj-14", 0 ],
					"destination" : [ "obj-36", 0 ],
					"hidden" : 0,
					"midpoints" : [ 268.5, 374.0, 3.0, 374.0, 3.0, 423.0, 14.5, 423.0 ]
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-28", 0 ],
					"destination" : [ "obj-36", 0 ],
					"hidden" : 0,
					"midpoints" : [ 25.5, 370.0, 14.5, 370.0 ]
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-29", 0 ],
					"destination" : [ "obj-36", 0 ],
					"hidden" : 0,
					"midpoints" : [ 25.5, 320.0, 14.5, 320.0 ]
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-31", 0 ],
					"destination" : [ "obj-36", 0 ],
					"hidden" : 0,
					"midpoints" : [  ]
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-34", 0 ],
					"destination" : [ "obj-36", 0 ],
					"hidden" : 0,
					"midpoints" : [ 25.5, 271.0, 14.5, 271.0 ]
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-27", 0 ],
					"destination" : [ "obj-36", 0 ],
					"hidden" : 0,
					"midpoints" : [ 25.5, 216.0, 14.5, 216.0 ]
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-36", 1 ],
					"destination" : [ "obj-33", 0 ],
					"hidden" : 0,
					"midpoints" : [ 115.0, 454.0, 144.5, 454.0 ]
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-36", 0 ],
					"destination" : [ "obj-39", 1 ],
					"hidden" : 0,
					"midpoints" : [ 14.5, 499.0, 170.5, 499.0 ]
				}

			}
 ]
	}

}
