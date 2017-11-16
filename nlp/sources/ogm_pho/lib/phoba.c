/*
 *  Function for Phonet
 *  Copyright (c) 2006 Pertimm by P.Constant,G.Logerot and L.Rigouste
 *  Dev : December 2006
 *  Version 1.0
*/
#include "ogm_pho.h"

static og_status PhoTestReallocBa(struct lang_context *lang_context, int added_size);
static og_status PhoReallocBa(struct lang_context *lang_context, int added_size);
static og_status PhoTestReallocBaClass(struct lang_context *lang_context, int added_size);
static og_status PhoReallocBaClass(struct lang_context *lang_context, int added_size);

og_status PhoAppendBa(struct lang_context *lang_context, int is, unsigned char *s)
{
  if (is <= 0) DONE;
  IFE(PhoTestReallocBa(lang_context, is));
  memcpy(lang_context->Ba + lang_context->BaUsed, s, is);
  lang_context->BaUsed += is;
  DONE;
}


static og_status PhoTestReallocBa(struct lang_context *lang_context, int added_size)
{
  /** +9 because we want to have always extra chars at the end for safety reasons **/
  if (lang_context->BaUsed + added_size + 9 > lang_context->BaSize)
  {
    IFE(PhoReallocBa(lang_context, added_size));
  }
  DONE;
}

static og_status PhoReallocBa(struct lang_context *lang_context, int added_size)
{
  struct og_ctrl_pho *ctrl_pho = lang_context->ctrl_pho;

  if (ctrl_pho->loginfo->trace & DOgPhoTraceMemory)
  {
    OgMsg(ctrl_pho->hmsg, "", DOgMsgDestInLog, "PhoReallocBa: max Ba size (%d) reached", lang_context->BaSize);
  }
  unsigned a = lang_context->BaSize;
  unsigned a1 = a + added_size;
  unsigned b = a1 + (a1 >> 2) + 1;

  unsigned char *og_buffer=(unsigned char *)malloc(b*sizeof(unsigned char));
  IFn(og_buffer)
  {
    char erreur[DPcSzErr];
    sprintf(erreur, "PhoReallocBa: malloc error on Ba");
    PcErr(-1, erreur);
    DPcErr;
  }

  memcpy(og_buffer, lang_context->Ba, a * sizeof(unsigned char));
  DPcFree(lang_context->Ba);
  lang_context->Ba = og_buffer;
  lang_context->BaSize = b;

  if (ctrl_pho->loginfo->trace & DOgPhoTraceMemory)
  {
    OgMsg(ctrl_pho->hmsg, "", DOgMsgDestInLog, "PhoReallocBa: new Ba size is %d\n", lang_context->BaSize);
  }

  #ifdef DOgNoMainBufferReallocation
    sprintf(erreur,"PhoReallocBa: BaSize reached (%d)",lang_context->BaSize);
    PcErr(-1,erreur); DPcErr;
  #endif

  DONE;
}


og_status PhoAppendBaClass(struct lang_context *lang_context, int is, unsigned char *s)
{
  if (is <= 0) DONE;
  IFE(PhoTestReallocBaClass(lang_context, is));
  memcpy(lang_context->BaClass + lang_context->BaClassUsed, s, is);
  lang_context->BaClassUsed += is;
  DONE;
}


static og_status PhoTestReallocBaClass(struct lang_context *lang_context, int added_size)
{
  /** +9 because we want to have always extra chars at the end for safety reasons **/
  if (lang_context->BaClassUsed + added_size + 9 > lang_context->BaClassSize)
  {
    IFE(PhoReallocBaClass(lang_context, added_size));
  }
  DONE;
}

static og_status PhoReallocBaClass(struct lang_context *lang_context, int added_size)
{
  struct og_ctrl_pho *ctrl_pho = lang_context->ctrl_pho;

  if (ctrl_pho->loginfo->trace & DOgPhoTraceMemory)
  {
    OgMsg(ctrl_pho->hmsg, "", DOgMsgDestInLog, "PhoReallocBaClass: max BaClass size (%d) reached",
        lang_context->BaClassSize);
  }
  unsigned a = lang_context->BaClassSize;
  unsigned a1 = a + added_size;
  unsigned b = a1 + (a1 >> 2) + 1;

  unsigned char *og_buffer=(unsigned char *)malloc(b*sizeof(unsigned char));
  IFn(og_buffer)
  {
    char erreur[DPcSzErr];
    sprintf(erreur, "PhoReallocBaClass: malloc error on BaClass");
    PcErr(-1, erreur);
    DPcErr;
  }

  memcpy(og_buffer, lang_context->BaClass, a * sizeof(unsigned char));
  DPcFree(lang_context->BaClass);
  lang_context->BaClass = og_buffer;
  lang_context->BaClassSize = b;

  if (ctrl_pho->loginfo->trace & DOgPhoTraceMemory)
  {
    OgMsg(ctrl_pho->hmsg, "", DOgMsgDestInLog, "PhoReallocBaClass: new BaClass size is %d\n", lang_context->BaClassSize);
  }

  #ifdef DOgNoMainBufferReallocation
    sprintf(erreur,"PhoReallocBaClass: BaClassSize reached (%d)",lang_context->BaClassSize);
    PcErr(-1,erreur); DPcErr;
  #endif

  DONE;
}
