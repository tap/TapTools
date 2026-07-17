/// @file
/// @brief      Umbrella header for the TapTools DSP kernel — pulls in every kernel header.
/// @details    Each kernel is self-contained and can be included individually; this header
///             exists for convenience and as the marker the build systems probe for.
// SPDX-License-Identifier: BSD-3-Clause
// Copyright 2003-2026 Timothy Place.

#pragma once

#include "autowah.h"
#include "bridged_t.h"
#include "conv_engine.h"
#include "fft.h"
#include "grm_comb.h"
#include "grm_pitchaccum.h"
#include "ladder.h"
#include "metal_bank.h"
#include "nr.h"
#include "spectra.h"
#include "stft.h"
#include "svf.h"
#include "swing_vca.h"
#include "tr808_clap.h"
#include "tr808_cowbell.h"
#include "tr808_cymbal.h"
#include "tr808_hat.h"
#include "tr808_kick.h"
#include "tr808_rim.h"
#include "tr808_snare.h"
#include "tr808_tom.h"
#include "vco.h"
#include "vocoder.h"
