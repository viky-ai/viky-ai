/*
 *  Buffer for all operation handling
 *  Copyright (c) 2006 Pertimm by Patrick Constant
 *  Dev : August 2006
 *  Version 1.0
*/
#include "ogm_uci.h"





int UcicAppendBa(ctrl_ucic,is,s)
struct og_ctrl_ucic *ctrl_ucic;
int is; unsigned char *s;
{
if (is<=0) DONE;
IFE(UcicTestReallocBa(ctrl_ucic,is));
memcpy(ctrl_ucic->Ba + ctrl_ucic->BaUsed, s, is);
ctrl_ucic->BaUsed += is;
DONE;
}




int UcicTestReallocBa(ctrl_ucic,added_size)
struct og_ctrl_ucic *ctrl_ucic;
int added_size;
{
/** +9 because we want to have always extra chars at the end for safety reasons **/
if (ctrl_ucic->BaUsed + added_size +9 > ctrl_ucic->BaSize) {
  IFE(UcicReallocBa(ctrl_ucic,added_size));
  }
DONE;
}




int UcicReallocBa(ctrl_ucic,added_size)
struct og_ctrl_ucic *ctrl_ucic;
int added_size;
{
unsigned a, a1, b; unsigned char *og_buffer;
char erreur[DOgErrorSize];

if (ctrl_ucic->loginfo->trace & DOgUciClientTraceMemory) {
  OgMsg(ctrl_ucic->hmsg, "", DOgMsgDestInLog
    , "UcicReallocBa: max Ba size (%d) reached, added_size=%d"
    , ctrl_ucic->BaSize,added_size);
  }
a = ctrl_ucic->BaSize; a1 = a+added_size; b = a1 + (a1>>2) + 1;
IFn(og_buffer=(unsigned char *)malloc(b*sizeof(unsigned char))) {
  sprintf(erreur,"UcicReallocBa: malloc error on Ba");
  OgErr(ctrl_ucic->herr,erreur); DPcErr;
  }

memcpy( og_buffer, ctrl_ucic->Ba, a*sizeof(unsigned char));
DPcFree(ctrl_ucic->Ba); ctrl_ucic->Ba = og_buffer;
ctrl_ucic->BaSize = b;

if (ctrl_ucic->loginfo->trace & DOgUciClientTraceMemory) {
  OgMsg(ctrl_ucic->hmsg, "", DOgMsgDestInLog
    , "UcicReallocBa: new Ba size is %d", ctrl_ucic->BaSize);
  }

#ifdef DOgNoMainBufferReallocation
sprintf(erreur,"UcicReallocBa: BaSize reached (%d)",ctrl_ucic->BaSize);
OgErr(ctrl_ucic->herr,erreur); DPcErr;
#endif

DONE;
}







