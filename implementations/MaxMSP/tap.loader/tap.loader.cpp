/* 
	tap.loader
	when max is launched, check for the presence of the required shared libraries
	if not present, then copy them into place
 
	finally, init the shared libs by loading one of the externs that links to them

	Copyright © 2009
*/

		//#import <Cocoa/Cocoa.h>
		//#include <Carbon/Carbon.h>
		//#include <dlfcn.h>

#include "ext.h"
#include "ext_obex.h"
#include "commonsyms.h"


// Data Structure for this object
typedef struct _loader{
	t_object		ob;	
} t_loader;


// Prototypes
void*	loader_new(t_symbol *name, long argc, t_atom *argv);
//void	loader_checklibs(void);
//void	loader_loadobj(NSString* mainBundlePath);


// Globals
t_class		*loader_class;


/************************************************************************************/
// Main() Function

int main(void)
{
	t_class *c;

	c = class_new((char*)"tap.loader",(method)loader_new, (method)NULL, sizeof(t_loader), (method)NULL, A_GIMME, 0);
	common_symbols_init();

	class_register(_sym_box, c);
	loader_class = c;
	
//	loader_checklibs();
	
	post("TapTools 4             by Timothy Place             http://74objects.com/");
	
	return 0;
}


/************************************************************************************/
// Object Life

void *loader_new(t_symbol *name, long argc, t_atom *argv)
{
	t_loader *self = (t_loader*)object_alloc(loader_class);

	if (self) {
		; // this object does nothing...
	}
	return self;
}


// TODO: we can copy these files if they don't exist, but what if they exist and the versions are old?
/*
void loader_checklibs(void)
{	
	NSFileManager *fileManager = [NSFileManager defaultManager];
	NSBundle* mainBundle = [NSBundle mainBundle];
	NSString* mainBundlePath = [mainBundle bundlePath];
	NSMutableString* taptoolsPath = [[NSMutableString alloc] initWithString:mainBundlePath];
	NSMutableString* librariesPath = [[NSMutableString alloc] initWithString:mainBundlePath];
	NSMutableString* extensionsPath = [[NSMutableString alloc] initWithString:mainBundlePath];
	BOOL isDirectory = NO;
	NSError* err = nil;
	
	[taptoolsPath appendString: @"/Contents/TapTools"];
	[librariesPath appendString: @"/Contents/TapTools/lib"];
	[extensionsPath appendString: @"/Contents/TapTools/extensions"];
	
	if (![fileManager fileExistsAtPath:librariesPath isDirectory: &isDirectory]) {
		[fileManager createDirectoryAtPath:taptoolsPath withIntermediateDirectories:TRUE attributes:nil error:&err];
		[fileManager copyItemAtPath:@"/Library/Application Support/TapTools/lib"  toPath:librariesPath error:&err];
		[fileManager copyItemAtPath:@"/Library/Application Support/TapTools/extensions" toPath:extensionsPath error:&err];
	}
	
	loader_loadobj(mainBundlePath);
}


void loader_loadobj(NSString* mainBundlePath)
{
	NSMutableString* tappathPath = [[NSMutableString alloc] initWithString:mainBundlePath];
	const char* cstr;
	void*	executable;

	[tappathPath appendString: @"/../Cycling '74/TapTools/TapTools Externals/tap.path.mxo/Contents/MacOS/tap.path"];
//	NSMutableString* tappathPath = [[NSMutableString alloc] initWithUTF8String:"/Applications/Max6/Cycling '74/TapTools/TapTools Externals/tap.path.mxo/Contents/MacOS/tap.path"];
//	NSMutableString* tappathPath = [[NSMutableString alloc] initWithUTF8String:"/Applications/Max6/TapTools/TapTools Externals/tap.path.mxo/Contents/MacOS/tap.path"];
	
	cstr = [tappathPath UTF8String];
	
	executable = dlopen(cstr, RTLD_LAZY);
	if (executable) {
		typedef void (*fn)();
		fn f = (fn)dlsym(executable, "main");
		
		if (f)
			(*f)();
	}
	else {
		post("TapTools failed to load.  Here's why:");
		post(dlerror());
	}
}

*/
