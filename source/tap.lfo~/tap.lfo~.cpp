/* 
 *	External object for Max/MSP
 *	Copyright © 2004 by Timothy Place
 * 
 *	License: This code is licensed under the terms of the "New BSD License"
 *	http://creativecommons.org/licenses/BSD/
 */


#include "TTClassWrapperMax.h"
#include "../ttblue/tt_audio_base.h"
#include "../ttblue/tt_audio_signal.h"
#include "../ttblue/tt_lfo.h"

#define NUM_OUTPUTS 1

// Data Structure for this object
typedef struct _lfo{
    t_pxobject 		x_obj;
	tt_lfo			*my_lfo;
    tt_audio_signal	*signal_out[NUM_OUTPUTS];
	double			attr_frequency;
	double			attr_depth;
	t_symbol		*attr_shape;
	long			attr_mute;
} t_lfo;

// Prototypes for methods: need a method for each incoming message type
void *lfo_new(t_symbol *msg, short argc, t_atom *argv);						// New Object Creation Method
void lfo_free(t_lfo *x);
void lfo_assist(t_lfo *x, void *b, long msg, long arg, char *dst);			// Assistance Method
void lfo_reset(t_lfo *x);
void lfo_dsp(t_lfo *x, t_signal **sp, short *count);
void lfo_dsp64(t_lfo *x, t_object *dsp64, short *count, double samplerate, long maxvectorsize, long flags);						// ../../../../Jamoma/Core/DSP/library/build/JamomaDSP.dylib Method
t_int *lfo_perform(t_int *w);
void lfo_perform64(t_lfo *x, t_object *dsp64, double **ins, long numins, double **outs, long numouts, long sampleframes, long flags, void *userparam);												// An MSP Perform (signal) Method
t_max_err attr_set_frequency(t_lfo *x, void *attr, long argc, t_atom *argv);
t_max_err attr_set_depth(t_lfo *x, void *attr, long argc, t_atom *argv);
t_max_err attr_set_shape(t_lfo *x, void *attr, long argc, t_atom *argv);
t_max_err lfo_setmute(t_lfo *x, void *attr, long argc, t_atom *argv);
void lfo_anything(t_lfo *x, t_symbol *msg, long argc, t_atom *argv);

// Globals
static t_class *lfo_class;			// Required. Global pointing to this class


/************************************************************************************/
// Main() Function

extern "C" int TTCLASSWRAPPERMAX_EXPORT main(void)
{
	t_class *c;
	
	c = class_new("tap.lfo~",(method)lfo_new, (method)lfo_free, sizeof(t_lfo), 
		(method)0L, A_GIMME, 0);

		common_symbols_init();										// Initialize TapTools
 	class_addmethod(c, (method)lfo_reset,	 		"reset", 		0);		
 	class_addmethod(c, (method)lfo_dsp,	 			"dsp", 			A_CANT, 0);		
	class_addmethod(c, (method)lfo_dsp64, "dsp64", A_CANT, 0);
	class_addmethod(c, (method)lfo_assist, 			"assist", 		A_CANT, 0); 
	class_addmethod(c, (method)attr_set_frequency,	"/frequency",	A_GIMME, 0);
	class_addmethod(c, (method)attr_set_depth,		"/depth",		A_GIMME, 0);
	class_addmethod(c, (method)attr_set_shape,		"/shape",		A_GIMME, 0);
	class_addmethod(c, (method)lfo_setmute,			"/audio/mute",	A_GIMME, 0);
	class_addmethod(c, (method)lfo_anything,		"anything",		A_GIMME, 0);
	class_addmethod(c, (method)stdinletinfo,		"inletinfo",	A_CANT, 0);

	CLASS_ATTR_DOUBLE(c,		"frequency",	0,	t_lfo, attr_frequency);
	CLASS_ATTR_ACCESSORS(c,	"frequency",	NULL, attr_set_frequency);

	CLASS_ATTR_DOUBLE(c,		"depth",		0,	t_lfo, attr_depth);
	CLASS_ATTR_ACCESSORS(c,	"depth",		NULL, attr_set_depth);

	CLASS_ATTR_SYM(c,		"shape",		0,	t_lfo, attr_shape);
	CLASS_ATTR_ACCESSORS(c,	"shape",		NULL, attr_set_shape);
	CLASS_ATTR_ENUM(c,		"shape",		0,	"sine triangle square ramp sawtooth");

	class_dspinit(c);									// Setup object's class to work with MSP
class_register(_sym_box, c); 	lfo_class = c;
}


/************************************************************************************/
// Object Creation Method

void *lfo_new(t_symbol *msg, short argc, t_atom *argv)
{
    t_lfo *x = (t_lfo *)object_alloc(lfo_class);;
	short i;
    if(x){
    	object_obex_store((void *)x, _sym_dumpout, (object *)outlet_new(x, NULL));	// dumpout	
	    dsp_setup((t_pxobject *)x, 1);				// Create Object and 1 Inlet (last argument)
	    outlet_new((t_pxobject *)x, "signal");		// Create a signal Outlet

		//taptools_max::set_global_sr(sys_getsr());
		x->my_lfo = new tt_lfo;
		for(i=0; i<NUM_OUTPUTS; i++){
			x->signal_out[i] = new tt_audio_signal(1);
		}
		
		x->attr_shape = gensym("sine");
		x->attr_mute = 0;
		attr_args_process(x,argc,argv);				//handle attribute args	
	}
	return (x);									// Return the pointer
}


