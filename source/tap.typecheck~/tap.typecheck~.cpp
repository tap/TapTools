/* 
 *	External object for Max/MSP
 *	Copyright © 2001 by Timothy Place
 * 
 *	License: This code is licensed under the terms of the "New BSD License"
 *	http://creativecommons.org/licenses/BSD/
 */

#include "TTClassWrapperMax.h"			

static t_class *typecheck_tilde_class;				// Required. Global pointing to this class

typedef struct _tc			// Data Structure for this object
{
	t_pxobject	x_obj;			// This object - must be first
	void 		*bangOut;		// bang outlet
	void 		*intOut;		// int outlet
	void 		*floatOut;		// Floating-point outlet
	void 		*symbolOut;		// symbol outlet
	void 		*listOut;		// list outlet
	void 		*legallistOut;	// legal-list outlet
} t_tc;


// Prototypes for methods: need a method for each incoming message type:
void typecheck_tilde_perform64(t_tc *x, t_object *dsp64, double **ins, long numins, double **outs, long numouts, long sampleframes, long flags, void *userparam);
void typecheck_tilde_dsp64(t_tc *x, t_object *dsp64, short *count, double samplerate, long maxvectorsize, long flags);				// ../../../../Jamoma/Core/DSP/library/build/JamomaDSP.dylib Method
void typecheck_tilde_assist(t_tc *x, void *b, long m, long a, char *s);		// Assistance Method
void *typecheck_tilde_new(void);												// New Object Creation Method
void typecheck_tilde_bang(t_tc *x);											// Bang method
void typecheck_tilde_int(t_tc *x, long value);								// Int method
void typecheck_tilde_float(t_tc *x, double value);							// Float method
void typecheck_tilde_symbol(t_tc *x, Symbol *msg, short argc, Atom *argv);	// Symbol method
void typecheck_tilde_list(t_tc *x, Symbol *msg, short argc, Atom *argv);		// List method


/************************************************************************************/
// Main() Function

extern "C" int TTCLASSWRAPPERMAX_EXPORT main(void)
{
	t_class *c;

	c = class_new("tap.typecheck~",(method)typecheck_tilde_new, (method)dsp_free, sizeof(t_tc), 
		(method)0L, 0L, 0);	// No args for this object

		common_symbols_init();									// Initialize TapTools

	class_addmethod(c, (method)typecheck_tilde_dsp64, "dsp64", A_CANT, 0);
	class_addmethod(c, (method)typecheck_tilde_bang,		"bang", 0L);			// bang input
    class_addmethod(c, (method)typecheck_tilde_int, 		"int", A_LONG, 0L);		// Input as int
    class_addmethod(c, (method)typecheck_tilde_float, 	"float", A_FLOAT, 0L);	// Input as double
    class_addmethod(c, (method)typecheck_tilde_list,		"list", A_GIMME, 0L);
    class_addmethod(c, (method)typecheck_tilde_symbol,	"anything", A_GIMME, 0L);
    class_addmethod(c, (method)typecheck_tilde_assist, 	"assist", A_CANT, 0L); 
	class_addmethod(c, (method)stdinletinfo,			"inletinfo",	A_CANT, 0);

	class_dspinit(c);									// Setup object's class to work with MSP
class_register(_sym_box, c); 	typecheck_tilde_class = c;
}


/************************************************************************************/
// Object Creation Method

void *typecheck_tilde_new(void)
{
	t_tc *x = (t_tc *)object_alloc(typecheck_tilde_class);;
	
	if(x){
		object_obex_store((void *)x, _sym_dumpout, (object *)outlet_new(x,NULL));	// dumpout
		dsp_setup((t_pxobject *)x,1);
		x->legallistOut = listout(x);			// legal list outlet (last outlet)
	    x->symbolOut = outlet_new(x, 0);	
	    x->listOut = listout(x);				// original list outlet (used to output both legal and illegal lists)
	    x->floatOut = floatout(x);
		x->intOut = intout(x);
		x->bangOut = bangout(x);
		outlet_new((t_object *)x, "signal");	// first outlet (signal)
	}
	return (x);
}


/************************************************************************************/
// Methods bound to input/inlets

// Method for Assistance Messages
void typecheck_tilde_assist(t_tc *x, void *b, long msg, long arg, char *dst)
{
	if(msg==1) 		// Inlets
		strcpy(dst, "Any Input");
	else if(msg==2){ 	// Outlets
		switch(arg){
			case 0: strcpy(dst, "(signal) Output"); break;
			case 1: strcpy(dst, "(bang) Output"); break;
			case 2: strcpy(dst, "(int) Output"); break;
			case 3: strcpy(dst, "(double) Output"); break;
			case 4: strcpy(dst, "(list) Output"); break;
			case 5: strcpy(dst, "(symbol) Output"); break;
			case 6: strcpy(dst, "(list) legal-list Output"); break;
		}
	}
}


void typecheck_tilde_bang(t_tc *x)
{
	outlet_bang(x->bangOut);			// output the result		
}


void typecheck_tilde_int(t_tc *x, long value)
{
	outlet_int(x->intOut, value);		// output the result		
}


void typecheck_tilde_float(t_tc *x, double value)
{
	outlet_float(x->floatOut, value);		// output the result		
}


void typecheck_tilde_list(t_tc *x, Symbol *msg, short argc, Atom *argv)
{
	outlet_list(x->legallistOut, 0L, argc, argv);		// output the result
}


void typecheck_tilde_symbol(t_tc *x, Symbol *msg, short argc, Atom *argv)
{
	if(argc == 0)
		outlet_anything(x->symbolOut, msg, argc, argv);	// output the result
	else
		outlet_anything(x->listOut, msg, argc, argv);		// output the result	
}


void typecheck_tilde_perform64(t_tc *x, t_object *dsp64, double **ins, long numins, double **outs, long numouts, long sampleframes, long flags, void *userparam)
{
	double *in = ins[0];
	double *out = outs[0];
			
	while (sampleframes--) {
		*out++ = *in++;
	}
}


void typecheck_tilde_dsp64(t_tc *x, t_object *dsp64, short *count, double samplerate, long maxvectorsize, long flags)
{
	if (count[0])
   		object_method(dsp64, gensym("dsp_add64"), (t_object*)x, typecheck_tilde_perform64, 0, NULL);
   	else
   		;
}
