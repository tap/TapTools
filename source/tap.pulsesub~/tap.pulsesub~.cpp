/* 
 *	External object for Max/MSP
 *	Copyright © 2003 by Timothy Place
 * 
 *	License: This code is licensed under the terms of the "New BSD License"
 *	http://creativecommons.org/licenses/BSD/
 */

#include "TTClassWrapperMax.h"

extern "C" TTErr TTLoadJamomaExtension_EffectsLib(void);
extern "C" TTErr TTLoadJamomaExtension_GeneratorLib(void);
extern "C" TTErr TTLoadJamomaExtension_MathLib(void);

extern "C" int TTCLASSWRAPPERMAX_EXPORT main(void)
{
	WrappedClassPtr	w;
	TTErr			err;
	
	TTLoadJamomaExtension_EffectsLib();
	TTLoadJamomaExtension_GeneratorLib();
	TTLoadJamomaExtension_MathLib();
	
	err = wrapTTClassAsMaxClass(TT("pulsesub"), "tap.pulsesub~", &w);
	if (!err) {
		CLASS_ATTR_ENUM(w->maxClass,	"mode",		0,	"linear exponential");
	}
}
