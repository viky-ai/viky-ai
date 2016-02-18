/*
 *	Buffer for all operation handling
 *	Copyright (c) 2009 Pertimm by Patrick Constant
 *	Dev : October 2009
 *	Version 1.0
*/
#include "ogm_ltrap.h"


STATICF(int) LtrapReallocBa(pr_(struct og_ctrl_ltrap *) pr(int));



int LtrapAppendBa(ctrl_ltrap,is,s)
struct og_ctrl_ltrap *ctrl_ltrap;
int is; unsigned char *s;
{
if (is<=0) DONE;
IFE(LtrapTestReallocBa(ctrl_ltrap,is));
memcpy(ctrl_ltrap->Ba + ctrl_ltrap->BaUsed, s, is);
ctrl_ltrap->BaUsed += is;
DONE;
}




int LtrapTestReallocBa(ctrl_ltrap,added_size)
struct og_ctrl_ltrap *ctrl_ltrap;
int added_size;
{
/** +9 because we want to have always extra chars at the end for safety reasons **/
if (ctrl_ltrap->BaUsed + added_size +9 > ctrl_ltrap->BaSize) {
  IFE(LtrapReallocBa(ctrl_ltrap,added_size));
  }
DONE;
}




STATICF(int) LtrapReallocBa(ctrl_ltrap,added_size)
struct og_ctrl_ltrap *ctrl_ltrap;
int added_size;
{
unsigned a, a1, b; unsigned char *og_buffer;
char erreur[DOgErrorSize];

if (ctrl_ltrap->loginfo->trace & DOgLtrapTraceMemory) {
  OgMsg(ctrl_ltrap->hmsg,"",DOgMsgDestInLog
    , "LtrapReallocBa from ogm_ltrap: max Ba size (%d) reached, added_size=%d"
    , ctrl_ltrap->BaSize,added_size);
  }
a = ctrl_ltrap->BaSize; a1 = a+added_size; b = a1 + (a1>>2) + 1;
IFn(og_buffer=(unsigned char *)malloc(b*sizeof(unsigned char))) {
  sprintf(erreur,"LtrapReallocBa from ogm_ltrap: malloc error on Ba");
  OgErr(ctrl_ltrap->herr,erreur); DPcErr;
  }

memcpy( og_buffer, ctrl_ltrap->Ba, a*sizeof(unsigned char));
DPcFree(ctrl_ltrap->Ba); ctrl_ltrap->Ba = og_buffer;
ctrl_ltrap->BaSize = b;

if (ctrl_ltrap->loginfo->trace & DOgLtrapTraceMemory) {
  OgMsg(ctrl_ltrap->hmsg,"",DOgMsgDestInLog
    , "LtrapReallocBa from ogm_ltrap: new Ba size is %d", ctrl_ltrap->BaSize);
  }

#ifdef DOgNoMainBufferReallocation
sprintf(erreur,"LtrapReallocBa from ogm_ltrap: BaSize reached (%d)",ctrl_ltrap->BaSize);
OgErr(ctrl_ltrap->herr,erreur); DPcErr;
#endif

DONE;
}







