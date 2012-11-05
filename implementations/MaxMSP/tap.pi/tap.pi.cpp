#include "TTClassWrapperMax.h"

static t_class*   pi_class;		// Required. Global pointing to this class 

// Data structure for this object 
typedef struct _pi{
	Object		obj;		// Must always be the first field; used by Max
	void*		my_outlet;	// outlet for script result
} t_pi;

// Prototypes for methods: need a method for each incoming message
void* pi_new(void);
void  pi_assist(t_pi *x, void *b, long msg, long arg, char* str);
void  pi_bang(t_pi* x);


/************************************************************************************/
// Main() Function

extern "C" int TTCLASSWRAPPERMAX_EXPORT main(void)
{
	t_class *c;
	
	c = class_new("tap.pi",(method)pi_new, (method)0L, sizeof(t_pi), 
		(method)0L, A_GIMME, 0);

		common_symbols_init();								// Initialize TapTools
 	class_addmethod(c, (method)pi_bang, 	"bang", 0L);		
	class_addmethod(c, (method)pi_assist, 	"assist", A_CANT, 0L); 

class_register(_sym_box, c); 	pi_class = c;
}

/************************************************************************************/
// Object Creation Method

void* pi_new(void)
{
	t_pi* x = (t_pi*)object_alloc(pi_class);

	if (x) {
    	object_obex_store((void *)x, _sym_dumpout, (object *)outlet_new(x,NULL));	// dumpout	
		x->my_outlet = floatout(x);		// define an outlet
	}
	return x;						// Give this object's pointer to Max
}

/************************************************************************************/
// Methods bound to input/inlets

// Method for Assistance Messages
void pi_assist(t_pi *x, void *b, long msg, long arg, char *dst)
{
	if(msg==1)						// Inlets
		strcpy(dst, "bang to return pi");
	else if(msg==2)					// Outlets
		strcpy(dst, "pi");				
}

// Bang Method
void pi_bang(t_pi* x)
{
	outlet_float(x->my_outlet, kTTPi);
}