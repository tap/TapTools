/* 
 *	tap.noise~
 *	External object for Max
 *
 *	Copyright © 2012 by Timothy Place
 * 	All Rights Reserved
 */

#include "TTClassWrapperMax.h"


int TTCLASSWRAPPERMAX_EXPORT main(void)
{	
	WrappedClassOptionsPtr	options = new WrappedClassOptions;
	WrappedClassPtr			c = NULL;	
	TTValue					v(1);
	
	TTDSPInit();
	options->append(TT("fixedNumOutputChannels"), v);
	wrapTTClassAsMaxClass(TT("noise"), "tap.noise~", &c, options);
	CLASS_ATTR_ENUM(c->maxClass, "mode", 0, "white pink brown blue gauss");
	
	return 0;
}

