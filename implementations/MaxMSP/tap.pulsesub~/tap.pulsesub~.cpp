// MSP External: tap.pulsesub
// ADSR envelope substitution
// Copyright 2003-2012 by Tim Place

#include "TTClassWrapperMax.h"


extern "C" int TTCLASSWRAPPERMAX_EXPORT main(void)
{
	WrappedClassPtr	w;
	TTErr			err;
	
	err = wrapTTClassAsMaxClass(TT("pulsesub"), "tap.pulsesub~", &w);
	if (!err) {
		CLASS_ATTR_ENUM(w->maxClass,	"mode",		0,	"linear exponential");
	}
}
