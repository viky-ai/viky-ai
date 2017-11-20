/*
 *  Initialization for ogm_pho functions
 *  Copyright (c) 2008 Pertimm by P.Constant,G.Logerot and L.Rigouste
 *  Dev : November 2006, April 2008
 *  Version 1.1
 */
#include "ogm_pho.h"

static og_status PhoParseOgmSsiConf(struct og_ctrl_pho *ctrl_pho);
static og_status PhoCopyLangContext(struct lang_context *lang_context_to_copy, struct lang_context *new_lang_context);

PUBLIC(void *) OgPhoInit(struct og_pho_param *param)
{

  struct og_ctrl_pho *ctrl_pho = (struct og_ctrl_pho *)malloc(sizeof(struct og_ctrl_pho));
  IFn(ctrl_pho)
  {
    char erreur[DOgErrorSize];
    sprintf(erreur,"OgPhoInit: malloc error on ctrl_pho");
    OgErr(param->herr,erreur); return(0);
  }
  memset(ctrl_pho,0,sizeof(struct og_ctrl_pho));

  ctrl_pho->herr = param->herr;
  ctrl_pho->hmutex = param->hmutex;
  ctrl_pho->cloginfo = param->loginfo;
  ctrl_pho->loginfo = &ctrl_pho->cloginfo;

  struct og_msg_param msg_param[1];
  memset(msg_param,0,sizeof(struct og_msg_param));
  msg_param->herr=ctrl_pho->herr;
  msg_param->hmutex=ctrl_pho->hmutex;
  msg_param->loginfo.trace = DOgMsgTraceMinimal+DOgMsgTraceMemory;
  msg_param->loginfo.where = ctrl_pho->loginfo->where;
  msg_param->module_name="ogm_pho";
  IFn(ctrl_pho->hmsg=OgMsgInit(msg_param)) return(0);
  IF(OgMsgTuneInherit(ctrl_pho->hmsg,param->hmsg)) return(0);

  ctrl_pho->lang_context_map = g_hash_table_new_full(g_direct_hash, g_direct_equal, NULL, (GCacheDestroyFunc) PhoLangContextDestroy);
  IFN(ctrl_pho->lang_context_map)
  {
    char erreur[DOgErrorSize];
    sprintf(erreur,"OgPhoInit: malloc error on : lang_context_map = g_hash_table_new_full()");
    OgErr(param->herr,erreur);
    return NULL;
  }

  IFn(ctrl_pho->lang_context_heap = OgHeapSliceInit(ctrl_pho->hmsg, "pho_lang_context_heap", sizeof(struct lang_context), 2, 8)) return NULL;


  /* here we are duplicating a previous instance of phonet */
  if(param->hpho_to_inherit)
  {
    ctrl_pho->is_inherited=1;
    struct og_ctrl_pho * inheriting_pho=(struct og_ctrl_pho *)(param->hpho_to_inherit);

    GList *list_key = g_hash_table_get_keys(inheriting_pho->lang_context_map);
    for (GList *iter = list_key; iter; iter = iter->next)
    {
      size_t Ilang_context_heap = 0;
      struct lang_context *new_lang_context = (struct lang_context *) OgHeapNewCell(ctrl_pho->lang_context_heap, &Ilang_context_heap);
      new_lang_context->ctrl_pho = ctrl_pho;

      struct lang_context *lang_context_to_copy = g_hash_table_lookup(inheriting_pho->lang_context_map, iter->data);

      IF(PhoCopyLangContext(lang_context_to_copy, new_lang_context)) return (0);

      g_hash_table_insert(ctrl_pho->lang_context_map, iter->data, new_lang_context);
    }

    return((void *)ctrl_pho);
  }

  snprintf(ctrl_pho->conf_directory, DPcPathSize, "%s", param->conf_directory);
  snprintf(ctrl_pho->conf_filename, DPcPathSize, "%s", param->conf_filename);

  IF(PhoParseOgmSsiConf(ctrl_pho)) return NULL;

  IF(PhoReadConfFiles(ctrl_pho)) return(0);

  return((void *)ctrl_pho);
}

