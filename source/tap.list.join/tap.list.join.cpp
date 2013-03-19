/* join.c -- output the join of a group of numbers ------- */
/*			 updated for CodeWarrior	68k / PPC version Summer 96 -RD		*/
/*           updated for OSX December 2002  -Rd     */
// updated to max list size of 512 for use in Hipno, 2004, by Timothy Place

#include "TTClassWrapperMax.h"

#define MAXSIZE 512

#define JOIN_PROXY_GETINLET(x) (proxy_getinlet? proxy_getinlet((t_object *)x) : x->j_inletnum)

/* join object data structure */
typedef struct _join
{
	t_object j_ob;			/* must begin every object */
	int j_len[2];
	t_atom j_at[2][MAXSIZE];
	long j_inletnum;
	void *j_prox1;			/* proxy */
	void *j_out1;			/* outlet */
} t_join;

static t_class *s_join_class;			/* global variable that contains the t_join class */

void join_bang(t_join *x);
void join_list(t_join *x, t_symbol *s, short ac, t_atom *av);
void join_anything(t_join *x, t_symbol *s, short ac, t_atom *av);
void join_output(t_join *x);
void join_int(t_join *x, long i);
void join_float(t_join *x, double f);
void *join_new(t_symbol *s, short ac, t_atom *av);

static t_symbol *ps_list;

extern "C" int TTCLASSWRAPPERMAX_EXPORT main(void)
{
	s_join_class = class_new("tap.list.join", (method)join_new, NULL, sizeof(t_join), NULL, A_GIMME, 0);
	
	class_addmethod(s_join_class, (method)join_bang,	"bang",		0);
	class_addmethod(s_join_class, (method)join_int,		"int",		A_LONG, 0);		/* bind this method to the bang symbol */
	class_addmethod(s_join_class, (method)join_float,	"float",	A_FLOAT, 0);
	class_addmethod(s_join_class, (method)join_anything,"anything",	A_GIMME, 0);	/* bind method for list */
	class_addmethod(s_join_class, (method)join_list,	"list",		A_GIMME, 0);	/* bind method for list */

	ps_list = gensym("list");
}

void join_bang(t_join *x)	/* argument is a pointer to an instance */
{
	long in = proxy_getinlet((t_object*)x);
	if (in == 0) join_output(x) ;
}

void join_list(t_join *x, Symbol *s, short ac, Atom *av)		
{
	int i;
	t_atom *at;	
	long in = proxy_getinlet((t_object*)x);
	
	//TTLimitMax<int>(ac, MAXSIZE);
	at = x->j_at[in];
	x->j_len[in] = ac;		
	for (i=0; i<ac; ++i) *at++ = *av++;
	if (in == 0) join_output(x) ;
}

void join_anything(t_join *x, Symbol *s, short ac, Atom *av)
{
	int i;
	t_atom *at;	
	long in = proxy_getinlet((t_object*)x);
		
	//TTLimitMax<int>(ac, MAXSIZE-1);
	at = x->j_at[in];
	atom_setsym(at, s);
	at ++;
	x->j_len[in] = ac+1;		
	for (i=0; i<ac; ++i) *at++ = *av++;
	if (in == 0) join_output(x) ;
}

void join_output(t_join *x)
{
	t_atom *at1, *at2;
	t_atom atoms[MAXSIZE*2];
	int i;
	t_symbol *s;
	
	if (x->j_len[0] && x->j_at[0][0].a_type != A_SYM 
		|| x->j_len[0] == 0 && x->j_len[1] && x->j_at[1][0].a_type != A_SYM) {
		at1 = atoms;
		at2 = x->j_at[0];
		for (i=0; i<x->j_len[0]; ++i) *at1++ = *at2++;	
		at2 = x->j_at[1];
		for (i=0; i<x->j_len[1]; ++i) *at1++ = *at2++;	
		outlet_list(x->j_out1, ps_list, x->j_len[0]+x->j_len[1], atoms);
	} else if (x->j_len[0]) {
		s = x->j_at[0][0].a_w.w_sym;
		at1 = atoms;
		at2 = x->j_at[0]+1;
		for (i=0; i<x->j_len[0]-1; ++i) *at1++ = *at2++;	
		at2 = x->j_at[1];
		for (i=0; i<x->j_len[1]; ++i) *at1++ = *at2++;	
		outlet_anything(x->j_out1, s, x->j_len[0]+x->j_len[1]-1, atoms);
	} else if (x->j_len[1]) {
		s = x->j_at[1][0].a_w.w_sym;
		at1 = atoms;
		at2 = x->j_at[1];
		for (i=0; i<x->j_len[1]; ++i) *at1++ = *at2++;	
		outlet_anything(x->j_out1, s, x->j_len[0]+x->j_len[1]-1, atoms);
	}
}

void join_int(t_join *x, long i)
{
	long in = proxy_getinlet((t_object*)x);
	
	x->j_len[in] = 1;
	atom_setlong(x->j_at[in], i);
	if (in == 0) join_output(x) ;
}

void join_float(t_join *x, double f)        /* float method added -RDD june 96 */
{
	long in = proxy_getinlet((t_object*)x);
	
	x->j_len[in] = 1;
	atom_setfloat(x->j_at[in], f);
	if (in == 0) join_output(x) ;
}


/* function run to create a new instance of the t_join class */
void *join_new(t_symbol *s, short ac, t_atom *av)
{
	t_join *x;							/* object we'll be creating */
	int i;
	t_atom *at;
	
	x = (t_join *)object_alloc(s_join_class);	/* allocates memory and sticks in an inlet */
	x->j_prox1 = proxy_new(x, 1L, &x->j_inletnum);
	x->j_out1 = intout(x);
	x->j_len[0] = 0;
	x->j_len[1] = 0;
	x->j_inletnum = 0;
	
	//TTLimitMax<int>(ac, MAXSIZE);		/* arguements to join mow accepted -RD */ 
	at = x->j_at[1];
	x->j_len[1] = ac;		
	for (i=0; i<ac; ++i) *at++ = *av++;
	
	return (x);					/* always return a copy of the created object */
}

