/* slice.c -- (formerly listsplit.c) slice off parts of a list ------- */
/*				updated for CodeWarrior	68k / PPC version Summer 96 -RD		*/
/*           updated for OSX December 2002  -Rd     */
// updated to max list size of 512 for use in Hipno, 2004, by Timothy Place

#include "TTClassWrapperMax.h"

#define MAXSIZE 512

/* t_slice object data structure */
typedef struct _slice
{
	t_object s_ob;		/* must begin every object */
	int s_split;
	int s_len;
	t_atom s_at[MAXSIZE];
	void *s_out1, *s_out2;	/* outlet */
} t_slice;

static t_class *s_slice_class;			/* global variable that contains the ListSplit class */

void slice_bang(t_slice *x);
void slice_list(t_slice *x, t_symbol *s, int ac, t_atom *av);
void slice_anything(t_slice *x, t_symbol *s, int ac, t_atom *av);
void slice_output(t_slice *x);
void slice_int(t_slice *x, long i);
void slice_float(t_slice *x, double f);
void slice_in1(t_slice *x, long s);
void *slice_new(long s);

static t_symbol *ps_list;

extern "C" int TTCLASSWRAPPERMAX_EXPORT main(void)
{
	s_slice_class = class_new("tap.list.slice", (method)slice_new, NULL, sizeof(t_slice), NULL, A_GIMME, 0);
	
	class_addmethod(s_slice_class, (method)slice_bang,		"bang",		0);
	class_addmethod(s_slice_class, (method)slice_int,		"int",		A_LONG, 0);		/* bind this method to the bang symbol */
	class_addmethod(s_slice_class, (method)slice_in1,		"in1",		A_LONG, 0);		/* bind this method to the bang symbol */
	class_addmethod(s_slice_class, (method)slice_float,		"float",	A_FLOAT, 0);
	class_addmethod(s_slice_class, (method)slice_anything,	"anything",	A_GIMME, 0);	/* bind method for list */
	class_addmethod(s_slice_class, (method)slice_list,		"list",		A_GIMME, 0);	/* bind method for list */

	ps_list = gensym("list");
}

void slice_bang(t_slice *x)	/* argument is a pointer to an instance */
{
	slice_output(x);
}

void slice_list(t_slice *x, t_symbol *s, int ac, t_atom *av)			
{
	int i;
	t_atom *at;	
	
	at = x->s_at;
	TTLimitMax<int>(ac, MAXSIZE);
	x->s_len = ac;		
	for (i=0; i<ac; ++i) *at++ = *av++;
	slice_output(x);
}

void slice_anything(t_slice *x, t_symbol *s, int ac, t_atom *av)		
{
	int i;
	t_atom *at;	
		
	TTLimitMax<int>(ac, MAXSIZE-1);
	at = x->s_at;
	atom_setsym(at, s);
	at ++;
	x->s_len = ac+1;		
	for (i=0; i<ac; ++i) *at++ = *av++;
	slice_output(x) ;
}

void slice_output(t_slice *x)
{
	t_atom *at;	
	
	if (x->s_len > x->s_split) {
		at = x->s_at + x->s_split;

		if (at->a_type != A_SYM) {
			if (x->s_len - x->s_split == 1) {
				if (at->a_type == A_LONG) {
					outlet_int(x->s_out2, at->a_w.w_long);
				} else if (at->a_type == A_FLOAT) {
					outlet_float(x->s_out2, at->a_w.w_float);
				}
			} else {
				outlet_list(x->s_out2,ps_list,x->s_len - x->s_split,at);
			}
		} else {
			outlet_anything(x->s_out2,at->a_w.w_sym,x->s_len - x->s_split - 1,at+1);
		}
	}
	if (x->s_len >= x->s_split) {	
		at = x->s_at;
		if (x->s_split == 1) {
			if (at->a_type == A_LONG) {
				outlet_int(x->s_out1, at->a_w.w_long);
			} else if (at->a_type == A_FLOAT) {
				outlet_float(x->s_out1, at->a_w.w_float);
			} else {
				outlet_anything(x->s_out1, at->a_w.w_sym, 0, (t_atom *)NIL);
			}
		} else if (at->a_type != A_SYM) {
			outlet_list(x->s_out1,ps_list,x->s_split,at);
		} else {
			outlet_anything(x->s_out1, at->a_w.w_sym, x->s_split-1, at+1);
		}
	} else {
		at = x->s_at;
		if (x->s_split == 1) {
			if (at->a_type == A_LONG) {
				outlet_int(x->s_out1, at->a_w.w_long);
			} else if (at->a_type == A_FLOAT) {
				outlet_float(x->s_out1, at->a_w.w_float);
			} else {
				outlet_anything(x->s_out1, at->a_w.w_sym, 0, (t_atom *)NIL);
			}
		} else if (at->a_type != A_SYM) {
			outlet_list(x->s_out1,ps_list,x->s_len,at);
		} else {
			outlet_anything(x->s_out1,at->a_w.w_sym,x->s_len-1,at+1);
		}
	}
}

void slice_int(t_slice *x, long i)
{
	x->s_len = 1;
	atom_setlong(x->s_at, i);
	outlet_int(x->s_out1, i);
}

void slice_float(t_slice *x, double f)     		/* float method added jun 96 -RDD */
{
	x->s_len = 1;
	atom_setfloat(x->s_at, f);
	outlet_float(x->s_out1, f);
}

void slice_in1(t_slice *x, long s)
{
	x->s_split = s < 1 ? 1 : s;
}


/* function run to create a new instance of the t_slice class */
void *slice_new(long s)
{
	t_slice *x;							/* object we'll be creating */
	
	x = (t_slice *)object_alloc(s_slice_class);	/* allocates memory and sticks in an inlet */
	intin(x, 1);                        	/* added jun 96 rdd - was missing from code */
	x->s_out2 = intout(x);
	x->s_out1 = intout(x);
	x->s_len = 0;
	x->s_split = s < 1 ? 1 : s;
	return (x);						/* always return a copy of the created object */
}

