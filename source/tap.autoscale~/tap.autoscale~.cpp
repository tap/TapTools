/* 
 *	External object for Max/MSP
 *	Copyright © 2005 by Jesse Allison
 * 
 *	License: This code is licensed under the terms of the "New BSD License"
 *	http://creativecommons.org/licenses/BSD/
 */

#include "TTClassWrapperMax.h"


static t_class *range_class;

typedef struct _range
{
    t_pxobject 	x_obj;
	double 		gain;
	double		outputLow;		// attr
	double		outputHigh;		// attr
	double		attr_input_low;
	double		inputLow;
	double		attr_input_high;
	double		inputHigh;
	long 		automatic;		// attr
} t_autoscale;


void range_perform64(t_autoscale *x, t_object *dsp64, double **ins, long numins, double **outs, long numouts, long sampleframes, long flags, void *userparam);
void range_reset(t_autoscale *x);					// reset routine bound to bang & reset message
void range_print(t_autoscale *x);
void autoscale_dsp64(t_autoscale *x, t_object *dsp64, short *count, double samplerate, long maxvectorsize, long flags);
void range_assist(t_autoscale *x, void *b, long m, long a, char *s);
void *range_new(t_symbol *msg, short argc, t_atom *argv);


extern "C" int TTCLASSWRAPPERMAX_EXPORT main(void)
{
	t_class *c;

	c = class_new("tap.autoscale~",(method)range_new, (method)dsp_free, sizeof(t_autoscale), 
		(method)0L, A_GIMME, 0);

	common_symbols_init();

	class_addmethod(c, (method)range_print,		"bang", 0L);
	class_addmethod(c, (method)range_reset,		"reset", 0L);
	class_addmethod(c, (method)autoscale_dsp64, "dsp64", A_CANT, 0);
	class_addmethod(c, (method)range_assist, 	"assist", A_CANT, 0L); 
	class_addmethod(c, (method)stdinletinfo,	"inletinfo",	A_CANT, 0);

	CLASS_ATTR_LONG(c,		"auto",			0,	t_autoscale, automatic);
	CLASS_ATTR_STYLE(c,		"auto",			0,	"onoff");
	
	CLASS_ATTR_DOUBLE(c,		"input_low",	0,	t_autoscale, attr_input_low);
	CLASS_ATTR_DOUBLE(c,		"input_high",	0,	t_autoscale, attr_input_high);
	CLASS_ATTR_DOUBLE(c,		"output_low",	0,	t_autoscale, outputLow);
	CLASS_ATTR_DOUBLE(c,		"output_high",	0,	t_autoscale, outputHigh);

	class_dspinit(c);									// Setup object's class to work with MSP
	range_class = c; class_register(_sym_box, c);
}


void range_perform64(t_autoscale *x, t_object *dsp64, double **ins, long numins, double **outs, long numouts, long sampleframes, long flags, void *userparam)
{
	double *in = ins[0];
	double *out = outs[0];
	int n = sampleframes;
	double val;
	double inputLow = x->inputLow;
	double inputHigh = x->inputHigh;
	double outputLow = x->outputLow;
	double outputHigh = x->outputHigh;
	double gain = x->gain;
			
	gain = (outputHigh - outputLow) / (inputHigh - inputLow);
	
	if (x->automatic) {	
		while (n--) {
			val = *in++;							// Must add auto range checking & calculation
			if (val > inputHigh) {
				inputHigh = val;
				gain = (outputHigh - outputLow) / (inputHigh - inputLow);
			}
			else if (val < inputLow) {
				inputLow = val;
				gain = (outputHigh - outputLow) / (inputHigh - inputLow);
			}
			*out++ = (((val - inputLow) * gain) + outputLow);		// calculate and fill output vector 
		}
	}
	else {
		while (n--) {
			val = *in++;							// Must add auto range checking & calculation
			*out++ = (((val - inputLow) * gain) + outputLow);		// calculate and fill output vector 
		}
	}
	
	x->inputLow = inputLow;
	x->inputHigh = inputHigh;
	x->outputLow = outputLow;
	x->outputHigh = outputHigh;
	x->gain = gain;
}


void autoscale_dsp64(t_autoscale *x, t_object *dsp64, short *count, double samplerate, long maxvectorsize, long flags)
{
	object_method(dsp64, gensym("dsp_add64"), (t_object*)x, range_perform64, 0, NULL);
}


// should reset values to 0.?
void range_reset(t_autoscale *x)
{
	x->gain = 1.0;
	x->inputLow = x->attr_input_low;
	x->inputHigh = x->attr_input_high;
}


// should print range values
void range_print(t_autoscale *x)
{	
	object_post((t_object *)x, "InputLow = %f InputHigh = %f OutputLow = %f OutputHigh = %f Gain = %f",x->inputLow, x->inputHigh,x->outputLow,x->outputHigh,x->gain);
}


void range_assist(t_autoscale *x, void *b, long m, long a, char *s)
{
	switch(a){
		case 0:
			strcpy(s,"(Signal) Input signal");
			break;
		case 1: 
			strcpy(s,"(double) Output Low");
			break;
		case 2: 
			strcpy(s,"(double) Output High");
			break;
		case 3:
			strcpy(s,"(Signal) Scaled Output");
			break;
	}
}


void *range_new(t_symbol *msg, short argc, t_atom *argv)
{
    t_autoscale *x = (t_autoscale *)object_alloc(range_class);
	if(x){
    	object_obex_store((void *)x, _sym_dumpout, (object *)outlet_new(x,NULL));	// dumpout		
	    dsp_setup((t_pxobject *)x, 1);							// 1 inlet
	    outlet_new((t_pxobject *)x, "signal");					// 1 signal outlet

		x->inputLow = x->attr_input_low = 0.49;		// defaults...
		x->inputHigh = x->attr_input_high = 0.51;						
		x->outputLow = 0.;
		x->outputHigh = 1.0;
		x->automatic = 1;
		x->gain = 1.;

		attr_args_process(x, argc, argv);				//handle attribute args					
	}
    return (x);
}



