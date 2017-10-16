/*
 *	Buffer for all operation handling
 *	Copyright (c) 2005 Pertimm by Patrick Constant
 *	Dev : April 2005
 *	Version 1.0
*/
#include "ogm_zlib.h"





int OgZlibAppendBa(ctrl_zlib,is,s)
struct og_ctrl_zlib *ctrl_zlib;
int is; unsigned char *s;
{
if (is<=0) DONE;
IFE(OgZlibTestReallocBa(ctrl_zlib,is));
memcpy(ctrl_zlib->Ba + ctrl_zlib->BaUsed, s, is);
ctrl_zlib->BaUsed += is;
DONE;
}




int OgZlibTestReallocBa(ctrl_zlib,added_size)
struct og_ctrl_zlib *ctrl_zlib;
int added_size;
{
/** +9 because we want to have always extra chars at the end for safety reasons **/
if (ctrl_zlib->BaUsed + added_size +9 > ctrl_zlib->BaSize) {
  IFE(OgZlibReallocBa(ctrl_zlib,added_size));
  }
DONE;
}




int OgZlibReallocBa(ctrl_zlib,added_size)
struct og_ctrl_zlib *ctrl_zlib;
int added_size;
{
unsigned a, a1, b; unsigned char *og_buffer;
char erreur[DOgErrorSize];

if (ctrl_zlib->loginfo->trace & DOgZlibTraceMemory) {
  OgThrMessageLog(ctrl_zlib->hmutex, DOgMlogInLog, ctrl_zlib->loginfo->where, 0
              , "OgZlibReallocBa from ogm_url: max Ba size (%d) reached, added_size=%d"
              , ctrl_zlib->BaSize,added_size);
  }
a = ctrl_zlib->BaSize; a1 = a+added_size; b = a1 + (a1>>2) + 1;
IFn(og_buffer=(unsigned char *)malloc(b*sizeof(unsigned char))) {
  sprintf(erreur,"OgZlibReallocBa from ogm_url: malloc error on Ba");
  OgErr(ctrl_zlib->herr,erreur); DPcErr;
  }

memcpy( og_buffer, ctrl_zlib->Ba, a*sizeof(unsigned char));
DPcFree(ctrl_zlib->Ba); ctrl_zlib->Ba = og_buffer;
ctrl_zlib->BaSize = b;

if (ctrl_zlib->loginfo->trace & DOgZlibTraceMemory) {
  OgThrMessageLog(ctrl_zlib->hmutex, DOgMlogInLog, ctrl_zlib->loginfo->where, 0
              , "OgZlibReallocBa from ogm_url: new Ba size is %d", ctrl_zlib->BaSize);
  }

#ifdef DOgNoMainBufferReallocation
sprintf(erreur,"OgZlibReallocBa from ogm_url: BaSize reached (%d)",ctrl_zlib->BaSize);
OgErr(ctrl_zlib->herr,erreur); DPcErr;
#endif

DONE;
}







