/*
 *  Handling trf structures
 *  Copyright (c) 2009 Pertimm by Patrick Constant
 *  Dev: November 2009
 *  Version 1.0
*/
#include "ogm_ltras.h"




int LtrasResetTrf(struct og_ctrl_ltras *ctrl_ltras, struct og_ltra_trfs *trfs, int Itrf)
{
struct og_ltra_trf *trf = trfs->Trf + Itrf;
memset(trf,0,sizeof(struct og_ltra_trf));
trf->span_start_trf=(-1);
trf->start_word=(-1);
trf->start_module = (-1);
trf->trfs=trfs;
DONE;
}




int LtrasAllocTrf(struct og_ctrl_ltras *ctrl_ltras, struct og_ltra_trfs *trfs, struct og_ltra_trf **ptrf)
{
char erreur[DOgErrorSize];
struct og_ltra_trf *trf = 0;
int i=trfs->TrfNumber;

beginAllocTrf:

if (trfs->TrfUsed < trfs->TrfNumber) {
  i = trfs->TrfUsed++;
  }

if (i == trfs->TrfNumber) {
  unsigned a, b; struct og_ltra_trf *og_trf;

  if (ctrl_ltras->loginfo->trace & DOgLtrasTraceMemory) {
    OgMsg(ctrl_ltras->hmsg,"",DOgMsgDestInLog
      , "AllocTrf: max Trf number (%d) reached"
      , trfs->TrfNumber);
    }
  a = trfs->TrfNumber; b = a + (a>>2) + 1;
  IFn(og_trf=(struct og_ltra_trf *)malloc(b*sizeof(struct og_ltra_trf))) {
    sprintf(erreur,"AllocTrf: malloc error on Trf");
    OgErr(ctrl_ltras->herr,erreur); DPcErr;
    }

  memcpy( og_trf, trfs->Trf, a*sizeof(struct og_ltra_trf));
  DPcFree(trfs->Trf); trfs->Trf = og_trf;
  trfs->TrfNumber = b;

  if (ctrl_ltras->loginfo->trace & DOgLtrasTraceMemory) {
    OgMsg(ctrl_ltras->hmsg,"",DOgMsgDestInLog
      , "AllocTrf: new Trf number is %d\n", trfs->TrfNumber);
    }

#ifdef DOgNoMainBufferReallocation
  sprintf(erreur,"AllocTrf: TrfNumber reached (%d)",trfs->TrfNumber);
  OgErr(ctrl_ltras->herr,erreur); DPcErr;
#endif

  goto beginAllocTrf;
  }

trf = trfs->Trf + i;
IFE(LtrasResetTrf(ctrl_ltras, trfs,i));
trf->start_module = (-1);
if (ptrf) *ptrf = trf;
return(i);
}


