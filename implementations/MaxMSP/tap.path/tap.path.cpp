#include "TTClassWrapperMax.h"

// Data structure for this object 
typedef struct _tappath{
	Object		obj;			// Must always be the first field; used by Max
	void*		resOut;			// outlet for script result
} t_tappath;

// Prototypes for methods: need a method for each incoming message
void* tappath_new(void);
void  tappath_assist(t_tappath *x, void *b, long msg, long arg, char* str);
void  tappath_bang(t_tappath* x);
void  tappath_prefs(t_tappath* x);
void  tappath_home(t_tappath* x);
void  tappath_boot(t_tappath* x);
void  tappath_docs(t_tappath* x);
void  tappath_temp(t_tappath* x);

// Globals
static t_class*		tappath_class;		// Required. Global pointing to this class 
static t_symbol*	ps_path_temp;
static t_symbol*	ps_path_home;
static t_symbol*	ps_path_docs;
static t_symbol*	ps_path_prefs;
static t_symbol*	ps_path_boot;

/************************************************************************************/
// Main() Function

extern "C" int TTCLASSWRAPPERMAX_EXPORT main(void)
{
	t_class *c;

	c = class_new("tap.path",(method)tappath_new, (method)0L, sizeof(t_tappath), 
		(method)0L, 0);

		common_symbols_init();								// Initialize TapTools
	class_addmethod(c, (method)tappath_prefs,		"prefs", 0L);
	class_addmethod(c, (method)tappath_home,		"home", 0L);
	class_addmethod(c, (method)tappath_boot,		"startup", 0L);
	class_addmethod(c, (method)tappath_boot,		"boot", 0L);
	class_addmethod(c, (method)tappath_docs,		"docs", 0L);
	class_addmethod(c, (method)tappath_temp,		"temp", 0L);
 	class_addmethod(c, (method)tappath_bang,		"bang", 0L);		
	class_addmethod(c, (method)tappath_assist,		"assist", A_CANT, 0L); 
	class_addmethod(c, (method)stdinletinfo,		"inletinfo",	A_CANT, 0);

	class_register(_sym_box, c);
	tappath_class = c;
	
	{
		char	temppath[MAX_PATH_CHARS];
		char	outpath[MAX_PATH_CHARS];
		OSErr	err = 0;
		FSRef	folderref;
		char	authpath[MAX_PATH_CHARS];

		// 1. Temp Folder
		err = FSFindFolder(kUserDomain, kTemporaryFolderType, kCreateFolder, &folderref);
		err = FSRefMakePath(&folderref, (UInt8 *)temppath, MAX_PATH_CHARS-1);
		path_nameconform(temppath, outpath, PATH_STYLE_MAX, PATH_TYPE_ABSOLUTE);
		ps_path_temp = gensym(outpath);

		// 2. Home Folder
		err = FSFindFolder(kUserDomain, kCurrentUserFolderType, kCreateFolder, &folderref);
		err = FSRefMakePath(&folderref, (UInt8 *)temppath, MAX_PATH_CHARS-1);
		path_nameconform(temppath, outpath, PATH_STYLE_MAX, PATH_TYPE_ABSOLUTE);
		ps_path_home = gensym(outpath);

		// 3. Documents Folder
		err = FSFindFolder(kUserDomain, kCurrentUserFolderType, kCreateFolder, &folderref);
		err = FSRefMakePath(&folderref, (UInt8 *)temppath, MAX_PATH_CHARS-1);
		strcat(temppath, "/Documents");
		path_nameconform(temppath, outpath, PATH_STYLE_MAX, PATH_TYPE_ABSOLUTE);
		ps_path_docs = gensym(outpath);
	
		// 4. Preferences Folder
		err = FSFindFolder(kUserDomain, kPreferencesFolderType, kCreateFolder, &folderref);
		err = FSRefMakePath(&folderref, (UInt8 *)temppath, MAX_PATH_CHARS-1);
		path_nameconform(temppath, outpath, PATH_STYLE_MAX, PATH_TYPE_ABSOLUTE);
		ps_path_prefs = gensym(outpath);

		// 5. Boot path
		path_nameconform((char*)"/", outpath, PATH_STYLE_MAX, PATH_TYPE_ABSOLUTE);
		ps_path_boot = gensym(outpath);
	}
}




/************************************************************************************/
// Object Creation Method

void* tappath_new(void)
{
	t_tappath* x = (t_tappath*)object_alloc(tappath_class);
	
	if (x) {	
    	object_obex_store((void *)x, _sym_dumpout, (object *)outlet_new(x,NULL));	// dumpout	
		x->resOut = outlet_new(x, 0L);		// define an outlet
	}
	return x;								// Give this object's pointer to Max
} 


/************************************************************************************/
// Methods bound to input/inlets

// Method for Assistance Messages
void tappath_assist(t_tappath *x, void *b, long msg, long arg, char *dst)
{
	if(msg==1)						// Inlets
		strcpy(dst, "bang to return home tappath");
	else if(msg==2)					// Outlets
		strcpy(dst, "tappath of the current home directory");				
}

void tappath_bang(t_tappath* x)
{
	outlet_anything(x->resOut, ps_path_home, 0, (t_atom *)NIL);
}

void  tappath_prefs(t_tappath* x)
{
	outlet_anything(x->resOut, ps_path_prefs, 0, (t_atom *)NIL);
}

void  tappath_home(t_tappath* x)
{
	outlet_anything(x->resOut, ps_path_home, 0, (t_atom *)NIL);
}

void  tappath_boot(t_tappath* x)
{
	outlet_anything(x->resOut, ps_path_boot, 0, (t_atom *)NIL);
}

void  tappath_docs(t_tappath* x)
{
	outlet_anything(x->resOut, ps_path_docs, 0, (t_atom *)NIL);
}

void  tappath_temp(t_tappath* x)
{
	outlet_anything(x->resOut, ps_path_temp, 0, (t_atom *)NIL);
}

