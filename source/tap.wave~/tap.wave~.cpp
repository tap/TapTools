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
#include "../ttblue/tt_buffer.h"
#include "../ttblue/tt_buffer_play.h"

#define NUM_INPUTS 1
#define NUM_OUTPUTS 1

// Data Structure for this object
typedef struct _maxwave{
    t_pxobject 			x_obj;
	tt_buffer_play		*my_wave;
	tt_buffer			*another_waveform;
    tt_audio_signal		*signal_in[NUM_INPUTS];
    tt_audio_signal		*signal_out[NUM_OUTPUTS];
	float				attr_gain;
	t_symbol			*attr_mode;
} t_maxwave;


// Prototypes for methods: need a method for each incoming message type
void *maxwave_new(t_symbol *msg, short argc, t_atom *argv);					// New Object Creation Method
void maxwave_free(t_maxwave *x);
void maxwave_assist(t_maxwave *x, void *b, long msg, long arg, char *dst);	// Assistance Method
void maxwave_dsp(t_maxwave *x, t_signal **sp, short *count);				// ../../../../Jamoma/Core/DSP/library/build/JamomaDSP.dylib Method
t_int *maxwave_perform(t_int *w);											// An MSP Perform (signal) Method
t_int *maxwave_perform2(t_int *w);
t_max_err attr_set_gain(t_maxwave *x, void *attr, long argc, t_atom *argv);
t_max_err attr_set_mode(t_maxwave *x, void *attr, long argc, t_atom *argv);

// Globals
static t_class		*maxwave_class;				// Required. Global pointing to this class
static t_symbol	*ps_normalized;
static t_symbol	*ps_samples;
static t_symbol	*ps_ms;


/************************************************************************************/
// Main() Function

extern "C" int TTCLASSWRAPPERMAX_EXPORT main(void)
{
	t_class *c;

	c = class_new("tap.wave~",(method)maxwave_new, (method)maxwave_free, sizeof(t_maxwave), 
		(method)0L, A_GIMME, 0);

		common_symbols_init();										// Initialize TapTools
 	class_addmethod(c, (method)maxwave_dsp, 	"dsp", A_CANT, 0L);		
	class_addmethod(c, (method)maxwave_assist, 	"assist", A_CANT, 0L); 

	CLASS_ATTR_FLOAT(c,		"gain",		0,	t_maxwave, attr_gain);
	CLASS_ATTR_ACCESSORS(c,	"gain",		NULL, attr_set_gain);

	CLASS_ATTR_SYM(c,		"mode",		0,	t_maxwave, attr_mode);
	CLASS_ATTR_ACCESSORS(c,	"mode",		NULL, attr_set_mode);

	class_dspinit(c);									// Setup object's class to work with MSP
class_register(_sym_box, c); 	maxwave_class = c;
	
	// Init Globals
	ps_normalized = gensym("normalized");
	ps_samples = gensym("samples");
	ps_ms = gensym("ms");
}


/************************************************************************************/
// Object Creation Method

void *maxwave_new(t_symbol *msg, short argc, t_atom *argv)
{
    t_maxwave *x = (t_maxwave *)object_alloc(maxwave_class);;
	short i;

    if(x){	
    	object_obex_store((void *)x, _sym_dumpout, (object *)outlet_new(x,NULL));	// dumpout	
	    dsp_setup((t_pxobject *)x,1);										// Create Object and 1 Inlet (last argument)
	    outlet_new((t_pxobject *)x, "signal");								// Create a signal Outlet

		//taptools_max::set_global_sr(sys_getsr());							// Set the Tap.Tools global SR
		x->my_wave = new tt_buffer_play;									// Create the playback object
		x->another_waveform = new tt_buffer;								// Create the buffer
		for(i=0; i<NUM_INPUTS; i++)
			x->signal_in[i] = new tt_audio_signal;
		for(i=0; i<NUM_OUTPUTS; i++)
			x->signal_out[i] = new tt_audio_signal;

		x->another_waveform->set_attr(tt_buffer::k_length_samples, 8192);	// Size the buffer
		x->another_waveform->fill(tt_buffer::k_sine);						// Fill the buffer
		x->my_wave->set_buffer(x->another_waveform);						// bind the playback object to the buffer
		x->attr_mode = ps_normalized;										// Set default to prevent quickref crash

		attr_args_process(x,argc,argv);										//handle attribute args					
	}
	return (x);																// Return the pointer
	#pragma unused(msg)
}

// Memory Deallocation
void maxwave_free(t_maxwave *x)
{
	short i;
	
	dsp_free((t_pxobject *)x);
	delete x->my_wave;
	delete x->another_waveform;
	for(i=0; i<NUM_INPUTS; i++)
		delete x->signal_in[i];
	for(i=0; i<NUM_OUTPUTS; i++)
		delete x->signal_out[i];

}


/************************************************************************************/
// Methods bound to input/inlets

// Method for Assistance Messages
void maxwave_assist(t_maxwave *x, void *b, long msg, long arg, char *dst)
{
	if(msg==1) 	// Inlets
		strcpy(dst, "(signal) input, control messages");		
	else if(msg==2) // Outlets
		strcpy(dst, "(signal) output");
		
	#pragma unused(x)
	#pragma unused(b)
	#pragma unused(arg)
}


// ATTRIBUTE
t_max_err attr_set_gain(t_maxwave *x, void *attr, long argc, t_atom *argv)
{
	x->attr_gain = atom_getfloat(argv);
	x->my_wave->set_attr(tt_buffer_play::k_gain, x->attr_gain);
	
	return MAX_ERR_NONE;
	#pragma unused(attr)
}

// ATTRIBUTE
t_max_err attr_set_mode(t_maxwave *x, void *attr, long argc, t_atom *argv)
{
	if(argc && argv){
		x->attr_mode = atom_getsym(argv);
	}
		
	if(x->attr_mode == ps_normalized)
		x->my_wave->set_attr(tt_buffer_play::k_mode, tt_buffer_play::k_mode_normalized);
	else if(x->attr_mode == ps_samples)
		x->my_wave->set_attr(tt_buffer_play::k_mode, tt_buffer_play::k_mode_samples);
	else if(x->attr_mode == ps_ms)
		x->my_wave->set_attr(tt_buffer_play::k_mode, tt_buffer_play::k_mode_ms);
	else
		object_error((t_object *)x, "invalid mode specified");

	return MAX_ERR_NONE;
	#pragma unused(attr)
}


// Perform (signal) Method: With Modulation
t_int *maxwave_perform(t_int *w)
{
   	t_maxwave *x = (t_maxwave *)(w[1]);					// Pointer
    x->signal_in[0]->set_vector((t_float *)(w[2])); 	// Input
    x->signal_out[0]->set_vector((t_float *)(w[3]));	// Output
	x->signal_in[0]->vectorsize = (int)(w[4]);			// Vector Size
			
	if(!(x->x_obj.z_disabled))
		x->my_wave->dsp_vector_calc(x->signal_in[0], x->signal_out[0]);	// location is the "in" vector

    return (w + 5);
}


// ../../../../Jamoma/Core/DSP/library/build/JamomaDSP.dylib Method
void maxwave_dsp(t_maxwave *x, t_signal **sp, short *count)
{
	x->my_wave->set_sr(sp[0]->s_sr);
	dsp_add(maxwave_perform, 4, x, sp[0]->s_vec, sp[1]->s_vec, sp[0]->s_n); // Add Perform Method to the ../../../../Jamoma/Core/DSP/library/build/JamomaDSP.dylib Call Chain
	
	#pragma unused(count)
}

