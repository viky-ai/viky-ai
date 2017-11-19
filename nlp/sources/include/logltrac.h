/*
 *  Header for library ogm_ltrac.dll
 *  Copyright (c) 2009-2017 Pertimm by P.Constant
 *  Dev : November 2009, November 2017
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

struct og_ltrac_dictionaries
{
  void *ha_base;
  void *ha_swap;
  void *ha_phon;
};

struct og_ltrac_word_input
{
  int value_length;
  unsigned char *value;
  int language_code;
  int frequency;
};

struct og_ltrac_param
{
  void *herr, *hmsg;
  ogmutex_t *hmutex;
  struct og_loginfo loginfo;
  char WorkingDirectory[DPcPathSize];
  char dictionaries_directory[DPcPathSize];
  int dictionaries_in_data_directory;
  char data_directory[DPcPathSize];
};

DEFPUBLIC(void *) OgLtracInit(pr(struct og_ltrac_param *));
DEFPUBLIC(int) OgLtracFlush(pr(void *));

DEFPUBLIC(int) OgLtracDicInit(void *handle, struct og_ltrac_dictionaries *dictionaries);
DEFPUBLIC(int) OgLtracDicWrite(void *handle);

DEFPUBLIC(int) OgLtracAddWord(void *handle, struct og_ltrac_word_input *word_input);

DEFPUBLIC(int) OgLtracDicBaseLog(void *handle, void *ha_base);
DEFPUBLIC(int) OgLtracDicSwapLog(void *handle, void *ha_swap);
DEFPUBLIC(int) OgLtracDicPhonLog(void *handle, void *ha_phon);

#define _LOGLTRACALIVE_
#endif

