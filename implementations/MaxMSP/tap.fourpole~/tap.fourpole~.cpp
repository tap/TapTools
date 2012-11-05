// MSP External:
// 4 pole lowpass filter

#include "TTClassWrapperMax.h"	// Required for all TapTools External Objects
#include "../ttblue/tt_audio_base.h"
#include "../ttblue/tt_audio_signal.h"
#include "../ttblue/tt_lowpass_fourpole.h"

#define NUM_INPUTS 1
#define NUM_OUTPUTS 1

// Data Structure for this object
typedef struct _fourpole
{
    t_pxobject 				x_obj;
	tt_lowpass_fourpole		*myFilter;
    tt_audio_signal			*signal_in[NUM_INPUTS];
    tt_audio_signal			*signal_out[NUM_OUTPUTS];
	float					attr_frequency;
	float					attr_resonance;
} t_fourpole;

// Prototypes for methods: need a method for each incoming message type
void *fourpole_new(t_symbol *msg, short argc, t_atom *argv);				// New Object Creation Method
t_int *fourpole_perform(t_int *w);											// An MSP Perform (signal) Method
t_int *fourpole_perform2(t_int *w);
void fourpole_dsp(t_fourpole *x, t_signal **sp, short *count);				// DSP Method
void fourpole_assist(t_fourpole *x, void *b, long msg, long arg, char *dst);	// Assistance Method
void fourpole_free(t_fourpole *x);
void fourpole_clear(t_fourpole *x);
t_max_err attr_set_frequency(t_fourpole *x, void *attr, long argc, t_atom *argv);
t_max_err attr_set_resonance(t_fourpole *x, void *attr, long argc, t_atom *argv);

// Global
static t_class *fourpole_class;				// Required. Global pointing to this class


/************************************************************************************/
// Main() Function

extern "C" int TTCLASSWRAPPERMAX_EXPORT main(void)
{
	long attrflags = 0;
	t_class *c;
	t_object *attr;

	c = class_new("tap.fourpole~",(method)fourpole_new, (method)fourpole_free, sizeof(t_fourpole), 
		(method)0L, A_GIMME, 0);

		common_symbols_init();														// Initialize TapTools
	class_addmethod(c, (method)fourpole_clear,			"clear", 0L);
 	class_addmethod(c, (method)fourpole_dsp, 			"dsp", A_CANT, 0L);		
	class_addmethod(c, (method)fourpole_assist, 		"assist", A_CANT, 0L); 

/*
	CLASS_ATTR_FLOAT(c,		"frequency",	0,	t_lfo, attr_frequency);
	CLASS_ATTR_ACCESSORS(c,	"frequency",	NULL, attr_set_frequency);
*/

	attr = attr_offset_new("frequency", _sym_float32, attrflags,
		(method)0L,(method)attr_set_frequency, calcoffset(t_fourpole, attr_frequency));
	class_addattr(c, attr);
	
	attr = attr_offset_new("resonance", _sym_float32, attrflags,
		(method)0L,(method)attr_set_resonance, calcoffset(t_fourpole, attr_resonance));
	class_addattr(c, attr);	

	class_dspinit(c);									// Setup object's class to work with MSP
class_register(_sym_box, c); 	fourpole_class = c;
}


/************************************************************************************/
// Object Creation Method

void *fourpole_new(t_symbol *msg, short argc, t_atom *argv)
{
    t_fourpole *x = (t_fourpole *)object_alloc(fourpole_class);;
	short i;
	
    if(x){
    	object_obex_store((void *)x, _sym_dumpout, (object *)outlet_new(x,NULL));	// dumpout	
	    dsp_setup((t_pxobject *)x, 2);				// Create Object and 2 inlets
	    outlet_new((t_pxobject *)x, "signal");		// Create a signal Outlet

		x->myFilter = new tt_lowpass_fourpole;
		for(i=0; i<NUM_INPUTS; i++)
			x->signal_in[i] = new tt_audio_signal;
		for(i=0; i<NUM_OUTPUTS; i++)
			x->signal_out[i] = new tt_audio_signal;

		x->myFilter->set_sr(sys_getsr());
		attr_args_process(x,argc,argv);				//handle attribute args					
	}
	return (x);										// Return the pointer
	#pragma unused(msg)
}

