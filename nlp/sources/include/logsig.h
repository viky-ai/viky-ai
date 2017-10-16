/*
 *	Header for library ogm_sig.dll
 *	Copyright (c) 2007 Pertimm by Patrick Constant
 *	Dev : August 2007
 *	Version 1.0
*/
#ifndef _LOGSIGALIVE_
#include <loggen.h>
#include <logthr.h>


#define DOgSigBanner  "ogm_sig V1.04, Copyright (c) 2007 Pertimm, Inc."
#define DOgSigVersion 104


/** Trace levels **/
#define DOgSigTraceMinimal          0x1
#define DOgSigTraceMemory           0x2


#define DOgSigFlagNil   0
#define DOgSigFlagIgn   1
#define DOgSigFlagDfl   2


struct og_sig_param {
  void *herr; ogmutex_t *hmutex;
  struct og_loginfo loginfo;
  }; 

DEFPUBLIC(void *) OgSigInit(pr(struct og_sig_param *));
DEFPUBLIC(int) OgSignal(pr_(void *) pr_(int) pr_(void (*func)(pr_(void *) pr(int))) pr_(void *) pr(int));
DEFPUBLIC(int) OgSigFlush(pr(void *));

#define _LOGSIGALIVE_
#endif


