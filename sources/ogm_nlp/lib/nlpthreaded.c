/*
 *  Natural Language Processing library
 *  NLP call init/reset/flush
 *  Copyright (c) 2017 Pertimm, by Patrick Constant
 *  Dev : September 2017
 *  Version 1.0
 */
#include "ogm_nlp.h"

PUBLIC(og_nlpi) OgNlpRequestInit(og_nlp ctrl_nlp, struct og_nlpi_param *param)
{
  struct og_ctrl_nlpi *ctrl_nlpi = (struct og_ctrl_nlpi *) malloc(sizeof(struct og_ctrl_nlpi));
  IFn(ctrl_nlpi)
  {
    og_char_buffer erreur[DOgErrorSize];
    sprintf(erreur, "OgNlpRequestInit: malloc error on ctrl_nlpi");
    OgErr(param->herr, erreur);
    return NULL;
  }

  memset(ctrl_nlpi, 0, sizeof(struct og_ctrl_nlpi));
  ctrl_nlpi->ctrl_nlp = ctrl_nlp;
  ctrl_nlpi->herr = param->herr;
  ctrl_nlpi->hmutex = param->hmutex;
  memcpy(ctrl_nlpi->loginfo, &param->loginfo, sizeof(struct og_loginfo));

  og_char_buffer nlpc_name[DPcPathSize];

  struct og_msg_param msg_param[1];
  memset(msg_param, 0, sizeof(struct og_msg_param));
  msg_param->herr = ctrl_nlpi->herr;
  msg_param->hmutex = ctrl_nlpi->hmutex;
  msg_param->loginfo.trace = DOgMsgTraceMinimal + DOgMsgTraceMemory;
  msg_param->loginfo.where = ctrl_nlpi->loginfo->where;

  snprintf(nlpc_name, DPcPathSize, "%s_msg", param->name);
  msg_param->module_name = nlpc_name;
  IFn(ctrl_nlpi->hmsg = OgMsgInit(msg_param)) return (0);
  IF(OgMsgTuneInherit(ctrl_nlpi->hmsg, param->hmsg)) return (0);

  // interpret specific memory
  IF(NlpInterpretInit(ctrl_nlpi, param))
  {
    og_char_buffer erreur[DOgErrorSize];
    sprintf(erreur, "OgNlpRequestInit: error on OgNlpInterpretInit");
    OgErr(ctrl_nlpi->herr, erreur);
    return NULL;
  }

  return ctrl_nlpi;

}

PUBLIC(og_status) OgNlpRequestReset(og_nlpi ctrl_nlpi)
{
  IFE(NlpInterpretReset(ctrl_nlpi));

  json_decrefp(&ctrl_nlpi->json_answer);

  DONE;
}

PUBLIC(og_status) OgNlpRequestFlush(og_nlpi ctrl_nlpi)
{
  IFE(NlpInterpretFlush(ctrl_nlpi));


  IFE(OgMsgFlush(ctrl_nlpi->hmsg));

  DPcFree(ctrl_nlpi);

  DONE;
}
