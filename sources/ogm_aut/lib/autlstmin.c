/*
 *  Handling of
 *  Copyright (c) 2000-2006 Pertimm by Patrick Constant
 *  Dev : May 2000, February 2006
 *  Version 1.1
*/
#include "ogm_aut.h"



int AllocLstmin(ctrl_aut)
struct og_ctrl_aut *ctrl_aut;
{
struct lstmin *lstmin;
char erreur[DOgErrorSize];
int i = ctrl_aut->LstminNumber;

beginAllocLstmin:

if (ctrl_aut->LstminUsed < ctrl_aut->LstminNumber) {
  /** zero cell is forbidden because zero value of index is used **/
  IFn(ctrl_aut->LstminUsed) {
    lstmin = ctrl_aut->Lstmin + 0;
    memset(lstmin,0,sizeof(struct lstmin));
    ctrl_aut->LstminUsed++;
    }
  i = ctrl_aut->LstminUsed++;
  }

if (i == ctrl_aut->LstminNumber) {
  if (ctrl_aut->loginfo->trace & DOgAutTraceMinimization) {
    OgMessageLog( DOgMlogInLog, ctrl_aut->loginfo->where, 0
                , "AllocLstmin (%s): max Lstmin number (%d) reached"
                ,ctrl_aut->name , ctrl_aut->LstminNumber);
    }
  unsigned a = ctrl_aut->LstminNumber;
  size_t b = (a * 1.1) + 1;

  // use a temporary pointer to avoid realloc error
  struct lstmin *og_lstmin = (struct lstmin *) realloc(ctrl_aut->Lstmin, b*sizeof(struct lstmin));
  IFn(og_lstmin)
  {
    snprintf(erreur, DOgErrorSize, "AllocLstmin on '%s': realloc error with"
        " new_size=%zu bytes (LstminNumber=%zu)", ctrl_aut->name, b*sizeof(struct lstmin), b);
    OgErr(ctrl_aut->herr, erreur);
    DPcErr;
  }
  ctrl_aut->Lstmin = og_lstmin;
  ctrl_aut->LstminNumber = b;

  if (ctrl_aut->loginfo->trace & DOgAutTraceMinimization) {
    OgMessageLog( DOgMlogInLog, ctrl_aut->loginfo->where, 0
                , "AllocLstmin (%s): new Lstmin number is %d\n", ctrl_aut->name, ctrl_aut->LstminNumber);
    }

#ifdef DOgNoMainBufferReallocation
  sprintf(erreur,"AllocLstmin (%s): LstminNumber reached (%d)",ctrl_aut->name,ctrl_aut->LstminNumber);
  OgErr(ctrl_aut->herr,erreur); DPcErr;
#endif

  goto beginAllocLstmin;
  }


if (i == ctrl_aut->LstminNumber) {
  sprintf(erreur,"AllocLstmin (%s): LstminNumber reached (%d)",ctrl_aut->name,ctrl_aut->LstminNumber);
  OgErr(ctrl_aut->herr,erreur); DPcErr;
  }

lstmin = ctrl_aut->Lstmin + i;
memset(lstmin,0,sizeof(struct lstmin));

return(i);
}



