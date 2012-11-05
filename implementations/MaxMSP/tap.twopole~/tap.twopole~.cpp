/* 
 *	External object for Max/MSP
 *	Copyright © 2004 by Timothy Place
 * 
 *	License: This code is licensed under the terms of the "New BSD License"
 *	http://creativecommons.org/licenses/BSD/
 */

#include "TTClassWrapperMax.h"		// Required for all TapTools External Objects
#include "../ttblue/tt_audio_base.h"
#include "../ttblue/tt_audio_signal.h"
#include "../ttblue/tt_lowpass_twopole.h"

#define NUM_INPUTS 2
#define NUM_OUTPUTS 2

// Data Structure for this object
typedef struct _twopole{
    t_pxobject 				x_obj;
	tt_lowpass_twopole		*myFilter;
    tt_audio_signal	*signal_in[NUM_INPUTS];
    tt_audio_signal	*signal_out[NUM_OUTPUTS];
	float					attr_frequency;
	float					attr_resonance;
} t_twopole;

// Prototypes for methods: need a method for each incoming message type
void *twopole_new(t_symbol *msg, long argc, t_atom *argv);					// New Object Creation Method
t_int *twopole_perform(t_int *w);											// An MSP Perform (signal) Method
t_int *twopole_perform2(t_int *w);
void twopole_dsp(t_twopole *x, t_signal **sp, short *count);				// DSP Method
void twopole_assist(t_twopole *x, void *b, long msg, long arg, char *dst);	// Assistance Method
t_max_err attr_set_frequency(t_twopole *x, void *attr, long argc, t_atom *argv);
t_max_err attr_set_resonance(t_twopole *x, void *attr, long argc, t_atom *argv);
void twopole_clear(t_twopole *x);
void twopole_free(t_twopole *x);

// Globals
static t_class *twopole_class;			// Required. Global pointing to this class


/************************************************************************************/
// Main() Function

extern "C" int TTCLASSWRAPPERMAX_EXPORT main(void)
{
	t_class *c;

	c = class_new("tap.twopole~",(method)twopole_new, (method)twopole_free, sizeof(t_twopole), 
		(method)0L, A_GIMME, 0);

		common_symbols_init();										// Initialize TapTools
	class_addmethod(c, (method)twopole_clear,	"clear", 0L);
 	class_addmethod(c, (method)twopole_dsp, 	"dsp", A_CANT, 0L);		
	class_addmethod(c, (method)twopole_assist, 	"assist", A_CANT, 0L); 

	CLASS_ATTR_FLOAT(c,		"frequency",		0,	t_twopole, attr_frequency);
	CLASS_ATTR_ACCESSORS(c,	"frequency",		NULL, attr_set_frequency);

	CLASS_ATTR_FLOAT(c,		"resonance",		0,	t_twopole, attr_resonance);
	CLASS_ATTR_ACCESSORS(c,	"resonance",		NULL, attr_set_resonance);

	class_dspinit(c);									// Setup object's class to work with MSP
class_register(_sym_box, c); 	twopole_class = c;
}


/************************************************************************************/
// Object Creation Method

void *twopole_new(t_symbol *msg, long argc, t_atom *argv)
{
    t_twopole *x = (t_twopole *)object_alloc(twopole_class);;
 	short i;
 	
    if(x){
    	object_obex_store((void *)x, _sym_dumpout, (object *)outlet_new(x,NULL));	// dumpout	
	    dsp_setup((t_pxobject *)x, 2);
  	    x->x_obj.z_misc = Z_NO_INPLACE;  			// ESSENTIAL!   		
  
	    outlet_new((t_pxobject *)x, "signal");		// Create a signal Outlet
	    outlet_new((t_pxobject *)x, "signal");		// Create a signal Outlet

		x->myFilter = new tt_lowpass_twopole;
		x->myFilter->set_sr(sys_getsr());
		for(i=0; i<NUM_INPUTS; i++)
			x->signal_in[i] = new tt_audio_signal;
		for(i=0; i<NUM_OUTPUTS; i++)
			x->signal_out[i] = new tt_audio_signal;

		attr_args_process(x, argc, argv);			//handle attribute args					
	}
	return(x);										// Return the pointer
	#pragma unused(msg)
}

// Memory Deallocation
void twopole_free(t_twopole *x)
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
void twopole_assist(t_twopole *x, void *b, long msg, long arg, char *dst)
{
	if(msg==1) 	// Inlets
		strcpy(dst, "(signal) input, control messages");		
	else if(msg==2) // Outlets
		strcpy(dst, "(signal) Filtered output");
	#pragma unused(x)
	#pragma unused(b)
	#pragma unused(arg)
}


// ATTRIBUTE
t_max_err attr_set_frequency(t_twopole *x, void *attr, long argc, t_atom *argv)
{
	x->attr_frequency = atom_getfloat(argv);
	x->myFilter->set_attr(tt_lowpass_twopole::k_frequency, x->attr_frequency);
	
	return MAX_ERR_NONE;
	#pragma unused(attr)
}

// ATTRIBUTE
t_max_err attr_set_resonance(t_twopole *x, void *attr, long argc, t_atom *argv)
{
	x->attr_resonance = atom_getfloat(argv);
	x->myFilter->set_attr(tt_lowpass_twopole::k_resonance, x->attr_resonance);
	
	return MAX_ERR_NONE;
	#pragma unused(attr)
}


// MESSAGE: clear
void twopole_clear(t_twopole *x)
{
	x->myFilter->clear();
}


tt_audio_signal temp_in1, temp_in2, temp_out1, temp_out2;

// Perform (signal) Method
t_int *twopole_perform(t_int *w)
{
   	t_twopole *x = (t_twopole *)(w[1]);		// Pointer
    x->signal_in[0]->set_vector((t_float *)(w[2])); 	// Input
    x->signal_out[0]->set_vector((t_float *)(w[3]));	// Output
	x->signal_in[0]->vectorsize = (int)(w[4]);			// Vector Size
			
	if (!(x->x_obj.z_disabled))
		x->myFilter->dsp_vector_calc(x->signal_in[0], x->signal_out[0]);	

    return (w + 5);		// Return a pointer to the NEXT object in the DSP call chain
}


// Perform (signal) Method
t_int *twopole_perform2(t_int *w)
{
   	t_twopole *x = (t_twopole *)(w[1]);		// Pointer
  	x->signal_in[0]->set_vector((t_float *)(w[2])); 	// Input
    x->signal_in[1]->set_vector((t_float *)(w[3])); 	// Input
    x->signal_out[0]->set_vector((t_float *)(w[4]));	// Output
    x->signal_out[1]->set_vector((t_float *)(w[5]));	// Output
	x->signal_in[0]->vectorsize = (int)(w[6]);			// Vector Size
			
	if (!(x->x_obj.z_disabled))
		x->myFilter->dsp_vector_calc(x->signal_in[0], x->signal_in[1], x->signal_out[0], x->signal_out[1]);	

    return (w + 7);		// Return a pointer to the NEXT object in the DSP call chain
}


// DSP Method
void twopole_dsp(t_twopole *x, t_signal **sp, short *count)
{
	x->myFilter->set_sr(sp[0]->s_sr);
	x->myFilter->clear();
	if(count[1] && count[3])
		dsp_add(twopole_perform2, 6, x, sp[0]->s_vec, sp[1]->s_vec, sp[2]->s_vec, sp[3]->s_vec, sp[0]->s_n);
	else
		dsp_add(twopole_perform, 4, x, sp[0]->s_vec, sp[2]->s_vec, sp[0]->s_n); // Add Perform Method to the DSP Call Chain
	#pragma unused(count)
}

