/* 
 *	External object for Max/MSP
 *	Copyright © 2004 by Timothy Place
 * 
 *	License: This code is licensed under the terms of the "New BSD License"
 *	http://creativecommons.org/licenses/BSD/
 */

#include "TTClassWrapperMax.h"	// Required for all TapTools External Objects
#include "../ttblue/tt_audio_base.h"
#include "../ttblue/tt_audio_signal.h"
#include "../ttblue/tt_lowpass_onepole.h"

#define NUM_INPUTS 1
#define NUM_OUTPUTS 1

// Data Structure for this object
typedef struct _onepole{
    t_pxobject 				x_obj;
	tt_lowpass_onepole		*myFilter;
    tt_audio_signal			*signal_in[NUM_INPUTS];
    tt_audio_signal			*signal_out[NUM_OUTPUTS];
} t_onepole;

// Prototypes for methods: need a method for each incoming message type
void *onepole_new(double value);											// New Object Creation Method
t_int *onepole_perform(t_int *w);											// An MSP Perform (signal) Method
void onepole_dsp(t_onepole *x, t_signal **sp, short *count);				// ../../../../Jamoma/Core/DSP/library/build/JamomaDSP.dylib Method
void onepole_assist(t_onepole *x, void *b, long msg, long arg, char *dst);	// Assistance Method
void onepole_float(t_onepole *x, double value);								// Cutoff Freq	
void onepole_coef(t_onepole *x, double value);								// Direct spec of coef
void onepole_clear(t_onepole *x);
void onepole_free(t_onepole *x);

// Globals
static t_class *onepole_class;				// Required. Global pointing to this class


/************************************************************************************/
// Main() Function

extern "C" int TTCLASSWRAPPERMAX_EXPORT main(void)
{
	t_class *c;

	c = class_new("tap.onepole~",(method)onepole_new, (method)onepole_free, sizeof(t_onepole), 
		(method)0L, A_DEFFLOAT, 0);

		common_symbols_init();									// Initialize TapTools
	class_addmethod(c, (method)onepole_clear,	"clear", 0L);
	class_addmethod(c, (method)onepole_coef,	"coef", A_FLOAT, 0L);
	class_addmethod(c, (method)onepole_float,	"float", A_FLOAT, 0L);
 	class_addmethod(c, (method)onepole_dsp, 	"dsp", A_CANT, 0L);		
	class_addmethod(c, (method)onepole_assist, 	"assist", A_CANT, 0L); 

	class_dspinit(c);									// Setup object's class to work with MSP
class_register(_sym_box, c); 	onepole_class = c;
}


/************************************************************************************/
// Object Creation Method

void *onepole_new(double value)
{
    t_onepole *x = (t_onepole *)object_alloc(onepole_class);;
	short i;
	
    if(x){
    	object_obex_store((void *)x, _sym_dumpout, (object *)outlet_new(x,NULL));	// dumpout	
	    dsp_setup((t_pxobject *)x,1);				// Create Object and 1 Inlet (last argument)
	    outlet_new((t_pxobject *)x, "signal");		// Create a signal Outlet

		x->myFilter = new tt_lowpass_onepole;
		for(i=0; i<NUM_INPUTS; i++)
			x->signal_in[i] = new tt_audio_signal;
		for(i=0; i<NUM_OUTPUTS; i++)
			x->signal_out[i] = new tt_audio_signal;
		
		x->myFilter->set_sr(sys_getsr());		
		if (value == 0)	value = 0.001;				// Default feedback coef.
		if ((value > 0) && (value < 1))
			x->myFilter->set_attr(tt_lowpass_onepole::k_coefficient, value);
		else 
			x->myFilter->set_attr(tt_lowpass_onepole::k_frequency, value);
		
		//attr_args_process(x,argc,argv);				//handle attribute args							
	}
	return (x);									// Return the pointer
}

// Memory Deallocation
void onepole_free(t_onepole *x)
{
	short i;
	
	dsp_free((t_pxobject *)x);
	delete x->myFilter;
	for(i=0; i<NUM_INPUTS; i++)
		delete x->signal_in[i];
	for(i=0; i<NUM_OUTPUTS; i++)
		delete x->signal_out[i];
}


/************************************************************************************/
// Methods bound to input/inlets

// Method for Assistance Messages
void onepole_assist(t_onepole *x, void *b, long msg, long arg, char *dst)
{
	if(msg==1) 	// Inlets
		strcpy(dst, "(signal) input, control messages");		
	else if(msg==2) // Outlets
		strcpy(dst, "(signal) Filtered output");
}


// float Method
void onepole_float(t_onepole *x, double value)	
{
	x->myFilter->set_attr(tt_lowpass_onepole::k_frequency, value);
}


// directly specify a coefficient
void onepole_coef(t_onepole *x, double value)
{
	x->myFilter->set_attr(tt_lowpass_onepole::k_coefficient, value);
}


// reset the filter
void onepole_clear(t_onepole *x)
{
	x->myFilter->clear();
}


// Perform (signal) Method
t_int *onepole_perform(t_int *w)
{
   	t_onepole *x = (t_onepole *)(w[1]);		// Pointer
    x->signal_in[0]->set_vector((t_float *)(w[2])); 	// Input
    x->signal_out[0]->set_vector((t_float *)(w[3]));	// Output
	x->signal_in[0]->vectorsize = (int)(w[4]);			// Vector Size
			
	if (!(x->x_obj.z_disabled))
		x->myFilter->dsp_vector_calc(x->signal_in[0], x->signal_out[0]);	

    return (w + 5);		// Return a pointer to the NEXT object in the ../../../../Jamoma/Core/DSP/library/build/JamomaDSP.dylib call chain
}


// ../../../../Jamoma/Core/DSP/library/build/JamomaDSP.dylib Method
void onepole_dsp(t_onepole *x, t_signal **sp, short *count)
{
	x->myFilter->set_sr(sp[0]->s_sr);
	x->myFilter->clear();
	dsp_add(onepole_perform, 4, x, sp[0]->s_vec, sp[1]->s_vec, sp[0]->s_n); // Add Perform Method to the ../../../../Jamoma/Core/DSP/library/build/JamomaDSP.dylib Call Chain
}

