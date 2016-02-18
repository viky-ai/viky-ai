/*
 *	Buffer for all operation handling
 *	Copyright (c) 2006 Pertimm by Patrick Constant
 *	Dev : August 2006
 *	Version 1.0
*/
#include "ogm_uci.h"





int UciAppendBa(ctrl_uci,is,s)
struct og_ctrl_uci *ctrl_uci;
int is; unsigned char *s;
{
if (is<=0) DONE;
IFE(UciTestReallocBa(ctrl_uci,is));
memcpy(ctrl_uci->Ba + ctrl_uci->BaUsed, s, is);
ctrl_uci->BaUsed += is;
DONE;
}




int UciTestReallocBa(ctrl_uci,added_size)
struct og_ctrl_uci *ctrl_uci;
int added_size;
{
/** +9 because we want to have always extra chars at the end for safety reasons **/
if (ctrl_uci->BaUsed + added_size +9 > ctrl_uci->BaSize) {
  IFE(UciReallocBa(ctrl_uci,added_size));
  }
DONE;
}




int UciReallocBa(ctrl_uci,added_size)
struct og_ctrl_uci *ctrl_uci;
int added_size;
{
unsigned a, a1, b; unsigned char *og_buffer;
char erreur[DOgErrorSize];

if (ctrl_uci->loginfo->trace & DOgUciTraceMemory) {
  OgMsg(ctrl_uci->hmsg, "", DOgMlogInLog
    , "UciReallocBa: max Ba size (%d) reached, added_size=%d"
    , ctrl_uci->BaSize,added_size);
  }
a = ctrl_uci->BaSize; a1 = a+added_size; b = a1 + (a1>>2) + 1;
IFn(og_buffer=(unsigned char *)malloc(b*sizeof(unsigned char))) {
  sprintf(erreur,"UciReallocBa: malloc error on Ba");
  OgErr(ctrl_uci->herr,erreur); DPcErr;
  }

memcpy( og_buffer, ctrl_uci->Ba, a*sizeof(unsigned char));
DPcFree(ctrl_uci->Ba); ctrl_uci->Ba = og_buffer;
ctrl_uci->BaSize = b;

if (ctrl_uci->loginfo->trace & DOgUciTraceMemory) {
  OgMsg(ctrl_uci->hmsg, "", DOgMlogInLog
    , "UciReallocBa: new Ba size is %d", ctrl_uci->BaSize);
  }

#ifdef DOgNoMainBufferReallocation
sprintf(erreur,"UciReallocBa: BaSize reached (%d)",ctrl_uci->BaSize);
OgErr(ctrl_uci->herr,erreur); DPcErr;
#endif

DONE;
}







