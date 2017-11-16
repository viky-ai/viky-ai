/*
 *  The Lem2 module for linguistic transformations
 *  Copyright (c) 2010 Pertimm by Patrick Constant
 *  Dev: May,September 2010
 *  Version 1.0
*/
#include <logltras.h>
#include <logheap.h>
#include <logldi.h>
#include <logstm.h>


struct og_ctrl_lem2 {
  void *herr,*hmsg,*hltras; ogmutex_t *hmutex;
  struct og_loginfo cloginfo,*loginfo;
  struct og_ldi_input ldi_input[1];
  int check_words_in_dictionary;
  int no_dictionary_frequency;
  int is_inherited;
  void *hldi; int hldi_warned;
  struct og_stm_levenshtein_input_param levenshtein_costs[1];
  void *hstm;
  og_heap hba;
  };


struct og_info_lem2 {
  struct og_ltra_module_input *module_input;
  struct og_ctrl_lem2 *ctrl_lem2;
  struct og_ltra_trfs *trfs;
  int Itrf;
  };




static int LtrasModuleLem21(struct og_ltra_module_input *module_input, struct og_ltra_trfs *trfs, int Itrf);
static int LtrasModuleLem22(void *info, struct og_ldi_output *output);




void *OgLtrasModuleLem2Init(struct og_ltra_module_param *param)
{
struct og_msg_param cmsg_param,*msg_param=&cmsg_param;
struct og_ctrl_lem2 *ctrl_lem2;
char erreur[DOgErrorSize];
double dfrequency;

IFn(ctrl_lem2=(struct og_ctrl_lem2 *)malloc(sizeof(struct og_ctrl_lem2))) {
  sprintf(erreur,"OgLtrasInit: malloc error on ctrl_lem2");
  OgErr(param->herr,erreur); return(0);
  }
memset(ctrl_lem2,0,sizeof(struct og_ctrl_lem2));

ctrl_lem2->herr = param->herr;
ctrl_lem2->hltras = param->hltras;
ctrl_lem2->hmutex = param->hmutex;
ctrl_lem2->cloginfo = param->loginfo;
ctrl_lem2->loginfo = &ctrl_lem2->cloginfo;

memset(msg_param,0,sizeof(struct og_msg_param));
msg_param->herr=ctrl_lem2->herr;
msg_param->hmutex=ctrl_lem2->hmutex;
msg_param->loginfo.trace = DOgMsgTraceMinimal+DOgMsgTraceMemory;
msg_param->loginfo.where = ctrl_lem2->loginfo->where;
msg_param->module_name="ltra_module_lem2";
IFn(ctrl_lem2->hmsg=OgMsgInit(msg_param)) return(0);
IF(OgMsgTuneInherit(ctrl_lem2->hmsg,param->hmsg)) return(0);

ctrl_lem2->hldi = OgLtrasHldi(ctrl_lem2->hltras);
ctrl_lem2->hstm = OgLtrasHstm(ctrl_lem2->hltras);

IFn(ctrl_lem2->hba=OgHeapInit(ctrl_lem2->hmsg,"lem2_ba",sizeof(unsigned char),0x400)) return(0);

/** needs to be called so that we have the right max frequency **/
OgLtrasHaBase(ctrl_lem2->hltras);
OgLtrasGetFrequencyFromNormalizedFrequency(ctrl_lem2->hltras, 0.9, &dfrequency);
ctrl_lem2->no_dictionary_frequency = (int)dfrequency;

IF(OgLtrasGetLevenshteinCosts(ctrl_lem2->hltras, ctrl_lem2->levenshtein_costs)) return(0);

return((void *)ctrl_lem2);
}




int OgLtrasModuleLem2Flush(void *handle)
{
struct og_ctrl_lem2 *ctrl_lem2 = (struct og_ctrl_lem2 *)handle;
IFE(OgHeapFlush(ctrl_lem2->hba));
IFE(OgMsgFlush(ctrl_lem2->hmsg));
DPcFree(ctrl_lem2);
DONE;
}





