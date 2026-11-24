/// @file
/// @brief      The oldest DspTap this tree was tested with, asserted wherever a kernel reaches into tap::dsp.
/// @details    DspTap is tagged on its own cadence, and before 1.0 a minor version may break a
///             consumer, so TapTools states the DspTap it was tested with as a compile-time floor
///             (DspTap's README, "Versioning"): the pin in `submodules/dsptap` names the tag, and
///             this header refuses anything older. Every TapTools header that includes a tap/dsp
///             header includes this one too, so a consumer that supplies its own tap::dsp (rather
///             than the pinned submodule) meets the floor in the first translation unit that
///             compiles a DspTap-backed kernel — not only through the umbrella header, which no
///             translation unit in this tree includes. A floor states what was tested, not a
///             promise of what later DspTap versions keep working.
// SPDX-License-Identifier: MIT
// Copyright 2026 Timothy Place and the TapTools contributors

#pragma once

#include "tap/dsp/version.h"

static_assert(TAP_DSP_VERSION >= TAP_DSP_VERSION_ENCODE(0, 7, 0),
              "TapTools needs DspTap 0.7.0 or newer (the tag submodules/dsptap pins)");
