/*
 *	Set of functions that basically handle file messages
 *	Copyright (c) 1998	Ogmios by M'hammed El Ajli
 *	Dev : May 1998
 *	Version 1.0
*/
#include <logmlog.h>
#include <logpath.h>
#include <lpcmcr.h>
#include <loggen.h>

static int OgDirLookMessage1(char *, char *, int, int);



/******************************************************************************
*
*       FUNCTION:       OgSendMessage:
*
*       PURPOSE:        send flag message with file   
*
*****************************************************************************/
PUBLIC(int) OgSendMessage(char*where,char *exe_name,char *ext,int delete_message)
{
return(OgDirSendMessage(DOgDirControl,where,exe_name,ext,delete_message));
}


PUBLIC(int) OgDirSendMessage(char *control_dir,char*where,char *exe_name,char *ext,int delete_message)
{
FILE *fd;
char message_file[DogMaxPathSize];
char file_path_nul[DogMaxPathSize];

sprintf(file_path_nul,"%s/%s%s",control_dir,exe_name,DOgExt_Nul);
sprintf(message_file,"%s/%s%s",control_dir,exe_name,ext);
/**creer le fichier s'il n'existe pas**/

if(access(message_file,0)!=0){
  if (delete_message) {
    OgDirDeleteMessages(control_dir,where,exe_name);
  }
  IF(OgCheckOrCreateDir(message_file,1,where)) { 
    MessageErrorLog(0,where,0,"OgSendMessage: can't create directories in %s ",message_file);    
    DPcErr;
  }
  if(rename(file_path_nul,message_file)){
    if ((fd=fopen(message_file,"w"))==0) {
	  MessageErrorLog(0,where,0,"OgSendMessage: can't create %s ",message_file);    
	  DPcErr;
    }
    fclose(fd);
  }
   /** It is better to write log message out of this function **/
  /** Otherwise, every program has this log message **/
  /* MessageInfoLog(0,where,0,"OgSendMessage: message '%s' sent",message_file); */
}
DONE;
}





/******************************************************************************
*
*       FUNCTION:       OgDeleteMessages:
*
*       PURPOSE:        deletes all flags for program defined by 'basi_name'
*
*****************************************************************************/

PUBLIC(int) OgDeleteMessages(char *where, char *exe_name)
{
return(OgDirDeleteMessages(DOgDirControl,where,exe_name));
}


PUBLIC(int) OgDirDeleteMessages(char *control_dir,char *where, char *exe_name)
{
char search_path[DogMaxPathSize];
char file_path[DogMaxPathSize];
char file_path_nul[DogMaxPathSize];
struct og_file cstr_file;
struct og_file *str_file = &cstr_file;
int retour;

sprintf(search_path,"%s/%s.*",control_dir,exe_name);
sprintf(file_path_nul,"%s/%s%s",control_dir,exe_name,DOgExt_Nul);

#ifdef DEVERMINE
MessageInfoLog(0,where,0,"OgDeleteMessages: deleting control files '%s'\n",search_path);   
#endif

memset(str_file,0,sizeof(struct og_file));

IFE(retour=OgFindFirstFile(str_file,search_path)); if(!retour) DONE;

do {
  sprintf(file_path,"%s/%s",control_dir,str_file->File_Path);
  if(rename(file_path,file_path_nul)){
    IF(unlink(file_path)) {
      MessageInfoLog(0,where,0,"OgDeleteMessages: impossible to delete file '%s'",
		file_path);
	  continue;
    }
  }
#ifdef DEVERMINE
  MessageInfoLog(0,where,0,"OgDeleteMessages: renaming '%s' into %s",file_path,file_path_nul);
#endif
  }
while (OgFindNextFile(str_file));

OgFindClose(str_file);
DONE;
}

/******************************************************************************
*
*       FUNCTION:       OgLookMessage:
*
*       PURPOSE:        Look message, return :
*						0 if nothing special
*						1 if suspend
*						2 if stop
*
*****************************************************************************/

#if(DPcSystem == DPcSystemUnix)

#include <string.h>
#include <strings.h>
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <logpath.h>



PUBLIC(int) OgLookSystemMessageEx(char *exe_name,int *MessFinishNow,int trace)
{
return(*MessFinishNow);
}


PUBLIC(int) OgDirLookMessageEx(char *control_dir, char *exe_name, int *MessFinishNow, int trace)
{
if(*MessFinishNow==2) return(2);
*MessFinishNow=OgDirLookMessage1(control_dir,exe_name,*MessFinishNow,trace);
if(*MessFinishNow==2) return(*MessFinishNow);
return (*MessFinishNow);
}


