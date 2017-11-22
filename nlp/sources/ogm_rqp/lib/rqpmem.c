/*
 *  Calculating memory used for ogm_rqp
 *  Copyright (c) 2011 Pertimm by Patrick Constant
 *  Dev: March 2011
 *  Version 1.0
*/
#include "ogm_rqp.h"


#define DOgShowMem(x,y,z) { if (must_log) { Og64FormatThousand(x,v,1); OgMsg(ctrl_rqp->hmsg,"",DOgMsgDestInLog,"%s%s:%s %16s",header,y,z,v); } }


struct og_rqp_memory {
  ogint64_t total,total_module;
  ogint64_t ctrl_rqp;
  ogint64_t Node,Ba,Bx;
  ogint64_t ha_subtree_id;
  };


/*
 *  Logs all the memory module and calculates all the memory.
*/

PUBLIC(int) OgRqpMem(void *hrqp, int must_log, int module_level, ogint64_t *pmem)
{
struct og_ctrl_rqp *ctrl_rqp = (struct og_ctrl_rqp *)hrqp;
char header[DPcPathSize],v[128];
struct og_rqp_memory cm,*m=&cm;
int i;

IFn(hrqp) DONE;

if (pmem) *pmem=0;
if (must_log) {
  for (i=0; i<module_level; i++) header[i]=' '; header[i]=0;
  }
memset(m,0,sizeof(struct og_rqp_memory));

m->ctrl_rqp=sizeof(struct og_ctrl_rqp);
m->Node=ctrl_rqp->NodeNumber*sizeof(struct node);
m->Ba=ctrl_rqp->BaSize*sizeof(unsigned char);
m->Bx=ctrl_rqp->BxSize*sizeof(unsigned char);
IFE(OgAutMem(ctrl_rqp->ha_subtree_id,0,module_level+2,&m->ha_subtree_id));

m->total_module = m->ctrl_rqp
                + m->Node
                + m->Ba
                + m->Bx
                + m->ha_subtree_id
                ;

m->total += m->total_module;

DOgShowMem(m->total_module    ,"total_module rqp"," ")
DOgShowMem(m->ctrl_rqp        ,"ctrl_rqp","         ")
DOgShowMem(m->Node            ,"Node","             ")
DOgShowMem(m->Ba              ,"Ba","               ")
DOgShowMem(m->Bx              ,"Bx","               ")
DOgShowMem(m->ha_subtree_id   ,"ha_subtree_id","    ")

if (pmem) *pmem=m->total;

DONE;
}




