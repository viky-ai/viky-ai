/*
 *  Main program for Message Management
 *  Copyright (c) 2007 Pertimm by Patrick Constant and Guillaume Logerot
 *  Dev : March,April 2007
 *  Version 1.1
*/
#include "ogm_msg.h"
#include <stdarg.h>
#include <logpath.h>

#if (DPcSystem!=DPcSystemWin32)
#include <unistd.h>
#include <sys/syscall.h>
#include <sys/types.h>
#endif

static int    logMsg(void *handle, char criticity, const char *format, va_list args);

/**
 * Init Log and Error handler
 * \param moduleName
 * \param where
 * \param logMutex (NULL=> init. new mutex)
 * \return log handler or NULL in case of error
 **/
PUBLIC(void *) OgLogInit(const char *moduleName, const char *pipeName, int traceLevel, int mode) {
  char erreurMsg[DOgErrorSize];
  void *hmsg;
  struct og_msg_param msg_param;
  char label[DPcPathSize];

  // init ogm_msg_param
  memset(&msg_param,0,sizeof(struct og_msg_param));
  IFn(msg_param.herr=OgErrInit()) {
    fprintf(stderr,"Internal Error %d (%s): OgInitLog failed (OgErrInit)",__LINE__,moduleName==NULL?"":moduleName); // tODO
    return NULL;
  }

  msg_param.hmutex=(ogmutex_t *) malloc(sizeof(ogmutex_t)); // init mutex but deprecated for OgLog functions
  IFn(moduleName) sprintf(label,"log");
  else sprintf(label,"%s_log",moduleName);
  IF (OgInitCriticalSection(msg_param.hmutex,label)) {
    fprintf(stderr,"Internal Error %d (%s): OgInitLog failed (OgInitCriticalSection)",__LINE__,moduleName==NULL?"":moduleName); // tODO
    return NULL;
  }
  msg_param.loginfo.trace = traceLevel;
  msg_param.loginfo.where = (char *) pipeName;
  msg_param.module_name = (char *) moduleName;

  // Open Pipe
  if (mode==DOgMsgLogPipe && (msg_param.pipe=OgOpenWriterPipe(msg_param.herr,pipeName))==0) {
    OgErrLast(msg_param.herr,erreurMsg,NULL);
    fprintf(stderr, "%s", erreurMsg);
    return NULL;
  }
  else
  {
    unsigned char current_log_dir[DPcPathSize];
    const char *default_log_dir = OgGetWorkingDirLog();

    if (default_log_dir != NULL)
    {
      sprintf(current_log_dir, "%s/%s", default_log_dir, DOgDirLog);
    }
    else
    {
      sprintf(current_log_dir, "%s", DOgDirLog);
    }

    IF(OgCheckOrCreateDir(current_log_dir, 0, (char *) pipeName))
    {
      fprintf(stderr, "Can't create directory '%s' (%s)", current_log_dir, OgStrError(erreurMsg));
      return NULL;
    }
  }

  IFn(hmsg=OgMsgInit(&msg_param)) {
    fprintf(stderr,"Internal Error %d: OgInitLog failed (OgMsgInit)",__LINE__); // TODO
    return NULL;
  }
  return hmsg;
}







PUBLIC(int) OgMsg(void *handle, char *id, int mask, const char * format, ...)
{
struct og_ctrl_msg *ctrl_msg = (struct og_ctrl_msg *)handle;
char    text[DOgMlogMaxMessageSize+1024];
char    stemplate[DPcPathSize];
int     mlog_mask,gmask;
va_list arg_ptr;
int     res;

IFE(MsgMaskMix(ctrl_msg,mask,&gmask));
if(ctrl_msg->pipe!=0) { // pipe log
   va_start(arg_ptr,format);
   switch (gmask & 0xF) {
   case DOgMsgSeverityEmergency: res=logMsg(ctrl_msg,'X',format, arg_ptr); break;
   case DOgMsgSeverityAlert:     res=logMsg(ctrl_msg,'A',format, arg_ptr); break;
   case DOgMsgSeverityCritical:  res=logMsg(ctrl_msg,'C',format, arg_ptr); break;
   case DOgMsgSeverityError:     res=logMsg(ctrl_msg,'E',format, arg_ptr); break;
   case DOgMsgSeverityWarning:   res=logMsg(ctrl_msg,'W',format, arg_ptr); break;
   case DOgMsgSeverityNotice:    res=logMsg(ctrl_msg,'N',format, arg_ptr); break;
   case DOgMsgSeverityInfo:      res=logMsg(ctrl_msg,'I',format, arg_ptr); break;
   case DOgMsgSeverityDebug:     res=logMsg(ctrl_msg,'D',format, arg_ptr); break;
   default: res=logMsg(ctrl_msg,'D',format, arg_ptr);
   }
   va_end(arg_ptr);
   return res;
}

IFE(OgMsgTemplateBuild(ctrl_msg,id,stemplate,gmask));

if(gmask & (DOgMsgDestInLog+DOgMsgDestMBox+DOgMsgDestInErr+DOgMsgDestSysErr)) {
  va_start(arg_ptr,format);
  vsnprintf(text,DOgMlogMaxMessageSize/2,format,arg_ptr); text[DOgMlogMaxMessageSize/2]=0;
  mlog_mask = OgMsgMessageLogMask(gmask);
  OgEnterCriticalSection(ctrl_msg->hmutex);
  OgMessageLog(mlog_mask,ctrl_msg->loginfo->where,0,"%s%s",stemplate,text);
  OgLeaveCriticalSection(ctrl_msg->hmutex);
  va_end(arg_ptr);
  }

DONE;
}


