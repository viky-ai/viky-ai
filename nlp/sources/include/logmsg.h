/*
 *  Header for library ogm_msg.dll
 *  Copyright (c) 2007 Pertimm by Patrick Constant and Guillaume Logerot
 *  Dev : March,April,May 2007
 *  Version 1.2
*/
#ifndef _LOGMSGALIVE_
#include <loggen.h>
#include <logthr.h>


#define DOgMsgBanner  "ogm_msg V1.08, Copyright (c) 2007 Pertimm, Inc."
#define DOgMsgVersion 108


/** Trace levels **/
#define DOgMsgTraceMinimal            0x1
#define DOgMsgTraceMemory             0x2

/** Mask definition for severity **/
#define DOgMsgSeverityEmergency       0x1
#define DOgMsgSeverityAlert           0x2
#define DOgMsgSeverityCritical        0x3
#define DOgMsgSeverityError           0x4
#define DOgMsgSeverityWarning         0x5
#define DOgMsgSeverityNotice          0x6
#define DOgMsgSeverityInfo            0x7
#define DOgMsgSeverityDebug           0x8

/** Mask definition for destination **/
#define DOgMsgDestInLog               0x10
#define DOgMsgDestMBox                0x20
#define DOgMsgDestInErr               0x40
#define DOgMsgDestSysErr              0x80

/** Mask definition for parameters **/
#define DOgMsgParamLogDate            0x10000
#define DOgMsgParamDateIn             0x20000
#define DOgMsgParamNoCr               0x40000
#define DOgMsgParamUnicode            0x80000

/** Tuning types **/
#define DOgMsgTuneMask            0x1
#define DOgMsgTuneShowTimestamp   0x2
#define DOgMsgTuneShowId          0x3

/** Values for DOgMsgTuneShowId **/
#define DOgMsgTuneShowIdModule    0x1
#define DOgMsgTuneShowIdMessage   0x2

/** Logging mode **/
#define DOgMsgLogFile           0
#define DOgMsgLogPipe           1

struct og_msg_param {
  void              *herr;
  ogmutex_t         *hmutex;
  struct og_loginfo  loginfo;
  // msg options
  char              *module_name;
  int                show_id;
  int                show_timestamp;
  NATIVE_FILE        pipe;
  // program calling encapsulate
  struct og_loginfo *prog_loginfo;
  };


DEFPUBLIC(void *)OgMsgInit(struct og_msg_param *);
DEFPUBLIC(int) OgMsgFlush(void *);

DEFPUBLIC(int) OgMsgTune(void *, int, void *);
DEFPUBLIC(int) OgMsgTuneInherit(void *, void *);

DEFPUBLIC(int) OgMsg(void *, char *, int, const char *, ...);
DEFPUBLIC(int) OgMsgErr(void *, char *, int, int, char *, int, int);

DEFPUBLIC(int) OgMsgMaskInterpret(void *, char *, int *);
DEFPUBLIC(int) OgMsgMaskString(void *, int, char *);

DEFPUBLIC(void *) OgLogInit(const char *moduleName, const char *pipeName, int traceLevel, int mode);
DEFPUBLIC(int)    OgLogFlush(void *handle);
DEFPUBLIC(int)    OgLogConsole(void *handle, const char * format, ...);

DEFPUBLIC(int)    OgLogEmergency(void *handle, const char * format, ...);
DEFPUBLIC(int)    OgLogAlert(void *handle, const char * format, ...);
DEFPUBLIC(int)    OgLogCritical(void *handle, const char * format, ...);
DEFPUBLIC(int)    OgLogError(void *handle, const char * format, ...);
DEFPUBLIC(int)    OgLogWarning(void *handle, const char * format, ...);
DEFPUBLIC(int)    OgLogNotice(void *handle, const char * format, ...);
DEFPUBLIC(int)    OgLogInfo(void *handle, const char * format, ...);
DEFPUBLIC(int)    OgLogDebug(void *handle, const char * format, ...);

DEFPUBLIC(void *) OgLogGetErr(void *handle);
DEFPUBLIC(void *) OgLogGetMutex(void *handle);
DEFPUBLIC(int)    OgLogGetTrace(void *handle);
DEFPUBLIC(char *) OgLogGetWhere(void *handle);
DEFPUBLIC(char *) OgLogGetModuleName(void *handle);
DEFPUBLIC(char *) OgLogLastErr(void *handle);

DEFPUBLIC(int)    OgLogSetConsoleStream(void *handle, FILE *stream);
DEFPUBLIC(FILE *) OgLogGetConsoleStream(void *handle);

// Log backtrace in critical level to track potential bug
#include <execinfo.h>
#ifdef _EXECINFO_H
#define OG_LOG_BACKTRACE(hmsg, str_error_message)  { void *DPT_trace[255]; int DPT_trace_size = backtrace(DPT_trace, sizeof(DPT_trace)); char **DPT_messages = backtrace_symbols(DPT_trace, DPT_trace_size); OgLogCritical(hmsg, "%s, backtrace :", str_error_message); for (int DPT_i = 0; DPT_i < DPT_trace_size; DPT_i++) { OgLogCritical(hmsg, "\t%s", DPT_messages[DPT_i]); } OgLogCritical(hmsg, "\t%s\n", "..."); free(DPT_messages); }
#else
#define OG_LOG_BACKTRACE(hmsg, str_error_message)  { OgLogCritical(hmsg, "%s, no backtrace available (no execinfo.h)", str_error_message); }
#endif

#define _LOGMSGALIVE_
#endif


