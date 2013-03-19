/* 
 *	External object for Max/MSP
 *	Copyright © 2002 by Timothy Place
 * 
 *	License: This code is licensed under the terms of the "New BSD License"
 *	http://creativecommons.org/licenses/BSD/
 */

#include "TTClassWrapperMax.h"


static t_class *scale_class;				// Required. Global pointing to this class

typedef struct _scale				// Data Structure for this object
{
	t_pxobject	x_obj;				// Header;  Must always be the first field; used by MSP
	double		s_inlow;					// low value from the typed in argument
	double		s_inhigh;					// high value from the typed in argument
	double		s_outlow;
	double		s_outhigh;
	double		s_inscale;
	double		s_outdiff;
	short		x_connected[4];
} t_scale;

// Prototypes for methods: need a method for each incoming message type
void *scale_new(Symbol *s, short argc, Atom *argv);			// New Object Creation Method
void scale_perform64(t_scale *x, t_object *dsp64, double **ins, long numins, double **outs, long numouts, long sampleframes, long flags, void *userparam);
void scale_perform264(t_scale *x, t_object *dsp64, double **ins, long numins, double **outs, long numouts, long sampleframes, long flags, void *userparam);
void scale_dsp64(t_scale *x, t_object *dsp64, short *count, double samplerate, long maxvectorsize, long flags);			// ../../../../Jamoma/Core/DSP/library/build/JamomaDSP.dylib Method
void scale_assist(t_scale *x, void *b, long m, long a, char *s);		// Assistance Method
void scale_float(t_scale *x, double val);


/************************************************************************************/
// Main() Function

extern "C" int TTCLASSWRAPPERMAX_EXPORT main(void)
{
	t_class *c;

	c = class_new("tap.scale~",(method)scale_new, (method)dsp_free, sizeof(t_scale), 
		(method)0L, A_GIMME, 0);

		common_symbols_init();														// Initialize TapTools
	class_addmethod(c, (method)scale_dsp64,		"dsp64", A_CANT, 0);
    class_addmethod(c, (method)scale_float, 	"float", A_FLOAT, 0L);	// Input as double
    class_addmethod(c, (method)scale_assist, 	"assist", A_CANT, 0L); 
	class_addmethod(c, (method)stdinletinfo,	"inletinfo",	A_CANT, 0);

	class_dspinit(c);									// Setup object's class to work with MSP
class_register(_sym_box, c); 	scale_class = c;
}


/************************************************************************************/
// Object Creation Method

void *scale_new(Symbol *s, short argc, Atom *argv)
{
	t_scale *x = (t_scale *)object_alloc(scale_class);;
	if(x){
		if(argc){										// Initialize the following if there are arguments...
			x->s_inlow = argv[0].a_w.w_float;	
			x->s_inhigh = argv[1].a_w.w_float;	
			x->s_outlow = argv[2].a_w.w_float;		
			x->s_outhigh = argv[3].a_w.w_float;
		}
	   	else{											// Initialize the following if there are no arguments...
			x->s_inlow = 0;	
			x->s_inhigh = 127;		
			x->s_outlow = 0;		
			x->s_outhigh  = 1;	
	     }
		object_obex_store((void *)x, _sym_dumpout, (object *)outlet_new(x,NULL));	// dumpout
		dsp_setup((t_pxobject *)x, 5);					// Create Object with 5 Inlets (input, inlow, inhigh, outlow, outhigh)
	   	outlet_new((t_object *)x, "signal");			// Create a signal outlet    
		x->s_inscale = 1 / (x->s_inhigh - x->s_inlow);
		x->s_outdiff = x->s_outhigh - x->s_outlow;
		
		// PROCESS ATTRIBUTES HERE IF I ADD THEM LATER
	}
	return x;										// Return pointer
}


/************************************************************************************/
// Methods bound to input/inlets

// Method for Assistance Messages
void scale_assist(t_scale *x, void *b, long msg, long arg, char *dst)
{
	if(msg==1){ 	// Inlets
		switch(arg){
			case 0: strcpy(dst, "(signal) value to be scaled"); break;
			case 1: strcpy(dst, "(signal/float) low input value"); break;
			case 2: strcpy(dst, "(signal/float) high input value"); break;
			case 3: strcpy(dst, "(signal/float) low output value"); break;
			case 4: strcpy(dst, "(signal/float) high output value"); break;
			case 5: strcpy(dst, "(signal/float) exponential base value"); break;
		}
	}
	else if(msg==2) // Outlets
		strcpy(dst, "(signal) scaled output value");
}


// Method for double input
void scale_float(t_scale *x, double val)
{
	switch (x->x_obj.z_in){
		case 1:
			x->s_inlow = val;
			x->s_inscale = 1 / (x->s_inhigh - x->s_inlow);
			break;
		case 2:
			x->s_inhigh = val;
			x->s_inscale = 1 / (x->s_inhigh - x->s_inlow);
			break;
		case 3:
			x->s_outlow = val;
			x->s_outdiff = x->s_outhigh - x->s_outlow;
			break;
		case 4:
			x->s_outhigh = val;
			x->s_outdiff = x->s_outhigh - x->s_outlow;
			break;
	}
}


void scale_perform64(t_scale *x, t_object *dsp64, double **ins, long numins, double **outs, long numouts, long sampleframes, long flags, void *userparam)
{
	double *in1 = ins[0];					// Input 1
	double *out1 = outs[0];					// Output 1
	int n = sampleframes;
	double value;									//  Variable for temporary storage
 
	while (n--) {
		value = *in1++;
		value = (value - x->s_inlow) * x->s_inscale;
		value = (value * x->s_outdiff) + x->s_outlow;
		*out1++ = value;											
	}
    return;
}


void scale_perform264(t_scale *x, t_object *dsp64, double **ins, long numins, double **outs, long numouts, long sampleframes, long flags, void *userparam)
{
	double value, inscale, outdiff;
	
	double *in1 = ins[0];					// Input 1
 	double inlow = x->x_connected[1]? *ins[1] : x->s_inlow;
	double inhigh = x->x_connected[2]? *ins[2] : x->s_inhigh;
	double outlow = x->x_connected[3]? *ins[3] : x->s_outlow;
	double outhigh = x->x_connected[4]? *ins[4] : x->s_outhigh;
	double *out1 = outs[0];					// Output 1
	int n = sampleframes;
 
 	inscale = 1 / (inhigh - inlow);
 	outdiff = outhigh - outlow;
 
	while (n--) {
		value = *in1++;
		value = (value - inlow) * inscale;
		value = (value * outdiff) + outlow;
		*out1++ = value;											
	}
out:
	return;
}


void scale_dsp64(t_scale *x, t_object *dsp64, short *count, double samplerate, long maxvectorsize, long flags)
{
	if ((count[1]) || (count[2]) || (count[3]) || (count[4])) {
	 	x->x_connected[1] = count[1];
		x->x_connected[2] = count[2];
		x->x_connected[3] = count[3];
		x->x_connected[4] = count[4]; 	
		object_method(dsp64, gensym("dsp_add64"), (t_object*)x, scale_perform264, 0, NULL);
	}
	else
		object_method(dsp64, gensym("dsp_add64"), (t_object*)x, scale_perform64, 0, NULL);
}
