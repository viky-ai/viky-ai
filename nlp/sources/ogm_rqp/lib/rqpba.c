/*
 *	Buffer for all operation handling
 *	Copyright (c) 2006 Pertimm by Patrick Constant
 *	Dev : July 2006
 *	Version 1.0
*/
#include "ogm_rqp.h"





int RqpAppendBa(struct og_ctrl_rqp *ctrl_rqp, int is, unsigned char *s)
{
if (is<=0) DONE;
IFE(RqpTestReallocBa(ctrl_rqp,is));
memcpy(ctrl_rqp->Ba + ctrl_rqp->BaUsed, s, is);
ctrl_rqp->BaUsed += is;
DONE;
}




int RqpTestReallocBa(struct og_ctrl_rqp *ctrl_rqp, int added_size)
{
/** +9 because we want to have always extra chars at the end for safety reasons **/
if (ctrl_rqp->BaUsed + added_size +9 > ctrl_rqp->BaSize) {
  IFE(RqpReallocBa(ctrl_rqp,added_size));
  }
DONE;
}




int RqpReallocBa(struct og_ctrl_rqp *ctrl_rqp, int added_size)
{
unsigned a, a1, b; unsigned char *og_buffer;
char erreur[DOgErrorSize];

if (ctrl_rqp->loginfo->trace & DOgRqpTraceMemory) {
  OgMsg(ctrl_rqp->hmsg,"",DOgMsgDestInLog
    , "RqpReallocBa: max Ba size (%d) reached"
    , ctrl_rqp->BaSize);
  }
a = ctrl_rqp->BaSize; a1 = a+added_size; b = a1 + (a1>>2) + 1;
IFn(og_buffer=(unsigned char *)malloc(b*sizeof(unsigned char))) {
  sprintf(erreur,"RqpReallocBa: malloc error on Ba");
  OgErr(ctrl_rqp->herr,erreur); DPcErr;
  }

memcpy( og_buffer, ctrl_rqp->Ba, a*sizeof(unsigned char));
DPcFree(ctrl_rqp->Ba); ctrl_rqp->Ba = og_buffer;
ctrl_rqp->BaSize = b;

if (ctrl_rqp->loginfo->trace & DOgRqpTraceMemory) {
  OgMsg(ctrl_rqp->hmsg,"",DOgMsgDestInLog
    , "RqpReallocBa: new Ba size is %d\n", ctrl_rqp->BaSize);
  }

#ifdef DOgNoMainBufferReallocation
sprintf(erreur,"RqpReallocBa: BaSize reached (%d)",ctrl_rqp->BaSize);
OgErr(ctrl_rqp->herr,erreur); DPcErr;
#endif

DONE;
}




