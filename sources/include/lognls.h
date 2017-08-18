/*
 *  Header for library libognls.dll
 *  Copyright (c) 2017 Pertimm by P.Constant
 *  Dev : August 2017
 *  Version 1.0
 * Natural Language Server
 */
#ifndef _LOGNLSALIVE_
#include <lpcgentype.h>
#include <stdarg.h>
#include <loggen.h>
#include <logmsg.h>

#define DOgNlsBanner  "libognls V1.00, Copyright (c) 2017 Pertimm"
#define DOgNlsVersion 100

/** Trace levels **/
#define DOgNlsTraceMinimal                0x1
#define DOgNlsTraceMemory                 0x2
#define DOgNlsTraceAllRequest             0x4
#define DOgNlsTraceAnswer                 0x8
#define DOgNlsTraceSocket                 0x10
#define DOgNlsTraceSocketSize             0x20
#define DOgNlsTraceLT                     0x40
#define DOgNlsTraceLTSearch               0x80
#define DOgNlsTraceDefault               (DOgNlsTraceMinimal+DOgNlsTraceMemory)

/** Default value for configuration file information **/
#define DOgNlsMaxListeningThreads          4
#define DOgNlsPermanentThreads             1
#define DOgNlsAnswerTimeout                0
#define DOgNlsBacklogMaxPendingRequests  200
#define DOgNlsBacklogTimeout               0
#define DOgNlsSocketReadTimeout            0
#define DOgNlsRequestProcessingTimeout     0
#define DOgNlsIndexingTimeout              30000
#define DOgNlsStopTimeout                  30000

struct og_nls_param
{
  void *herr, *hmsg;
  ogmutex_t *hmutex;
  struct og_loginfo loginfo;
  char WorkingDirectory[DPcPathSize];
  char configuration_file[DPcPathSize];
  int argc;
  char **argv;
};

DEFPUBLIC(char *) OgNlsBanner(void);
DEFPUBLIC(void *) OgNlsInit(struct og_nls_param *param, void **phandle);
DEFPUBLIC(int) OgNlsFlush(void *hnls);
DEFPUBLIC(int) OgNlsRun(void *handle);
DEFPUBLIC(int) OgNlsOnSignal(void *handle);

#define _LOGNLSALIVE_
#endif

