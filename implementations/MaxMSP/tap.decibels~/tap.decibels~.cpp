/* 
 *	External object for Max/MSP
 *	Copyright © 2000 by Timothy Place
 * 
 *	License: This code is licensed under the terms of the "New BSD License"
 *	http://creativecommons.org/licenses/BSD/
 */

#include "TTClassWrapperMax.h"
#include "../ttblue/tt_audio_base.h"


typedef struct _decibels {
	t_pxobject 	x_obj;				// This object - must be first
	void 		*decibels_out;		// Floating-point outlet
	int 		moden;				// attribute: mode (currently stored as a simple number)
	t_symbol	*attr_mode;
} t_decibels;


// Class Globals
static t_class 	*decibels_class;			// Required. Global pointing to this class
static t_symbol	*ps_amp2db, *ps_db2amp, *ps_mm2db, *ps_db2mm, *ps_mm2amp, *ps_amp2mm, *ps_db2midi, *ps_midi2db;

// Prototypes for methods: need a method for each incoming message type:
t_int *decibels_perform(t_int *w);
void decibels_perform64(t_decibels *x, t_object *dsp64, double **ins, long numins, double **outs, long numouts, long sampleframes, long flags, void *userparam);										// An MSP Perform (signal) Method
void decibels_dsp(t_decibels *x, t_signal **sp, short *count);
void decibels_dsp64(t_decibels *x, t_object *dsp64, short *count, double samplerate, long maxvectorsize, long flags);			// ../../../../../Jamoma/Core/DSP/library/build/JamomaDSP.dylib Method
void decibels_assist(t_decibels *x, void *b, long m, long a, char *s);	// Assistance Method
void *decibels_new(t_symbol *msg, short argc, t_atom *argv);			// New Object Creation Method
void decibels_float(t_decibels *x, double value);				// Float method
void decibels_int(t_decibels *x, long value);
t_max_err attr_set_mode(t_decibels *x, void *attr, long argc, t_atom *argv);
double decibels_calc(t_decibels *x, double value);


/************************************************************************************/
// Main() Function

extern "C" int TTCLASSWRAPPERMAX_EXPORT main(void)
{
	t_class *c;

	c = class_new("tap.decibels~",(method)decibels_new, (method)dsp_free, sizeof(t_decibels), 
		(method)0L, A_GIMME, 0);

		common_symbols_init();
	class_addmethod(c, (method)decibels_dsp, 			"dsp", A_CANT, 0L);		
	class_addmethod(c, (method)decibels_dsp64, "dsp64", A_CANT, 0);
    class_addmethod(c, (method)decibels_int, 			"int", A_LONG, 0L);		// Input as int
    class_addmethod(c, (method)decibels_float, 			"float", A_FLOAT, 0L);	// Input as double
    class_addmethod(c, (method)decibels_assist, 		"assist", A_CANT, 0L); 
	class_addmethod(c, (method)stdinletinfo,			"inletinfo",	A_CANT, 0);

	CLASS_ATTR_SYM(c,		"mode",			0,	t_decibels, attr_mode);
	CLASS_ATTR_ACCESSORS(c,	"mode",			NULL, attr_set_mode);
	CLASS_ATTR_ENUM(c,		"mode",			0,	"amp2db db2amp mm2db db2mm mm2amp amp2mm db2midi midi2db");

	class_dspinit(c);									// Setup object's class to work with MSP
class_register(_sym_box, c); 	decibels_class = c;

	// initialize class globals
	ps_amp2db = gensym("amp2db");
	ps_db2amp = gensym("db2amp");
	ps_mm2db = gensym("mm2db");
	ps_db2mm = gensym("db2mm");
	ps_mm2amp = gensym("mm2amp");
	ps_amp2mm = gensym("amp2mm");
	ps_db2midi = gensym("db2midi");
	ps_midi2db = gensym("midi2db");
}


/************************************************************************************/
// Object Creation Method

void *decibels_new(t_symbol *msg, short argc, t_atom *argv)
{
	t_decibels *x;
	long attrstart;
	long argument = 0;

	attrstart = attr_args_offset(argc, argv);
	if (attrstart && argv)
		atom_arg_getlong(&argument, 0, attrstart, argv);	// support a normal int argument for bwc

	if (x = (t_decibels *)object_alloc(decibels_class)) {
		object_obex_store((void *)x, _sym_dumpout, (object *)outlet_new(x,NULL));	// dumpout
		dsp_setup((t_pxobject *)x,1);
		
    	x->decibels_out = floatout(x);						// Create a floating-point Outlet
		outlet_new((t_object *)x, "signal");
		
		
		// Handle Arguments...		
		if (msg == gensym("tap.atodb~")){
			x->moden = 0;
			object_post((t_object *)x, "     tap.decibels~ initialized in mode 0");
		}
		else if (msg == gensym("tap.dbtoa~")){
			x->moden = 1;
			object_post((t_object *)x, "     tap.decibels~ initialized in mode 1");
		}
		else x->moden = argument;

		switch(x->moden){
			case 0:	x->attr_mode = ps_amp2db; break;
			case 1: x->attr_mode = ps_db2amp; break;
			case 2: x->attr_mode = ps_mm2db; break;
			case 3: x->attr_mode = ps_db2mm; break;
			case 4: x->attr_mode = ps_mm2amp; break;
			case 5: x->attr_mode = ps_amp2mm; break;
			case 6: x->attr_mode = ps_db2midi; break;
			case 7: x->attr_mode = ps_midi2db; break;
		}
		attr_args_process(x,argc,argv); //handle attribute args	
	}
	return (x);
}


