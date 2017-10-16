/*
 *  Handling addresses and corresponding sockets
 *  Copyright (c) 2005 Pertimm by Patrick Constant
 *  Dev : March 2005
 *  Version 1.0
*/
#include "ogm_addr.h"





int GetAso(ctrl_addr,paso)
struct og_ctrl_addr *ctrl_addr;
struct aso **paso;
{
int i = ctrl_addr->AsoNumber;
struct aso *aso = 0;

beginGetAso:

if (ctrl_addr->FreeAso == -1) {
  if (ctrl_addr->AsoUsed < ctrl_addr->AsoNumber) {
    i = ctrl_addr->AsoUsed++;
    aso = ctrl_addr->Aso + i;
    }
  }
else {
  i = ctrl_addr->FreeAso;
  aso = ctrl_addr->Aso + i;
  ctrl_addr->FreeAso = aso->nx_free;
  }

if (i == ctrl_addr->AsoNumber) {
  char erreur[DOgErrorSize];
  unsigned a, b; struct aso *og_aso;
  if (ctrl_addr->loginfo->trace & DOgAddrTraceMemory) {
    OgMsg(ctrl_addr->hmsg, "",DOgMsgDestInLog
      , "GetAso: max aso number (%d) reached"
      , ctrl_addr->AsoNumber);
    }
  a = ctrl_addr->AsoNumber; b = a + (a>>2) + 1;
  IFn(og_aso=(struct aso *)malloc(b*sizeof(struct aso))) {
    sprintf(erreur,"GetAso: malloc error on Aso for %d structures and %ld bytes", b, b*sizeof(struct aso));
    OgErr(ctrl_addr->herr,erreur); DPcErr;
    }
  memcpy( og_aso, ctrl_addr->Aso, a*sizeof(struct aso));

  DPcFree(ctrl_addr->Aso); ctrl_addr->Aso = og_aso;
  ctrl_addr->AsoNumber = b;

  if (ctrl_addr->loginfo->trace & DOgAddrTraceMemory) {
    OgMsg(ctrl_addr->hmsg, "",DOgMsgDestInLog
      , "GetAso: new aso number is %d", ctrl_addr->AsoNumber);
    }

  if (ctrl_addr->FreeAso != -1) {
    sprintf(erreur,"GetAso: ctrl_addr->FreeAso != -1");
    OgErr(ctrl_addr->herr,erreur); DPcErr;
    }

#ifdef DOgNoMainBufferReallocation
  sprintf(erreur, "GetAso: no more aso max is %d", ctrl_addr->AsoNumber);
  OgErr(ctrl_addr->herr,erreur); DPcErr;
#endif

  goto beginGetAso;
  }

memset(aso,0,sizeof(struct aso));
aso->hsocket = DOgSocketError;

if (paso) *paso = aso;
return(i);
}



