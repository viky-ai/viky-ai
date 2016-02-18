/*
 *	Header for library ogm_ltrap.dll
 *  Copyright (c) 2010 Pertimm by P.Constant
 *  Dev : November 2010
 *  Version 1.0
*/
#ifndef _LOGLTRAPALIVE_
#include <loggen.h>
#include <logthr.h>
#include <logmsg.h>
#include <loguni.h>
#include <logaut.h>


#define DOgLtrapBanner  "ogm_ltrap V1.00, Copyright (c) 2010 Pertimm, Inc."
#define DOgLtrapVersion 100

/** Trace levels **/
#define DOgLtrapTraceMinimal          0x1
#define DOgLtrapTraceMemory           0x2
#define DOgLtrapTraceAdd              0x4
#define DOgLtrapTraceBuild            0x8
#define DOgLtrapTraceSort             0x10
#define DOgLtrapTraceSend             0x20


#define DOgLtrapSynchronizationLength     128
#define DOgLtrapMaxCandidates             0
#define DOgLtrapMaxSolutions              1
#define DOgLtrapHeaderLength              4
#define DOgLtrapMinSwapWordLength         2


#define DOgLtrapModuleConfiguration   "conf/ltrap_conf.xml"

#define DOgLtrapLexiconTypeBase   1
#define DOgLtrapLexiconTypeSwap   2
#define DOgLtrapLexiconTypePhon   3


struct og_ltrap_param {
  void *herr,*hmsg; ogmutex_t *hmutex;
  struct og_loginfo loginfo;
  char WorkingDirectory[DPcPathSize];
  char configuration_file[DPcPathSize];
  int max_word_frequency; 
  void *ha_base,*ha_swap,*ha_phon;
  int max_small_word_length;
  void *ha_small_words;
  void *ha_false;
  };

struct og_ltrap_token {
  int start,length,length_string; unsigned char *string;
  double global_score; int global_frequency;
  int solution_number,solution_complete;
  int starting,ending,position;
  int modified,frequency;
  };

struct og_ltrap_input {
  int lexicon_type, language, iuni; unsigned char *uni;
  int (*send_token)(void *context, struct og_ltrap_token *token);
  int synchronization_length,max_candidates,max_solutions,max_header_length;
  int min_swap_word_length;
  double score_factor;
  double cut_cost;
  void *context;
  };

DEFPUBLIC(void *) OgLtrapInit(pr(struct og_ltrap_param *));
DEFPUBLIC(int) OgLtrapFlush(pr(void *));
DEFPUBLIC(int) OgLtrap(void *handle, struct og_ltrap_input *input);


#define _LOGLTRAPALIVE_
#endif


