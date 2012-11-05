// MSP External: quantize~.c
// An MSP object to round the sample values of audio streams
// by Timothy Place, December 1999

#include "TTClassWrapperMax.h"		

static t_class *quantize_class;	// Required. Global pointing to this class

// Data Structure for this object
typedef struct _quantize{
	t_pxobject	x_obj;		// Header;  Must always be the first field; used by MSP
	long		arg;		// Value from the typed in argument
} t_quantize;


// Prototypes for methods: need a method for each incoming message type
void *quantize_new(long value);											// New Object Creation Method
t_int *quantize_perform(t_int *w);
void quantize_perform64(t_quantize *x, t_object *dsp64, double **ins, long numins, double **outs, long numouts, long sampleframes, long flags, void *userparam);										// An MSP Perform (signal) Method
void quantize_dsp(t_quantize *x, t_signal **sp, short *count);
void quantize_dsp64(t_quantize *x, t_object *dsp64, short *count, double samplerate, long maxvectorsize, long flags);			// DSP Method
void quantize_assist(t_quantize *x, void *b, long m, long a, char *s);	// Assistance Method


/************************************************************************************/
// Main() Function

extern "C" int TTCLASSWRAPPERMAX_EXPORT main(void)
{
	t_class *c;

	c = class_new("tap.quantize~",(method)quantize_new, (method)dsp_free, sizeof(t_quantize), 
		(method)0L, A_LONG, 0);

		common_symbols_init();
 	class_addmethod(c, (method)quantize_dsp, 			"dsp", A_CANT, 0L);		
	class_addmethod(c, (method)quantize_dsp64, "dsp64", A_CANT, 0);
	class_addmethod(c, (method)quantize_assist, 		"assist", A_CANT, 0L); 
	class_addmethod(c, (method)stdinletinfo,			"inletinfo",	A_CANT, 0);

	class_dspinit(c);
class_register(_sym_box, c); 	quantize_class = c;
}


/************************************************************************************/
// Object Creation Method

void *quantize_new(long value)
{
	t_quantize *x = (t_quantize *)object_alloc(quantize_class);;
	if(x){
	   	object_obex_store((void *)x, _sym_dumpout, (object *)outlet_new(x,NULL));	// dumpout	
		dsp_setup((t_pxobject *)x, 1);				// Create Object and 1 Inlet (last argument)
		outlet_new((t_pxobject *)x, "signal");		// Create an Outlet
		x->arg = value;								// Init the typed in argument
	}
	return x;									// Return the pointer
}


/************************************************************************************/
// Methods bound to input/inlets

// Method for Assistance Messages
void quantize_assist(t_quantize *x, void *b, long msg, long arg, char *dst)
{
	if(msg==1) 	// Inlets
		strcpy(dst, "(signal) To be quantized"); 
	else if(msg==2) // Outlets
		strcpy(dst, "(signal) Quantization Result");
}


// Perform (signal) Method
t_int *quantize_perform(t_int *w)
{
	t_float *in = (t_float *)(w[1]);   		// Input
	t_float *out = (t_float *)(w[2]);		// Output
   	t_quantize *x = (t_quantize *)(w[3]);	// Pointer
	int n = (int)(w[4]);					// Vector Size
	long arg = x->arg;
	double value;
	long temp;
		
	while (n--){
		value = *in++;
		if (value >= 0.0){						// Two separate routines for positive & negative numbers
			value = value * arg;				// shift the decimal place over
			value = value + 0.5;				// preparation for truncation
			temp = value;						// truncate - convert to int
			value = temp;						// convert back to double-float
			//incoming = incoming * 0.01;		// shift the decimal back to where it belongs - NOTE: DOES NOT YIELD "TESTABLE" SIGNAL VALUES IN MSP!?!?!?
		}else{
			value = value * arg;				// shift the decimal place over
			value = value - 0.5;				// preparation for truncation
			temp = value;						// truncate - convert to int
			value = temp;						// convert back to double-float
			//incoming = incoming * 0.01;		// shift the decimal back to where it belongs - NOTE: DOES NOT YIELD "TESTABLE" SIGNAL VALUES IN MSP!?!?!?
		}
		*out++ = value; 					// Run input through Decimal Rounding Function
	}					                       
	return (w + 5);							// Return a pointer to the NEXT object in the DSP call chain
}


void quantize_perform64(t_quantize *x, t_object *dsp64, double **ins, long numins, double **outs, long numouts, long sampleframes, long flags, void *userparam)
{
	double *in = ins[0];
	double *out = outs[0];
   	
	int n = sampleframes;
	long arg = x->arg;
	double value;
	long temp;
		
	while (n--){
		value = *in++;
		if (value >= 0.0){						// Two separate routines for positive & negative numbers
			value = value * arg;				// shift the decimal place over
			value = value + 0.5;				// preparation for truncation
			temp = value;						// truncate - convert to int
			value = temp;						// convert back to double-float
			//incoming = incoming * 0.01;		// shift the decimal back to where it belongs - NOTE: DOES NOT YIELD "TESTABLE" SIGNAL VALUES IN MSP!?!?!?
		}else{
			value = value * arg;				// shift the decimal place over
			value = value - 0.5;				// preparation for truncation
			temp = value;						// truncate - convert to int
			value = temp;						// convert back to double-float
			//incoming = incoming * 0.01;		// shift the decimal back to where it belongs - NOTE: DOES NOT YIELD "TESTABLE" SIGNAL VALUES IN MSP!?!?!?
		}
		*out++ = value; 					// Run input through Decimal Rounding Function
	}					                       
	return;
}



// DSP Method
void quantize_dsp(t_quantize *x, t_signal **sp, short *count)
{
	dsp_add(quantize_perform, 4, sp[0]->s_vec, sp[1]->s_vec, x, sp[0]->s_n); // Add Perform Method to the DSP Call Chain
}


void quantize_dsp64(t_quantize *x, t_object *dsp64, short *count, double samplerate, long maxvectorsize, long flags)
{
	object_method(dsp64, gensym("dsp_add64"), (t_object*)x, quantize_perform64, 0, NULL);
}
