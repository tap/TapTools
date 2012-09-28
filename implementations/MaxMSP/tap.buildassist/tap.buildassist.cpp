// MSP External: tap.buildassist
// flag Max's collective builder to automatically include files

#include "TapToolsObject.h"


// Data Structure for this object
typedef struct _buildassist			
{
	t_object		ob;	
	t_symbol		*attr_type;
	t_symbol		*attr_path;
	t_symbol		*attr_extension;
} t_buildassist;


// Prototypes for methods: need a method for each incoming message type:
void buildassist_assist(t_buildassist *x, void *b, long m, long a, char *s);		// Assistance Method
void *buildassist_new(t_symbol *msg, short argc, t_atom *argv);		// New Object Creation Method
void buildassist_fileusage(t_buildassist *x, void *w);
void buildassist_free(t_buildassist *x);

// Globals
static t_class		*buildassist_class;		// Required. Global pointing to this class
static t_symbol	*ps_file;
static t_symbol	*ps_folder;
static t_symbol	*ps_external;
static t_symbol	*ps_win_external;
static short		item_count;				// These are used to prevent multiple including during a Max session
static t_symbol	*item[1024];


/************************************************************************************/
// Main() Function

extern "C" int TAP_EXPORT_MAXOBJ main(void)
{
	t_class *c;

	c = class_new("tap.buildassist",(method)buildassist_new, (method)buildassist_free, sizeof(t_buildassist), (method)0L, A_GIMME, 0);
	common_symbols_init();

	class_addmethod(c, (method)buildassist_fileusage,	"fileusage", A_CANT, 0L);
    class_addmethod(c, (method)buildassist_assist,		"assist", A_CANT, 0L); 
    class_addmethod(c, (method)object_obex_dumpout, 	"dumpout", A_CANT,0);
	class_addmethod(c, (method)stdinletinfo,			"inletinfo",	A_CANT, 0);

	CLASS_ATTR_SYM(c,		"type",			0,	t_buildassist, attr_type);
	CLASS_ATTR_SYM(c,		"path",			0,	t_buildassist, attr_path);
	CLASS_ATTR_SYM(c,		"extension",	0,	t_buildassist, attr_extension);

    class_register(CLASS_BOX, c);
    buildassist_class = c;

	ps_file = gensym("file");
	ps_folder = gensym("folder");
	ps_external = gensym("external");
	ps_win_external = gensym("win_external");
}


/************************************************************************************/
// Object Creation Method

void *buildassist_new(t_symbol *msg, short argc, t_atom *argv)
{
	t_buildassist *x;									// Declare an object (based on our struct)
	long attrstart;

	attrstart = attr_args_offset(argc, argv);

	// Create object, store pointer to it (get 1 inlet free)
	x = (t_buildassist *)object_alloc(buildassist_class);		

	if(x){
		object_obex_store((void *)x, _sym_dumpout, (object *)outlet_new(x,NULL));	// dumpout	
		x->attr_type = ps_file;
		x->attr_path = gensym("none");
		x->attr_extension = gensym("none");
    	attr_args_process(x,argc,argv);		 //handle attribute args	
	}
	return (x);								// Return pointer to our instance
}


void buildassist_free(t_buildassist *x)
{
	item_count = 0;		// Reset the class global counter!
}


/************************************************************************************/
// Methods bound to input/inlets

// Method for Assistance Messages
void buildassist_assist(t_buildassist *x, void *b, long msg, long arg, char *dst)
{
	if(msg==1) 							// Inlets
		strcpy(dst, "Input");
	else if(msg==2){ 					// Outlets
		switch(arg){
			case 0: strcpy(dst, "Repetition Filtered Output"); break;
			case 1: strcpy(dst, "(bang) input is the same as the last input"); break;
			case 2: strcpy(dst, "dumpout"); break;
 		}
 	}
}


// Method called by collective builder
/*
 * Here is the prototype for fileusage_addfile:
 *		void fileusage_addfile(void *w, long flags, char *filename, short path);
 * flags can be:
 *		#define COLLECTIVE_FILECOPY 1 // copy this file to support folder instead of to app
 */
void buildassist_fileusage(t_buildassist *x, void *w)
{
	short	err;
	short	path_id;
	char	filename[512];
	void	*folderstate;
	long	filetype;
	short	descend = 0;	// API reference says this is unused (but required)
	short	i;
	
	for(i=0; i<item_count; i++){
		if(x->attr_path == item[i]){
			object_post((t_object *)x, "tap.buildassist already added the item called '%s'", x->attr_path->s_name);
			return;
		}
	}
	
	strcpy(filename, x->attr_path->s_name);

	#ifdef MAC_VERSION
		if(x->attr_extension == ps_external){
				strcat(filename, ".mxo");
		}
	#else // WIN_VERSION
		if((x->attr_extension == ps_external) || (x->attr_extension == ps_win_external)){
				strcat(filename, ".mxe");		
		}
	#endif
	
	if(x->attr_type == ps_file){
		err = locatefile_extended(filename, &path_id, &filetype, NULL, -1);		// FIND THE FILE
		if(err){
			object_error((t_object *)x, "cannot locate file: %s", filename);
			return;
		}
		fileusage_addfile(w, 0, filename, path_id);	
	}
	else if(x->attr_type == ps_folder){
		err = nameinpath(filename, &path_id);		// GET PATH
		if(err){
			object_error((t_object *)x, "cannot locate file: %s", filename);
			return;
		}
		folderstate = path_openfolder(path_id);		// OPEN FOLDER
		if(folderstate == 0){
			object_error((t_object *)x, "ERROR IN PATH_OPENFOLDER: %i", err);
			return;
		}
		while(path_foldernextfile(folderstate, &filetype, filename, descend)){		// ITERATE THROUGH FILES
			fileusage_addfile(w, 0, filename, path_id);			// DECLARE FILE FOR COLLECTIVE BUILDER
		}
		path_closefolder(folderstate);		// CLOSE FOLDER
	}
	else{
		object_error((t_object *)x, "invalid mode specified");
		return;
	}
	item[i] = x->attr_path;
	item_count++;
}
