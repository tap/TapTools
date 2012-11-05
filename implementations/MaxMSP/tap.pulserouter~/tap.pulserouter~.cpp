/*
 *  jta.pr.c
 *  jta.pulserouter~
 *
 *  Created by Jesse Allison on Mon May 31 2004.
 *  Copyright (c) 2004 __MyCompanyName__. All rights reserved.
 *
 */

#include "TTClassWrapperMax.h"
#include "buffer.h"

#define OBJECT_NAME "tap.pulserouter~"  // name of object

#define OUTLET_MAX  64					// Max # of outlets
#define OUTLET_MIN  2					// min # of outlets
#define VEC_SIZE	OUTLET_MAX + 5		// size of vector array why +5?

static void *pulserouter_class;   // required global pointer to this class.


/* structure definition for the object */

typedef struct _pulserouter {
	t_pxobject  pr_obj;
	long		pr_outletcount;
	double		pr_currIndex[OUTLET_MAX];
	double		pr_normalized_position;
	double		pr_last_sample;			// last sample
	double		pr_last_phasor;		// last phasor position
	t_symbol	*l_sym;			// buffer name
	t_buffer	*l_buf;			// Buffer reference
} t_pulserouter;


/* Method & Function Definitions */

void *pulserouter_new(t_symbol *s,long outlets);
void pulserouter_perform64(t_pulserouter *x, t_object *dsp64, double **ins, long numins, double **outs, long numouts, long sampleframes, long flags, void *userparam);
void pulserouter_dsp64(t_pulserouter *x, t_object *dsp64, short *count, double samplerate, long maxvectorsize, long flags);
void pulserouter_set(t_pulserouter *x, t_symbol *s);		// add for buffer setting
void pulserouter_assist(t_pulserouter *x, t_object *b, long msg, long arg, char *s);
void pulserouter_getinfo(t_pulserouter *x);


/************************************************************************************
void main(void)

inputs:			nothing
description:	called the first time the object is used in Max, defines inlets, outlets & accepted messages
returns:		nothing
************************************************************************************/

extern "C" int TTCLASSWRAPPERMAX_EXPORT main(void)
{
	//	substitute if unbuffered		setup((t_messlist **)&pulserouter_class, (method)pulserouter_new, (method)dsp_free, (short)sizeof(t_pulserouter), 0L, A_DEFLONG, 0);
	setup((t_messlist **)&pulserouter_class, (method)pulserouter_new, (method)dsp_free, sizeof(t_pulserouter), 0L, A_SYM, A_DEFLONG, 0);

	addmess((method)pulserouter_dsp64, "dsp64", A_CANT, 0);
	addmess((method)pulserouter_set, "set", A_SYM, 0);  // add for buffer acquisition
	addmess((method)pulserouter_assist, "assist", A_CANT, 0);
	addmess((method)pulserouter_getinfo, "getinfo", A_NOTHING, 0);
	//class_addmethod(c, (method)stdinletinfo,	"inletinfo",	A_CANT, 0);
	
	dsp_initclass();
}


/************************************************************************************
Object Creation Routine		void *pulserouter_new(long outlets)
input:			number of outlets
desctription:   defines inlets & # of outlets based on argument
output:			nothing
************************************************************************************/

void *pulserouter_new(t_symbol *s, long outlets)
{
//  Unbuffered void *pulserouter_new(long outlets)
	long i;
	
	t_pulserouter *x = (t_pulserouter *)newobject(pulserouter_class);
	
	// add to add variable outlets
	// x->pr_outletcount = outlets>OUTLET_MAX?OUTLET_MAX:outlets<OUTLET_MIN?OUTLET_MIN:outlets;
	x->pr_outletcount = 5;
	x->pr_normalized_position = 0.0;
	x->pr_last_sample = 0.0;
	x->pr_last_phasor = 0.0;
	
	dsp_setup((t_pxobject *)x, 1);		// 1 inlet
	
	for (i=0; i < x->pr_outletcount; i++) {
		outlet_new((t_pxobject *)x, "signal");   // creates x outlets
	}
	
	// Buffer Prep stuff
	
	x->l_sym = s;								// Buffer name argument
		
	// MOVING THIS TO THE DSP METHOD - DONT TRY IT HERE !  20041216/TAP
	//defer_low(x, (method)pulserouter_set, s,0,0L);		// guarantee buffer has loaded before binding to it

		
	return (x);
}


/************************************************************************************
Set Buffer Method   void pulserouter_set(t_pulserouter *x, t_symbol *s)
inputs:			x = pointer to this object
				s = name of the buffer
description:	binds buffer to object
outputs:		nothing
************************************************************************************/

void pulserouter_set(t_pulserouter *x, t_symbol *s)
{
	t_buffer *b;
	
	x->l_sym = s;
	if ((b = (t_buffer *)(s->s_thing)) && ob_sym(b) == gensym("buffer~"))		// && ob_sym(b) == taptools_glob_sym_buffer
	{
		x->l_buf = b;
	}
	else {
		object_error((t_object *)x, "no buffer %s", s->s_name);
		x->l_buf = 0;
	}
}


