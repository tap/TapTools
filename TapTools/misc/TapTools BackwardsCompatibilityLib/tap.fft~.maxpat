{
	"patcher" : 	{
		"rect" : [ 49.0, 40.0, 677.0, 420.0 ],
		"bgcolor" : [ 0.1, 0.1, 0.1, 1.0 ],
		"bglocked" : 0,
		"defrect" : [ 49.0, 40.0, 677.0, 420.0 ],
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
					"patching_rect" : [ 211.0, 43.0, 92.0, 17.0 ],
					"numoutlets" : 8,
					"fontname" : "Geneva",
					"outlettype" : [ "signal", "bang", "int", "float", "list", "", "list", "" ],
					"id" : "obj-1"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "newobj",
					"text" : "buffer~ #0wind 11.61",
					"fontsize" : 7.9,
					"numinlets" : 1,
					"patching_rect" : [ 255.0, 318.0, 109.0, 17.0 ],
					"numoutlets" : 2,
					"fontname" : "Geneva",
					"outlettype" : [ "float", "bang" ],
					"id" : "obj-2"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "newobj",
					"text" : "patcher genWind",
					"fontsize" : 7.9,
					"numinlets" : 1,
					"patching_rect" : [ 255.0, 275.0, 81.0, 17.0 ],
					"numoutlets" : 1,
					"fontname" : "Geneva",
					"outlettype" : [ "" ],
					"id" : "obj-3",
					"patcher" : 					{
						"rect" : [ 10.0, 100.0, 405.0, 421.0 ],
						"bgcolor" : [ 0.1, 0.1, 0.1, 1.0 ],
						"bglocked" : 0,
						"defrect" : [ 10.0, 100.0, 405.0, 421.0 ],
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
									"maxclass" : "comment",
									"text" : "hanning",
									"fontsize" : 7.9,
									"numinlets" : 1,
									"patching_rect" : [ 174.0, 213.0, 50.0, 17.0 ],
									"numoutlets" : 0,
									"fontname" : "Geneva",
									"frgb" : [ 0.658824, 0.658824, 0.658824, 1.0 ],
									"id" : "obj-1"
								}

							}
, 							{
								"box" : 								{
									"maxclass" : "newobj",
									"text" : "expr sqrt($f1)",
									"fontsize" : 7.9,
									"numinlets" : 1,
									"patching_rect" : [ 97.0, 248.0, 75.0, 17.0 ],
									"numoutlets" : 1,
									"fontname" : "Geneva",
									"outlettype" : [ "" ],
									"id" : "obj-2"
								}

							}
, 							{
								"box" : 								{
									"maxclass" : "newobj",
									"text" : "\/ 511.",
									"fontsize" : 7.9,
									"numinlets" : 2,
									"patching_rect" : [ 95.0, 163.0, 38.0, 17.0 ],
									"numoutlets" : 1,
									"fontname" : "Geneva",
									"outlettype" : [ "float" ],
									"id" : "obj-3"
								}

							}
, 							{
								"box" : 								{
									"maxclass" : "newobj",
									"text" : "expr 0.5 * (1 + cos((3.14159 + 3.14159*2* $f1)))",
									"fontsize" : 7.9,
									"numinlets" : 1,
									"patching_rect" : [ 97.0, 191.0, 278.0, 17.0 ],
									"numoutlets" : 1,
									"fontname" : "Geneva",
									"outlettype" : [ "" ],
									"id" : "obj-4"
								}

							}
, 							{
								"box" : 								{
									"maxclass" : "newobj",
									"text" : "Uzi 512",
									"fontsize" : 7.9,
									"numinlets" : 2,
									"patching_rect" : [ 74.0, 109.0, 43.0, 17.0 ],
									"numoutlets" : 3,
									"fontname" : "Geneva",
									"outlettype" : [ "bang", "bang", "int" ],
									"id" : "obj-5"
								}

							}
, 							{
								"box" : 								{
									"maxclass" : "newobj",
									"text" : "counter 0 511",
									"fontsize" : 7.9,
									"numinlets" : 5,
									"patching_rect" : [ 74.0, 134.0, 72.0, 17.0 ],
									"numoutlets" : 4,
									"fontname" : "Geneva",
									"outlettype" : [ "int", "", "", "int" ],
									"id" : "obj-6",
									"save" : [ "#N", "counter", 0, 511, ";", "#X", "flags", 0, 0, ";" ]
								}

							}
, 							{
								"box" : 								{
									"maxclass" : "button",
									"numinlets" : 1,
									"patching_rect" : [ 74.0, 85.0, 15.0, 15.0 ],
									"numoutlets" : 1,
									"outlettype" : [ "bang" ],
									"id" : "obj-7"
								}

							}
, 							{
								"box" : 								{
									"maxclass" : "comment",
									"text" : "Sqrt for two Overlap",
									"fontsize" : 7.9,
									"numinlets" : 1,
									"patching_rect" : [ 178.0, 251.0, 87.0, 17.0 ],
									"numoutlets" : 0,
									"fontname" : "Geneva",
									"frgb" : [ 0.658824, 0.658824, 0.658824, 1.0 ],
									"id" : "obj-8"
								}

							}
, 							{
								"box" : 								{
									"maxclass" : "inlet",
									"numinlets" : 0,
									"patching_rect" : [ 74.0, 64.0, 15.0, 15.0 ],
									"numoutlets" : 1,
									"outlettype" : [ "bang" ],
									"id" : "obj-9",
									"comment" : ""
								}

							}
, 							{
								"box" : 								{
									"maxclass" : "newobj",
									"text" : "pack 0 0.",
									"fontsize" : 7.9,
									"numinlets" : 2,
									"patching_rect" : [ 74.0, 289.0, 49.0, 17.0 ],
									"numoutlets" : 1,
									"fontname" : "Geneva",
									"outlettype" : [ "" ],
									"id" : "obj-10"
								}

							}
, 							{
								"box" : 								{
									"maxclass" : "outlet",
									"numinlets" : 1,
									"patching_rect" : [ 74.0, 323.0, 15.0, 15.0 ],
									"numoutlets" : 0,
									"id" : "obj-11",
									"comment" : ""
								}

							}
 ],
						"lines" : [ 							{
								"patchline" : 								{
									"source" : [ "obj-2", 0 ],
									"destination" : [ "obj-10", 1 ],
									"hidden" : 0
								}

							}
, 							{
								"patchline" : 								{
									"source" : [ "obj-4", 0 ],
									"destination" : [ "obj-2", 0 ],
									"hidden" : 0
								}

							}
, 							{
								"patchline" : 								{
									"source" : [ "obj-3", 0 ],
									"destination" : [ "obj-4", 0 ],
									"hidden" : 0
								}

							}
, 							{
								"patchline" : 								{
									"source" : [ "obj-6", 0 ],
									"destination" : [ "obj-3", 0 ],
									"hidden" : 0
								}

							}
, 							{
								"patchline" : 								{
									"source" : [ "obj-10", 0 ],
									"destination" : [ "obj-11", 0 ],
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
									"source" : [ "obj-7", 0 ],
									"destination" : [ "obj-5", 0 ],
									"hidden" : 0
								}

							}
, 							{
								"patchline" : 								{
									"source" : [ "obj-9", 0 ],
									"destination" : [ "obj-7", 0 ],
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
					"text" : "peek~ #0wind",
					"fontsize" : 7.9,
					"numinlets" : 3,
					"patching_rect" : [ 255.0, 296.0, 109.0, 17.0 ],
					"numoutlets" : 1,
					"fontname" : "Geneva",
					"outlettype" : [ "float" ],
					"id" : "obj-4"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "newobj",
					"text" : "loadbang",
					"fontsize" : 7.9,
					"numinlets" : 1,
					"patching_rect" : [ 255.0, 255.0, 45.0, 17.0 ],
					"numoutlets" : 1,
					"fontname" : "Geneva",
					"outlettype" : [ "bang" ],
					"id" : "obj-5"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "newobj",
					"text" : "\/~ #1",
					"fontsize" : 7.9,
					"numinlets" : 2,
					"patching_rect" : [ 484.0, 209.0, 35.0, 17.0 ],
					"numoutlets" : 1,
					"fontname" : "Geneva",
					"outlettype" : [ "signal" ],
					"id" : "obj-6"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "newobj",
					"text" : "send~ #0windowB",
					"fontsize" : 7.9,
					"numinlets" : 1,
					"patching_rect" : [ 432.0, 274.0, 90.0, 17.0 ],
					"numoutlets" : 0,
					"fontname" : "Geneva",
					"id" : "obj-7"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "newobj",
					"text" : "send~ #0windowA",
					"fontsize" : 7.9,
					"numinlets" : 1,
					"color" : [ 0.380392, 0.611765, 0.611765, 1.0 ],
					"patching_rect" : [ 79.0, 273.0, 91.0, 17.0 ],
					"numoutlets" : 0,
					"fontname" : "Geneva",
					"id" : "obj-8"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "newobj",
					"text" : "cycle~ #0wind",
					"fontsize" : 7.9,
					"numinlets" : 2,
					"patching_rect" : [ 79.0, 236.0, 64.0, 17.0 ],
					"numoutlets" : 1,
					"fontname" : "Geneva",
					"outlettype" : [ "signal" ],
					"id" : "obj-9"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "newobj",
					"text" : "cycle~ #0wind",
					"fontsize" : 7.9,
					"numinlets" : 2,
					"patching_rect" : [ 432.0, 237.0, 64.0, 17.0 ],
					"numoutlets" : 1,
					"fontname" : "Geneva",
					"outlettype" : [ "signal" ],
					"id" : "obj-10"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "comment",
					"text" : "window function",
					"linecount" : 2,
					"fontsize" : 7.9,
					"numinlets" : 1,
					"patching_rect" : [ 142.0, 238.0, 47.0, 28.0 ],
					"numoutlets" : 0,
					"fontname" : "Geneva",
					"frgb" : [ 0.658824, 0.658824, 0.658824, 1.0 ],
					"id" : "obj-11"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "comment",
					"text" : "window function (sync'd to half frame)",
					"linecount" : 2,
					"fontsize" : 7.9,
					"numinlets" : 1,
					"patching_rect" : [ 496.0, 239.0, 109.0, 28.0 ],
					"numoutlets" : 0,
					"fontname" : "Geneva",
					"frgb" : [ 0.658824, 0.658824, 0.658824, 1.0 ],
					"id" : "obj-12"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "newobj",
					"text" : "\/~ #1",
					"fontsize" : 7.9,
					"numinlets" : 2,
					"patching_rect" : [ 131.0, 207.0, 35.0, 17.0 ],
					"numoutlets" : 1,
					"fontname" : "Geneva",
					"outlettype" : [ "signal" ],
					"id" : "obj-13"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "outlet",
					"numinlets" : 1,
					"patching_rect" : [ 614.0, 348.0, 15.0, 15.0 ],
					"numoutlets" : 0,
					"id" : "obj-14",
					"comment" : ""
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "outlet",
					"numinlets" : 1,
					"patching_rect" : [ 630.0, 348.0, 15.0, 15.0 ],
					"numoutlets" : 0,
					"id" : "obj-15",
					"comment" : ""
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "outlet",
					"numinlets" : 1,
					"patching_rect" : [ 417.0, 123.0, 15.0, 15.0 ],
					"numoutlets" : 0,
					"id" : "obj-16",
					"comment" : "FFT2-Imaginary"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "outlet",
					"numinlets" : 1,
					"patching_rect" : [ 77.0, 124.0, 15.0, 15.0 ],
					"numoutlets" : 0,
					"id" : "obj-17",
					"comment" : "FFT1-Imaginary"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "outlet",
					"numinlets" : 1,
					"patching_rect" : [ 350.0, 123.0, 15.0, 15.0 ],
					"numoutlets" : 0,
					"id" : "obj-18",
					"comment" : "FFT2-Real"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "outlet",
					"numinlets" : 1,
					"patching_rect" : [ 24.0, 124.0, 15.0, 15.0 ],
					"numoutlets" : 0,
					"id" : "obj-19",
					"comment" : "FFT1-Real"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "comment",
					"text" : "FFT out 1",
					"fontsize" : 7.9,
					"numinlets" : 1,
					"patching_rect" : [ 32.0, 141.0, 48.0, 17.0 ],
					"numoutlets" : 0,
					"fontname" : "Geneva",
					"frgb" : [ 0.658824, 0.658824, 0.658824, 1.0 ],
					"id" : "obj-20"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "comment",
					"text" : "FFT out 2",
					"fontsize" : 7.9,
					"numinlets" : 1,
					"patching_rect" : [ 367.0, 125.0, 48.0, 17.0 ],
					"numoutlets" : 0,
					"fontname" : "Geneva",
					"frgb" : [ 0.658824, 0.658824, 0.658824, 1.0 ],
					"id" : "obj-21"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "newobj",
					"text" : "receive~ #0windowB",
					"fontsize" : 7.9,
					"numinlets" : 1,
					"patching_rect" : [ 367.0, 24.0, 104.0, 17.0 ],
					"numoutlets" : 1,
					"fontname" : "Geneva",
					"outlettype" : [ "signal" ],
					"id" : "obj-22"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "newobj",
					"text" : "receive~ #0windowA",
					"fontsize" : 7.9,
					"numinlets" : 1,
					"color" : [ 0.380392, 0.611765, 0.611765, 1.0 ],
					"patching_rect" : [ 24.0, 24.0, 105.0, 17.0 ],
					"numoutlets" : 1,
					"fontname" : "Geneva",
					"outlettype" : [ "signal" ],
					"id" : "obj-23"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "comment",
					"text" : "(signal) Input",
					"fontsize" : 7.9,
					"numinlets" : 1,
					"patching_rect" : [ 228.0, 24.0, 77.0, 17.0 ],
					"numoutlets" : 0,
					"fontname" : "Geneva",
					"frgb" : [ 0.658824, 0.658824, 0.658824, 1.0 ],
					"id" : "obj-24"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "inlet",
					"numinlets" : 0,
					"patching_rect" : [ 211.0, 24.0, 15.0, 15.0 ],
					"numoutlets" : 1,
					"outlettype" : [ "" ],
					"id" : "obj-25",
					"comment" : "(signal) Input"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "newobj",
					"text" : "*~",
					"fontsize" : 7.9,
					"numinlets" : 2,
					"patching_rect" : [ 350.0, 76.0, 27.0, 17.0 ],
					"numoutlets" : 1,
					"fontname" : "Geneva",
					"outlettype" : [ "signal" ],
					"id" : "obj-26"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "newobj",
					"text" : "*~",
					"fontsize" : 7.9,
					"numinlets" : 2,
					"patching_rect" : [ 24.0, 76.0, 27.0, 17.0 ],
					"numoutlets" : 1,
					"fontname" : "Geneva",
					"outlettype" : [ "signal" ],
					"id" : "obj-27"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "newobj",
					"text" : "fft~ #1 #1 0",
					"fontsize" : 7.9,
					"numinlets" : 2,
					"color" : [ 0.4, 0.4, 0.8, 1.0 ],
					"patching_rect" : [ 24.0, 99.0, 116.0, 17.0 ],
					"numoutlets" : 3,
					"fontname" : "Geneva",
					"outlettype" : [ "signal", "signal", "signal" ],
					"id" : "obj-28"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "newobj",
					"text" : "fft~ #1 #1 #2",
					"fontsize" : 7.9,
					"numinlets" : 2,
					"color" : [ 0.4, 0.4, 0.8, 1.0 ],
					"patching_rect" : [ 350.0, 99.0, 145.0, 17.0 ],
					"numoutlets" : 3,
					"fontname" : "Geneva",
					"outlettype" : [ "signal", "signal", "signal" ],
					"id" : "obj-29"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "comment",
					"text" : "apply window",
					"fontsize" : 7.9,
					"numinlets" : 1,
					"patching_rect" : [ 55.0, 76.0, 73.0, 17.0 ],
					"numoutlets" : 0,
					"fontname" : "Geneva",
					"frgb" : [ 0.658824, 0.658824, 0.658824, 1.0 ],
					"id" : "obj-30"
				}

			}
 ],
		"lines" : [ 			{
				"patchline" : 				{
					"source" : [ "obj-29", 2 ],
					"destination" : [ "obj-15", 0 ],
					"hidden" : 0,
					"midpoints" : [ 485.5, 189.0, 639.0, 189.0 ]
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-28", 2 ],
					"destination" : [ "obj-14", 0 ],
					"hidden" : 0,
					"midpoints" : [ 130.5, 193.0, 623.0, 193.0 ]
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-6", 0 ],
					"destination" : [ "obj-10", 1 ],
					"hidden" : 0
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-29", 2 ],
					"destination" : [ "obj-6", 0 ],
					"hidden" : 0
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-10", 0 ],
					"destination" : [ "obj-7", 0 ],
					"hidden" : 0
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-29", 1 ],
					"destination" : [ "obj-16", 0 ],
					"hidden" : 0
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-22", 0 ],
					"destination" : [ "obj-26", 1 ],
					"hidden" : 0
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-29", 0 ],
					"destination" : [ "obj-18", 0 ],
					"hidden" : 0
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-26", 0 ],
					"destination" : [ "obj-29", 0 ],
					"hidden" : 0
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-1", 0 ],
					"destination" : [ "obj-26", 0 ],
					"hidden" : 0,
					"midpoints" : [ 220.5, 66.0, 359.5, 66.0 ]
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-3", 0 ],
					"destination" : [ "obj-4", 0 ],
					"hidden" : 0
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-5", 0 ],
					"destination" : [ "obj-3", 0 ],
					"hidden" : 0
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-25", 0 ],
					"destination" : [ "obj-1", 0 ],
					"hidden" : 0
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-13", 0 ],
					"destination" : [ "obj-9", 1 ],
					"hidden" : 0
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-28", 2 ],
					"destination" : [ "obj-13", 0 ],
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
					"source" : [ "obj-28", 1 ],
					"destination" : [ "obj-17", 0 ],
					"hidden" : 0
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-1", 0 ],
					"destination" : [ "obj-27", 1 ],
					"hidden" : 0,
					"midpoints" : [ 220.5, 66.0, 41.5, 66.0 ]
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-28", 0 ],
					"destination" : [ "obj-19", 0 ],
					"hidden" : 0
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-27", 0 ],
					"destination" : [ "obj-28", 0 ],
					"hidden" : 0
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-23", 0 ],
					"destination" : [ "obj-27", 0 ],
					"hidden" : 0
				}

			}
 ]
	}

}
