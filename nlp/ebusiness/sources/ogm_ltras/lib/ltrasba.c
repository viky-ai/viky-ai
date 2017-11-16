/*
 *	Buffer for all operation handling
 *	Copyright (c) 2009 Pertimm by Patrick Constant
 *	Dev : November 2009
 *	Version 1.0
*/
#include "ogm_ltras.h"





int LtrasAppendBa(struct og_ctrl_ltras *ctrl_ltras,int is, unsigned char *s)
{
if (is<=0) DONE;
IFE(LtrasTestReallocBa(ctrl_ltras,is));
memcpy(ctrl_ltras->Ba + ctrl_ltras->BaUsed, s, is);
ctrl_ltras->BaUsed += is;
DONE;
}





int LtrasTestReallocBa(struct og_ctrl_ltras *ctrl_ltras, int added_size)
{
/** +9 because we want to have always extra chars at the end for safety reasons **/
if (ctrl_ltras->BaUsed + added_size +9 > ctrl_ltras->BaSize) {
  IFE(LtrasReallocBa(ctrl_ltras,added_size));
  }
DONE;
}






int LtrasReallocBa(struct og_ctrl_ltras *ctrl_ltras,int added_size)
{
unsigned a, a1, b; unsigned char *og_buffer;
char erreur[DPcSzErr];

if (ctrl_ltras->loginfo->trace & DOgLtrasTraceMemory) {
  OgMsg(ctrl_ltras->hmsg,"",DOgMsgDestInLog, "LtrasReallocBa: max Ba size (%d) reached", ctrl_ltras->BaSize);
  }
a = ctrl_ltras->BaSize; a1 = a+added_size; b = a1 + (a1>>2) + 1;
IFn(og_buffer=(unsigned char *)malloc(b*sizeof(unsigned char))) {
  sprintf(erreur,"LtrasReallocBa: malloc error on Ba (%d bytes)",b);
  PcErr(-1,erreur); DPcErr;
  }

memcpy( og_buffer, ctrl_ltras->Ba, a*sizeof(unsigned char));
DPcFree(ctrl_ltras->Ba); ctrl_ltras->Ba = og_buffer;
ctrl_ltras->BaSize = b;

if (ctrl_ltras->loginfo->trace & DOgLtrasTraceMemory) {
  OgMsg(ctrl_ltras->hmsg,"",DOgMsgDestInLog,"LtrasReallocBa: new Ba size is %d\n", ctrl_ltras->BaSize);
  }

#ifdef DOgNoMainBufferReallocation
sprintf(erreur,"LtrasReallocBa: BaSize reached (%d)",ctrl_ltras->BaSize);
PcErr(-1,erreur); DPcErr;
#endif

DONE;
}




