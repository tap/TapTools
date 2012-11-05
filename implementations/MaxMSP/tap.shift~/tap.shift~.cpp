// MSP External: tap.shift~
// pitch shifter

#include "TTClassWrapperMax.h"
#include "../ttblue/tt_audio_base.h"
#include "../ttblue/tt_audio_signal.h"
#include "../ttblue/tt_delay.h"
#include "../ttblue/tt_phasor.h"
#include "../ttblue/tt_buffer.h"
#include "../ttblue/tt_buffer_window.h"
#include "../ttblue/tt_gain.h"
#include "../ttblue/tt_offset.h"
#include "../ttblue/tt_onewrap.h"
#include "../ttblue/tt_add.h"
#include "../ttblue/tt_shift.h"

#define NUM_INPUTS 1
#define NUM_OUTPUTS 1

// Data Structure for this object
typedef struct _shift{
    t_pxobject 			x_obj;
	tt_shift			*shifter;
    tt_audio_signal		*signal_in[NUM_INPUTS];
    tt_audio_signal		*signal_out[NUM_OUTPUTS];
	double				attr_window_size;	// ATTRIBUTE
	double				attr_ratio;			// ATTRIBUTE
} t_shift;

// Prototypes for methods: need a method for each incoming message type
void *shift_new(t_symbol *msg, long argc, t_atom *argv);				// New Object Creation Method
t_int *shift_perform(t_int *w);
void shift_perform64(t_shift *x, t_object *dsp64, double **ins, long numins, double **outs, long numouts, long sampleframes, long flags, void *userparam);											// An MSP Perform (signal) Method
void shift_dsp(t_shift *x, t_signal **sp, short *count);
void shift_dsp64(t_shift *x, t_object *dsp64, short *count, double samplerate, long maxvectorsize, long flags);				// DSP Method
void shift_assist(t_shift *x, void *b, long msg, long arg, char *dst);	// Assistance Method
void shift_free(t_shift *x);
t_max_err attr_set_ratio(t_shift *x, void *attr, long argc, t_atom *argv);
t_max_err attr_set_window_size(t_shift *x, void *attr, long argc, t_atom *argv);
void shift_int(t_shift *x, long value);
void shift_float(t_shift *x, double value);
void shift_clear(t_shift *x);

// Globals
static t_class *shift_class;		// Required. Global pointing to this class


/************************************************************************************/
// Main() Function

extern "C" int TTCLASSWRAPPERMAX_EXPORT main(void)
{
	t_class *c;
	
	c = class_new("tap.shift~",(method)shift_new, (method)shift_free, sizeof(t_shift), 
		(method)0L, A_GIMME, 0);

		common_symbols_init();								// Initialize TapTools

	// ADD ATTRIBUTES
	CLASS_ATTR_DOUBLE(c,		"ratio",			0,	t_shift, attr_ratio);
	CLASS_ATTR_ACCESSORS(c,	"ratio",			NULL, attr_set_ratio);

	CLASS_ATTR_DOUBLE(c,		"window_size",		0,	t_shift, attr_window_size);
	CLASS_ATTR_ACCESSORS(c,	"window_size",		NULL, attr_set_window_size);

	// ADD METHODS
	class_addmethod(c, (method)shift_float, 	"float", 	A_FLOAT, 0L);
	class_addmethod(c, (method)shift_int,	 	"int", 		A_LONG, 0L);
	class_addmethod(c, (method)shift_clear, 	"clear", 	0L);
	class_addmethod(c, (method)shift_dsp, 		"dsp", 		A_CANT, 0L);		
#ifdef SUPPORT_64_BIT
	class_addmethod(c, (method)shift_dsp64, "dsp64", A_CANT, 0);
#endif
    class_addmethod(c, (method)shift_assist, 	"assist", 	A_CANT, 0L); 
	class_addmethod(c, (method)stdinletinfo,	"inletinfo",	A_CANT, 0);

	// WRAP UP
    class_dspinit(c);									// Setup object's class to work with MSP
class_register(_sym_box, c); 	shift_class = c;
}


/************************************************************************************/
// Object Creation Method

void *shift_new(t_symbol *msg, long argc, t_atom *argv)
{
    t_shift *x = (t_shift *)object_alloc(shift_class);;
    short i;
    
    if(x){
    	object_obex_store((void *)x, _sym_dumpout, (object *)outlet_new(x,NULL));	// dumpout	
	    dsp_setup((t_pxobject *)x, 3);				// Create Object and 3 Inlets
	    x->x_obj.z_misc = Z_NO_INPLACE;  			// ESSENTIAL?
	    outlet_new((t_pxobject *)x, "signal");		// Create a signal Outlet
								
		tt_audio_base::set_global_sr(sys_getsr());	// Set Tap.Tools global SR...
		tt_audio_base::set_global_vectorsize(sys_getblksize());	// Set Tap.Tools global vector size...

		x->shifter = new tt_shift;
		for(i=0; i<NUM_INPUTS; i++)
			x->signal_in[i] = new tt_audio_signal;
		for(i=0; i<NUM_OUTPUTS; i++)
			x->signal_out[i] = new tt_audio_signal;
		
		// Set initial state
		x->attr_ratio = 1.0;
		x->attr_window_size = 87.0;

		attr_args_process(x,argc,argv);			//handle attribute args	
	}
	return (x);									// Return the pointer
}

