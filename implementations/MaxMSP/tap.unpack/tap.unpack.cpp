/* 
	tap.unpack
 		an unpack object that is specified with a single argument for how many outlets (any type)
	Copyright © 2003 by Timothy Place
*/

#include "TTClassWrapperMax.h"

#define MAX_LIST_LENGTH 250

static t_class *unpack_class;			// Required: Global pointer for our class

typedef struct _unpack 		// Data Structure for this object
{
	t_object		ob;								// REQUIRED: Our object
	void 			*my_outlet[MAX_LIST_LENGTH];	// my outlet array -- NOTE: the attribute dump outlet is handled automagically 
	short			numoutlets;						// how many outlets in the object
	char			triggerlist[MAX_LIST_LENGTH];	// ATTRIBUTE: storage of which inlets tigger output from the object (essentially an array of toggles)
	long			triggerlistlen;
} t_unpack;


// Prototypes for our methods:
void *unpack_new(t_symbol *s, long argc, t_atom *argv);
void unpack_free(t_unpack *x);
void unpack_assist(t_unpack *x, void *b, long msg, long arg, char *dst);
void unpack_symbol(t_unpack *x, t_symbol *msg, short argc, t_atom *argv);
void unpack_list(t_unpack *x, t_symbol *msg, short argc, t_atom *argv);


/************************************************************************************/
// Main() Function

extern "C" int TTCLASSWRAPPERMAX_EXPORT main(void)
{
	t_class *c;

	c = class_new("tap.unpack",(method)unpack_new, (method)unpack_free, sizeof(t_unpack), (method)0L, A_GIMME, 0);

		common_symbols_init();
    class_addmethod(c, (method)unpack_list,				"list", A_GIMME, 0L);
    class_addmethod(c, (method)unpack_symbol,			"anything", A_GIMME, 0L);
    class_addmethod(c, (method)unpack_assist, 			"assist", A_CANT, 0L); 
	class_addmethod(c, (method)stdinletinfo,			"inletinfo",	A_CANT, 0);
	
	CLASS_ATTR_CHAR_VARSIZE(c, "triggers", 0, t_unpack, triggerlist, triggerlistlen, MAX_LIST_LENGTH);

class_register(_sym_box, c); 	unpack_class = c;
}


/************************************************************************************/
// Object Creation Method

void *unpack_new(t_symbol *s, long argc, t_atom *argv)
{
	t_unpack	*x;
	short	i;

	x = (t_unpack *)object_alloc(unpack_class);; 	// only max object, no jit object - use this function for jitter objects...
	if(x){
		object_obex_store((void *)x, _sym_dumpout, (object *)(outlet_new(x,NULL)));	// dumpout
			
		if(argc==0)									// If no arguments
			x->numoutlets = 2;						// default number of list elements
		else{
			if(argv[0].a_type == A_LONG)			// If first arg is a long
				x->numoutlets = argv[0].a_w.w_long;	// number of list elements is based on first argument
			else
				x->numoutlets = 2;					// else use the default
		}
		
		// Create Outlets
		for (i = x->numoutlets-1; i >= 0; i--)
			x->my_outlet[i] = outlet_new(x, 0L);
			
		// Set defaults before I go loading in values from the attributes
		for(i=0; i<MAX_LIST_LENGTH; i++)
			x->triggerlist[i] = 0;			// inlets do not trigger output, except...
		x->triggerlist[0] = 1;				// first inlet triggers output
		
		attr_args_process(x,argc,argv);				// handle attribute args	
	}
	return (x);									// return the pointer to our new instantiation
}


void unpack_free(t_unpack *x)
{
	;
}


/************************************************************************************/
// Methods bound to input/inlets

// Method for Assistance Messages
void unpack_assist(t_unpack *x, void *b, long msg, long arg, char *dst)
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

// SYMBOL INPUT
void unpack_symbol(t_unpack *x, t_symbol *msg, short argc, t_atom *argv)
{
	int	i;
	
	for(i=argc-1; i>=0; i--){
		if(i > (x->numoutlets - 2)){
			object_obex_dumpout(x, gensym("error not enough outlets for list"), 0, 0L);
		}
		else{
			switch(argv[i].a_type){
				case A_LONG:
					outlet_int(x->my_outlet[i+1], argv[i].a_w.w_long);
					break;
				case A_FLOAT:
					outlet_float(x->my_outlet[i+1], argv[i].a_w.w_float);
					break;
				case A_SYM:
					outlet_anything(x->my_outlet[i+1], argv[i].a_w.w_sym, 0, (t_atom *)NIL);
					break;
				default:
					object_obex_dumpout(x, gensym("error unrecognised atom for list element"), 0, 0L);		
					break;
			}	
		}
	}
	outlet_anything(x->my_outlet[0], msg, 0, (t_atom *)NIL);
	object_obex_dumpout(x, gensym("done"), 0, 0L);	
}

// LIST INPUT
void unpack_list(t_unpack *x, t_symbol *msg, short argc, t_atom *argv)
{
	int	i;

	for(i=argc-1; i>=0; i--){
		if(i > (x->numoutlets - 1)){
			object_obex_dumpout(x, gensym("error not enough outlets for list"), 0, 0L);	
		}
		else{
			switch(argv[i].a_type){
				case A_LONG:
					outlet_int(x->my_outlet[i], argv[i].a_w.w_long);
					break;
				case A_FLOAT:
					outlet_float(x->my_outlet[i], argv[i].a_w.w_float);
					break;
				case A_SYM:
					outlet_anything(x->my_outlet[i], argv[i].a_w.w_sym, 0, (t_atom *)NIL);
					break;
				default:
					object_obex_dumpout(x, gensym("error unrecognised atom for list element"), 0, 0L);		
					break;
			}	
		}
	}
	object_obex_dumpout(x, gensym("done"), 0, 0L);	
}

