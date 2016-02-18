/*
 *	Function for Phonet
 *  Copyright (c) 2006 Pertimm by P.Constant,G.Logerot and L.Rigouste
 *  Dev : December 2006
 *  Version 1.0
*/
#include "ogm_pho.h"

STATICF(int) PhoTestReallocBa(pr_(struct og_ctrl_pho *) pr(int));
STATICF(int) PhoReallocBa(pr_(struct og_ctrl_pho *) pr(int));




int PhoAppendBa(ctrl_pho,is,s)
struct og_ctrl_pho *ctrl_pho;
int is; unsigned char *s;
{
if (is<=0) DONE;
IFE(PhoTestReallocBa(ctrl_pho,is));
memcpy(ctrl_pho->Ba + ctrl_pho->BaUsed, s, is);
ctrl_pho->BaUsed += is;
DONE;
}


STATICF(int) PhoTestReallocBa(ctrl_pho,added_size)
struct og_ctrl_pho *ctrl_pho;
int added_size;
{
/** +9 because we want to have always extra chars at the end for safety reasons **/
if (ctrl_pho->BaUsed + added_size +9 > ctrl_pho->BaSize) {
  IFE(PhoReallocBa(ctrl_pho,added_size));
  }
DONE;
}

STATICF(int) PhoReallocBa(ctrl_pho,added_size)
struct og_ctrl_pho *ctrl_pho;
int added_size;
{
unsigned a, a1, b; unsigned char *og_buffer;
char erreur[DPcSzErr];

if (ctrl_pho->loginfo->trace & DOgPhoTraceMemory) {
  OgMsg(ctrl_pho->hmsg,"",DOgMsgDestInLog,"PhoReallocBa: max Ba size (%d) reached", ctrl_pho->BaSize);
  }
a = ctrl_pho->BaSize; a1 = a+added_size; b = a1 + (a1>>2) + 1;
IFn(og_buffer=(unsigned char *)malloc(b*sizeof(unsigned char))) {
  sprintf(erreur,"PhoReallocBa: malloc error on Ba");
  PcErr(-1,erreur); DPcErr;
  }

memcpy( og_buffer, ctrl_pho->Ba, a*sizeof(unsigned char));
DPcFree(ctrl_pho->Ba); ctrl_pho->Ba = og_buffer;
ctrl_pho->BaSize = b;

if (ctrl_pho->loginfo->trace & DOgPhoTraceMemory) {
  OgMsg(ctrl_pho->hmsg,"",DOgMsgDestInLog,"PhoReallocBa: new Ba size is %d\n", ctrl_pho->BaSize);
  }

#ifdef DOgNoMainBufferReallocation
sprintf(erreur,"PhoReallocBa: BaSize reached (%d)",ctrl_pho->BaSize);
PcErr(-1,erreur); DPcErr;
#endif

DONE;
}


STATICF(int) PhoTestReallocBaClass(pr_(struct og_ctrl_pho *) pr(int));
STATICF(int) PhoReallocBaClass(pr_(struct og_ctrl_pho *) pr(int));




int PhoAppendBaClass(ctrl_pho,is,s)
struct og_ctrl_pho *ctrl_pho;
int is; unsigned char *s;
{
if (is<=0) DONE;
IFE(PhoTestReallocBaClass(ctrl_pho,is));
memcpy(ctrl_pho->BaClass + ctrl_pho->BaClassUsed, s, is);
ctrl_pho->BaClassUsed += is;
DONE;
}


STATICF(int) PhoTestReallocBaClass(ctrl_pho,added_size)
struct og_ctrl_pho *ctrl_pho;
int added_size;
{
/** +9 because we want to have always extra chars at the end for safety reasons **/
if (ctrl_pho->BaClassUsed + added_size +9 > ctrl_pho->BaClassSize) {
  IFE(PhoReallocBaClass(ctrl_pho,added_size));
  }
DONE;
}

STATICF(int) PhoReallocBaClass(ctrl_pho,added_size)
struct og_ctrl_pho *ctrl_pho;
int added_size;
{
unsigned a, a1, b; unsigned char *og_buffer;
char erreur[DPcSzErr];

if (ctrl_pho->loginfo->trace & DOgPhoTraceMemory) {
  OgMsg(ctrl_pho->hmsg,"",DOgMsgDestInLog,"PhoReallocBaClass: max BaClass size (%d) reached", ctrl_pho->BaClassSize);
  }
a = ctrl_pho->BaClassSize; a1 = a+added_size; b = a1 + (a1>>2) + 1;
IFn(og_buffer=(unsigned char *)malloc(b*sizeof(unsigned char))) {
  sprintf(erreur,"PhoReallocBaClass: malloc error on BaClass");
  PcErr(-1,erreur); DPcErr;
  }

memcpy( og_buffer, ctrl_pho->BaClass, a*sizeof(unsigned char));
DPcFree(ctrl_pho->BaClass); ctrl_pho->BaClass = og_buffer;
ctrl_pho->BaClassSize = b;

if (ctrl_pho->loginfo->trace & DOgPhoTraceMemory) {
  OgMsg(ctrl_pho->hmsg,"",DOgMsgDestInLog,"PhoReallocBaClass: new BaClass size is %d\n", ctrl_pho->BaClassSize);
  }

#ifdef DOgNoMainBufferReallocation
sprintf(erreur,"PhoReallocBaClass: BaClassSize reached (%d)",ctrl_pho->BaClassSize);
PcErr(-1,erreur); DPcErr;
#endif

DONE;
}
