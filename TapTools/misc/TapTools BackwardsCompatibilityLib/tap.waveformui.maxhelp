{
	"patcher" : 	{
		"rect" : [ 411.0, 68.0, 562.0, 694.0 ],
		"bgcolor" : [ 0.1, 0.1, 0.1, 1.0 ],
		"bglocked" : 0,
		"defrect" : [ 411.0, 68.0, 562.0, 694.0 ],
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
					"maxclass" : "message",
					"text" : "set 5000.",
					"fontsize" : 7.9,
					"numinlets" : 2,
					"patching_rect" : [ 445.0, 103.0, 53.0, 15.0 ],
					"numoutlets" : 1,
					"fontname" : "Arial",
					"outlettype" : [ "" ],
					"hidden" : 1,
					"id" : "obj-1"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "comment",
					"text" : "set the position of the selection region by beginning point and length send the set message into inlet 2-6 to set the corresponding region w\/o output [set beginning length]",
					"linecount" : 3,
					"fontsize" : 7.9,
					"numinlets" : 1,
					"patching_rect" : [ 131.0, 215.0, 270.0, 38.0 ],
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
					"patching_rect" : [ 484.0, 209.0, 45.0, 17.0 ],
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
					"patching_rect" : [ 431.0, 209.0, 45.0, 17.0 ],
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
					"maxclass" : "newobj",
					"text" : "pak set 200. 1200.",
					"fontsize" : 7.9,
					"numinlets" : 3,
					"patching_rect" : [ 418.0, 229.0, 95.0, 17.0 ],
					"numoutlets" : 1,
					"fontname" : "Arial",
					"outlettype" : [ "" ],
					"id" : "obj-5"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "panel",
					"bgcolor" : [ 0.807843, 0.807843, 0.807843, 1.0 ],
					"numinlets" : 1,
					"patching_rect" : [ 128.0, 207.0, 402.0, 51.0 ],
					"numoutlets" : 0,
					"rounded" : 0,
					"id" : "obj-6"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "comment",
					"text" : "Recording On\/Off",
					"fontsize" : 7.9,
					"numinlets" : 1,
					"patching_rect" : [ 150.0, 263.0, 102.0, 17.0 ],
					"numoutlets" : 0,
					"fontname" : "Arial",
					"frgb" : [ 0.658824, 0.658824, 0.658824, 1.0 ],
					"id" : "obj-7"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "toggle",
					"numinlets" : 1,
					"patching_rect" : [ 132.0, 262.0, 15.0, 15.0 ],
					"numoutlets" : 1,
					"outlettype" : [ "int" ],
					"id" : "obj-8"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "number",
					"htextcolor" : [ 0.870588, 0.870588, 0.870588, 1.0 ],
					"bgcolor" : [ 0.866667, 0.866667, 0.866667, 1.0 ],
					"fontsize" : 7.9,
					"numinlets" : 1,
					"patching_rect" : [ 252.0, 269.0, 35.0, 17.0 ],
					"numoutlets" : 2,
					"fontname" : "Arial",
					"outlettype" : [ "int", "bang" ],
					"id" : "obj-9",
					"triscale" : 0.9,
					"textcolor" : [ 0.0, 0.0, 0.0, 1.0 ]
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "comment",
					"text" : "polling time for filling the waveform",
					"fontsize" : 7.9,
					"numinlets" : 1,
					"patching_rect" : [ 289.0, 270.0, 183.0, 17.0 ],
					"numoutlets" : 0,
					"fontname" : "Arial",
					"frgb" : [ 0.658824, 0.658824, 0.658824, 1.0 ],
					"id" : "obj-10"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "message",
					"text" : "brgb 33 220 100 255",
					"fontsize" : 7.9,
					"numinlets" : 2,
					"patching_rect" : [ 143.0, 187.0, 105.0, 15.0 ],
					"numoutlets" : 1,
					"fontname" : "Arial",
					"outlettype" : [ "" ],
					"id" : "obj-11"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "message",
					"text" : "brgb 128 140 190 255",
					"fontsize" : 7.9,
					"numinlets" : 2,
					"patching_rect" : [ 131.0, 171.0, 113.0, 15.0 ],
					"numoutlets" : 1,
					"fontname" : "Arial",
					"outlettype" : [ "" ],
					"id" : "obj-12"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "comment",
					"text" : "Set background color (only replaced while recording)",
					"linecount" : 2,
					"fontsize" : 7.9,
					"numinlets" : 1,
					"patching_rect" : [ 257.0, 174.0, 151.0, 28.0 ],
					"numoutlets" : 0,
					"fontname" : "Arial",
					"frgb" : [ 0.658824, 0.658824, 0.658824, 1.0 ],
					"id" : "obj-13"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "panel",
					"bgcolor" : [ 0.807843, 0.807843, 0.807843, 1.0 ],
					"numinlets" : 1,
					"patching_rect" : [ 128.0, 169.0, 402.0, 36.0 ],
					"numoutlets" : 0,
					"rounded" : 0,
					"id" : "obj-14"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "comment",
					"text" : "Set buffer size to correspond with the length of the buffer being recorded into e.g. bozo",
					"linecount" : 2,
					"fontsize" : 7.9,
					"numinlets" : 1,
					"patching_rect" : [ 214.0, 135.0, 212.0, 28.0 ],
					"numoutlets" : 0,
					"fontname" : "Arial",
					"frgb" : [ 0.658824, 0.658824, 0.658824, 1.0 ],
					"id" : "obj-15"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "flonum",
					"htextcolor" : [ 0.870588, 0.870588, 0.870588, 1.0 ],
					"bgcolor" : [ 0.866667, 0.866667, 0.866667, 1.0 ],
					"fontsize" : 7.9,
					"numinlets" : 1,
					"patching_rect" : [ 131.0, 133.0, 53.0, 17.0 ],
					"numoutlets" : 2,
					"fontname" : "Arial",
					"outlettype" : [ "float", "bang" ],
					"id" : "obj-16",
					"triscale" : 0.9,
					"textcolor" : [ 0.0, 0.0, 0.0, 1.0 ]
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "message",
					"text" : "buffer_size $1",
					"fontsize" : 7.9,
					"numinlets" : 2,
					"patching_rect" : [ 131.0, 151.0, 75.0, 15.0 ],
					"numoutlets" : 1,
					"fontname" : "Arial",
					"outlettype" : [ "" ],
					"id" : "obj-17"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "panel",
					"bgcolor" : [ 0.807843, 0.807843, 0.807843, 1.0 ],
					"numinlets" : 1,
					"patching_rect" : [ 128.0, 131.0, 402.0, 36.0 ],
					"numoutlets" : 0,
					"rounded" : 0,
					"id" : "obj-18"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "message",
					"text" : "1",
					"fontsize" : 7.9,
					"numinlets" : 2,
					"patching_rect" : [ 418.0, 94.0, 16.0, 15.0 ],
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
					"text" : "!- 0.",
					"fontsize" : 7.9,
					"numinlets" : 2,
					"patching_rect" : [ 304.0, 576.0, 29.0, 17.0 ],
					"numoutlets" : 1,
					"fontname" : "Arial",
					"outlettype" : [ "float" ],
					"id" : "obj-20"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "message",
					"text" : "compile",
					"fontsize" : 7.9,
					"numinlets" : 2,
					"patching_rect" : [ 29.0, 395.0, 43.0, 15.0 ],
					"numoutlets" : 1,
					"fontname" : "Arial",
					"outlettype" : [ "" ],
					"id" : "obj-21"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "message",
					"text" : "set bozo",
					"fontsize" : 7.9,
					"numinlets" : 2,
					"patching_rect" : [ 4.0, 550.0, 45.0, 15.0 ],
					"numoutlets" : 1,
					"fontname" : "Arial",
					"outlettype" : [ "" ],
					"hidden" : 1,
					"id" : "obj-22"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "newobj",
					"text" : "unpack 0. 0.",
					"fontsize" : 7.9,
					"numinlets" : 1,
					"patching_rect" : [ 270.0, 555.0, 63.0, 17.0 ],
					"numoutlets" : 2,
					"fontname" : "Arial",
					"outlettype" : [ "float", "float" ],
					"id" : "obj-23"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "waveform~",
					"selectioncolor" : [ 0.0, 0.372549, 1.0, 0.5 ],
					"tickmarkcolor" : [ 0.392157, 0.392157, 0.392157, 1.0 ],
					"bgcolor" : [ 0.235294, 0.698039, 0.678431, 1.0 ],
					"setmode" : 1,
					"buffername" : "bozo",
					"numinlets" : 5,
					"patching_rect" : [ 270.0, 598.0, 262.0, 44.0 ],
					"numoutlets" : 6,
					"outlettype" : [ "float", "float", "float", "float", "list", "" ],
					"waveformcolor" : [ 0.129412, 0.0, 0.0, 1.0 ],
					"labelbgcolor" : [ 0.745098, 0.537255, 1.0, 1.0 ],
					"id" : "obj-24",
					"textcolor" : [  ]
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "comment",
					"text" : "Display the green region",
					"fontsize" : 7.9,
					"numinlets" : 1,
					"patching_rect" : [ 343.0, 576.0, 133.0, 17.0 ],
					"numoutlets" : 0,
					"fontname" : "Arial",
					"frgb" : [ 0.658824, 0.658824, 0.658824, 1.0 ],
					"id" : "obj-25"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "newobj",
					"text" : "unpack 0. 0.",
					"fontsize" : 7.9,
					"numinlets" : 1,
					"patching_rect" : [ 5.0, 572.0, 63.0, 17.0 ],
					"numoutlets" : 2,
					"fontname" : "Arial",
					"outlettype" : [ "float", "float" ],
					"id" : "obj-26"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "waveform~",
					"selectioncolor" : [ 0.0, 0.372549, 1.0, 0.5 ],
					"tickmarkcolor" : [ 0.392157, 0.392157, 0.392157, 1.0 ],
					"bgcolor" : [ 0.235294, 0.698039, 0.678431, 1.0 ],
					"setmode" : 1,
					"buffername" : "bozo",
					"numinlets" : 5,
					"patching_rect" : [ 5.0, 598.0, 262.0, 44.0 ],
					"numoutlets" : 6,
					"outlettype" : [ "float", "float", "float", "float", "list", "" ],
					"waveformcolor" : [ 0.129412, 0.0, 0.0, 1.0 ],
					"labelbgcolor" : [ 0.745098, 0.537255, 1.0, 1.0 ],
					"id" : "obj-27",
					"textcolor" : [  ]
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "message",
					"text" : "loop 1, 1",
					"fontsize" : 7.9,
					"numinlets" : 2,
					"patching_rect" : [ 440.0, 83.0, 49.0, 15.0 ],
					"numoutlets" : 1,
					"fontname" : "Arial",
					"outlettype" : [ "" ],
					"hidden" : 1,
					"id" : "obj-28"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "newobj",
					"text" : "adc~",
					"fontsize" : 7.9,
					"numinlets" : 1,
					"patching_rect" : [ 181.0, 277.0, 30.0, 17.0 ],
					"numoutlets" : 2,
					"fontname" : "Arial",
					"outlettype" : [ "signal", "signal" ],
					"id" : "obj-29"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "newobj",
					"text" : "p FillWaveform",
					"fontsize" : 7.9,
					"numinlets" : 3,
					"patching_rect" : [ 132.0, 301.0, 107.0, 17.0 ],
					"numoutlets" : 1,
					"fontname" : "Arial",
					"outlettype" : [ "" ],
					"id" : "obj-30",
					"patcher" : 					{
						"rect" : [ 587.0, 217.0, 388.0, 415.0 ],
						"bgcolor" : [ 0.1, 0.1, 0.1, 1.0 ],
						"bglocked" : 0,
						"defrect" : [ 587.0, 217.0, 388.0, 415.0 ],
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
						"boxes" : [ 							{
								"box" : 								{
									"maxclass" : "newobj",
									"text" : "qlim 1",
									"fontsize" : 7.9,
									"numinlets" : 2,
									"patching_rect" : [ 51.0, 178.0, 35.0, 17.0 ],
									"numoutlets" : 1,
									"fontname" : "Arial",
									"outlettype" : [ "" ],
									"id" : "obj-1"
								}

							}
, 							{
								"box" : 								{
									"maxclass" : "message",
									"text" : "resetScroll",
									"fontsize" : 7.9,
									"numinlets" : 2,
									"patching_rect" : [ 3.0, 303.0, 60.0, 15.0 ],
									"numoutlets" : 1,
									"fontname" : "Arial",
									"outlettype" : [ "" ],
									"id" : "obj-2"
								}

							}
, 							{
								"box" : 								{
									"maxclass" : "newobj",
									"text" : "sel 0",
									"fontsize" : 7.9,
									"numinlets" : 2,
									"patching_rect" : [ 3.0, 92.0, 31.0, 17.0 ],
									"numoutlets" : 2,
									"fontname" : "Arial",
									"outlettype" : [ "bang", "" ],
									"id" : "obj-3"
								}

							}
, 							{
								"box" : 								{
									"maxclass" : "newobj",
									"text" : "gate",
									"fontsize" : 7.9,
									"numinlets" : 2,
									"patching_rect" : [ 51.0, 135.0, 27.0, 17.0 ],
									"numoutlets" : 1,
									"fontname" : "Arial",
									"outlettype" : [ "" ],
									"id" : "obj-4"
								}

							}
, 							{
								"box" : 								{
									"maxclass" : "inlet",
									"numinlets" : 0,
									"patching_rect" : [ 77.0, 44.0, 15.0, 15.0 ],
									"numoutlets" : 1,
									"outlettype" : [ "" ],
									"id" : "obj-5",
									"comment" : "record on\/off"
								}

							}
, 							{
								"box" : 								{
									"maxclass" : "newobj",
									"text" : "record~ bozo",
									"fontsize" : 7.9,
									"numinlets" : 3,
									"patching_rect" : [ 77.0, 85.0, 68.0, 17.0 ],
									"numoutlets" : 1,
									"fontname" : "Arial",
									"outlettype" : [ "signal" ],
									"id" : "obj-6"
								}

							}
, 							{
								"box" : 								{
									"maxclass" : "inlet",
									"numinlets" : 0,
									"patching_rect" : [ 282.0, 43.0, 15.0, 15.0 ],
									"numoutlets" : 1,
									"outlettype" : [ "int" ],
									"id" : "obj-7",
									"comment" : "polling time"
								}

							}
, 							{
								"box" : 								{
									"maxclass" : "number",
									"htextcolor" : [ 0.870588, 0.870588, 0.870588, 1.0 ],
									"bgcolor" : [ 0.866667, 0.866667, 0.866667, 1.0 ],
									"fontsize" : 7.9,
									"numinlets" : 1,
									"patching_rect" : [ 282.0, 66.0, 35.0, 17.0 ],
									"numoutlets" : 2,
									"fontname" : "Arial",
									"outlettype" : [ "int", "bang" ],
									"id" : "obj-8",
									"triscale" : 0.9,
									"textcolor" : [ 0.0, 0.0, 0.0, 1.0 ]
								}

							}
, 							{
								"box" : 								{
									"maxclass" : "newobj",
									"text" : "sel 10",
									"fontsize" : 7.9,
									"numinlets" : 2,
									"patching_rect" : [ 11.0, 222.0, 35.0, 17.0 ],
									"numoutlets" : 2,
									"fontname" : "Arial",
									"outlettype" : [ "bang", "" ],
									"id" : "obj-9"
								}

							}
, 							{
								"box" : 								{
									"maxclass" : "newobj",
									"text" : "snapshot~ 20",
									"fontsize" : 7.9,
									"numinlets" : 2,
									"patching_rect" : [ 77.0, 110.0, 69.0, 17.0 ],
									"numoutlets" : 1,
									"fontname" : "Arial",
									"outlettype" : [ "float" ],
									"id" : "obj-10"
								}

							}
, 							{
								"box" : 								{
									"maxclass" : "newobj",
									"text" : "snapshot~ 20",
									"fontsize" : 7.9,
									"numinlets" : 2,
									"patching_rect" : [ 155.0, 113.0, 69.0, 17.0 ],
									"numoutlets" : 1,
									"fontname" : "Arial",
									"outlettype" : [ "float" ],
									"id" : "obj-11"
								}

							}
, 							{
								"box" : 								{
									"maxclass" : "newobj",
									"text" : "average~ 100",
									"fontsize" : 7.9,
									"numinlets" : 1,
									"patching_rect" : [ 155.0, 75.0, 72.0, 17.0 ],
									"numoutlets" : 1,
									"fontname" : "Arial",
									"outlettype" : [ "signal" ],
									"id" : "obj-12"
								}

							}
, 							{
								"box" : 								{
									"maxclass" : "newobj",
									"text" : "b 1",
									"fontsize" : 7.9,
									"numinlets" : 1,
									"patching_rect" : [ 201.0, 264.0, 22.0, 17.0 ],
									"numoutlets" : 1,
									"fontname" : "Arial",
									"outlettype" : [ "bang" ],
									"id" : "obj-13"
								}

							}
, 							{
								"box" : 								{
									"maxclass" : "newobj",
									"text" : "counter 0 10",
									"fontsize" : 7.9,
									"numinlets" : 5,
									"patching_rect" : [ 51.0, 199.0, 66.0, 17.0 ],
									"numoutlets" : 4,
									"fontname" : "Arial",
									"outlettype" : [ "int", "", "", "int" ],
									"id" : "obj-14",
									"save" : [ "#N", "counter", 0, 10, ";", "#X", "flags", 0, 0, ";" ]
								}

							}
, 							{
								"box" : 								{
									"maxclass" : "newobj",
									"text" : "b 1",
									"fontsize" : 7.9,
									"numinlets" : 1,
									"patching_rect" : [ 51.0, 158.0, 22.0, 17.0 ],
									"numoutlets" : 1,
									"fontname" : "Arial",
									"outlettype" : [ "bang" ],
									"id" : "obj-15"
								}

							}
, 							{
								"box" : 								{
									"maxclass" : "message",
									"text" : "clear",
									"fontsize" : 7.9,
									"numinlets" : 2,
									"patching_rect" : [ 157.0, 288.0, 32.0, 15.0 ],
									"numoutlets" : 1,
									"fontname" : "Arial",
									"outlettype" : [ "" ],
									"id" : "obj-16"
								}

							}
, 							{
								"box" : 								{
									"maxclass" : "message",
									"text" : "dump",
									"fontsize" : 7.9,
									"numinlets" : 2,
									"patching_rect" : [ 201.0, 289.0, 32.0, 15.0 ],
									"numoutlets" : 1,
									"fontname" : "Arial",
									"outlettype" : [ "" ],
									"id" : "obj-17"
								}

							}
, 							{
								"box" : 								{
									"maxclass" : "newobj",
									"text" : "coll stuff 1",
									"fontsize" : 7.9,
									"numinlets" : 1,
									"patching_rect" : [ 143.0, 314.0, 59.0, 17.0 ],
									"numoutlets" : 4,
									"fontname" : "Arial",
									"outlettype" : [ "", "", "", "" ],
									"id" : "obj-18",
									"save" : [ "#N", "coll", "stuff", 1, ";" ],
									"saved_object_attributes" : 									{
										"embed" : 0
									}

								}

							}
, 							{
								"box" : 								{
									"maxclass" : "newobj",
									"text" : "loadbang",
									"fontsize" : 7.9,
									"numinlets" : 1,
									"patching_rect" : [ 213.0, 28.0, 46.0, 17.0 ],
									"numoutlets" : 1,
									"fontname" : "Arial",
									"outlettype" : [ "bang" ],
									"id" : "obj-19"
								}

							}
, 							{
								"box" : 								{
									"maxclass" : "message",
									"text" : "absolute",
									"fontsize" : 7.9,
									"numinlets" : 2,
									"patching_rect" : [ 213.0, 50.0, 45.0, 15.0 ],
									"numoutlets" : 1,
									"fontname" : "Arial",
									"outlettype" : [ "" ],
									"id" : "obj-20"
								}

							}
, 							{
								"box" : 								{
									"maxclass" : "outlet",
									"numinlets" : 1,
									"patching_rect" : [ 143.0, 354.0, 15.0, 15.0 ],
									"numoutlets" : 0,
									"id" : "obj-21",
									"comment" : ""
								}

							}
, 							{
								"box" : 								{
									"maxclass" : "newobj",
									"text" : "pack 1 0. 0.",
									"fontsize" : 7.9,
									"numinlets" : 3,
									"patching_rect" : [ 51.0, 223.0, 62.0, 17.0 ],
									"numoutlets" : 1,
									"fontname" : "Arial",
									"outlettype" : [ "" ],
									"id" : "obj-22"
								}

							}
, 							{
								"box" : 								{
									"maxclass" : "inlet",
									"numinlets" : 0,
									"patching_rect" : [ 155.0, 46.0, 15.0, 15.0 ],
									"numoutlets" : 1,
									"outlettype" : [ "signal" ],
									"id" : "obj-23",
									"comment" : "Audio Signal"
								}

							}
, 							{
								"box" : 								{
									"maxclass" : "comment",
									"text" : "where[0:1], how high [0:1]",
									"fontsize" : 7.9,
									"numinlets" : 1,
									"patching_rect" : [ 125.0, 226.0, 132.0, 17.0 ],
									"numoutlets" : 0,
									"fontname" : "Arial",
									"frgb" : [ 0.658824, 0.658824, 0.658824, 1.0 ],
									"id" : "obj-24"
								}

							}
 ],
						"lines" : [ 							{
								"patchline" : 								{
									"source" : [ "obj-7", 0 ],
									"destination" : [ "obj-8", 0 ],
									"hidden" : 0
								}

							}
, 							{
								"patchline" : 								{
									"source" : [ "obj-8", 0 ],
									"destination" : [ "obj-11", 1 ],
									"hidden" : 0
								}

							}
, 							{
								"patchline" : 								{
									"source" : [ "obj-19", 0 ],
									"destination" : [ "obj-20", 0 ],
									"hidden" : 0
								}

							}
, 							{
								"patchline" : 								{
									"source" : [ "obj-13", 0 ],
									"destination" : [ "obj-17", 0 ],
									"hidden" : 0
								}

							}
, 							{
								"patchline" : 								{
									"source" : [ "obj-9", 0 ],
									"destination" : [ "obj-13", 0 ],
									"hidden" : 0
								}

							}
, 							{
								"patchline" : 								{
									"source" : [ "obj-3", 0 ],
									"destination" : [ "obj-16", 0 ],
									"hidden" : 0
								}

							}
, 							{
								"patchline" : 								{
									"source" : [ "obj-18", 3 ],
									"destination" : [ "obj-16", 0 ],
									"hidden" : 0,
									"midpoints" : [ 192.5, 341.0, 304.0, 341.0, 304.0, 255.0, 166.5, 255.0 ]
								}

							}
, 							{
								"patchline" : 								{
									"source" : [ "obj-12", 0 ],
									"destination" : [ "obj-11", 0 ],
									"hidden" : 0
								}

							}
, 							{
								"patchline" : 								{
									"source" : [ "obj-20", 0 ],
									"destination" : [ "obj-12", 0 ],
									"hidden" : 0
								}

							}
, 							{
								"patchline" : 								{
									"source" : [ "obj-23", 0 ],
									"destination" : [ "obj-12", 0 ],
									"hidden" : 0
								}

							}
, 							{
								"patchline" : 								{
									"source" : [ "obj-18", 0 ],
									"destination" : [ "obj-21", 0 ],
									"hidden" : 0
								}

							}
, 							{
								"patchline" : 								{
									"source" : [ "obj-2", 0 ],
									"destination" : [ "obj-21", 0 ],
									"hidden" : 0
								}

							}
, 							{
								"patchline" : 								{
									"source" : [ "obj-22", 0 ],
									"destination" : [ "obj-18", 0 ],
									"hidden" : 0
								}

							}
, 							{
								"patchline" : 								{
									"source" : [ "obj-16", 0 ],
									"destination" : [ "obj-18", 0 ],
									"hidden" : 0
								}

							}
, 							{
								"patchline" : 								{
									"source" : [ "obj-17", 0 ],
									"destination" : [ "obj-18", 0 ],
									"hidden" : 0
								}

							}
, 							{
								"patchline" : 								{
									"source" : [ "obj-8", 0 ],
									"destination" : [ "obj-10", 1 ],
									"hidden" : 0
								}

							}
, 							{
								"patchline" : 								{
									"source" : [ "obj-11", 0 ],
									"destination" : [ "obj-22", 2 ],
									"hidden" : 0
								}

							}
, 							{
								"patchline" : 								{
									"source" : [ "obj-10", 0 ],
									"destination" : [ "obj-22", 1 ],
									"hidden" : 0
								}

							}
, 							{
								"patchline" : 								{
									"source" : [ "obj-6", 0 ],
									"destination" : [ "obj-10", 0 ],
									"hidden" : 0
								}

							}
, 							{
								"patchline" : 								{
									"source" : [ "obj-5", 0 ],
									"destination" : [ "obj-6", 0 ],
									"hidden" : 0
								}

							}
, 							{
								"patchline" : 								{
									"source" : [ "obj-23", 0 ],
									"destination" : [ "obj-6", 0 ],
									"hidden" : 0
								}

							}
, 							{
								"patchline" : 								{
									"source" : [ "obj-10", 0 ],
									"destination" : [ "obj-4", 1 ],
									"hidden" : 0
								}

							}
, 							{
								"patchline" : 								{
									"source" : [ "obj-14", 0 ],
									"destination" : [ "obj-22", 0 ],
									"hidden" : 0
								}

							}
, 							{
								"patchline" : 								{
									"source" : [ "obj-1", 0 ],
									"destination" : [ "obj-14", 0 ],
									"hidden" : 0
								}

							}
, 							{
								"patchline" : 								{
									"source" : [ "obj-15", 0 ],
									"destination" : [ "obj-1", 0 ],
									"hidden" : 0
								}

							}
, 							{
								"patchline" : 								{
									"source" : [ "obj-4", 0 ],
									"destination" : [ "obj-15", 0 ],
									"hidden" : 0
								}

							}
, 							{
								"patchline" : 								{
									"source" : [ "obj-5", 0 ],
									"destination" : [ "obj-4", 0 ],
									"hidden" : 0
								}

							}
, 							{
								"patchline" : 								{
									"source" : [ "obj-14", 0 ],
									"destination" : [ "obj-9", 0 ],
									"hidden" : 0
								}

							}
, 							{
								"patchline" : 								{
									"source" : [ "obj-3", 0 ],
									"destination" : [ "obj-2", 0 ],
									"hidden" : 0
								}

							}
, 							{
								"patchline" : 								{
									"source" : [ "obj-5", 0 ],
									"destination" : [ "obj-3", 0 ],
									"hidden" : 0
								}

							}
 ]
					}
,
					"saved_object_attributes" : 					{
						"default_fontsize" : 9.0,
						"fontsize" : 9.0,
						"globalpatchername" : "",
						"default_fontname" : "Verdana",
						"fontname" : "Verdana",
						"default_fontface" : 0,
						"fontface" : 0
					}

				}

			}
