/**
 *  Natural Language Processing library
 *  NLP local thread init/reset/flush
 *
 *  Copyright (c) 2017 Pertimm, by Patrick Constant
 *  Dev : September 2017
 *  Version 1.0
 */
#include "ogm_nlp.h"

PUBLIC(og_nlp_th) OgNlpThreadedInit(og_nlp ctrl_nlp, struct og_nlp_threaded_param *param)
{
  struct og_ctrl_nlp_threaded *ctrl_nlp_th = (struct og_ctrl_nlp_threaded *) malloc(
      sizeof(struct og_ctrl_nlp_threaded));
  IFn(ctrl_nlp_th)
  {
    og_char_buffer erreur[DOgErrorSize];
    sprintf(erreur, "OgNlpRequestInit: malloc error on ctrl_nlp_th");
    OgErr(param->herr, erreur);
    return NULL;
  }

  memset(ctrl_nlp_th, 0, sizeof(struct og_ctrl_nlp_threaded));
  ctrl_nlp_th->ctrl_nlp = ctrl_nlp;
  ctrl_nlp_th->herr = param->herr;
  ctrl_nlp_th->hmutex = param->hmutex;
  memcpy(ctrl_nlp_th->loginfo, &param->loginfo, sizeof(struct og_loginfo));
  snprintf(ctrl_nlp_th->name, DPcPathSize, "%s", param->name);

  //Only for debugging
  //ctrl_nlp_th->loginfo->trace = 0xffff;

  og_char_buffer nlpc_name[DPcPathSize];

  struct og_msg_param msg_param[1];
  memset(msg_param, 0, sizeof(struct og_msg_param));
  msg_param->herr = ctrl_nlp_th->herr;
  msg_param->hmutex = ctrl_nlp_th->hmutex;
  msg_param->loginfo.trace = DOgMsgTraceMinimal + DOgMsgTraceMemory;
  msg_param->loginfo.where = ctrl_nlp_th->loginfo->where;

  snprintf(nlpc_name, DPcPathSize, "%s_msg", param->name);
  msg_param->module_name = nlpc_name;
  IFn(ctrl_nlp_th->hmsg = OgMsgInit(msg_param)) return (0);
  IF(OgMsgTuneInherit(ctrl_nlp_th->hmsg, param->hmsg)) return (0);

  // interpret specific memory
  IF(NlpInterpretInit(ctrl_nlp_th, param))
  {
    og_char_buffer erreur[DOgErrorSize];
    sprintf(erreur, "OgNlpRequestInit: error on OgNlpInterpretInit");
    OgErr(ctrl_nlp_th->herr, erreur);
    return NULL;
  }

  g_queue_init(ctrl_nlp_th->package_in_used);
  g_queue_init(ctrl_nlp_th->sorted_request_expressions);

  ctrl_nlp_th->timeout_in = nlp_timeout_in_NONE;

  IF(NlpJsInit(ctrl_nlp_th))
  {
    NlpThrowErrorTh(ctrl_nlp_th, "OgNlpThreadedInit failed on NlpJsInit");
    return NULL;
  }

  IF(NlpLtracInit(ctrl_nlp_th)) return NULL;
  IF(NlpLtrasInit(ctrl_nlp_th)) return NULL;

  IF(NlpLtracEntityPrepareInit(ctrl_nlp_th, param->name)) return NULL;

  struct og_stm_param stm_param[1];
  memset(stm_param,0,sizeof(struct og_stm_param));
  stm_param->herr=ctrl_nlp_th->herr;
  stm_param->hmsg=ctrl_nlp_th->hmsg;
  stm_param->hmutex=ctrl_nlp_th->hmutex;
  stm_param->loginfo.trace = DOgStmTraceMinimal+DOgStmTraceMemory;
  stm_param->loginfo.where = ctrl_nlp_th->loginfo->where;
  strcpy(stm_param->WorkingDirectory,ctrl_nlp_th->ctrl_nlp->WorkingDirectory);
  IFn(ctrl_nlp_th->hstm=OgStmInit(stm_param)) return(0);
  IF(OgStmInitDefaultCosts(ctrl_nlp_th->hstm,ctrl_nlp_th->levenshtein_costs)) return(0);
  ctrl_nlp_th->levenshtein_costs->punctuation_cost = 0.01;

  return ctrl_nlp_th;

}

PUBLIC(og_status) OgNlpThreadedReset(og_nlp_th ctrl_nlp_th)
{
  IFE(OgNlpThreadedResetKeepJsonAnswer(ctrl_nlp_th));

  json_decrefp(&ctrl_nlp_th->json_answer);
  json_decrefp(&ctrl_nlp_th->json_answer_error);

  DONE;
}

og_status OgNlpThreadedResetKeepJsonAnswer(og_nlp_th ctrl_nlp_th)
{
  ctrl_nlp_th->timeout_in = nlp_timeout_in_NONE;

  IFE(OgNlpSynchroUnLockAll(ctrl_nlp_th));

  // free current package beeing created
  NlpPackageDestroyIfNotUsed(ctrl_nlp_th->package_in_progress);
  ctrl_nlp_th->package_in_progress = NULL;

  IFE(NlpInterpretReset(ctrl_nlp_th));

  IFE(NlpPackageMarkAllInUsedAsUnused(ctrl_nlp_th));

  IFE(NlpJsReset(ctrl_nlp_th));

  DONE;
}

PUBLIC(og_status) OgNlpThreadedFlush(og_nlp_th ctrl_nlp_th)
{
  IFE(NlpInterpretFlush(ctrl_nlp_th));

  IFE(NlpJsFlush(ctrl_nlp_th));

  IFE(NlpLtracFlush(ctrl_nlp_th));
  IFE(NlpLtrasFlush(ctrl_nlp_th));

  IFE(NlpLtracEntityPrepareFlush(ctrl_nlp_th));

  IFE(OgStmFlush(ctrl_nlp_th->hstm));

  IFE(OgMsgFlush(ctrl_nlp_th->hmsg));

  DPcFree(ctrl_nlp_th);

  DONE;
}


PUBLIC(og_status) OgNlpThreadedGetCustomError(og_nlp_th ctrl_nlp_th, json_t **json_answer_error)
{
  if (ctrl_nlp_th->json_answer_error)
  {
    if (json_answer_error)
    {
      *json_answer_error = ctrl_nlp_th->json_answer_error;
      json_incref(*json_answer_error);
    }

    json_decrefp(&ctrl_nlp_th->json_answer_error);
  }
  else
  {
    if (json_answer_error)
    {
      *json_answer_error = NULL;
    }
  }

  DONE;
}