#else
#if(DPcSystem == DPcSystemWin32)

#include <windows.h>
#include <io.h>


/*
 *  When we create a window with CreateWindow, we need to read the 
 *  message list very often because otherwise, opening a program like word
 *  takes a long time (more than one minute instead of a few seconds).
 *  This is certainly due to some time-outs on the message queue.
 *  This function is thus supposed to be called at least every second.
*/

PUBLIC(int) OgLookSystemMessageEx(char *exe_name,int *MessFinishNow, int trace)
{
MSG msg;
if(*MessFinishNow==2) return(2);
if(PeekMessage(&msg, 0, 0, 0,PM_REMOVE)) {
  if (msg.message== WM_QUERYENDSESSION) {
    if (trace) MessageInfoLog(0,exe_name,0,"End Session (from OgLookSystemMessage)");
    *MessFinishNow=2;
    }
  }
return(*MessFinishNow);
}



PUBLIC(int) OgDirLookMessageEx(control_dir,exe_name,MessFinishNow,trace)
char *control_dir,*exe_name; int *MessFinishNow; int trace;
{
if(*MessFinishNow==2) return(2);
*MessFinishNow=OgDirLookMessage1(control_dir,exe_name,*MessFinishNow,trace);
if(*MessFinishNow==2) return(*MessFinishNow);
OgLookSystemMessageEx(exe_name,MessFinishNow,trace);
return(*MessFinishNow);
}

#endif
#endif



PUBLIC(int) OgLookSystemMessage(char *exe_name, int *MessFinishNow)
{
return(OgLookSystemMessageEx(exe_name,MessFinishNow,1));
}



PUBLIC(int) OgDirLookMessage(char *control_dir, char *exe_name, int *MessFinishNow)
{
return(OgDirLookMessageEx(control_dir,exe_name,MessFinishNow,1));
}



PUBLIC(int) OgLookMessage(char *exe_name, int *MessFinishNow)
{
return(OgDirLookMessage(DOgDirControl,exe_name,MessFinishNow));
}

/******************************************************************************
*
*       FUNCTION:       OgDirLookMessage1:
*
*       PURPOSE:        Looks for file message. Is a static function 
*						because we use 'OgLookMessage' outside of the DLL
*
*****************************************************************************/

static int OgDirLookMessage1(char *control_dir,char *exe_name,int MessF,int trace)
{
char message_file[1024];
  
if(MessF==2) return(MessF);
sprintf(message_file,"%s/%s%s",control_dir,exe_name,DOgExt_Stp);
if(!access(message_file,0)){
  if(MessF!=2 && trace) MessageInfoLog(0,exe_name,0,"Message file: '%s' ",message_file);    
  MessF=2;
  return(MessF);
}
sprintf(message_file,"%s/%s%s",control_dir,exe_name,DOgExt_Spd);
if(!access(message_file,0)){
  if(MessF!=1 && trace) MessageInfoLog(0,exe_name,0,"Message file: '%s'",message_file);    
  MessF=1;
  return(MessF);
}
return(MessF);
}




/******************************************************************************
*
*       FUNCTION:       OgLookMessageFile:
*
*       PURPOSE:        Looks for a file message
*
*****************************************************************************/

PUBLIC(int) OgLookMessageFile(char *exe_name, char *ext)
{
return(OgDirLookMessageFile(DOgDirControl,exe_name,ext));
}


PUBLIC(int) OgDirLookMessageFile(char *control_dir, char *exe_name, char *ext)
{
char message_file[1024];
sprintf(message_file,"%s/%s%s",control_dir,exe_name,ext);
if(!access(message_file,0)){
  return(1);
  }
return(0);
}




/******************************************************************************
*
*       FUNCTION:       OgRemoveMessageFile:
*
*       PURPOSE:        Removes a file message
*
*****************************************************************************/


PUBLIC(int) OgRemoveMessageFile(char *exe_name,char *ext)
{
char message_file[DogMaxPathSize];
char file_path_nul[DogMaxPathSize];

sprintf(file_path_nul,"%s/%s%s",DOgDirControl,exe_name,DOgExt_Nul);
sprintf(message_file,"%s/%s%s",DOgDirControl,exe_name,ext);
if(rename(message_file,file_path_nul)){
	return(remove(message_file));
}
return(0);
}
