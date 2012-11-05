// MSP External: tap.typecheck~.c
// check datatype and send out the appropriate outlet

#include "TTClassWrapperMax.h"			

static t_class *tc_class;			// Required. Global pointing to this class

typedef struct _tc			// Data Structure for this object
{
	t_object 	x_obj;			// This object - must be first
	void 		*bangOut;		// bang outlet
	void 		*intOut;		// int outlet
	void 		*floatOut;		// Floating-point outlet
	void 		*symbolOut;		// symbol outlet
	void 		*listOut;		// list outlet
	void 		*legallistOut;	// legal-list outlet
} t_tc;

// Prototypes for methods: need a method for each incoming message type:
void tc_assist(t_tc *x, void *b, long m, long a, char *s);		// Assistance Method
void *tc_new(void);												// New Object Creation Method
void tc_bang(t_tc *x);											// Bang method
void tc_int(t_tc *x, long value);								// Int method
void tc_float(t_tc *x, double value);							// Float method
void tc_symbol(t_tc *x, Symbol *msg, short argc, Atom *argv);	// Symbol method
void tc_list(t_tc *x, Symbol *msg, short argc, Atom *argv);		// List method

/************************************************************************************/
// Main() Function

extern "C" int TTCLASSWRAPPERMAX_EXPORT main(void)
{
	t_class *c;

	c = class_new("tap.typecheck",(method)tc_new, 0L, sizeof(t_tc), 
		(method)0L, 0L, 0);	// No args for this object

		common_symbols_init();									// Initialize TapTools
	class_addmethod(c, (method)tc_bang,		"bang", 0L);			// bang input
    class_addmethod(c, (method)tc_int, 		"int", A_LONG, 0L);		// Input as int
    class_addmethod(c, (method)tc_float, 	"float", A_FLOAT, 0L);	// Input as float
    class_addmethod(c, (method)tc_list,		"list", A_GIMME, 0L);
    class_addmethod(c, (method)tc_symbol,	"anything", A_GIMME, 0L);
    class_addmethod(c, (method)tc_assist, 	"assist", A_CANT, 0L); 

class_register(_sym_box, c); 	tc_class = c;
}

/************************************************************************************/
// Object Creation Method

void *tc_new(void)
{
	t_tc *x = (t_tc *)object_alloc(tc_class);;
	
	if(x){
		object_obex_store((void *)x, _sym_dumpout, (object *)outlet_new(x,NULL));	// dumpout
		x->legallistOut = listout(x);		// legal list outlet (last outlet)
	    x->symbolOut = outlet_new(x, 0);	
	    x->listOut = listout(x);			// original list outlet (used to output both legal and illegal lists)
	    x->floatOut = floatout(x);
		x->intOut = intout(x);
		x->bangOut = bangout(x);
	}
	return (x);
}

/************************************************************************************/
// Methods bound to input/inlets

// Method for Assistance Messages
void tc_assist(t_tc *x, void *b, long msg, long arg, char *dst)
{
	if(msg==1) 		// Inlets
		strcpy(dst, "Any Non-Signal Input");
	else if(msg==2){ 	// Outlets
		switch(arg){
			case 0: strcpy(dst, "(bang) Output"); break;
			case 1: strcpy(dst, "(int) Output"); break;
			case 2: strcpy(dst, "(float) Output"); break;
			case 3: strcpy(dst, "(list) Output"); break;
			case 4: strcpy(dst, "(symbol) Output"); break;
			case 5: strcpy(dst, "(list) legal-list Output"); break;
		}
	}
}


void tc_bang(t_tc *x)
{
	outlet_bang(x->bangOut);			// output the result		
}


void tc_int(t_tc *x, long value)
{
	outlet_int(x->intOut, value);		// output the result		
}


void tc_float(t_tc *x, double value)
{
	outlet_float(x->floatOut, value);		// output the result		
}


void tc_list(t_tc *x, Symbol *msg, short argc, Atom *argv)
{
	outlet_list(x->legallistOut, 0L, argc, argv);		// output the result
}


void tc_symbol(t_tc *x, Symbol *msg, short argc, Atom *argv)
{
	if(argc == 0)
		outlet_anything(x->symbolOut, msg, argc, argv);	// output the result
	else
		outlet_anything(x->listOut, msg, argc, argv);		// output the result	
}