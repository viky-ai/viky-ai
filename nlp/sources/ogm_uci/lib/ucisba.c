/*
 *  Buffer for all operation handling
 *  Copyright (c) 2006 Pertimm by Patrick Constant
 *  Dev : August 2006
 *  Version 1.0
*/
#include "ogm_uci.h"





int UcisAppendBa(ctrl_ucis,is,s)
struct og_ctrl_ucis *ctrl_ucis;
int is; og_string s;
{
if (is<=0) DONE;
IFE(UcisTestReallocBa(ctrl_ucis,is));
memcpy(ctrl_ucis->Ba + ctrl_ucis->BaUsed, s, is);
ctrl_ucis->BaUsed += is;
DONE;
}




int UcisTestReallocBa(ctrl_ucis,added_size)
struct og_ctrl_ucis *ctrl_ucis;
int added_size;
{
/** +9 because we want to have always extra chars at the end for safety reasons **/
if (ctrl_ucis->BaUsed + added_size +9 > ctrl_ucis->BaSize) {
  IFE(UcisReallocBa(ctrl_ucis,added_size));
  }
DONE;
}




int UcisReallocBa(ctrl_ucis,added_size)
struct og_ctrl_ucis *ctrl_ucis;
int added_size;
{
unsigned a, a1, b; unsigned char *og_buffer;
char erreur[DOgErrorSize];

if (ctrl_ucis->loginfo->trace & DOgUciClientTraceMemory) {
  OgMsg(ctrl_ucis->hmsg, "", DOgMsgDestInLog
    , "UcisReallocBa: max Ba size (%d) reached, added_size=%d"
    , ctrl_ucis->BaSize,added_size);
  }
a = ctrl_ucis->BaSize; a1 = a+added_size; b = a1 + (a1>>2) + 1;
IFn(og_buffer=(unsigned char *)malloc(b*sizeof(unsigned char))) {
  sprintf(erreur,"UcisReallocBa: malloc error on Ba");
  OgErr(ctrl_ucis->herr,erreur); DPcErr;
  }

memcpy( og_buffer, ctrl_ucis->Ba, a*sizeof(unsigned char));
DPcFree(ctrl_ucis->Ba); ctrl_ucis->Ba = og_buffer;
ctrl_ucis->BaSize = b;

if (ctrl_ucis->loginfo->trace & DOgUciClientTraceMemory) {
  OgMsg(ctrl_ucis->hmsg, "", DOgMsgDestInLog
    , "UcisReallocBa: new Ba size is %d", ctrl_ucis->BaSize);
  }

#ifdef DOgNoMainBufferReallocation
sprintf(erreur,"UcisReallocBa: BaSize reached (%d)",ctrl_ucis->BaSize);
OgErr(ctrl_ucis->herr,erreur); DPcErr;
#endif

DONE;
}







