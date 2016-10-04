/*
 *  Initialisation functions for Linguistic Trf search library
 *  Copyright (c) 2009-2010 Pertimm, by Patrick Constant
 *  Dev : November 2009, February 2010
 *  Version 1.1
*/
#include "ogm_ltras.h"


static int LtrasAddModules(struct og_ctrl_ltras *ctrl_ltras);
static int LtrasGetMaxWordFrequency(struct og_ctrl_ltras *ctrl_ltras);
static og_bool OgLtrasCheckDictionaryDir(struct og_ctrl_ltras *ctrl_ltras, og_string ltras_auf_pattern);



PUBLIC(void *) OgLtrasInit(param)
struct og_ltras_param *param;
{
  struct og_ctrl_ltras *ctrl_ltras;
  IFn(ctrl_ltras=(struct og_ctrl_ltras *)malloc(sizeof(struct og_ctrl_ltras)))
  {
    char erreur[DOgErrorSize];
    sprintf(erreur,"OgLtrasInit: malloc error on ctrl_ltras");
    OgErr(param->herr,erreur); return(0);
  }
  memset(ctrl_ltras,0,sizeof(struct og_ctrl_ltras));

  ctrl_ltras->herr = param->herr;
  ctrl_ltras->hmutex = param->hmutex;
  ctrl_ltras->cloginfo = param->loginfo;
  ctrl_ltras->loginfo = &ctrl_ltras->cloginfo;

  struct og_msg_param msg_param[1];
  memset(msg_param, 0, sizeof(struct og_msg_param));
  msg_param->herr = ctrl_ltras->herr;
  msg_param->hmutex = ctrl_ltras->hmutex;
  msg_param->loginfo.trace = DOgMsgTraceMinimal+DOgMsgTraceMemory;
  msg_param->loginfo.where = ctrl_ltras->loginfo->where;
  msg_param->module_name = "ogm_ltras";
  IFn(ctrl_ltras->hmsg = OgMsgInit(msg_param)) return(0);
  IF(OgMsgTuneInherit(ctrl_ltras->hmsg, param->hmsg)) return(0);

  ctrl_ltras->hltras_to_inherit = param->hltras_to_inherit;
  strcpy(ctrl_ltras->WorkingDirectory,param->WorkingDirectory);
  strcpy(ctrl_ltras->configuration_file,param->configuration_file);
  IFn(ctrl_ltras->configuration_file[0])
  {
    if(ctrl_ltras->WorkingDirectory[0])
    sprintf(ctrl_ltras->configuration_file,"%s/%s",ctrl_ltras->WorkingDirectory,DOgLtrasModuleConfiguration);
    else
    strcpy(ctrl_ltras->configuration_file,DOgLtrasModuleConfiguration);
  }

// Check if dictionaries_directory contains ltras_*.auf
  ctrl_ltras->dictionaries_directory[0] = 0;
  if(param->dictionaries_directory[0])
  {
    og_char_buffer ltra_auf_pattern[DPcPathSize];
    snprintf(ltra_auf_pattern, DPcPathSize, "%s/%s", param->dictionaries_directory, "ltra_*.auf");

    if(OgLtrasCheckDictionaryDir(ctrl_ltras, ltra_auf_pattern))
    {
      snprintf(ctrl_ltras->dictionaries_directory, DPcPathSize, "%s", param->dictionaries_directory);
    }
    else
    {
      og_char_buffer log_msg[DPcPathSize];
      snprintf(log_msg, DPcPathSize, "OgLtrasInit : '%s' does not exist, using default directory as fallback (ling/)", ltra_auf_pattern);
      IF(OgMsg(ctrl_ltras->hmsg, 0 , DOgMsgDestInLog + DOgMsgSeverityDebug, log_msg)) return NULL;
    }
  }

  if (ctrl_ltras->dictionaries_directory[0] == 0 && ctrl_ltras->WorkingDirectory[0])
  {
    snprintf(ctrl_ltras->dictionaries_directory, DPcPathSize, "%s/ling", ctrl_ltras->WorkingDirectory);
  }

  strcpy(ctrl_ltras->caller_label,param->caller_label);
  strcpy(ctrl_ltras->output_file,param->output_file);
  ctrl_ltras->first_added_result=1;

