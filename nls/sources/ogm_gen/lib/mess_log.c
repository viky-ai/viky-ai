/*
 *  This is for message logging
 *  Copyright (c) 1997,98-2000 Ogmios by M'hammed El Ajli and Patrick Constant
 *  Dev : October 1997, January 1998, June 2000
 *  Version 1.3
*/
#include <lpcosys.h>
#include <lpcerr.h>
#include <time.h>
#include <logmlog.h>
#include <sys/stat.h>
#include <stdarg.h>
#include <loggen.h>
#include <string.h>


#define DOgNormPathFlagNormal   1
#define DOgNormPathFlagOneDot   2
#define DOgNormPathFlagTwoDot   3
#define DOgNormPathFlagDeleted  4

struct og_norm_path {
  int start,length;
  int flag;
  };


#if (DPcSystem == DPcSystemUnix)

/*
 *  argv0 can be either a full path, and in that case this is it.
 *  or it can be a local path, it then needs to be combined
 *  with a getcwd.
*/
PUBLIC(int) OgGetExecutablePath(char *argv0, char *path)
{
char cwd[DPcPathSize];
int i,ipath;

if (argv0[0]=='/') {
  strcpy(path,argv0);
  }
else {
  getcwd(cwd,DPcPathSize);
  sprintf(path,"%s/%s",cwd,argv0);
  }

ipath=strlen(path);
for (i=ipath-1; i>=0; i--) {
  if (path[i]=='/') { ipath=i; path[ipath]=0; break; }
  }

OgNormalizePath(path,path);

return(0);
}



PUBLIC(int) OgGetTempPath(int itemp_path, char *temp_path, void *herr)
{
strcpy(temp_path,"/tmp/");

DONE;
}



#else
#if (DPcSystem == DPcSystemWin32)


PUBLIC(int) OgGetExecutablePath(argv0,path0)
char *argv0,*path0;
{
int i,ipath;
char path[DPcPathSize];

if(GetModuleFileName(NULL, path, 512)==0) return(1);
/* Because this function can add \\?\ at the beginning of the path,
 * we test it and remove it because we do not need. See the following link for more information:
 * http://msdn.microsoft.com/library/default.asp?url=/library/en-us/fileio/fs/naming_a_file.asp */
ipath=strlen(path);
if (ipath>=4 && !memcmp(path,"\\\\?\\",4)) {
  if (ipath>4) memmove(path,path+4,ipath-4); ipath-=4;
  }

for (i=ipath-1; i>=0; i--) {
  if (path[i]=='/' || path[i]=='\\') { path[i]=0; break; }
  else if (path[i]==':') { path[i+1]=0; break; }
  }

OgGetLongFileName(path,path0);
return(0);
}


/** This function is to be replaced by OgGetExecutablePath **/
PUBLIC(int) GetExePath(path)
char *path;
{
int i,ipath;

if(GetModuleFileName(NULL, path, 512)==0) return(1);
/* Because this function can add \\?\ at the beginning of the path,
 * we test it and remove it because we do not need. See the following link for more information:
 * http://msdn.microsoft.com/library/default.asp?url=/library/en-us/fileio/fs/naming_a_file.asp */
ipath=strlen(path);
if (ipath>=4 && !memcmp(path,"\\\\?\\",4)) {
  if (ipath>4) memmove(path,path+4,ipath-4); ipath-=4;
  }

for (i=ipath-1; i>=0; i--) {
  if (path[i]=='/' || path[i]=='\\') { path[i]=0; break; }
  else if (path[i]==':') { path[i+1]=0; break; }
  }
return(0);
}



PUBLIC(int) OgGetTempPath(itemp_path,temp_path,herr)
int itemp_path; char *temp_path; void *herr;
{
int retour;
char buffer[DPcPathSize];
char erreur[DOgErrorSize],sys_erreur[DOgErrorSize];

IFn(retour=GetTempPath(itemp_path,buffer)) {
  OgSysErrMes(OgSysErr(),DOgErrorSize/2,sys_erreur);
  sprintf(erreur, "OgGetTempPath: %s",sys_erreur);
  OgErr(herr,erreur); DPcErr;
  }
OgGetLongFileName(buffer,temp_path);
return(retour);
}