PUBLIC(int) OgLogFlush(void *handle) {
  struct og_ctrl_msg *ctrl_msg = (struct og_ctrl_msg *)handle;

  assert(handle!=NULL);
  IF (OgFlushCriticalSection(ctrl_msg->hmutex)) {
    fprintf(stderr,"Internal Error %d: OgLogFLush (OgFlushCriticalSection)",__LINE__);
    DPcErr;
  }
  free(ctrl_msg->hmutex);
  IF(OgErrFlush(ctrl_msg->herr)) {
    fprintf(stderr,"Internal Error %d: OgLogFlush (OgErrFLush)",__LINE__);
    DPcErr;
  }
  IF (OgMsgFlush(ctrl_msg)) {
    fprintf(stderr,"Internal Error %d: OgLogFlush (OgMsgFlush)",__LINE__);
    DPcErr;
  }
  DONE;
}



PUBLIC(int) OgLogConsole(void *handle, const char *format,...)
{
  struct og_ctrl_msg *ctrl_msg = (struct og_ctrl_msg *)handle;
  char                text[DOgMlogMaxMessageSize+1024];
  va_list args;

  va_start(args,format);
  vsnprintf(text,DOgMlogMaxMessageSize+1023, format, args); text[DOgMlogMaxMessageSize+1023]='\0';
  va_end(args);
#if DPcSystem==DPcSystemWin32
  CharToOem(text,text); // convert char to OEM
#endif
  if (handle==NULL || ctrl_msg->consoleStream==NULL) {
    fprintf(stdout, "%s", text); fflush(stdout);
  }
  else {
    fprintf(ctrl_msg->consoleStream, "%s", text); fflush(ctrl_msg->consoleStream);
    // OgWritePipe(ctrl_msg->herr, ctrl_msg->consoleStream, text, strlen(text));
  }
  DONE;
}

PUBLIC(int) OgLogEmergency(void *handle, const char * format, ...)
{
  va_list args;
  int     res;
  char    text[DOgMlogMaxMessageSize+1024];

  va_start(args,format);
  vsnprintf(text,DOgMlogMaxMessageSize+1023,format,args); text[DOgMlogMaxMessageSize+1023]='\0';
  res=OgMsg(handle,"[EMERGENCY]",DOgMsgSeverityEmergency+DOgMsgDestInLog,text);
  va_end(args);
  return res;
}

PUBLIC(int) OgLogAlert(void *handle, const char * format, ...)
{
  va_list args;
  int     res;
  char    text[DOgMlogMaxMessageSize+1024];

  va_start(args,format);
  vsnprintf(text,DOgMlogMaxMessageSize+1023,format,args); text[DOgMlogMaxMessageSize+1023]='\0';
  res=OgMsg(handle,"[ALERT]",DOgMsgSeverityAlert+DOgMsgDestInLog,text);
  va_end(args);
  return res;
}

PUBLIC(int) OgLogCritical(void *handle, const char * format, ...)
{
  va_list args;
  int     res;
  char    text[DOgMlogMaxMessageSize+1024];

  va_start(args,format);
  vsnprintf(text,DOgMlogMaxMessageSize+1023,format,args); text[DOgMlogMaxMessageSize+1023]='\0';
  res=OgMsg(handle,"[CRITICAL]",DOgMsgSeverityCritical+DOgMsgDestInLog,text);
  va_end(args);
  return res;
}

PUBLIC(int) OgLogError(void *handle, const char * format, ...)
{
  va_list args;
  int     res;
  char    text[DOgMlogMaxMessageSize+1024];

  va_start(args,format);
  vsnprintf(text,DOgMlogMaxMessageSize+1023,format,args); text[DOgMlogMaxMessageSize+1023]='\0';
  res=OgMsg(handle,"[ERROR]",DOgMsgSeverityError+DOgMsgDestInLog,text);
  va_end(args);
  return res;
}

PUBLIC(int) OgLogWarning(void *handle, const char * format, ...)
{
  va_list args;
  int     res;
  char    text[DOgMlogMaxMessageSize+1024];

  va_start(args,format);
  vsnprintf(text,DOgMlogMaxMessageSize+1023,format,args); text[DOgMlogMaxMessageSize+1023]='\0';
  res=OgMsg(handle,"[WARNING]",DOgMsgSeverityNotice+DOgMsgDestInLog,text);
  va_end(args);
  return res;
}

