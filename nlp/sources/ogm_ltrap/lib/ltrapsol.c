/*
 *	Handling sol heap.
 *	Copyright (c) 2009 Pertimm by Patrick Constant
 *	Dev : October 2009
 *	Version 1.0
*/
#include "ogm_ltrap.h"


static int LtrapAllocSol(struct og_ctrl_ltrap *,struct sol **);




int LtrapSolAdd(struct og_ctrl_ltrap *ctrl_ltrap)
{
int Isol;
struct sol *sol;
IFE(Isol=LtrapAllocSol(ctrl_ltrap,&sol));
return(Isol);
}





static int LtrapAllocSol(ctrl_ltrap,psol)
struct og_ctrl_ltrap *ctrl_ltrap;
struct sol **psol;
{
char erreur[DOgErrorSize];
struct sol *sol = 0;
int i=ctrl_ltrap->SolNumber;

beginLtrapAllocSol:

if (ctrl_ltrap->SolUsed < ctrl_ltrap->SolNumber) {
  i = ctrl_ltrap->SolUsed++; 
  }

if (i == ctrl_ltrap->SolNumber) {
  unsigned a, b; struct sol *og_l;

  if (ctrl_ltrap->loginfo->trace & DOgLtrapTraceMemory) {
    OgMsg(ctrl_ltrap->hmsg,"",DOgMsgDestInLog
      , "LtrapAllocSol: max Sol number (%d) reached"
      , ctrl_ltrap->SolNumber);
    }
  a = ctrl_ltrap->SolNumber; b = a + (a>>2) + 1;
  IFn(og_l=(struct sol *)malloc(b*sizeof(struct sol))) {
    sprintf(erreur,"LtrapAllocSol: malloc error on Sol");
    OgErr(ctrl_ltrap->herr,erreur); DPcErr;
    }

  memcpy( og_l, ctrl_ltrap->Sol, a*sizeof(struct sol));
  DPcFree(ctrl_ltrap->Sol); ctrl_ltrap->Sol = og_l;
  ctrl_ltrap->SolNumber = b;

  if (ctrl_ltrap->loginfo->trace & DOgLtrapTraceMemory) {
    OgMsg(ctrl_ltrap->hmsg,"",DOgMsgDestInLog
      , "LtrapAllocSol: new Sol number is %d\n", ctrl_ltrap->SolNumber);
    }

#ifdef DOgNoMainBufferReallocation
  sprintf(erreur,"LtrapAllocSol: SolNumber reached (%d)",ctrl_ltrap->SolNumber);
  OgErr(ctrl_ltrap->herr,erreur); DPcErr;
#endif

  goto beginLtrapAllocSol;
  }

sol = ctrl_ltrap->Sol + i;
memset(sol,0,sizeof(struct sol));
sol->start_spansol = (-1);

if (psol) *psol = sol;

return(i);
}


