/*
 *  Calculating memory used for ogm_aut
 *  Copyright (c) 2006-2009 Pertimm by Patrick Constant
 *  Dev : March 2006, March 2009
 *  Version 1.1
*/
#include "ogm_aut.h"


#define DOgShowMem(x,y,z) { if (must_log) { Og64FormatThousand(x,v,1); OgMessageLog(DOgMlogInLog,ctrl_aut->loginfo->where,0,"%s%s:%s %16s",header,y,z,v); } }


struct og_aut_memory {
  ogint64_t total,total_module;
  ogint64_t ctrl_aut,State,FreeState;
  ogint64_t Fstate,Gstate;
  ogint64_t Lstmin;
  };


/*
 *  Logs all the memory module and calculates all the memory for ogm_jsrv
 *  ml is module level and ll is local level.
*/

PUBLIC(int) OgAutMem(haut,must_log,module_level,pmem)
void *haut; int must_log,module_level;
ogint64_t *pmem;
{
struct og_ctrl_aut *ctrl_aut = (struct og_ctrl_aut *)haut;
char header[DPcPathSize],v[128];
struct og_aut_memory cm,*m=&cm;
int i;

if (pmem) *pmem=0;
IFn(haut) DONE;

if (must_log) {
  for (i=0; i<module_level; i++) header[i]=' '; header[i]=0;
  }
memset(m,0,sizeof(struct og_aut_memory));

m->ctrl_aut=sizeof(struct og_ctrl_aut);
m->State=ctrl_aut->StateNumber*sizeof(struct state);
m->FreeState=ctrl_aut->StateNumber*sizeof(struct free_state);
m->Fstate=ctrl_aut->FstateUsed*sizeof(struct fstate);
m->Gstate=ctrl_aut->GstateUsed*sizeof(struct gstate);
m->Lstmin=ctrl_aut->LstminNumber*sizeof(struct lstmin);

m->total_module = m->ctrl_aut
                + m->State
                + m->FreeState
                + m->Fstate
                + m->Gstate
                + m->Lstmin;

m->total += m->total_module;

DOgShowMem(m->total_module    ,"total_module aut"," ");
DOgShowMem(m->ctrl_aut        ,"ctrl_aut","         ");
DOgShowMem(m->State           ,"State","            ");
DOgShowMem(m->FreeState       ,"FreeState","        ");
DOgShowMem(m->Fstate          ,"Fstate","           ");
DOgShowMem(m->Gstate          ,"Gstate","           ");
DOgShowMem(m->Lstmin          ,"Lstmin","           ");

if (pmem) *pmem=m->total;

DONE;
}




PUBLIC(int) OgAutMemory(void *handle, struct aut_memory *memory)
{
struct og_ctrl_aut *ctrl_aut = (struct og_ctrl_aut *)handle;
memory->aut_used = ctrl_aut->StateUsed;
memory->aut_number = ctrl_aut->StateNumber;
memory->aut_size_cell = sizeof(struct state);

if (ctrl_aut->is_aug) {
  memory->large_fast_automaton = 1;
  memory->auf_used = ctrl_aut->GstateUsed;
  memory->auf_number = ctrl_aut->FstateAllocated;
  memory->auf_size_cell = sizeof(struct fstate);
  }
else {
  memory->large_fast_automaton = 0;
  memory->auf_used = ctrl_aut->FstateUsed;
  memory->auf_number = ctrl_aut->FstateAllocated;
  memory->auf_size_cell = sizeof(struct gstate);
  }

DONE;
}
