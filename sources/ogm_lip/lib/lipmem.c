/*
 *  Calculating memory used for ogm_lip
 *  Copyright (c) 2009 Pertimm by Patrick Constant
 *  Dev : March 2009
 *  Version 1.0
*/
#include "ogm_lip.h"


#define DOgShowMem(x,y,z) { if (must_log) { Og64FormatThousand(x,v,1); OgMsg(ctrl_lip->hmsg,"",DOgMsgDestInLog,"%s%s:%s %16s",header,y,z,v); } }


struct og_lip_memory {
  ogint64_t total,total_module;
  ogint64_t ctrl_lip,Pawo;
  };


/*
 *  Logs all the memory module and calculates all the memory.
*/

PUBLIC(int) OgLipMem(void *hlip, int must_log, int module_level, ogint64_t *pmem)
{
struct og_ctrl_lip *ctrl_lip = (struct og_ctrl_lip *)hlip;
char header[DPcPathSize],v[128];
struct og_lip_memory cm,*m=&cm;
int i;

IFn(hlip) DONE;

if (pmem) *pmem=0;
if (must_log) {
  for (i=0; i<module_level; i++) header[i]=' '; header[i]=0;
  }
memset(m,0,sizeof(struct og_lip_memory));

m->ctrl_lip=sizeof(struct og_ctrl_lip);
m->Pawo=ctrl_lip->PawoNumber*sizeof(struct pawo);

m->total_module = m->ctrl_lip
                + m->Pawo;

m->total += m->total_module;

DOgShowMem(m->total_module    ,"total_module lip"," ")
DOgShowMem(m->ctrl_lip        ,"ctrl_lip","         ")
DOgShowMem(m->Pawo            ,"Pawo","             ")

if (pmem) *pmem=m->total;

DONE;
}




