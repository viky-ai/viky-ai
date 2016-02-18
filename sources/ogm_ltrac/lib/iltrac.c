/*
 *  Initialisation functions for Linguistic Transformation compile library
 *  Copyright (c) 2009 Pertimm, Inc. by Patrick Constant
 *  Dev : November 2009
 *  Version 1.0
*/
#include "ogm_ltrac.h"




PUBLIC(void *) OgLtracInit(param)
struct og_ltrac_param *param;
{
struct og_msg_param cmsg_param,*msg_param=&cmsg_param;
struct og_aut_param caut_param,*aut_param=&caut_param;
char dictionaries_directory[DPcPathSize];
struct og_ctrl_ltrac *ctrl_ltrac;
char erreur[DOgErrorSize];
char value[DPcPathSize];
int found;

IFn(ctrl_ltrac=(struct og_ctrl_ltrac *)malloc(sizeof(struct og_ctrl_ltrac))) {
  sprintf(erreur,"OgLtracInit: malloc error on ctrl_ltrac");
  OgErr(param->herr,erreur); return(0);
  }
memset(ctrl_ltrac,0,sizeof(struct og_ctrl_ltrac));

ctrl_ltrac->herr = param->herr;
ctrl_ltrac->hmutex = param->hmutex;
ctrl_ltrac->cloginfo = param->loginfo;
ctrl_ltrac->loginfo = &ctrl_ltrac->cloginfo;
strcpy(ctrl_ltrac->WorkingDirectory,param->WorkingDirectory);

memset(msg_param,0,sizeof(struct og_msg_param));
msg_param->herr=ctrl_ltrac->herr;
msg_param->hmutex=ctrl_ltrac->hmutex;
msg_param->loginfo.trace = DOgMsgTraceMinimal+DOgMsgTraceMemory;
msg_param->loginfo.where = ctrl_ltrac->loginfo->where;
msg_param->module_name="ogm_ltrac";
IFn(ctrl_ltrac->hmsg=OgMsgInit(msg_param)) return(0);
IF(OgMsgTuneInherit(ctrl_ltrac->hmsg,param->hmsg)) return(0);

memset(aut_param,0,sizeof(struct og_aut_param));
aut_param->herr=ctrl_ltrac->herr;
aut_param->hmutex=ctrl_ltrac->hmutex;
aut_param->loginfo.trace = DOgAutTraceMinimal+DOgAutTraceMemory;
aut_param->loginfo.where = ctrl_ltrac->loginfo->where;
aut_param->state_number = 0x1000;
sprintf(aut_param->name,"ltrac attrstr");
IFn(ctrl_ltrac->ha_attrstr=OgAutInit(aut_param)) return(0);

sprintf(aut_param->name,"ltrac attrnum");
IFn(ctrl_ltrac->ha_attrnum=OgAutInit(aut_param)) return(0);

if(ctrl_ltrac->WorkingDirectory[0]) sprintf(ctrl_ltrac->configuration_file,"%s/conf/ogm_ssi.txt",ctrl_ltrac->WorkingDirectory);
else strcpy(ctrl_ltrac->configuration_file,"conf/ogm_ssi.txt");

strcpy(ctrl_ltrac->data_directory,"");
if (param->data_directory[0]) {
  OgTrimString(param->data_directory,ctrl_ltrac->data_directory);
  }
else {
  IF(found=OgDipperConfGetVar(ctrl_ltrac->configuration_file,"data_directory",value,DPcPathSize)) return(0);
  if (found) {
    OgTrimString(value,ctrl_ltrac->data_directory);
    }
  }

dictionaries_directory[0]=0;
if (param->dictionaries_directory[0]) {
  if (ctrl_ltrac->WorkingDirectory[0] && !OgIsAbsolutePath(param->dictionaries_directory)) {
    sprintf(dictionaries_directory,"%s/%s",ctrl_ltrac->WorkingDirectory,param->dictionaries_directory);
    }
  else {
    strcpy(dictionaries_directory,param->dictionaries_directory);
    }
  }
else if (param->dictionaries_in_data_directory) {
  sprintf(dictionaries_directory,"%s/%s",ctrl_ltrac->data_directory,DOgSidxLtraDirectory);
  }
else {
  if (ctrl_ltrac->WorkingDirectory[0]) {
    sprintf(dictionaries_directory,"%s/ling",ctrl_ltrac->WorkingDirectory);
    }
  else {
    sprintf(dictionaries_directory,"ling");
    }
  }

IF(OgCheckOrCreateDir(dictionaries_directory,0,ctrl_ltrac->loginfo->where)) return(0);

sprintf(ctrl_ltrac->name_base,"%s/ltra_base.auf",dictionaries_directory);
sprintf(ctrl_ltrac->name_swap,"%s/ltra_swap.auf",dictionaries_directory);
sprintf(ctrl_ltrac->name_phon,"%s/ltra_phon.auf",dictionaries_directory);
sprintf(ctrl_ltrac->name_aspell,"%s/ltra_aspell.txt",dictionaries_directory);

if(ctrl_ltrac->WorkingDirectory[0]) {
  sprintf(ctrl_ltrac->log_base,"%s/log/ltra_base.log",ctrl_ltrac->WorkingDirectory);
  sprintf(ctrl_ltrac->log_swap,"%s/log/ltra_swap.log",ctrl_ltrac->WorkingDirectory);
  sprintf(ctrl_ltrac->log_phon,"%s/log/ltra_phon.log",ctrl_ltrac->WorkingDirectory);
  }
else {
  strcpy(ctrl_ltrac->log_base,"log/ltra_base.log");
  strcpy(ctrl_ltrac->log_swap,"log/ltra_swap.log");
  strcpy(ctrl_ltrac->log_phon,"log/ltra_phon.log");
  }

return((void *)ctrl_ltrac);
}





PUBLIC(int) OgLtracFlush(handle)
void *handle;
{
struct og_ctrl_ltrac *ctrl_ltrac = (struct og_ctrl_ltrac *)handle;
IFn(handle) DONE;

DPcFree(ctrl_ltrac);
DONE;
}