/************************************************************************************
DSP Signal Processing   void pulserouter_dsp(t_pulserouter *x, t_signal **sp, short *count)
inputs:			x = pointer to this object
				sp = array of pointers to input & output signals
				count = # of signals attached
description:	adds dsp to dsp chain
outputs:		nothing
************************************************************************************/

void pulserouter_dsp64(t_pulserouter *x, t_object *dsp64, short *count, double samplerate, long maxvectorsize, long flags)
{
	pulserouter_set(x, x->l_sym);	
	object_method(dsp64, gensym("dsp_add64"), (t_object*)x, pulserouter_perform64, 0, NULL);	
}


/************************************************************************************
DSP Perform Method   t_int *pulserouter_perform(t_int *w)
inputs:			w = array of signal vectors specified in pulserouter_dsp
description:	counts pulses & routes them to successive outlets
outputs:		pointer to next
************************************************************************************/

void pulserouter_perform64(t_pulserouter *x, t_object *dsp64, double **ins, long numins, double **outs, long numouts, long sampleframes, long flags, void *userparam)
{	
	double*	in1 = ins[0];	
	int 		n = sampleframes;
	t_buffer*	b = x->l_buf;						// our buffer
	float*		tab;								// will point to our buffer's values
	long 		index,phasor_sample,frames,nc;		// index counter, length of buffer, # of channels
	int 		i;
	double 		val, phasor;
	double 		outlet[5];
	
	// add double trigger protection
	// if((value != 0.0) && (x->last_sample == 0.0)){ double trig protection
	
	if (x->l_buf == 0) {
		//object_error((t_object *)x, "no buffer is bound to this object");
		goto byebye;
	}
	
	tab = b->b_samples;			// point tab to our sample values
	frames = b->b_frames;		// get the buffer's length  is this in samples?
	nc = b->b_nchans;			// get the number of channels
	
	if (nc > 1) {
		//object_error((t_object *)x, "does not support multiple channel buffers");
		goto byebye;
	}
	
	index = x->pr_last_phasor * (frames-1);			// sets index at last phasor position in buffer
	
	while (n--) {
	
		phasor = *in1++;
		val = 0.;
		phasor_sample = (phasor * (frames-1));

								// CHECK INDEX TO MAKE SURE THE PHASOR HASEN'T RESTARTED
		if (phasor > x->pr_last_phasor) {				// add up all values in the buffer
			for(; index < phasor_sample; index++) {
				val = val + tab[index];
			}
		} else {										// add up all values from last Phasor position to frames-1, and 0. to phasor_sample
			for (; index < (frames-1); index++){
				val = val + tab[index];
			}
			for (index = 0; index < phasor_sample; index++) {
				val = val + tab[index];
			}
		}
			
		if (val == 0.0) {								// set outs to 0 if val = 0
			for(i=0; i < x->pr_outletcount; i++) {
				*outs[i]++ = 0.0;
			}
		} else {							// check for each "bit" set from addition of buffer.  
			if (val - 15. > 0) {			// would be much quicker by bit checking real bits!
				val = val - 16.;
				outlet[4] = 1.0;
			} else {
				outlet[4] = 0.0;
			}
			if (val - 7. > 0) {
				val = val - 8.;
				outlet[3] = 1.0;
			} else {
				outlet[3] = 0.0;
			}
			if (val - 3. > 0) {
				val = val - 4.;
				outlet[2] = 1.0;
			} else {
				outlet[2] = 0.0;
			}
			if (val - 1. > 0) {
				val = val - 2.;
				outlet[1] = 1.0;
			} else {
				outlet[1] = 0.0;
			}
			if (val == 1) {
				outlet[0] = 1.0;
			} else {
				outlet[0] = 0.0;
			}
													// sets outs[i] to outlet[i] which was set 0 or 1 above
			for(i=0; i < x->pr_outletcount; i++) {
				*outs[i]++ = outlet[i];
			}
		}

		index = phasor_sample;						// index = last phasor position
		x->pr_last_phasor = phasor;					// last phasor 
	}
byebye:	
	return;

}


/************************************************************************************
Assist Method   void pulserouter_assist(t_pulserouter *x, t_object *b, long msg, long arg, char *s)
inputs:			x pointer to object, b, arg, msg,s point to our object
description:	Does nothing with the double
outputs:		nothing
************************************************************************************/

void pulserouter_assist(t_pulserouter *x, t_object *b, long msg, long arg, char *s)
{
	if (msg = ASSIST_OUTLET) {
		strcpy(s,"(signal) Routed Pulse Out");
	}
	else {
		strcpy(s,"(signal) Pulses to be Routed");
	}
}


/************************************************************************************
Get_info Method   void pulserouter_getinfo(t_pulserouter *x)
inputs:			x pointer to object, b, arg, msg,s point to our object
description:	Does nothing with the double
outputs:		nothing
************************************************************************************/

void pulserouter_getinfo(t_pulserouter *x)
{
	object_post((t_object *)x, "%s object by Jesse Allison", OBJECT_NAME);
	object_post((t_object *)x, "Updated on %s - www.electrotap.com", __DATE__);
}
