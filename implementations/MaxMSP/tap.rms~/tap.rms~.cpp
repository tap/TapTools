/* 
 *	External object for Max/MSP
 *	Copyright © 2003 by Timothy Place
 * 
 *	License: This code is licensed under the terms of the "New BSD License"
 *	http://creativecommons.org/licenses/BSD/
 */

#include "TTClassWrapperMax.h"

int TTCLASSWRAPPERMAX_EXPORT main(void)
{
	WrappedClassPtr	w;
	
	TT../../../../../Jamoma/Core/DSP/library/build/JamomaDSP.dylibInit();
	return wrapTTClassAsMaxClass(TT("tap.rms"), "tap.rms~", &w);
}
