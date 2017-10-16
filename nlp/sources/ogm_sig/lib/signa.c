/*
 *	Handling signa structure.
 *	Copyright (c) 2007 Pertimm by Patrick Constant
 *	Dev : August 2007
 *	Version 1.0
*/
#include "ogm_sig.h"



STATICF(int) AllocSigna(pr_(struct og_ctrl_sig *) pr(struct signa **));




int SigAddSigna(ctrl_sig,signal_type,func,context)
struct og_ctrl_sig *ctrl_sig; int signal_type;
void (*func)(pr_(void *) pr(int));
void *context;
{
struct signa *signa;
int found,Isigna;

IFE(found=SigGetSigna(ctrl_sig,signal_type,&Isigna));
if (found) {
  signa = ctrl_sig->Signa + Isigna;
  signa->func=func;
  return(Isigna);
  }

IFE(Isigna=AllocSigna(ctrl_sig,&signa));
signa->signal_type=signal_type;
signa->context=context;
signa->func=func;
return(Isigna);
}






int SigGetSigna(ctrl_sig,signal_type,pIsigna)
struct og_ctrl_sig *ctrl_sig; 
int signal_type, *pIsigna;
{
struct signa *signa;
int i;

*pIsigna=(-1);

for (i=0; i<ctrl_sig->SignaUsed; i++) {
  signa = ctrl_sig->Signa + i;
  if (signa->signal_type == signal_type) { *pIsigna=i; return(1); }
  }
return(0);
}






STATICF(int) AllocSigna(ctrl_sig,psigna)
struct og_ctrl_sig *ctrl_sig;
struct signa **psigna;
{
char erreur[DOgErrorSize];
struct signa *signa = 0;
int i=ctrl_sig->SignaNumber;

beginAllocSigna:

if (ctrl_sig->SignaUsed < ctrl_sig->SignaNumber) {
  i = ctrl_sig->SignaUsed++; 
  }

if (i == ctrl_sig->SignaNumber) {
  unsigned a, b; struct signa *og_l;

  if (ctrl_sig->loginfo->trace & DOgSigTraceMemory) {
    OgMessageLog(DOgMlogInLog,ctrl_sig->loginfo->where,0
      , "AllocSigna: max Signa number (%d) reached"
      , ctrl_sig->SignaNumber);
    }
  a = ctrl_sig->SignaNumber; b = a + (a>>2) + 1;
  IFn(og_l=(struct signa *)malloc(b*sizeof(struct signa))) {
    sprintf(erreur,"AllocSigna: malloc error on Signa");
    OgErr(ctrl_sig->herr,erreur); DPcErr;
    }

  memcpy( og_l, ctrl_sig->Signa, a*sizeof(struct signa));
  DPcFree(ctrl_sig->Signa); ctrl_sig->Signa = og_l;
  ctrl_sig->SignaNumber = b;

  if (ctrl_sig->loginfo->trace & DOgSigTraceMemory) {
    OgMessageLog(DOgMlogInLog,ctrl_sig->loginfo->where,0
      , "AllocSigna: new Signa number is %d\n", ctrl_sig->SignaNumber);
    }

#ifdef DOgNoMainBufferReallocation
  sprintf(erreur,"AllocSigna: SignaNumber reached (%d)",ctrl_sig->SignaNumber);
  OgErr(ctrl_sig->herr,erreur); DPcErr;
#endif

  goto beginAllocSigna;
  }

signa = ctrl_sig->Signa + i;
memset(signa,0,sizeof(struct signa));

if (psigna) *psigna = signa;

return(i);
}



