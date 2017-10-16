/*
 *	Copyright (c) 1998,2004 Pertimm by Patrick Constant
 *	Dev : May 1998, February 2004
 *	Version 1.2
*/
#include <lpcmcr.h>
#include <lpcosys.h>
#include <logpath.h>
#include <logmlog.h>
#include <loggen.h>
#include <string.h>


/*#define DEVERMINE*/

/*
 *	Reads a file in a directory, with the idea
 *	of getting a variable and its value in the file name.
 *	the file has the following form :
 *	<exe_type><variable>_<value>.rqs
 *	or <exe_type><variable>_<value>.rqs
 *	for example hcheck_t.rqs to put the 
 *	'check' variable's value at 'true',
 *	for h (hard) process.
 *	When the file is found, it is renamed 
 *	in .ack (acknowlegment) extension.
 *	Returns 1 if found, 2 if found and acknowlegded.
 *  When variable finishes with a '_' we remove it
 *  and we make sure we are at the end of the file
 *  because of control files which can start with the
 *  same string.
*/

PUBLIC(int) OgGetVariableFile(char *where, char *variable0, char *value)
{
char *ptstr,*DOgMAIN,*DOgINSTANCE;
char variable[1024];
char file_path[1024];
char pattern_path[1024];
struct og_file cstr_file;
struct og_file *str_file = &cstr_file;
int ivariable,retour,found=0;

strcpy(variable,variable0);
memset(str_file,0,sizeof(struct og_file));

value[0]=0;

if((DOgMAIN=getenv("DOgMAIN")) && (DOgINSTANCE=getenv("DOgINSTANCE"))) {
  sprintf(pattern_path,"%s/%s/%s/%s/*.rqs",DOgMAIN,DOgPathMAINtoINSTANCES,DOgINSTANCE,DOgDirControl); }
else {
  sprintf(pattern_path, "%s/*.rqs",DOgDirControl); }

#ifdef DEVERMINE
MessageInfoLog(0,where,0,"Looking for '%s' with '%s'",pattern_path,variable);
getcwd(file_path,1024); MessageInfoLog(0,where,0,"cwd '%s'",file_path);
#endif

ivariable=strlen(variable);
if (variable[ivariable-1]=='_') variable[--ivariable]=0;
IFE(retour=OgFindFirstFile(str_file,pattern_path)); if (!retour) return(0);
do {
  char *s = str_file->File_Path;
  if (s[ivariable] == '_' || s[ivariable] == '.') {
    if (!Ogmemicmp(variable,s,ivariable)) {
      strcpy(file_path,s); found=1;
      break;
      }
    }
  }
while ( OgFindNextFile(str_file) );
OgFindClose(str_file);

IFn(found) return(0);

/** No '_' means a default value of 't' **/
if((ptstr=strchr(file_path,'_'))==NULL) strcpy(value,"t");
else strcpy(value,ptstr+1);
if((ptstr=strchr(value,'.'))!=NULL) *ptstr=0;

#ifdef DEVERMINE
MessageInfoLog(0,where,0,"Looking for '%s' with '%s' value is '%s'",pattern_path,variable,value);
#endif
return(1);
}



