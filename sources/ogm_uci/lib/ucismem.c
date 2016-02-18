/*
 *  Calculating memory used for ogm_ucis
 *  Copyright (c) 2009 Pertimm by Patrick Constant
 *  Dev : March 2009
 *  Version 1.0
*/
#include "ogm_uci.h"


#define DOgShowMem(x,y,z) { if (must_log) { Og64FormatThousand(x,v,1); OgMsg(ctrl_ucis->hmsg,"",DOgMlogInLog,"%s%s:%s %16s",header,y,z,v); } }


struct og_ucis_memory {
  ogint64_t total,total_module;
  ogint64_t ctrl_ucis,Ba;
  ogint64_t huci;
  };


/*
 *  Logs all the memory module and calculates all the memory.
*/

PUBLIC(int) OgUcisMem(void *hucis, int must_log, int module_level,ogint64_t *pmem)
{
struct og_ctrl_ucis *ctrl_ucis = (struct og_ctrl_ucis *)hucis;
char header[DPcPathSize],v[128];
struct og_ucis_memory cm,*m=&cm;
int i;

if (pmem) *pmem=0;
IFn(hucis) DONE;

if (must_log) {
  for (i=0; i<module_level; i++) header[i]=' '; header[i]=0;
  }
memset(m,0,sizeof(struct og_ucis_memory));

m->ctrl_ucis=sizeof(struct og_ctrl_ucis);
m->Ba=ctrl_ucis->BaSize*sizeof(unsigned char);

IFE(OgUciMem(ctrl_ucis->huci,must_log,module_level+2,&m->huci));

m->total_module = m->ctrl_ucis
                + m->Ba
                + m->huci
                ;

m->total += m->total_module;

DOgShowMem(m->total_module    ,"total_module ucis"," ")
DOgShowMem(m->ctrl_ucis       ,"ctrl_ucis","         ")
DOgShowMem(m->Ba              ,"Ba","                ")
DOgShowMem(m->huci            ,"huci","              ")

if (pmem) *pmem=m->total;

DONE;
}




