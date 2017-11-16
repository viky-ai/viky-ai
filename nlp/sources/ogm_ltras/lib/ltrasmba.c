/*
 *	Buffer for all operations handling on trfs
 *	Copyright (c) 2009 Pertimm by Patrick Constant
 *	Dev: November 2009
 *	Version 1.0
*/
#include "ogm_ltras.h"



int LtrasTrfsAppendBa(struct og_ctrl_ltras *ctrl_ltras,struct og_ltra_trfs *trfs,int is, unsigned char *s)
{
if (is<=0) DONE;
IFE(LtrasTrfsTestReallocBa(ctrl_ltras,trfs,is));
memcpy(trfs->Ba + trfs->BaUsed, s, is);
trfs->BaUsed += is;
DONE;
}



int LtrasTrfsTestReallocBa(struct og_ctrl_ltras *ctrl_ltras,struct og_ltra_trfs *trfs,int added_size)
{
/** +9 because we want to have always extra chars at the end for safety reasons **/
if (trfs->BaUsed + added_size +9 > trfs->BaSize) {
  IFE(LtrasTrfsReallocBa(ctrl_ltras,trfs,added_size));
  }
DONE;
}



int LtrasTrfsReallocBa(struct og_ctrl_ltras *ctrl_ltras,struct og_ltra_trfs *trfs,int added_size)
{
unsigned a, a1, b; unsigned char *og_buffer;
char erreur[DPcSzErr];

if (ctrl_ltras->loginfo->trace & DOgLtrasTraceMemory) {
  OgMsg(ctrl_ltras->hmsg,"",DOgMsgDestInLog, "LtrasTrfsReallocBa: max Ba size (%d) reached", trfs->BaSize);
  }
a = trfs->BaSize; a1 = a+added_size; b = a1 + (a1>>2) + 1;
IFn(og_buffer=(unsigned char *)malloc(b*sizeof(unsigned char))) {
  sprintf(erreur,"LtrasTrfsReallocBa: malloc error on Ba (%d bytes)",b);
  PcErr(-1,erreur); DPcErr;
  }

memcpy( og_buffer, trfs->Ba, a*sizeof(unsigned char));
DPcFree(trfs->Ba); trfs->Ba = og_buffer;
trfs->BaSize = b;

if (ctrl_ltras->loginfo->trace & DOgLtrasTraceMemory) {
  OgMsg(ctrl_ltras->hmsg,"",DOgMsgDestInLog,"LtrasTrfsReallocBa: new Ba size is %d\n", trfs->BaSize);
  }

#ifdef DOgNoMainBufferReallocation
sprintf(erreur,"LtrasTrfsReallocBa: BaSize reached (%d)",trfs->BaSize);
PcErr(-1,erreur); DPcErr;
#endif

DONE;
}