  struct og_lip_param lip_param[1];
  memset(lip_param,0,sizeof(struct og_lip_param));
  lip_param->herr=ctrl_ltras->herr;
  lip_param->hmsg=ctrl_ltras->hmsg;
  lip_param->hmutex=ctrl_ltras->hmutex;
  lip_param->loginfo.trace = DOgLipTraceMinimal+DOgLipTraceMemory;
  lip_param->loginfo.where = ctrl_ltras->loginfo->where;
  lip_param->conf = &ctrl_ltras->lip_conf;

  if (ctrl_ltras->WorkingDirectory[0])
  {
    sprintf(lip_param->filename, "%s/conf/%s", ctrl_ltras->WorkingDirectory, DOgLipConfPunctuationFileName);
  }
  else
  {
    sprintf(lip_param->filename, "conf/%s", DOgLipConfPunctuationFileName);
  }

  IFn(ctrl_ltras->hlip=OgLipInit(lip_param)) return(0);

  ctrl_ltras->NodeNumber = DOgLtrasNodeNumber;
  int size = ctrl_ltras->NodeNumber*sizeof(struct node);
  IFn(ctrl_ltras->Node=(struct node *)malloc(size))
  {
    char erreur[DOgErrorSize];
    sprintf(erreur,"OgLtrasInit: malloc error on Node (%d bytes)",size);
    OgErr(ctrl_ltras->herr,erreur); return(0);
  }

  ctrl_ltras->BaSize = DOgLtrasBaSize;
  size = ctrl_ltras->BaSize*sizeof(unsigned char);
  IFn(ctrl_ltras->Ba=(unsigned char *)malloc(size))
  {
    char erreur[DOgErrorSize];
    sprintf(erreur,"OgLtrasInit: malloc error on Ba (%d bytes)",size);
    OgErr(ctrl_ltras->herr,erreur); return(0);
  }

  /** temporary buffer for all usages **/
  og_char_buffer lt_heap_name[DOgAutNameSize];
  snprintf(lt_heap_name, DOgAutNameSize, "%s_ltras_ba1", param->caller_label);
  IFn(ctrl_ltras->hba1=OgHeapInit(ctrl_ltras->hmsg,lt_heap_name,sizeof(unsigned char),0x400)) return(0);
  snprintf(lt_heap_name, DOgAutNameSize, "%s_ltras_ba2", param->caller_label);
  IFn(ctrl_ltras->hba2=OgHeapInit(ctrl_ltras->hmsg,lt_heap_name,sizeof(unsigned char),0x400)) return(0);

  struct og_rqp_param rqp_param[1];
  memset(rqp_param,0,sizeof(struct og_rqp_param));
  rqp_param->herr=ctrl_ltras->herr;
  rqp_param->hmsg=ctrl_ltras->hmsg;
  rqp_param->hmutex=ctrl_ltras->hmutex;
  rqp_param->loginfo.trace = DOgRqpTraceMinimal+DOgRqpTraceMemory;
  rqp_param->loginfo.where = ctrl_ltras->loginfo->where;
  IFn(ctrl_ltras->hrqp=OgRqpInit(rqp_param)) return(0);

  struct og_aut_param aut_param[1];
  memset(aut_param,0,sizeof(struct og_aut_param));
  aut_param->herr=ctrl_ltras->herr;
  aut_param->hmutex=ctrl_ltras->hmutex;
  aut_param->loginfo.trace = DOgAutTraceMinimal+DOgAutTraceMemory;
  aut_param->loginfo.where = ctrl_ltras->loginfo->where;
  aut_param->state_number = 0;

  if (ctrl_ltras->hltras_to_inherit)
  {
    struct og_ctrl_ltras *inheriting_ltras=(struct og_ctrl_ltras *)param->hltras_to_inherit;
    ctrl_ltras->ha_false = inheriting_ltras->ha_false;
  }
  else
  {

    aut_param->state_number = 0x800;
    sprintf(aut_param->name,"false");
    IFn(ctrl_ltras->ha_false=OgAutInit(aut_param)) return(0);

    char ltras_false[DPcPathSize];
    if (ctrl_ltras->WorkingDirectory[0]) sprintf(ltras_false,"%s/ling/ltras_false.xml",ctrl_ltras->WorkingDirectory);
    else strcpy(ltras_false,"ling/ltras_false.xml");

    og_status status = OgXmlXsdValidateFile(ctrl_ltras->hmsg, ctrl_ltras->herr, ctrl_ltras->WorkingDirectory,
        ltras_false, "ling/xsd/ltras_false.xsd");
    IF(status) return (0);

    IF(LtrasFalseReadConf(ctrl_ltras,ltras_false)) return(0);
  }

