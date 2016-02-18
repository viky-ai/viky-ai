/*
 *	Buffer for all operation handling
 *	Copyright (c) 2006 Pertimm by Patrick Constant
 *	Dev : July 2006
 *	Version 1.0
*/
#include "ogm_http.h"





int HttpAppendBa(ctrl_http,is,s)
struct og_ctrl_http *ctrl_http;
int is; unsigned char *s;
{
if (is<=0) DONE;
IFE(HttpTestReallocBa(ctrl_http,is));
memcpy(ctrl_http->Ba + ctrl_http->BaUsed, s, is);
ctrl_http->BaUsed += is;
DONE;
}




int HttpTestReallocBa(ctrl_http,added_size)
struct og_ctrl_http *ctrl_http;
int added_size;
{
/** +9 because we want to have always extra chars at the end for safety reasons **/
if (ctrl_http->BaUsed + added_size +9 > ctrl_http->BaSize) {
  IFE(HttpReallocBa(ctrl_http,added_size));
  }
DONE;
}




int HttpReallocBa(ctrl_http,added_size)
struct og_ctrl_http *ctrl_http;
int added_size;
{
unsigned a, a1, b; unsigned char *og_buffer;
char erreur[DPcSzErr];

if (ctrl_http->loginfo->trace & DOgHttpTraceMemory) {
  OgMessageLog( DOgMlogInLog, ctrl_http->loginfo->where, 0
              , "HttpReallocBa: max Ba size (%d) reached"
              , ctrl_http->BaSize);
  }
a = ctrl_http->BaSize; a1 = a+added_size; b = a1 + (a1>>2) + 1;
IFn(og_buffer=(unsigned char *)malloc(b*sizeof(unsigned char))) {
  sprintf(erreur,"HttpReallocBa: malloc error on Ba");
  PcErr(-1,erreur); DPcErr;
  }

memcpy( og_buffer, ctrl_http->Ba, a*sizeof(unsigned char));
DPcFree(ctrl_http->Ba); ctrl_http->Ba = og_buffer;
ctrl_http->BaSize = b;

if (ctrl_http->loginfo->trace & DOgHttpTraceMemory) {
  OgMessageLog( DOgMlogInLog, ctrl_http->loginfo->where, 0
              , "HttpReallocBa: new Ba size is %d\n", ctrl_http->BaSize);
  }

#ifdef DOgNoMainBufferReallocation
sprintf(erreur,"HttpReallocBa: BaSize reached (%d)",ctrl_http->BaSize);
PcErr(-1,erreur); DPcErr;
#endif

DONE;
}




