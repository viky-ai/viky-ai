/*
 *  Calculating memory used for ogm_pho
 *  Copyright (c) 2009 Pertimm by Patrick Constant
 *  Dev : March 2009
 *  Version 1.0
*/
#include "ogm_pho.h"


#define DOgShowMem(x,y,z) { if (must_log) { Og64FormatThousand(x,v,1); OgMsg(ctrl_pho->hmsg,"",DOgMsgDestInLog,"%s%s:%s %16s",header,y,z,v); } }


struct og_pho_memory {
  ogint64_t total,total_module;
  ogint64_t ctrl_pho,ha_rules,Rule,Matching;
  ogint64_t Ba,Char_class,BaClass;
  };


/*
 *  Logs all the memory module and calculates all the memory.
*/

PUBLIC(int) OgPhoMem(hpho,must_log,module_level,pmem)
void *hpho; int must_log,module_level;
ogint64_t *pmem;
{
struct og_ctrl_pho *ctrl_pho = (struct og_ctrl_pho *)hpho;
char header[DPcPathSize],v[128];
struct og_pho_memory cm,*m=&cm;
int i;

if (pmem) *pmem=0;
IFn(hpho) DONE;

if (must_log) {
  for (i=0; i<module_level; i++) header[i]=' '; header[i]=0;
  }
memset(m,0,sizeof(struct og_pho_memory));

m->ctrl_pho=sizeof(struct og_ctrl_pho);

IFE(OgAutMem(ctrl_pho->ha_rules,0,module_level+2,&m->ha_rules));

m->Rule=ctrl_pho->RuleNumber*sizeof(struct rule);
m->Matching=ctrl_pho->MatchingNumber*sizeof(struct matching);
m->Ba=ctrl_pho->BaSize*sizeof(unsigned char);
m->Char_class=ctrl_pho->Char_classNumber*sizeof(struct char_class);
m->BaClass=ctrl_pho->BaClassSize*sizeof(unsigned char);

m->total_module = m->ctrl_pho
                + m->ha_rules
                + m->Rule
                + m->Matching
                + m->Ba
                + m->Char_class
                + m->BaClass;

m->total += m->total_module;

DOgShowMem(m->total_module    ,"total_module pho"," ")
DOgShowMem(m->ctrl_pho        ,"ctrl_pho","         ")
DOgShowMem(m->ha_rules        ,"ha_rules","         ")
DOgShowMem(m->Rule            ,"Rule","             ")
DOgShowMem(m->Matching        ,"Matching","         ")
DOgShowMem(m->Ba              ,"Ba","               ")
DOgShowMem(m->Char_class      ,"Char_class","       ")
DOgShowMem(m->BaClass         ,"BaClass","          ")

if (pmem) *pmem=m->total;

DONE;
}




