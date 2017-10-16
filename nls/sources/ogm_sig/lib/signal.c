/*
 *	Encoding the signal function as OgSignal
 *	Copyright (c) 2007 Pertimm by Patrick Constant
 *	Dev : August 2007
 *	Version 1.0
*/
#include "ogm_sig.h"
#include <signal.h>



STATICF(void) OgSignalFunc(pr(int));





PUBLIC(int) OgSignal(handle, signal_type, func, context, flag)
void *handle; int signal_type; 
void (*func)(pr_(void *) pr(int));
void *context; int flag;
{
struct og_ctrl_sig *ctrl_sig = (struct og_ctrl_sig *)handle;
void (*signal_func)(pr(int));
#if (DPcSystem == DPcSystemUnix)
char sys_erreur[DPcSzErr], erreur[DPcSzErr];
int retour;
#endif

IFE(SigAddSigna(ctrl_sig,signal_type,func,context));

signal_func=OgSignalFunc;
switch(flag) {
  case DOgSigFlagIgn: signal_func=SIG_IGN; break;
  case DOgSigFlagDfl: signal_func=SIG_DFL; break;
  }

#if (DPcSystem == DPcSystemUnix)

retour = signal(signal_type,OgSignalFunc);
if (retour == SIG_ERR) {
  int nerr = OgSysErrMes(SIG_ERR,DPcSzErr,sys_erreur);
  sprintf(erreur,"OgSignal: signal:(%d) %s\n",nerr,sys_erreur);
  OgErr(ctrl_sig->herr,erreur); DPcErr;
  }

#else
#if (DPcSystem == DPcSystemWin32)
signal(signal_type,OgSignalFunc);
#endif
#endif

DONE;
}





STATICF(void) OgSignalFunc(signal_type)
int signal_type;
{
struct og_ctrl_sig *ctrl_sig = OgCtrlSig;
struct signa *signa;
int found,Isigna;

found=SigGetSigna(ctrl_sig,signal_type,&Isigna);
if (!found) return;

signa = ctrl_sig->Signa + Isigna;
IFx(signa->func) {
  (*signa->func)(signa->context,signal_type);
  }

return;
}