PUBLIC(int) OgLogNotice(void *handle, const char * format, ...)
{
  va_list args;
  char    text[DOgMlogMaxMessageSize+1024];
  int     res;

  va_start(args,format);
  vsnprintf(text,DOgMlogMaxMessageSize+1023,format,args); text[DOgMlogMaxMessageSize+1023]='\0';
  res=OgMsg(handle,"[NOTICE]",DOgMsgSeverityNotice+DOgMsgDestInLog,text);
  va_end(args);
  return res;
}

PUBLIC(int) OgLogInfo(void *handle, const char * format, ...)
{
  va_list args;
  int     res;
  char    text[DOgMlogMaxMessageSize+1024];

  va_start(args,format);
  vsnprintf(text,DOgMlogMaxMessageSize+1023,format,args); text[DOgMlogMaxMessageSize+1023]='\0';
  res=OgMsg(handle,"[INFO]",DOgMsgSeverityInfo+DOgMsgDestInLog,text);
  va_end(args);
  return res;
}

PUBLIC(int) OgLogDebug(void *handle, const char * format, ...)
{
  va_list args;
  int     res;
  char    text[DOgMlogMaxMessageSize+1024];

  va_start(args,format);
  vsnprintf(text,DOgMlogMaxMessageSize+1023,format,args); text[DOgMlogMaxMessageSize+1023]='\0';
  res=OgMsg(handle,"[DEBUG]",DOgMsgSeverityDebug+DOgMsgDestInLog,text);
  va_end(args);
  return res;
}

/** herr Accessor
 *
 **/
PUBLIC(void *) OgLogGetErr(void *handle)
{
  struct og_ctrl_msg *ctrl_msg = (struct og_ctrl_msg *)handle;

  assert(ctrl_msg!=NULL);
  return ctrl_msg->herr;
}

/** hmutex Accessor
 *
 **/
PUBLIC(void *) OgLogGetMutex(void *handle)
{
  struct og_ctrl_msg *ctrl_msg = (struct og_ctrl_msg *)handle;

  assert(ctrl_msg!=NULL);
  return ctrl_msg->hmutex;
}

PUBLIC(int) OgLogGetTrace(void *handle)
{
  struct og_ctrl_msg *ctrl_msg = (struct og_ctrl_msg *)handle;
  assert(handle!=NULL);
  return ctrl_msg->cloginfo.trace;
}

PUBLIC(char *) OgLogGetWhere(void *handle)
{
  struct og_ctrl_msg *ctrl_msg = (struct og_ctrl_msg *)handle;
  assert(handle!=NULL);
  return ctrl_msg->cloginfo.where;
}

PUBLIC(char *) OgLogGetModuleName(void *handle)
{
  struct og_ctrl_msg *ctrl_msg = (struct og_ctrl_msg *)handle;
  assert(handle!=NULL);
  return ctrl_msg->module_name;
}

PUBLIC(char *) OgLogLastErr(void *handle) {
  static char msg[DOgErrorSize];

  struct og_ctrl_msg *ctrl_msg = (struct og_ctrl_msg *)handle;
  assert(handle!=NULL);
  OgErrLast(ctrl_msg->herr,msg,NULL); // get last OgErr Error
  return msg;
}

DEFPUBLIC(int) OgLogSetConsoleStream(void *handle, FILE *stream) {
  struct og_ctrl_msg *ctrl_msg = (struct og_ctrl_msg *)handle;
  assert(handle!=NULL);
  ctrl_msg->consoleStream=stream;
  DONE;
}

DEFPUBLIC(FILE *) OgLogGetConsoleStream(void *handle) {
  struct og_ctrl_msg *ctrl_msg = (struct og_ctrl_msg *)handle;
  assert(handle!=NULL);
  return ctrl_msg->consoleStream;
}



static int logMsg(void *handle, char criticity, const char *format, va_list args) {
  struct og_ctrl_msg *ctrl_msg = (struct og_ctrl_msg *)handle;
  char                text[DOgMlogMaxMessageSize+1024];
  char                date[256];

#if DPcSystem==DPcSystemWin32
  snprintf(text,100,"%s|%c|%*s|%13s|%5d|%5d| ",OgGetTimeISO8601(date,255),criticity,DOgInstanceNameSize,ctrl_msg->instance_name,ctrl_msg->prog_name,
     GetCurrentProcessId(),GetCurrentThreadId());
#else
  #ifdef SYS_gettid
  snprintf(text,100,"%s|%c|%*s|%13s|%5d|%5lu| ",OgGetTimeISO8601(date,255),criticity,DOgInstanceNameSize,ctrl_msg->instance_name,ctrl_msg->prog_name,getpid(),syscall(SYS_gettid));  // return LWP number
  #else
  snprintf(text,100,"%s|%c|%*s|%13s|%5d|%5lu| ",OgGetTimeISO8601(date,255),criticity,DOgInstanceNameSize,ctrl_msg->instance_name,ctrl_msg->prog_name,getpid(),0ul);
  #endif
#endif
  vsnprintf(text+73,DOgMlogMaxMessageSize+1023-73, format, args); text[DOgMlogMaxMessageSize+1023-73]='\0';
  return OgWritePipe(ctrl_msg->herr, ctrl_msg->pipe, text, strlen(text)+1); // send '\0' char in pipe to cut message if necessary
}
