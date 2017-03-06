/*
 *  Main function for Linguistic Transformation compile library
 *  Copyright (c) 2009 Pertimm, Inc. by Patrick Constant
 *  Dev : November 2009
 *  Version 1.0
*/
#include "ogm_ltrac.h"



int LtracDicAdd(struct og_ctrl_ltrac *ctrl_ltrac, struct og_ltrac_input *input, struct ltrac_dic_input *dic_input)
{

  int length = 0;
  if (ctrl_ltrac->ha_filter)
  {
    char string[2 * DPcPathSize];
    memcpy(string, dic_input->value, dic_input->value_length);
    string[dic_input->value_length] = 0;
    string[dic_input->value_length + 1] = '|';
    length = dic_input->value_length + 2;
    if (ctrl_ltrac->ha_seen)
    {
      IFE(OgAutAdd(ctrl_ltrac->ha_seen, length, string));
    }
  }

  if (dic_input->frequency < input->min_frequency) DONE;

  if (ctrl_ltrac->ha_filter)
  {
    char string[2 * DPcPathSize];
    if (ctrl_ltrac->loginfo->trace & DOgLtracTraceAdd)
    {
      char buffer[DPcPathSize];
      int ibuffer = 0;
      IFE(OgUniToCp(length,string,DPcPathSize,&ibuffer,buffer,DOgCodePageUTF8,0,0));
      OgMsg(ctrl_ltrac->hmsg, "", DOgMsgDestInLog, "LtracDicAdd: looking for %s", buffer);
    }

    unsigned char out[DPcAutMaxBufferSize + 9];
    oindex states[DPcAutMaxBufferSize + 9];
    int nstate0, nstate1, iout;
    og_status retour = OgAutScanf(ctrl_ltrac->ha_filter, length, string, &iout, out, &nstate0, &nstate1, states);
    IFE(retour);
    if (!retour) DONE;
  }

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
    if (dic_input->frequency >= input->min_frequency_swap)
    {
      /* word_less_1_letter | language_code position frequency word */
      IFE(LtracDicSwapAdd(ctrl_ltrac, dic_input));
    }
  }
  if (input->dictionaries_to_export & DOgLtracDictionaryTypePhon)
  {
    /* phonetic_form | language_code frequency **/
    IFE(LtracDicPhonAdd(ctrl_ltrac, dic_input));
  }


  DONE;
}




int LtracDicInit(struct og_ctrl_ltrac *ctrl_ltrac, struct og_ltrac_input *input)
{
struct og_aut_param caut_param,*aut_param=&caut_param;
struct og_pho_param cpho_param,*pho_param=&cpho_param;

memset(aut_param,0,sizeof(struct og_aut_param));
aut_param->herr=ctrl_ltrac->herr;
aut_param->hmutex=ctrl_ltrac->hmutex;
aut_param->loginfo.trace = DOgAutTraceMinimal+DOgAutTraceMemory;
aut_param->loginfo.where = ctrl_ltrac->loginfo->where;
aut_param->state_number = 0x1000;


if (input->dictionaries_to_export & DOgLtracDictionaryTypeBase) {
  sprintf(aut_param->name,"ltrac base");
  IFn(ctrl_ltrac->ha_base=OgAutInit(aut_param)) DPcErr;
  }
if (input->dictionaries_to_export & DOgLtracDictionaryTypeSwap) {
  sprintf(aut_param->name,"ltrac swap");
  IFn(ctrl_ltrac->ha_swap=OgAutInit(aut_param)) DPcErr;
  }
if (input->dictionaries_to_export & DOgLtracDictionaryTypePhon) {
  sprintf(aut_param->name,"ltrac phon");
  IFn(ctrl_ltrac->ha_phon=OgAutInit(aut_param)) DPcErr;
  memset(pho_param,0,sizeof(struct og_pho_param));
  pho_param->herr=ctrl_ltrac->herr;
  pho_param->hmsg=ctrl_ltrac->hmsg;
  pho_param->hmutex=ctrl_ltrac->hmutex;
  pho_param->loginfo.trace = DOgPhoTraceMinimal+DOgPhoTraceMemory;
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

  if (OgFileExists(pho_param->conf_directory)) {
     IFn(ctrl_ltrac->hpho=OgPhoInit(pho_param)) DPcErr;
     }
  else {
    OgMsg(ctrl_ltrac->hmsg,"",DOgMsgDestInLog
      , "LtracDicInit: impossible to open '%s' phonetic dictionary will not be created",pho_param->conf_filename);
    }
  }


DONE;
}


int LtracDicWrite(struct og_ctrl_ltrac *ctrl_ltrac, struct og_ltrac_input *input)
{
  if (input->dictionaries_to_export & DOgLtracDictionaryTypeBase)
  {
    if (input->dictionaries_minimization & DOgLtracDictionaryTypeBase)
    {
      IFE(OgAum(ctrl_ltrac->ha_base));
    }
    IFE(OgAuf(ctrl_ltrac->ha_base, 0));
    IFE(OgAufWrite(ctrl_ltrac->ha_base, ctrl_ltrac->name_base));
  }
  if (input->dictionaries_to_export & DOgLtracDictionaryTypeSwap)
  {
    if (input->dictionaries_minimization & DOgLtracDictionaryTypeSwap)
    {
      IFE(OgAum(ctrl_ltrac->ha_swap));
    }
    IFE(OgAuf(ctrl_ltrac->ha_swap, 0));
    IFE(OgAufWrite(ctrl_ltrac->ha_swap, ctrl_ltrac->name_swap));
  }
  if (input->dictionaries_to_export & DOgLtracDictionaryTypePhon)
  {
    if (input->dictionaries_minimization & DOgLtracDictionaryTypePhon)
    {
      IFE(OgAum(ctrl_ltrac->ha_phon));
    }
    IFE(OgAuf(ctrl_ltrac->ha_phon, 0));
    IFE(OgAufWrite(ctrl_ltrac->ha_phon, ctrl_ltrac->name_phon));
    IFE(OgPhoFlush(ctrl_ltrac->hpho));
  }
  DONE;
}





int LtracDicFlush(struct og_ctrl_ltrac *ctrl_ltrac, struct og_ltrac_input *input)
{

if (input->dictionaries_to_export & DOgLtracDictionaryTypeBase) {
  IFE(OgAutFlush(ctrl_ltrac->ha_base));
  }
if (input->dictionaries_to_export & DOgLtracDictionaryTypeSwap) {
  IFE(OgAutFlush(ctrl_ltrac->ha_swap));
  }
if (input->dictionaries_to_export & DOgLtracDictionaryTypePhon) {
  IFE(OgAutFlush(ctrl_ltrac->ha_phon));
  }
DONE;
}