int OgLtrasModuleLem2(struct og_ltra_module_input *module_input
    , struct og_ltra_trfs *input, struct og_ltra_trfs **output, ogint64_t *elapsed)
{
  struct og_ctrl_lem2 *ctrl_lem2 = (struct og_ctrl_lem2 *) module_input->handle;
  int nb_languages;
  unsigned char languages[DOgLangMax];
  struct og_ldi_dictionaries dictionaries[DOgLangMax];
  struct og_ldi_input *ldi_input = ctrl_lem2->ldi_input;
  void *hltras = ctrl_lem2->hltras;
  struct og_ltra_trfs *trfs;
  char buffer[DPcPathSize];
  int i, TrfUsed1;
  int found, language_code;
  char *value;

  ogint64_t micro_clock_start = OgMicroClock();

  IFn(ctrl_lem2->hldi)
  {
    if (!ctrl_lem2->hldi_warned)
    {
      OgMsg(ctrl_lem2->hmsg, "", DOgMsgDestInLog
          , "OgLtrasModuleLem2: ldi dictionary not initialized, lemmatisation not active");
      ctrl_lem2->hldi_warned = 1;
    }
  }

  /* Checking words in basic dictionary for frequency, frequency
   * of the word in the dictionary is interesting as we choose
   * the best lemmatisation based upon the final_score which
   * includes scores and frequencies. When we do check the words
   * in the dictionary, we calculate real frequency even for basic
   * words. Otherwise, we use a frequency corresponding to
   * a normalized frequency of 0.9. We need to use a frequency
   * as there are otherwise possibilities that the solutions
   * are removed because of a minimum_final_score. */
  ctrl_lem2->check_words_in_dictionary = 1;
  IFE(found=OgLtrasGetParameterValue(ctrl_lem2->hltras,"lem_check_words_in_dictionary",DPcPathSize,buffer));
  if (found)
  {
    if (!Ogstricmp(buffer, "yes")) ctrl_lem2->check_words_in_dictionary = 1;
    else if (!Ogstricmp(buffer, "no")) ctrl_lem2->check_words_in_dictionary = 0;
  }
  if (module_input->argc > 1)
  {
    value = module_input->argv[1];
    if (!Ogstricmp(value, "yes")) ctrl_lem2->check_words_in_dictionary = 1;
    else if (!Ogstricmp(value, "no")) ctrl_lem2->check_words_in_dictionary = 0;
  }

  /** syntax can be lem2 lem2(yes/no) or lem2(yes/no,en,fr,de,...) **/
  nb_languages = 0;
  memset(languages, 0, sizeof(char) * DOgLangMax);
  if (module_input->argc > 2)
  {
    for (i = 2; i < module_input->argc; i++)
    {
      language_code = OgCodeToIso639(module_input->argv[i]);
      if (language_code > 0)
      {
        languages[language_code] = 1;
        nb_languages++;
      }
    }
  }

  memset(ldi_input, 0, sizeof(struct og_ldi_input));
  if (nb_languages <= 0)
  {
    dictionaries[0].linguistic_type = DOgLdiTypeLem2;
    dictionaries[0].language = DOgLangNil;
    dictionaries[0].target_language = DOgLangNil;
    ldi_input->nb_dictionaries = 1;
  }
  else
  {
    ldi_input->nb_dictionaries = 0;
    for (i = 0; i < DOgLangMax; i++)
    {
      if (languages[i])
      {
        dictionaries[ldi_input->nb_dictionaries].linguistic_type = DOgLdiTypeLem2;
        dictionaries[ldi_input->nb_dictionaries].language = i;
        dictionaries[ldi_input->nb_dictionaries].target_language = DOgLangNil;
        ldi_input->nb_dictionaries++;
      }
    }
  }

  ldi_input->codepage = DOgCodePageUnicode;
  ldi_input->answer_codepage = DOgCodePageUnicode;
  ldi_input->dictionaries = dictionaries;

  IFE(OgLtrasTrfsDuplicate(hltras, input, &trfs));
  *output = trfs;

  /** single lemmatisation  **/
  TrfUsed1 = trfs->TrfUsed;
  for (i = 0; i < TrfUsed1; i++)
  {
    IFE(LtrasModuleLem21(module_input, trfs, i));
  }

  if (ctrl_lem2->loginfo->trace & DOgLtrasTraceModuleLem)
  {
    OgMsg(ctrl_lem2->hmsg, "", DOgMsgDestInLog
        , "OgLtrasModuleLem2: after lemmatisation:");
    IFE(OgLtrasTrfsLog(hltras, trfs));
  }

  *elapsed = OgMicroClock() - micro_clock_start;

  DONE;
}




static int LtrasModuleLem21(struct og_ltra_module_input *module_input, struct og_ltra_trfs *trfs, int Itrf)
{
struct og_ctrl_lem2 *ctrl_lem2 = (struct og_ctrl_lem2 *)module_input->handle;
struct og_ldi_input *ldi_input=ctrl_lem2->ldi_input,limited_ldi_input[1];
struct og_ldi_dictionaries dictionaries[DOgLangMax];
struct og_ltra_trf *trf = trfs->Trf+Itrf;
struct og_info_lem2 cinfo,*info=&cinfo;
struct og_ltra_word *word;

if (trf->nb_words > 1) DONE;
word = trfs->Word+trf->start_word;

memset(info,0,sizeof(struct og_info_lem2));
info->module_input=module_input;
info->ctrl_lem2=ctrl_lem2;
info->trfs=trfs;
info->Itrf=Itrf;

if (word->language) {
  ldi_input = limited_ldi_input;
  memcpy(ldi_input,ctrl_lem2->ldi_input,sizeof(struct og_ldi_input));
  ldi_input->dictionaries=dictionaries;
  ldi_input->nb_dictionaries=1;
  dictionaries[0].linguistic_type = DOgLdiTypeLem2;
  dictionaries[0].language=word->language;
  dictionaries[0].target_language=DOgLangNil;
  }

/* trfs->Ba can be reallocated in LtrasModuleLem12 */
IFE(OgHeapResetWithoutReduce(ctrl_lem2->hba));
IFE(OgHeapAppend(ctrl_lem2->hba,word->length,trfs->Ba+word->start));
IFn(ldi_input->content=OgHeapGetCell(ctrl_lem2->hba,0)) DPcErr;
ldi_input->content_length=word->length;

IFN(ctrl_lem2->hldi) CONT;

IFE(OgLdiFind(ctrl_lem2->hldi,ldi_input,LtrasModuleLem22,info));

DONE;
}





