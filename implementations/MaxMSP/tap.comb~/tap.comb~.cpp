// MSP External: tap.lp-comb~.c
// Comb filter with a lowpass filter in the feedback loop

#include "TTClassWrapperMax.h"
#include "../ttblue/tt_audio_base.h"
#include "../ttblue/tt_audio_signal.h"
#include "../ttblue/tt_comb.h"

#define NUM_INPUTS 2
#define NUM_OUTPUTS 1

typedef struct _comb					// Data Structure for this object
{
	t_pxobject 		x_obj;
	tt_comb			*mycomb;
    tt_audio_signal	*signal_in[NUM_INPUTS];
    tt_audio_signal	*signal_out[NUM_OUTPUTS];
	t_symbol		*attr_mode;			// Attributes...
	double			attr_feedback;
	long			attr_autoclip;
	double			attr_delay;
	double			attr_decay;
	double			attr_lowpass;
	double			attr_buffersize;
} t_comb;


// Prototypes for methods: need a method for each incoming message type:
t_int *comb_perform(t_int *w);
void comb_perform64(t_comb *x, t_object *dsp64, double **ins, long numins, double **outs, long numouts, long sampleframes, long flags, void *userparam);									// An MSP Perform (signal) Method
t_int *comb2_perform(t_int *w);
void comb2_perform64(t_comb *x, t_object *dsp64, double **ins, long numins, double **outs, long numouts, long sampleframes, long flags, void *userparam);									// An MSP Perform (signal) Method
void comb_float(t_comb *x, double phase);						//
void comb_int(t_comb *x, long n);								//
void comb_dsp(t_comb *x, t_signal **sp, short *count);
void comb_dsp64(t_comb *x, t_object *dsp64, short *count, double samplerate, long maxvectorsize, long flags);			// DSP Method
void comb_assist(t_comb *x, void *b, long m, long a, char *s);	// Assistance Method
void comb_free(t_comb *x);										// Memory Deallocation Function
void comb_deallocate(t_comb *x);								// More Memory Deallocation Function
void comb_allocate(t_comb *x, double msr);						// Memory Allocation Function
void comb_clear(t_comb *x);										// "Clear" Method
void *comb_new(t_symbol *s, short argc, t_atom *argv);			// New Object Creation Method
void comb_lp_cf(t_comb *x, double value);
void comb_autoclip(t_comb *x, long toggle);						// toggle autoclipping
t_max_err attr_set_feedback(t_comb *x, void *attr, long argc, t_atom *argv);
t_max_err attr_set_autoclip(t_comb *x, void *attr, long argc, t_atom *argv);
t_max_err attr_set_delay(t_comb *x, void *attr, long argc, t_atom *argv);
t_max_err attr_set_buffersize(t_comb *x, void *attr, long argc, t_atom *argv);
t_max_err attr_set_decay(t_comb *x, void *attr, long argc, t_atom *argv);
t_max_err attr_set_lowpass(t_comb *x, void *attr, long argc, t_atom *argv);

// Globals
static t_class*	s_comb_class;
static t_class* s_lp_comb_class;

/************************************************************************************/
// Main() Function

