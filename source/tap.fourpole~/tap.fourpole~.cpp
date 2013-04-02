/* 
 *	External object for Max/MSP
 *	Copyright © 2006 by Timothy Place
 * 
 *	License: This code is licensed under the terms of the "New BSD License"
 *	http://creativecommons.org/licenses/BSD/
 */

#include "TTClassWrapperMax.h"

extern "C" TTErr TTLoadJamomaExtension_FilterLib(void);

extern "C" int TTCLASSWRAPPERMAX_EXPORT main(void)
{
	WrappedClassPtr	w;
	TTErr			err;
	
	TTLoadJamomaExtension_FilterLib();
	
	err = wrapTTClassAsMaxClass(TT("lowpass.4"), "tap.fourpole~", &w);
	if (!err) {
		CLASS_ATTR_STYLE(w->maxClass,	"bypass",		0,	"onoff");
		CLASS_ATTR_STYLE(w->maxClass,	"dcblocker",	0,	"onoff");
	}
}
