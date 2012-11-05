// MSP External: tap.rms~.c
// RMS envelope following

#include "TTClassWrapperMax.h"

int TTCLASSWRAPPERMAX_EXPORT main(void)
{
	WrappedClassPtr	w;
	
	TTDSPInit();
	return wrapTTClassAsMaxClass(TT("tap.rms"), "tap.rms~", &w);
}