// Memory Deallocation
void shift_free(t_shift *x)
{
	short i;
	
	dsp_free((t_pxobject *)x);
	delete x->shifter;
	for(i=0; i<NUM_INPUTS; i++)
		delete x->signal_in[i];
	for(i=0; i<NUM_OUTPUTS; i++)
		delete x->signal_out[i];
}


/************************************************************************************/
// Methods bound to input/inlets

// Method for Assistance Messages
void shift_assist(t_shift *x, void *b, long msg, long arg, char *dst)
{
	if(msg==1){ 		// Inlets
		switch(arg){
			case 0: strcpy(dst, "(signal) input"); break;
			case 1: strcpy(dst, "(double) pitch-shift ratio"); break;
			case 2: strcpy(dst, "(double) window size in ms"); break;
		}
	}
	else if(msg==2){ 	// Outlets
		switch(arg){
			case 0: strcpy(dst, "(signal) Filtered Output"); break;
			case 1: strcpy(dst, "dumpout"); break;
		}
	}	

	#pragma unused(x)
	#pragma unused(b)
}


// METHOD: old fashioned inlet input
void shift_int(t_shift *x, long value)
{
	shift_float(x, value);
}

void shift_float(t_shift *x, double value)
{
	switch (x->x_obj.z_in) {
		case 1:
			x->attr_ratio = value;
			x->shifter->set_attr(tt_shift::k_ratio, x->attr_ratio);
			break;
		case 2:
			x->attr_window_size = value;
			x->shifter->set_attr(tt_shift::k_windowsize, x->attr_window_size);
			break;
	}
}


// ATTRIBUTE
t_max_err attr_set_ratio(t_shift *x, void *attr, long argc, t_atom *argv)
{
	if(argc && argv){
		x->attr_ratio = atom_getfloat(argv);
		x->shifter->set_attr(tt_shift::k_ratio, x->attr_ratio);
	}
	return MAX_ERR_NONE;
	#pragma unused(attr)	
}

// ATTRIBUTE
t_max_err attr_set_window_size(t_shift *x, void *attr, long argc, t_atom *argv)
{
	if(argc && argv){
		x->attr_window_size = atom_getfloat(argv);
		x->shifter->set_attr(tt_shift::k_windowsize, x->attr_window_size);
	}
	return MAX_ERR_NONE;
	#pragma unused(attr)	
}


// MESSAGE
void shift_clear(t_shift *x)
{
	x->shifter->clear();
}


// Perform (signal) Method
t_int *shift_perform(t_int *w)
{
   	t_shift *x = (t_shift *)(w[1]);						// Pointer
    x->signal_in[0]->set_vector((t_float *)(w[2])); 	// Input
    x->signal_out[0]->set_vector((t_float *)(w[3]));	// Output
	x->signal_in[0]->vectorsize = (int)(w[4]);			// Vector Size
	
	if(x->x_obj.z_disabled) goto out;
	
	x->shifter->dsp_vector_calc(x->signal_in[0], x->signal_out[0]);

out:
    return (w + 5);		// Return a pointer to the NEXT object in the DSP call chain
}


#ifdef SUPPORT_64_BIT
void shift_perform64(t_shift *x, t_object *dsp64, double **ins, long numins, double **outs, long numouts, long sampleframes, long flags, void *userparam)
{
   	
    x->signal_in[0]->set_vector((t_float *)(w[2])); 	// Input
    x->signal_out[0]->set_vector((t_float *)(w[3]));	// Output
	x->signal_in[0]->vectorsize = (int)(w[4]);			// Vector Size
	
	if(x->x_obj.z_disabled) goto out;
	
	x->shifter->dsp_vector_calc(x->signal_in[0], x->signal_out[0]);

out:
    return;
}
#endif


// DSP Method
void shift_dsp(t_shift *x, t_signal **sp, short *count)
{
	x->shifter->set_sr(sp[0]->s_sr);
	x->shifter->set_vectorsize(sp[0]->s_n);

	dsp_add(shift_perform, 4, x, sp[0]->s_vec, sp[3]->s_vec, sp[0]->s_n); // Add Perform Method to the DSP Call Chain
	#pragma unused(count)
}


#ifdef SUPPORT_64_BIT
void shift_dsp64(t_shift *x, t_object *dsp64, short *count, double samplerate, long maxvectorsize, long flags)
{
	x->shifter->set_sr(samplerate);
	x->shifter->set_vectorsize(maxvectorsize);

	object_method(dsp64, gensym("dsp_add64"), (t_object*)x, shift_perform64, 0, NULL);
}
#endif

