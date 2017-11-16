/*
 *  Header for library ogm_pho.dll
 *  Copyright (c) 2006-2007 Pertimm by P.Constant,G.Logerot and L.Rigouste
 *  Dev : November 2006, June,July 2007, May,June 2008
 *  Version 1.3
*/
#ifndef _LOGPHOALIVE_
#include <loggen.h>
#include <logmsg.h>
#include <logthr.h>
#include <loguni.h>


#define DOgPhoBanner  "ogm_pho V1.24, Copyright (c) 2006-2008 Pertimm, Inc."
#define DOgPhoVersion 124

/** Trace levels **/
#define DOgPhoTraceMinimal              0x1
#define DOgPhoTraceMemory               0x2
#define DOgPhoTraceInit                 0x4
#define DOgPhoTraceMain                 0x8
#define DOgPhoTracePhoReadConf          0x10
#define DOgPhoTracePhoReadConfDetail    0x20
#define DOgPhoTraceRules                0x40
#define DOgPhoTraceRulesMatch           0x80

#define DOgPhoTracePhonet               0x1000

#define DOgPhoConfigurationDirectory     "conf/phonetic"
#define DOgPhoConfigurationFile          "phonet_conf.xml"

struct og_pho_param {
  void *herr,*hmsg; ogmutex_t *hmutex;
  struct og_loginfo loginfo;
  char conf_directory[DPcPathSize];
  char conf_filename[DPcPathSize];
  void *hpho_to_inherit;
  };

struct og_pho_input
{
  int iB;
  unsigned char *B;
  int lang;
};

struct og_pho_output {
  int iB;
  unsigned char *B;
  };

DEFPUBLIC(void *) OgPhoInit(struct og_pho_param *param);
DEFPUBLIC(int) OgPhoFlush(void *handle);

DEFPUBLIC(int) OgPhonet(pr_(void *) pr_(struct og_pho_input *) pr(struct og_pho_output *));
DEFPUBLIC(int) OgPhoMem(void *,int,int,ogint64_t *);

#define _LOGPHOALIVE_
#endif


