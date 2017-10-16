/*
 *	This is filename handling utilities for indexing chain.
 *	Copyright (c) 1997,98	Ogmios by M'hammed El Ajli
 *	Dev : October 1997, January 1998
 *	Version 1.1
*/
#include <lpcosys.h>
#include <string.h>
#include <stdarg.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <logmlog.h>
#include <loggen.h>
#include <glib.h>
#include <glib/gstdio.h>


/******************************************************************************
*
*       FUNCTION:       RenameTemp:
*
*       PURPOSE:        rename file with the extention .tmp to
*						to the normaly file.  
*	
*****************************************************************************/
PUBLIC(int) OgRename_Ext(char *file1,char *fileext,
					   int flag_error, char *where, char *c_file, int c_line)
{ 
  char		*pt,filenew[1024];
  int			valret=0;
  strcpy(filenew,file1);
  pt=strrchr(filenew,'.');
  strcpy(pt,fileext);
  if ((valret=rename(file1,filenew))) {
    MessageErrorLog(0,where,0,
	  "RenameTemp: Impossible to rename '%s' into '%s' (%s:%d)",
      file1,filenew,c_file,c_line);
    if (flag_error&1) return(valret);
    MessageErrorLog(0,where,0,"RenameTemp: exiting program %s.exe",where);
    exit(0);
  }
  return(valret);
}


/******************************************************************************
*
*       FUNCTION:       WriteToFile:
*
*       PURPOSE:        open write and close file .
*
*****************************************************************************/
PUBLIC(int) MeWriteToFile(char *pathfile,char *ffile, 
					const void * buff,unsigned long leng)
{
	FILE			*sFfile;
	char			FFilename[1024];	

	if(pathfile==NULL) sprintf( FFilename,"%s",ffile);
	else sprintf( FFilename,"%s%s",pathfile,ffile);

	if ((sFfile = fopen(FFilename,"a+")) == NULL) return(1);
	fwrite(buff,1,leng,sFfile);
	/*fprintf(sFfile,"%s\n");*/
	fclose(sFfile);	
	return(0);
}



/******************************************************************************
*
*       FUNCTION:       WriteToFile:
*
*       PURPOSE:        open write and close file .
*
*****************************************************************************/
PUBLIC(int) WriteToFile(char *pathfile, char *ffile, 
				const char *format, ... )
{
	FILE			*sFfile;
	char			text[1024];
	char			FFilename[1024];	
	va_list         arg_ptr;

	va_start( arg_ptr, format );
	vsprintf( text, format, arg_ptr );
	va_end( arg_ptr );
	if(pathfile==NULL) sprintf( FFilename,"%s",ffile);
	else sprintf( FFilename,"%s%s",pathfile,ffile);

	if ((sFfile = fopen(FFilename,"a+")) == NULL) return(1);
	
	fprintf(sFfile,"%s",text);
	fclose(sFfile);	
	return(0);
}

/******************************************************************************
*
*       FUNCTION:       OgCheckOrCreateDir
*
*       PURPOSE:        check if path exists, otherwise,
*						creates the corresponding directory
*						if isfile is true, the last object
*						is considered as a file and thus not created
*						as a directory.
*
*****************************************************************************/
PUBLIC(int) OgCheckOrCreateDir(char *path0, int isfile, char *where) 
{
int i,state,c;
int iname_dir;
int len=strlen(path0);
char path[DPcPathSize],name_dir[DPcPathSize];
char erreur[DPcSzErr];
struct stat info;

strcpy(path,path0);

if (isfile) {
  for (i=len-1; i>=0; i--) {
	if (path[i]=='/' || path[i]=='\\') { len=i; path[len]=0; break; }
    }
  /** simple filename without '/' or '\\' **/
  if (i<0) return(0);
  }

/** don't do anything if directory exists **/
if (stat(path,&info)==0) return(0);

state=1; iname_dir=0;
for (i=0; i<=len; i++) {
  if (path[i]=='/' || path[i]=='\\' || path[i]==0) {
    int iiname_dir=iname_dir;
	/*MessageInfoLog(0,where,0,"OgCheckOrCreateDir: found path '%s' len=%d i=%d",path,len,i);*/
	name_dir[iname_dir]=0; iname_dir=0; c=path[i];
	/** Volume specification, don't create any directory **/
	if (iiname_dir==2 && name_dir[1]==':') continue;
    if (iiname_dir==0) continue;
    path[i]=0;
	/*MessageInfoLog(0,where,0,"OgCheckOrCreateDir: try path '%s' len=%d i=%d",path,len,i);*/
	/** The directory doesn't exist, create it **/
	if (stat(path,&info)==(-1)) {
	/*MessageInfoLog(0,where,0,"OgCheckOrCreateDir: creating dir '%s'",path);*/
	  if (OgMkdir(path,0777)==(-1)) { 
        path[i]=c; sprintf(erreur,"OgCheckOrCreateDir: cannot create directory '%.180s'", path);
        PcErr(-1,erreur); DPcErr;
        }
	  }
	/** This is not a directory, error **/
    else if (!OgIsDir(info.st_mode)) {
      sprintf(erreur,"OgCheckOrCreateDir: not a directory '%.180s'", path);
      PcErr(-1,erreur); DPcErr;
	  }
    path[i]=c;
	/** The directory exists **/
	continue;
    }
  name_dir[iname_dir++]=path[i];
  }
return(0);
}



/*
 *  Checks if a path is an existing directory or not.
 *  returns zero if the file/directory 'path0' can be created
 *  returns the length where a file exists. Typically, we have:
 *  /g/cc.in2p3.fr/doc/phpman.php/man/ok/0/index.html 
 *  but file /g/cc.in2p3.fr/doc/phpman.php exists
 *  so it returns strlen(/g/cc.in2p3.fr/doc/phpman.php).
 *  This permits to create an md5 directory from the file name.
 *  so that grabbed files can be downloaded properly.
 *  typically we would have: /g/cc.in2p3.fr/doc/dir-n3tsly
 *  and grabbed file would be written in:
 *  /g/cc.in2p3.fr/doc/dir-n3tsly/man/ok/0/index.html
*/

PUBLIC(int) OgCheckDir(char *path0, int isfile) 
{
int i,c,len=strlen(path0);
char path[DPcPathSize];
struct stat info;

strcpy(path,path0);

if (isfile) {
  for (i=len-1; i>=0; i--) {
	if (path[i]=='/' || path[i]=='\\') { len=i; path[len]=0; break; }
    }
  }

for (i=0; i<=len; i++) {
  if (path[i]=='/' || path[i]=='\\' || path[i]==0) {
	c=path[i]; 
	/** Volume specification, don't create any directory **/
	if (i==2 && path[1]==':') continue;
    if (i==0) continue; path[i]=0;
    /** The directory or file doesn't exist **/
	if (stat(path,&info)==(-1)) return(0);
	/** This is not a directory, most likely a file **/
    else if (!OgIsDir(info.st_mode)) return(i);
    path[i]=c; /** The directory exists **/
	continue;
    }
  }
return(0);
}




/******************************************************************************
*
*       FUNCTION:       OgGetFileSize
*
*       PURPOSE:        gets file size
*
*****************************************************************************/
PUBLIC(int) OgGetFileSize(char *path)
{
struct stat file_stat;
if (g_stat(path,&file_stat)) return(0);
return(file_stat.st_size);
}