og_status PhoCopyLangContext(struct lang_context *lang_context_to_copy, struct lang_context *new_lang_context)
{
  new_lang_context->max_steps = lang_context_to_copy->max_steps;
  memcpy(new_lang_context->space_character, lang_context_to_copy->space_character, 2 * sizeof(unsigned char));
  new_lang_context->non_alpha_to_space = lang_context_to_copy->non_alpha_to_space;
  memcpy(new_lang_context->appending_characters, lang_context_to_copy->appending_characters,
      2 * DOgMaxAppendingCharacters * sizeof(unsigned char));
  new_lang_context->appending_characters_number = lang_context_to_copy->appending_characters_number;
  new_lang_context->ha_rules = lang_context_to_copy->ha_rules;
  new_lang_context->Rule = lang_context_to_copy->Rule;
  new_lang_context->RuleNumber = lang_context_to_copy->RuleNumber;
  new_lang_context->RuleUsed = lang_context_to_copy->RuleUsed;
  new_lang_context->Char_class = lang_context_to_copy->Char_class;
  new_lang_context->Char_classNumber = lang_context_to_copy->Char_classNumber;
  new_lang_context->Char_classUsed = lang_context_to_copy->Char_classUsed;
  new_lang_context->BaClass = lang_context_to_copy->BaClass;
  new_lang_context->BaClassSize = lang_context_to_copy->BaClassSize;
  new_lang_context->BaClassUsed = lang_context_to_copy->BaClassUsed;

  new_lang_context->MatchingNumber = DOgMatchingNumber;
  int size = new_lang_context->MatchingNumber * sizeof(struct matching);
  IFn(new_lang_context->Matching=(struct matching *)malloc(size))
  {
    char erreur[DOgErrorSize];
    sprintf(erreur, "OgPhoInit: malloc error on Matching (%d bytes)", size);
    OgErr(new_lang_context->ctrl_pho->herr, erreur);
    DPcErr;
  }

  new_lang_context->BaSize = DOgBaSize;
  size = new_lang_context->BaSize * sizeof(unsigned char);
  IFn(new_lang_context->Ba=(unsigned char *)malloc(size))
  {
    char erreur[DOgErrorSize];
    sprintf(erreur, "OgPhoInit: malloc error on Ba (%d bytes)", size);
    OgErr(new_lang_context->ctrl_pho->herr, erreur);
    DPcErr;
  }
  DONE;
}


