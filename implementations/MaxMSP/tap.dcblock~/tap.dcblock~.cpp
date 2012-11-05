/* 
 *	tt.dcblock~
 *	External object for Max/MSP
 *	Remove DC Offsets from a signal
 *	Example project for TTBlue
 *	Copyright © 2008 by Timothy Place
 * 
 * License: This code is licensed under the terms of the GNU LGPL
 * http://www.gnu.org/licenses/lgpl.html 
 */

#include "TTClassWrapperMax.h"

extern "C" int TTCLASSWRAPPERMAX_EXPORT main(void)
{
	WrappedClassPtr	w;
	ClassPtr		c;
	TTErr			err;
	
	err = wrapTTClassAsMaxClass(TT("dcblock"), "tap.dcblock~", &w);
	if(!err){
		c = w->maxClass;
		CLASS_ATTR_STYLE(c,	"bypass",	0,	"onoff");
	}
}
	
