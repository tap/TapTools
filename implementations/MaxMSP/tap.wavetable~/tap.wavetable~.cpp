/* 
 *	External object for Max/MSP
 *	Copyright © 2003 by Timothy Place
 * 
 *	License: This code is licensed under the terms of the "New BSD License"
 *	http://creativecommons.org/licenses/BSD/
 */

#include "TTClassWrapperMax.h"	// Required for all TapTools External Objects
#include "../ttblue/tt_audio_base.h"
#include "../ttblue/tt_audio_signal.h"
#include "../ttblue/tt_wavetable.h"

#define NUM_INPUTS 1
#define NUM_OUTPUTS 1

// Data Structure for this object
typedef struct _wavetable{
    t_pxobject 			x_obj;
	tt_wavetable		*my_wavetable;
//	tt_buffer			*another_waveform;
    tt_audio_signal		*signal_in[NUM_INPUTS];
    tt_audio_signal		*signal_out[NUM_OUTPUTS];
	float				attr_frequency;
} t_wavetable;

// Prototypes for methods: need a method for each incoming message type
void *wavetable_new(t_symbol *msg, short argc, t_atom *argv);					// New Object Creation Method
void wavetable_free(t_wavetable *x);
void wavetable_assist(t_wavetable *x, void *b, long msg, long arg, char *dst);	// Assistance Method
void wavetable_dsp(t_wavetable *x, t_signal **sp, short *count);				// ../../../../../Jamoma/Core/DSP/library/build/JamomaDSP.dylib Method
t_int *wavetable_perform(t_int *w);												// An MSP Perform (signal) Method
t_int *wavetable_perform2(t_int *w);
t_max_err attr_set_frequency(t_wavetable *x, void *attr, long argc, t_atom *argv);

// Globals
static t_class *wavetable_class;			// Required. Global pointing to this class


/************************************************************************************/
// Main() Function

extern "C" int TTCLASSWRAPPERMAX_EXPORT main(void)
{
	t_class *c;
	
	c = class_new("tap.wavetable~",(method)wavetable_new, (method)wavetable_free, sizeof(t_wavetable), 
		(method)0L, A_GIMME, 0);

		common_symbols_init();
 	class_addmethod(c, (method)wavetable_dsp,	 	"dsp", A_CANT, 0L);		
	class_addmethod(c, (method)wavetable_assist, 	"assist", A_CANT, 0L); 

	CLASS_ATTR_FLOAT(c,		"frequency",		0,	t_wavetable, attr_frequency);
	CLASS_ATTR_ACCESSORS(c,	"frequency",		NULL, attr_set_frequency);

	class_dspinit(c);									// Setup object's class to work with MSP
class_register(_sym_box, c); 	wavetable_class = c;
}


/************************************************************************************/
// Object Creation Method

void *wavetable_new(t_symbol *msg, short argc, t_atom *argv)
{
    t_wavetable *x = (t_wavetable *)object_alloc(wavetable_class);;
	short i;
	
    if(x){
    	object_obex_store((void *)x, _sym_dumpout, (object *)outlet_new(x,NULL));	// dumpout	
	    dsp_setup((t_pxobject *)x,1);				// Create Object and 1 Inlet (last argument)
	    outlet_new((t_pxobject *)x, "signal");		// Create a signal Outlet

		x->my_wavetable->set_global_sr(sys_getsr());
		x->my_wavetable = new tt_wavetable;
		for(i=0; i<NUM_INPUTS; i++)
			x->signal_in[i] = new tt_audio_signal;
		for(i=0; i<NUM_OUTPUTS; i++)
			x->signal_out[i] = new tt_audio_signal;

		//x->my_wavetable->set_sr(sys_getsr());
		
// it should do this by default anyway...
//		x->my_wavetable->set_attr(tt_wavetable::k_mode, tt_wavetable::k_mode_sine);
		
/*		x->another_waveform = new tt_buffer;
		x->another_waveform->set_sr(sys_getsr());
		x->another_waveform->set_attr(tt_buffer::k_length_samples, 256);
		x->another_waveform->fill(tt_buffer::k_gauss, 0.125, 0.5);

	//	Just have this here to prove that I can do it - tested it and it works...
	//	x->my_wavetable->set_wavetable(x->another_waveform);
*/	
		attr_args_process(x,argc,argv);				//handle attribute args	
	}
	return (x);									// Return the pointer
}

// Memory Deallocation
void wavetable_free(t_wavetable *x)
{
	short i;
	
	dsp_free((t_pxobject *)x);
	delete x->my_wavetable;
	for(i=0; i<NUM_INPUTS; i++)
		delete x->signal_in[i];
	for(i=0; i<NUM_OUTPUTS; i++)
		delete x->signal_out[i];
//	delete x->another_waveform;
}


/************************************************************************************/
// Methods bound to input/inlets

// Method for Assistance Messages
void wavetable_assist(t_wavetable *x, void *b, long msg, long arg, char *dst)
{
	if(msg==1) 	// Inlets
		strcpy(dst, "(signal) input, control messages");		
	else if(msg==2) // Outlets
		strcpy(dst, "(signal) output");
}


// ATTRIBUTE
t_max_err attr_set_frequency(t_wavetable *x, void *attr, long argc, t_atom *argv)
{
	x->attr_frequency = atom_getfloat(argv);
	x->my_wavetable->set_attr(tt_wavetable::k_frequency, x->attr_frequency);
	
	return MAX_ERR_NONE;
	#pragma unused(attr)
}


// Perform (signal) Method: No Modulation
t_int *wavetable_perform(t_int *w)
{
   	t_wavetable *x = (t_wavetable *)(w[1]);				// Pointer
    x->signal_out[0]->set_vector((t_float *)(w[2]));	// Output
	x->signal_out[0]->vectorsize = (int)(w[3]);			// Vector Size
			
	if (!(x->x_obj.z_disabled))
		x->my_wavetable->dsp_vector_calc(x->signal_out[0]);	// no modulation input

    return (w + 4);
}


// Perform (signal) Method: With Modulation
t_int *wavetable_perform2(t_int *w)
{
   	t_wavetable *x = (t_wavetable *)(w[1]);				// Pointer
    x->signal_in[0]->set_vector((t_float *)(w[2])); 	// Input
    x->signal_out[0]->set_vector((t_float *)(w[3]));	// Output
	x->signal_in[0]->vectorsize = (int)(w[4]);			// Vector Size
			
	if (!(x->x_obj.z_disabled))
		x->my_wavetable->dsp_vector_calc(x->signal_in[0], x->signal_out[0]);	// modulated by "in" vector

    return (w + 5);
}


// ../../../../../Jamoma/Core/DSP/library/build/JamomaDSP.dylib Method
void wavetable_dsp(t_wavetable *x, t_signal **sp, short *count)
{
	x->my_wavetable->set_sr(sp[0]->s_sr);
	if(count[0])
		dsp_add(wavetable_perform2, 4, x, sp[0]->s_vec, sp[1]->s_vec, sp[0]->s_n); // Add Perform Method to the ../../../../../Jamoma/Core/DSP/library/build/JamomaDSP.dylib Call Chain
	else
		dsp_add(wavetable_perform, 3, x, sp[1]->s_vec, sp[0]->s_n);
}