/************************************************************************************/
// Methods bound to input/inlets

// Method for Assistance Messages
void decibels_assist(t_decibels *x, void *b, long msg, long arg, char *dst)
{
	if(msg==1) 		// Inlets
		strcpy(dst, "(signal/float) Input");
	else if(msg==2){ 	// Outlets
		switch(arg){
			case 0: strcpy(dst, "(signal) Output"); break;
			case 1: strcpy(dst, "(double) Output"); break;
			case 2: strcpy(dst, "dumpout"); break;
		}
	}
}


// Set mode
t_max_err attr_set_mode(t_decibels *x, void *attr, long argc, t_atom *argv)
{
	if(argc && argv){
		switch(argv[0].a_type){
			case A_LONG:		// Accepts int arg for backward compatibility with TapTools 1.x
				x->moden = atom_getlong(argv);
				switch(x->moden){
					case 0:	x->attr_mode = ps_amp2db; break;
					case 1: x->attr_mode = ps_db2amp; break;
					case 2: x->attr_mode = ps_mm2db; break;
					case 3: x->attr_mode = ps_db2mm; break;
					case 4: x->attr_mode = ps_mm2amp; break;
					case 5: x->attr_mode = ps_amp2mm; break;
					case 6: x->attr_mode = ps_db2midi; break;
					case 7: x->attr_mode = ps_midi2db; break;
				}
				break;
			case A_SYM:			// Symbol arg is the newer (and prefered) way to do it
				x->attr_mode = atom_getsym(argv);
				if(x->attr_mode == ps_amp2db) x->moden = 0;
				else if(x->attr_mode == ps_db2amp) x->moden = 1;
				else if(x->attr_mode == ps_mm2db) x->moden = 2;
				else if(x->attr_mode == ps_db2mm) x->moden = 3;
				else if(x->attr_mode == ps_mm2amp) x->moden = 4;
				else if(x->attr_mode == ps_amp2mm) x->moden = 5;
				else if(x->attr_mode == ps_db2midi) x->moden = 6;
				else if(x->attr_mode == ps_midi2db) x->moden = 7;
				else object_error((t_object *)x, "invalid mode specified");
				break;
		}
	}
	return MAX_ERR_NONE;
	#pragma unused(attr)
}


// method for double input
void decibels_float(t_decibels *x, double value)
{
	outlet_float(x->decibels_out, decibels_calc(x, value));	// output the result							
}


// method for int input
void decibels_int(t_decibels *x, long value)
{
	decibels_float(x, (double)value);
}


// Calculate it
double decibels_calc(t_decibels *x, double value)
{
	switch(x->moden){
		case 0:
			return tt_audio_base::amplitude_to_decibels(value);
		case 1:
			return tt_audio_base::decibels_to_amplitude(value);
		case 2:
			return tt_audio_base::millimeters_to_decibels(value);
		case 3:
			return tt_audio_base::decibels_to_millimeters(value);
		case 4:
			return tt_audio_base::millimeters_to_amplitude(value);
		case 5:
			return tt_audio_base::amplitude_to_millimeters(value);
		case 6:
			return tt_audio_base::decibels_to_xmidi(value);
		case 7: 
			return tt_audio_base::xmidi_to_decibels(value);
	}
	return 0;	// if all else fails...
}


// Perform (signal) Method - delay is a constant (not a signal)
t_int *decibels_perform(t_int *w)
{
	t_decibels *x = (t_decibels *)(w[1]);		
	t_float *in = (t_float *)(w[2]);
	t_float *out = (t_float *)(w[3]);
	int n = (int)(w[4]);
	
	if (x->x_obj.z_disabled) goto out;
		
	while(n--) 
		*out++ = decibels_calc(x, *in++);					

out:
	return (w+5);
}


void decibels_perform64(t_decibels *x, t_object *dsp64, double **ins, long numins, double **outs, long numouts, long sampleframes, long flags, void *userparam)
{
	
	double *in = ins[0];
	double *out = outs[0];
	int n = sampleframes;
	
	if (x->x_obj.z_disabled) goto out;
		
	while(n--) 
		*out++ = decibels_calc(x, *in++);					

out:
	return;
}



// ../../../../../Jamoma/Core/DSP/library/build/JamomaDSP.dylib Method
void decibels_dsp(t_decibels *x, t_signal **sp, short *count)
{
	if (count[0] && count[1])
   		dsp_add(decibels_perform, 4, x, sp[0]->s_vec, sp[1]->s_vec, sp[0]->s_n);
}


void decibels_dsp64(t_decibels *x, t_object *dsp64, short *count, double samplerate, long maxvectorsize, long flags)
{
	if (count[0] && count[1])
   		object_method(dsp64, gensym("dsp_add64"), (t_object*)x, decibels_perform64, 0, NULL);
}

