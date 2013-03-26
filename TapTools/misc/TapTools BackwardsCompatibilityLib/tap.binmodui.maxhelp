{
	"patcher" : 	{
		"rect" : [ 68.0, 186.0, 951.0, 547.0 ],
		"bgcolor" : [ 0.1, 0.1, 0.1, 1.0 ],
		"bglocked" : 0,
		"defrect" : [ 68.0, 186.0, 951.0, 547.0 ],
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
					"text" : "print",
					"fontsize" : 7.9,
					"numinlets" : 1,
					"patching_rect" : [ 585.0, 480.0, 31.0, 17.0 ],
					"numoutlets" : 0,
					"fontname" : "Arial",
					"id" : "obj-1"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "newobj",
					"varname" : "u497000202",
					"text" : "autopattr",
					"fontsize" : 7.9,
					"numinlets" : 1,
					"patching_rect" : [ 425.0, 457.0, 53.0, 17.0 ],
					"numoutlets" : 4,
					"fontname" : "Arial",
					"outlettype" : [ "", "", "", "" ],
					"id" : "obj-2",
					"save2" : [ "#X", "prestore", "depth", 84, 0, 2, 0.55294, 0.70588, 0.49412, 0.2, 0.1, 0.5, 0.2, 1, 0.2, 0.6, 0.3, 1, 0, 1, 0, -0.072626, -0.085714, 0.5, 64, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, ";", "#X", "prestore", "depth", 84, 40, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, ";", "#X", "prestore", "depth", 84, 80, 0.5, 0.5, 0.5, 0.5, ";", "#X", "prestore", "frequency", 84, 0, 2, 0.55294, 0.70588, 0.49412, 0.2, 0.1, 0.5, 0.2, 1, 0.2, 0.6, 0.3, 1, 2, 1, 0, -0.888268, 1, 0.5, 64, 1, 0.007047, 0.007047, 1, 1, 0.021142, 0.021142, 1, 1, 0.035237, 1, 1, 0.049332, 0.049332, 1, 1, 0.063427, 0.063427, 1, 0.077522, ";", "#X", "prestore", "frequency", 84, 40, 0.077522, 1, 1, 0.091617, 0.091617, 1, 1, 0.105712, 1, 1, 0.119807, 0.119807, 1, 1, 0.133902, 0.133902, 1, 0.147997, 0.147997, 1, 1, 0.162092, 0.162092, 1, 1, 0.176187, 1, 1, 0.190282, 0.190282, 1, 1, 0.204377, 0.204377, 1, 0.218472, 0.218472, 1, 1, 0.232567, ";", "#X", "prestore", "frequency", 84, 80, 0.232567, 1, 1, 0.246662, ";" ]
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "message",
					"text" : "compile",
					"fontsize" : 7.9,
					"numinlets" : 2,
					"patching_rect" : [ 442.0, 379.0, 43.0, 15.0 ],
					"numoutlets" : 1,
					"fontname" : "Arial",
					"outlettype" : [ "" ],
					"id" : "obj-3"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "number",
					"htextcolor" : [ 0.870588, 0.870588, 0.870588, 1.0 ],
					"bgcolor" : [ 0.866667, 0.866667, 0.866667, 1.0 ],
					"fontsize" : 7.9,
					"numinlets" : 1,
					"patching_rect" : [ 694.0, 428.0, 35.0, 17.0 ],
					"numoutlets" : 2,
					"fontname" : "Arial",
					"outlettype" : [ "int", "bang" ],
					"id" : "obj-4",
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
					"patching_rect" : [ 642.0, 428.0, 35.0, 17.0 ],
					"numoutlets" : 2,
					"fontname" : "Arial",
					"outlettype" : [ "int", "bang" ],
					"id" : "obj-5",
					"triscale" : 0.9,
					"textcolor" : [ 0.0, 0.0, 0.0, 1.0 ]
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "message",
					"text" : "recall $1",
					"fontsize" : 7.9,
					"numinlets" : 2,
					"patching_rect" : [ 642.0, 446.0, 50.0, 15.0 ],
					"numoutlets" : 1,
					"fontname" : "Arial",
					"outlettype" : [ "" ],
					"id" : "obj-6"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "message",
					"text" : "store $1",
					"fontsize" : 7.9,
					"numinlets" : 2,
					"patching_rect" : [ 694.0, 446.0, 48.0, 15.0 ],
					"numoutlets" : 1,
					"fontname" : "Arial",
					"outlettype" : [ "" ],
					"id" : "obj-7"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "newobj",
					"varname" : "u603000305",
					"text" : "pattrstorage @autorestore 0 @outputmode 1 @savemode 0",
					"fontsize" : 7.9,
					"numinlets" : 1,
					"patching_rect" : [ 642.0, 471.0, 274.0, 17.0 ],
					"numoutlets" : 1,
					"fontname" : "Arial",
					"outlettype" : [ "" ],
					"id" : "obj-8",
					"saved_object_attributes" : 					{
						"client_rect" : [ 4, 44, 990, 244 ],
						"storage_rect" : [ 0, 0, 640, 240 ]
					}

				}

			}