  struct og_pho_param pho_param[1];
  memset(pho_param,0,sizeof(struct og_pho_param));
  pho_param->herr=ctrl_ltras->herr;
  pho_param->hmsg=ctrl_ltras->hmsg;
  pho_param->hmutex=ctrl_ltras->hmutex;
  pho_param->loginfo.trace = DOgPhoTraceMinimal+DOgPhoTraceMemory;
  pho_param->loginfo.where = ctrl_ltras->loginfo->where;
  if (ctrl_ltras->WorkingDirectory[0]) sprintf(pho_param->conf,"%s/conf/phonet_ltra_conf.xml",ctrl_ltras->WorkingDirectory);
  else strcpy(pho_param->conf,"conf/phonet_ltra_conf.xml");
  if (OgFileExists(pho_param->conf))
  {
    if (ctrl_ltras->hltras_to_inherit)
    {
      struct og_ctrl_ltras *inheriting_ltras = (struct og_ctrl_ltras *) ctrl_ltras->hltras_to_inherit;
      pho_param->hpho_to_inherit = inheriting_ltras->hpho;
    }
    IFn(ctrl_ltras->hpho=OgPhoInit(pho_param)) return(0);
  }
  else
  {
    OgMsg(ctrl_ltras->hmsg,"",DOgMsgDestInLog
        , "OgLtrasInit: impossible to open '%s'",pho_param->conf);
  }

  struct og_ldi_param ldi_param[1];
  memset(ldi_param,0,sizeof(struct og_ldi_param));
  ldi_param->herr=ctrl_ltras->herr;
  ldi_param->hmsg=ctrl_ltras->hmsg;
  ldi_param->hmutex=ctrl_ltras->hmutex;
  ldi_param->loginfo.trace = DOgLdiTraceMinimal+DOgLdiTraceMemory;
  ldi_param->loginfo.where = ctrl_ltras->loginfo->where;
  strcpy(ldi_param->WorkingDirectory,ctrl_ltras->WorkingDirectory);
  if (ctrl_ltras->WorkingDirectory[0]) sprintf(ldi_param->configuration_file,"%s/conf/ldi_conf.xml",ctrl_ltras->WorkingDirectory);
  else strcpy(ldi_param->configuration_file,"conf/ldi_conf.xml");
  if (OgFileExists(ldi_param->configuration_file))
  {
    if (ctrl_ltras->hltras_to_inherit)
    {
      /** ldi is reentrant and thread-safe by inheriting automatons handles **/
      struct og_ctrl_ltras *inheriting_ltras = (struct og_ctrl_ltras *) ctrl_ltras->hltras_to_inherit;
      ldi_param->hldi_to_inherit = inheriting_ltras->hldi;
    }
    IFn(ctrl_ltras->hldi=OgLdiInit(ldi_param)) return(0);
  }
  else
  {
    OgMsg(ctrl_ltras->hmsg,"",DOgMsgDestInLog
        , "OgLtrasInit: impossible to open '%s'",ldi_param->configuration_file);
  }

  aut_param->state_number = 0x800;
  sprintf(aut_param->name,"suggest");
  IFn(ctrl_ltras->ha_suggest=OgAutInit(aut_param)) return(0);

  sprintf(aut_param->name,"oper");
  IFn(ctrl_ltras->ha_oper=OgAutInit(aut_param)) return(0);

  sprintf(aut_param->name,"param");
  IFn(ctrl_ltras->ha_param=OgAutInit(aut_param)) return(0);

  struct og_stm_param stm_param[1];
  memset(stm_param,0,sizeof(struct og_stm_param));
  stm_param->herr=ctrl_ltras->herr;
  stm_param->hmsg=ctrl_ltras->hmsg;
  stm_param->hmutex=ctrl_ltras->hmutex;
  stm_param->loginfo.trace = DOgStmTraceMinimal+DOgStmTraceMemory;
  stm_param->loginfo.where = ctrl_ltras->loginfo->where;
  strcpy(stm_param->WorkingDirectory,ctrl_ltras->WorkingDirectory);
  IFn(ctrl_ltras->hstm=OgStmInit(stm_param)) return(0);