#endif
#endif





/*
 * Absolutizes and normalize path using Current Working Directory
 * OgAbsolutizePath(path,path) is working.
*/

PUBLIC(int) OgAbsolutizePath(char *path, char *absolutized_path)
{
char cwd[DPcPathSize],buffer[DPcPathSize];

if (OgIsAbsolutePath(path)) {
  IFE(OgNormalizePath(path,absolutized_path));
  }
else {
  getcwd(cwd,DPcPathSize);
  sprintf(buffer,"%s/%s",cwd,path);
  IFE(OgNormalizePath(buffer,absolutized_path));
  }

DONE;
}





PUBLIC(int) OgIsAbsolutePath(char *path)
{
if (path[0]=='/' || path[0]=='\\') return(1);
else if (isalpha(path[0]) && path[1]==':') return(1);
return(0);
}




/*
 *  Changes paths like "/home/ogpub/../pertimm" into "/home/pertimm"
 *  It is possible to use the same variable for both arguments
 *  as the building of normalized path is done at the end:
 *  OgNormalizePath(path,path) is working.
 *  Removes '/' or '\' at the end.
 *  We use DPcPathSize*2 because sometimes, normalization
 *  come from a double path.
*/

PUBLIC(int) OgNormalizePath(char *path, char *normalized_path)
{
int i, j, c, state=1, inp=0, end=0;
struct og_norm_path np[DPcPathSize*2];
int ipath, inpath, start, last_normal;
char npath[DPcPathSize*2];

/** Handles beginning of path when it is absolute **/
ipath = strlen(path);
if (ipath >= 1 && path[0] == '/') start=1;
else if (ipath >= 2 && !memcmp(path,"\\\\",2)) start=2;
else if (ipath >= 2 && path[1]==':') start=2;
else start=0;

for (i=start; !end; i++) {
  c = path[i];
  if (c==0) end=1;
  switch (state) {
    /** at the beginning or between directory names **/
    case 1:
      if (c=='/' || c=='\\') state=1;
      else {
        np[inp].start = i; np[inp].length = 0;
        np[inp].flag = DOgNormPathFlagNormal;
        state=2;
        }
      break;
    /** in directory name **/
    case 2:
      if (c=='/' || c=='\\' || c==0) {
        np[inp].length = i - np[inp].start;
        inp++; state=1;
        }
      break;
    }
  }

/** np[i].length can not be zero **/
for (i=0; i<inp; i++) {
  if (np[i].length == 1 && path[np[i].start]=='.') np[i].flag = DOgNormPathFlagOneDot;
  if (np[i].length == 2 && !memcmp(path+np[i].start,"..",2)) np[i].flag = DOgNormPathFlagTwoDot;
  }

for (i=0; i<inp; i++) {
  if (np[i].flag == DOgNormPathFlagTwoDot) {
    for (j=i-1; j>=0; j--) {
      if (np[j].flag == DOgNormPathFlagNormal) {
        np[j].flag = DOgNormPathFlagDeleted; break;
        }
      }
    }
  }

#ifdef DEVERMINE
for (i=0; i<inp; i++) {
  printf("%3d %d '%.*s'\n", np[i].length, np[i].flag, np[i].length, path+np[i].start);
  }
#endif

last_normal=0;
for (i=0; i<inp; i++) {
  if (np[i].flag != DOgNormPathFlagNormal) continue;
  last_normal = i;
  }

inpath=0;
if (start) {
  memcpy(npath,path,start); inpath+=start;
  if (path[1]==':') npath[inpath++]='/';
  }
for (i=0; i<inp; i++) {
  if (np[i].flag != DOgNormPathFlagNormal) continue;
  memcpy(npath+inpath,path+np[i].start,np[i].length);
  inpath += np[i].length;
  if (i<last_normal) npath[inpath++]='/';
  }


memcpy(normalized_path,npath,inpath);
normalized_path[inpath] = 0;

DONE;
}




