/*
 *	Buffer for all operation handling
 *	Copyright (c) 2004 Pertimm by Patrick Constant
 *	Dev : January 2004
 *	Version 1.0
*/
#include "ogm_addr.h"





int OgAddrAppendBa(ctrl_addr,is,s)
struct og_ctrl_addr *ctrl_addr;
int is; unsigned char *s;
{
if (is<=0) DONE;
IFE(OgAddrTestReallocBa(ctrl_addr,is));
memcpy(ctrl_addr->Ba + ctrl_addr->BaUsed, s, is);
ctrl_addr->BaUsed += is;
DONE;
}




int OgAddrTestReallocBa(ctrl_addr,added_size)
struct og_ctrl_addr *ctrl_addr;
int added_size;
{
/** +9 because we want to have always extra chars at the end for safety reasons **/
if (ctrl_addr->BaUsed + added_size +9 > ctrl_addr->BaSize) {
  IFE(OgAddrReallocBa(ctrl_addr,added_size));
  }
DONE;
}




int OgAddrReallocBa(ctrl_addr,added_size)
struct og_ctrl_addr *ctrl_addr;
int added_size;
{
unsigned a, a1, b; unsigned char *og_buffer;
char erreur[DOgErrorSize];

if (ctrl_addr->loginfo->trace & DOgAddrTraceMemory) {
  OgMsg(ctrl_addr->hmsg, "",DOgMlogInLog
              , "OgAddrReallocBa from ogm_url: max Ba size (%d) reached, added_size=%d"
              , ctrl_addr->BaSize,added_size);
  }
a = ctrl_addr->BaSize; a1 = a+added_size; b = a1 + (a1>>2) + 1;
IFn(og_buffer=(unsigned char *)malloc(b*sizeof(unsigned char))) {
  sprintf(erreur,"OgAddrReallocBa from ogm_url: malloc error on Ba");
  OgErr(ctrl_addr->herr,erreur); DPcErr;
  }

memcpy( og_buffer, ctrl_addr->Ba, a*sizeof(unsigned char));
DPcFree(ctrl_addr->Ba); ctrl_addr->Ba = og_buffer;
ctrl_addr->BaSize = b;

if (ctrl_addr->loginfo->trace & DOgAddrTraceMemory) {
  OgMsg(ctrl_addr->hmsg, "",DOgMlogInLog
              , "OgAddrReallocBa from ogm_url: new Ba size is %d", ctrl_addr->BaSize);
  }

#ifdef DOgNoMainBufferReallocation
sprintf(erreur,"OgAddrReallocBa from ogm_url: BaSize reached (%d)",ctrl_addr->BaSize);
OgErr(ctrl_addr->herr,erreur); DPcErr;
#endif

DONE;
}







