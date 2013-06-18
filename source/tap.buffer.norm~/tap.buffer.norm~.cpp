/* 
 *	External object for Max/MSP
 *	Copyright © 2004 by Timothy Place
 * 
 *	License: This code is licensed under the terms of the "New BSD License"
 *	http://creativecommons.org/licenses/BSD/
 */

#include "ext.h"
#include "z_dsp.h"
#include "ext_strings.h"
#include "buffer.h"


typedef struct _index{
    t_pxobject		l_obj;
	t_buffer_ref*	l_buffer;
} t_norm;


// Function & Method Prototypes
t_max_err norm_notify(t_norm *x, t_symbol *s, t_symbol *msg, void *sender, void *data);
void norm_dblclick(t_norm *x);
void norm_set(t_norm *x, t_symbol *s);
void* norm_new(t_symbol *s);
void norm_free(t_norm *x);
void norm_assist(t_norm *x, void *b, long m, long a, char *s);
void norm_calc(t_norm *x);


static t_class *norm_class;


/*************************************************************************************/
// Main() Function

extern "C" int C74_EXPORT main(void)
{
	t_class* c = class_new("tap.buffer.norm~",(method)norm_new, (method)norm_free, sizeof(t_norm), (method)0L, A_SYM, 0);

	class_addmethod(c, (method)norm_calc,		"bang", A_LONG, 0L);
	class_addmethod(c, (method)norm_set,		"set", A_SYM, 0L);
	class_addmethod(c, (method)norm_assist, 	"assist", A_CANT, 0L); 
	class_addmethod(c, (method)norm_dblclick,	"dblclick",	A_CANT, 0);
	class_addmethod(c, (method)norm_notify,		"notify",	A_CANT, 0);

	class_register(CLASS_BOX, c);
	norm_class = c;
	return 0;
}


/*************************************************************************************/
// Object Creation Routine

void* norm_new(t_symbol* s)
{
    t_norm* x = (t_norm*)object_alloc(norm_class);	

	norm_set(x, s);									// Buffer name argument	
    return x;
}


void norm_free(t_norm* x)
{
	object_free(x->l_buffer);
}


/*************************************************************************************/
// Bound to input/inlet methods

// Set Buffer Method
void norm_set(t_norm *x, t_symbol *name)
{
	if (!x->l_buffer)
		x->l_buffer = buffer_ref_new((t_object*)x, name);
	else
		buffer_ref_set(x->l_buffer, name);
}


// Method for Assistance Messages
void norm_assist(t_norm *x, void *b, long msg, long arg, char *dst)
{
	strcpy(dst, "bang to normalize the buffer");
}


// Handle notifications from the buffer~
t_max_err norm_notify(t_norm *x, t_symbol *s, t_symbol *msg, void *sender, void *data)
{
	return buffer_ref_notify(x->l_buffer, s, msg, sender, data);
}


// Open the buffer~ view window
void norm_dblclick(t_norm *x)
{
	buffer_view(buffer_ref_getobject(x->l_buffer));
}


// Normalize the buffer~ content
void norm_calc(t_norm *x)
{
	t_buffer_obj	*b = buffer_ref_getobject(x->l_buffer);
	t_float			*tab = buffer_locksamples(b);
	
	if (tab) {
		long 	channelcount = buffer_getchannelcount(b);
		long 	framecount = buffer_getframecount(b);
		long	samplecount = channelcount * framecount;	
		long 	i;
		double	current_samp = 0.0, mult_factor = 0.0;	// current sample value
	
		for (i=0; i < samplecount; i++) {				// FIND PEAK VALUE
			if (fabs(tab[i]) > current_samp)
				current_samp = fabs(tab[i]);
		}

		if (current_samp > 0.0) {
			mult_factor = 0.9999 / current_samp; 		// FIND THE FACTOR
			for (i=0; i < samplecount; i++)				// SCALE THE VALUES
				tab[i] *= mult_factor;
			buffer_setdirty(b);							// mark the buffer so that other objects know that there are updates
		}
		buffer_unlocksamples(b);
	}	
}
