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
  struct og_ctrl_nlp_threaded *ctrl_nlp_th = (struct og_ctrl_nlp_threaded *) malloc(sizeof(struct og_ctrl_nlp_threaded));
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

  return ctrl_nlp_th;

}

PUBLIC(og_status) OgNlpThreadedReset(og_nlp_th ctrl_nlp_th)
{
  ctrl_nlp_th->timeout_in = nlp_timeout_in_NONE;

  IFE(OgNlpSynchroUnLockAll(ctrl_nlp_th));

  IFE(NlpInterpretReset(ctrl_nlp_th));

  IFE(NlpPackageMarkAllInUsedAsUnused(ctrl_nlp_th));

  json_decrefp(&ctrl_nlp_th->json_answer);

  DONE;
}

PUBLIC(og_status) OgNlpThreadedFlush(og_nlp_th ctrl_nlp_th)
{
  IFE(NlpInterpretFlush(ctrl_nlp_th));

  IFE(OgMsgFlush(ctrl_nlp_th->hmsg));

  DPcFree(ctrl_nlp_th);

  DONE;
}