// Memory Deallocation
void fourpole_free(t_fourpole *x)
{
	short i;
	
	dsp_free((t_pxobject *)x);
	delete x->myFilter;
	for(i=0; i<NUM_INPUTS; i++)
		delete x->signal_in[i];
	for(i=0; i<NUM_OUTPUTS; i++)
		delete x->signal_out[i];
}


/************************************************************************************/
// Methods bound to input/inlets

// Method for Assistance Messages
void fourpole_assist(t_fourpole *x, void *b, long msg, long arg, char *dst)
{
	if(msg==1) 	// Inlets
		strcpy(dst, "(signal) input, control messages");		
	else if(msg==2) // Outlets
		strcpy(dst, "(signal) Filtered output");
	#pragma unused(x)
	#pragma unused(b)
	#pragma unused(arg)
}


// Method to reset the filter
void fourpole_clear(t_fourpole *x)
{
	x->myFilter->clear();
}


// ATTRIBUTE
t_max_err attr_set_frequency(t_fourpole *x, void *attr, long argc, t_atom *argv)
{
	x->attr_frequency = atom_getfloat(argv);
	x->myFilter->set_attr(tt_lowpass_fourpole::k_frequency, x->attr_frequency);
	
	return MAX_ERR_NONE;
	#pragma unused(attr)
}

// ATTRIBUTE
t_max_err attr_set_resonance(t_fourpole *x, void *attr, long argc, t_atom *argv)
{
	x->attr_resonance = atom_getfloat(argv);
	x->myFilter->set_attr(tt_lowpass_fourpole::k_resonance, x->attr_resonance);
	
	return MAX_ERR_NONE;
	#pragma unused(attr)
}


// Perform (signal) Method
t_int *fourpole_perform(t_int *w)
{
   	t_fourpole *x = (t_fourpole *)(w[1]);				// Pointer
    x->signal_in[0]->set_vector((t_float *)(w[2])); 	// Input
    x->signal_out[0]->set_vector((t_float *)(w[3]));	// Output
	x->signal_in[0]->vectorsize = (int)(w[4]);			// Vector Size
			
	if (!(x->x_obj.z_disabled))
		x->myFilter->dsp_vector_calc(x->signal_in[0], x->signal_out[0]);	

    return (w + 5);		// Return a pointer to the NEXT object in the DSP call chain
}


// Perform (signal) Method
t_int *fourpole_perform2(t_int *w)
{
   	t_fourpole *x = (t_fourpole *)(w[1]);				// Pointer
    x->signal_in[0]->set_vector((t_float *)(w[2])); 	// Input
    t_float *freq = (t_float *)(w[3]);
    x->signal_out[0]->set_vector((t_float *)(w[4]));	// Output
	x->signal_in[0]->vectorsize = (int)(w[5]);			// Vector Size
			
	if(!(x->x_obj.z_disabled)){
		x->attr_frequency = *freq;
		x->myFilter->set_attr(tt_lowpass_fourpole::k_frequency, x->attr_frequency);
		x->myFilter->dsp_vector_calc(x->signal_in[0], x->signal_out[0]);	
	}
    return (w + 6);		// Return a pointer to the NEXT object in the DSP call chain
}


// DSP Method
void fourpole_dsp(t_fourpole *x, t_signal **sp, short *count)
{
	x->myFilter->set_sr(sp[0]->s_sr);
	x->myFilter->clear();
	if(count[1])
		dsp_add(fourpole_perform2, 5, x, sp[0]->s_vec, sp[1]->s_vec, sp[2]->s_vec, sp[0]->s_n);
	else
		dsp_add(fourpole_perform, 4, x, sp[0]->s_vec, sp[2]->s_vec, sp[0]->s_n); // Add Perform Method to the DSP Call Chain
}