  /* A default value must be set, in case the ha_base is not existing
   * the LtrasGetMaxWordFrequency calculates the real value when the ha_base
   * automaton exists and as soon as it is loaded
   * We use a 10 value so that log10(10)=1; */
  ctrl_ltras->max_word_frequency=10;
  ctrl_ltras->max_word_frequency_log10=log10(ctrl_ltras->max_word_frequency);

  IF(LtrasAddModules(ctrl_ltras)) return(0);

  ctrl_ltras->statistics.global_start=OgMicroClock();
  ctrl_ltras->statistics.min_elapsed=0x7fffffff;
  ctrl_ltras->statistics.max_elapsed=0;

  return((void *)ctrl_ltras);
}


int LtrasReset(struct og_ctrl_ltras *ctrl_ltras)
{
ctrl_ltras->NodeUsed=0;
/** Keeping the Ba part for modules **/
ctrl_ltras->BaUsed=ctrl_ltras->BaModuleUsed;
DONE;
}



PUBLIC(void *) OgLtrasHaBase(void *handle)
{
struct og_ctrl_ltras *ctrl_ltras = (struct og_ctrl_ltras *)handle;
struct og_aut_param caut_param,*aut_param=&caut_param;
char ha_name[DPcPathSize];

if (ctrl_ltras->ha_base_accessed) return(ctrl_ltras->ha_base);
ctrl_ltras->ha_base_accessed=1;

if (ctrl_ltras->hltras_to_inherit) {
  ctrl_ltras->ha_base=OgLtrasHaBase(ctrl_ltras->hltras_to_inherit);
  }
else {
  memset(aut_param,0,sizeof(struct og_aut_param));
  aut_param->herr=ctrl_ltras->herr;
  aut_param->hmutex=ctrl_ltras->hmutex;
  aut_param->loginfo.trace = DOgAutTraceMinimal+DOgAutTraceMemory;
  aut_param->loginfo.where = ctrl_ltras->loginfo->where;
  aut_param->state_number = 0;
  sprintf(aut_param->name,"ltra_base");
  IFn(ctrl_ltras->ha_base=OgAutInit(aut_param)) return(0);
  if (ctrl_ltras->dictionaries_directory[0]) sprintf(ha_name,"%s/ltra_base.auf",ctrl_ltras->dictionaries_directory);
  else strcpy(ha_name,"ling/ltra_base.auf");

  if (!OgFileExists(ha_name))
  {
    OgMsg(ctrl_ltras->hmsg, "", DOgMsgDestInLog, "OgLtrasHaBase : impossible to open '%s'", ha_name);
    OgAutFlush(ctrl_ltras->ha_base);
    ctrl_ltras->ha_base = NULL;
    return NULL;
  }

  IF(OgAufRead(ctrl_ltras->ha_base, ha_name)) return NULL;

  }
/** Needs to be calculated as soon as the ha_base automaton is loaded **/
IF(LtrasGetMaxWordFrequency(ctrl_ltras)) return(0);

return(ctrl_ltras->ha_base);
}




