/*
 *  Construction de l'automate rapide.
 *  Copyright (c) 1996-2006 Pertimm by Patrick Constant
 *  Dev : D\'ecembre 1996, Janvier 1997, March 2006
 *  Version 1.2
*/

#include "ogm_aut.h"



PUBLIC(int) OgAuf(handle,use_aug)
void *handle;
int use_aug;
{
struct og_ctrl_aut *ctrl_aut = (struct og_ctrl_aut *)handle;
if (ctrl_aut->StateUsed+10 >= (1<<(DPcMxindex-2))) use_aug=1;
if (use_aug) {
  if (ctrl_aut->FstateAllocated) {
    ctrl_aut->FstateAllocated=0;
    DPcFree(ctrl_aut->Fstate);
    }
  return(Aug(ctrl_aut));
  }
DPcFree(ctrl_aut->Gstate);
return(Auf(ctrl_aut));
}

PUBLIC(int) OgAufClean(handle)
void *handle;
{
struct og_ctrl_aut *ctrl_aut = (struct og_ctrl_aut *)handle;
DPcFree(ctrl_aut->State);

DONE;
}


