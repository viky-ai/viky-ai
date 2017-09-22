/*
 *  Initialization for Natural Language functions
 *  Copyright (c) 2017 by Patrick Constant
 *  Dev : August 2017
 *  Version 1.0
 */
#include "ogm_nlp.h"
#include <stddef.h>

PUBLIC(og_nlp) OgNlpInit(struct og_nlp_param *param)
{
  char erreur[DOgErrorSize];
  struct og_ctrl_nlp *ctrl_nlp = (struct og_ctrl_nlp *) malloc(sizeof(struct og_ctrl_nlp));
  IFn(ctrl_nlp)
  {
    sprintf(erreur, "OgNlpInit: malloc error on ctrl_nlp");
    OgErr(param->herr, erreur);
    return (0);
  }

  memset(ctrl_nlp, 0, sizeof(struct og_ctrl_nlp));
  ctrl_nlp->herr = param->herr;
  ctrl_nlp->hmutex = param->hmutex;
  memcpy(ctrl_nlp->loginfo, &param->loginfo, sizeof(struct og_loginfo));

  struct og_msg_param msg_param[1];
  memset(msg_param, 0, sizeof(struct og_msg_param));
  msg_param->herr = ctrl_nlp->herr;
  msg_param->hmutex = ctrl_nlp->hmutex;
  msg_param->loginfo.trace = DOgMsgTraceMinimal + DOgMsgTraceMemory;
  msg_param->loginfo.where = ctrl_nlp->loginfo->where;
  msg_param->module_name = "nlp";
  IFn(ctrl_nlp->hmsg=OgMsgInit(msg_param)) return (0);
  IF(OgMsgTuneInherit(ctrl_nlp->hmsg,param->hmsg)) return (0);

  return ctrl_nlp;
}

PUBLIC(int) OgNlpFlush(og_nlp handle)
{
  struct og_ctrl_nlp *ctrl_nlp = handle;

  DPcFree(ctrl_nlp);
  DONE;
}

