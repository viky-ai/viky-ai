/*
 *	Initialization for ogm_sig functions
 *	Copyright (c) 2007 Pertimm by Patrick Constant
 *	Dev : August 2007
 *	Version 1.0
*/
#include "ogm_sig.h"



/*
 *  This global declaration is necessary because signal handler do not have a context
 *  but anyway, signals are global to a program. This means that this API
 *  must never be declare within another API, but only in the main() function
 *  of the program. otherwise we might be mixed-up unecessarily.
*/

struct og_ctrl_sig *OgCtrlSig;



PUBLIC(void *) OgSigInit(param)
struct og_sig_param *param;
{
char erreur[DOgErrorSize];
struct og_ctrl_sig *ctrl_sig;
int size;

IFn(ctrl_sig=(struct og_ctrl_sig *)malloc(sizeof(struct og_ctrl_sig))) {
  sprintf(erreur,"OgSigInit: malloc error on ctrl_sig");
  OgErr(param->herr,erreur); return(0);
  }

OgCtrlSig = ctrl_sig;
memset(ctrl_sig,0,sizeof(struct og_ctrl_sig));

ctrl_sig->herr = param->herr;
ctrl_sig->hmutex = param->hmutex;
ctrl_sig->cloginfo = param->loginfo;
ctrl_sig->loginfo = &ctrl_sig->cloginfo;

ctrl_sig->SignaNumber = DOgSsrvSignaNumber;
size = ctrl_sig->SignaNumber*sizeof(struct signa);
IFn(ctrl_sig->Signa=(struct signa *)malloc(size)) {
  sprintf(erreur,"OgSsrvInit: malloc error on Signa (%d bytes)",size);
  OgErr(ctrl_sig->herr,erreur); return(0);
  }


return((void *)ctrl_sig);
}






PUBLIC(int) OgSigFlush(handle)
void *handle;
{
struct og_ctrl_sig *ctrl_sig = (struct og_ctrl_sig *)handle;

DPcFree(ctrl_sig->Signa);
DPcFree(ctrl_sig);
DONE;
}



