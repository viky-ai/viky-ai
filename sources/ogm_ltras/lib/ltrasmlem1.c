/*
 *  The Lem1 module for linguistic transformations
 *  Copyright (c) 2010 Pertimm by Patrick Constant
 *  Dev: May,September 2010
 *  Version 1.0
*/
#include <logltras.h>
#include <logheap.h>
#include <logldi.h>
#include <logstm.h>


struct og_ctrl_lem1 {
  void *herr,*hmsg,*hltras; ogmutex_t *hmutex;
  struct og_loginfo cloginfo,*loginfo;
  struct og_ldi_input ldi_input[1];
  int check_words_in_dictionary;
  int no_dictionary_frequency;
  int is_inherited;
  void *hldi; int hldi_warned;
  struct og_stm_levenshtein_input_param levenshtein_costs[1];
  void *hstm;
  int compound_prefix_length;
  int compound_prefix_length_activated;
  og_heap hba;
  };


struct og_info_lem1 {
  struct og_ltra_module_input *module_input;
  struct og_ctrl_lem1 *ctrl_lem1;
  struct og_ltra_trfs *trfs;
  int Itrf;
  };




static int LtrasModuleLem11(struct og_ltra_module_input *module_input, struct og_ltra_trfs *trfs, int Itrf);
static int LtrasModuleLem12(void *info, struct og_ldi_output *output);
static int LtrasModuleLem1Clean(struct og_ltra_module_input *module_input, struct og_ltra_trfs *trfs, int TrfUsed1);




void *OgLtrasModuleLem1Init(struct og_ltra_module_param *param)
{
struct og_msg_param cmsg_param,*msg_param=&cmsg_param;
struct og_ctrl_lem1 *ctrl_lem1;
char erreur[DOgErrorSize];
double dfrequency;

IFn(ctrl_lem1=(struct og_ctrl_lem1 *)malloc(sizeof(struct og_ctrl_lem1))) {
  sprintf(erreur,"OgLtrasModuleLem1Init: malloc error on ctrl_lem1");
  OgErr(param->herr,erreur); return(0);
  }
memset(ctrl_lem1,0,sizeof(struct og_ctrl_lem1));

ctrl_lem1->herr = param->herr;
ctrl_lem1->hltras = param->hltras;
ctrl_lem1->hmutex = param->hmutex;
ctrl_lem1->cloginfo = param->loginfo;
ctrl_lem1->loginfo = &ctrl_lem1->cloginfo;

memset(msg_param,0,sizeof(struct og_msg_param));
msg_param->herr=ctrl_lem1->herr;
msg_param->hmutex=ctrl_lem1->hmutex;
msg_param->loginfo.trace = DOgMsgTraceMinimal+DOgMsgTraceMemory;
msg_param->loginfo.where = ctrl_lem1->loginfo->where;
msg_param->module_name="ltra_module_lem1";
IFn(ctrl_lem1->hmsg=OgMsgInit(msg_param)) return(0);
IF(OgMsgTuneInherit(ctrl_lem1->hmsg,param->hmsg)) return(0);

ctrl_lem1->hldi = OgLtrasHldi(ctrl_lem1->hltras);
ctrl_lem1->hstm = OgLtrasHstm(ctrl_lem1->hltras);

IFn(ctrl_lem1->hba=OgHeapInit(ctrl_lem1->hmsg,"lem1_ba",sizeof(unsigned char),0x400)) return(0);

/** needs to be called so that we have the right max frequency **/
OgLtrasHaBase(ctrl_lem1->hltras);
OgLtrasGetFrequencyFromNormalizedFrequency(ctrl_lem1->hltras, 0.9, &dfrequency);
ctrl_lem1->no_dictionary_frequency = (int)dfrequency;

IF(OgLtrasGetLevenshteinCosts(ctrl_lem1->hltras, ctrl_lem1->levenshtein_costs)) return(0);

return((void *)ctrl_lem1);
}




int OgLtrasModuleLem1Flush(void *handle)
{
struct og_ctrl_lem1 *ctrl_lem1 = (struct og_ctrl_lem1 *)handle;
IFE(OgHeapFlush(ctrl_lem1->hba));
IFE(OgMsgFlush(ctrl_lem1->hmsg));
DPcFree(ctrl_lem1);
DONE;
}