og_status PhoInitLangContext(struct og_ctrl_pho *ctrl_pho, struct lang_context *lang_context)
{

  lang_context->ctrl_pho = ctrl_pho;

  struct og_aut_param aut_param[1];
  memset(aut_param, 0, sizeof(struct og_aut_param));
  aut_param->herr = ctrl_pho->herr;
  aut_param->hmutex = ctrl_pho->hmutex;
  aut_param->loginfo.trace = DOgAutTraceMinimal;
  aut_param->loginfo.where = ctrl_pho->loginfo->where;
  aut_param->state_number = 0x0;
  sprintf(aut_param->name, "rules");
  IFn(lang_context->ha_rules=OgAutInit(aut_param)) return (0);

  lang_context->RuleNumber = DOgRuleNumber;
  int size = lang_context->RuleNumber * sizeof(struct rule);
  IFn(lang_context->Rule=(struct rule *)malloc(size))
  {
    char erreur[DOgErrorSize];
    sprintf(erreur, "OgPhoInit: malloc error on Rule (%d bytes)", size);
    OgErr(ctrl_pho->herr, erreur);
    DPcErr;
  }

  lang_context->MatchingNumber = DOgMatchingNumber;
  size = lang_context->MatchingNumber * sizeof(struct matching);
  IFn(lang_context->Matching=(struct matching *)malloc(size))
  {
    char erreur[DOgErrorSize];
    sprintf(erreur, "OgPhoInit: malloc error on Matching (%d bytes)", size);
    OgErr(ctrl_pho->herr, erreur);
    DPcErr;
  }

  lang_context->BaSize = DOgBaSize;
  size = lang_context->BaSize * sizeof(unsigned char);
  IFn(lang_context->Ba=(unsigned char *)malloc(size))
  {
    char erreur[DOgErrorSize];
    sprintf(erreur, "OgPhoInit: malloc error on Ba (%d bytes)", size);
    OgErr(ctrl_pho->herr, erreur);
    DPcErr;
  }

  lang_context->Char_classNumber = DOgChar_classNumber;
  size = lang_context->Char_classNumber * sizeof(struct char_class);
  IFn(lang_context->Char_class=(struct char_class *)malloc(size))
  {
    char erreur[DOgErrorSize];
    sprintf(erreur, "OgPhoInit: malloc error on Char_class (%d bytes)", size);
    OgErr(ctrl_pho->herr, erreur);
    DPcErr;
  }

  lang_context->BaClassSize = DOgBaClassSize;
  size = lang_context->BaClassSize * sizeof(unsigned char);
  IFn(lang_context->BaClass=(unsigned char *)malloc(size))
  {
    char erreur[DOgErrorSize];
    sprintf(erreur, "OgPhoInit: malloc error on BaClass (%d bytes)", size);
    OgErr(ctrl_pho->herr, erreur);
    DPcErr;
  }

  lang_context->max_steps = 1;
  memcpy(lang_context->space_character, "\0[", 2);

  if (ctrl_pho->loginfo->trace & DOgPhoTraceRules)
  {
    OgMsg(ctrl_pho->hmsg, "", DOgMsgDestInLog, "Dumping of ha_rules into file ha_rules.aut");
    IFE(OgAufWrite(lang_context->ha_rules,"log/ha_rules.auf"));
    IFE(RulesLog(lang_context,"log/rules_list.txt"));
    IFE(ClassLog(lang_context,"log/class_list.txt"));
  }

  DONE;

}


static og_status PhoParseOgmSsiConf(struct og_ctrl_pho *ctrl_pho)
{
  og_char_buffer conf_file[DPcPathSize];

  // conf directory is conf/phonetic/
  snprintf(conf_file, DPcPathSize, "%s/../%s", ctrl_pho->conf_directory, DOgOgmSsiTxt);

  og_char_buffer value[DPcPathSize];
  og_bool found = OgDipperConfGetVar(conf_file, "phonetic_default_language", value, DPcPathSize);
  IFE(found);
  if (found)
  {
    OgTrimString(value, value);
    ctrl_pho->phonetic_default_language = OgCodeToIso639(value);
    if (ctrl_pho->loginfo->trace & DOgPhoTraceMinimal)
    {
      OgMsg(ctrl_pho->hmsg, "", DOgMsgDestInLog, "phonetic_default_language: %s", value);
    }
  }
  else
  {
    ctrl_pho->phonetic_default_language = DOgPhoneticDefaultLanguage;
  }


  DONE;
}

void PhoLangContextDestroy(struct lang_context *lang_context)
{
  if (!lang_context->ctrl_pho->is_inherited)
  {
    OgAutFlush(lang_context->ha_rules);
    DPcFree(lang_context->Rule);
    DPcFree(lang_context->Char_class);
    DPcFree(lang_context->BaClass);
  }

  DPcFree(lang_context->Matching);
  DPcFree(lang_context->Ba);

}

PUBLIC(int) OgPhoFlush(void *handle)
{
  struct og_ctrl_pho *ctrl_pho = (struct og_ctrl_pho *) handle;

  IFn(ctrl_pho) DONE;

  g_hash_table_destroy(ctrl_pho->lang_context_map);
  IFE(OgHeapFlush(ctrl_pho->lang_context_heap));

  IFE(OgMsgFlush(ctrl_pho->hmsg));

  DPcFree(ctrl_pho);
  DONE;
}