, 			{
				"box" : 				{
					"maxclass" : "newobj",
					"text" : "print E",
					"fontsize" : 7.9,
					"numinlets" : 1,
					"patching_rect" : [ 317.0, 527.0, 39.0, 17.0 ],
					"numoutlets" : 0,
					"fontname" : "Arial",
					"id" : "obj-31"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "newobj",
					"text" : "print D",
					"fontsize" : 7.9,
					"numinlets" : 1,
					"patching_rect" : [ 239.0, 527.0, 40.0, 17.0 ],
					"numoutlets" : 0,
					"fontname" : "Arial",
					"id" : "obj-32"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "newobj",
					"text" : "print C",
					"fontsize" : 7.9,
					"numinlets" : 1,
					"patching_rect" : [ 161.0, 527.0, 40.0, 17.0 ],
					"numoutlets" : 0,
					"fontname" : "Arial",
					"id" : "obj-33"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "newobj",
					"text" : "print B",
					"fontsize" : 7.9,
					"numinlets" : 1,
					"patching_rect" : [ 83.0, 527.0, 40.0, 17.0 ],
					"numoutlets" : 0,
					"fontname" : "Arial",
					"id" : "obj-34"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "newobj",
					"text" : "print A",
					"fontsize" : 7.9,
					"numinlets" : 1,
					"patching_rect" : [ 5.0, 527.0, 41.0, 17.0 ],
					"numoutlets" : 0,
					"fontname" : "Arial",
					"id" : "obj-35"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "jsui",
					"numinlets" : 6,
					"patching_rect" : [ 5.0, 421.0, 325.0, 100.0 ],
					"numoutlets" : 5,
					"outlettype" : [ "", "", "", "", "" ],
					"id" : "obj-36",
					"jsarguments" : [ 5, 5000.0 ],
					"filename" : "tap.waveformui.js"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "comment",
					"text" : "jsui for selecting up to 5 regions of a buffer",
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
					"text" : "tap.waveformui",
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
					"text" : "10",
					"fontsize" : 7.9,
					"numinlets" : 2,
					"patching_rect" : [ 380.0, 97.0, 20.0, 15.0 ],
					"numoutlets" : 1,
					"fontname" : "Arial",
					"outlettype" : [ "" ],
					"hidden" : 1,
					"id" : "obj-39"
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
					"id" : "obj-40"
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
					"id" : "obj-41"
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
					"id" : "obj-42"
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
					"id" : "obj-43"
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
					"id" : "obj-44",
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
					"id" : "obj-45"
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
					"id" : "obj-46"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "newobj",
					"text" : "buffer~ bozo 5000",
					"fontsize" : 7.9,
					"numinlets" : 1,
					"patching_rect" : [ 18.0, 150.0, 93.0, 17.0 ],
					"numoutlets" : 2,
					"fontname" : "Arial",
					"outlettype" : [ "float", "bang" ],
					"id" : "obj-47"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "ezdac~",
					"numinlets" : 2,
					"patching_rect" : [ 40.0, 204.0, 33.0, 33.0 ],
					"numoutlets" : 0,
					"id" : "obj-48"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "comment",
					"text" : "To use, click the appropriately colored box in the upper-left corner of the interface. Then click in the interface and drag to change the selected region",
					"linecount" : 4,
					"fontsize" : 7.9,
					"numinlets" : 1,
					"patching_rect" : [ 339.0, 426.0, 188.0, 49.0 ],
					"numoutlets" : 0,
					"fontname" : "Arial",
					"frgb" : [ 0.658824, 0.658824, 0.658824, 1.0 ],
					"id" : "obj-49"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "comment",
					"text" : "Display the orange selection",
					"fontsize" : 7.9,
					"numinlets" : 1,
					"patching_rect" : [ 73.0, 572.0, 133.0, 17.0 ],
					"numoutlets" : 0,
					"fontname" : "Arial",
					"frgb" : [ 0.658824, 0.658824, 0.658824, 1.0 ],
					"id" : "obj-50"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "comment",
					"text" : "2 arguments: 1st = number of regions (it is set to support 5, although you can adjust the js file to add more), 2nd = buffer length [default 1000. ms]",
					"linecount" : 7,
					"fontsize" : 7.9,
					"numinlets" : 1,
					"patching_rect" : [ 7.0, 304.0, 114.0, 80.0 ],
					"numoutlets" : 0,
					"fontname" : "Arial",
					"frgb" : [ 0.658824, 0.658824, 0.658824, 1.0 ],
					"id" : "obj-51"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "comment",
					"text" : "Shift-Click for higher resolution",
					"fontsize" : 7.9,
					"numinlets" : 1,
					"patching_rect" : [ 342.0, 497.0, 203.0, 17.0 ],
					"numoutlets" : 0,
					"fontname" : "Arial",
					"frgb" : [ 0.658824, 0.658824, 0.658824, 1.0 ],
					"id" : "obj-52"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "comment",
					"text" : "The patcher FillWaveform averages the amplitude of the audio that is recorded into buffer~ bozo. It then places it into a coll object paired with its position in the window. Every 10th value, the list is dumped into tap.waveformui and is drawn in the window. The waveform that shows up corresponds to the audio in buffer~bozo. Now you can use the 5 colored selection regions to define regions of audio in the buffer. When you change the region, the beginning and end points are sent out the appropriate outlet for use in waveform~, play~, groove~, etc.",
					"linecount" : 7,
					"fontsize" : 7.9,
					"numinlets" : 1,
					"patching_rect" : [ 132.0, 322.0, 381.0, 80.0 ],
					"numoutlets" : 0,
					"fontname" : "Arial",
					"frgb" : [ 0.658824, 0.658824, 0.658824, 1.0 ],
					"id" : "obj-53"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "panel",
					"bgcolor" : [ 0.741176, 0.333333, 0.035294, 1.0 ],
					"numinlets" : 1,
					"patching_rect" : [ 2.0, 569.0, 267.0, 77.0 ],
					"numoutlets" : 0,
					"rounded" : 0,
					"id" : "obj-54",
					"border" : 1
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "panel",
					"bgcolor" : [ 0.509804, 0.980392, 0.627451, 1.0 ],
					"numinlets" : 1,
					"patching_rect" : [ 267.0, 569.0, 267.0, 77.0 ],
					"numoutlets" : 0,
					"rounded" : 0,
					"id" : "obj-55",
					"border" : 1
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "comment",
					"text" : "Turn on to record audio into the buffer",
					"linecount" : 2,
					"fontsize" : 7.9,
					"numinlets" : 1,
					"patching_rect" : [ 17.0, 174.0, 96.0, 28.0 ],
					"numoutlets" : 0,
					"fontname" : "Arial",
					"frgb" : [ 0.658824, 0.658824, 0.658824, 1.0 ],
					"id" : "obj-56"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "panel",
					"bgcolor" : [ 0.807843, 0.807843, 0.807843, 1.0 ],
					"numinlets" : 1,
					"patching_rect" : [ 334.0, 421.0, 196.0, 137.0 ],
					"numoutlets" : 0,
					"rounded" : 0,
					"id" : "obj-57"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "panel",
					"bgcolor" : [ 0.807843, 0.807843, 0.807843, 1.0 ],
					"numinlets" : 1,
					"patching_rect" : [ 128.0, 260.0, 402.0, 36.0 ],
					"numoutlets" : 0,
					"rounded" : 0,
					"id" : "obj-58"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "panel",
					"bgcolor" : [ 0.807843, 0.807843, 0.807843, 1.0 ],
					"numinlets" : 1,
					"patching_rect" : [ 128.0, 298.0, 402.0, 120.0 ],
					"numoutlets" : 0,
					"rounded" : 0,
					"id" : "obj-59"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "panel",
					"bgcolor" : [ 0.807843, 0.807843, 0.807843, 1.0 ],
					"numinlets" : 1,
					"patching_rect" : [ 0.0, 298.0, 126.0, 120.0 ],
					"numoutlets" : 0,
					"rounded" : 0,
					"id" : "obj-60"
				}

			}
 ],
		"lines" : [ 			{
				"patchline" : 				{
					"source" : [ "obj-3", 0 ],
					"destination" : [ "obj-5", 2 ],
					"hidden" : 0
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-4", 0 ],
					"destination" : [ "obj-5", 1 ],
					"hidden" : 0
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-40", 0 ],
					"destination" : [ "obj-1", 0 ],
					"hidden" : 1
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-40", 0 ],
					"destination" : [ "obj-28", 0 ],
					"hidden" : 1
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-40", 0 ],
					"destination" : [ "obj-19", 0 ],
					"hidden" : 1
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-40", 0 ],
					"destination" : [ "obj-39", 0 ],
					"hidden" : 1
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-20", 0 ],
					"destination" : [ "obj-24", 1 ],
					"hidden" : 0
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-23", 1 ],
					"destination" : [ "obj-20", 1 ],
					"hidden" : 0
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-5", 0 ],
					"destination" : [ "obj-36", 5 ],
					"hidden" : 0,
					"midpoints" : [ 427.5, 250.0, 533.0, 250.0, 533.0, 411.0, 320.5, 411.0 ]
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-36", 4 ],
					"destination" : [ "obj-31", 0 ],
					"hidden" : 0
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-23", 0 ],
					"destination" : [ "obj-20", 0 ],
					"hidden" : 0
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-23", 0 ],
					"destination" : [ "obj-24", 0 ],
					"hidden" : 0
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-22", 0 ],
					"destination" : [ "obj-24", 0 ],
					"hidden" : 1
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-36", 2 ],
					"destination" : [ "obj-23", 0 ],
					"hidden" : 0
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-42", 0 ],
					"destination" : [ "obj-43", 0 ],
					"hidden" : 1
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-44", 0 ],
					"destination" : [ "obj-42", 0 ],
					"hidden" : 1
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-39", 0 ],
					"destination" : [ "obj-9", 0 ],
					"hidden" : 1
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-36", 3 ],
					"destination" : [ "obj-32", 0 ],
					"hidden" : 0
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-9", 0 ],
					"destination" : [ "obj-30", 2 ],
					"hidden" : 0
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-26", 1 ],
					"destination" : [ "obj-27", 3 ],
					"hidden" : 0
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-29", 0 ],
					"destination" : [ "obj-30", 1 ],
					"hidden" : 0
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-36", 2 ],
					"destination" : [ "obj-33", 0 ],
					"hidden" : 0
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-28", 0 ],
					"destination" : [ "obj-30", 0 ],
					"hidden" : 1
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-8", 0 ],
					"destination" : [ "obj-30", 0 ],
					"hidden" : 0
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-19", 0 ],
					"destination" : [ "obj-8", 0 ],
					"hidden" : 1
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-26", 0 ],
					"destination" : [ "obj-27", 2 ],
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
					"source" : [ "obj-1", 0 ],
					"destination" : [ "obj-16", 0 ],
					"hidden" : 1
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-36", 1 ],
					"destination" : [ "obj-34", 0 ],
					"hidden" : 0
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-22", 0 ],
					"destination" : [ "obj-27", 0 ],
					"hidden" : 1
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-36", 0 ],
					"destination" : [ "obj-26", 0 ],
					"hidden" : 0
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-36", 0 ],
					"destination" : [ "obj-35", 0 ],
					"hidden" : 0
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-11", 0 ],
					"destination" : [ "obj-36", 0 ],
					"hidden" : 1
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-12", 0 ],
					"destination" : [ "obj-36", 0 ],
					"hidden" : 1
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-17", 0 ],
					"destination" : [ "obj-36", 0 ],
					"hidden" : 1
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-21", 0 ],
					"destination" : [ "obj-36", 0 ],
					"hidden" : 0
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-30", 0 ],
					"destination" : [ "obj-36", 0 ],
					"hidden" : 0,
					"midpoints" : [ 141.5, 320.0, 130.0, 320.0, 130.0, 413.0, 14.5, 413.0 ]
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-40", 0 ],
					"destination" : [ "obj-22", 0 ],
					"hidden" : 1
				}

			}
 ]
	}

}
