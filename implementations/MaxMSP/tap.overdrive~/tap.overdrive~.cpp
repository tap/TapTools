#include "TTClassWrapperMax.h"


extern "C" int TTCLASSWRAPPERMAX_EXPORT main(void)
{
	WrappedClassPtr	w;
	TTErr			err;
	
	err = wrapTTClassAsMaxClass(TT("overdrive"), "tap.overdrive~", &w);
	if (!err) {
		CLASS_ATTR_STYLE(w->maxClass,	"bypass",		0,	"onoff");
		CLASS_ATTR_STYLE(w->maxClass,	"dcblocker",	0,	"onoff");
	}
}