/******************************************************************************
*
*       FUNCTION:       OgGetCmdParameter:
*
*       PURPOSE:        Gets one by one the strings that are separated
*            with blank characters, from the command line
*            of a WinMain function. We also handle the quotes.
*
*****************************************************************************/


PUBLIC(int) OgGetCmdParameter(char *cmd_line,char *cmd_param,int *pos)
{
int i,c,oldc=0,state=1;
int icmd_param=0;
for (i=(*pos); cmd_line[i]!=0; i++) {
  c = cmd_line[i];
  if (c=='\\' && cmd_line[i+1]=='"');
  else {
    switch (state) {
      case 1:
        if (c==' ') {
          if (icmd_param==0) continue;
          else goto endOgGetCmdParameter;
          }
        else if (oldc!='\\' && c=='"') state=2;
        else cmd_param[icmd_param++]=cmd_line[i];
        break;
      case 2:
        if (oldc!='\\' && c=='"') { i++; goto endOgGetCmdParameter; }
        else cmd_param[icmd_param++]=cmd_line[i];
        break;
      }
    }
  oldc=c;
  }

endOgGetCmdParameter:

*pos=i;
cmd_param[icmd_param]=0;
return(icmd_param);
}



/******************************************************************************
*
*       FUNCTION:       MessageInfoLog:
*
*       PURPOSE:        info for debug.
*
*****************************************************************************/

PUBLIC(int) MessageInfoLog(int mb,char *title,char *file_ptr, const char * format, ... )
{
int retour;
va_list arg_ptr;
char text[DOgMlogMaxMessageSize+1024];

va_start( arg_ptr, format );
vsprintf( text, format, arg_ptr );
if(mb==0)
  retour=OgMessageLog(DOgMlogInLog,title,file_ptr,"%s",text);
else if(mb==1)
  retour=OgMessageLog(DOgMlogInLog+DOgMlogMBox,title,file_ptr,"%s",text);
else if(mb==2)
  retour=OgMessageLog(DOgMlogMBox,title,file_ptr,"%s",text);
else retour=(-1);
va_end( arg_ptr );
return(retour);
}

/******************************************************************************
*
*       FUNCTION:       MessageInfoDLog:
*
*       PURPOSE:        info for debug and write date.
*
*****************************************************************************/

PUBLIC(int) MessageInfoDLog(int mb,char *title,char *file_ptr, const char * format, ... )
{
int retour=0;
va_list arg_ptr;
char text[DOgMlogMaxMessageSize+1024];

va_start( arg_ptr, format );
vsprintf( text, format, arg_ptr );
if(mb==0)
  retour=OgMessageLog(DOgMlogInLog+DOgMlogDate,
    title,file_ptr,"%s",text);
else if(mb==1)
  retour=OgMessageLog(DOgMlogInLog+DOgMlogDate+DOgMlogMBox,
    title,file_ptr,"%s",text);
else if(mb==2)
  retour=OgMessageLog(DOgMlogMBox,title,file_ptr,"%s",text);
va_end( arg_ptr );
return(retour);
}



/******************************************************************************
*
*       FUNCTION:       MessageErrorLog:
*
*       PURPOSE:        write Error message in file.
*
*****************************************************************************/

PUBLIC(int) MessageErrorLog(int mb,char *title,char * file_ptr,const char * format, ... )
{
int retour=0;
va_list arg_ptr;
char text[DOgMlogMaxMessageSize+1024];

va_start( arg_ptr, format );
vsprintf( text, format, arg_ptr );
if(mb==0)
  retour=OgMessageLog(
    DOgMlogInLog+DOgMlogInErr+DOgMlogSysErr+DOgMlogDate,
  title,file_ptr,"%s",text);
else if(mb==1)
  retour=OgMessageLog(
    DOgMlogInLog+DOgMlogInErr+DOgMlogSysErr+DOgMlogDate+DOgMlogMBox,
  title,file_ptr,"%s",text);
else if(mb==2)
  retour=OgMessageLog(
    DOgMlogSysErr+DOgMlogMBox,
  title,file_ptr,"%s",text);
va_end( arg_ptr );
return(retour);
}




/*
 *  To globally change the working directory for OgMessageLog function.
 *  passing zero resets to normal behavior (based upon cwd).
*/

