/*
 *  The Except module for linguistic transformations
 *  Copyright (c) 2010 Pertimm by Patrick Constant
 *  Dev: February 2010
 *  Version 1.0
*/
#include "ltrasmexcept.h"

static int LtrasModuleExcept1(struct og_ltra_module_input *module_input
  , struct og_ltra_trfs *trfs, int Itrf);




void *OgLtrasModuleExceptInit(struct og_ltra_module_param *param)
{
  struct og_ctrl_except *ctrl_except = (struct og_ctrl_except *) malloc(sizeof(struct og_ctrl_except));
  IFn(ctrl_except)
  {
    char erreur[DOgErrorSize];
    sprintf(erreur, "OgLtrasInit: malloc error on ctrl_except");
    OgErr(param->herr, erreur);
    return (0);
  }
  memset(ctrl_except, 0, sizeof(struct og_ctrl_except));

  ctrl_except->herr = param->herr;
  ctrl_except->hltras = param->hltras;
  ctrl_except->hmutex = param->hmutex;
  ctrl_except->cloginfo = param->loginfo;
  ctrl_except->loginfo = &ctrl_except->cloginfo;

  struct og_msg_param msg_param[1];
  memset(msg_param, 0, sizeof(struct og_msg_param));
  msg_param->herr = ctrl_except->herr;
  msg_param->hmutex = ctrl_except->hmutex;
  msg_param->loginfo.trace = DOgMsgTraceMinimal + DOgMsgTraceMemory;
  msg_param->loginfo.where = ctrl_except->loginfo->where;
  msg_param->module_name = "ltra_module_except";
  IFn(ctrl_except->hmsg = OgMsgInit(msg_param))
  return (0);
  IF(OgMsgTuneInherit(ctrl_except->hmsg, param->hmsg))
  return (0);

  ctrl_except->BaSize = DOgLtrasExceptBaSize;
  int size = ctrl_except->BaSize * sizeof(unsigned char);
  IFn(ctrl_except->Ba = (unsigned char *) malloc(size))
  {
    char erreur[DOgErrorSize];
    sprintf(erreur, "OgLtrasInit: malloc error on Ba (%d bytes)", size);
    OgErr(ctrl_except->herr, erreur);
    return (0);
  }

  ctrl_except->hpho = OgLtrasHpho(ctrl_except->hltras);
  ctrl_except->hstm = OgLtrasHstm(ctrl_except->hltras);

  if (param->hmodule_to_inherit)
  {
    struct og_ctrl_except *inheriting_except = (struct og_ctrl_except *) param->hmodule_to_inherit;
    ctrl_except->is_inherited = 1;
    ctrl_except->ha_except = inheriting_except->ha_except;
  }
  else
  {
    struct og_aut_param aut_param[1];
    memset(aut_param, 0, sizeof(struct og_aut_param));
    aut_param->herr = ctrl_except->herr;
    aut_param->hmutex = ctrl_except->hmutex;
    aut_param->loginfo.trace = DOgAutTraceMinimal + DOgAutTraceMemory;
    aut_param->loginfo.where = ctrl_except->loginfo->where;
    aut_param->state_number = 0x1000;
    sprintf(aut_param->name, "ltras_module_except");
    IFn(ctrl_except->ha_except = OgAutInit(aut_param))
    return (0);
    char *WorkingDirectory = OgLtrasWorkingDirectory(ctrl_except->hltras);

    char ltras_except[DPcPathSize];
    if (WorkingDirectory[0]) sprintf(ltras_except, "%s/ling/ltras_except.xml", WorkingDirectory);
    else strcpy(ltras_except, "ling/ltras_except.xml");

    og_status status = OgXmlXsdValidateFile(ctrl_except->hmsg, ctrl_except->herr, WorkingDirectory, ltras_except,
        "ling/xsd/ltras_except.xsd");
    IF (status)
    return (0);

    IF (LtrasModuleExceptReadConf( ctrl_except, ltras_except))
    return (0);
  }

  /** Default value for exception score is 0.99 **/
  ctrl_except->exc_score = 0.99;
  char buffer[DPcPathSize];
  og_bool found = OgLtrasGetParameterValue(ctrl_except->hltras, "exc_score", DPcPathSize, buffer);
  IF (found)
  return (0);
  if (found)
  {
    ctrl_except->exc_score = atof(buffer);
    if (ctrl_except->exc_score > 1.0) ctrl_except->exc_score = 1.0;
    else if (ctrl_except->exc_score < 0.0) ctrl_except->exc_score = 0.0;
  }

  ctrl_except->exc_phonetic_cost_reduction_ratio = 0.5;
  found = OgLtrasGetParameterValue(ctrl_except->hltras, "exc_phonetic_cost_reduction_ratio", DPcPathSize,
      buffer);
  IF (found)
  return (0);
  if (found)
  {
    ctrl_except->exc_phonetic_cost_reduction_ratio = atof(buffer);
    if (ctrl_except->exc_phonetic_cost_reduction_ratio > 1.0) ctrl_except->exc_phonetic_cost_reduction_ratio = 1.0;
    else if (ctrl_except->exc_phonetic_cost_reduction_ratio < 0.0) ctrl_except->exc_phonetic_cost_reduction_ratio = 0.0;
  }

  IF(OgLtrasGetLevenshteinCosts(ctrl_except->hltras, ctrl_except->levenshtein_costs))
  return (0);

  return ((void *) ctrl_except);
}




