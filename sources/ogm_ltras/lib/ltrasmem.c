/*
 *  Calculating memory used for ogm_ltras
 *  Copyright (c) 2011 Pertimm by Patrick Constant
 *  Dev: March 2011
 *  Version 1.0
*/
#include "ogm_ltras.h"


#define DOgShowMem(x,y,z) { if (must_log) { Og64FormatThousand(x,v,1); OgMsg(ctrl_ltras->hmsg,"",DOgMsgDestInLog,"%s%s:%s %16s",header,y,z,v); } }


struct og_ltras_memory {
  ogint64_t total,total_module;
  ogint64_t ctrl_ltras;
  ogint64_t hlip;
  ogint64_t Node,Ba;
  ogint64_t hrqp,ha_false,hpho,hldi,ha_suggest,ha_oper,ha_param,hstm;
  };


/*
 *  Logs all the memory module and calculates all the memory.
*/

PUBLIC(int) OgLtrasMem(void *hltras, int must_log, int module_level, ogint64_t *pmem)
{
struct og_ctrl_ltras *ctrl_ltras = (struct og_ctrl_ltras *)hltras;
char header[DPcPathSize],v[128];
struct og_ltras_memory cm,*m=&cm;
int i;

if (pmem) *pmem=0;
IFn(hltras) DONE;

if (must_log) {
  for (i=0; i<module_level; i++) header[i]=' '; header[i]=0;
  }
memset(m,0,sizeof(struct og_ltras_memory));

m->ctrl_ltras=sizeof(struct og_ctrl_ltras);

IFE(OgLipMem(ctrl_ltras->hlip,0,module_level+2,&m->hlip));
m->Node=ctrl_ltras->NodeNumber*sizeof(struct node);
m->Ba=ctrl_ltras->BaSize*sizeof(unsigned char);
IFE(OgRqpMem(ctrl_ltras->hrqp,0,module_level+2,&m->hrqp));
IFE(OgAutMem(ctrl_ltras->ha_false,0,module_level+2,&m->ha_false));
IFE(OgPhoMem(ctrl_ltras->hpho,0,module_level+2,&m->hpho));
//IFE(OgLdiMem(ctrl_ltras->hldi,0,module_level+2,&m->hldi));
IFE(OgAutMem(ctrl_ltras->ha_suggest,0,module_level+2,&m->ha_suggest));
IFE(OgAutMem(ctrl_ltras->ha_oper,0,module_level+2,&m->ha_oper));
IFE(OgAutMem(ctrl_ltras->ha_param,0,module_level+2,&m->ha_param));
//IFE(OgStmMem(ctrl_ltras->hstm,0,module_level+2,&m->hstm));

m->total_module = m->ctrl_ltras
                + m->hlip
                + m->Node
                + m->Ba
                + m->hrqp
                + m->ha_false
                + m->hpho
                + m->hldi
                + m->ha_suggest
                + m->ha_oper
                + m->ha_param
                + m->hstm
                ;

m->total += m->total_module;

DOgShowMem(m->total_module    ,"total_module ltras"," ")
DOgShowMem(m->ctrl_ltras      ,"ctrl_ltras","         ")
DOgShowMem(m->hlip            ,"hlip","               ")
DOgShowMem(m->Node            ,"Node","               ")
DOgShowMem(m->Ba              ,"Ba","                 ")
DOgShowMem(m->hrqp            ,"hrqp","               ")
DOgShowMem(m->ha_false        ,"ha_false","           ")
DOgShowMem(m->hpho            ,"hpho","               ")
DOgShowMem(m->hldi            ,"hldi","               ")
DOgShowMem(m->ha_suggest      ,"ha_suggest","         ")
DOgShowMem(m->ha_param        ,"ha_param","           ")
DOgShowMem(m->hstm            ,"hstm","               ")

if (pmem) *pmem=m->total;

DONE;
}




