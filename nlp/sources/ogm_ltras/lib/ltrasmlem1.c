/*
 *  The Lem1 module for linguistic transformations
 *  Copyright (c) 2010 Pertimm by Patrick Constant
 *  Dev: May,September 2010
 *  Version 1.0
*/
#include <logltras.h>
#include <logheap.h>
#include <logstm.h>


struct og_ctrl_lem1 {
  void *herr,*hmsg,*hltras; ogmutex_t *hmutex;
  struct og_loginfo cloginfo,*loginfo;
  // struct og_ldi_input ldi_input[1];
  int check_words_in_dictionary;
  int no_dictionary_frequency;
  int is_inherited;
  void *hldi; int hldi_warned;
  struct og_stm_levenshtein_input_param levenshtein_costs[1];
  void *hstm;
  int compound_prefix_length;
  int compound_prefix_length_activated;
  og_heap hba;
  };


struct og_info_lem1 {
  struct og_ltra_module_input *module_input;
  struct og_ctrl_lem1 *ctrl_lem1;
  struct og_ltra_trfs *trfs;
  int Itrf;
  };


void *OgLtrasModuleLem1Init(struct og_ltra_module_param *param)
{
struct og_msg_param cmsg_param,*msg_param=&cmsg_param;
struct og_ctrl_lem1 *ctrl_lem1;
char erreur[DOgErrorSize];
double dfrequency;

IFn(ctrl_lem1=(struct og_ctrl_lem1 *)malloc(sizeof(struct og_ctrl_lem1))) {
  sprintf(erreur,"OgLtrasModuleLem1Init: malloc error on ctrl_lem1");
  OgErr(param->herr,erreur); return(0);
  }
memset(ctrl_lem1,0,sizeof(struct og_ctrl_lem1));

ctrl_lem1->herr = param->herr;
ctrl_lem1->hltras = param->hltras;
ctrl_lem1->hmutex = param->hmutex;
ctrl_lem1->cloginfo = param->loginfo;
ctrl_lem1->loginfo = &ctrl_lem1->cloginfo;

memset(msg_param,0,sizeof(struct og_msg_param));
msg_param->herr=ctrl_lem1->herr;
msg_param->hmutex=ctrl_lem1->hmutex;
msg_param->loginfo.trace = DOgMsgTraceMinimal+DOgMsgTraceMemory;
msg_param->loginfo.where = ctrl_lem1->loginfo->where;
msg_param->module_name="ltra_module_lem1";
IFn(ctrl_lem1->hmsg=OgMsgInit(msg_param)) return(0);
IF(OgMsgTuneInherit(ctrl_lem1->hmsg,param->hmsg)) return(0);

ctrl_lem1->hldi = OgLtrasHldi(ctrl_lem1->hltras);
ctrl_lem1->hstm = OgLtrasHstm(ctrl_lem1->hltras);

IFn(ctrl_lem1->hba=OgHeapInit(ctrl_lem1->hmsg,"lem1_ba",sizeof(unsigned char),0x400)) return(0);

/** needs to be called so that we have the right max frequency **/
OgLtrasHaBase(ctrl_lem1->hltras);
OgLtrasGetFrequencyFromNormalizedFrequency(ctrl_lem1->hltras, 0.9, &dfrequency);
ctrl_lem1->no_dictionary_frequency = (int)dfrequency;

IF(OgLtrasGetLevenshteinCosts(ctrl_lem1->hltras, ctrl_lem1->levenshtein_costs)) return(0);

return((void *)ctrl_lem1);
}




int OgLtrasModuleLem1Flush(void *handle)
{
struct og_ctrl_lem1 *ctrl_lem1 = (struct og_ctrl_lem1 *)handle;
IFE(OgHeapFlush(ctrl_lem1->hba));
IFE(OgMsgFlush(ctrl_lem1->hmsg));
DPcFree(ctrl_lem1);
DONE;
}





int OgLtrasModuleLem1(struct og_ltra_module_input *module_input
    , struct og_ltra_trfs *input, struct og_ltra_trfs **output, ogint64_t *elapsed)
{
  struct og_ctrl_lem1 *ctrl_lem1 = (struct og_ctrl_lem1 *) module_input->handle;

  // BRU disable LDI to avoid deps in viky.ai
  char erreur[DOgErrorSize];
  snprintf(erreur, DOgErrorSize, "BRU disable LDI to avoid deps in viky.ai. Line %d of file %s (function %s)\n", __LINE__, __FILE__, __func__);
  OgErr(ctrl_lem1->herr, erreur);
  DPcErr;
}
