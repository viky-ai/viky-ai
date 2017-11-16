/*
 *  The Tra module for linguistic transformations
 *  Copyright (c) 2010 Pertimm by Patrick Constant
 *  Dev: May,September 2010
 *  Version 1.0
*/
#include <logltras.h>
#include <logstm.h>


struct og_ctrl_tra {
  void *herr,*hmsg,*hltras; ogmutex_t *hmutex;
  struct og_loginfo cloginfo,*loginfo;
  // struct og_ldi_input ldi_input[1];
  int check_words_in_dictionary;
  int no_dictionary_frequency;
  int is_inherited;
  void *hldi; int hldi_warned;
  struct og_stm_levenshtein_input_param levenshtein_costs[1];
  void *hstm;
  };


struct og_info_tra {
  struct og_ltra_module_input *module_input;
  struct og_ctrl_tra *ctrl_tra;
  struct og_ltra_trfs *trfs;
  int Itrf;
  };


struct og_tra_translation {
  int language_code;
  int target_language_code;
  };




void *OgLtrasModuleTraInit(struct og_ltra_module_param *param)
{
struct og_msg_param cmsg_param,*msg_param=&cmsg_param;
struct og_ctrl_tra *ctrl_tra;
char erreur[DOgErrorSize];
double dfrequency;

IFn(ctrl_tra=(struct og_ctrl_tra *)malloc(sizeof(struct og_ctrl_tra))) {
  sprintf(erreur,"OgLtrasModuleTraInit: malloc error on ctrl_tra");
  OgErr(param->herr,erreur); return(0);
  }
memset(ctrl_tra,0,sizeof(struct og_ctrl_tra));

ctrl_tra->herr = param->herr;
ctrl_tra->hltras = param->hltras;
ctrl_tra->hmutex = param->hmutex;
ctrl_tra->cloginfo = param->loginfo;
ctrl_tra->loginfo = &ctrl_tra->cloginfo;

memset(msg_param,0,sizeof(struct og_msg_param));
msg_param->herr=ctrl_tra->herr;
msg_param->hmutex=ctrl_tra->hmutex;
msg_param->loginfo.trace = DOgMsgTraceMinimal+DOgMsgTraceMemory;
msg_param->loginfo.where = ctrl_tra->loginfo->where;
msg_param->module_name="ltra_module_tra";
IFn(ctrl_tra->hmsg=OgMsgInit(msg_param)) return(0);
IF(OgMsgTuneInherit(ctrl_tra->hmsg,param->hmsg)) return(0);

ctrl_tra->hldi = OgLtrasHldi(ctrl_tra->hltras);
ctrl_tra->hstm = OgLtrasHstm(ctrl_tra->hltras);

/** needs to be called so that we have the right max frequency **/
OgLtrasHaBase(ctrl_tra->hltras);
OgLtrasGetFrequencyFromNormalizedFrequency(ctrl_tra->hltras, 0.9, &dfrequency);
ctrl_tra->no_dictionary_frequency = (int)dfrequency;

IF(OgLtrasGetLevenshteinCosts(ctrl_tra->hltras, ctrl_tra->levenshtein_costs)) return(0);

return((void *)ctrl_tra);
}




int OgLtrasModuleTraFlush(void *handle)
{
struct og_ctrl_tra *ctrl_tra = (struct og_ctrl_tra *)handle;
IFE(OgMsgFlush(ctrl_tra->hmsg));
DPcFree(ctrl_tra);
DONE;
}





int OgLtrasModuleTra(struct og_ltra_module_input *module_input
    , struct og_ltra_trfs *input, struct og_ltra_trfs **output, ogint64_t *elapsed)
{
  struct og_ctrl_tra *ctrl_tra = (struct og_ctrl_tra *) module_input->handle;

  // BRU disable LDI to avoid deps in viky.ai
  char erreur[DOgErrorSize];
  snprintf(erreur, DOgErrorSize, "BRU disable LDI to avoid deps in viky.ai. Line %d of file %s (function %s)\n", __LINE__, __FILE__, __func__);
  OgErr(ctrl_tra->herr, erreur);
  DPcErr;
}
