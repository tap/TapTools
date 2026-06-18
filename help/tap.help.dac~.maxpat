{
 "patcher": {
  "fileversion": 1,
  "appversion": {
   "major": 7,
   "minor": 0,
   "revision": 4,
   "architecture": "x64",
   "modernui": 1
  },
  "rect": [
   80.0,
   80.0,
   200.0,
   180.0
  ],
  "bglocked": 0,
  "openinpresentation": 0,
  "default_fontsize": 10.0,
  "default_fontface": 0,
  "default_fontname": "Verdana",
  "gridonopen": 1,
  "gridsize": [
   5.0,
   5.0
  ],
  "description": "Shared help output module: horizontal stereo live.meter~ + a local dac~ (toggle to turn this patcher's audio on/off). 2 signal inlets (L/R), 2 pass-through outlets.",
  "boxes": [
   {
    "box": {
     "id": "obj-1",
     "maxclass": "inlet",
     "numinlets": 0,
     "numoutlets": 1,
     "patching_rect": [
      12.0,
      10.0,
      25.0,
      25.0
     ],
     "outlettype": [
      ""
     ]
    }
   },
   {
    "box": {
     "id": "obj-2",
     "maxclass": "inlet",
     "numinlets": 0,
     "numoutlets": 1,
     "patching_rect": [
      120.0,
      10.0,
      25.0,
      25.0
     ],
     "outlettype": [
      ""
     ]
    }
   },
   {
    "box": {
     "id": "obj-3",
     "maxclass": "live.meter~",
     "numinlets": 1,
     "numoutlets": 1,
     "patching_rect": [
      12.0,
      45.0,
      132.0,
      11.0
     ],
     "outlettype": [
      "float"
     ],
     "orientation": 1,
     "slidercolor": [
      0.0,
      0.74,
      0.0,
      1.0
     ]
    }
   },
   {
    "box": {
     "id": "obj-4",
     "maxclass": "live.meter~",
     "numinlets": 1,
     "numoutlets": 1,
     "patching_rect": [
      12.0,
      60.0,
      132.0,
      11.0
     ],
     "outlettype": [
      "float"
     ],
     "orientation": 1,
     "slidercolor": [
      0.0,
      0.74,
      0.0,
      1.0
     ]
    }
   },
   {
    "box": {
     "id": "obj-5",
     "maxclass": "toggle",
     "numinlets": 1,
     "numoutlets": 1,
     "patching_rect": [
      12.0,
      86.0,
      22.0,
      22.0
     ],
     "outlettype": [
      "int"
     ]
    }
   },
   {
    "box": {
     "id": "obj-6",
     "maxclass": "newobj",
     "numinlets": 2,
     "numoutlets": 0,
     "patching_rect": [
      42.0,
      87.0,
      38.0,
      22.0
     ],
     "text": "dac~"
    }
   },
   {
    "box": {
     "id": "obj-7",
     "maxclass": "comment",
     "numinlets": 1,
     "numoutlets": 0,
     "patching_rect": [
      88.0,
      88.0,
      64.0,
      20.0
     ],
     "text": "audio on/off"
    }
   },
   {
    "box": {
     "id": "obj-8",
     "maxclass": "comment",
     "numinlets": 1,
     "numoutlets": 0,
     "patching_rect": [
      12.0,
      0.0,
      130.0,
      20.0
     ],
     "text": "stereo monitor + output"
    }
   },
   {
    "box": {
     "id": "obj-9",
     "maxclass": "outlet",
     "numinlets": 1,
     "numoutlets": 0,
     "patching_rect": [
      12.0,
      118.0,
      25.0,
      25.0
     ]
    }
   },
   {
    "box": {
     "id": "obj-10",
     "maxclass": "outlet",
     "numinlets": 1,
     "numoutlets": 0,
     "patching_rect": [
      120.0,
      118.0,
      25.0,
      25.0
     ]
    }
   }
  ],
  "lines": [
   {
    "patchline": {
     "source": [
      "obj-1",
      0
     ],
     "destination": [
      "obj-3",
      0
     ],
     "hidden": 0,
     "disabled": 0
    }
   },
   {
    "patchline": {
     "source": [
      "obj-1",
      0
     ],
     "destination": [
      "obj-6",
      0
     ],
     "hidden": 0,
     "disabled": 0
    }
   },
   {
    "patchline": {
     "source": [
      "obj-1",
      0
     ],
     "destination": [
      "obj-9",
      0
     ],
     "hidden": 0,
     "disabled": 0
    }
   },
   {
    "patchline": {
     "source": [
      "obj-2",
      0
     ],
     "destination": [
      "obj-4",
      0
     ],
     "hidden": 0,
     "disabled": 0
    }
   },
   {
    "patchline": {
     "source": [
      "obj-2",
      0
     ],
     "destination": [
      "obj-6",
      1
     ],
     "hidden": 0,
     "disabled": 0
    }
   },
   {
    "patchline": {
     "source": [
      "obj-2",
      0
     ],
     "destination": [
      "obj-10",
      0
     ],
     "hidden": 0,
     "disabled": 0
    }
   },
   {
    "patchline": {
     "source": [
      "obj-5",
      0
     ],
     "destination": [
      "obj-6",
      0
     ],
     "hidden": 0,
     "disabled": 0
    }
   }
  ]
 }
}
