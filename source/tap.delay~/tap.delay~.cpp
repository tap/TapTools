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
#include "../ttblue/tt_delay.h"

#define NUM_INPUTS 2
#define NUM_OUTPUTS 1

// Data Structure for this object
typedef struct _mydelay{
    t_pxobject 		x_obj;
	tt_delay		*audio_delay;
    tt_audio_signal	*signal_in[NUM_INPUTS];
    tt_audio_signal	*signal_out[NUM_OUTPUTS];
	float			attr_delay;				// delay in ms
	t_symbol		*attr_interpolation;	// interp type
	float			maxDelay;
}t_mydelay;

// Prototypes for methods: need a method for each incoming message type
void *mydelay_new(t_symbol *msg, long argc, t_atom *argv);					// New Object Creation Method
t_int *mydelay_perform(t_int *w);											// An MSP Perform (signal) Method
t_int *mydelay_perform2(t_int *w);
void mydelay_dsp(t_mydelay *x, t_signal **sp, short *count);				// ../../../../Jamoma/Core/DSP/library/build/JamomaDSP.dylib Method
void mydelay_assist(t_mydelay *x, void *b, long msg, long arg, char *dst);	// Assistance Method
void mydelay_free(t_mydelay *x);
t_max_err attr_set_delay(t_mydelay *x, void *attr, long argc, t_atom *argv);
t_max_err attr_set_interpolation(t_mydelay *x, void *attr, long argc, t_atom *argv);
void mydelay_samps(t_mydelay *x, long value);

// Globals
static t_class 	*mydelay_class;	// Required. Global pointing to this class
static t_symbol	*ps_none;
static t_symbol	*ps_linear;
static t_symbol	*ps_polynomial;

/************************************************************************************/
// Main() Function

extern "C" int TTCLASSWRAPPERMAX_EXPORT main(void)
{
	t_class *c;

	c = class_new("tap.delay~",(method)mydelay_new, (method)mydelay_free, sizeof(t_mydelay), 
		(method)0L, A_GIMME, 0);

		common_symbols_init();
	class_addmethod(c, (method)mydelay_samps,	"delay_in_samples", A_LONG, 0L);
 	class_addmethod(c, (method)mydelay_dsp, 	"dsp", A_CANT, 0L);		
	class_addmethod(c, (method)mydelay_assist, 	"assist", A_CANT, 0L); 
	class_addmethod(c, (method)stdinletinfo,	"inletinfo",	A_CANT, 0);

	CLASS_ATTR_FLOAT(c,		"delay",			0,	t_mydelay, attr_delay);
	CLASS_ATTR_ACCESSORS(c,	"delay",			NULL, attr_set_delay);

	CLASS_ATTR_SYM(c,		"interpolation",	0,	t_mydelay, attr_interpolation);
	CLASS_ATTR_ACCESSORS(c,	"interpolation",	NULL, attr_set_interpolation);
	CLASS_ATTR_ENUM(c,		"interpolation",	0,	"none linear polynomial");

	class_dspinit(c);									// Setup object's class to work with MSP
class_register(_sym_box, c); 	mydelay_class = c;
	
	// Init Globals
	ps_none = gensym("none");
	ps_linear = gensym("linear");
	ps_polynomial = gensym("polynomial");
}


/************************************************************************************/
// Object Creation Method

void *mydelay_new(t_symbol *msg, long argc, t_atom *argv)
{
    t_mydelay	*x; 
	long		attrstart;
	float		buffersize_in_ms = 200.0;
	short		i;

	attrstart = attr_args_offset(argc, argv);
	if(attrstart && argv)
		atom_arg_getfloat(&buffersize_in_ms, 0, attrstart, argv);	// support a normal float argument	

	x = (t_mydelay *)object_alloc(mydelay_class);;
	if(x){
    	object_obex_store((void *)x, _sym_dumpout, (object *)outlet_new(x,NULL));	// dumpout	
	    dsp_setup((t_pxobject *)x,2);
	    outlet_new((t_pxobject *)x, "signal");			// Create a signal Outlet
		
		x->maxDelay = buffersize_in_ms;
		
		tt_audio_base::set_global_sr(sys_getsr());
		x->audio_delay = new tt_delay(buffersize_in_ms);
		for(i=0; i<NUM_INPUTS; i++)
			x->signal_in[i] = new tt_audio_signal;
		for(i=0; i<NUM_OUTPUTS; i++)
			x->signal_out[i] = new tt_audio_signal;
		
		x->attr_interpolation = ps_none;
		attr_args_process(x,argc,argv);					//handle attribute args			
	}
	return (x);									// Return the pointer
	#pragma unused(msg)
}