static char *OgWorkingDirLog = 0;
static char OgWorkingDirLogContent[DPcPathSize];


PUBLIC(int) OgSetWorkingDirLog(const char *working_dir)
{
  IFn(working_dir) OgWorkingDirLog = 0;
  else
  {
    snprintf(OgWorkingDirLogContent, DPcPathSize, "%s", working_dir);
    OgWorkingDirLog = OgWorkingDirLogContent;
  }
  DONE;
}

PUBLIC(const char *) OgGetWorkingDirLog()
{
  return OgWorkingDirLog;
}


/*
 *  Dynamically changing the rotation size of the logs.
 *  default value is DOgMlogMaxFileSize (4mb)
*/
static size_t OgMlogMaxFileSize = DOgMlogMaxFileSize;

PUBLIC(int) OgMlogMaxFileSizeSet(size_t max_file_size)
{
OgMlogMaxFileSize = max_file_size;
DONE;
}




/******************************************************************************
*
*       FUNCTION:       OgMessageLog:
*
*       PURPOSE:        write any type of message in file.
*    mb is a binary combination of the following values :
*    DOgMlogInLog  : writes in the file 'LOG/title.log' the message 'file_ptr'
*    DOgMlogMBox   : the message 'file_ptr' is sent to a message box
*    DOgMlogInErr  : writes in the file 'LOG/title.err' the message 'file_ptr'
*    DOgMlogSysErr : gets the system error and writes it in the above files
*      DOgMlogDate   : writes the date in the above files on a separate line
*      DOgMlogDateIn : writes the date in the above files on the same line
*      DOgMlogNoCr   : doesn't write a CR at the end of the line
*      DOgMlogUnicode: writes the string (ascii) as Unicode
*
*****************************************************************************/

PUBLIC(int) OgMessageLog(int mb,char *title,char * file_ptr,
             const char * format, ... )
{
int i,itext,file_exists=0;
FILE *file_log, *file_err;
/* We use big buffer, because we don't want printing problems
 * with those functions, but we still must be careful with
 * the size of what we print...
*/
char text[DOgMlogMaxMessageSize+1024], textc[DOgMlogMaxMessageSize+1024];
char LogFilename[DPcPathSize];
char ExLogFilename[DPcPathSize];
char file_buffer[DPcPathSize];
char *file, *pt, *cr;
struct stat etat;
va_list arg_ptr;
time_t time_of_day;
char *mode;

va_start( arg_ptr, format );
vsnprintf( text, DOgMlogMaxMessageSize/2, format, arg_ptr ); text[DOgMlogMaxMessageSize/2]=0;
va_end( arg_ptr );

if (file_ptr==0) {
  if (title==0) DONE;
  if (OgWorkingDirLog) sprintf(file_buffer,"%s/log/%s.log",OgWorkingDirLog,title);
  else sprintf(file_buffer,"log/%s.log",title);
  file=file_buffer;
  }
else {
  if (OgWorkingDirLog) {
    /** We check if file_buffer is absolute to avoid concatenation **/
    if (file_ptr[0]=='/' || file_ptr[0]=='\\' || file_ptr[1]==':') file=file_ptr;
    else {
      sprintf(file_buffer,"%s/%s",OgWorkingDirLog,file_ptr);
      file=file_buffer;
      }
    }
  else file=file_ptr;
  }

if (mb & DOgMlogSysErr) OgOgMessageLog(mb, text, textc);
else sprintf(textc,"%s",text);

time_of_day = time(NULL);

if (mb & DOgMlogNoCr) cr=""; else cr="\n";
if (mb & DOgMlogUnicode) mode = "a+b"; else mode = "a+";
if (mb & (DOgMlogInLog+DOgMlogInErr)) { sprintf( LogFilename, "%s", file ); }
if (mb & DOgMlogInLog) {
  if ((stat(LogFilename,&etat)==0)&&(etat.st_size>OgMlogMaxFileSize)) {
  strcpy(ExLogFilename,LogFilename);
  pt=ExLogFilename+strlen(ExLogFilename);
  pt--;*pt='f';
  remove(ExLogFilename);
  rename(LogFilename,ExLogFilename);
    }
  if (mb & DOgMlogUnicode) {
    file_exists=OgFileExists(LogFilename);
    }
  IFx(file_log=fopen(LogFilename,mode)) {
    if (mb & DOgMlogDate)
      sprintf(text,"%s",OgGmtime( &time_of_day));
    if (mb & DOgMlogDateIn)
      sprintf(text,"%s at %.24s%s",textc,OgGmtime( &time_of_day),cr);
    else
      sprintf(text,"%s%s",textc,cr);
    if (mb & DOgMlogUnicode) {
      if (!file_exists) { fputc(0xfe,file_log); fputc(0xff,file_log); }
      itext = strlen(text);
      for (i=0; i<itext; i++) {
        fputc(0,file_log); fputc(text[i],file_log);
        }
      }
    else fprintf(file_log,"%s",text);
    fclose(file_log);
    }
  }

if(mb & DOgMlogInErr) {
  strcpy(LogFilename+strlen(LogFilename)-4,".err");
  if (mb & DOgMlogUnicode) {
    file_exists=OgFileExists(LogFilename);
    }
  IFx(file_err=fopen(LogFilename,mode)){
    if (mb & DOgMlogDate)
      sprintf(text,"%s",OgGmtime( &time_of_day));
    if (mb & DOgMlogDateIn)
      sprintf(text,"%s at %.24s%s",textc,OgGmtime( &time_of_day),cr);
    else
      sprintf(text,"%s%s",textc,cr);
    if (mb & DOgMlogUnicode) {
      if (!file_exists) { fputc(0xfe,file_err); fputc(0xff,file_err); }
      itext = strlen(text);
      for (i=0; i<itext; i++) {
        fputc(0,file_err); fputc(text[i],file_err);
        }
      }
    else fprintf(file_err,"%s",text);
    fclose(file_err);
  }
  }

if(mb & DOgMlogMBox) {
  OgMessageBox(NULL,textc,title,DOgMessageBoxInformation);
  }

DONE;
}




