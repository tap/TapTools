/* 
 *	tap.pan~
 *	External object for Max/MSP
 *
 *	Copyright © 2001 by Timothy Place
 * 	All Rights Reserved.
 */

#include "TTClassWrapperMax.h"


int TTCLASSWRAPPERMAX_EXPORT main(void)
{	
	WrappedClassOptionsPtr	options = new WrappedClassOptions;
	WrappedClassPtr			c = NULL;	
	TTValue					v(2);
	
	TTDSPInit();
	options->append(TT("fixedNumOutputChannels"), v);
	wrapTTClassAsMaxClass(TT("panorama"), "tap.pan~", &c, options);
	CLASS_ATTR_ENUM(c->maxClass, "mode", 0, "calculate lookup");
	CLASS_ATTR_ENUM(c->maxClass, "shape", 0, "equalPower linear squareRoot");
	
	return 0;
}
