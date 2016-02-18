/*
 *	Handling of 
 *	Copyright (c) 2000-2006 Pertimm by Patrick Constant
 *	Dev : May 2000, February 2006
 *	Version 1.1
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
  unsigned a, b; struct lstmin *og_lstmin;

  if (ctrl_aut->loginfo->trace & DOgAutTraceMinimization) {
    OgMessageLog( DOgMlogInLog, ctrl_aut->loginfo->where, 0
                , "AllocLstmin (%s): max Lstmin number (%d) reached"
                ,ctrl_aut->name , ctrl_aut->LstminNumber);
    }
  a = ctrl_aut->LstminNumber; b = a + (a>>2) + 1;
  IFn(og_lstmin=(struct lstmin *)malloc(b*sizeof(struct lstmin))) {
    sprintf(erreur,"AllocLstmin (%s): malloc error on Lstmin",ctrl_aut->name);
    OgErr(ctrl_aut->herr,erreur); DPcErr;
    }

  memcpy( og_lstmin, ctrl_aut->Lstmin, a*sizeof(struct lstmin));
  DPcFree(ctrl_aut->Lstmin); ctrl_aut->Lstmin = og_lstmin;
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