extern "C" int TTCLASSWRAPPERMAX_EXPORT main(void)
{
	t_class* c1;
	t_class* c2;

	c1 = class_new("tap.comb~",(method)comb_new, (method)comb_free, sizeof(t_comb), (method)0L, A_GIMME, 0);
	c2 = class_new("tap.lp-comb~",(method)comb_new, (method)comb_free, sizeof(t_comb), (method)0L, A_GIMME, 0);

	common_symbols_init();

	class_addmethod(c1, (method)comb_dsp, 				"dsp", A_CANT, 0L);		
#ifdef SUPPORT_64_BIT
	class_addmethod(c, (method)comb_dsp64, "dsp64", A_CANT, 0);
#endif
    class_addmethod(c1, (method)comb_int, 				"int", A_LONG, 0L);		// Input as int
    class_addmethod(c1, (method)comb_float, 			"float", A_FLOAT, 0L);	// Input as double
    class_addmethod(c1, (method)comb_lp_cf,				"lpass", A_FLOAT, 0L);	// for backwards compatibility
    class_addmethod(c1, (method)comb_clear,				"clear", 0L);
    class_addmethod(c1, (method)comb_assist, 			"assist", A_CANT, 0L); 
    class_addmethod(c1, (method)object_obex_dumpout, 	"dumpout", A_CANT,0);      
	class_addmethod(c1, (method)stdinletinfo,			"inletinfo",	A_CANT, 0);
	
	class_addmethod(c2, (method)comb_dsp, 				"dsp", A_CANT, 0L);
#ifdef SUPPORT_64_BIT
	class_addmethod(c, (method)comb_dsp64, "dsp64", A_CANT, 0);
#endif
    class_addmethod(c2, (method)comb_int, 				"int", A_LONG, 0L);		// Input as int
    class_addmethod(c2, (method)comb_float, 				"float", A_FLOAT, 0L);	// Input as double
    class_addmethod(c2, (method)comb_lp_cf,				"lpass", A_FLOAT, 0L);	// for backwards compatibility
    class_addmethod(c2, (method)comb_clear,				"clear", 0L);
    class_addmethod(c2, (method)comb_assist, 			"assist", A_CANT, 0L); 
    class_addmethod(c2, (method)object_obex_dumpout, 	"dumpout", A_CANT,0);      
	class_addmethod(c2, (method)stdinletinfo,			"inletinfo",	A_CANT, 0);

	CLASS_ATTR_DOUBLE(c1,		"feedback",			0,	t_comb, attr_feedback);
	CLASS_ATTR_DOUBLE(c2,		"feedback",			0,	t_comb, attr_feedback);
	CLASS_ATTR_ACCESSORS(c1,	"feedback",			NULL, attr_set_feedback);
	CLASS_ATTR_ACCESSORS(c2,	"feedback",			NULL, attr_set_feedback);

	CLASS_ATTR_LONG(c1,			"autoclip",			0,	t_comb, attr_autoclip);
	CLASS_ATTR_LONG(c2,			"autoclip",			0,	t_comb, attr_autoclip);
	CLASS_ATTR_ACCESSORS(c1,	"autoclip",			NULL, attr_set_autoclip);
	CLASS_ATTR_ACCESSORS(c2,	"autoclip",			NULL, attr_set_autoclip);
	CLASS_ATTR_STYLE(c1,		"autoclip",			0, "onoff");
	CLASS_ATTR_STYLE(c2,		"autoclip",			0, "onoff");

	CLASS_ATTR_DOUBLE(c1,		"delay",			0,	t_comb, attr_delay);
	CLASS_ATTR_DOUBLE(c2,		"delay",			0,	t_comb, attr_delay);
	CLASS_ATTR_ACCESSORS(c1,	"delay",			NULL, attr_set_delay);
	CLASS_ATTR_ACCESSORS(c2,	"delay",			NULL, attr_set_delay);

	CLASS_ATTR_DOUBLE(c1,		"decay",			0,	t_comb, attr_decay);
	CLASS_ATTR_DOUBLE(c2,		"decay",			0,	t_comb, attr_decay);
	CLASS_ATTR_ACCESSORS(c1,	"decay",			NULL, attr_set_decay);
	CLASS_ATTR_ACCESSORS(c2,	"decay",			NULL, attr_set_decay);

	CLASS_ATTR_DOUBLE(c1,		"buffersize",		0,	t_comb, attr_buffersize);
	CLASS_ATTR_DOUBLE(c2,		"buffersize",		0,	t_comb, attr_buffersize);
	CLASS_ATTR_ACCESSORS(c1,	"buffersize",		NULL, attr_set_buffersize);
	CLASS_ATTR_ACCESSORS(c2,	"buffersize",		NULL, attr_set_buffersize);

	CLASS_ATTR_DOUBLE(c1,		"lowpass",			0,	t_comb, attr_lowpass);
	CLASS_ATTR_DOUBLE(c2,		"lowpass",			0,	t_comb, attr_lowpass);
	CLASS_ATTR_ACCESSORS(c1,	"lowpass",			NULL, attr_set_lowpass);
	CLASS_ATTR_ACCESSORS(c2,	"lowpass",			NULL, attr_set_lowpass);
		
	class_dspinit(c1);									// Setup object's class to work with MSP
	class_dspinit(c2);									// Setup object's class to work with MSP
	s_comb_class = c1;    class_register(_sym_nobox, c1);
	s_lp_comb_class = c2; class_register(_sym_nobox, c2);
}


/************************************************************************************/
// Object Creation Method