int OgLtrasModuleExceptFlush(void *handle)
{
struct og_ctrl_except *ctrl_except = (struct og_ctrl_except *)handle;
if (!ctrl_except->is_inherited) {
  IFE(OgAutFlush(ctrl_except->ha_except));
  }
IFE(OgMsgFlush(ctrl_except->hmsg));
DPcFree(ctrl_except->Ba);
DPcFree(ctrl_except);
DONE;
}





int OgLtrasModuleExcept(struct og_ltra_module_input *module_input
    , struct og_ltra_trfs *input, struct og_ltra_trfs **output, ogint64_t *elapsed)
{
  struct og_ctrl_except *ctrl_except = (struct og_ctrl_except *) module_input->handle;
  void *hltras = ctrl_except->hltras;
  struct og_ltra_trfs *trfs;
  char buffer[DPcPathSize];
  int i, found, TrfUsed;
  char *value;

  ogint64_t micro_clock_start=OgMicroClock();

  ctrl_except->use_phonetics = 0;
  IFE(found=OgLtrasGetParameterValue(ctrl_except->hltras,"exc_use_phonetics",DPcPathSize,buffer));
  if (found)
  {
    if (!Ogstricmp(buffer, "yes")) ctrl_except->use_phonetics = 1;
    else if (!Ogstricmp(buffer, "no")) ctrl_except->use_phonetics = 0;
  }
  if (module_input->argc > 1)
  {
    value = module_input->argv[1];
    if (!Ogstricmp(value, "yes")) ctrl_except->use_phonetics = 1;
    else if (!Ogstricmp(value, "no")) ctrl_except->use_phonetics = 0;
  }

  IFE(OgLtrasTrfsDuplicate(hltras, input, &trfs));
  *output = trfs;

  TrfUsed = trfs->TrfUsed;
  for (i = 0; i < TrfUsed; i++)
  {
    IFE(LtrasModuleExcept1(module_input, trfs, i));
  }

  *elapsed=OgMicroClock()-micro_clock_start;

  DONE;
}





