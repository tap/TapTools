/* 
 *	External object for Max/MSP
 *	Copyright © 2000 by Timothy Place
 * 
 *	License: This code is licensed under the terms of the "New BSD License"
 *	http://creativecommons.org/licenses/BSD/
 */

#include "TTClassWrapperMax.h"		// stuff needed for TapTools Externals
#include "../ttblue/tt_audio_base.h"
#include "../ttblue/tt_audio_signal.h"
#include "../ttblue/tt_polar.h"

#define NUM_INPUTS 1
#define NUM_OUTPUTS 1

// Data Structure for this object
typedef struct _polar{
	t_pxobject 		x_obj;
	tt_polar		*polar;
    tt_audio_signal	*signal_in[NUM_INPUTS];
    tt_audio_signal	*signal_out[NUM_OUTPUTS];
	long			attr_mode;
} t_polar;

// Prototypes for methods: need a method for each incoming message type:
t_int *polar_perform(t_int *w);										// An MSP Perform (signal) Method
void polar_dsp(t_polar *x, t_signal **sp, short *count);			// DSP Method
void polar_assist(t_polar *x, void *b, long m, long a, char *s);	// Assistance Method
void polar_free(t_polar *x);										// Memory Deallocation Function
void *polar_new(t_symbol *s, short argc, t_atom *argv);				// New Object Creation Method
t_max_err attr_set_mode(t_polar *x, void *attr, long argc, t_atom *argv);

// Globals
static t_class *polar_class;						// Required. Global pointing to this class


/************************************************************************************/
// Main() Function

extern "C" int TTCLASSWRAPPERMAX_EXPORT main(void)
{
	t_class *c;
	
	c = class_new("tap.polar~",(method)polar_new, (method)polar_free, sizeof(t_polar), 
		(method)0L, A_GIMME, 0);

		common_symbols_init();									// Initialize TapTools
//	class_addmethod(c, (method)allpass_clear,	"clear", 0L);
 	class_addmethod(c, (method)polar_dsp, 		"dsp", A_CANT, 0L);		
	class_addmethod(c, (method)polar_assist, 	"assist", A_CANT, 0L); 

	CLASS_ATTR_LONG(c,		"mute",	0,		t_polar, attr_mode);
	CLASS_ATTR_ACCESSORS(c,	"mute",	NULL,	attr_set_mode);

	class_dspinit(c);									// Setup object's class to work with MSP
class_register(_sym_box, c); 	polar_class = c;
}

/************************************************************************************/
// Object Creation Method

void *polar_new(t_symbol *s, short argc, t_atom *argv)
{
	t_polar *x;
	long attrstart;
	long argument = 0;
	short i;

	attrstart = attr_args_offset(argc, argv);
	if (attrstart && argv)
		atom_arg_getlong(&argument, 0, attrstart, argv);	// support a normal int argument for bwc
 	
	x = (t_polar *)object_alloc(polar_class);;	// create max object
	if(x){
    	object_obex_store((void *)x, _sym_dumpout, (object *)outlet_new(x,NULL));	// dumpout	
		dsp_setup((t_pxobject *)x,2);							// 2 MSP signal inlets
		outlet_new((t_object *)x, "signal");					// outlet #2
		outlet_new((t_object *)x, "signal");					// outlet #1
		x->polar = new tt_polar;								// Tap.Tools polar object
		for(i=0; i<NUM_INPUTS; i++)
			x->signal_in[i] = new tt_audio_signal;
		for(i=0; i<NUM_OUTPUTS; i++)
			x->signal_out[i] = new tt_audio_signal;
		
		x->attr_mode = argument;			// process the backwards compatible argument
		if(x->attr_mode)
			x->polar->set_attr(tt_polar::k_mode, tt_polar::k_mode_poltocar);
		else
			x->polar->set_attr(tt_polar::k_mode, tt_polar::k_mode_cartopol);
		
		attr_args_process(x,argc,argv);				//handle attribute args							
	}
	return (x);												// return pointer to our max object
}

// Memory Deallocation
void polar_free(t_polar *x)
{
	short i;
	
	dsp_free((t_pxobject *)x);		// THIS MUST BE FIRST
	delete x->polar;
	for(i=0; i<NUM_INPUTS; i++)
		delete x->signal_in[i];
	for(i=0; i<NUM_OUTPUTS; i++)
		delete x->signal_out[i];
}


/************************************************************************************/
// Methods bound to input/inlets

// Method for Assistance Messages
void polar_assist(t_polar *x, void *b, long msg, long arg, char *dst)
{
	if(msg==1)	 	// Inlets
		strcpy(dst, "(signal) Input");
	else if(msg==2) // Outlets
		strcpy(dst, "(signal) Processed Output");

}


// ATTRIBUTE
t_max_err attr_set_mode(t_polar *x, void *attr, long argc, t_atom *argv)
{
	x->attr_mode = atom_getlong(argv);
	if(x->attr_mode)
		x->polar->set_attr(tt_polar::k_mode, tt_polar::k_mode_poltocar);
	else
		x->polar->set_attr(tt_polar::k_mode, tt_polar::k_mode_cartopol);
	
	return MAX_ERR_NONE;
	#pragma unused(attr)
}


// Perform (signal) Method - delay is a signal
t_int *polar_perform(t_int *w)
{
	t_polar *x = (t_polar *)(w[1]);		
	x->signal_in[0]->set_vector((t_float *)(w[2])); 	// Input
	x->signal_in[1]->set_vector((t_float *)(w[3])); 	// Input
    x->signal_out[0]->set_vector((t_float *)(w[4]));	// Output
    x->signal_out[1]->set_vector((t_float *)(w[5]));	// Output
	x->signal_in[0]->vectorsize = (int)(w[6]);			// Vector Size

	if(!(x->x_obj.z_disabled)){
		x->polar->dsp_vector_calc(x->signal_in[0], x->signal_in[1], x->signal_out[0], x->signal_out[1]);
	}
	return (w+7);
}


// DSP Method
void polar_dsp(t_polar *x, t_signal **sp, short *count)
{
	x->polar->set_sr(sp[0]->s_sr);	
    dsp_add(polar_perform, 6, x, sp[0]->s_vec, sp[1]->s_vec, sp[2]->s_vec, sp[3]->s_vec, sp[0]->s_n);
    
    #pragma unused(count)
}