void *comb_new(t_symbol *s, short argc, t_atom *argv)
{
	t_comb *x;
	long attrstart;
	long arguments[5] = {0, 0, 0, 0, 0}; 
	short i, arguments_len = 0;

	attrstart = attr_args_offset(argc, argv);	// support normal arguments for backward compatibility
	if(attrstart && argv){
		for(i=0; i<attrstart; i++)
			arguments[i] = atom_getfloat(argv+i);
		arguments_len = i;
	}

	if(s == gensym("tap.lp-comb~"))
		x = (t_comb *)object_alloc(s_lp_comb_class);
	else
		x = (t_comb *)object_alloc(s_comb_class);

	if(x){
		object_obex_store((void *)x, _sym_dumpout, (object *)outlet_new(x, NULL));	// dumpout
		dsp_setup((t_pxobject *)x, 3);
		outlet_new((t_object *)x, "signal");

		x->attr_buffersize = arguments[0];
			if(x->attr_buffersize == 0) x->attr_buffersize = 200.0;
		x->attr_delay = arguments[1];
		x->attr_feedback = arguments[2];
		x->attr_autoclip = arguments[3];
		x->attr_lowpass = arguments[4];
			if(x->attr_lowpass == 0) x->attr_lowpass = 20000.0;
	
		tt_audio_base::set_global_sr(sys_getsr());				// set taptools globals with msp's globals...
		tt_audio_base::set_global_vectorsize(sys_getblksize());
		x->mycomb = new tt_comb(x->attr_buffersize);			// create taptools comb filter object
		for(i=0; i<NUM_INPUTS; i++)
			x->signal_in[i] = new tt_audio_signal;
		for(i=0; i<NUM_OUTPUTS; i++)
			x->signal_out[i] = new tt_audio_signal;
		
		if(arguments_len >= 2) 
			x->mycomb->set_attr(tt_comb::k_feedback, x->attr_feedback);
		x->mycomb->set_attr(tt_comb::k_clip, x->attr_autoclip);
		x->mycomb->set_attr(tt_comb::k_cutoff_frequency, x->attr_lowpass);
			
		attr_args_process(x,argc,argv);							// handle attribute args
	}
	return(x);
}

// Memory Deallocation
void comb_free(t_comb *x)
{
	short i;
	
	dsp_free((t_pxobject *)x);		// THIS MUST BE FIRST
	delete x->mycomb;
	for(i=0; i<NUM_INPUTS; i++)
		delete x->signal_in[i];
	for(i=0; i<NUM_OUTPUTS; i++)
		delete x->signal_out[i];
}


/************************************************************************************/
// Methods bound to input/inlets

// Method for Assistance Messages
void comb_assist(t_comb *x, void *b, long msg, long arg, char *dst)
{
	if(msg==1){ 	// Inlets
		switch(arg){
			case 0: strcpy(dst, "(signal) Input"); break;
			case 1: strcpy(dst, "(signal/float) Delay Time in ms"); break;
			case 2: strcpy(dst, "(double) Decay Time in seconds"); break;
		}
	}
	else if(msg==2){ // Outlets
		switch(arg){
			case 0: strcpy(dst, "(signal) Filtered Output"); break;
			case 1: strcpy(dst, "dumpout"); break;
		}
	}	
}


// Method for the "clear" message
void comb_clear(t_comb *x)
{
	x->mycomb->clear();
}


// Method for lp cf
void comb_lp_cf(t_comb *x, double value)
{
	x->attr_lowpass = value;
	x->mycomb->set_attr(tt_comb::k_cutoff_frequency, x->attr_lowpass);
}


// ATTRIBUTE: feedback
t_max_err attr_set_feedback(t_comb *x, void *attr, long argc, t_atom *argv)
{
	x->attr_feedback = atom_getfloat(argv);
	x->mycomb->set_attr(tt_comb::k_feedback, x->attr_feedback);

	return MAX_ERR_NONE;
	#pragma unused(attr)
}


// ATTRIBUTE: autoclip
t_max_err attr_set_autoclip(t_comb *x, void *attr, long argc, t_atom *argv)
{
	x->attr_autoclip = atom_getlong(argv);
	x->mycomb->set_attr(tt_comb::k_clip, x->attr_autoclip);

	return MAX_ERR_NONE;
	#pragma unused(attr)
}


// ATTRIBUTE: delay
t_max_err attr_set_delay(t_comb *x, void *attr, long argc, t_atom *argv)
{
	x->attr_delay = atom_getfloat(argv);
	x->mycomb->set_attr(tt_comb::k_delay, x->attr_delay);		

	return MAX_ERR_NONE;
	#pragma unused(attr)
}


// ATTRIBUTE: buffersize
t_max_err attr_set_buffersize(t_comb *x, void *attr, long argc, t_atom *argv)
{
	x->attr_buffersize = atom_getfloat(argv);
	x->mycomb->set_attr(tt_comb::k_buffersize, x->attr_buffersize);		

	return MAX_ERR_NONE;
	#pragma unused(attr)
}


// ATTRIBUTE: decay
t_max_err attr_set_decay(t_comb *x, void *attr, long argc, t_atom *argv)
{
	x->attr_decay = atom_getfloat(argv);
	x->mycomb->set_attr(tt_comb::k_decay, x->attr_decay);		

	return MAX_ERR_NONE;
	#pragma unused(attr)
}


