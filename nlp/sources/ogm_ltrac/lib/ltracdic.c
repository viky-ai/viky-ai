/*
 *  Main function for Linguistic Transformation compile library
 *  Copyright (c) 2009 Pertimm, Inc. by Patrick Constant
 *  Dev : November 2009
 *  Version 1.0
 */
#include "ogm_ltrac.h"

int LtracDicAdd(struct og_ctrl_ltrac *ctrl_ltrac, struct ltrac_dic_input *dic_input)
{
  struct og_ltrac_input *input = ctrl_ltrac->input;

  if (dic_input->frequency < input->min_frequency) DONE;

  if (ctrl_ltrac->loginfo->trace & DOgLtracTraceAdd)
  {
    char buffer[DPcPathSize];
    int ibuffer = 0;
    IFE(OgUniToCp(dic_input->value_length,dic_input->value,DPcPathSize,&ibuffer,buffer,DOgCodePageUTF8,0,0));
    OgMsg(ctrl_ltrac->hmsg, "", DOgMsgDestInLog, "LtracAttributesAdd: adding '%s' with frequency %d", buffer,
        dic_input->frequency);
  }

  if (input->dictionaries_to_export & DOgLtracDictionaryTypeBase)
  {
    /* word | language_code frequency **/
    IFE(LtracDicBaseAdd(ctrl_ltrac, dic_input));
  }

  if (input->dictionaries_to_export & DOgLtracDictionaryTypeSwap)
  {
    // limit swap to small words
    if (dic_input->value_length <= DOgLtracSwapMaxWordsSizeUnicode)
    {
      if (dic_input->frequency >= input->min_frequency_swap)
      {
        /* word_less_1_letter | language_code position frequency word */
        IFE(LtracDicSwapAdd(ctrl_ltrac, dic_input));
      }
    }
  }

  if (input->dictionaries_to_export & DOgLtracDictionaryTypePhon)
  {
    /* phonetic_form | language_code frequency **/
    IFE(LtracDicPhonAdd(ctrl_ltrac, dic_input));
  }

  DONE;
}

PUBLIC(int) OgLtracDicInit(void *handle, struct og_ltrac_dictionaries *dictionaries)
{
  struct og_ctrl_ltrac *ctrl_ltrac = (struct og_ctrl_ltrac *) handle;
  struct og_ltrac_input *input = ctrl_ltrac->input;
  struct og_aut_param aut_param[1];

  memset(aut_param, 0, sizeof(struct og_aut_param));
  aut_param->herr = ctrl_ltrac->herr;
  aut_param->hmutex = ctrl_ltrac->hmutex;
  aut_param->loginfo.trace = DOgAutTraceMinimal;
  aut_param->loginfo.where = ctrl_ltrac->loginfo->where;
  aut_param->state_number = 0x10;

  if (input->dictionaries_to_export & DOgLtracDictionaryTypeBase)
  {
    sprintf(aut_param->name, "ltrac base");
    IFn(ctrl_ltrac->ha_base=OgAutInit(aut_param)) DPcErr;
  }
  if (input->dictionaries_to_export & DOgLtracDictionaryTypeSwap)
  {
    sprintf(aut_param->name, "ltrac swap");
    IFn(ctrl_ltrac->ha_swap=OgAutInit(aut_param)) DPcErr;
  }
  if (input->dictionaries_to_export & DOgLtracDictionaryTypePhon)
  {
    sprintf(aut_param->name, "ltrac phon");
    IFn(ctrl_ltrac->ha_phon=OgAutInit(aut_param)) DPcErr;
  }

  dictionaries->ha_base = ctrl_ltrac->ha_base;
  dictionaries->ha_swap = ctrl_ltrac->ha_swap;
  dictionaries->ha_phon = ctrl_ltrac->ha_phon;

  DONE;
}

PUBLIC(int) OgLtracFrequenciesSet(void *handle, int min_frequency, int min_frequency_swap)
{
  struct og_ctrl_ltrac *ctrl_ltrac = (struct og_ctrl_ltrac *) handle;
  struct og_ltrac_input *input = ctrl_ltrac->input;
  input->min_frequency = min_frequency;
  input->min_frequency_swap = min_frequency_swap;
  DONE;
}

PUBLIC(int) OgLtracDicWrite(void *handle)
{
  struct og_ctrl_ltrac *ctrl_ltrac = (struct og_ctrl_ltrac *) handle;
  struct og_ltrac_input *input = ctrl_ltrac->input;

  if (input->dictionaries_to_export & DOgLtracDictionaryTypeBase)
  {
    if (input->dictionaries_minimization & DOgLtracDictionaryTypeBase)
    {
      IFE(OgAum(ctrl_ltrac->ha_base));
    }
    IFE(OgAuf(ctrl_ltrac->ha_base, 0));
    IFE(OgAufClean(ctrl_ltrac->ha_base));
  }

  if (input->dictionaries_to_export & DOgLtracDictionaryTypePhon)
  {
    if (input->dictionaries_minimization & DOgLtracDictionaryTypePhon)
    {
      IFE(OgAum(ctrl_ltrac->ha_phon));
    }
    IFE(OgAuf(ctrl_ltrac->ha_phon, 0));
    IFE(OgAufClean(ctrl_ltrac->ha_phon));
  }

  if (input->dictionaries_to_export & DOgLtracDictionaryTypeSwap)
  {
    if (input->dictionaries_minimization & DOgLtracDictionaryTypeSwap)
    {
      IFE(OgAum(ctrl_ltrac->ha_swap));
    }
    IFE(OgAuf(ctrl_ltrac->ha_swap, 0));
    IFE(OgAufClean(ctrl_ltrac->ha_swap));
  }

  DONE;
}
