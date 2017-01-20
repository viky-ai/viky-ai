/*
 *  Initialisation functions for Linguistic Trf search library
 *  Copyright (c) 2010 Pertimm by Patrick Constant
 *  Dev : January 2010
 *  Version 1.0
*/
#include "ogm_ltrap.h"




PUBLIC(void *) OgLtrapInit(param)
struct og_ltrap_param *param;
{
struct og_msg_param cmsg_param,*msg_param=&cmsg_param;
struct og_ctrl_ltrap *ctrl_ltrap;
char erreur[DOgErrorSize];
int size;

IFn(ctrl_ltrap=(struct og_ctrl_ltrap *)malloc(sizeof(struct og_ctrl_ltrap))) {
  sprintf(erreur,"OgLtrapInit: malloc error on ctrl_ltrap");
  OgErr(param->herr,erreur); return(0);
  }
memset(ctrl_ltrap,0,sizeof(struct og_ctrl_ltrap));

ctrl_ltrap->herr = param->herr;
ctrl_ltrap->hmutex = param->hmutex;
ctrl_ltrap->cloginfo = param->loginfo;
ctrl_ltrap->loginfo = &ctrl_ltrap->cloginfo;
//ctrl_ltrap->loginfo->trace=0xfff;

strcpy(ctrl_ltrap->WorkingDirectory,param->WorkingDirectory);
strcpy(ctrl_ltrap->configuration_file,param->configuration_file);
IFn(ctrl_ltrap->configuration_file[0]) {
  strcpy(ctrl_ltrap->configuration_file,DOgLtrapModuleConfiguration);
  }

memset(msg_param,0,sizeof(struct og_msg_param));
msg_param->herr=ctrl_ltrap->herr;
msg_param->hmutex=ctrl_ltrap->hmutex;
msg_param->loginfo.trace = DOgMsgTraceMinimal+DOgMsgTraceMemory;
msg_param->loginfo.where = ctrl_ltrap->loginfo->where;
msg_param->module_name="ogm_ltrap";
IFn(ctrl_ltrap->hmsg=OgMsgInit(msg_param)) return(0);
IF(OgMsgTuneInherit(ctrl_ltrap->hmsg,param->hmsg)) return(0);

ctrl_ltrap->SpanNumber = DOgLtrapSpanNumber;
size = ctrl_ltrap->SpanNumber*sizeof(struct span);
IFn(ctrl_ltrap->Span=(struct span *)malloc(size)) {
  sprintf(erreur,"OgLtrapInit: malloc error on Span (%d bytes)",size);
  OgErr(ctrl_ltrap->herr,erreur); return(0);
  }

ctrl_ltrap->SpansolNumber = DOgLtrapSpansolNumber;
size = ctrl_ltrap->SpansolNumber*sizeof(struct spansol);
IFn(ctrl_ltrap->Spansol=(struct spansol *)malloc(size)) {
  sprintf(erreur,"OgLtrapInit: malloc error on Spansol (%d bytes)",size);
  OgErr(ctrl_ltrap->herr,erreur); return(0);
  }

ctrl_ltrap->SequenceNumber = DOgLtrapSequenceNumber;
size = ctrl_ltrap->SequenceNumber*sizeof(struct sequence);
IFn(ctrl_ltrap->Sequence=(struct sequence *)malloc(size)) {
  sprintf(erreur,"OgLtrapInit: malloc error on Sequence (%d bytes)",size);
  OgErr(ctrl_ltrap->herr,erreur); return(0);
  }

ctrl_ltrap->SolNumber = DOgLtrapSolNumber;
size = ctrl_ltrap->SolNumber*sizeof(struct sol);
IFn(ctrl_ltrap->Sol=(struct sol *)malloc(size)) {
  sprintf(erreur,"OgLtrapInit: malloc error on Sol (%d bytes)",size);
  OgErr(ctrl_ltrap->herr,erreur); return(0);
  }

ctrl_ltrap->BaSize = DOgLtrapBaSize;
size = ctrl_ltrap->BaSize*sizeof(unsigned char);
IFn(ctrl_ltrap->Ba=(unsigned char *)malloc(size)) {
  sprintf(erreur,"OgLtrasInit: malloc error on Ba (%d bytes)",size);
  OgErr(ctrl_ltrap->herr,erreur); return(0);
  }

ctrl_ltrap->max_word_frequency = param->max_word_frequency;
ctrl_ltrap->max_word_frequency_log10=log10(ctrl_ltrap->max_word_frequency);

IFn(param->ha_base) {

  struct og_aut_param aut_param[1];
  memset(aut_param, 0, sizeof(struct og_aut_param));

  aut_param->herr=ctrl_ltrap->herr;
  aut_param->hmutex=ctrl_ltrap->hmutex;
  aut_param->loginfo.trace = DOgAutTraceMinimal+DOgAutTraceMemory;
  aut_param->loginfo.where = ctrl_ltrap->loginfo->where;

  sprintf(aut_param->name,"ltra_base");
  IFn(ctrl_ltrap->ha_base=OgAutInit(aut_param)) return(0);
    unsigned char* ha_name = "ling/ltra_base.auf";
    if (OgFileExists(ha_name))
    {
      IF(OgAufRead(ctrl_ltrap->ha_base, ha_name)) return(0);
    }
  }
else ctrl_ltrap->ha_base = param->ha_base;

IFn(param->ha_swap) {

  struct og_aut_param aut_param[1];
  memset(aut_param, 0, sizeof(struct og_aut_param));

  aut_param->herr=ctrl_ltrap->herr;
  aut_param->hmutex=ctrl_ltrap->hmutex;
  aut_param->loginfo.trace = DOgAutTraceMinimal+DOgAutTraceMemory;
  aut_param->loginfo.where = ctrl_ltrap->loginfo->where;

  sprintf(aut_param->name,"ltra_swap");
  IFn(ctrl_ltrap->ha_swap=OgAutInit(aut_param)) return(0);
    unsigned char* ha_name = "ling/ltra_swap.auf";
    if (OgFileExists(ha_name))
    {
      IF(OgAufRead(ctrl_ltrap->ha_swap, ha_name)) return(0);
    }
  }
else ctrl_ltrap->ha_swap = param->ha_swap;

IFn(param->ha_phon) {

  struct og_aut_param aut_param[1];
  memset(aut_param, 0, sizeof(struct og_aut_param));

  aut_param->herr=ctrl_ltrap->herr;
  aut_param->hmutex=ctrl_ltrap->hmutex;
  aut_param->loginfo.trace = DOgAutTraceMinimal+DOgAutTraceMemory;
  aut_param->loginfo.where = ctrl_ltrap->loginfo->where;

  sprintf(aut_param->name,"ltra_phon");
  IFn(ctrl_ltrap->ha_phon=OgAutInit(aut_param)) return(0);
    unsigned char* ha_name = "ling/ltra_phon.auf";
    if (OgFileExists(ha_name))
    {
      IF(OgAufRead(ctrl_ltrap->ha_phon, ha_name)) return(0);
    }
  }
else ctrl_ltrap->ha_phon = param->ha_phon;

ctrl_ltrap->max_small_word_length = param->max_small_word_length;
ctrl_ltrap->ha_small_words = param->ha_small_words;
ctrl_ltrap->ha_false = param->ha_false;

return((void *)ctrl_ltrap);
}




int LtrapReset(struct og_ctrl_ltrap *ctrl_ltrap)
{
ctrl_ltrap->SpanUsed=0;
ctrl_ltrap->SpansolUsed=0;
ctrl_ltrap->SequenceUsed=0;
ctrl_ltrap->SolUsed=0;
ctrl_ltrap->BaUsed=0;
DONE;
}




PUBLIC(int) OgLtrapFlush(handle)
void *handle;
{
struct og_ctrl_ltrap *ctrl_ltrap = (struct og_ctrl_ltrap *)handle;
IFn(handle) DONE;

IFE(OgMsgFlush(ctrl_ltrap->hmsg));
DPcFree(ctrl_ltrap->Span);
DPcFree(ctrl_ltrap->Spansol);
DPcFree(ctrl_ltrap->Sequence);
DPcFree(ctrl_ltrap->Sol);
DPcFree(ctrl_ltrap->Ba);
DPcFree(ctrl_ltrap);
DONE;
}



