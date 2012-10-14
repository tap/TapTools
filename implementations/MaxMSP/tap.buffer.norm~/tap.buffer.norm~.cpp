// tap.norm~.c  
// normalize a buffer

#include "TTClassWrapperMax.h"
#include "buffer.h"

static t_class *norm_class;

typedef struct _index{
    t_pxobject 	l_obj;
    t_symbol 	*l_sym;		// Current Buffer Name
    t_buffer	*l_buf;		// Buffer Reference
} t_norm;


// Function & Method Prototypes
void norm_dsp(t_norm *x, t_signal **sp, short *count);
void norm_set(t_norm *x, t_symbol *s);
void *norm_new(t_symbol *s);
void norm_assist(t_norm *x, void *b, long m, long a, char *s);
void norm_calc(t_norm *x);


/*************************************************************************************/
// Main() Function

extern "C" int TTCLASSWRAPPERMAX_EXPORT main(void)
{
	t_class *c;

	c = class_new("tap.buffer.norm~",(method)norm_new, (method)dsp_free, sizeof(t_norm), 
		(method)0L, A_SYM, 0);

	common_symbols_init();										// Initialize TapTools
	class_addmethod(c, (method)norm_calc,		"bang", A_LONG, 0L);
	class_addmethod(c, (method)norm_set,		"set", A_SYM, 0L);
 	class_addmethod(c, (method)norm_dsp, 		"dsp", A_CANT, 0L);		
	class_addmethod(c, (method)norm_assist, 	"assist", A_CANT, 0L); 

	class_dspinit(c);									// Setup object's class to work with MSP
	class_register(_sym_box, c);
	norm_class = c;
	return 0;
}


/*************************************************************************************/
// Object Creation Routine

void *norm_new(t_symbol *s)
{
    t_norm *x = (t_norm *)object_alloc(norm_class);	
	if(x){
	   	object_obex_store((void *)x, _sym_dumpout, (object *)outlet_new(x,NULL));	// dumpout	
	    dsp_setup((t_pxobject *)x,1);					// Create Object and 1 Inlet (last argument)
	    outlet_new((t_pxobject *)x, "signal");			// Create a signal Outlet

		x->l_sym = s;									// Buffer name argument	
		defer_low(x, (method)norm_set,s,0,0L);			// By defering this method, we can guarantee
														//	that the buffer has loaded before we try binding to it
	}
    return (x);											// Return the pointer
}



/*************************************************************************************/
// Bound to input/inlet methods

// Set Buffer Method - (also used by DSP method)
void norm_set(t_norm *x, t_symbol *s)
{
	t_buffer *b;
	
	x->l_sym = s;
	if ((b = (t_buffer *)(s->s_thing)) && ob_sym(b) == gensym("buffer~")){
		x->l_buf = b;
	} 
	else 
	{
		object_error((t_object *)x, "no buffer~ %s", s->s_name);
		x->l_buf = 0;
	}
}


// Method for Assistance Messages
void norm_assist(t_norm *x, void *b, long msg, long arg, char *dst)
{
	strcpy(dst, "bang to normalize the buffer");
}


// norm point calculation
void norm_calc(t_norm *x)
{
	t_buffer *b = x->l_buf;							// Our Buffer
	float *tab;										// Will point to our buffer's values
	long i;
	double current_samp = 0.0, mult_factor = 0.0;	// current sample value
	long saveinuse;

	if(x->l_buf == 0){
		object_error((t_object *)x, "no buffer is bound to this object");
		return;
	}

	// LOCK DOWN THE BUFFER
	saveinuse = b->b_inuse;
	b->b_inuse = true;


	tab = b->b_samples;				// point tab to our sample values
	
	// FIND PEAK VALUE
	for(i=0; i < b->b_size; i++){
		if(fabs(tab[i]) > current_samp)
			current_samp = fabs(tab[i]);
	}

	// FIND THE FACTOR
	if(current_samp == 0){
		object_error((t_object *)x, "buffer is empty, cannot normalize");
		goto out;
	}
	mult_factor = 0.9999 / current_samp;

	// SCALE THE VALUES
	for(i=0; i < b->b_size; i++){
		tab[i] *= mult_factor;
	}
out:
	// UNLOCK THE BUFFER
	b->b_inuse = saveinuse;
		
	// UPDATE FLAG
	object_method((t_object*)b, gensym("dirty"));
}


// DSP METHOD - USED ONLY FOR BINDING TO THE BUFFER
void norm_dsp(t_norm *x, t_signal **sp, short *count)
{
	norm_set(x, x->l_sym);
}
