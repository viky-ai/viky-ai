/*
 *  Calculating memory used for ogm_uci
 *  Copyright (c) 2009 Pertimm by Patrick Constant
 *  Dev : March 2009
 *  Version 1.0
*/
#include "ogm_uci.h"


#define DOgShowMem(x,y,z) { if (must_log) { Og64FormatThousand(x,v,1); OgMsg(ctrl_uci->hmsg,"",DOgMsgDestInLog,"%s%s:%s %16s",header,y,z,v); } }


struct og_uci_memory {
  ogint64_t total,total_module;
  ogint64_t ctrl_uci,Ba;
  ogint64_t hhttp;
  };


/*
 *  Logs all the memory module and calculates all the memory.
*/

PUBLIC(int) OgUciMem(void *huci, int must_log, int module_level,ogint64_t *pmem)
{
struct og_ctrl_uci *ctrl_uci = (struct og_ctrl_uci *)huci;
char header[DPcPathSize],v[128];
struct og_uci_memory cm,*m=&cm;
int i;

if (pmem) *pmem=0;
IFn(huci) DONE;

if (must_log) {
  for (i=0; i<module_level; i++) header[i]=' '; header[i]=0;
  }
memset(m,0,sizeof(struct og_uci_memory));

m->ctrl_uci=sizeof(struct og_ctrl_uci);
m->Ba=ctrl_uci->BaSize*sizeof(unsigned char);
IFE(OgHttpMem(ctrl_uci->hhttp,0,module_level+2,&m->hhttp));

m->total_module = m->ctrl_uci
                + m->Ba
                + m->hhttp
                ;

m->total += m->total_module;

DOgShowMem(m->total_module    ,"total_module uci"," ")
DOgShowMem(m->ctrl_uci        ,"ctrl_uci","         ")
DOgShowMem(m->Ba              ,"Ba","               ")
DOgShowMem(m->hhttp           ,"hhttp","            ")

if (pmem) *pmem=m->total;

DONE;
}