static int LtrasModuleExcept1(struct og_ltra_module_input *module_input
    , struct og_ltra_trfs *trfs, int Itrf)
{
  struct og_stm_levenshtein_input_param lev_input_params[1];
  struct og_ctrl_except *ctrl_except = (struct og_ctrl_except *) module_input->handle;
  struct og_ltra_add_trf_input ctinput, *tinput = &ctinput;
  int words_length;
  unsigned char words[DPcPathSize];
  int ibuffer;
  unsigned char buffer[DPcPathSize];
  struct og_pho_output coutput, *output = &coutput;
  struct og_ltra_trf *trf = trfs->Trf + Itrf;
  struct og_pho_input cinput, *input = &cinput;
  int min_post_phonetisation_char_number = 2;
  int ibuf;
  unsigned char buf[DPcPathSize];
  unsigned char out[DPcAutMaxBufferSize + 9];
  oindex states[DPcAutMaxBufferSize + 9];
  int min_phonetisation_char_number = 3;
  int i, c, retour, nstate0, nstate1, iout;
  void *hltras = ctrl_except->hltras;
  struct og_ltra_word *word;
  int added;

  //Keep original string of trfs to compare with transmormation string and
  //to calculate a levenchtein score
  unsigned char origin[DPcPathSize];
  int origin_length = 0;
  IFE(OgLtrasTrfGetOriginalString(hltras, trfs, Itrf, origin, DPcPathSize, &origin_length));

  words_length = 0;
  memset(tinput, 0, sizeof(struct og_ltra_add_trf_input));
  for (i = 0; i < trf->nb_words; i++)
  {
    word = trfs->Word + trf->start_word + i;
    if (i > 0)
    {
      words[words_length++] = 0;
      words[words_length++] = ' ';
    }
    memcpy(words + words_length, trfs->Ba + word->start, word->length);
    words_length += word->length;
  }

  ibuffer = 0;
  buffer[ibuffer++] = 0;
  buffer[ibuffer++] = 'f';
  buffer[ibuffer++] = 0;
  buffer[ibuffer++] = ':';
  memcpy(buffer + ibuffer, words, words_length);
  ibuffer += words_length;
  buffer[ibuffer++] = 0;
  buffer[ibuffer++] = 1;

  word = trfs->Word + trf->start_word + 0;

  //Get score of previous transformation to combine to except score
  //otherwise, since except_score is not a calculated levenshtein score
  //and since except module is terminal, we only keep except score which is high (0.99)
  //and for instance for a del followed by an except, it doesn't make sense

  double previous_module_score = 1-trf->global_score;

  if ((retour = OgAutScanf(ctrl_except->ha_except, ibuffer, buffer, &iout, out, &nstate0, &nstate1, states)))
  {
    do
    {
      // update *trf in inner loop to protect against reallocation
      trf = trfs->Trf + Itrf;

      memset(tinput, 0, sizeof(struct og_ltra_add_trf_input));
      tinput->start = trf->start;
      tinput->length = trf->length;

      struct og_ltra_add_trf_word *new_word = tinput->word + tinput->nb_words;
      new_word->string = out;
      new_word->string_length = iout;
      /** Making sure the exception is not discarding by the frequency ratio (except if frequency ratio > 1) **/
      new_word->frequency = word->frequency;
      new_word->base_frequency = word->base_frequency;
      new_word->start_position = trf->start;
      new_word->length_position = trf->length;
      tinput->nb_words++;
      tinput->module_id = module_input->id;
      //combined score with previous module global score
      tinput->score = 1 - (previous_module_score + (1 - ctrl_except->exc_score));
      tinput->final = 1;
      tinput->from_trf = Itrf;
      IFE(OgLtrasTrfAdd(hltras, trfs, tinput, 0));
    }
    while ((retour = OgAutScann(ctrl_except->ha_except, &iout, out, nstate0, &nstate1, states)));
  }

  if (!ctrl_except->use_phonetics) DONE;

  if (words_length < min_phonetisation_char_number * 2) DONE;
  if (words_length >= DOgStmMaxWordLength) DONE;

  /* We do not phonetize names with digits */
  for (i = 0; i < words_length; i += 2)
  {
    c = (words[i] << 8) + words[i + 1];
    if (OgUniIsdigit(c)) DONE;
  }
  input->iB = words_length;
  input->B = words;

  //changer quand on gère les langues
  input->lang = 34;// fr

  IFE(OgPhonet(ctrl_except->hpho, input, output));
  if (output->iB < min_post_phonetisation_char_number * 2) DONE;

  ibuffer = 0;
  buffer[ibuffer++] = 0;
  buffer[ibuffer++] = 'p';
  buffer[ibuffer++] = 0;
  buffer[ibuffer++] = ':';
  memcpy(buffer + ibuffer, output->B, output->iB);
  ibuffer += output->iB;
  buffer[ibuffer++] = 0;
  buffer[ibuffer++] = 1;

  if (ctrl_except->loginfo->trace & DOgLtrasTraceModuleExc)
  {
    IFE(OgUniToCp(output->iB,output->B,DPcPathSize,&ibuf,buf,DOgCodePageUTF8,0,0));
    OgMsg(ctrl_except->hmsg, "", DOgMsgDestInLog
        , "LtrasModuleExcept1: looking for phonetic form '%s'", buf);
  }

  if ((retour = OgAutScanf(ctrl_except->ha_except, ibuffer, buffer, &iout, out, &nstate0, &nstate1, states)))
  {
    do
    {
      memset(tinput, 0, sizeof(struct og_ltra_add_trf_input));
      tinput->start = trf->start;
      tinput->length = trf->length;

      struct og_ltra_add_trf_word *new_word = tinput->word + tinput->nb_words;
      new_word->string = out;
      new_word->string_length = iout;
      new_word->frequency = 100;
      /** Making sure the exception is not discarding by the frequency ratio (except if frequency ratio > 1) **/
      new_word->base_frequency = word->base_frequency + new_word->frequency;
      tinput->nb_words++;
      tinput->module_id = module_input->id;

      memcpy(lev_input_params, ctrl_except->levenshtein_costs, sizeof(struct og_stm_levenshtein_input_param));

      lev_input_params->insertion_cost *= ctrl_except->exc_phonetic_cost_reduction_ratio;
      lev_input_params->same_letter_insertion_cost *= ctrl_except->exc_phonetic_cost_reduction_ratio;
      lev_input_params->deletion_cost *= ctrl_except->exc_phonetic_cost_reduction_ratio;
      lev_input_params->same_letter_deletion_cost *= ctrl_except->exc_phonetic_cost_reduction_ratio;
      lev_input_params->substitution_cost *= ctrl_except->exc_phonetic_cost_reduction_ratio;
      lev_input_params->accents_substitution_cost *= ctrl_except->exc_phonetic_cost_reduction_ratio;
      lev_input_params->swap_cost *= ctrl_except->exc_phonetic_cost_reduction_ratio;

      double dlevenshtein_distance = OgStmLevenshteinFast(ctrl_except->hstm, origin_length, origin,
              new_word->string_length, new_word->string, lev_input_params);
      IFE(dlevenshtein_distance);

      tinput->score = 1.0 - dlevenshtein_distance;

      IFE(added = OgLtrasTrfAdd(hltras, trfs, tinput, 0));

      if (ctrl_except->loginfo->trace & DOgLtrasTraceModuleExc)
      {
        IFE(OgUniToCp(new_word->string_length,new_word->string,DPcPathSize,&ibuf,buf,DOgCodePageUTF8,0,0));
        OgMsg(ctrl_except->hmsg, "", DOgMsgDestInLog
            , "LtrasModuleExcept1: found '%s' from phonetic form, added=%d", buf, added);
      }

    }
    while ((retour = OgAutScann(ctrl_except->ha_except, &iout, out, nstate0, &nstate1, states)));
  }

  DONE;
}






