/* 
 *	External object for Max/MSP
 *	Copyright © 2003 by Timothy Place
 * 
 *	License: This code is licensed under the terms of the "New BSD License"
 *	http://creativecommons.org/licenses/BSD/
 */

#include "TTClassWrapperMax.h"

#define MAX_LIST_LENGTH 250

static t_class *pack_class;			// Required: Global pointer for our class

// Data Structure for this object
typedef struct _pack{
	t_object		ob;								// REQUIRED: Our object
	void			*myproxies[MAX_LIST_LENGTH-1];	// for proxy inlets
	long			inletnum;						// proxy input inlet number
	void 			*my_outlet;						// my outlet -- NOTE: the attribute dump outlet (2nd outlet) is handled automagically 
													//		... so we don't worry about it
	Atom			mylist[MAX_LIST_LENGTH];		// storage for output list
	short			mylistlen;						// how many elements in our list
	char			triggerlist[MAX_LIST_LENGTH];	// ATTRIBUTE: storage of which inlets tigger output from the object (essentially an array of toggles)
	long			triggerlistlen;
} t_pack;


// Prototypes for our methods:
void *pack_new(t_symbol *msg, short argc, t_atom *argv);
void pack_free(t_pack *x);
void pack_assist(t_pack *x, void *b, long msg, long arg, char *dst);
void pack_inletinfo(t_pack *x, void *b, long a, char *t);
void pack_symbol(t_pack *x, t_symbol *msg, short argc, t_atom *argv);
void pack_int(t_pack *x, long value);
void pack_float(t_pack *x, double value);
void pack_list(t_pack *x, t_symbol *msg, short argc, t_atom *argv);


/************************************************************************************/
// Main() Function

extern "C" int TTCLASSWRAPPERMAX_EXPORT main(void)
{
	t_class *c;

	c = class_new("tap.pack",(method)pack_new, (method)pack_free, sizeof(t_pack), 
		(method)0L, A_GIMME, 0);

		common_symbols_init();
	class_addmethod(c, (method)pack_int,				"int", 0L);
 	class_addmethod(c, (method)pack_float, 				"float", 0L);
  	class_addmethod(c, (method)pack_list, 				"list", A_GIMME, 0L);	
  	class_addmethod(c, (method)pack_symbol, 			"anything", A_GIMME, 0L);	
	class_addmethod(c, (method)pack_assist, 			"assist", A_CANT, 0L); 
	class_addmethod(c, (method)pack_inletinfo,			"inletinfo",	A_CANT, 0);

	CLASS_ATTR_CHAR_VARSIZE(c, "triggers", 0, t_pack, triggerlist, triggerlistlen, MAX_LIST_LENGTH);

class_register(_sym_box, c); 	pack_class = c;
}


/************************************************************************************/
// Object Creation Method

void *pack_new(t_symbol *msg, short argc, t_atom *argv)
{
	t_pack	*x;
	short	i;

	x = (t_pack *)object_alloc(pack_class);;
	if(x){
    	object_obex_store((void *)x, _sym_dumpout, (object *)outlet_new(x,NULL));	// dumpout	
	
		x->my_outlet = outlet_new(x,0L);					// create an outlet (this is my outlet) and store the pointer in the struct

		
		if(argc==0)									// If no arguments
			x->mylistlen = 2;						// default number of list elements
		else{
			if(argv[0].a_type == A_LONG)			// If first arg is a long
				x->mylistlen = argv[0].a_w.w_long;	// number of list elements is based on first argument
			else
				x->mylistlen = 2;					// else use the default
		}
		
		// Create Inlets (inlets beyond the default (num 0) inlet
		for (i = x->mylistlen-1; i >= 1; i--)
			x->myproxies[i] = proxy_new(x, i, &x->inletnum);

		// Set defaults before I go loading in values from the attributes
		for(i=0; i<MAX_LIST_LENGTH; i++)
			x->triggerlist[i] = 0;			// inlets do not trigger output, except...
		x->triggerlist[0] = 1;				// first inlet triggers output
		x->triggerlistlen = 1;
		
		// handle attribute args
		attr_args_process(x,argc,argv);

		// Initialize the internal list
		for (i=0; i< MAX_LIST_LENGTH; i++)
			SETLONG(x->mylist+i, 0);
	}
	return (x);											// return the pointer to our new instantiation
}


void pack_free(t_pack *x)
{
	short i;
	
	for(i = x->mylistlen-1; i >= 1; i--)
		freeobject((t_object *)x->myproxies[i]);
}


/************************************************************************************/
// Methods bound to input/inlets

// Method for Assistance Messages
void pack_assist(t_pack *x, void *b, long msg, long arg, char *dst)
{
	if(msg==1) 						// Inlet
		strcpy(dst, "Input");
	else if(msg==2){ 				// Outlets
		switch(arg){
			case 0: strcpy(dst, "Output"); break;
			case 1: strcpy(dst, "Attribute Stuff"); break;
 		}
 	}		
}

void pack_inletinfo(t_pack *x, void *b, long a, char *t)
{
	*t = 1;
	if(a < x->triggerlistlen){
		if(x->triggerlist[a])
			*t = 0;
	}
}


// INTEGER INPUT
void pack_int(t_pack *x, long value)
{
	SETLONG(x->mylist+(x->inletnum), value);
		
	if (x->triggerlist[(x->inletnum)] != 0)
		outlet_list(x->my_outlet, 0L, x->mylistlen, x->mylist);	// output the result	
}

// FLOATING POINT INPUT
// on Windows this object only works if the argument is declared double?
void pack_float(t_pack *x, double value)
{
	long inletnum = proxy_getinlet((object *)x);
	atom_setfloat(&(x->mylist[inletnum]), value);
		
	if (x->triggerlist[(x->inletnum)] != 0)
		outlet_list(x->my_outlet, 0L, x->mylistlen, x->mylist);	// output the result	
}

// SYMBOL INPUT
void pack_symbol(t_pack *x, t_symbol *msg, short argc, t_atom *argv)
{
	long inletnum = proxy_getinlet((object *)x);
	SETSYM(x->mylist+(inletnum), msg);

	if (x->triggerlist[(x->inletnum)] != 0)
		outlet_list(x->my_outlet, 0L, x->mylistlen, x->mylist);	// output the result	
}

// LIST INPUT
void pack_list(t_pack *x, t_symbol *msg, short argc, t_atom *argv)
{
	long inletnum = proxy_getinlet((object *)x);

	switch(argv[0].a_type){
		case A_LONG:
			SETLONG(x->mylist+(inletnum), argv[0].a_w.w_long);
			break;
		case A_FLOAT:
			SETFLOAT(x->mylist+(inletnum), argv[0].a_w.w_float);
			break;
		default:
			object_obex_dumpout(x, gensym("error - unrecognised 1st element for list"), 0, 0L);		
			break;
	}

	if (x->triggerlist[(inletnum)] != 0)
		outlet_list(x->my_outlet, 0L, x->mylistlen, x->mylist);	// output the result	
}