int OgLtrasModuleLem1(struct og_ltra_module_input *module_input
    , struct og_ltra_trfs *input, struct og_ltra_trfs **output, ogint64_t *elapsed)
{
  struct og_ctrl_lem1 *ctrl_lem1 = (struct og_ctrl_lem1 *) module_input->handle;
  int nb_languages;
  unsigned char languages[DOgLangMax];
  struct og_ldi_dictionaries dictionaries[DOgLangMax];
  struct og_ldi_input *ldi_input = ctrl_lem1->ldi_input;
  void *hltras = ctrl_lem1->hltras;
  struct og_ltra_trfs *trfs;
  char buffer[DPcPathSize];
  int i, TrfUsed1, TrfUsed2;
  int found, language_code;
  char *value;

  ogint64_t micro_clock_start = OgMicroClock();

  IFn(ctrl_lem1->hldi)
  {
    if (!ctrl_lem1->hldi_warned)
    {
      OgMsg(ctrl_lem1->hmsg, "", DOgMsgDestInLog
          , "OgLtrasModuleLem1: ldi dictionary not initialized, lemmatisation not active");
      ctrl_lem1->hldi_warned = 1;
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
  ctrl_lem1->check_words_in_dictionary = 1;
  IFE(found=OgLtrasGetParameterValue(ctrl_lem1->hltras,"lem_check_words_in_dictionary",DPcPathSize,buffer));
  if (found)
  {
    if (!Ogstricmp(buffer, "yes")) ctrl_lem1->check_words_in_dictionary = 1;
    else if (!Ogstricmp(buffer, "no")) ctrl_lem1->check_words_in_dictionary = 0;
  }
  if (module_input->argc > 1)
  {
    value = module_input->argv[1];
    if (!Ogstricmp(value, "yes")) ctrl_lem1->check_words_in_dictionary = 1;
    else if (!Ogstricmp(value, "no")) ctrl_lem1->check_words_in_dictionary = 0;
  }

  /** syntax can be lem1 lem1(yes/no) or lem1(yes/no,en,fr,de,...) **/
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
    dictionaries[0].linguistic_type = DOgLdiTypeLem1;
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
        dictionaries[ldi_input->nb_dictionaries].linguistic_type = DOgLdiTypeLem1;
        dictionaries[ldi_input->nb_dictionaries].language = i;
        dictionaries[ldi_input->nb_dictionaries].target_language = DOgLangNil;
        ldi_input->nb_dictionaries++;
      }
    }
  }

  ldi_input->codepage = DOgCodePageUnicode;
  ldi_input->answer_codepage = DOgCodePageUnicode;
  ldi_input->dictionaries = dictionaries;

  ctrl_lem1->compound_prefix_length_activated = 0;

  IFE(OgLtrasTrfsDuplicate(hltras, input, &trfs));
  *output = trfs;

  /** first lemmatisation, possibly form -> root or root -> form **/
  TrfUsed1 = trfs->TrfUsed;
  for (i = 0; i < TrfUsed1; i++)
  {
    IFE(LtrasModuleLem11(module_input, trfs, i));
  }

  if (ctrl_lem1->loginfo->trace & DOgLtrasTraceModuleLem)
  {
    OgMsg(ctrl_lem1->hmsg, "", DOgMsgDestInLog
        , "OgLtrasModuleLem1: after first lemmatisation:");
    IFE(OgLtrasTrfsLog(hltras, trfs));
  }

  if (ctrl_lem1->compound_prefix_length_activated)
  {
    IFE(LtrasModuleLem1Clean(module_input, trfs, TrfUsed1));

    if (ctrl_lem1->loginfo->trace & DOgLtrasTraceModuleLem)
    {
      OgMsg(ctrl_lem1->hmsg, "", DOgMsgDestInLog
          , "OgLtrasModuleLem1: after first lemmatisation cleaning:");
      IFE(OgLtrasTrfsLog(hltras, trfs));
    }
  }

  /* second lemmatisation for new trfs, necessary when initial
   * form is not a root, e.g. : travels -> travel, and then
   * travel -> traveled, traveling */
  TrfUsed2 = trfs->TrfUsed;
  for (i = TrfUsed1; i < TrfUsed2; i++)
  {
    IFE(LtrasModuleLem11(module_input, trfs, i));
  }

  if (ctrl_lem1->loginfo->trace & DOgLtrasTraceModuleLem)
  {
    OgMsg(ctrl_lem1->hmsg, "", DOgMsgDestInLog
        , "OgLtrasModuleLem1: after second lemmatisation:");
    IFE(OgLtrasTrfsLog(hltras, trfs));
  }

  *elapsed = OgMicroClock() - micro_clock_start;

  DONE;
}




