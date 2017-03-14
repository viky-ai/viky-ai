/*
 *  Header for library ogm_ltrac.dll
 *  Copyright (c) 2009 Pertimm by P.Constant
 *  Dev : November 2009
 *  Version 1.0
*/
#ifndef _LOGLTRACALIVE_
#include <loggen.h>
#include <logthr.h>
#include <loguni.h>
#include <logmsg.h>


#define DOgLtracBanner  "ogm_ltrac V2.00, Copyright (c) 2017 Pertimm, Inc."
#define DOgLtracVersion 200

/** Trace levels **/
#define DOgLtracTraceMinimal        0x1
#define DOgLtracTraceMemory         0x2
#define DOgLtracTraceAdd            0x4


#define DOgLtracDictionaryTypeBase         0x1
#define DOgLtracDictionaryTypeSwap         0x2
#define DOgLtracDictionaryTypePhon         0x4


struct og_ltrac_input {
  int input_type;
  int dictionaries_to_export;
  int dictionaries_minimization;
  int min_frequency;
  int min_frequency_swap;
  };

struct og_ltrac_param {
  void *herr,*hmsg; ogmutex_t *hmutex;
  struct og_loginfo loginfo;
  char WorkingDirectory[DPcPathSize];
  char dictionaries_directory[DPcPathSize];
  int dictionaries_in_data_directory;
  char data_directory[DPcPathSize];
  };


DEFPUBLIC(void *) OgLtracInit(pr(struct og_ltrac_param *));
DEFPUBLIC(int) OgLtrac(void *handle,struct og_ltrac_input *input);
DEFPUBLIC(int) OgLtracFlush(pr(void *));

DEFPUBLIC(int) OgLtracDicBaseLog(void *handle);
DEFPUBLIC(int) OgLtracDicSwapLog(void *handle);
DEFPUBLIC(int) OgLtracDicPhonLog(void *handle);
DEFPUBLIC(int) OgLtracDicExpressionsLog(void *handle);


#define _LOGLTRACALIVE_
#endif