PUBLIC(void *) OgLtrasHaSwap(void *handle)
{
struct og_ctrl_ltras *ctrl_ltras = (struct og_ctrl_ltras *)handle;
struct og_aut_param caut_param,*aut_param=&caut_param;
char ha_name[DPcPathSize];

if (ctrl_ltras->ha_swap_accessed) return(ctrl_ltras->ha_swap);
ctrl_ltras->ha_swap_accessed=1;

if (ctrl_ltras->hltras_to_inherit) {
  ctrl_ltras->ha_swap=OgLtrasHaSwap(ctrl_ltras->hltras_to_inherit);
  }
else {
  memset(aut_param,0,sizeof(struct og_aut_param));
  aut_param->herr=ctrl_ltras->herr;
  aut_param->hmutex=ctrl_ltras->hmutex;
  aut_param->loginfo.trace = DOgAutTraceMinimal+DOgAutTraceMemory;
  aut_param->loginfo.where = ctrl_ltras->loginfo->where;
  aut_param->state_number = 0;
  sprintf(aut_param->name,"ltra_swap");
  IFn(ctrl_ltras->ha_swap=OgAutInit(aut_param)) return(0);
  if (ctrl_ltras->dictionaries_directory[0]) sprintf(ha_name,"%s/ltra_swap.auf",ctrl_ltras->dictionaries_directory);
  else strcpy(ha_name,"ling/ltra_swap.auf");

  if (!OgFileExists(ha_name))
  {
    OgMsg(ctrl_ltras->hmsg, "", DOgMsgDestInLog, "OgLtrasHaSwap : impossible to open '%s'", ha_name);
    OgAutFlush(ctrl_ltras->ha_swap);
    ctrl_ltras->ha_swap = NULL;
    return NULL;
  }

  IF(OgAufRead(ctrl_ltras->ha_swap, ha_name)) return NULL;

  }

return(ctrl_ltras->ha_swap);
}




PUBLIC(void *) OgLtrasHaPhon(void *handle)
{
struct og_ctrl_ltras *ctrl_ltras = (struct og_ctrl_ltras *)handle;
struct og_aut_param caut_param,*aut_param=&caut_param;
char ha_name[DPcPathSize];

if (ctrl_ltras->ha_phon_accessed) return(ctrl_ltras->ha_phon);
ctrl_ltras->ha_phon_accessed=1;

if (ctrl_ltras->hltras_to_inherit) {
  ctrl_ltras->ha_phon=OgLtrasHaPhon(ctrl_ltras->hltras_to_inherit);
  }
else {
  memset(aut_param,0,sizeof(struct og_aut_param));
  aut_param->herr=ctrl_ltras->herr;
  aut_param->hmutex=ctrl_ltras->hmutex;
  aut_param->loginfo.trace = DOgAutTraceMinimal+DOgAutTraceMemory;
  aut_param->loginfo.where = ctrl_ltras->loginfo->where;
  aut_param->state_number = 0;
  sprintf(aut_param->name,"ltra_phon");
  IFn(ctrl_ltras->ha_phon=OgAutInit(aut_param)) return(0);
  if (ctrl_ltras->dictionaries_directory[0]) sprintf(ha_name,"%s/ltra_phon.auf",ctrl_ltras->dictionaries_directory);
  else strcpy(ha_name,"ling/ltra_phon.auf");

  if (!OgFileExists(ha_name))
  {
    OgMsg(ctrl_ltras->hmsg, "", DOgMsgDestInLog, "OgLtrasHaPhon : impossible to open '%s'", ha_name);
    OgAutFlush(ctrl_ltras->ha_phon);
    ctrl_ltras->ha_phon = NULL;
    return NULL;
  }

    IF(OgAufRead(ctrl_ltras->ha_phon, ha_name)) return NULL;

  }

return(ctrl_ltras->ha_phon);
}




PUBLIC(void *) OgLtrasHaFalse(void *handle)
{
struct og_ctrl_ltras *ctrl_ltras = (struct og_ctrl_ltras *)handle;
return(ctrl_ltras->ha_false);
}




PUBLIC(void *) OgLtrasHpho(void *handle)
{
struct og_ctrl_ltras *ctrl_ltras = (struct og_ctrl_ltras *)handle;
return(ctrl_ltras->hpho);
}




PUBLIC(void *) OgLtrasHldi(void *handle)
{
struct og_ctrl_ltras *ctrl_ltras = (struct og_ctrl_ltras *)handle;
return(ctrl_ltras->hldi);
}




PUBLIC(void *) OgLtrasHstm(void *handle)
{
struct og_ctrl_ltras *ctrl_ltras = (struct og_ctrl_ltras *)handle;
return(ctrl_ltras->hstm);
}




PUBLIC(double) OgLtrasScoreFactor(void *handle)
{
struct og_ctrl_ltras *ctrl_ltras = (struct og_ctrl_ltras *)handle;
return(ctrl_ltras->input->score_factor);
}

PUBLIC(double) OgLtrasFrequencyRatio(void *handle)
{
  struct og_ctrl_ltras *ctrl_ltras = (struct og_ctrl_ltras *) handle;
  return (ctrl_ltras->input->frequency_ratio);
}