static int LtrasModuleLem11(struct og_ltra_module_input *module_input, struct og_ltra_trfs *trfs, int Itrf)
{
struct og_ctrl_lem1 *ctrl_lem1 = (struct og_ctrl_lem1 *)module_input->handle;
struct og_ldi_input *ldi_input=ctrl_lem1->ldi_input,limited_ldi_input[1];
struct og_ldi_dictionaries dictionaries[DOgLangMax];
struct og_ltra_trf *trf = trfs->Trf+Itrf;
struct og_info_lem1 cinfo,*info=&cinfo;
struct og_ltra_word *word;

if (trf->nb_words > 1) DONE;
word = trfs->Word+trf->start_word;

memset(info,0,sizeof(struct og_info_lem1));
info->module_input=module_input;
info->ctrl_lem1=ctrl_lem1;
info->trfs=trfs;
info->Itrf=Itrf;

if (word->language) {
  ldi_input = limited_ldi_input;
  memcpy(ldi_input,ctrl_lem1->ldi_input,sizeof(struct og_ldi_input));
  ldi_input->dictionaries=dictionaries;
  ldi_input->nb_dictionaries=1;
  dictionaries[0].linguistic_type = DOgLdiTypeLem1;
  dictionaries[0].language=word->language;
  dictionaries[0].target_language=DOgLangNil;
  }

/* trfs->Ba can be reallocated in LtrasModuleLem12 */
IFE(OgHeapResetWithoutReduce(ctrl_lem1->hba));
IFE(OgHeapAppend(ctrl_lem1->hba,word->length,trfs->Ba+word->start));
IFn(ldi_input->content=OgHeapGetCell(ctrl_lem1->hba,0)) DPcErr;
ldi_input->content_length=word->length;


if (ctrl_lem1->compound_prefix_length_activated) {
  ldi_input->compound_prefix_length_activated=1;
  ldi_input->compound_prefix_length=ctrl_lem1->compound_prefix_length;
  }

IFN(ctrl_lem1->hldi) CONT;

IFE(OgLdiFind(ctrl_lem1->hldi,ldi_input,LtrasModuleLem12,info));

DONE;
}





