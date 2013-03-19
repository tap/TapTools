/* 
 *	External object for Max/MSP
 *	Copyright © 2003 by Timothy Place
 * 
 *	License: This code is licensed under the terms of the "New BSD License"
 *	http://creativecommons.org/licenses/BSD/
 */

#include "TTClassWrapperMax.h"


typedef struct _diff{				// Data Structure for this object
	t_pxobject 	x_obj;
	double 		diff_b1; 
}t_diff;


// Prototypes for methods: need a method for each incoming message type
void *diff_new(void);											// New Object Creation Method
t_int *diff_perform(t_int *w);
void diff_perform64(t_diff *x, t_object *dsp64, double **ins, long numins, double **outs, long numouts, long sampleframes, long flags, void *userparam);									// An MSP Perform (signal) Method
void diff_dsp(t_diff *x, t_signal **sp, short *count);
void diff_dsp64(t_diff *x, t_object *dsp64, short *count, double samplerate, long maxvectorsize, long flags);			// ../../../../Jamoma/Core/DSP/library/build/JamomaDSP.dylib Method
void diff_assist(t_diff *x, void *b, long m, long a, char *s);	// Assistance Method


static t_class *diff_class;				// Required. Global pointing to this class


/************************************************************************************/
// Main() Function

extern "C" int TTCLASSWRAPPERMAX_EXPORT main(void)
{
	t_class *c;

	c = class_new("tap.diff~",(method)diff_new, (method)dsp_free, sizeof(t_diff), 
		(method)0L, A_GIMME, 0);

		common_symbols_init();
 	class_addmethod(c, (method)diff_dsp, 	"dsp", A_CANT, 0L);		
	class_addmethod(c, (method)diff_dsp64, "dsp64", A_CANT, 0);
	class_addmethod(c, (method)diff_assist, "assist", A_CANT, 0L); 
	class_addmethod(c, (method)stdinletinfo,"inletinfo",	A_CANT, 0);

	class_dspinit(c);
class_register(_sym_box, c); 	diff_class = c;
}


/************************************************************************************/
// Object Creation Method

void *diff_new(void)
{
	t_diff *x = (t_diff *)object_alloc(diff_class);;
	if(x){	
    	object_obex_store((void *)x, _sym_dumpout, (object *)outlet_new(x,NULL));	// dumpout	
		dsp_setup((t_pxobject *)x, 1);				// Create Object and 1 Inlet (last argument)
		outlet_new((t_pxobject *)x, "signal");		// Create a signal Outlet  
		x->diff_b1 = 0;								// initialize value
	}
	return (x);									// Return the pointer
}


/************************************************************************************/
// Methods bound to input/inlets

// Method for Assistance Messages
void diff_assist(t_diff *x, void *b, long msg, long arg, char *dst)
{
	if(msg==1) 	// Inlets
		strcpy(dst, "(signal) Input");
	else if(msg==2) // Outlets
		strcpy(dst, "(signal) Filtered Output");
}


// Perform (signal) Method
t_int *diff_perform(t_int *w)
{
	t_float *in = (t_float *)(w[1]);		// Input
	t_float *out = (t_float *)(w[2]);	// Output
   	t_diff *x = (t_diff *)(w[3]);		// Pointer
	int n = (int)(w[4]);				// Vector Size	
	double	val;	
					
	if (x->x_obj.z_disabled) return (w+5);

	while(--n){
		val = *++in;					// Input
		val = val - x->diff_b1;			// 6dB per octave highpass
		val = TTClip(val, -1., 1.);	// Clip 
		*++out = val;					// Output
		x->diff_b1 = val;				// Store Feedback Sample
	}		
	return (w + 5);					// Return a pointer to the NEXT object in the ../../../../Jamoma/Core/DSP/library/build/JamomaDSP.dylib call chain
}


void diff_perform64(t_diff *x, t_object *dsp64, double **ins, long numins, double **outs, long numouts, long sampleframes, long flags, void *userparam)
{
	double *in = ins[0];
	double *out = outs[0];
   	
	int n = sampleframes;
	double	val;	
					
	if (x->x_obj.z_disabled) return;

	while(--n){
		val = *++in;					// Input
		val = val - x->diff_b1;			// 6dB per octave highpass
		val = TTClip(val, -1., 1.);	// Clip 
		*++out = val;					// Output
		x->diff_b1 = val;				// Store Feedback Sample
	}		
	return;
}



// ../../../../Jamoma/Core/DSP/library/build/JamomaDSP.dylib Method
void diff_dsp(t_diff *x, t_signal **sp, short *count)
{	
	x->diff_b1 = 0;							
	dsp_add(diff_perform, 4, sp[0]->s_vec-1, sp[1]->s_vec-1, x, sp[0]->s_n+1);
}

void diff_dsp64(t_diff *x, t_object *dsp64, short *count, double samplerate, long maxvectorsize, long flags)
{	
	x->diff_b1 = 0;							
	object_method(dsp64, gensym("dsp_add64"), (t_object*)x, diff_perform64, 0, NULL);
}