PUBLIC(og_bool) OgLtrasScoreFactorIsLogPosActivated(void *handle)
{
  struct og_ctrl_ltras *ctrl_ltras = (struct og_ctrl_ltras *)handle;
  return(ctrl_ltras->input->log_pos);
}


PUBLIC(int) OgLtrasMaxWordFrequency(void *handle)
{
struct og_ctrl_ltras *ctrl_ltras = (struct og_ctrl_ltras *)handle;
return(ctrl_ltras->max_word_frequency);
}




PUBLIC(char *) OgLtrasWorkingDirectory(void *handle)
{
struct og_ctrl_ltras *ctrl_ltras = (struct og_ctrl_ltras *)handle;
return(ctrl_ltras->WorkingDirectory);
}




PUBLIC(og_status) OgLtrasGetFrequencyFromNormalizedFrequency(void *handle, double normalized_frequency, double *pfrequency)
{
struct og_ctrl_ltras *ctrl_ltras = (struct og_ctrl_ltras *)handle;
double frequency_log10;

frequency_log10 = normalized_frequency * ctrl_ltras->max_word_frequency_log10;
*pfrequency = pow(10,frequency_log10);

DONE;
}



PUBLIC(og_status) OgLtrasGetMaximumTransformation(void *handle,int *pmaximum_transformation_length,int **pmaximum_transformation)
{
struct og_ctrl_ltras *ctrl_ltras = (struct og_ctrl_ltras *)handle;
*pmaximum_transformation_length = DOgLtrasMaximumTransformationLength;
*pmaximum_transformation = ctrl_ltras->maximum_transformation;
DONE;
}


static int LtrasAddModules(struct og_ctrl_ltras *ctrl_ltras)
{
  ctrl_ltras->ModuleNumber = DOgLtrasModuleNumber;
  int size = ctrl_ltras->ModuleNumber * sizeof(struct module);
  IFn(ctrl_ltras->Module=(struct module *)malloc(size))
  {
    char erreur[DOgErrorSize];
    sprintf(erreur, "LtrasAddModules: malloc error on Module (%d bytes)", size);
    OgErr(ctrl_ltras->herr, erreur);
    DPcErr;
  }

  /** Zero module is reserved for basic trfs **/
  struct module *module;
  IFE(LtrasAllocModule(ctrl_ltras, &module));

  /** Add other modules from XML configuration file **/

  og_status status = OgXmlXsdValidateFile(ctrl_ltras->hmsg, ctrl_ltras->herr, ctrl_ltras->WorkingDirectory,
      ctrl_ltras->configuration_file, "conf/xsd/ltras_conf.xsd");
  IFE(status);
  IFE(OgLtrasModuleReadConf(ctrl_ltras, ctrl_ltras->configuration_file));

  struct og_ltra_module_param module_param[1];
  memset(module_param, 0, sizeof(struct og_ltra_module_param));
  module_param->hltras = ctrl_ltras;
  module_param->herr = ctrl_ltras->herr;
  module_param->hmsg = ctrl_ltras->hmsg;
  module_param->hmutex = ctrl_ltras->hmutex;
  module_param->loginfo.trace = ctrl_ltras->loginfo->trace;
  module_param->loginfo.where = ctrl_ltras->loginfo->where;

  for (int i = 0; i < ctrl_ltras->ModuleUsed; i++)
  {
    module = ctrl_ltras->Module + i;
    IFn(module->init) continue;
    if (ctrl_ltras->hltras_to_inherit)
    {
      struct og_ctrl_ltras *inheriting_ltras = (struct og_ctrl_ltras *) ctrl_ltras->hltras_to_inherit;
      module_param->hmodule_to_inherit = inheriting_ltras->Module[i].handle;
    }
    IFn(module->handle=module->init(module_param))
    {
      unsigned char *module_name = ctrl_ltras->Ba + module->start_name;
      OgMsg(ctrl_ltras->hmsg, "", DOgMsgDestInLog + DOgMsgSeverityWarning,
          "LtrasAddModules: module '%s' disactivated because of following error(s):", module_name);
      OgMsgErr(ctrl_ltras->hmsg, module_name, 0, 0, 0, DOgMsgSeverityWarning,
      DOgErrLogFlagNotInErr + DOgErrLogFlagNoSystemError + DOgErrLogFlagNoDate);
      module->init = 0;
      module->module = 0;
      module->flush = 0;
    }
  }

  ctrl_ltras->BaModuleUsed = ctrl_ltras->BaUsed;

  DONE;
}


