/* 
 *	External object for Max/MSP
 *	Copyright © 2003 by Timothy Place
 * 
 *	License: This code is licensed under the terms of the "New BSD License"
 *	http://creativecommons.org/licenses/BSD/
 */

#include "TTClassWrapperMax.h"
#include "../ttblue/tt_audio_base.h"
#include "../ttblue/tt_audio_signal.h"
#include "../ttblue/tt_ramp.h"

#define NUM_OUTPUTS 1

// Data Structure for this object
typedef struct _ramper{
    t_pxobject 		x_obj;
	tt_ramp			*my_ramp;
    tt_audio_signal	*signal_out[NUM_OUTPUTS];
	t_symbol		*attr_mode;
} t_ramper;

// Prototypes for methods: need a method for each incoming message type
void *ramper_new(t_symbol *msg, short argc, t_atom *argv);					// New Object Creation Method
void ramper_free(t_ramper *x);
void ramper_assist(t_ramper *x, void *b, long msg, long arg, char *dst);	// Assistance Method
void ramper_dsp(t_ramper *x, t_signal **sp, short *count);					// ../../../../Jamoma/Core/DSP/library/build/JamomaDSP.dylib Method
t_int *ramper_perform(t_int *w);											// An MSP Perform (signal) Method
void ramper_list(t_ramper *x, double end_value, double time);
void ramper_float(t_ramper *x, double current_value);
void ramper_int(t_ramper *x, long value);
t_max_err attr_set_mode(t_ramper *x, void *attr, long argc, t_atom *argv);

// Globals
static t_class		*ramper_class;			// Required. Global pointing to this class
static t_symbol	*ps_vector_accurate;
static t_symbol	*ps_sample_accurate;


/************************************************************************************/
// Main() Function

extern "C" int TTCLASSWRAPPERMAX_EXPORT main(void)
{
	t_class *c;
	
	c = class_new("tap.ramp~",(method)ramper_new, (method)ramper_free, sizeof(t_ramper), 
		(method)0L, A_GIMME, 0);

		common_symbols_init();											// Initialize TapTools
    class_addmethod(c, (method)ramper_int, 		"int", A_FLOAT, 0L);		// Input as float
    class_addmethod(c, (method)ramper_float,	"float", A_FLOAT, 0L);		// Input as float
    class_addmethod(c, (method)ramper_list,		"list", A_FLOAT, A_FLOAT);
	class_addmethod(c, (method)ramper_dsp, 		"dsp", A_CANT, 0L);		
    class_addmethod(c, (method)ramper_assist, 	"assist", A_CANT, 0L); 
	class_addmethod(c, (method)stdinletinfo,	"inletinfo",	A_CANT, 0);

	CLASS_ATTR_SYM(c,		"mode",		0,	t_ramper, attr_mode);
	CLASS_ATTR_ACCESSORS(c,	"mode",		NULL, attr_set_mode);
	CLASS_ATTR_ENUM(c,		"mode",		0,	"vector_accurate sample_accurate");
	
	class_dspinit(c);									// Setup object's class to work with MSP
class_register(_sym_box, c); 	ramper_class = c;
	
	// Init Globals
	ps_vector_accurate = gensym("vector_accurate");
	ps_sample_accurate = gensym("sample_accurate");
}


/************************************************************************************/
// Object Creation Method

void *ramper_new(t_symbol *msg, short argc, t_atom *argv)
{
    t_ramper *x;
    short i;
    
    x = (t_ramper *)object_alloc(ramper_class);
	
    if (x) {
		object_obex_store((void *)x, _sym_dumpout, (object *)outlet_new(x,NULL));	// dumpout
    	dsp_setup((t_pxobject *)x,1);				// Create Object and 1 Inlet (last argument)
	    outlet_new((t_pxobject *)x, "signal");		// Create a signal Outlet

		x->my_ramp = new tt_ramp;
		x->my_ramp->set_sr(sys_getsr());
		for(i=0; i<NUM_OUTPUTS; i++)
			x->signal_out[i] = new tt_audio_signal;
		
		x->attr_mode = ps_vector_accurate;			// default
		
		attr_args_process(x, argc, argv); 			//handle attribute args			
	}
	return (x);										// Return the pointer
}

// Memory Deallocation
void ramper_free(t_ramper *x)
{
	short i;
	
	dsp_free((t_pxobject *)x);
	delete x->my_ramp;
	for(i=0; i<NUM_OUTPUTS; i++)
		delete x->signal_out[i];
}


/************************************************************************************/
// Methods bound to input/inlets

// Method for Assistance Messages
void ramper_assist(t_ramper *x, void *b, long msg, long arg, char *dst)
{
	if(msg==1) 	// Inlets
		strcpy(dst, "(signal) input, control messages");		
	else if(msg==2) // Outlets
		strcpy(dst, "(signal) output");
}


// method
void ramper_list(t_ramper *x, double end_value, double time)
{
	x->my_ramp->set_attr(tt_ramp::k_destination_value, end_value);
	x->my_ramp->set_attr(tt_ramp::k_ramp_ms, time);
}


// method
void ramper_float(t_ramper *x, double current_value)
{
	x->my_ramp->set_attr(tt_ramp::k_current_value, current_value);
}

// method
void ramper_int(t_ramper *x, long value)
{
	ramper_float(x, value);
}


// Set mode
t_max_err attr_set_mode(t_ramper *x, void *attr, long argc, t_atom *argv)
{
	if(argc && argv){
		t_symbol *value = atom_getsym(argv);

		if(value == gensym("sample_accurate"))
			x->my_ramp->set_attr(tt_ramp::k_mode, tt_ramp::k_mode_sample_accurate);
		else if(value == gensym("vector_accurate"))
			x->my_ramp->set_attr(tt_ramp::k_mode, tt_ramp::k_mode_vector_accurate);		
		else
			object_error((t_object *)x, "bad argument for mode");
	}
	
	return MAX_ERR_NONE;
	#pragma unused(attr)
}


// Perform (signal) Method
t_int *ramper_perform(t_int *w)
{
   	t_ramper *x = (t_ramper *)(w[1]);					// Pointer
    x->signal_out[0]->set_vector((t_float *)(w[2]));	// Output
	x->signal_out[0]->vectorsize = (int)(w[3]);			// Vector Size
			
	if(!(x->x_obj.z_disabled))
		x->my_ramp->dsp_vector_calc(x->signal_out[0]);	

    return (w + 4);						// Return a pointer to the NEXT object in the ../../../../Jamoma/Core/DSP/library/build/JamomaDSP.dylib call chain
}


// ../../../../Jamoma/Core/DSP/library/build/JamomaDSP.dylib Method
void ramper_dsp(t_ramper *x, t_signal **sp, short *count)
{
	x->my_ramp->set_sr(sp[0]->s_sr);
	dsp_add(ramper_perform, 3, x, sp[0]->s_vec, sp[0]->s_n); 	// Add Perform Method to the ../../../../Jamoma/Core/DSP/library/build/JamomaDSP.dylib Call Chain
}

