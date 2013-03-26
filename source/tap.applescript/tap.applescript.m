/* 
 *	External object for Max/MSP
 *	Copyright © 2007 by Timothy Place
 * 
 *	License: This code is licensed under the terms of the "New BSD License"
 *	http://creativecommons.org/licenses/BSD/
 */

#include <cstdarg>
#include "ext.h"
BEGIN_USING_C_LINKAGE
#import "MaxObject/MaxObject.h"
#import "MaxObject/maxobjectbridge.h"
END_USING_C_LINKAGE


@interface TapApplescript : MaxObject 
{
	@public
		t_symbol		*filename;
	@private
		NSAppleScript	*appleScriptObject;
}		
- (id) initWithObject:(t_object *)x name:(t_symbol *)s numArgs:(long)argc andValues:(t_atom *)argv;
- (void)dealloc;
- (t_max_err) bangMessage;
- (t_max_err) readTypedMessage:(t_symbol *)s withNumArgs:(long)argc andValues:(t_atom *)argv;
- (t_max_err) readMessage:(t_symbol*)s;
- (void) doRead:(t_symbol *)s withNumArgs:(long)argc andValues:(t_atom *)argv;
- (t_max_err) scriptTypedMessage:(t_symbol *)s withNumArgs:(long)argc andValues:(t_atom *)argv;
@end


// Entry Point when loaded by Max
int main(void)
{
	[MaxObject createMaxClassWithName:"tap.applescript" fromObjcClassWithName:"TapApplescript"];
	return 0;
}


/********************************************************************************************************/
@implementation TapApplescript

- (id) initWithObject:(t_object *)x name:(t_symbol *)s numArgs:(long)argc andValues:(t_atom *)argv
{
	[super initWithObject:x name:s numArgs:argc andValues:argv];
	[self createInletWithIndex:0 named:"input"		withAssistanceMessage:"bang executes script, other messages set it up."];
	[self createOutletWithIndex:0 named:"result"	withAssistanceMessage:"data returned from the script"];
	[self createOutletWithIndex:1 named:"done"		withAssistanceMessage:"sends bang when a script is completely read from a file."];

	atom_arg_getsym(&filename, 0, argc, argv);
	if(filename)
		[self readMessage:filename];

	return self;
}

- (void)dealloc
{
    [super dealloc];
}

/********************************************************************************************************/

void appendatom(t_atom *a, char *s) 
{
	char buf[ATOM_MAX_STRLEN];
	char c;
	
	c = a->a_type;
	buf[0] = 0;
	if (c == A_SYM)
		snprintf(buf, ATOM_MAX_STRLEN, "%s ", a->a_w.w_sym->s_name);
//	else if (c == A_OBJ)
//		snprintf(buf, ATOM_MAX_STRLEN, "%s ", OB_NAME(a->a_w.w_obj));
	else if (c == A_LONG)
		snprintf(buf, ATOM_MAX_STRLEN, "%ld ", a->a_w.w_long);
	else if (c == A_DOLLAR)
		snprintf(buf, ATOM_MAX_STRLEN, "$%ld ", a->a_w.w_long);
	else if (c == A_FLOAT)
		snprintf(buf, ATOM_MAX_STRLEN, "%f ", a->a_w.w_float);
	else if (c == A_DOLLSYM)	// $ddz$ - added 7/10/2001
		snprintf(buf, ATOM_MAX_STRLEN, "$%s", a->a_w.w_sym->s_name);
	else
		return;
	strcat(s,buf);
}

void atoms_totext(char *s, short ac, t_atom *av)
{
	long i;
	long sl;
	
	s[0] = 0;
	for (i=0; i < ac; i++,av++)
		appendatom(av,s);		
	sl = strlen(s);
	if (s[sl-1] == ' ')
		s[sl-1] = 0;
}


- (t_max_err) bangMessage
{
	NSDictionary			*errorInfo = NULL;
	NSAppleEventDescriptor	*result = NULL;
	NSString				*resultString = NULL;
	char					*resultCString = NULL;
	
	if(appleScriptObject == nil)
		return MAX_ERR_GENERIC;

	result = [appleScriptObject executeAndReturnError:&errorInfo];
	if(result){
		resultString = [result stringValue];
		resultCString = (char*)[resultString cString];
		if(resultCString)
			[self sendMessage: gensym(resultCString) toOutletNamed: _sym_result];
	}
	return MAX_ERR_NONE;
}

- (t_max_err) readTypedMessage:(t_symbol *)s withNumArgs:(long)argc andValues:(t_atom *)argv
{
	if(argc)
		defer(maxObjectBridge, (method)maxobject_method, gensym("doRead:withNumArgs:andValues:"), 1, argv);
	else
		defer(maxObjectBridge, (method)maxobject_method, gensym("doRead:withNumArgs:andValues:"), 0, argv);
	return MAX_ERR_NONE;
}

- (t_max_err) readMessage:(t_symbol*)s
{
	t_atom a;
	
	atom_setsym(&a, s);
	return [self readTypedMessage:nil withNumArgs:1 andValues:&a];
}

- (void) doRead:(t_symbol *)s withNumArgs:(long)argc andValues:(t_atom *)argv
{
	char 			aFilename[256];			// for storing the name of the file locally
	char 			fullpath[512];			// path and name passed on to the xml parser
	char			fullpath2[512];
	short 			path;					// pathID#
	t_fourcc		outtype;				// the file type that is actually true
	char 			*temppath;
	NSString 		*formattedPath;

	// FIND THE FILE WE WANT TO READ
	if(!argc){
		if(open_dialog(aFilename, &path, &outtype ,NULL, -1))				// Returns 0 if successful
			return;															// User Cancelled
	}
	else{
		strcpy(aFilename, atom_getsym(argv)->s_name);						// Copy symbol argument to a local string
		if(locatefile_extended(aFilename, &path, &outtype, NULL, -1)){		// Returns 0 if successful
			object_error(maxObjectBridge, "file not found - %s", s->s_name);
			return;															// Not found
		}
	}

	path_topathname(path, aFilename, fullpath);
	temppath = strchr(fullpath, ':');
	*temppath = '\0';
	temppath += 1;

	// at this point temppath points to the path after the volume, and out_filepath points to the volume
	sprintf(fullpath2, "/Volumes/%s%s", fullpath, temppath);

	formattedPath = [[NSString alloc] initWithUTF8String:fullpath2];

	if(appleScriptObject != nil)
		[appleScriptObject release];	
	appleScriptObject = [[NSAppleScript alloc] initWithContentsOfURL:[NSURL fileURLWithPath:formattedPath] error:nil];
	[self sendMessage: gensym("bang") toOutletNamed: gensym("done")];
}


- (t_max_err) scriptTypedMessage:(t_symbol *)s withNumArgs:(long)argc andValues:(t_atom *)argv
{
	char		text[ATOM_MAX_STRLEN];
	NSString	*source;
	
	if(appleScriptObject != nil)
		[appleScriptObject release];
	
	atoms_totext(text, argc, argv);
	source = [[NSString alloc] initWithUTF8String:text];
	appleScriptObject = [[NSAppleScript alloc] initWithSource:source];
	[source release];
	return MAX_ERR_NONE;
}

@end