static int LtrasModuleLem12(void *handle, struct og_ldi_output *output)
{
  struct og_info_lem1 *info = (struct og_info_lem1 *) handle;
  struct og_ltra_add_trf_input ctinput, *tinput = &ctinput;
  struct og_ltra_trf *trf = info->trfs->Trf + info->Itrf;
  struct og_ctrl_lem1 *ctrl_lem1 = info->ctrl_lem1;
  int Itrf_basic;
  int iscontent;
  char scontent[DPcPathSize];
  struct og_ltra_trfs *trfs = info->trfs;
  struct og_ltra_word *word;
  struct og_ltra_add_trf_word *new_word;
  int isword;
  char sword[DPcPathSize];
  char buffer[DPcPathSize];
  int Intrf, added;
  double score = 0;

   //Keep original string of trfs to compare with transmormation string and
   //to calculate a levenchtein score
  unsigned char origin[DPcPathSize];
  int origin_length = 0;
  void *hltras = ctrl_lem1->hltras;
  IFE(OgLtrasTrfGetOriginalString(hltras, info->trfs, info->Itrf, origin, DPcPathSize, &origin_length));

  word = trfs->Word + trf->start_word;

  if (ctrl_lem1->loginfo->trace & DOgLtrasTraceModuleLem)
  {
    IFE(OgUniToCp(output->content_length,output->content,DPcPathSize,&iscontent,scontent,DOgCodePageUTF8,0,0));
    IFE(OgUniToCp(word->length,trfs->Ba+word->start,DPcPathSize,&isword,sword,DOgCodePageUTF8,0,0));
    sprintf(buffer, "LtrasModuleLem12: found (%s) '%s' > '%s' from '%s'"
        , OgIso639ToCode(output->language)
            , OgLdiDictTypeToStr(output->linguistic_type)
            , scontent, sword);
    OgMsg(ctrl_lem1->hmsg, "", DOgMsgDestInLog, "%s", buffer);
  }

  Itrf_basic = (-1);
  if (info->Itrf < trfs->TrfBasicUsed) Itrf_basic = info->Itrf;
  else
  {
    for (int i = 0; i < trfs->TrfBasicUsed; i++)
    {
      struct og_ltra_trf *trf_tmp = trfs->Trf + i;
      if (trf->start != trf_tmp->start) continue;
      if (trf->length != trf_tmp->length) continue;
      Itrf_basic = i;
      break;
    }
  }
  /** this should not happen **/
  if (Itrf_basic < 0) DONE;

  double dlevenshtein_distance = OgStmLevenshteinFast(ctrl_lem1->hstm, origin_length, origin, output->content_length,
      output->content, ctrl_lem1->levenshtein_costs);
  IFE(dlevenshtein_distance);

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
  if (ctrl_lem1->check_words_in_dictionary)
  {
    og_bool found = OgLtrasTrfCalculateFrequency(ctrl_lem1->hltras, new_word->string_length, new_word->string, new_word->language,
            &new_word->frequency);
    IFE(found);
    if (!found) DONE;
  }
  else
  {
    new_word->frequency = ctrl_lem1->no_dictionary_frequency;
  }
  new_word->base_frequency = word->base_frequency;
  tinput->nb_words++;
  tinput->module_id = info->module_input->id;
  tinput->language = output->language;
  tinput->score = score;
  tinput->final = 1;
  IFE(added = OgLtrasTrfAdd(ctrl_lem1->hltras, trfs, tinput, &Intrf));
  if (added)
  {
    struct og_ltra_trf *ntrf = trfs->Trf + Intrf;
    ntrf->span_start_trf = Itrf_basic;
    ntrf->span_nb_trfs = 1;
    IFE(OgLtrasTrfCalculateGlobal(ctrl_lem1->hltras, trfs, Intrf
        , &ntrf->global_frequency, &ntrf->global_score, &ntrf->final_score));
    ntrf->compound_prefix_length = output->compound_prefix_length;
    if (ctrl_lem1->compound_prefix_length_activated)
    {
      /*  tut is a form of tun, thus we keep only the small prefixes
       *  $ ogldi -l de schlankheitsinstitut
       *  (de) root > schlankheitsinstitun [17]
       *  (de) form > schlankheitsinstitute [12]
       *  (de) form > schlankheitsinstituten [12]
       *  (de) form > schlankheitsinstituts [12]
       */
      int compound_prefix_length = output->compound_prefix_length;
      if (compound_prefix_length < 0) compound_prefix_length = 0;
      if (ctrl_lem1->compound_prefix_length > compound_prefix_length)
      ctrl_lem1->compound_prefix_length = compound_prefix_length;
    }
    else
    {
      ctrl_lem1->compound_prefix_length_activated = 1;
      if (output->compound_prefix_length < 0) ctrl_lem1->compound_prefix_length = 0;
      else ctrl_lem1->compound_prefix_length = output->compound_prefix_length;
    }
  }

  DONE;
}




static int LtrasModuleLem1Clean(struct og_ltra_module_input *module_input, struct og_ltra_trfs *trfs, int TrfUsed1)
{
struct og_ctrl_lem1 *ctrl_lem1 = (struct og_ctrl_lem1 *)module_input->handle;
struct og_ltra_trf *trf;
int i,j,TrfUsed2;

if (!ctrl_lem1->compound_prefix_length_activated) DONE;

TrfUsed2=trfs->TrfUsed;
for (i=j=TrfUsed1; i<TrfUsed2; i++) {
  trf = trfs->Trf+i;
  if (j!=i) {
    memcpy(trfs->Trf+j,trfs->Trf+i,sizeof(struct og_ltra_trf));
    }
  if (trf->compound_prefix_length <= ctrl_lem1->compound_prefix_length) j++;
  }

trfs->TrfUsed=j;
DONE;
}


