#include "TTClassWrapperMax.h"
#include "jpatcher_api.h"
#include "jgraphics.h"


typedef struct windowdrag{
	t_jbox		box;
	t_rect 		rect;			// the window rect when a click is initiated
	t_pt		pt;				// the mouse location when a click is initiated
	char		attr_rounded;
} t_windowdrag;


void *windowdrag_new(t_symbol *s, short argc, t_atom *argv);
void windowdrag_free(t_windowdrag *x);
void windowdrag_notify(t_windowdrag *x, t_symbol *s, t_symbol *msg, void *sender, void *data);
void windowdrag_paint(t_windowdrag *x, t_object *view);
void windowdrag_mousedown(t_windowdrag *x, t_object *patcherview, t_pt px, long modifiers);
void windowdrag_mousedragdelta(t_windowdrag *x, t_object *patcherview, t_pt pt, long modifiers);


static t_class*	s_windowdrag_class = NULL;


extern "C" int TTCLASSWRAPPERMAX_EXPORT main(void)
{	
	t_class *c;
	long	flags;

	c = class_new("tap.windowdrag",(method)windowdrag_new, (method)windowdrag_free, sizeof(t_windowdrag), (method)NULL, A_GIMME, 0);

	flags = JBOX_COLOR;
	jbox_initclass(c, flags);
	c->c_flags |= CLASS_FLAG_NEWDICTIONARY;

		common_symbols_init();
	class_addmethod(c, (method)windowdrag_notify,			"notify",			A_CANT, 0);
	class_addmethod(c, (method)windowdrag_paint,			"paint",			A_CANT, 0);
	class_addmethod(c, (method)windowdrag_mousedown,		"mousedown",		A_CANT, 0);
	class_addmethod(c, (method)windowdrag_mousedragdelta,	"mousedrag",		A_CANT, 0);
	//class_addmethod(c, (method)stdinletinfo,				"inletinfo",	A_CANT, 0);

	CLASS_ATTR_DEFAULT(c, 	"patching_rect",	0, "0. 0. 200. 20.");
	CLASS_ATTR_DEFAULT(c,	"color",			0, "0.1 0.1 0.1 0.1");

	CLASS_ATTR_CHAR(c,					"rounded", 0, t_windowdrag, attr_rounded);
	CLASS_ATTR_FILTER_CLIP(c,			"rounded", 0, 100);
	CLASS_ATTR_LABEL(c,					"rounded", 0, "Round-ness of Box Corners");
	CLASS_ATTR_DEFAULT_SAVE_PAINT(c,	"rounded", 0, "7");
	CLASS_ATTR_CATEGORY(c,				"rounded", 0, "Appearance");


class_register(_sym_box, c); 	s_windowdrag_class = c;
}



void *windowdrag_new(t_symbol *s, short argc, t_atom *argv)
{
	t_windowdrag	*x = NULL;
	t_dictionary 	*d = NULL;
	long 			flags;
	
	if(!(d=object_dictionaryarg(argc, argv)))
		return NULL;	

	x = (t_windowdrag*)object_alloc(s_windowdrag_class);;
	if(x){
		flags = 0 
				| JBOX_DRAWFIRSTIN
				| JBOX_NODRAWBOX
		//		| JBOX_DRAWINLAST
				| JBOX_TRANSPARENT	
		//		| JBOX_NOGROW
		//		| JBOX_GROWY
				| JBOX_GROWBOTH
		//		| JBOX_IGNORELOCKCLICK
				| JBOX_HILITE
				| JBOX_BACKGROUND
		//		| JBOX_NOFLOATINSPECTOR
		//		| JBOX_TEXTFIELD
				;

		jbox_new(&x->box, flags, argc, argv);
		x->box.b_firstin = (t_object *)x;
				
		attr_dictionary_process(x, d);
		jbox_ready(&x->box);

		x = (t_windowdrag *)object_register(CLASS_BOX, symbol_unique(), x);
		object_attach_byptr(x, x);
	}
	return(x);
}


void windowdrag_free(t_windowdrag *x)
{
	object_detach_byptr(x, x); 
	object_unregister(x); 
	jbox_free(&x->box);
}


#pragma mark -
#pragma mark methods

void windowdrag_notify(t_windowdrag *x, t_symbol *s, t_symbol *msg, void *sender, void *data)
{
	if((msg == _sym_attr_modified) && (sender == x)){
		jbox_redraw(&x->box);
	}
}


#pragma mark -
#pragma mark drawing and appearance

void windowdrag_paint(t_windowdrag *x, t_object *view)
{
	t_rect 		rect;
	t_jgraphics *g;
	double 		border_thickness = 0.5;

	g = (t_jgraphics*) patcherview_get_jgraphics(view);
	jbox_get_rect_for_view((t_object*) &x->box, view, &rect);

	// clear the background
	jgraphics_rectangle_rounded(g,  border_thickness, 
									border_thickness, 
									rect.width - ((border_thickness) * 2.0), 
									rect.height - ((border_thickness) * 2.0), 
									x->attr_rounded, x->attr_rounded); 
	jgraphics_set_source_jrgba(g,	&x->box.b_color);
	jgraphics_fill(g);
}


void windowdrag_mousedown(t_windowdrag *x, t_object *patcherview, t_pt px, long modifiers)
{	
	object_attr_get_rect(patcherview, _sym_windowrect, &x->rect);
	x->pt.x = px.x;// - r.x;
	x->pt.y = px.y;// - r.y;
}


// mousedragdelta sends the amount the mouse moved in t_pt
void windowdrag_mousedragdelta(t_windowdrag *x, t_object *patcherview, t_pt pt, long modifiers)
{
	t_pt	delta;

	delta.x = pt.x - x->pt.x;
	delta.y = pt.y - x->pt.y;
	x->rect.x += delta.x;
	x->rect.y += delta.y;
	object_attr_set_rect(patcherview, _sym_windowrect, &x->rect);
}