static int LtrasGetMaxWordFrequency(struct og_ctrl_ltras *ctrl_ltras)
{
unsigned char *p,out[DPcAutMaxBufferSize+9];
int attribute_number,language,frequency;
oindex states[DPcAutMaxBufferSize+9];
int retour,nstate0,nstate1,iout;

if (ctrl_ltras->hltras_to_inherit)
{
  struct og_ctrl_ltras *ctrl_ltras_parent = (struct og_ctrl_ltras *) ctrl_ltras->hltras_to_inherit;
  ctrl_ltras->max_word_frequency_log10 = ctrl_ltras_parent->max_word_frequency_log10;

  DONE;
}

/** 10 is minimum value to avoid a null, negative or infinite log10 **/
ctrl_ltras->max_word_frequency=10;
if ((retour=OgAufScanf(ctrl_ltras->ha_base,-1,"",&iout,out,&nstate0,&nstate1,states))) {
  do {
    int i,c,sep=(-1);
    IFE(retour);
    for (i=0; i<iout; i+=2) {
      c = (out[i]<<8)+out[i+1];
      if (c==1) { sep=i; break; }
      }
    if (sep<0) continue;
    p=out+sep+2;
    IFE(DOgPnin4(ctrl_ltras->herr,&p,&attribute_number));
    IFE(DOgPnin4(ctrl_ltras->herr,&p,&language));
    IFE(DOgPnin4(ctrl_ltras->herr,&p,&frequency));
    if (ctrl_ltras->max_word_frequency < frequency) ctrl_ltras->max_word_frequency = frequency;
    }
  while((retour=OgAufScann(ctrl_ltras->ha_base,&iout,out,nstate0,&nstate1,states)));
  }
ctrl_ltras->max_word_frequency_log10=log10(ctrl_ltras->max_word_frequency);
DONE;
}






PUBLIC(og_status) OgLtrasFlush(handle)
void *handle;
{
struct og_ctrl_ltras *ctrl_ltras = (struct og_ctrl_ltras *)handle;
struct module *module;
int i;

IFn(handle) DONE;

for (i=0; i<ctrl_ltras->ModuleUsed; i++) {
  module = ctrl_ltras->Module + i;
  IFn(module->init) continue;
  IFn(module->flush) continue;
  IFE(module->flush(module->handle));
  }

if (!ctrl_ltras->hltras_to_inherit) {
  IFE(OgAutFlush(ctrl_ltras->ha_base));
  IFE(OgAutFlush(ctrl_ltras->ha_swap));
  IFE(OgAutFlush(ctrl_ltras->ha_phon));
  IFE(OgAutFlush(ctrl_ltras->ha_false));
  }

IFE(OgAutFlush(ctrl_ltras->ha_oper));
IFE(OgAutFlush(ctrl_ltras->ha_param));
IFE(OgAutFlush(ctrl_ltras->ha_suggest));
IFE(OgLipFlush(ctrl_ltras->hlip));
IFE(OgPhoFlush(ctrl_ltras->hpho));
IFE(OgLdiFlush(ctrl_ltras->hldi));
IFE(OgStmFlush(ctrl_ltras->hstm));
IFE(OgRqpFlush(ctrl_ltras->hrqp));

IFE(OgMsgFlush(ctrl_ltras->hmsg));

IFE(OgHeapFlush(ctrl_ltras->hba1));
IFE(OgHeapFlush(ctrl_ltras->hba2));
DPcFree(ctrl_ltras->Module);
DPcFree(ctrl_ltras->Node);
DPcFree(ctrl_ltras->Ba);

DPcFree(ctrl_ltras);
DONE;
}


static og_bool OgLtrasCheckDictionaryDir(struct og_ctrl_ltras *ctrl_ltras, og_string ltras_auf_pattern)
{
  struct og_file file[1];
  memset(file, 0, sizeof(struct og_file));

  og_bool found = OgFindFirstFile(file, (unsigned char*) ltras_auf_pattern);
  IFE(found);

  OgFindClose(file);

  return found;
}