// ATTRIBUTE: lowpass cutoff
t_max_err attr_set_lowpass(t_comb *x, void *attr, long argc, t_atom *argv)
{
	x->attr_lowpass = atom_getfloat(argv);
	x->mycomb->set_attr(tt_comb::k_cutoff_frequency, x->attr_lowpass);		

	return MAX_ERR_NONE;
	#pragma unused(attr)
}


// Method for Int input
void comb_int(t_comb *x, long n)
{
	comb_float(x,(double)n);	// cast as double-precision-float and pass off the data
}


// Method for double input
void comb_float(t_comb *x, double value)
{
	switch (x->x_obj.z_in) {
		case 1:
			x->attr_delay = value;
			x->mycomb->set_attr(tt_comb::k_delay, x->attr_delay);
			break;
		case 2:
			x->attr_decay = value;
			x->mycomb->set_attr(tt_comb::k_decay, x->attr_decay);
			break;
	}
}


// Perform (signal) Method - delay is a constant (not a signal)
t_int *comb_perform(t_int *w)
{
	t_comb *x = (t_comb *)(w[1]);		
    x->signal_in[0]->set_vector((t_float *)(w[2])); 	// Input
    x->signal_out[0]->set_vector((t_float *)(w[3]));	// Output
	x->signal_in[0]->vectorsize = (int)(w[4]);			// Vector Size
	
	if (!(x->x_obj.z_disabled))
		x->mycomb->dsp_vector_calc(x->signal_in[0], x->signal_out[0]);
		
	return (w+5);
}

#ifdef SUPPORT_64_BIT

void comb_perform64(t_comb *x, t_object *dsp64, double **ins, long numins, double **outs, long numouts, long sampleframes, long flags, void *userparam)
{
	
    x->signal_in[0]->set_vector((t_float *)(w[2])); 	// Input
    x->signal_out[0]->set_vector((t_float *)(w[3]));	// Output
	x->signal_in[0]->vectorsize = (int)(w[4]);			// Vector Size
	
	if (!(x->x_obj.z_disabled))
		x->mycomb->dsp_vector_calc(x->signal_in[0], x->signal_out[0]);
		
	return;
}
#endif


// Perform (signal) Method - delay is a signal
t_int *comb2_perform(t_int *w)
{
	t_comb *x = (t_comb *)(w[1]);		
    t_float	*delay = ((t_float *)(w[3]));				// Input (Delay Time)
    x->signal_in[0]->set_vector((t_float *)(w[2])); 	// Input
    x->signal_out[0]->set_vector((t_float *)(w[4]));	// Output
	x->signal_in[0]->vectorsize = (int)(w[5]);			// Vector Size

	if(!(x->x_obj.z_disabled)){
		x->attr_delay = *delay;
		x->mycomb->set_attr(tt_comb::k_delay, x->attr_delay);
		x->mycomb->dsp_vector_calc(x->signal_in[0], x->signal_out[0]);
	}

	return (w+6);
}

#ifdef SUPPORT_64_BIT

void comb2_perform64(t_comb *x, t_object *dsp64, double **ins, long numins, double **outs, long numouts, long sampleframes, long flags, void *userparam)
{
	
    double	*delay = ((t_float *)(w[3]));				// Input (Delay Time)
    x->signal_in[0]->set_vector((t_float *)(w[2])); 	// Input
    x->signal_out[0]->set_vector((t_float *)(w[4]));	// Output
	x->signal_in[0]->vectorsize = (int)(w[5]);			// Vector Size

	if(!(x->x_obj.z_disabled)){
		x->attr_delay = *delay;
		x->mycomb->set_attr(tt_comb::k_delay, x->attr_delay);
		x->mycomb->dsp_vector_calc(x->signal_in[0], x->signal_out[0]);
	}

	return;
}
#endif


// DSP Method
void comb_dsp(t_comb *x, t_signal **sp, short *count)
{
	x->mycomb->set_sr(sp[0]->s_sr);
	comb_clear(x);

	if (count[1])		// delay is a signal
		dsp_add(comb2_perform, 5, x, sp[0]->s_vec, sp[1]->s_vec, sp[3]->s_vec, sp[0]->s_n);
	else				// delay is a constant (non-signal)
		dsp_add(comb_perform, 4, x, sp[0]->s_vec, sp[3]->s_vec, sp[0]->s_n);
}

#ifdef SUPPORT_64_BIT

void comb_dsp64(t_comb *x, t_object *dsp64, short *count, double samplerate, long maxvectorsize, long flags)
{
	x->mycomb->set_sr(samplerate);
	comb_clear(x);

	if (count[1])		// delay is a signal
		object_method(dsp64, gensym("dsp_add64"), (t_object*)x, comb2_perform64, 0, NULL);
	else				// delay is a constant (non-signal)
		object_method(dsp64, gensym("dsp_add64"), (t_object*)x, comb_perform64, 0, NULL);
}

#endif