, 			{
				"box" : 				{
					"maxclass" : "newobj",
					"text" : "prepend controlmode",
					"fontsize" : 7.9,
					"numinlets" : 1,
					"patching_rect" : [ 432.0, 192.0, 102.0, 17.0 ],
					"numoutlets" : 1,
					"fontname" : "Arial",
					"outlettype" : [ "" ],
					"id" : "obj-9"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "umenu",
					"labelclick" : 1,
					"fontsize" : 7.9,
					"numinlets" : 1,
					"items" : [ "curve", ",", "square", ",", "clickable" ],
					"arrowlink" : 1,
					"patching_rect" : [ 433.0, 171.0, 79.0, 17.0 ],
					"numoutlets" : 3,
					"types" : [  ],
					"fontname" : "Arial",
					"outlettype" : [ "int", "", "" ],
					"id" : "obj-10"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "jsui",
					"varname" : "depth",
					"numinlets" : 6,
					"patching_rect" : [ 358.0, 214.0, 179.0, 105.0 ],
					"numoutlets" : 6,
					"outlettype" : [ "", "", "", "", "", "" ],
					"id" : "obj-11",
					"jsarguments" : [  ],
					"filename" : "tap.binmodui.js"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "newobj",
					"text" : "prepend controlmode",
					"fontsize" : 7.9,
					"numinlets" : 1,
					"patching_rect" : [ 177.0, 192.0, 102.0, 17.0 ],
					"numoutlets" : 1,
					"fontname" : "Arial",
					"outlettype" : [ "" ],
					"id" : "obj-12"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "comment",
					"text" : "choose a mode for controlling the values in the editor.",
					"linecount" : 2,
					"fontsize" : 7.9,
					"numinlets" : 1,
					"patching_rect" : [ 266.0, 162.0, 159.0, 28.0 ],
					"numoutlets" : 0,
					"fontname" : "Arial",
					"frgb" : [ 0.658824, 0.658824, 0.658824, 1.0 ],
					"id" : "obj-13"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "umenu",
					"labelclick" : 1,
					"fontsize" : 7.9,
					"numinlets" : 1,
					"items" : [ "curve", ",", "square", ",", "clickable" ],
					"arrowlink" : 1,
					"patching_rect" : [ 178.0, 171.0, 79.0, 17.0 ],
					"numoutlets" : 3,
					"types" : [  ],
					"fontname" : "Arial",
					"outlettype" : [ "int", "", "" ],
					"id" : "obj-14"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "comment",
					"text" : "tap.binmodui provides the list of values for each bin - you prepend the parameter name you want the values to control...",
					"linecount" : 2,
					"fontsize" : 7.9,
					"numinlets" : 1,
					"patching_rect" : [ 187.0, 333.0, 307.0, 28.0 ],
					"numoutlets" : 0,
					"fontname" : "Arial",
					"frgb" : [ 0.658824, 0.658824, 0.658824, 1.0 ],
					"id" : "obj-15"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "newobj",
					"text" : "prepend depth",
					"fontsize" : 7.9,
					"numinlets" : 1,
					"patching_rect" : [ 277.0, 364.0, 71.0, 17.0 ],
					"numoutlets" : 1,
					"fontname" : "Arial",
					"outlettype" : [ "" ],
					"id" : "obj-16"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "newobj",
					"text" : "prepend phase",
					"fontsize" : 7.9,
					"numinlets" : 1,
					"patching_rect" : [ 355.0, 364.0, 72.0, 17.0 ],
					"numoutlets" : 1,
					"fontname" : "Arial",
					"outlettype" : [ "" ],
					"id" : "obj-17"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "newobj",
					"text" : "prepend frequency",
					"fontsize" : 7.9,
					"numinlets" : 1,
					"patching_rect" : [ 177.0, 364.0, 92.0, 17.0 ],
					"numoutlets" : 1,
					"fontname" : "Arial",
					"outlettype" : [ "" ],
					"id" : "obj-18"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "jsui",
					"varname" : "frequency",
					"numinlets" : 6,
					"patching_rect" : [ 177.0, 214.0, 179.0, 105.0 ],
					"numoutlets" : 6,
					"outlettype" : [ "", "", "", "", "", "" ],
					"id" : "obj-19",
					"jsarguments" : [  ],
					"filename" : "tap.binmodui.js"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "newobj",
					"text" : "tap.thru",
					"fontsize" : 7.9,
					"numinlets" : 1,
					"patching_rect" : [ 329.0, 443.0, 45.0, 17.0 ],
					"numoutlets" : 1,
					"fontname" : "Arial",
					"outlettype" : [ "" ],
					"id" : "obj-20"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "button",
					"numinlets" : 1,
					"patching_rect" : [ 560.0, 210.0, 15.0, 15.0 ],
					"numoutlets" : 1,
					"outlettype" : [ "bang" ],
					"id" : "obj-21"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "newobj",
					"text" : "- 1.",
					"fontsize" : 7.9,
					"numinlets" : 2,
					"patching_rect" : [ 592.0, 255.0, 32.0, 17.0 ],
					"numoutlets" : 1,
					"fontname" : "Arial",
					"outlettype" : [ "float" ],
					"id" : "obj-22"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "newobj",
					"text" : "pack 0 0.",
					"fontsize" : 7.9,
					"numinlets" : 2,
					"patching_rect" : [ 592.0, 276.0, 50.0, 17.0 ],
					"numoutlets" : 1,
					"fontname" : "Arial",
					"outlettype" : [ "" ],
					"id" : "obj-23"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "newobj",
					"text" : "t 0.",
					"fontsize" : 7.9,
					"numinlets" : 1,
					"patching_rect" : [ 632.0, 255.0, 31.0, 17.0 ],
					"numoutlets" : 1,
					"fontname" : "Arial",
					"outlettype" : [ "float" ],
					"id" : "obj-24"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "newobj",
					"text" : "uzi 512",
					"fontsize" : 7.9,
					"numinlets" : 2,
					"patching_rect" : [ 560.0, 229.0, 43.0, 17.0 ],
					"numoutlets" : 3,
					"fontname" : "Arial",
					"outlettype" : [ "bang", "bang", "int" ],
					"id" : "obj-25"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "message",
					"text" : "depth $1 $2",
					"fontsize" : 7.9,
					"numinlets" : 2,
					"patching_rect" : [ 552.0, 302.0, 63.0, 15.0 ],
					"numoutlets" : 1,
					"fontname" : "Arial",
					"outlettype" : [ "" ],
					"id" : "obj-26"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "message",
					"text" : "frequency $1 $2",
					"fontsize" : 7.9,
					"numinlets" : 2,
					"patching_rect" : [ 631.0, 302.0, 84.0, 15.0 ],
					"numoutlets" : 1,
					"fontname" : "Arial",
					"outlettype" : [ "" ],
					"id" : "obj-27"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "comment",
					"text" : "The hard way: manually zeroing the phase and depth attributes",
					"linecount" : 2,
					"fontsize" : 7.9,
					"numinlets" : 1,
					"patching_rect" : [ 568.0, 172.0, 159.0, 28.0 ],
					"numoutlets" : 0,
					"fontname" : "Arial",
					"frgb" : [ 0.658824, 0.658824, 0.658824, 1.0 ],
					"id" : "obj-28"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "panel",
					"bgcolor" : [ 0.74902, 0.74902, 0.74902, 1.0 ],
					"numinlets" : 1,
					"patching_rect" : [ 540.0, 161.0, 205.0, 169.0 ],
					"numoutlets" : 0,
					"rounded" : 0,
					"id" : "obj-29",
					"border" : 1
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "toggle",
					"numinlets" : 1,
					"patching_rect" : [ 69.0, 199.0, 13.0, 13.0 ],
					"numoutlets" : 1,
					"outlettype" : [ "int" ],
					"id" : "obj-30"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "toggle",
					"numinlets" : 1,
					"patching_rect" : [ 24.0, 199.0, 13.0, 13.0 ],
					"numoutlets" : 1,
					"outlettype" : [ "int" ],
					"id" : "obj-31"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "newobj",
					"text" : "tap.elixir~",
					"fontsize" : 7.9,
					"numinlets" : 2,
					"patching_rect" : [ 13.0, 216.0, 55.0, 17.0 ],
					"numoutlets" : 2,
					"fontname" : "Arial",
					"outlettype" : [ "signal", "" ],
					"id" : "obj-32"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "newobj",
					"text" : "noise~",
					"fontsize" : 7.9,
					"numinlets" : 1,
					"patching_rect" : [ 58.0, 179.0, 38.0, 17.0 ],
					"numoutlets" : 1,
					"fontname" : "Arial",
					"outlettype" : [ "signal" ],
					"id" : "obj-33"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "comment",
					"text" : "The tap.fft.binmodulator~ object contains a bank of up to 512 individual LFOs internally, which are then applied to each of the bins of an FFT. In order to function properly the object must be used inside of a pfft~ patcher.",
					"linecount" : 4,
					"fontsize" : 7.9,
					"numinlets" : 1,
					"patching_rect" : [ 586.0, 37.0, 309.0, 49.0 ],
					"numoutlets" : 0,
					"fontname" : "Arial",
					"frgb" : [ 0.87451, 0.87451, 0.87451, 1.0 ],
					"id" : "obj-34",
					"textcolor" : [ 0.87451, 0.87451, 0.87451, 1.0 ]
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "newobj",
					"text" : "print dumpout",
					"fontsize" : 7.9,
					"numinlets" : 1,
					"patching_rect" : [ 51.0, 297.0, 71.0, 17.0 ],
					"numoutlets" : 0,
					"fontname" : "Arial",
					"id" : "obj-35"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "newobj",
					"text" : "tap.limi~ @preamp 12",
					"fontsize" : 7.9,
					"numinlets" : 3,
					"patching_rect" : [ 13.0, 332.0, 108.0, 17.0 ],
					"numoutlets" : 3,
					"fontname" : "Arial",
					"outlettype" : [ "signal", "signal", "" ],
					"id" : "obj-36"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "newobj",
					"text" : "pfft~ blue.pfft.binmodulator2~ 1024 4",
					"linecount" : 3,
					"fontsize" : 7.9,
					"numinlets" : 1,
					"patching_rect" : [ 13.0, 242.0, 110.0, 38.0 ],
					"numoutlets" : 2,
					"fontname" : "Arial",
					"outlettype" : [ "signal", "" ],
					"id" : "obj-37"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "number",
					"htextcolor" : [ 0.870588, 0.870588, 0.870588, 1.0 ],
					"bgcolor" : [ 0.866667, 0.866667, 0.866667, 1.0 ],
					"fontsize" : 7.9,
					"numinlets" : 1,
					"patching_rect" : [ 54.0, 402.0, 35.0, 17.0 ],
					"triangle" : 0,
					"transparent" : 1,
					"numoutlets" : 2,
					"fontname" : "Arial",
					"outlettype" : [ "int", "bang" ],
					"id" : "obj-38",
					"triscale" : 0.9,
					"textcolor" : [ 0.0, 0.0, 0.0, 1.0 ]
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "meter~",
					"warmcolor" : [ 0.85098, 0.85098, 0.0, 1.0 ],
					"bgcolor" : [ 0.164706, 0.164706, 0.164706, 1.0 ],
					"numinlets" : 1,
					"patching_rect" : [ 40.0, 355.0, 13.0, 65.0 ],
					"tepidcolor" : [ 0.6, 0.729412, 0.0, 1.0 ],
					"numoutlets" : 1,
					"outlettype" : [ "float" ],
					"interval" : 100,
					"numleds" : 20,
					"id" : "obj-39",
					"coldcolor" : [ 0.0, 0.658824, 0.0, 1.0 ]
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "toggle",
					"numinlets" : 1,
					"patching_rect" : [ 62.0, 135.0, 15.0, 15.0 ],
					"numoutlets" : 1,
					"outlettype" : [ "int" ],
					"id" : "obj-40"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "message",
					"text" : "open",
					"fontsize" : 7.9,
					"numinlets" : 2,
					"patching_rect" : [ 13.0, 481.0, 29.0, 15.0 ],
					"numoutlets" : 1,
					"fontname" : "Arial",
					"outlettype" : [ "" ],
					"id" : "obj-41"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "message",
					"text" : "loop $1",
					"fontsize" : 7.9,
					"numinlets" : 2,
					"patching_rect" : [ 31.0, 159.0, 42.0, 15.0 ],
					"numoutlets" : 1,
					"fontname" : "Arial",
					"outlettype" : [ "" ],
					"id" : "obj-42"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "comment",
					"text" : "idiosyncratic ui for tap.fft.binmodulator",
					"fontsize" : 10.533333,
					"numinlets" : 1,
					"patching_rect" : [ 133.0, 34.0, 309.0, 21.0 ],
					"numoutlets" : 0,
					"fontname" : "Arial",
					"frgb" : [ 0.87451, 0.87451, 0.87451, 1.0 ],
					"id" : "obj-43",
					"textcolor" : [ 0.87451, 0.87451, 0.87451, 1.0 ]
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "comment",
					"text" : "tap.binmodui",
					"fontsize" : 21.066666,
					"numinlets" : 1,
					"patching_rect" : [ 133.0, 5.0, 309.0, 35.0 ],
					"numoutlets" : 0,
					"fontname" : "Arial",
					"frgb" : [ 0.87451, 0.87451, 0.87451, 1.0 ],
					"id" : "obj-44",
					"textcolor" : [ 0.87451, 0.87451, 0.87451, 1.0 ]
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "gain~",
					"orientation" : 2,
					"numinlets" : 2,
					"patching_rect" : [ 14.0, 355.0, 26.0, 65.0 ],
					"numoutlets" : 2,
					"outlettype" : [ "signal", "int" ],
					"id" : "obj-45"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "toggle",
					"numinlets" : 1,
					"patching_rect" : [ 46.0, 135.0, 15.0, 15.0 ],
					"numoutlets" : 1,
					"outlettype" : [ "int" ],
					"id" : "obj-46"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "message",
					"text" : "open",
					"fontsize" : 7.9,
					"numinlets" : 2,
					"patching_rect" : [ 13.0, 135.0, 32.0, 15.0 ],
					"numoutlets" : 1,
					"fontname" : "Arial",
					"outlettype" : [ "" ],
					"id" : "obj-47"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "message",
					"text" : "1",
					"fontsize" : 7.9,
					"numinlets" : 2,
					"patching_rect" : [ 441.0, 78.0, 16.0, 15.0 ],
					"numoutlets" : 1,
					"fontname" : "Arial",
					"outlettype" : [ "" ],
					"hidden" : 1,
					"id" : "obj-48"
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
					"id" : "obj-49"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "newobj",
					"text" : "sfplay~",
					"fontsize" : 7.9,
					"numinlets" : 2,
					"patching_rect" : [ 13.0, 179.0, 43.0, 17.0 ],
					"numoutlets" : 2,
					"fontname" : "Arial",
					"outlettype" : [ "signal", "bang" ],
					"id" : "obj-50",
					"save" : [ "#N", "sfplay~", "", 1, 120960, 0, "", ";" ]
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "message",
					"text" : "startwindow",
					"fontsize" : 7.9,
					"numinlets" : 2,
					"patching_rect" : [ 13.0, 464.0, 65.0, 15.0 ],
					"numoutlets" : 1,
					"fontname" : "Arial",
					"outlettype" : [ "" ],
					"id" : "obj-51"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "ezdac~",
					"numinlets" : 2,
					"patching_rect" : [ 13.0, 429.0, 33.0, 33.0 ],
					"numoutlets" : 0,
					"id" : "obj-52"
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
					"id" : "obj-53"
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
					"id" : "obj-54"
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
					"id" : "obj-55"
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
					"id" : "obj-56"
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
					"id" : "obj-57",
					"textcolor" : [ 0.0, 0.0, 0.0, 1.0 ]
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "panel",
					"bgcolor" : [ 0.388235, 0.388235, 0.388235, 1.0 ],
					"background" : 1,
					"numinlets" : 1,
					"patching_rect" : [ 128.0, 0.0, 805.0, 128.0 ],
					"numoutlets" : 0,
					"rounded" : 0,
					"id" : "obj-58"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "comment",
					"text" : "Attributes",
					"fontsize" : 10.533333,
					"background" : 1,
					"numinlets" : 1,
					"patching_rect" : [ 130.0, 135.0, 253.0, 21.0 ],
					"numoutlets" : 0,
					"fontname" : "Arial",
					"frgb" : [ 0.658824, 0.658824, 0.658824, 1.0 ],
					"id" : "obj-59"
				}

			}