// Memory Deallocation
void mydelay_free(t_mydelay *x)
{
	short i;
	
	dsp_free((t_pxobject *)x);
	delete x->audio_delay;
	for(i=0; i<NUM_INPUTS; i++)
		delete x->signal_in[i];
	for(i=0; i<NUM_OUTPUTS; i++)
		delete x->signal_out[i];
}


/************************************************************************************/
// Methods bound to input/inlets

// Method for Assistance Messages
void mydelay_assist(t_mydelay *x, void *b, long msg, long arg, char *dst)
{
	if(msg==1) 	// Inlets
		strcpy(dst, "(signal) input, control messages");		
	else if(msg==2) // Outlets
		strcpy(dst, "(signal) Filtered output");
	#pragma unused(x)
	#pragma unused(b)
	#pragma unused(arg)
}



// ATTRIBUTE: delay time in ms
t_max_err attr_set_delay(t_mydelay *x, void *attr, long argc, t_atom *argv)
{
	x->attr_delay = TTClip(atom_getfloat(argv), 0.0f, x->maxDelay);
	x->audio_delay->set_attr(tt_delay::k_delay_ms, x->attr_delay);
	
	return MAX_ERR_NONE;
	#pragma unused(attr)
	#pragma unused(argc)
}


// ATTRIBUTE
t_max_err attr_set_interpolation(t_mydelay *x, void *attr, long argc, t_atom *argv)
{
	x->attr_interpolation = atom_getsym(argv);
	if(x->attr_interpolation == ps_none)
		x->audio_delay->set_attr(tt_delay::k_interpolation, tt_delay::k_interpolation_none);
	else if(x->attr_interpolation == ps_linear)
		x->audio_delay->set_attr(tt_delay::k_interpolation, tt_delay::k_interpolation_linear);
//	else if(x->attr_interpolation == ps_polynomial)
//		x->audio_delay->set_attr(tt_delay::k_interpolation, tt_delay::k_interpolation_polynomial);
	else
		object_error((t_object *)x, "invalid interpolation type specified");
		
	return MAX_ERR_NONE;
	#pragma unused(attr)
	#pragma unused(argc)
}


// directly specify a coefficient
void mydelay_samps(t_mydelay *x, long value)
{
	x->audio_delay->set_attr(tt_delay::k_delay_samples, value);
}


// Perform (signal) Method
t_int *mydelay_perform(t_int *w)
{
   	t_mydelay *x = (t_mydelay *)(w[1]);					// Pointer
    x->signal_in[0]->set_vector((t_float *)(w[2])); 	// Input
    x->signal_out[0]->set_vector((t_float *)(w[3]));	// Output
	x->signal_in[0]->vectorsize = (int)(w[4]);			// Vector Size
			
	if (!(x->x_obj.z_disabled))
		x->audio_delay->dsp_vector_calc(x->signal_in[0], x->signal_out[0]);	

    return (w + 5);		// Return a pointer to the NEXT object in the ../../../../Jamoma/Core/DSP/library/build/JamomaDSP.dylib call chain
}

// Perform (signal) Method
t_int *mydelay_perform2(t_int *w)
{
   	t_mydelay *x = (t_mydelay *)(w[1]);					// Pointer
    x->signal_in[0]->set_vector((t_float *)(w[2])); 	// Input
    x->signal_in[1]->set_vector((t_float *)(w[3])); 	// Input
    x->signal_out[0]->set_vector((t_float *)(w[4]));	// Output
	x->signal_in[0]->vectorsize = (int)(w[5]);			// Vector Size
			
	if (!(x->x_obj.z_disabled))
		x->audio_delay->dsp_vector_calc(x->signal_in[0], x->signal_in[1], x->signal_out[0]);	

    return (w + 6);		// Return a pointer to the NEXT object in the ../../../../Jamoma/Core/DSP/library/build/JamomaDSP.dylib call chain
}


// ../../../../Jamoma/Core/DSP/library/build/JamomaDSP.dylib Method
void mydelay_dsp(t_mydelay *x, t_signal **sp, short *count)
{
	x->audio_delay->set_sr(sp[0]->s_sr);
	if(count[1])
		dsp_add(mydelay_perform2, 5, x, sp[0]->s_vec, sp[1]->s_vec, sp[2]->s_vec, sp[0]->s_n);
	else
		dsp_add(mydelay_perform, 4, x, sp[0]->s_vec, sp[2]->s_vec, sp[0]->s_n); // Add Perform Method to the ../../../../Jamoma/Core/DSP/library/build/JamomaDSP.dylib Call Chain
	#pragma unused(count)
}

