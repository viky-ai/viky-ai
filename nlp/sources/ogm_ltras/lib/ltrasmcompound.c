/*
 *  The Compound module for linguistic transformations
 *  Copyright (c) 2017 Pertimm by Caroline Collet
 *  Dev: January 2017
 *  Version 1.0
 */

#include <logltras.h>
#include <logltrap.h>

#define DOgLtrasCompoundMaxDictionariesNumber 10
#define DOgLtrasCompoundMaxNbCandidates 100
#define DOgLtrasCompoundMaxNbSolutions 1

struct og_ctrl_compound
{
  void *herr;
  void *hmsg;
  ogmutex_t *hmutex;
  struct og_loginfo loginfo[1];

  void *hltras;
  void *hstm;
  void *hldi;
  void *hltrap;

  int new_words_length;
  unsigned char new_words[DPcPathSize];
  struct og_ltra_add_trf_input tinput[1];

  int Itrf;
  int iuni;
  unsigned char *uni;
  struct og_ltra_trfs *trfs;
  int max_nb_candidates;
  int max_nb_solutions;
  double cut_cost;

  og_bool dict_loaded_msg[DOgLangMax];

  struct og_stm_levenshtein_input_param levenshtein_costs[1];
};


void *OgLtrasModuleCompoundInit(struct og_ltra_module_param *param)
{
  struct og_ctrl_compound *ctrl_compound = (struct og_ctrl_compound *) malloc(sizeof(struct og_ctrl_compound));
  IFn(ctrl_compound)
  {
    char erreur[DOgErrorSize];
    sprintf(erreur, "OgLtrasInit: malloc error on ctrl_compound");
    OgErr(param->herr, erreur);
    return (0);
  }
  memset(ctrl_compound, 0, sizeof(struct og_ctrl_compound));

  ctrl_compound->herr = param->herr;
  ctrl_compound->hltras = param->hltras;
  ctrl_compound->hmutex = param->hmutex;
  ctrl_compound->loginfo[0] = param->loginfo;

  struct og_msg_param msg_param[1];
  memset(msg_param, 0, sizeof(struct og_msg_param));
  msg_param->herr = ctrl_compound->herr;
  msg_param->hmutex = ctrl_compound->hmutex;
  msg_param->loginfo.trace = DOgMsgTraceMinimal + DOgMsgTraceMemory;
  msg_param->loginfo.where = ctrl_compound->loginfo->where;
  msg_param->module_name = "ltra_module_compound";
  IFn(ctrl_compound->hmsg = OgMsgInit(msg_param)) return (0);
  IF(OgMsgTuneInherit(ctrl_compound->hmsg, param->hmsg)) return (0);

  struct og_aut_param aut_param[1];
  memset(aut_param, 0, sizeof(struct og_aut_param));
  aut_param->herr = ctrl_compound->herr;
  aut_param->hmutex = ctrl_compound->hmutex;
  aut_param->loginfo.trace = DOgAutTraceMinimal + DOgAutTraceMemory;
  aut_param->loginfo.where = ctrl_compound->loginfo->where;
  aut_param->state_number = 0x0;
  sprintf(aut_param->name, "ltras_module_compound");

  struct og_ltrap_param ltrap_param[1];
  memset(ltrap_param, 0, sizeof(struct og_ltrap_param));
  ltrap_param->herr = ctrl_compound->herr;
  ltrap_param->hmsg = ctrl_compound->hmsg;
  ltrap_param->hmutex = ctrl_compound->hmutex;
  ltrap_param->loginfo.trace = DOgLtrapTraceMinimal + DOgLtrapTraceMemory;
  if (ctrl_compound->loginfo->trace & DOgLtrasTraceModuleCompound)
  {
    ltrap_param->loginfo.trace |= DOgLtrapTraceAdd + DOgLtrapTraceBuild + DOgLtrapTraceSort + DOgLtrapTraceSend;
  }
  ltrap_param->loginfo.where = ctrl_compound->loginfo->where;
  ltrap_param->max_word_frequency = OgLtrasMaxWordFrequency(ctrl_compound->hltras);
  ltrap_param->ha_false = OgLtrasHaFalse(ctrl_compound->hltras);

  ltrap_param->ha_base = OgLtrasHaBase(ctrl_compound->hltras);
  ltrap_param->ha_swap = OgLtrasHaSwap(ctrl_compound->hltras);
  ltrap_param->ha_phon = OgLtrasHaPhon(ctrl_compound->hltras);

  IFn(ctrl_compound->hltrap = OgLtrapInit(ltrap_param)) return (0);

  ctrl_compound->hldi = OgLtrasHldi(ctrl_compound->hltras);

  ctrl_compound->hstm = OgLtrasHstm(ctrl_compound->hltras);
  IF(OgLtrasGetLevenshteinCosts(ctrl_compound->hltras, ctrl_compound->levenshtein_costs)) return (0);

  ctrl_compound->max_nb_candidates = DOgLtrasCompoundMaxNbCandidates;
  ctrl_compound->max_nb_solutions = DOgLtrasCompoundMaxNbSolutions;

  IF(OgStmGetSpaceCost(ctrl_compound->hstm, 0, TRUE, &ctrl_compound->cut_cost)) return (0);

  return ctrl_compound;
}

int OgLtrasModuleCompoundFlush(void *handle)
{
  struct og_ctrl_compound *ctrl_compound = (struct og_ctrl_compound *) handle;
  IFE(OgLtrapFlush(ctrl_compound->hltrap));

  IFE(OgMsgFlush(ctrl_compound->hmsg));
  DPcFree(ctrl_compound);
  DONE;
}

/*
 * the parameter max_nb_letters can only be zero or 1
 */
int OgLtrasModuleCompound(struct og_ltra_module_input *module_input, struct og_ltra_trfs *input,
    struct og_ltra_trfs **output, ogint64_t *elapsed)
{
  struct og_ctrl_compound *ctrl_compound = (struct og_ctrl_compound *) module_input->handle;

  // BRU disable LDI to avoid deps in viky.ai
  char erreur[DOgErrorSize];
  snprintf(erreur, DOgErrorSize, "BRU disable LDI to avoid deps in viky.ai. Line %d of file %s (function %s)\n", __LINE__, __FILE__, __func__);
  OgErr(ctrl_compound->herr, erreur);
  DPcErr;

}