, 			{
				"box" : 				{
					"maxclass" : "panel",
					"bgcolor" : [ 0.501961, 0.54902, 0.733333, 1.0 ],
					"background" : 1,
					"numinlets" : 1,
					"patching_rect" : [ 128.0, 133.0, 805.0, 20.0 ],
					"numoutlets" : 0,
					"rounded" : 0,
					"id" : "obj-60"
				}

			}
 ],
		"lines" : [ 			{
				"patchline" : 				{
					"source" : [ "obj-4", 0 ],
					"destination" : [ "obj-7", 0 ],
					"hidden" : 0
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-7", 0 ],
					"destination" : [ "obj-8", 0 ],
					"hidden" : 0
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-6", 0 ],
					"destination" : [ "obj-8", 0 ],
					"hidden" : 0
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-5", 0 ],
					"destination" : [ "obj-6", 0 ],
					"hidden" : 0
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-24", 0 ],
					"destination" : [ "obj-23", 1 ],
					"hidden" : 0
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-25", 2 ],
					"destination" : [ "obj-24", 0 ],
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
, 			{
				"patchline" : 				{
					"source" : [ "obj-22", 0 ],
					"destination" : [ "obj-23", 0 ],
					"hidden" : 0
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-25", 2 ],
					"destination" : [ "obj-22", 0 ],
					"hidden" : 0
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-6", 0 ],
					"destination" : [ "obj-1", 0 ],
					"hidden" : 0
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-21", 0 ],
					"destination" : [ "obj-25", 0 ],
					"hidden" : 0
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-23", 0 ],
					"destination" : [ "obj-26", 0 ],
					"hidden" : 0
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-49", 0 ],
					"destination" : [ "obj-48", 0 ],
					"hidden" : 1
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-10", 1 ],
					"destination" : [ "obj-9", 0 ],
					"hidden" : 0,
					"midpoints" : [ 472.5, 189.0, 441.5, 189.0 ]
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-3", 0 ],
					"destination" : [ "obj-11", 0 ],
					"hidden" : 0
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-9", 0 ],
					"destination" : [ "obj-11", 0 ],
					"hidden" : 0
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-27", 0 ],
					"destination" : [ "obj-20", 0 ],
					"hidden" : 0,
					"midpoints" : [ 640.5, 428.0, 338.5, 428.0 ]
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-26", 0 ],
					"destination" : [ "obj-20", 0 ],
					"hidden" : 0,
					"midpoints" : [ 561.5, 423.0, 338.5, 423.0 ]
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-18", 0 ],
					"destination" : [ "obj-20", 0 ],
					"hidden" : 0,
					"midpoints" : [ 186.5, 412.0, 338.5, 412.0 ]
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-16", 0 ],
					"destination" : [ "obj-20", 0 ],
					"hidden" : 0,
					"midpoints" : [ 286.5, 404.0, 338.5, 404.0 ]
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-11", 0 ],
					"destination" : [ "obj-16", 0 ],
					"hidden" : 0
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-55", 0 ],
					"destination" : [ "obj-56", 0 ],
					"hidden" : 1
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-57", 0 ],
					"destination" : [ "obj-55", 0 ],
					"hidden" : 1
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
					"destination" : [ "obj-19", 0 ],
					"hidden" : 0
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-3", 0 ],
					"destination" : [ "obj-19", 0 ],
					"hidden" : 0
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-14", 1 ],
					"destination" : [ "obj-12", 0 ],
					"hidden" : 0,
					"midpoints" : [ 217.5, 189.0, 186.5, 189.0 ]
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-48", 0 ],
					"destination" : [ "obj-30", 0 ],
					"hidden" : 1
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-33", 0 ],
					"destination" : [ "obj-32", 1 ],
					"hidden" : 0
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-30", 0 ],
					"destination" : [ "obj-32", 1 ],
					"hidden" : 0
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-45", 1 ],
					"destination" : [ "obj-38", 0 ],
					"hidden" : 1
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-37", 1 ],
					"destination" : [ "obj-35", 0 ],
					"hidden" : 0,
					"midpoints" : [ 113.5, 290.0, 60.5, 290.0 ]
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-45", 0 ],
					"destination" : [ "obj-52", 1 ],
					"hidden" : 0
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-45", 0 ],
					"destination" : [ "obj-39", 0 ],
					"hidden" : 1
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-40", 0 ],
					"destination" : [ "obj-42", 0 ],
					"hidden" : 0,
					"midpoints" : [ 71.0, 157.0, 40.5, 157.0 ]
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-41", 0 ],
					"destination" : [ "obj-52", 0 ],
					"hidden" : 1,
					"midpoints" : [ 22.5, 497.0, 6.0, 497.0, 6.0, 425.0, 22.5, 425.0 ]
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-51", 0 ],
					"destination" : [ "obj-52", 0 ],
					"hidden" : 1,
					"midpoints" : [ 22.5, 480.0, 8.0, 480.0, 8.0, 427.0, 22.5, 427.0 ]
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-45", 0 ],
					"destination" : [ "obj-52", 0 ],
					"hidden" : 0
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-36", 0 ],
					"destination" : [ "obj-45", 0 ],
					"hidden" : 0
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-37", 0 ],
					"destination" : [ "obj-36", 0 ],
					"hidden" : 0
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-32", 0 ],
					"destination" : [ "obj-37", 0 ],
					"hidden" : 0
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-20", 0 ],
					"destination" : [ "obj-37", 0 ],
					"hidden" : 0,
					"midpoints" : [ 338.5, 473.0, 139.0, 473.0, 139.0, 238.0, 22.5, 238.0 ]
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-50", 0 ],
					"destination" : [ "obj-32", 0 ],
					"hidden" : 0
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-31", 0 ],
					"destination" : [ "obj-32", 0 ],
					"hidden" : 0
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-42", 0 ],
					"destination" : [ "obj-50", 0 ],
					"hidden" : 0
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-46", 0 ],
					"destination" : [ "obj-50", 0 ],
					"hidden" : 0,
					"midpoints" : [ 55.0, 155.0, 22.5, 155.0 ]
				}

			}
, 			{
				"patchline" : 				{
					"source" : [ "obj-47", 0 ],
					"destination" : [ "obj-50", 0 ],
					"hidden" : 0
				}

			}
 ]
	}

}
