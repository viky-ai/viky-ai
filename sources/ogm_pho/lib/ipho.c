/*
 *	Initialization for ogm_pho functions
 *  Copyright (c) 2008 Pertimm by P.Constant,G.Logerot and L.Rigouste
 *  Dev : November 2006, April 2008
 *  Version 1.1
*/
#include "ogm_pho.h"


#define DOgRulesNumber            0xf000  /** 60k **/





PUBLIC(void *) OgPhoInit(param)
struct og_pho_param *param;
{
struct og_msg_param cmsg_param,*msg_param=&cmsg_param;
struct og_aut_param caut_param,*aut_param=&caut_param;
struct og_ctrl_pho *inheriting_pho;
struct og_ctrl_pho *ctrl_pho;
char erreur[DOgErrorSize];
int size;

IFn(ctrl_pho=(struct og_ctrl_pho *)malloc(sizeof(struct og_ctrl_pho))) {
  sprintf(erreur,"OgPhoInit: malloc error on ctrl_pho");
  OgErr(param->herr,erreur); return(0);
  }
memset(ctrl_pho,0,sizeof(struct og_ctrl_pho));

ctrl_pho->herr = param->herr;
ctrl_pho->hmutex = param->hmutex;
ctrl_pho->cloginfo = param->loginfo;
ctrl_pho->loginfo = &ctrl_pho->cloginfo;

memset(msg_param,0,sizeof(struct og_msg_param));
msg_param->herr=ctrl_pho->herr; 
msg_param->hmutex=ctrl_pho->hmutex;
msg_param->loginfo.trace = DOgMsgTraceMinimal+DOgMsgTraceMemory; 
msg_param->loginfo.where = ctrl_pho->loginfo->where;
msg_param->module_name="ogm_pho";
IFn(ctrl_pho->hmsg=OgMsgInit(msg_param)) return(0);
IF(OgMsgTuneInherit(ctrl_pho->hmsg,param->hmsg)) return(0);

/* here we are duplicating a previous instance of phonet */
if(param->hpho_to_inherit) {
  ctrl_pho->is_inherited=1;
  inheriting_pho=(struct og_ctrl_pho *)(param->hpho_to_inherit);
  ctrl_pho->max_steps = inheriting_pho->max_steps;
  memcpy(ctrl_pho->space_character,inheriting_pho->space_character,2*sizeof(unsigned char));
  ctrl_pho->non_alpha_to_space = inheriting_pho->non_alpha_to_space;
  memcpy(ctrl_pho->appending_characters,inheriting_pho->appending_characters,2*DOgMaxAppendingCharacters*sizeof(unsigned char));
  ctrl_pho->appending_characters_number = inheriting_pho->appending_characters_number;
  ctrl_pho->ha_rules = inheriting_pho->ha_rules;
  ctrl_pho->Rule = inheriting_pho->Rule;
  ctrl_pho->RuleNumber = inheriting_pho->RuleNumber;
  ctrl_pho->RuleUsed = inheriting_pho->RuleUsed;
  ctrl_pho->Char_class = inheriting_pho->Char_class;
  ctrl_pho->Char_classNumber = inheriting_pho->Char_classNumber;
  ctrl_pho->Char_classUsed = inheriting_pho->Char_classUsed;
  ctrl_pho->BaClass = inheriting_pho->BaClass;
  ctrl_pho->BaClassSize = inheriting_pho->BaClassSize;
  ctrl_pho->BaClassUsed = inheriting_pho->BaClassUsed;

  ctrl_pho->MatchingNumber = DOgMatchingNumber;
  size = ctrl_pho->MatchingNumber*sizeof(struct matching);
  IFn(ctrl_pho->Matching=(struct matching *)malloc(size)) {
    sprintf(erreur,"OgPhoInit: malloc error on Matching (%d bytes)",size);
    OgErr(ctrl_pho->herr,erreur); return(0);
    }

  ctrl_pho->BaSize = DOgBaSize;
  size = ctrl_pho->BaSize*sizeof(unsigned char);
  IFn(ctrl_pho->Ba=(unsigned char *)malloc(size)) {
    sprintf(erreur,"OgPhoInit: malloc error on Ba (%d bytes)",size);
    OgErr(ctrl_pho->herr,erreur); return(0);
    } 
  return((void *)ctrl_pho);
  }

memset(aut_param,0,sizeof(struct og_aut_param));
aut_param->herr=ctrl_pho->herr; 
aut_param->hmutex=ctrl_pho->hmutex;
aut_param->loginfo.trace = DOgAutTraceMinimal; 
aut_param->loginfo.where = ctrl_pho->loginfo->where;
aut_param->state_number = DOgRulesNumber;
sprintf(aut_param->name,"rules");
IFn(ctrl_pho->ha_rules=OgAutInit(aut_param)) return(0);

ctrl_pho->RuleNumber = DOgRuleNumber;
size = ctrl_pho->RuleNumber*sizeof(struct rule);
IFn(ctrl_pho->Rule=(struct rule *)malloc(size)) {
  sprintf(erreur,"OgPhoInit: malloc error on Rule (%d bytes)",size);
  OgErr(ctrl_pho->herr,erreur); return(0);
  }

ctrl_pho->MatchingNumber = DOgMatchingNumber;
size = ctrl_pho->MatchingNumber*sizeof(struct matching);
IFn(ctrl_pho->Matching=(struct matching *)malloc(size)) {
  sprintf(erreur,"OgPhoInit: malloc error on Matching (%d bytes)",size);
  OgErr(ctrl_pho->herr,erreur); return(0);
  }

ctrl_pho->BaSize = DOgBaSize;
size = ctrl_pho->BaSize*sizeof(unsigned char);
IFn(ctrl_pho->Ba=(unsigned char *)malloc(size)) {
  sprintf(erreur,"OgPhoInit: malloc error on Ba (%d bytes)",size);
  OgErr(ctrl_pho->herr,erreur); return(0);
  } 

ctrl_pho->Char_classNumber = DOgChar_classNumber;
size = ctrl_pho->Char_classNumber*sizeof(struct char_class);
IFn(ctrl_pho->Char_class=(struct char_class *)malloc(size)) {
  sprintf(erreur,"OgPhoInit: malloc error on Char_class (%d bytes)",size);
  OgErr(ctrl_pho->herr,erreur); return(0);
  }

ctrl_pho->BaClassSize = DOgBaClassSize;
size = ctrl_pho->BaClassSize*sizeof(unsigned char);
IFn(ctrl_pho->BaClass=(unsigned char *)malloc(size)) {
  sprintf(erreur,"OgPhoInit: malloc error on BaClass (%d bytes)",size);
  OgErr(ctrl_pho->herr,erreur); return(0);
  } 

ctrl_pho->max_steps=1;
memcpy(ctrl_pho->space_character,"\0[",2);

IF(PhoReadConf(ctrl_pho, param->conf)) return(0);

if (ctrl_pho->loginfo->trace & DOgPhoTraceRules) {
  OgMsg(ctrl_pho->hmsg,"",DOgMsgDestInLog,"Dumping of ha_rules into file ha_rules.aut");
  IF(OgAufWrite(ctrl_pho->ha_rules,"log/ha_rules.auf")) return(0);
  IF(RulesLog(ctrl_pho,"log/rules_list.txt")) return(0);
  IF(ClassLog(ctrl_pho,"log/class_list.txt")) return(0);
  }

return((void *)ctrl_pho);
}






PUBLIC(int) OgPhoFlush(handle)
void *handle;
{
struct og_ctrl_pho *ctrl_pho = (struct og_ctrl_pho *)handle;

IFn(ctrl_pho) DONE;

if (!ctrl_pho->is_inherited) {
  IFE(OgAutFlush(ctrl_pho->ha_rules));
  DPcFree(ctrl_pho->Rule);
  DPcFree(ctrl_pho->Char_class);
  DPcFree(ctrl_pho->BaClass);
  }
  
IFE(OgMsgFlush(ctrl_pho->hmsg));

DPcFree(ctrl_pho->Matching);
DPcFree(ctrl_pho->Ba);

DPcFree(ctrl_pho);
DONE;
}



