/*
 *  Initialisation functions for Linguistic Transformation compile library
 *  Copyright (c) 2009 Pertimm, Inc. by Patrick Constant
 *  Dev : November 2009
 *  Version 1.0
 */
#include "ogm_ltrac.h"

PUBLIC(void *) OgLtracInit(struct og_ltrac_param *param)
{

  struct og_ctrl_ltrac *ctrl_ltrac = ctrl_ltrac=(struct og_ctrl_ltrac *)malloc(sizeof(struct og_ctrl_ltrac));
  IFn(ctrl_ltrac)
  {
    char erreur[DPcPathSize];
    sprintf(erreur,"OgLtracInit: malloc error on ctrl_ltrac");
    OgErr(param->herr,erreur); return(0);
  }
  memset(ctrl_ltrac,0,sizeof(struct og_ctrl_ltrac));

  ctrl_ltrac->herr = param->herr;
  ctrl_ltrac->hmutex = param->hmutex;
  ctrl_ltrac->cloginfo = param->loginfo;
  ctrl_ltrac->loginfo = &ctrl_ltrac->cloginfo;
  strcpy(ctrl_ltrac->WorkingDirectory,param->WorkingDirectory);

  struct og_msg_param msg_param[1];
  memset(msg_param,0,sizeof(struct og_msg_param));
  msg_param->herr=ctrl_ltrac->herr;
  msg_param->hmutex=ctrl_ltrac->hmutex;
  msg_param->loginfo.trace = DOgMsgTraceMinimal+DOgMsgTraceMemory;
  msg_param->loginfo.where = ctrl_ltrac->loginfo->where;
  msg_param->module_name="ogm_ltrac";
  IFn(ctrl_ltrac->hmsg=OgMsgInit(msg_param)) return(0);
  IF(OgMsgTuneInherit(ctrl_ltrac->hmsg,param->hmsg)) return(0);

  if(ctrl_ltrac->WorkingDirectory[0]) sprintf(ctrl_ltrac->configuration_file,"%s/conf/ogm_ssi.txt",ctrl_ltrac->WorkingDirectory);
  else strcpy(ctrl_ltrac->configuration_file,"conf/ogm_ssi.txt");

  struct og_ltrac_input *input = ctrl_ltrac->input;
  input->dictionaries_to_export = DOgLtracDictionaryTypeBase+DOgLtracDictionaryTypeSwap+DOgLtracDictionaryTypePhon;
  input->dictionaries_minimization = 0;
  input->min_frequency = 1;
  input->min_frequency_swap = 1;

  struct og_pho_param pho_param[1];
  memset(pho_param, 0, sizeof(struct og_pho_param));
  pho_param->herr = ctrl_ltrac->herr;
  pho_param->hmsg = ctrl_ltrac->hmsg;
  pho_param->hmutex = ctrl_ltrac->hmutex;
  pho_param->loginfo.trace = DOgPhoTraceMinimal;
  pho_param->loginfo.where = ctrl_ltrac->loginfo->where;

  if (ctrl_ltrac->WorkingDirectory[0])
  {
    sprintf(pho_param->conf_directory, "%s/%s", ctrl_ltrac->WorkingDirectory, DOgPhoConfigurationDirectory);
    sprintf(pho_param->conf_filename, "phonet_ltra_conf.xml");
  }
  else
  {
    sprintf(pho_param->conf_directory, DOgPhoConfigurationDirectory);
    sprintf(pho_param->conf_filename, "phonet_ltra_conf.xml");
  }

  if (OgFileExists(pho_param->conf_directory))
  {
    ctrl_ltrac->hpho = NULL;
    IFn(ctrl_ltrac->hpho=OgPhoInit(pho_param)) return(0);
  }
  else
  {
    ctrl_ltrac->hpho = NULL;
    OgMsg(ctrl_ltrac->hmsg, "", DOgMsgDestInLog,
        "LtracDicInit: impossible to open '%s' phonetic dictionary will not be created", pho_param->conf_filename);
  }

  return ctrl_ltrac;
}

PUBLIC(int) OgLtracFlush(handle)
  void *handle;
{
  struct og_ctrl_ltrac *ctrl_ltrac = (struct og_ctrl_ltrac *) handle;
  IFn(handle) DONE;
  IFE(OgPhoFlush(ctrl_ltrac->hpho));
  IFE(OgMsgFlush(ctrl_ltrac->hmsg));
  DPcFree(ctrl_ltrac);
  DONE;
}

