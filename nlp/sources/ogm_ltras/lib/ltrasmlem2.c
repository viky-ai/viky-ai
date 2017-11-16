/*
 *  The Lem2 module for linguistic transformations
 *  Copyright (c) 2010 Pertimm by Patrick Constant
 *  Dev: May,September 2010
 *  Version 1.0
*/
#include <logltras.h>
#include <logheap.h>
#include <logstm.h>


struct og_ctrl_lem2 {
  void *herr,*hmsg,*hltras; ogmutex_t *hmutex;
  struct og_loginfo cloginfo,*loginfo;
  // struct og_ldi_input ldi_input[1];
  int check_words_in_dictionary;
  int no_dictionary_frequency;
  int is_inherited;
  void *hldi; int hldi_warned;
  struct og_stm_levenshtein_input_param levenshtein_costs[1];
  void *hstm;
  og_heap hba;
  };


struct og_info_lem2 {
  struct og_ltra_module_input *module_input;
  struct og_ctrl_lem2 *ctrl_lem2;
  struct og_ltra_trfs *trfs;
  int Itrf;
  };


void *OgLtrasModuleLem2Init(struct og_ltra_module_param *param)
{
struct og_msg_param cmsg_param,*msg_param=&cmsg_param;
struct og_ctrl_lem2 *ctrl_lem2;
char erreur[DOgErrorSize];
double dfrequency;

IFn(ctrl_lem2=(struct og_ctrl_lem2 *)malloc(sizeof(struct og_ctrl_lem2))) {
  sprintf(erreur,"OgLtrasInit: malloc error on ctrl_lem2");
  OgErr(param->herr,erreur); return(0);
  }
memset(ctrl_lem2,0,sizeof(struct og_ctrl_lem2));

ctrl_lem2->herr = param->herr;
ctrl_lem2->hltras = param->hltras;
ctrl_lem2->hmutex = param->hmutex;
ctrl_lem2->cloginfo = param->loginfo;
ctrl_lem2->loginfo = &ctrl_lem2->cloginfo;

memset(msg_param,0,sizeof(struct og_msg_param));
msg_param->herr=ctrl_lem2->herr;
msg_param->hmutex=ctrl_lem2->hmutex;
msg_param->loginfo.trace = DOgMsgTraceMinimal+DOgMsgTraceMemory;
msg_param->loginfo.where = ctrl_lem2->loginfo->where;
msg_param->module_name="ltra_module_lem2";
IFn(ctrl_lem2->hmsg=OgMsgInit(msg_param)) return(0);
IF(OgMsgTuneInherit(ctrl_lem2->hmsg,param->hmsg)) return(0);

ctrl_lem2->hldi = OgLtrasHldi(ctrl_lem2->hltras);
ctrl_lem2->hstm = OgLtrasHstm(ctrl_lem2->hltras);

IFn(ctrl_lem2->hba=OgHeapInit(ctrl_lem2->hmsg,"lem2_ba",sizeof(unsigned char),0x400)) return(0);

/** needs to be called so that we have the right max frequency **/
OgLtrasHaBase(ctrl_lem2->hltras);
OgLtrasGetFrequencyFromNormalizedFrequency(ctrl_lem2->hltras, 0.9, &dfrequency);
ctrl_lem2->no_dictionary_frequency = (int)dfrequency;

IF(OgLtrasGetLevenshteinCosts(ctrl_lem2->hltras, ctrl_lem2->levenshtein_costs)) return(0);

return((void *)ctrl_lem2);
}




int OgLtrasModuleLem2Flush(void *handle)
{
struct og_ctrl_lem2 *ctrl_lem2 = (struct og_ctrl_lem2 *)handle;
IFE(OgHeapFlush(ctrl_lem2->hba));
IFE(OgMsgFlush(ctrl_lem2->hmsg));
DPcFree(ctrl_lem2);
DONE;
}





int OgLtrasModuleLem2(struct og_ltra_module_input *module_input
    , struct og_ltra_trfs *input, struct og_ltra_trfs **output, ogint64_t *elapsed)
{
  struct og_ctrl_lem2 *ctrl_lem2 = (struct og_ctrl_lem2 *) module_input->handle;

  // BRU disable LDI to avoid deps in viky.ai
  char erreur[DOgErrorSize];
  snprintf(erreur, DOgErrorSize, "BRU disable LDI to avoid deps in viky.ai. Line %d of file %s (function %s)\n", __LINE__, __FILE__, __func__);
  OgErr(ctrl_lem2->herr, erreur);
  DPcErr;
}
