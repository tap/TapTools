{
	"patcher" : 	{
		"rect" : [ 120.0, 44.0, 547.0, 519.0 ],
		"bglocked" : 0,
		"defrect" : [ 120.0, 44.0, 547.0, 519.0 ],
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
					"text" : "basic 12 band vocoder",
					"fontsize" : 10.533333,
					"numinlets" : 1,
					"patching_rect" : [ 133.0, 34.0, 309.0, 21.0 ],
					"numoutlets" : 0,
					"fontname" : "Arial",
					"frgb" : [ 0.87451, 0.87451, 0.87451, 1.0 ],
					"id" : "obj-1",
					"textcolor" : [ 0.87451, 0.87451, 0.87451, 1.0 ]
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "comment",
					"text" : "tap.vocoderlite~",
					"fontsize" : 21.066666,
					"numinlets" : 1,
					"patching_rect" : [ 133.0, 5.0, 309.0, 35.0 ],
					"numoutlets" : 0,
					"fontname" : "Arial",
					"frgb" : [ 0.87451, 0.87451, 0.87451, 1.0 ],
					"id" : "obj-2",
					"textcolor" : [ 0.87451, 0.87451, 0.87451, 1.0 ]
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "message",
					"text" : "set 500",
					"fontsize" : 7.9,
					"numinlets" : 2,
					"patching_rect" : [ 466.0, 97.0, 44.0, 15.0 ],
					"numoutlets" : 1,
					"fontname" : "Arial",
					"outlettype" : [ "" ],
					"hidden" : 1,
					"id" : "obj-3"
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
					"id" : "obj-4"
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
					"id" : "obj-5"
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
					"id" : "obj-6"
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
					"patching_rect" : [ 445.0, 281.0, 49.0, 17.0 ],
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
					"maxclass" : "number",
					"htextcolor" : [ 0.870588, 0.870588, 0.870588, 1.0 ],
					"bgcolor" : [ 0.866667, 0.866667, 0.866667, 1.0 ],
					"fontsize" : 7.9,
					"numinlets" : 1,
					"patching_rect" : [ 193.0, 418.0, 35.0, 17.0 ],
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
					"maxclass" : "toggle",
					"numinlets" : 1,
					"patching_rect" : [ 182.0, 199.0, 15.0, 15.0 ],
					"numoutlets" : 1,
					"outlettype" : [ "int" ],
					"id" : "obj-10"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "message",
					"text" : "open",
					"fontsize" : 7.9,
					"numinlets" : 2,
					"patching_rect" : [ 142.0, 199.0, 39.0, 15.0 ],
					"numoutlets" : 1,
					"fontname" : "Arial",
					"outlettype" : [ "" ],
					"id" : "obj-11"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "flonum",
					"htextcolor" : [ 0.870588, 0.870588, 0.870588, 1.0 ],
					"bgcolor" : [ 0.866667, 0.866667, 0.866667, 1.0 ],
					"fontsize" : 7.9,
					"numinlets" : 1,
					"patching_rect" : [ 377.0, 287.0, 49.0, 17.0 ],
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
					"maxclass" : "comment",
					"text" : "carrier signal",
					"fontsize" : 7.9,
					"numinlets" : 1,
					"patching_rect" : [ 311.0, 285.0, 61.0, 17.0 ],
					"numoutlets" : 0,
					"fontname" : "Arial",
					"frgb" : [ 0.658824, 0.658824, 0.658824, 1.0 ],
					"id" : "obj-13"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "kslider",
					"presentation_rect" : [ 0.0, 0.0, 196.0, 35.0 ],
					"numinlets" : 2,
					"patching_rect" : [ 300.0, 161.0, 196.0, 35.0 ],
					"hkeycolor" : [ 0.501961, 0.501961, 0.501961, 1.0 ],
					"numoutlets" : 2,
					"outlettype" : [ "int", "int" ],
					"id" : "obj-14"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "comment",
					"text" : "Click a message box to play a chord for the carrier signal, or play some keys on the kslider. The 4 most recent notes will be synthesized by the sample-synth",
					"linecount" : 2,
					"fontsize" : 8.777778,
					"numinlets" : 1,
					"patching_rect" : [ 140.0, 132.0, 366.0, 30.0 ],
					"numoutlets" : 0,
					"fontname" : "Arial",
					"frgb" : [ 0.658824, 0.658824, 0.658824, 1.0 ],
					"id" : "obj-15"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "newobj",
					"text" : "p sample-synth",
					"fontsize" : 7.9,
					"numinlets" : 3,
					"color" : [ 1.0, 0.890196, 0.090196, 1.0 ],
					"patching_rect" : [ 303.0, 250.0, 87.0, 17.0 ],
					"numoutlets" : 1,
					"fontname" : "Arial",
					"outlettype" : [ "signal" ],
					"id" : "obj-16",
					"patcher" : 					{
						"rect" : [ 38.0, 147.0, 512.0, 353.0 ],
						"bgcolor" : [ 0.1, 0.1, 0.1, 1.0 ],
						"bglocked" : 0,
						"defrect" : [ 38.0, 147.0, 512.0, 353.0 ],
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
									"text" : "p sample_voice~",
									"fontsize" : 7.9,
									"numinlets" : 1,
									"patching_rect" : [ 272.0, 206.0, 78.0, 17.0 ],
									"numoutlets" : 1,
									"fontname" : "Arial",
									"outlettype" : [ "signal" ],
									"id" : "obj-1",
									"patcher" : 									{
										"rect" : [ 50.0, 40.0, 346.0, 375.0 ],
										"bgcolor" : [ 0.1, 0.1, 0.1, 1.0 ],
										"bglocked" : 0,
										"defrect" : [ 50.0, 40.0, 346.0, 375.0 ],
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
										"boxes" : [ 											{
												"box" : 												{
													"maxclass" : "newobj",
													"text" : "loadbang",
													"fontsize" : 7.9,
													"numinlets" : 1,
													"patching_rect" : [ 176.0, 189.0, 46.0, 17.0 ],
													"numoutlets" : 1,
													"fontname" : "Arial",
													"outlettype" : [ "bang" ],
													"id" : "obj-1"
												}

											}
, 											{
												"box" : 												{
													"maxclass" : "message",
													"text" : "2266.371582",
													"fontsize" : 7.9,
													"numinlets" : 2,
													"patching_rect" : [ 176.0, 225.0, 71.0, 15.0 ],
													"numoutlets" : 1,
													"fontname" : "Arial",
													"outlettype" : [ "" ],
													"id" : "obj-2"
												}

											}
, 											{
												"box" : 												{
													"maxclass" : "message",
													"text" : "0.03",
													"fontsize" : 7.9,
													"numinlets" : 2,
													"patching_rect" : [ 142.0, 225.0, 29.0, 15.0 ],
													"numoutlets" : 1,
													"fontname" : "Arial",
													"outlettype" : [ "" ],
													"id" : "obj-3"
												}

											}
, 											{
												"box" : 												{
													"maxclass" : "message",
													"text" : "loop 1",
													"fontsize" : 7.9,
													"numinlets" : 2,
													"patching_rect" : [ 71.0, 225.0, 35.0, 15.0 ],
													"numoutlets" : 1,
													"fontname" : "Arial",
													"outlettype" : [ "" ],
													"id" : "obj-4"
												}

											}
, 											{
												"box" : 												{
													"maxclass" : "message",
													"text" : "startloop",
													"fontsize" : 7.9,
													"numinlets" : 2,
													"patching_rect" : [ 21.0, 225.0, 49.0, 15.0 ],
													"numoutlets" : 1,
													"fontname" : "Arial",
													"outlettype" : [ "" ],
													"id" : "obj-5"
												}

											}
, 											{
												"box" : 												{
													"maxclass" : "newobj",
													"text" : "sig~ 1",
													"fontsize" : 7.9,
													"numinlets" : 1,
													"patching_rect" : [ 108.0, 188.0, 35.0, 17.0 ],
													"numoutlets" : 1,
													"fontname" : "Arial",
													"outlettype" : [ "signal" ],
													"id" : "obj-6"
												}

											}
, 											{
												"box" : 												{
													"maxclass" : "newobj",
													"text" : "groove~ oscil",
													"fontsize" : 7.9,
													"numinlets" : 3,
													"patching_rect" : [ 108.0, 245.0, 69.0, 17.0 ],
													"numoutlets" : 2,
													"fontname" : "Arial",
													"outlettype" : [ "signal", "signal" ],
													"id" : "obj-7"
												}

											}
, 											{
												"box" : 												{
													"maxclass" : "flonum",
													"htextcolor" : [ 0.870588, 0.870588, 0.870588, 1.0 ],
													"bgcolor" : [ 0.866667, 0.866667, 0.866667, 1.0 ],
													"fontsize" : 7.9,
													"numinlets" : 1,
													"patching_rect" : [ 108.0, 112.0, 52.0, 17.0 ],
													"numoutlets" : 2,
													"fontname" : "Arial",
													"outlettype" : [ "float", "bang" ],
													"id" : "obj-8",
													"triscale" : 0.9,
													"textcolor" : [ 0.0, 0.0, 0.0, 1.0 ]
												}

											}
, 											{
												"box" : 												{
													"maxclass" : "newobj",
													"text" : "mtof",
													"fontsize" : 7.9,
													"numinlets" : 1,
													"patching_rect" : [ 108.0, 89.0, 29.0, 17.0 ],
													"numoutlets" : 1,
													"fontname" : "Arial",
													"outlettype" : [ "float" ],
													"id" : "obj-9"
												}

											}
, 											{
												"box" : 												{
													"maxclass" : "flonum",
													"htextcolor" : [ 0.870588, 0.870588, 0.870588, 1.0 ],
													"bgcolor" : [ 0.866667, 0.866667, 0.866667, 1.0 ],
													"fontsize" : 7.9,
													"numinlets" : 1,
													"patching_rect" : [ 108.0, 159.0, 43.0, 17.0 ],
													"numoutlets" : 2,
													"fontname" : "Arial",
													"outlettype" : [ "float", "bang" ],
													"id" : "obj-10",
													"triscale" : 0.9,
													"textcolor" : [ 0.0, 0.0, 0.0, 1.0 ]
												}

											}
, 											{
												"box" : 												{
													"maxclass" : "flonum",
													"htextcolor" : [ 0.870588, 0.870588, 0.870588, 1.0 ],
													"bgcolor" : [ 0.866667, 0.866667, 0.866667, 1.0 ],
													"fontsize" : 7.9,
													"numinlets" : 1,
													"patching_rect" : [ 108.0, 65.0, 47.0, 17.0 ],
													"numoutlets" : 2,
													"fontname" : "Arial",
													"outlettype" : [ "float", "bang" ],
													"id" : "obj-11",
													"triscale" : 0.9,
													"textcolor" : [ 0.0, 0.0, 0.0, 1.0 ]
												}

											}
, 											{
												"box" : 												{
													"maxclass" : "comment",
													"text" : "note #",
													"fontsize" : 8.777778,
													"numinlets" : 1,
													"patching_rect" : [ 125.0, 42.0, 35.0, 18.0 ],
													"numoutlets" : 0,
													"fontname" : "Times New Roman",
													"frgb" : [ 0.658824, 0.658824, 0.658824, 1.0 ],
													"id" : "obj-12"
												}

											}
, 											{
												"box" : 												{
													"maxclass" : "outlet",
													"numinlets" : 1,
													"patching_rect" : [ 108.0, 272.0, 15.0, 15.0 ],
													"numoutlets" : 0,
													"id" : "obj-13",
													"comment" : "(signal) Left Output"
												}

											}
, 											{
												"box" : 												{
													"maxclass" : "inlet",
													"numinlets" : 0,
													"patching_rect" : [ 108.0, 42.0, 15.0, 15.0 ],
													"numoutlets" : 1,
													"outlettype" : [ "float" ],
													"id" : "obj-14",
													"comment" : "(int) note number"
												}

											}
, 											{
												"box" : 												{
													"maxclass" : "newobj",
													"text" : "\/ 261.625",
													"fontsize" : 8.777778,
													"numinlets" : 2,
													"patching_rect" : [ 108.0, 134.0, 72.0, 18.0 ],
													"numoutlets" : 1,
													"fontname" : "Courier",
													"outlettype" : [ "float" ],
													"id" : "obj-15"
												}

											}
, 											{
												"box" : 												{
													"maxclass" : "comment",
													"text" : "base frequency of sample (middle C)",
													"linecount" : 2,
													"fontsize" : 7.9,
													"numinlets" : 1,
													"patching_rect" : [ 171.0, 131.0, 87.0, 28.0 ],
													"numoutlets" : 0,
													"fontname" : "Arial",
													"frgb" : [ 0.658824, 0.658824, 0.658824, 1.0 ],
													"id" : "obj-16"
												}

											}
, 											{
												"box" : 												{
													"maxclass" : "comment",
													"text" : "Desired MIDI note # for sample",
													"linecount" : 2,
													"fontsize" : 7.9,
													"numinlets" : 1,
													"patching_rect" : [ 157.0, 63.0, 86.0, 28.0 ],
													"numoutlets" : 0,
													"fontname" : "Arial",
													"frgb" : [ 0.658824, 0.658824, 0.658824, 1.0 ],
													"id" : "obj-17"
												}

											}
, 											{
												"box" : 												{
													"maxclass" : "comment",
													"text" : "Ratio for playback speed",
													"fontsize" : 7.9,
													"numinlets" : 1,
													"patching_rect" : [ 152.0, 161.0, 115.0, 17.0 ],
													"numoutlets" : 0,
													"fontname" : "Arial",
													"frgb" : [ 0.658824, 0.658824, 0.658824, 1.0 ],
													"id" : "obj-18"
												}

											}
, 											{
												"box" : 												{
													"maxclass" : "comment",
													"text" : "Left Output",
													"fontsize" : 7.9,
													"numinlets" : 1,
													"patching_rect" : [ 54.0, 273.0, 57.0, 17.0 ],
													"numoutlets" : 0,
													"fontname" : "Arial",
													"frgb" : [ 0.658824, 0.658824, 0.658824, 1.0 ],
													"id" : "obj-19"
												}

											}
, 											{
												"box" : 												{
													"maxclass" : "comment",
													"text" : "MIDI to Frequency Conversion",
													"fontsize" : 7.9,
													"numinlets" : 1,
													"patching_rect" : [ 138.0, 91.0, 145.0, 17.0 ],
													"numoutlets" : 0,
													"fontname" : "Arial",
													"frgb" : [ 0.658824, 0.658824, 0.658824, 1.0 ],
													"id" : "obj-20"
												}

											}
, 											{
												"box" : 												{
													"maxclass" : "comment",
													"text" : "Desired frequency of sample",
													"fontsize" : 7.9,
													"numinlets" : 1,
													"patching_rect" : [ 162.0, 112.0, 134.0, 17.0 ],
													"numoutlets" : 0,
													"fontname" : "Arial",
													"frgb" : [ 0.658824, 0.658824, 0.658824, 1.0 ],
													"id" : "obj-21"
												}

											}
 ],
										"lines" : [ 											{
												"patchline" : 												{
													"source" : [ "obj-1", 0 ],
													"destination" : [ "obj-2", 0 ],
													"hidden" : 0
												}

											}
, 											{
												"patchline" : 												{
													"source" : [ "obj-2", 0 ],
													"destination" : [ "obj-7", 2 ],
													"hidden" : 0
												}

											}
, 											{
												"patchline" : 												{
													"source" : [ "obj-1", 0 ],
													"destination" : [ "obj-3", 0 ],
													"hidden" : 0,
													"midpoints" : [ 185.5, 216.0, 151.5, 216.0 ]
												}

											}
, 											{
												"patchline" : 												{
													"source" : [ "obj-3", 0 ],
													"destination" : [ "obj-7", 1 ],
													"hidden" : 0
												}

											}
, 											{
												"patchline" : 												{
													"source" : [ "obj-7", 0 ],
													"destination" : [ "obj-13", 0 ],
													"hidden" : 0
												}

											}
, 											{
												"patchline" : 												{
													"source" : [ "obj-4", 0 ],
													"destination" : [ "obj-7", 0 ],
													"hidden" : 0
												}

											}
, 											{
												"patchline" : 												{
													"source" : [ "obj-5", 0 ],
													"destination" : [ "obj-7", 0 ],
													"hidden" : 0
												}

											}
, 											{
												"patchline" : 												{
													"source" : [ "obj-6", 0 ],
													"destination" : [ "obj-7", 0 ],
													"hidden" : 0
												}

											}
, 											{
												"patchline" : 												{
													"source" : [ "obj-10", 0 ],
													"destination" : [ "obj-6", 0 ],
													"hidden" : 0
												}

											}
, 											{
												"patchline" : 												{
													"source" : [ "obj-15", 0 ],
													"destination" : [ "obj-10", 0 ],
													"hidden" : 0
												}

											}
, 											{
												"patchline" : 												{
													"source" : [ "obj-8", 0 ],
													"destination" : [ "obj-15", 0 ],
													"hidden" : 0
												}

											}
, 											{
												"patchline" : 												{
													"source" : [ "obj-9", 0 ],
													"destination" : [ "obj-8", 0 ],
													"hidden" : 0
												}

											}
, 											{
												"patchline" : 												{
													"source" : [ "obj-11", 0 ],
													"destination" : [ "obj-9", 0 ],
													"hidden" : 0
												}

											}
, 											{
												"patchline" : 												{
													"source" : [ "obj-14", 0 ],
													"destination" : [ "obj-11", 0 ],
													"hidden" : 0
												}

											}
, 											{
												"patchline" : 												{
													"source" : [ "obj-1", 0 ],
													"destination" : [ "obj-4", 0 ],
													"hidden" : 0,
													"midpoints" : [ 185.5, 214.0, 80.5, 214.0 ]
												}

											}
, 											{
												"patchline" : 												{
													"source" : [ "obj-1", 0 ],
													"destination" : [ "obj-5", 0 ],
													"hidden" : 0,
													"midpoints" : [ 185.5, 212.0, 30.5, 212.0 ]
												}

											}
 ]
									}
,
									"saved_object_attributes" : 									{
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
, 							{
								"box" : 								{
									"maxclass" : "newobj",
									"text" : "p sample_voice~",
									"fontsize" : 7.9,
									"numinlets" : 1,
									"patching_rect" : [ 189.0, 206.0, 78.0, 17.0 ],
									"numoutlets" : 1,
									"fontname" : "Arial",
									"outlettype" : [ "signal" ],
									"id" : "obj-2",
									"patcher" : 									{
										"rect" : [ 50.0, 40.0, 346.0, 375.0 ],
										"bgcolor" : [ 0.1, 0.1, 0.1, 1.0 ],
										"bglocked" : 0,
										"defrect" : [ 50.0, 40.0, 346.0, 375.0 ],
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
										"boxes" : [ 											{
												"box" : 												{
													"maxclass" : "newobj",
													"text" : "loadbang",
													"fontsize" : 7.9,
													"numinlets" : 1,
													"patching_rect" : [ 176.0, 189.0, 46.0, 17.0 ],
													"numoutlets" : 1,
													"fontname" : "Arial",
													"outlettype" : [ "bang" ],
													"id" : "obj-1"
												}

											}
, 											{
												"box" : 												{
													"maxclass" : "message",
													"text" : "2266.371582",
													"fontsize" : 7.9,
													"numinlets" : 2,
													"patching_rect" : [ 176.0, 225.0, 71.0, 15.0 ],
													"numoutlets" : 1,
													"fontname" : "Arial",
													"outlettype" : [ "" ],
													"id" : "obj-2"
												}

											}
, 											{
												"box" : 												{
													"maxclass" : "message",
													"text" : "0.03",
													"fontsize" : 7.9,
													"numinlets" : 2,
													"patching_rect" : [ 142.0, 225.0, 29.0, 15.0 ],
													"numoutlets" : 1,
													"fontname" : "Arial",
													"outlettype" : [ "" ],
													"id" : "obj-3"
												}

											}
, 											{
												"box" : 												{
													"maxclass" : "message",
													"text" : "loop 1",
													"fontsize" : 7.9,
													"numinlets" : 2,
													"patching_rect" : [ 71.0, 225.0, 35.0, 15.0 ],
													"numoutlets" : 1,
													"fontname" : "Arial",
													"outlettype" : [ "" ],
													"id" : "obj-4"
												}

											}
, 											{
												"box" : 												{
													"maxclass" : "message",
													"text" : "startloop",
													"fontsize" : 7.9,
													"numinlets" : 2,
													"patching_rect" : [ 21.0, 225.0, 49.0, 15.0 ],
													"numoutlets" : 1,
													"fontname" : "Arial",
													"outlettype" : [ "" ],
													"id" : "obj-5"
												}

											}
, 											{
												"box" : 												{
													"maxclass" : "newobj",
													"text" : "sig~ 1",
													"fontsize" : 7.9,
													"numinlets" : 1,
													"patching_rect" : [ 108.0, 188.0, 35.0, 17.0 ],
													"numoutlets" : 1,
													"fontname" : "Arial",
													"outlettype" : [ "signal" ],
													"id" : "obj-6"
												}

											}
, 											{
												"box" : 												{
													"maxclass" : "newobj",
													"text" : "groove~ oscil",
													"fontsize" : 7.9,
													"numinlets" : 3,
													"patching_rect" : [ 108.0, 245.0, 69.0, 17.0 ],
													"numoutlets" : 2,
													"fontname" : "Arial",
													"outlettype" : [ "signal", "signal" ],
													"id" : "obj-7"
												}

											}
, 											{
												"box" : 												{
													"maxclass" : "flonum",
													"htextcolor" : [ 0.870588, 0.870588, 0.870588, 1.0 ],
													"bgcolor" : [ 0.866667, 0.866667, 0.866667, 1.0 ],
													"fontsize" : 7.9,
													"numinlets" : 1,
													"patching_rect" : [ 108.0, 112.0, 52.0, 17.0 ],
													"numoutlets" : 2,
													"fontname" : "Arial",
													"outlettype" : [ "float", "bang" ],
													"id" : "obj-8",
													"triscale" : 0.9,
													"textcolor" : [ 0.0, 0.0, 0.0, 1.0 ]
												}

											}
, 											{
												"box" : 												{
													"maxclass" : "newobj",
													"text" : "mtof",
													"fontsize" : 7.9,
													"numinlets" : 1,
													"patching_rect" : [ 108.0, 89.0, 29.0, 17.0 ],
													"numoutlets" : 1,
													"fontname" : "Arial",
													"outlettype" : [ "float" ],
													"id" : "obj-9"
												}

											}
, 											{
												"box" : 												{
													"maxclass" : "flonum",
													"htextcolor" : [ 0.870588, 0.870588, 0.870588, 1.0 ],
													"bgcolor" : [ 0.866667, 0.866667, 0.866667, 1.0 ],
													"fontsize" : 7.9,
													"numinlets" : 1,
													"patching_rect" : [ 108.0, 159.0, 43.0, 17.0 ],
													"numoutlets" : 2,
													"fontname" : "Arial",
													"outlettype" : [ "float", "bang" ],
													"id" : "obj-10",
													"triscale" : 0.9,
													"textcolor" : [ 0.0, 0.0, 0.0, 1.0 ]
												}

											}
, 											{
												"box" : 												{
													"maxclass" : "flonum",
													"htextcolor" : [ 0.870588, 0.870588, 0.870588, 1.0 ],
													"bgcolor" : [ 0.866667, 0.866667, 0.866667, 1.0 ],
													"fontsize" : 7.9,
													"numinlets" : 1,
													"patching_rect" : [ 108.0, 65.0, 47.0, 17.0 ],
													"numoutlets" : 2,
													"fontname" : "Arial",
													"outlettype" : [ "float", "bang" ],
													"id" : "obj-11",
													"triscale" : 0.9,
													"textcolor" : [ 0.0, 0.0, 0.0, 1.0 ]
												}

											}
, 											{
												"box" : 												{
													"maxclass" : "comment",
													"text" : "note #",
													"fontsize" : 8.777778,
													"numinlets" : 1,
													"patching_rect" : [ 125.0, 42.0, 35.0, 18.0 ],
													"numoutlets" : 0,
													"fontname" : "Times New Roman",
													"frgb" : [ 0.658824, 0.658824, 0.658824, 1.0 ],
													"id" : "obj-12"
												}

											}
, 											{
												"box" : 												{
													"maxclass" : "outlet",
													"numinlets" : 1,
													"patching_rect" : [ 108.0, 272.0, 15.0, 15.0 ],
													"numoutlets" : 0,
													"id" : "obj-13",
													"comment" : "(signal) Left Output"
												}

											}
, 											{
												"box" : 												{
													"maxclass" : "inlet",
													"numinlets" : 0,
													"patching_rect" : [ 108.0, 42.0, 15.0, 15.0 ],
													"numoutlets" : 1,
													"outlettype" : [ "float" ],
													"id" : "obj-14",
													"comment" : "(int) note number"
												}

											}
, 											{
												"box" : 												{
													"maxclass" : "newobj",
													"text" : "\/ 261.625",
													"fontsize" : 8.777778,
													"numinlets" : 2,
													"patching_rect" : [ 108.0, 134.0, 72.0, 18.0 ],
													"numoutlets" : 1,
													"fontname" : "Courier",
													"outlettype" : [ "float" ],
													"id" : "obj-15"
												}

											}
, 											{
												"box" : 												{
													"maxclass" : "comment",
													"text" : "base frequency of sample (middle C)",
													"linecount" : 2,
													"fontsize" : 7.9,
													"numinlets" : 1,
													"patching_rect" : [ 171.0, 131.0, 87.0, 28.0 ],
													"numoutlets" : 0,
													"fontname" : "Arial",
													"frgb" : [ 0.658824, 0.658824, 0.658824, 1.0 ],
													"id" : "obj-16"
												}

											}
, 											{
												"box" : 												{
													"maxclass" : "comment",
													"text" : "Desired MIDI note # for sample",
													"linecount" : 2,
													"fontsize" : 7.9,
													"numinlets" : 1,
													"patching_rect" : [ 157.0, 63.0, 86.0, 28.0 ],
													"numoutlets" : 0,
													"fontname" : "Arial",
													"frgb" : [ 0.658824, 0.658824, 0.658824, 1.0 ],
													"id" : "obj-17"
												}

											}
, 											{
												"box" : 												{
													"maxclass" : "comment",
													"text" : "Ratio for playback speed",
													"fontsize" : 7.9,
													"numinlets" : 1,
													"patching_rect" : [ 152.0, 161.0, 115.0, 17.0 ],
													"numoutlets" : 0,
													"fontname" : "Arial",
													"frgb" : [ 0.658824, 0.658824, 0.658824, 1.0 ],
													"id" : "obj-18"
												}

											}
, 											{
												"box" : 												{
													"maxclass" : "comment",
													"text" : "Left Output",
													"fontsize" : 7.9,
													"numinlets" : 1,
													"patching_rect" : [ 54.0, 273.0, 57.0, 17.0 ],
													"numoutlets" : 0,
													"fontname" : "Arial",
													"frgb" : [ 0.658824, 0.658824, 0.658824, 1.0 ],
													"id" : "obj-19"
												}

											}
, 											{
												"box" : 												{
													"maxclass" : "comment",
													"text" : "MIDI to Frequency Conversion",
													"fontsize" : 7.9,
													"numinlets" : 1,
													"patching_rect" : [ 138.0, 91.0, 145.0, 17.0 ],
													"numoutlets" : 0,
													"fontname" : "Arial",
													"frgb" : [ 0.658824, 0.658824, 0.658824, 1.0 ],
													"id" : "obj-20"
												}

											}
, 											{
												"box" : 												{
													"maxclass" : "comment",
													"text" : "Desired frequency of sample",
													"fontsize" : 7.9,
													"numinlets" : 1,
													"patching_rect" : [ 162.0, 112.0, 134.0, 17.0 ],
													"numoutlets" : 0,
													"fontname" : "Arial",
													"frgb" : [ 0.658824, 0.658824, 0.658824, 1.0 ],
													"id" : "obj-21"
												}

											}
 ],
										"lines" : [ 											{
												"patchline" : 												{
													"source" : [ "obj-1", 0 ],
													"destination" : [ "obj-2", 0 ],
													"hidden" : 0
												}

											}
, 											{
												"patchline" : 												{
													"source" : [ "obj-2", 0 ],
													"destination" : [ "obj-7", 2 ],
													"hidden" : 0
												}

											}
, 											{
												"patchline" : 												{
													"source" : [ "obj-1", 0 ],
													"destination" : [ "obj-3", 0 ],
													"hidden" : 0,
													"midpoints" : [ 185.5, 216.0, 151.5, 216.0 ]
												}

											}
, 											{
												"patchline" : 												{
													"source" : [ "obj-3", 0 ],
													"destination" : [ "obj-7", 1 ],
													"hidden" : 0
												}

											}
, 											{
												"patchline" : 												{
													"source" : [ "obj-7", 0 ],
													"destination" : [ "obj-13", 0 ],
													"hidden" : 0
												}

											}
, 											{
												"patchline" : 												{
													"source" : [ "obj-6", 0 ],
													"destination" : [ "obj-7", 0 ],
													"hidden" : 0
												}

											}
, 											{
												"patchline" : 												{
													"source" : [ "obj-5", 0 ],
													"destination" : [ "obj-7", 0 ],
													"hidden" : 0
												}

											}
, 											{
												"patchline" : 												{
													"source" : [ "obj-4", 0 ],
													"destination" : [ "obj-7", 0 ],
													"hidden" : 0
												}

											}
, 											{
												"patchline" : 												{
													"source" : [ "obj-10", 0 ],
													"destination" : [ "obj-6", 0 ],
													"hidden" : 0
												}

											}
, 											{
												"patchline" : 												{
													"source" : [ "obj-15", 0 ],
													"destination" : [ "obj-10", 0 ],
													"hidden" : 0
												}

											}
, 											{
												"patchline" : 												{
													"source" : [ "obj-8", 0 ],
													"destination" : [ "obj-15", 0 ],
													"hidden" : 0
												}

											}
, 											{
												"patchline" : 												{
													"source" : [ "obj-9", 0 ],
													"destination" : [ "obj-8", 0 ],
													"hidden" : 0
												}

											}
, 											{
												"patchline" : 												{
													"source" : [ "obj-11", 0 ],
													"destination" : [ "obj-9", 0 ],
													"hidden" : 0
												}

											}
, 											{
												"patchline" : 												{
													"source" : [ "obj-14", 0 ],
													"destination" : [ "obj-11", 0 ],
													"hidden" : 0
												}

											}
, 											{
												"patchline" : 												{
													"source" : [ "obj-1", 0 ],
													"destination" : [ "obj-4", 0 ],
													"hidden" : 0,
													"midpoints" : [ 185.5, 214.0, 80.5, 214.0 ]
												}

											}
, 											{
												"patchline" : 												{
													"source" : [ "obj-1", 0 ],
													"destination" : [ "obj-5", 0 ],
													"hidden" : 0,
													"midpoints" : [ 185.5, 212.0, 30.5, 212.0 ]
												}

											}
 ]
									}
,
									"saved_object_attributes" : 									{
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
, 							{
								"box" : 								{
									"maxclass" : "newobj",
									"text" : "p sample_voice~",
									"fontsize" : 7.9,
									"numinlets" : 1,
									"patching_rect" : [ 106.0, 206.0, 78.0, 17.0 ],
									"numoutlets" : 1,
									"fontname" : "Arial",
									"outlettype" : [ "signal" ],
									"id" : "obj-3",
									"patcher" : 									{
										"rect" : [ 50.0, 40.0, 346.0, 375.0 ],
										"bgcolor" : [ 0.1, 0.1, 0.1, 1.0 ],
										"bglocked" : 0,
										"defrect" : [ 50.0, 40.0, 346.0, 375.0 ],
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
										"boxes" : [ 											{
												"box" : 												{
													"maxclass" : "newobj",
													"text" : "loadbang",
													"fontsize" : 7.9,
													"numinlets" : 1,
													"patching_rect" : [ 176.0, 189.0, 46.0, 17.0 ],
													"numoutlets" : 1,
													"fontname" : "Arial",
													"outlettype" : [ "bang" ],
													"id" : "obj-1"
												}

											}
, 											{
												"box" : 												{
													"maxclass" : "message",
													"text" : "2266.371582",
													"fontsize" : 7.9,
													"numinlets" : 2,
													"patching_rect" : [ 176.0, 225.0, 71.0, 15.0 ],
													"numoutlets" : 1,
													"fontname" : "Arial",
													"outlettype" : [ "" ],
													"id" : "obj-2"
												}

											}
, 											{
												"box" : 												{
													"maxclass" : "message",
													"text" : "0.03",
													"fontsize" : 7.9,
													"numinlets" : 2,
													"patching_rect" : [ 142.0, 225.0, 29.0, 15.0 ],
													"numoutlets" : 1,
													"fontname" : "Arial",
													"outlettype" : [ "" ],
													"id" : "obj-3"
												}

											}
, 											{
												"box" : 												{
													"maxclass" : "message",
													"text" : "loop 1",
													"fontsize" : 7.9,
													"numinlets" : 2,
													"patching_rect" : [ 71.0, 225.0, 35.0, 15.0 ],
													"numoutlets" : 1,
													"fontname" : "Arial",
													"outlettype" : [ "" ],
													"id" : "obj-4"
												}

											}
, 											{
												"box" : 												{
													"maxclass" : "message",
													"text" : "startloop",
													"fontsize" : 7.9,
													"numinlets" : 2,
													"patching_rect" : [ 21.0, 225.0, 49.0, 15.0 ],
													"numoutlets" : 1,
													"fontname" : "Arial",
													"outlettype" : [ "" ],
													"id" : "obj-5"
												}

											}
, 											{
												"box" : 												{
													"maxclass" : "newobj",
													"text" : "sig~ 1",
													"fontsize" : 7.9,
													"numinlets" : 1,
													"patching_rect" : [ 108.0, 188.0, 35.0, 17.0 ],
													"numoutlets" : 1,
													"fontname" : "Arial",
													"outlettype" : [ "signal" ],
													"id" : "obj-6"
												}

											}
, 											{
												"box" : 												{
													"maxclass" : "newobj",
													"text" : "groove~ oscil",
													"fontsize" : 7.9,
													"numinlets" : 3,
													"patching_rect" : [ 108.0, 245.0, 69.0, 17.0 ],
													"numoutlets" : 2,
													"fontname" : "Arial",
													"outlettype" : [ "signal", "signal" ],
													"id" : "obj-7"
												}

											}
, 											{
												"box" : 												{
													"maxclass" : "flonum",
													"htextcolor" : [ 0.870588, 0.870588, 0.870588, 1.0 ],
													"bgcolor" : [ 0.866667, 0.866667, 0.866667, 1.0 ],
													"fontsize" : 7.9,
													"numinlets" : 1,
													"patching_rect" : [ 108.0, 112.0, 52.0, 17.0 ],
													"numoutlets" : 2,
													"fontname" : "Arial",
													"outlettype" : [ "float", "bang" ],
													"id" : "obj-8",
													"triscale" : 0.9,
													"textcolor" : [ 0.0, 0.0, 0.0, 1.0 ]
												}

											}
, 											{
												"box" : 												{
													"maxclass" : "newobj",
													"text" : "mtof",
													"fontsize" : 7.9,
													"numinlets" : 1,
													"patching_rect" : [ 108.0, 89.0, 29.0, 17.0 ],
													"numoutlets" : 1,
													"fontname" : "Arial",
													"outlettype" : [ "float" ],
													"id" : "obj-9"
												}

											}
, 											{
												"box" : 												{
													"maxclass" : "flonum",
													"htextcolor" : [ 0.870588, 0.870588, 0.870588, 1.0 ],
													"bgcolor" : [ 0.866667, 0.866667, 0.866667, 1.0 ],
													"fontsize" : 7.9,
													"numinlets" : 1,
													"patching_rect" : [ 108.0, 159.0, 43.0, 17.0 ],
													"numoutlets" : 2,
													"fontname" : "Arial",
													"outlettype" : [ "float", "bang" ],
													"id" : "obj-10",
													"triscale" : 0.9,
													"textcolor" : [ 0.0, 0.0, 0.0, 1.0 ]
												}

											}
, 											{
												"box" : 												{
													"maxclass" : "flonum",
													"htextcolor" : [ 0.870588, 0.870588, 0.870588, 1.0 ],
													"bgcolor" : [ 0.866667, 0.866667, 0.866667, 1.0 ],
													"fontsize" : 7.9,
													"numinlets" : 1,
													"patching_rect" : [ 108.0, 65.0, 47.0, 17.0 ],
													"numoutlets" : 2,
													"fontname" : "Arial",
													"outlettype" : [ "float", "bang" ],
													"id" : "obj-11",
													"triscale" : 0.9,
													"textcolor" : [ 0.0, 0.0, 0.0, 1.0 ]
												}

											}
, 											{
												"box" : 												{
													"maxclass" : "comment",
													"text" : "note #",
													"fontsize" : 8.777778,
													"numinlets" : 1,
													"patching_rect" : [ 125.0, 42.0, 35.0, 18.0 ],
													"numoutlets" : 0,
													"fontname" : "Times New Roman",
													"frgb" : [ 0.658824, 0.658824, 0.658824, 1.0 ],
													"id" : "obj-12"
												}

											}
, 											{
												"box" : 												{
													"maxclass" : "outlet",
													"numinlets" : 1,
													"patching_rect" : [ 108.0, 272.0, 15.0, 15.0 ],
													"numoutlets" : 0,
													"id" : "obj-13",
													"comment" : "(signal) Left Output"
												}

											}
, 											{
												"box" : 												{
													"maxclass" : "inlet",
													"numinlets" : 0,
													"patching_rect" : [ 108.0, 42.0, 15.0, 15.0 ],
													"numoutlets" : 1,
													"outlettype" : [ "float" ],
													"id" : "obj-14",
													"comment" : "(int) note number"
												}

											}
, 											{
												"box" : 												{
													"maxclass" : "newobj",
													"text" : "\/ 261.625",
													"fontsize" : 8.777778,
													"numinlets" : 2,
													"patching_rect" : [ 108.0, 134.0, 72.0, 18.0 ],
													"numoutlets" : 1,
													"fontname" : "Courier",
													"outlettype" : [ "float" ],
													"id" : "obj-15"
												}

											}
, 											{
												"box" : 												{
													"maxclass" : "comment",
													"text" : "base frequency of sample (middle C)",
													"linecount" : 2,
													"fontsize" : 7.9,
													"numinlets" : 1,
													"patching_rect" : [ 171.0, 131.0, 87.0, 28.0 ],
													"numoutlets" : 0,
													"fontname" : "Arial",
													"frgb" : [ 0.658824, 0.658824, 0.658824, 1.0 ],
													"id" : "obj-16"
												}

											}
, 											{
												"box" : 												{
													"maxclass" : "comment",
													"text" : "Desired MIDI note # for sample",
													"linecount" : 2,
													"fontsize" : 7.9,
													"numinlets" : 1,
													"patching_rect" : [ 157.0, 63.0, 86.0, 28.0 ],
													"numoutlets" : 0,
													"fontname" : "Arial",
													"frgb" : [ 0.658824, 0.658824, 0.658824, 1.0 ],
													"id" : "obj-17"
												}

											}
, 											{
												"box" : 												{
													"maxclass" : "comment",
													"text" : "Ratio for playback speed",
													"fontsize" : 7.9,
													"numinlets" : 1,
													"patching_rect" : [ 152.0, 161.0, 115.0, 17.0 ],
													"numoutlets" : 0,
													"fontname" : "Arial",
													"frgb" : [ 0.658824, 0.658824, 0.658824, 1.0 ],
													"id" : "obj-18"
												}

											}
, 											{
												"box" : 												{
													"maxclass" : "comment",
													"text" : "Left Output",
													"fontsize" : 7.9,
													"numinlets" : 1,
													"patching_rect" : [ 54.0, 273.0, 57.0, 17.0 ],
													"numoutlets" : 0,
													"fontname" : "Arial",
													"frgb" : [ 0.658824, 0.658824, 0.658824, 1.0 ],
													"id" : "obj-19"
												}

											}
, 											{
												"box" : 												{
													"maxclass" : "comment",
													"text" : "MIDI to Frequency Conversion",
													"fontsize" : 7.9,
													"numinlets" : 1,
													"patching_rect" : [ 138.0, 91.0, 145.0, 17.0 ],
													"numoutlets" : 0,
													"fontname" : "Arial",
													"frgb" : [ 0.658824, 0.658824, 0.658824, 1.0 ],
													"id" : "obj-20"
												}

											}
, 											{
												"box" : 												{
													"maxclass" : "comment",
													"text" : "Desired frequency of sample",
													"fontsize" : 7.9,
													"numinlets" : 1,
													"patching_rect" : [ 162.0, 112.0, 134.0, 17.0 ],
													"numoutlets" : 0,
													"fontname" : "Arial",
													"frgb" : [ 0.658824, 0.658824, 0.658824, 1.0 ],
													"id" : "obj-21"
												}

											}
 ],
										"lines" : [ 											{
												"patchline" : 												{
													"source" : [ "obj-1", 0 ],
													"destination" : [ "obj-2", 0 ],
													"hidden" : 0
												}

											}
, 											{
												"patchline" : 												{
													"source" : [ "obj-2", 0 ],
													"destination" : [ "obj-7", 2 ],
													"hidden" : 0
												}

											}
, 											{
												"patchline" : 												{
													"source" : [ "obj-1", 0 ],
													"destination" : [ "obj-3", 0 ],
													"hidden" : 0,
													"midpoints" : [ 185.5, 216.0, 151.5, 216.0 ]
												}

											}
, 											{
												"patchline" : 												{
													"source" : [ "obj-3", 0 ],
													"destination" : [ "obj-7", 1 ],
													"hidden" : 0
												}

											}
, 											{
												"patchline" : 												{
													"source" : [ "obj-7", 0 ],
													"destination" : [ "obj-13", 0 ],
													"hidden" : 0
												}

											}
, 											{
												"patchline" : 												{
													"source" : [ "obj-4", 0 ],
													"destination" : [ "obj-7", 0 ],
													"hidden" : 0
												}

											}
, 											{
												"patchline" : 												{
													"source" : [ "obj-5", 0 ],
													"destination" : [ "obj-7", 0 ],
													"hidden" : 0
												}

											}
, 											{
												"patchline" : 												{
													"source" : [ "obj-6", 0 ],
													"destination" : [ "obj-7", 0 ],
													"hidden" : 0
												}

											}
, 											{
												"patchline" : 												{
													"source" : [ "obj-10", 0 ],
													"destination" : [ "obj-6", 0 ],
													"hidden" : 0
												}

											}
, 											{
												"patchline" : 												{
													"source" : [ "obj-15", 0 ],
													"destination" : [ "obj-10", 0 ],
													"hidden" : 0
												}

											}
, 											{
												"patchline" : 												{
													"source" : [ "obj-8", 0 ],
													"destination" : [ "obj-15", 0 ],
													"hidden" : 0
												}

											}
, 											{
												"patchline" : 												{
													"source" : [ "obj-9", 0 ],
													"destination" : [ "obj-8", 0 ],
													"hidden" : 0
												}

											}
, 											{
												"patchline" : 												{
													"source" : [ "obj-11", 0 ],
													"destination" : [ "obj-9", 0 ],
													"hidden" : 0
												}

											}
, 											{
												"patchline" : 												{
													"source" : [ "obj-14", 0 ],
													"destination" : [ "obj-11", 0 ],
													"hidden" : 0
												}

											}
, 											{
												"patchline" : 												{
													"source" : [ "obj-1", 0 ],
													"destination" : [ "obj-4", 0 ],
													"hidden" : 0,
													"midpoints" : [ 185.5, 214.0, 80.5, 214.0 ]
												}

											}
, 											{
												"patchline" : 												{
													"source" : [ "obj-1", 0 ],
													"destination" : [ "obj-5", 0 ],
													"hidden" : 0,
													"midpoints" : [ 185.5, 212.0, 30.5, 212.0 ]
												}

											}
 ]
									}
,
									"saved_object_attributes" : 									{
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
, 							{
								"box" : 								{
									"maxclass" : "newobj",
									"text" : "p sample_voice~",
									"fontsize" : 7.9,
									"numinlets" : 1,
									"patching_rect" : [ 23.0, 206.0, 78.0, 17.0 ],
									"numoutlets" : 1,
									"fontname" : "Arial",
									"outlettype" : [ "signal" ],
									"id" : "obj-4",
									"patcher" : 									{
										"rect" : [ 50.0, 40.0, 346.0, 375.0 ],
										"bgcolor" : [ 0.1, 0.1, 0.1, 1.0 ],
										"bglocked" : 0,
										"defrect" : [ 50.0, 40.0, 346.0, 375.0 ],
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
										"boxes" : [ 											{
												"box" : 												{
													"maxclass" : "newobj",
													"text" : "loadbang",
													"fontsize" : 7.9,
													"numinlets" : 1,
													"patching_rect" : [ 176.0, 189.0, 46.0, 17.0 ],
													"numoutlets" : 1,
													"fontname" : "Arial",
													"outlettype" : [ "bang" ],
													"id" : "obj-1"
												}

											}
, 											{
												"box" : 												{
													"maxclass" : "message",
													"text" : "2266.371582",
													"fontsize" : 7.9,
													"numinlets" : 2,
													"patching_rect" : [ 176.0, 225.0, 71.0, 15.0 ],
													"numoutlets" : 1,
													"fontname" : "Arial",
													"outlettype" : [ "" ],
													"id" : "obj-2"
												}

											}
, 											{
												"box" : 												{
													"maxclass" : "message",
													"text" : "0.03",
													"fontsize" : 7.9,
													"numinlets" : 2,
													"patching_rect" : [ 142.0, 225.0, 29.0, 15.0 ],
													"numoutlets" : 1,
													"fontname" : "Arial",
													"outlettype" : [ "" ],
													"id" : "obj-3"
												}

											}
, 											{
												"box" : 												{
													"maxclass" : "message",
													"text" : "loop 1",
													"fontsize" : 7.9,
													"numinlets" : 2,
													"patching_rect" : [ 71.0, 225.0, 35.0, 15.0 ],
													"numoutlets" : 1,
													"fontname" : "Arial",
													"outlettype" : [ "" ],
													"id" : "obj-4"
												}

											}
, 											{
												"box" : 												{
													"maxclass" : "message",
													"text" : "startloop",
													"fontsize" : 7.9,
													"numinlets" : 2,
													"patching_rect" : [ 21.0, 225.0, 49.0, 15.0 ],
													"numoutlets" : 1,
													"fontname" : "Arial",
													"outlettype" : [ "" ],
													"id" : "obj-5"
												}

											}
, 											{
												"box" : 												{
													"maxclass" : "newobj",
													"text" : "sig~ 1",
													"fontsize" : 7.9,
													"numinlets" : 1,
													"patching_rect" : [ 108.0, 188.0, 35.0, 17.0 ],
													"numoutlets" : 1,
													"fontname" : "Arial",
													"outlettype" : [ "signal" ],
													"id" : "obj-6"
												}

											}
, 											{
												"box" : 												{
													"maxclass" : "newobj",
													"text" : "groove~ oscil",
													"fontsize" : 7.9,
													"numinlets" : 3,
													"patching_rect" : [ 108.0, 245.0, 69.0, 17.0 ],
													"numoutlets" : 2,
													"fontname" : "Arial",
													"outlettype" : [ "signal", "signal" ],
													"id" : "obj-7"
												}

											}
, 											{
												"box" : 												{
													"maxclass" : "flonum",
													"htextcolor" : [ 0.870588, 0.870588, 0.870588, 1.0 ],
													"bgcolor" : [ 0.866667, 0.866667, 0.866667, 1.0 ],
													"fontsize" : 7.9,
													"numinlets" : 1,
													"patching_rect" : [ 108.0, 112.0, 52.0, 17.0 ],
													"numoutlets" : 2,
													"fontname" : "Arial",
													"outlettype" : [ "float", "bang" ],
													"id" : "obj-8",
													"triscale" : 0.9,
													"textcolor" : [ 0.0, 0.0, 0.0, 1.0 ]
												}

											}
, 											{
												"box" : 												{
													"maxclass" : "newobj",
													"text" : "mtof",
													"fontsize" : 7.9,
													"numinlets" : 1,
													"patching_rect" : [ 108.0, 89.0, 29.0, 17.0 ],
													"numoutlets" : 1,
													"fontname" : "Arial",
													"outlettype" : [ "float" ],
													"id" : "obj-9"
												}

											}
, 											{
												"box" : 												{
													"maxclass" : "flonum",
													"htextcolor" : [ 0.870588, 0.870588, 0.870588, 1.0 ],
													"bgcolor" : [ 0.866667, 0.866667, 0.866667, 1.0 ],
													"fontsize" : 7.9,
													"numinlets" : 1,
													"patching_rect" : [ 108.0, 159.0, 43.0, 17.0 ],
													"numoutlets" : 2,
													"fontname" : "Arial",
													"outlettype" : [ "float", "bang" ],
													"id" : "obj-10",
													"triscale" : 0.9,
													"textcolor" : [ 0.0, 0.0, 0.0, 1.0 ]
												}

											}
, 											{
												"box" : 												{
													"maxclass" : "flonum",
													"htextcolor" : [ 0.870588, 0.870588, 0.870588, 1.0 ],
													"bgcolor" : [ 0.866667, 0.866667, 0.866667, 1.0 ],
													"fontsize" : 7.9,
													"numinlets" : 1,
													"patching_rect" : [ 108.0, 65.0, 47.0, 17.0 ],
													"numoutlets" : 2,
													"fontname" : "Arial",
													"outlettype" : [ "float", "bang" ],
													"id" : "obj-11",
													"triscale" : 0.9,
													"textcolor" : [ 0.0, 0.0, 0.0, 1.0 ]
												}

											}
, 											{
												"box" : 												{
													"maxclass" : "comment",
													"text" : "note #",
													"fontsize" : 8.777778,
													"numinlets" : 1,
													"patching_rect" : [ 125.0, 42.0, 35.0, 18.0 ],
													"numoutlets" : 0,
													"fontname" : "Times New Roman",
													"frgb" : [ 0.658824, 0.658824, 0.658824, 1.0 ],
													"id" : "obj-12"
												}

											}
, 											{
												"box" : 												{
													"maxclass" : "outlet",
													"numinlets" : 1,
													"patching_rect" : [ 108.0, 272.0, 15.0, 15.0 ],
													"numoutlets" : 0,
													"id" : "obj-13",
													"comment" : "(signal) Left Output"
												}

											}
, 											{
												"box" : 												{
													"maxclass" : "inlet",
													"numinlets" : 0,
													"patching_rect" : [ 108.0, 42.0, 15.0, 15.0 ],
													"numoutlets" : 1,
													"outlettype" : [ "float" ],
													"id" : "obj-14",
													"comment" : "(int) note number"
												}

											}
, 											{
												"box" : 												{
													"maxclass" : "newobj",
													"text" : "\/ 261.625",
													"fontsize" : 8.777778,
													"numinlets" : 2,
													"patching_rect" : [ 108.0, 134.0, 72.0, 18.0 ],
													"numoutlets" : 1,
													"fontname" : "Courier",
													"outlettype" : [ "float" ],
													"id" : "obj-15"
												}

											}
, 											{
												"box" : 												{
													"maxclass" : "comment",
													"text" : "base frequency of sample (middle C)",
													"linecount" : 2,
													"fontsize" : 7.9,
													"numinlets" : 1,
													"patching_rect" : [ 171.0, 131.0, 87.0, 28.0 ],
													"numoutlets" : 0,
													"fontname" : "Arial",
													"frgb" : [ 0.658824, 0.658824, 0.658824, 1.0 ],
													"id" : "obj-16"
												}

											}
, 											{
												"box" : 												{
													"maxclass" : "comment",
													"text" : "Desired MIDI note # for sample",
													"linecount" : 2,
													"fontsize" : 7.9,
													"numinlets" : 1,
													"patching_rect" : [ 157.0, 63.0, 86.0, 28.0 ],
													"numoutlets" : 0,
													"fontname" : "Arial",
													"frgb" : [ 0.658824, 0.658824, 0.658824, 1.0 ],
													"id" : "obj-17"
												}

											}
, 											{
												"box" : 												{
													"maxclass" : "comment",
													"text" : "Ratio for playback speed",
													"fontsize" : 7.9,
													"numinlets" : 1,
													"patching_rect" : [ 152.0, 161.0, 115.0, 17.0 ],
													"numoutlets" : 0,
													"fontname" : "Arial",
													"frgb" : [ 0.658824, 0.658824, 0.658824, 1.0 ],
													"id" : "obj-18"
												}

											}
, 											{
												"box" : 												{
													"maxclass" : "comment",
													"text" : "Left Output",
													"fontsize" : 7.9,
													"numinlets" : 1,
													"patching_rect" : [ 54.0, 273.0, 57.0, 17.0 ],
													"numoutlets" : 0,
													"fontname" : "Arial",
													"frgb" : [ 0.658824, 0.658824, 0.658824, 1.0 ],
													"id" : "obj-19"
												}

											}
, 											{
												"box" : 												{
													"maxclass" : "comment",
													"text" : "MIDI to Frequency Conversion",
													"fontsize" : 7.9,
													"numinlets" : 1,
													"patching_rect" : [ 138.0, 91.0, 145.0, 17.0 ],
													"numoutlets" : 0,
													"fontname" : "Arial",
													"frgb" : [ 0.658824, 0.658824, 0.658824, 1.0 ],
													"id" : "obj-20"
												}

											}
, 											{
												"box" : 												{
													"maxclass" : "comment",
													"text" : "Desired frequency of sample",
													"fontsize" : 7.9,
													"numinlets" : 1,
													"patching_rect" : [ 162.0, 112.0, 134.0, 17.0 ],
													"numoutlets" : 0,
													"fontname" : "Arial",
													"frgb" : [ 0.658824, 0.658824, 0.658824, 1.0 ],
													"id" : "obj-21"
												}

											}
 ],
										"lines" : [ 											{
												"patchline" : 												{
													"source" : [ "obj-1", 0 ],
													"destination" : [ "obj-2", 0 ],
													"hidden" : 0
												}

											}
, 											{
												"patchline" : 												{
													"source" : [ "obj-2", 0 ],
													"destination" : [ "obj-7", 2 ],
													"hidden" : 0
												}

											}
, 											{
												"patchline" : 												{
													"source" : [ "obj-1", 0 ],
													"destination" : [ "obj-3", 0 ],
													"hidden" : 0,
													"midpoints" : [ 185.5, 216.0, 151.5, 216.0 ]
												}

											}
, 											{
												"patchline" : 												{
													"source" : [ "obj-3", 0 ],
													"destination" : [ "obj-7", 1 ],
													"hidden" : 0
												}

											}
, 											{
												"patchline" : 												{
													"source" : [ "obj-7", 0 ],
													"destination" : [ "obj-13", 0 ],
													"hidden" : 0
												}

											}
, 											{
												"patchline" : 												{
													"source" : [ "obj-6", 0 ],
													"destination" : [ "obj-7", 0 ],
													"hidden" : 0
												}

											}
, 											{
												"patchline" : 												{
													"source" : [ "obj-5", 0 ],
													"destination" : [ "obj-7", 0 ],
													"hidden" : 0
												}

											}
, 											{
												"patchline" : 												{
													"source" : [ "obj-4", 0 ],
													"destination" : [ "obj-7", 0 ],
													"hidden" : 0
												}

											}
, 											{
												"patchline" : 												{
													"source" : [ "obj-10", 0 ],
													"destination" : [ "obj-6", 0 ],
													"hidden" : 0
												}

											}
, 											{
												"patchline" : 												{
													"source" : [ "obj-15", 0 ],
													"destination" : [ "obj-10", 0 ],
													"hidden" : 0
												}

											}
, 											{
												"patchline" : 												{
													"source" : [ "obj-8", 0 ],
													"destination" : [ "obj-15", 0 ],
													"hidden" : 0
												}

											}
, 											{
												"patchline" : 												{
													"source" : [ "obj-9", 0 ],
													"destination" : [ "obj-8", 0 ],
													"hidden" : 0
												}

											}
, 											{
												"patchline" : 												{
													"source" : [ "obj-11", 0 ],
													"destination" : [ "obj-9", 0 ],
													"hidden" : 0
												}

											}
, 											{
												"patchline" : 												{
													"source" : [ "obj-14", 0 ],
													"destination" : [ "obj-11", 0 ],
													"hidden" : 0
												}

											}
, 											{
												"patchline" : 												{
													"source" : [ "obj-1", 0 ],
													"destination" : [ "obj-4", 0 ],
													"hidden" : 0,
													"midpoints" : [ 185.5, 214.0, 80.5, 214.0 ]
												}

											}
, 											{
												"patchline" : 												{
													"source" : [ "obj-1", 0 ],
													"destination" : [ "obj-5", 0 ],
													"hidden" : 0,
													"midpoints" : [ 185.5, 212.0, 30.5, 212.0 ]
												}

											}
 ]
									}
,
									"saved_object_attributes" : 									{
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
, 							{
								"box" : 								{
									"maxclass" : "newobj",
									"text" : "pass~",
									"fontsize" : 7.9,
									"numinlets" : 1,
									"patching_rect" : [ 23.0, 259.0, 34.0, 17.0 ],
									"numoutlets" : 1,
									"fontname" : "Arial",
									"outlettype" : [ "signal" ],
									"id" : "obj-5"
								}

							}
, 							{
								"box" : 								{
									"maxclass" : "newobj",
									"text" : "pack",
									"fontsize" : 7.9,
									"numinlets" : 2,
									"color" : [ 1.0, 0.890196, 0.090196, 1.0 ],
									"patching_rect" : [ 23.0, 107.0, 28.0, 17.0 ],
									"numoutlets" : 1,
									"fontname" : "Arial",
									"outlettype" : [ "" ],
									"id" : "obj-6"
								}

							}
, 							{
								"box" : 								{
									"maxclass" : "newobj",
									"text" : "route 1 2 3 4",
									"fontsize" : 7.9,
									"numinlets" : 1,
									"color" : [ 1.0, 0.890196, 0.090196, 1.0 ],
									"patching_rect" : [ 23.0, 129.0, 68.0, 17.0 ],
									"numoutlets" : 5,
									"fontname" : "Arial",
									"outlettype" : [ "", "", "", "", "" ],
									"id" : "obj-7"
								}

							}
, 							{
								"box" : 								{
									"maxclass" : "newobj",
									"text" : "append 127",
									"fontsize" : 7.9,
									"numinlets" : 1,
									"color" : [ 1.0, 0.890196, 0.090196, 1.0 ],
									"patching_rect" : [ 23.0, 62.0, 59.0, 17.0 ],
									"numoutlets" : 1,
									"fontname" : "Arial",
									"outlettype" : [ "" ],
									"id" : "obj-8"
								}

							}
, 							{
								"box" : 								{
									"maxclass" : "newobj",
									"text" : "poly 4 1",
									"fontsize" : 7.9,
									"numinlets" : 2,
									"color" : [ 1.0, 0.890196, 0.090196, 1.0 ],
									"patching_rect" : [ 23.0, 84.0, 46.0, 17.0 ],
									"numoutlets" : 3,
									"fontname" : "Arial",
									"outlettype" : [ "int", "int", "int" ],
									"id" : "obj-9"
								}

							}
, 							{
								"box" : 								{
									"maxclass" : "newobj",
									"text" : "+ 0.",
									"fontsize" : 7.9,
									"numinlets" : 2,
									"color" : [ 1.0, 0.890196, 0.090196, 1.0 ],
									"patching_rect" : [ 272.0, 178.0, 32.0, 17.0 ],
									"numoutlets" : 1,
									"fontname" : "Arial",
									"outlettype" : [ "float" ],
									"id" : "obj-10"
								}

							}
, 							{
								"box" : 								{
									"maxclass" : "newobj",
									"text" : "+ 0.",
									"fontsize" : 7.9,
									"numinlets" : 2,
									"color" : [ 1.0, 0.890196, 0.090196, 1.0 ],
									"patching_rect" : [ 189.0, 178.0, 32.0, 17.0 ],
									"numoutlets" : 1,
									"fontname" : "Arial",
									"outlettype" : [ "float" ],
									"id" : "obj-11"
								}

							}
, 							{
								"box" : 								{
									"maxclass" : "newobj",
									"text" : "+ 0.",
									"fontsize" : 7.9,
									"numinlets" : 2,
									"color" : [ 1.0, 0.890196, 0.090196, 1.0 ],
									"patching_rect" : [ 106.0, 178.0, 32.0, 17.0 ],
									"numoutlets" : 1,
									"fontname" : "Arial",
									"outlettype" : [ "float" ],
									"id" : "obj-12"
								}

							}
, 							{
								"box" : 								{
									"maxclass" : "newobj",
									"text" : "+ 0.",
									"fontsize" : 7.9,
									"numinlets" : 2,
									"color" : [ 1.0, 0.890196, 0.090196, 1.0 ],
									"patching_rect" : [ 23.0, 178.0, 27.0, 17.0 ],
									"numoutlets" : 1,
									"fontname" : "Arial",
									"outlettype" : [ "float" ],
									"id" : "obj-13"
								}

							}
, 							{
								"box" : 								{
									"maxclass" : "inlet",
									"numinlets" : 0,
									"patching_rect" : [ 23.0, 42.0, 15.0, 15.0 ],
									"numoutlets" : 1,
									"outlettype" : [ "int" ],
									"id" : "obj-14",
									"comment" : "(float) Note 0-127"
								}

							}
, 							{
								"box" : 								{
									"maxclass" : "newobj",
									"text" : "* 0.01",
									"fontsize" : 7.9,
									"numinlets" : 2,
									"color" : [ 1.0, 0.890196, 0.090196, 1.0 ],
									"patching_rect" : [ 386.0, 66.0, 39.0, 17.0 ],
									"numoutlets" : 1,
									"fontname" : "Arial",
									"outlettype" : [ "float" ],
									"id" : "obj-15"
								}

							}
, 							{
								"box" : 								{
									"maxclass" : "flonum",
									"htextcolor" : [ 0.870588, 0.870588, 0.870588, 1.0 ],
									"bgcolor" : [ 0.866667, 0.866667, 0.866667, 1.0 ],
									"fontsize" : 7.9,
									"numinlets" : 1,
									"patching_rect" : [ 369.0, 135.0, 35.0, 17.0 ],
									"numoutlets" : 2,
									"fontname" : "Arial",
									"outlettype" : [ "float", "bang" ],
									"id" : "obj-16",
									"triscale" : 0.9,
									"textcolor" : [ 0.0, 0.0, 0.0, 1.0 ]
								}

							}
, 							{
								"box" : 								{
									"maxclass" : "newobj",
									"text" : "t b f",
									"fontsize" : 7.9,
									"numinlets" : 1,
									"color" : [ 1.0, 0.890196, 0.090196, 1.0 ],
									"patching_rect" : [ 386.0, 87.0, 27.0, 17.0 ],
									"numoutlets" : 2,
									"fontname" : "Arial",
									"outlettype" : [ "bang", "float" ],
									"id" : "obj-17"
								}

							}
, 							{
								"box" : 								{
									"maxclass" : "newobj",
									"text" : "+ 0.",
									"fontsize" : 7.9,
									"numinlets" : 2,
									"color" : [ 1.0, 0.890196, 0.090196, 1.0 ],
									"patching_rect" : [ 369.0, 114.0, 27.0, 17.0 ],
									"numoutlets" : 1,
									"fontname" : "Arial",
									"outlettype" : [ "float" ],
									"id" : "obj-18"
								}

							}
, 							{
								"box" : 								{
									"maxclass" : "inlet",
									"numinlets" : 0,
									"patching_rect" : [ 386.0, 47.0, 15.0, 15.0 ],
									"numoutlets" : 1,
									"outlettype" : [ "" ],
									"id" : "obj-19",
									"comment" : "(int) PITCH:Fine"
								}

							}
, 							{
								"box" : 								{
									"maxclass" : "comment",
									"text" : "Fine",
									"fontsize" : 7.9,
									"numinlets" : 1,
									"patching_rect" : [ 402.0, 47.0, 37.0, 17.0 ],
									"numoutlets" : 0,
									"fontname" : "Arial",
									"frgb" : [ 0.658824, 0.658824, 0.658824, 1.0 ],
									"id" : "obj-20"
								}

							}
, 							{
								"box" : 								{
									"maxclass" : "inlet",
									"numinlets" : 0,
									"patching_rect" : [ 331.0, 47.0, 15.0, 15.0 ],
									"numoutlets" : 1,
									"outlettype" : [ "" ],
									"id" : "obj-21",
									"comment" : "(int) PITCH:Coarse"
								}

							}
, 							{
								"box" : 								{
									"maxclass" : "newobj",
									"text" : "buffer~ oscil tap_help1.aiff -0.1",
									"fontsize" : 7.9,
									"numinlets" : 1,
									"color" : [ 1.0, 0.890196, 0.090196, 1.0 ],
									"patching_rect" : [ 145.0, 93.0, 155.0, 17.0 ],
									"numoutlets" : 2,
									"fontname" : "Arial",
									"outlettype" : [ "float", "bang" ],
									"id" : "obj-22"
								}

							}
, 							{
								"box" : 								{
									"maxclass" : "outlet",
									"numinlets" : 1,
									"patching_rect" : [ 23.0, 283.0, 15.0, 15.0 ],
									"numoutlets" : 0,
									"id" : "obj-23",
									"comment" : "(signal) Left channel"
								}

							}
, 							{
								"box" : 								{
									"maxclass" : "comment",
									"text" : "WAVEFORM",
									"fontsize" : 7.9,
									"numinlets" : 1,
									"patching_rect" : [ 144.0, 81.0, 62.0, 17.0 ],
									"numoutlets" : 0,
									"fontname" : "Arial",
									"frgb" : [ 0.658824, 0.658824, 0.658824, 1.0 ],
									"id" : "obj-24"
								}

							}
, 							{
								"box" : 								{
									"maxclass" : "comment",
									"text" : "Coarse",
									"fontsize" : 7.9,
									"numinlets" : 1,
									"patching_rect" : [ 347.0, 47.0, 38.0, 17.0 ],
									"numoutlets" : 0,
									"fontname" : "Arial",
									"frgb" : [ 0.658824, 0.658824, 0.658824, 1.0 ],
									"id" : "obj-25"
								}

							}
, 							{
								"box" : 								{
									"maxclass" : "comment",
									"text" : "PITCH ADJUSTMENT",
									"fontsize" : 7.9,
									"numinlets" : 1,
									"patching_rect" : [ 336.0, 32.0, 100.0, 17.0 ],
									"numoutlets" : 0,
									"fontname" : "Arial",
									"frgb" : [ 0.658824, 0.658824, 0.658824, 1.0 ],
									"id" : "obj-26"
								}

							}
, 							{
								"box" : 								{
									"maxclass" : "comment",
									"text" : "Pitch adjustment in semi-tones",
									"linecount" : 2,
									"fontsize" : 7.9,
									"numinlets" : 1,
									"patching_rect" : [ 403.0, 131.0, 81.0, 28.0 ],
									"numoutlets" : 0,
									"fontname" : "Arial",
									"frgb" : [ 0.658824, 0.658824, 0.658824, 1.0 ],
									"id" : "obj-27"
								}

							}
, 							{
								"box" : 								{
									"maxclass" : "comment",
									"text" : "NOTES TO PRODUCE",
									"fontsize" : 7.9,
									"numinlets" : 1,
									"patching_rect" : [ 20.0, 27.0, 104.0, 17.0 ],
									"numoutlets" : 0,
									"fontname" : "Arial",
									"frgb" : [ 0.658824, 0.658824, 0.658824, 1.0 ],
									"id" : "obj-28"
								}

							}
 ],
						"lines" : [ 							{
								"patchline" : 								{
									"source" : [ "obj-17", 1 ],
									"destination" : [ "obj-18", 1 ],
									"hidden" : 0
								}

							}
, 							{
								"patchline" : 								{
									"source" : [ "obj-15", 0 ],
									"destination" : [ "obj-17", 0 ],
									"hidden" : 0
								}

							}
, 							{
								"patchline" : 								{
									"source" : [ "obj-19", 0 ],
									"destination" : [ "obj-15", 0 ],
									"hidden" : 0
								}

							}
, 							{
								"patchline" : 								{
									"source" : [ "obj-18", 0 ],
									"destination" : [ "obj-16", 0 ],
									"hidden" : 0
								}

							}
, 							{
								"patchline" : 								{
									"source" : [ "obj-21", 0 ],
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
									"source" : [ "obj-16", 0 ],
									"destination" : [ "obj-10", 1 ],
									"hidden" : 0,
									"midpoints" : [ 378.5, 172.0, 294.5, 172.0 ]
								}

							}
, 							{
								"patchline" : 								{
									"source" : [ "obj-10", 0 ],
									"destination" : [ "obj-1", 0 ],
									"hidden" : 0
								}

							}
, 							{
								"patchline" : 								{
									"source" : [ "obj-7", 3 ],
									"destination" : [ "obj-10", 0 ],
									"hidden" : 0,
									"midpoints" : [ 69.25, 151.0, 281.5, 151.0 ]
								}

							}
, 							{
								"patchline" : 								{
									"source" : [ "obj-16", 0 ],
									"destination" : [ "obj-11", 1 ],
									"hidden" : 0,
									"midpoints" : [ 378.5, 170.0, 211.5, 170.0 ]
								}

							}
, 							{
								"patchline" : 								{
									"source" : [ "obj-11", 0 ],
									"destination" : [ "obj-2", 0 ],
									"hidden" : 0
								}

							}
, 							{
								"patchline" : 								{
									"source" : [ "obj-7", 2 ],
									"destination" : [ "obj-11", 0 ],
									"hidden" : 0,
									"midpoints" : [ 57.0, 153.0, 198.5, 153.0 ]
								}

							}
, 							{
								"patchline" : 								{
									"source" : [ "obj-16", 0 ],
									"destination" : [ "obj-12", 1 ],
									"hidden" : 0,
									"midpoints" : [ 378.5, 168.0, 128.5, 168.0 ]
								}

							}
, 							{
								"patchline" : 								{
									"source" : [ "obj-12", 0 ],
									"destination" : [ "obj-3", 0 ],
									"hidden" : 0
								}

							}
, 							{
								"patchline" : 								{
									"source" : [ "obj-7", 1 ],
									"destination" : [ "obj-12", 0 ],
									"hidden" : 0,
									"midpoints" : [ 44.75, 155.0, 115.5, 155.0 ]
								}

							}
, 							{
								"patchline" : 								{
									"source" : [ "obj-9", 1 ],
									"destination" : [ "obj-6", 1 ],
									"hidden" : 0
								}

							}
, 							{
								"patchline" : 								{
									"source" : [ "obj-16", 0 ],
									"destination" : [ "obj-13", 1 ],
									"hidden" : 0,
									"midpoints" : [ 378.5, 166.0, 40.5, 166.0 ]
								}

							}
, 							{
								"patchline" : 								{
									"source" : [ "obj-5", 0 ],
									"destination" : [ "obj-23", 0 ],
									"hidden" : 0
								}

							}
, 							{
								"patchline" : 								{
									"source" : [ "obj-1", 0 ],
									"destination" : [ "obj-5", 0 ],
									"hidden" : 0
								}

							}
, 							{
								"patchline" : 								{
									"source" : [ "obj-2", 0 ],
									"destination" : [ "obj-5", 0 ],
									"hidden" : 0
								}

							}
, 							{
								"patchline" : 								{
									"source" : [ "obj-3", 0 ],
									"destination" : [ "obj-5", 0 ],
									"hidden" : 0
								}

							}
, 							{
								"patchline" : 								{
									"source" : [ "obj-4", 0 ],
									"destination" : [ "obj-5", 0 ],
									"hidden" : 0
								}

							}
, 							{
								"patchline" : 								{
									"source" : [ "obj-13", 0 ],
									"destination" : [ "obj-4", 0 ],
									"hidden" : 0
								}

							}
, 							{
								"patchline" : 								{
									"source" : [ "obj-7", 0 ],
									"destination" : [ "obj-13", 0 ],
									"hidden" : 0
								}

							}
, 							{
								"patchline" : 								{
									"source" : [ "obj-6", 0 ],
									"destination" : [ "obj-7", 0 ],
									"hidden" : 0
								}

							}
, 							{
								"patchline" : 								{
									"source" : [ "obj-9", 0 ],
									"destination" : [ "obj-6", 0 ],
									"hidden" : 0
								}

							}
, 							{
								"patchline" : 								{
									"source" : [ "obj-8", 0 ],
									"destination" : [ "obj-9", 0 ],
									"hidden" : 0
								}

							}
, 							{
								"patchline" : 								{
									"source" : [ "obj-14", 0 ],
									"destination" : [ "obj-8", 0 ],
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
					"maxclass" : "comment",
					"text" : "<--Original Gain",
					"fontsize" : 7.9,
					"numinlets" : 1,
					"patching_rect" : [ 194.0, 385.0, 110.0, 17.0 ],
					"numoutlets" : 0,
					"fontname" : "Arial",
					"frgb" : [ 0.658824, 0.658824, 0.658824, 1.0 ],
					"id" : "obj-17"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "comment",
					"text" : "C min7 Chord",
					"fontsize" : 7.9,
					"numinlets" : 1,
					"patching_rect" : [ 387.0, 206.0, 78.0, 17.0 ],
					"numoutlets" : 0,
					"fontname" : "Arial",
					"frgb" : [ 0.658824, 0.658824, 0.658824, 1.0 ],
					"id" : "obj-18"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "message",
					"text" : "60, 63, 67, 70",
					"fontsize" : 7.9,
					"numinlets" : 2,
					"patching_rect" : [ 386.0, 221.0, 81.0, 15.0 ],
					"numoutlets" : 1,
					"fontname" : "Arial",
					"outlettype" : [ "" ],
					"id" : "obj-19"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "message",
					"text" : "60, 64, 67, 72",
					"fontsize" : 7.9,
					"numinlets" : 2,
					"patching_rect" : [ 303.0, 221.0, 81.0, 15.0 ],
					"numoutlets" : 1,
					"fontname" : "Arial",
					"outlettype" : [ "" ],
					"id" : "obj-20"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "message",
					"text" : "open",
					"fontsize" : 7.9,
					"numinlets" : 2,
					"patching_rect" : [ 142.0, 460.0, 44.0, 15.0 ],
					"numoutlets" : 1,
					"fontname" : "Arial",
					"outlettype" : [ "" ],
					"id" : "obj-21"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "gain~",
					"orientation" : 2,
					"numinlets" : 2,
					"patching_rect" : [ 143.0, 336.0, 22.0, 77.0 ],
					"numoutlets" : 2,
					"outlettype" : [ "signal", "int" ],
					"inc" : 1.07152,
					"id" : "obj-22"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "newobj",
					"text" : "sfplay~",
					"fontsize" : 7.9,
					"numinlets" : 2,
					"patching_rect" : [ 142.0, 227.0, 55.0, 17.0 ],
					"numoutlets" : 2,
					"fontname" : "Arial",
					"outlettype" : [ "signal", "bang" ],
					"id" : "obj-23",
					"save" : [ "#N", "sfplay~", "", 1, 40320, 0, "", ";" ]
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "gain~",
					"orientation" : 2,
					"numinlets" : 2,
					"patching_rect" : [ 170.0, 336.0, 22.0, 77.0 ],
					"numoutlets" : 2,
					"outlettype" : [ "signal", "int" ],
					"inc" : 1.07152,
					"id" : "obj-24"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "ezdac~",
					"numinlets" : 2,
					"patching_rect" : [ 142.0, 428.0, 33.0, 33.0 ],
					"numoutlets" : 0,
					"id" : "obj-25"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "newobj",
					"text" : "tap.vocoderlite~",
					"fontsize" : 7.9,
					"numinlets" : 4,
					"patching_rect" : [ 212.0, 311.0, 284.0, 17.0 ],
					"numoutlets" : 1,
					"fontname" : "Arial",
					"outlettype" : [ "signal" ],
					"id" : "obj-26"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "comment",
					"text" : "C Maj Chord",
					"fontsize" : 7.9,
					"numinlets" : 1,
					"patching_rect" : [ 303.0, 205.0, 73.0, 17.0 ],
					"numoutlets" : 0,
					"fontname" : "Arial",
					"frgb" : [ 0.658824, 0.658824, 0.658824, 1.0 ],
					"id" : "obj-27"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "comment",
					"text" : "<-Vocoder Gain",
					"fontsize" : 7.9,
					"numinlets" : 1,
					"patching_rect" : [ 200.0, 399.0, 98.0, 17.0 ],
					"numoutlets" : 0,
					"fontname" : "Arial",
					"frgb" : [ 0.658824, 0.658824, 0.658824, 1.0 ],
					"id" : "obj-28"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "comment",
					"text" : "excitation signal",
					"fontsize" : 7.9,
					"numinlets" : 1,
					"patching_rect" : [ 220.0, 285.0, 74.0, 17.0 ],
					"numoutlets" : 0,
					"fontname" : "Arial",
					"frgb" : [ 0.658824, 0.658824, 0.658824, 1.0 ],
					"id" : "obj-29"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "comment",
					"text" : "Q [default 80]",
					"fontsize" : 7.9,
					"numinlets" : 1,
					"patching_rect" : [ 349.0, 276.0, 85.0, 17.0 ],
					"numoutlets" : 0,
					"fontname" : "Arial",
					"frgb" : [ 0.658824, 0.658824, 0.658824, 1.0 ],
					"id" : "obj-30"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "comment",
					"text" : "Response Interval (default 5 ms)",
					"linecount" : 2,
					"fontsize" : 7.9,
					"numinlets" : 1,
					"patching_rect" : [ 419.0, 252.0, 100.0, 28.0 ],
					"numoutlets" : 0,
					"fontname" : "Arial",
					"frgb" : [ 0.658824, 0.658824, 0.658824, 1.0 ],
					"id" : "obj-31"
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
					"id" : "obj-32"
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
					"id" : "obj-33"
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
					"id" : "obj-34"
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
					"id" : "obj-35",
					"textcolor" : [ 0.0, 0.0, 0.0, 1.0 ]
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "comment",
					"text" : "by timothy place",
					"fontsize" : 12.288889,
					"background" : 1,
					"numinlets" : 1,
					"patching_rect" : [ 133.0, 87.0, 125.0, 23.0 ],
					"numoutlets" : 0,
					"fontname" : "Arial",
					"frgb" : [ 0.658824, 0.658824, 0.658824, 1.0 ],
					"id" : "obj-36"
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
					"id" : "obj-37"
				}

			}
 ],
		"lines" : [ 			{
				"patchline" : 				{
					"source" : [ "obj-8", 0 ],
					"destination" : [ "obj-26", 3 ],
					"hidden" : 0
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-7", 0 ],
					"destination" : [ "obj-3", 0 ],
					"hidden" : 1
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-7", 0 ],
					"destination" : [ "obj-4", 0 ],
					"hidden" : 1
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-7", 0 ],
					"destination" : [ "obj-5", 0 ],
					"hidden" : 1
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-12", 0 ],
					"destination" : [ "obj-26", 2 ],
					"hidden" : 0
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-7", 0 ],
					"destination" : [ "obj-6", 0 ],
					"hidden" : 1
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-16", 0 ],
					"destination" : [ "obj-26", 1 ],
					"hidden" : 0
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-14", 0 ],
					"destination" : [ "obj-16", 0 ],
					"hidden" : 0,
					"midpoints" : [ 309.5, 199.0, 472.0, 199.0, 472.0, 245.0, 312.5, 245.0 ]
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-19", 0 ],
					"destination" : [ "obj-16", 0 ],
					"hidden" : 0,
					"midpoints" : [ 395.5, 243.0, 312.5, 243.0 ]
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-20", 0 ],
					"destination" : [ "obj-16", 0 ],
					"hidden" : 0
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
					"source" : [ "obj-35", 0 ],
					"destination" : [ "obj-33", 0 ],
					"hidden" : 1
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-23", 0 ],
					"destination" : [ "obj-26", 0 ],
					"hidden" : 0,
					"midpoints" : [ 151.5, 259.0, 221.5, 259.0 ]
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-24", 1 ],
					"destination" : [ "obj-9", 0 ],
					"hidden" : 0
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-24", 0 ],
					"destination" : [ "obj-25", 1 ],
					"hidden" : 0
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-22", 0 ],
					"destination" : [ "obj-25", 1 ],
					"hidden" : 0
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-26", 0 ],
					"destination" : [ "obj-24", 0 ],
					"hidden" : 0,
					"midpoints" : [ 221.5, 331.0, 179.0, 331.0 ]
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-21", 0 ],
					"destination" : [ "obj-25", 0 ],
					"hidden" : 1,
					"midpoints" : [ 151.5, 477.0, 135.0, 477.0, 135.0, 423.0, 151.5, 423.0 ]
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-24", 0 ],
					"destination" : [ "obj-25", 0 ],
					"hidden" : 0
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-22", 0 ],
					"destination" : [ "obj-25", 0 ],
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
					"source" : [ "obj-10", 0 ],
					"destination" : [ "obj-23", 0 ],
					"hidden" : 0,
					"midpoints" : [ 191.0, 221.0, 151.5, 221.0 ]
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-11", 0 ],
					"destination" : [ "obj-23", 0 ],
					"hidden" : 0
				}

			}
 ]
	}

}