static int LtrasModuleLem22(void *handle, struct og_ldi_output *output)
{
  struct og_info_lem2 *info = (struct og_info_lem2 *) handle;
  struct og_ltra_add_trf_input ctinput, *tinput = &ctinput;
  struct og_ltra_trf *trf = info->trfs->Trf + info->Itrf;
  struct og_ctrl_lem2 *ctrl_lem2 = info->ctrl_lem2;
  int Itrf_basic;
  int iscontent;
  char scontent[DPcPathSize];
  struct og_ltra_trfs *trfs = info->trfs;
  struct og_ltra_word *word;
  struct og_ltra_add_trf_word *new_word;
  int isword;
  char sword[DPcPathSize];
  struct og_ltra_trf *trf_tmp, *ntrf;
  double dlevenshtein_distance;
  char buffer[DPcPathSize];
  int i, Intrf, added, found;
  double score = 0;

  //Keep original string of trfs to compare with transmormation string and
  //to calculate a levenchtein score
  unsigned char origin[DPcPathSize];
  int origin_length = 0;
  void *hltras = ctrl_lem2->hltras;
  IFE(OgLtrasTrfGetOriginalString(hltras, info->trfs, info->Itrf, origin, DPcPathSize, &origin_length));

  word = trfs->Word + trf->start_word;

  if (ctrl_lem2->loginfo->trace & DOgLtrasTraceModuleLem)
  {
    IFE(OgUniToCp(output->content_length,output->content,DPcPathSize,&iscontent,scontent,DOgCodePageUTF8,0,0));
    IFE(OgUniToCp(word->length,trfs->Ba+word->start,DPcPathSize,&isword,sword,DOgCodePageUTF8,0,0));
    sprintf(buffer, "LtrasModuleLem22: found (%s) '%s' > '%s' from '%s'"
        , OgIso639ToCode(output->language)
            , OgLdiDictTypeToStr(output->linguistic_type)
            , scontent, sword);
    OgMsg(ctrl_lem2->hmsg, "", DOgMsgDestInLog, "%s", buffer);
  }

  Itrf_basic = (-1);
  if (info->Itrf < trfs->TrfBasicUsed) Itrf_basic = info->Itrf;
  else
  {
    for (i = 0; i < trfs->TrfBasicUsed; i++)
    {
      trf_tmp = trfs->Trf + i;
      if (trf->start != trf_tmp->start) continue;
      if (trf->length != trf_tmp->length) continue;
      Itrf_basic = i;
      break;
    }
  }
  /** this should not happen **/
  if (Itrf_basic < 0) DONE;

   IFE(dlevenshtein_distance=OgStmLevenshteinFast(ctrl_lem2->hstm
     , origin_length, origin
     , output->content_length, output->content, ctrl_lem2->levenshtein_costs));
  /* We devide the distance by 10 so that score is high enough
   * to be above minimum score, otherwise, some lemmatisations
   * will be removed without reason */
  score = 1.0 - (dlevenshtein_distance * 0.1);

  memset(tinput, 0, sizeof(struct og_ltra_add_trf_input));
  tinput->start = trf->start;
  tinput->length = trf->length;
  new_word = tinput->word + tinput->nb_words;
  new_word->string = output->content;
  new_word->string_length = output->content_length;
  new_word->start_position = trf->start;
  new_word->length_position = trf->length;
  new_word->language = output->language;
  if (ctrl_lem2->check_words_in_dictionary)
  {
    IFE(
        found = OgLtrasTrfCalculateFrequency(ctrl_lem2->hltras, new_word->string_length, new_word->string, new_word->language,
            &new_word->frequency));
    if (!found) DONE;
  }
  else
  {
    new_word->frequency = ctrl_lem2->no_dictionary_frequency;
  }
  new_word->base_frequency = word->base_frequency;
  tinput->nb_words++;
  tinput->module_id = info->module_input->id;
  tinput->language = output->language;
  tinput->score = score;
  tinput->final = 1;
  IFE(added = OgLtrasTrfAdd(ctrl_lem2->hltras, trfs, tinput, &Intrf));
  if (added)
  {
    ntrf = trfs->Trf + Intrf;
    ntrf->span_start_trf = Itrf_basic;
    ntrf->span_nb_trfs = 1;
    IFE(OgLtrasTrfCalculateScoresFromTrf(ctrl_lem2->hltras, trfs, Intrf, ctrl_lem2->check_words_in_dictionary
        , &ntrf->word_frequency, &ntrf->expression_frequency, &ntrf->global_score, &ntrf->final_score));
  }

  DONE;
}