// Memory Deallocation
void lfo_free(t_lfo *x)
{
	short i;
	
	dsp_free((t_pxobject *)x);
	delete x->my_lfo;
	for(i=0; i<NUM_OUTPUTS; i++)
		delete x->signal_out[i];
}

/************************************************************************************/
// Methods bound to input/inlets

// Method for Assistance Messages
void lfo_assist(t_lfo *x, void *b, long msg, long arg, char *dst)
{
	if(msg==1) 	// Inlets
		strcpy(dst, "(signal) input, control messages");		
	else if(msg==2) // Outlets
		strcpy(dst, "(signal) output");
}


// RESET PHASE
void lfo_reset(t_lfo *x)
{
	x->my_lfo->phase_reset();
}


// ATTRIBUTE
t_max_err attr_set_frequency(t_lfo *x, void *attr, long argc, t_atom *argv)
{
	x->attr_frequency = atom_getfloat(argv);
	x->my_lfo->set_attr(tt_lfo::k_frequency, x->attr_frequency);
	
	return MAX_ERR_NONE;
}


// ATTRIBUTE
t_max_err attr_set_depth(t_lfo *x, void *attr, long argc, t_atom *argv)
{
	x->attr_depth = atom_getfloat(argv);
	x->my_lfo->set_attr(tt_lfo::k_depth, x->attr_depth);
	
	return MAX_ERR_NONE;
}


// ATTRIBUTE
t_max_err attr_set_shape(t_lfo *x, void *attr, long argc, t_atom *argv)
{
	if(argc && argv){
		x->attr_shape = atom_getsym(argv);
		if(x->attr_shape == gensym("sine")) 
			x->my_lfo->set_attr(tt_lfo::k_mode, tt_lfo::k_mode_sine_mod);
		else if(x->attr_shape == gensym("triangle")) 
			x->my_lfo->set_attr(tt_lfo::k_mode, tt_lfo::k_mode_triangle_mod);
		else if(x->attr_shape == gensym("square")) 
			x->my_lfo->set_attr(tt_lfo::k_mode, tt_lfo::k_mode_square_mod);
		else if(x->attr_shape == gensym("ramp")) 
			x->my_lfo->set_attr(tt_lfo::k_mode, tt_lfo::k_mode_ramp_mod);
		else if(x->attr_shape == gensym("sawtooth")) 
			x->my_lfo->set_attr(tt_lfo::k_mode, tt_lfo::k_mode_sawtooth_mod);
		else
			object_error((t_object *)x, "invalid shape specified");
	}
	return MAX_ERR_NONE;
	#pragma unused(attr)
}


t_max_err lfo_setmute(t_lfo *x, void *attr, long argc, t_atom *argv)
{
	x->attr_mute = atom_getlong(argv);
	return MAX_ERR_NONE;
}


// when used as the algorithm for a module, we use this to suppress errors for unhandles messages
void lfo_anything(t_lfo *x, t_symbol *msg, long argc, t_atom *argv)
{
	//object_post((t_object *)x, "anything: %s", msg->s_name);
}


// Perform (signal) Method: No Modulation
t_int *lfo_perform(t_int *w)
{
   	t_lfo *x = (t_lfo *)(w[1]);						// This instance
    t_float *out = (t_float *)(w[2]);
	short n = (int)(w[3]);							// Vector Size
			
	if(x->x_obj.z_disabled || x->attr_mute) 
		goto bye;

	x->my_lfo->dsp_vector_calc(x->signal_out[0]);		// no modulation input
	while(n--)
		*out++ = *x->signal_out[0]->vector;
	

bye:
    return (w + 4);
}


void lfo_perform64(t_lfo *x, t_object *dsp64, double **ins, long numins, double **outs, long numouts, long sampleframes, long flags, void *userparam)
{
   	
    double *out = outs[0];
	short n = sampleframes;
			
	if(x->x_obj.z_disabled || x->attr_mute) 
		goto bye;

	x->my_lfo->dsp_vector_calc(x->signal_out[0]);		// no modulation input
	while(n--)
		*out++ = *x->signal_out[0]->vector;
	

bye:
    return;
}



// ../../../../Jamoma/Core/DSP/library/build/JamomaDSP.dylib Method
void lfo_dsp(t_lfo *x, t_signal **sp, short *count)
{
	x->my_lfo->set_sr(sp[0]->s_sr);
	x->my_lfo->set_vectorsize(sp[0]->s_n);
	dsp_add(lfo_perform, 3, x, sp[1]->s_vec, sp[0]->s_n);
	#pragma unused(count)
}


void lfo_dsp64(t_lfo *x, t_object *dsp64, short *count, double samplerate, long maxvectorsize, long flags)
{
	x->my_lfo->set_sr(samplerate);
	x->my_lfo->set_vectorsize(maxvectorsize);
	object_method(dsp64, gensym("dsp_add64"), (t_object*)x, lfo_perform64, 0, NULL);
	#pragma unused(count)
}