PUBLIC(int) OgMessageShortUnicodeLog(char *filename, int ibuffer, short int *buffer)
{
FILE *fd;
char file[DPcPathSize];
int i,low,high,file_exists;

if (OgWorkingDirLog) {
  /** We check if file_buffer is absolute to avoid concatenation **/
  if (filename[0]=='/' || filename[0]=='\\' || filename[1]==':');
  else {
    sprintf(file,"%s/%s",OgWorkingDirLog,filename);
    filename=file;
    }
  }

file_exists=OgFileExists(filename);
IFn(fd=fopen(filename,"a+b")) DONE;
if (!file_exists) { fputc(0xfe,fd); fputc(0xff,fd); }
for (i=0; i<ibuffer; i++) {
  high=(buffer[i]>>8); low=(buffer[i]&0xff);
  fputc(high,fd); fputc(low,fd);
  }
fclose(fd);
DONE;
}





PUBLIC(int) OgMessageUnicodeLog(char *filename, int ibuffer, unsigned char *buffer)
{
FILE *fd;
int i,file_exists;
char file[DPcPathSize];

if (OgWorkingDirLog) {
  /** We check if file_buffer is absolute to avoid concatenation **/
  if (filename[0]=='/' || filename[0]=='\\' || filename[1]==':');
  else {
    sprintf(file,"%s/%s",OgWorkingDirLog,filename);
    filename=file;
    }
  }

file_exists=OgFileExists(filename);
IFn(fd=fopen(filename,"a+b")) DONE;
if (!file_exists) { fputc(0xfe,fd); fputc(0xff,fd); }
for (i=0; i<ibuffer; i++) {
  fputc(buffer[i],fd);
  }
fclose(fd);
DONE;
}




PUBLIC(int) OgLogErr(int mb,char *where, char *c_file, int c_line)
{
int h;
char erreur[DPcSzErr];
while(PcErrDiag(&h,erreur)) {
  OgMessageLog(mb,where,0,"%s (%s:%d)",erreur,c_file,c_line);
  }
return(0);
}


