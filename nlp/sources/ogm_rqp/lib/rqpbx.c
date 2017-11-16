/*
 *	Buffer for OgRqpXmlPrettyTree
 *	Copyright (c) 2010 Pertimm by Patrick Constant
 *	Dev : May 2010
 *	Version 1.0
*/
#include "ogm_rqp.h"





int RqpAppendBx(struct og_ctrl_rqp *ctrl_rqp, int is, unsigned char *s)
{
if (is<=0) DONE;
IFE(RqpTestReallocBx(ctrl_rqp,is));
memcpy(ctrl_rqp->Bx + ctrl_rqp->BxUsed, s, is);
ctrl_rqp->BxUsed += is;
DONE;
}




int RqpTestReallocBx(struct og_ctrl_rqp *ctrl_rqp, int added_size)
{
/** +9 because we want to have always extra chars at the end for safety reasons **/
if (ctrl_rqp->BxUsed + added_size +9 > ctrl_rqp->BxSize) {
  IFE(RqpReallocBx(ctrl_rqp,added_size));
  }
DONE;
}




int RqpReallocBx(struct og_ctrl_rqp *ctrl_rqp, int added_size)
{
unsigned a, a1, b; unsigned char *og_buffer;
char erreur[DOgErrorSize];

if (ctrl_rqp->loginfo->trace & DOgRqpTraceMemory) {
  OgMsg(ctrl_rqp->hmsg,"",DOgMsgDestInLog
    , "RqpReallocBx: max Bx size (%d) reached"
    , ctrl_rqp->BxSize);
  }
a = ctrl_rqp->BxSize; a1 = a+added_size; b = a1 + (a1>>2) + 1;
IFn(og_buffer=(unsigned char *)malloc(b*sizeof(unsigned char))) {
  sprintf(erreur,"RqpReallocBx: malloc error on Bx");
  OgErr(ctrl_rqp->herr,erreur); DPcErr;
  }

memcpy( og_buffer, ctrl_rqp->Bx, a*sizeof(unsigned char));
DPcFree(ctrl_rqp->Bx); ctrl_rqp->Bx = og_buffer;
ctrl_rqp->BxSize = b;

if (ctrl_rqp->loginfo->trace & DOgRqpTraceMemory) {
  OgMsg(ctrl_rqp->hmsg,"",DOgMsgDestInLog
    , "RqpReallocBx: new Bx size is %d\n", ctrl_rqp->BxSize);
  }

#ifdef DOgNoMainBufferReallocation
sprintf(erreur,"RqpReallocBx: BxSize reached (%d)",ctrl_rqp->BxSize);
OgErr(ctrl_rqp->herr,erreur); DPcErr;
#endif

DONE;
}




