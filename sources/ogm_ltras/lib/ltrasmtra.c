/*
 *  The Tra module for linguistic transformations
 *  Copyright (c) 2010 Pertimm by Patrick Constant
 *  Dev: May,September 2010
 *  Version 1.0
*/
#include <logltras.h>
#include <logldi.h>
#include <logstm.h>


struct og_ctrl_tra {
  void *herr,*hmsg,*hltras; ogmutex_t *hmutex;
  struct og_loginfo cloginfo,*loginfo;
  struct og_ldi_input ldi_input[1];
  int check_words_in_dictionary;
  int no_dictionary_frequency;
  int is_inherited;
  void *hldi; int hldi_warned;
  struct og_stm_levenshtein_input_param levenshtein_costs[1];
  void *hstm;
  };


struct og_info_tra {
  struct og_ltra_module_input *module_input;
  struct og_ctrl_tra *ctrl_tra;
  struct og_ltra_trfs *trfs;
  int Itrf;
  };


struct og_tra_translation {
  int language_code;
  int target_language_code;
  };



static int LtrasModuleTra1(struct og_ltra_module_input *module_input, struct og_ltra_trfs *trfs, int Itrf);
static int LtrasModuleTra2(void *info, struct og_ldi_output *output);




void *OgLtrasModuleTraInit(struct og_ltra_module_param *param)
{
struct og_msg_param cmsg_param,*msg_param=&cmsg_param;
struct og_ctrl_tra *ctrl_tra;
char erreur[DOgErrorSize];
double dfrequency;

IFn(ctrl_tra=(struct og_ctrl_tra *)malloc(sizeof(struct og_ctrl_tra))) {
  sprintf(erreur,"OgLtrasModuleTraInit: malloc error on ctrl_tra");
  OgErr(param->herr,erreur); return(0);
  }
memset(ctrl_tra,0,sizeof(struct og_ctrl_tra));

ctrl_tra->herr = param->herr;
ctrl_tra->hltras = param->hltras;
ctrl_tra->hmutex = param->hmutex;
ctrl_tra->cloginfo = param->loginfo;
ctrl_tra->loginfo = &ctrl_tra->cloginfo;

memset(msg_param,0,sizeof(struct og_msg_param));
msg_param->herr=ctrl_tra->herr;
msg_param->hmutex=ctrl_tra->hmutex;
msg_param->loginfo.trace = DOgMsgTraceMinimal+DOgMsgTraceMemory;
msg_param->loginfo.where = ctrl_tra->loginfo->where;
msg_param->module_name="ltra_module_tra";
IFn(ctrl_tra->hmsg=OgMsgInit(msg_param)) return(0);
IF(OgMsgTuneInherit(ctrl_tra->hmsg,param->hmsg)) return(0);

ctrl_tra->hldi = OgLtrasHldi(ctrl_tra->hltras);
ctrl_tra->hstm = OgLtrasHstm(ctrl_tra->hltras);

/** needs to be called so that we have the right max frequency **/
OgLtrasHaBase(ctrl_tra->hltras);
OgLtrasGetFrequencyFromNormalizedFrequency(ctrl_tra->hltras, 0.9, &dfrequency);
ctrl_tra->no_dictionary_frequency = (int)dfrequency;

IF(OgLtrasGetLevenshteinCosts(ctrl_tra->hltras, ctrl_tra->levenshtein_costs)) return(0);

return((void *)ctrl_tra);
}




int OgLtrasModuleTraFlush(void *handle)
{
struct og_ctrl_tra *ctrl_tra = (struct og_ctrl_tra *)handle;
IFE(OgMsgFlush(ctrl_tra->hmsg));
DPcFree(ctrl_tra);
DONE;
}





int OgLtrasModuleTra(struct og_ltra_module_input *module_input
    , struct og_ltra_trfs *input, struct og_ltra_trfs **output, ogint64_t *elapsed)
{
  struct og_ctrl_tra *ctrl_tra = (struct og_ctrl_tra *) module_input->handle;
  int nb_translations;
  struct og_tra_translation translation[DPcPathSize];
  struct og_ldi_dictionaries dictionaries[DOgLangMax];
  struct og_ldi_input *ldi_input = ctrl_tra->ldi_input;
  void *hltras = ctrl_tra->hltras;
  struct og_ltra_trfs *trfs;
  char buffer[DPcPathSize];
  int i, j, found, TrfUsed1;
  char *value;

  ogint64_t micro_clock_start = OgMicroClock();

  IFn(ctrl_tra->hldi)
  {
    if (!ctrl_tra->hldi_warned)
    {
      OgMsg(ctrl_tra->hmsg, "", DOgMsgDestInLog
          , "OgLtrasModuleTra: ldi dictionary not initialized, translation not active");
      ctrl_tra->hldi_warned = 1;
    }
  }

  /* Checking words in basic dictionary for frequency, frequency
   * of the word in the dictionary is interesting as we choose
   * the best translations based upon the final_score which
   * includes scores and frequencies. When we do check the words
   * in the dictionary, we calculate real frequency even for basic
   * words. Otherwise, we use a frequency corresponding to
   * a normalized frequency of 0.9. We need to use a frequency
   * as there are otherwise possibilities that the solutions
   * are removed because of a minimum_final_score. */
  ctrl_tra->check_words_in_dictionary = 1;
  IFE(found=OgLtrasGetParameterValue(ctrl_tra->hltras,"tra_check_words_in_dictionary",DPcPathSize,buffer));
  if (found)
  {
    if (!Ogstricmp(buffer, "yes")) ctrl_tra->check_words_in_dictionary = 1;
    else if (!Ogstricmp(buffer, "no")) ctrl_tra->check_words_in_dictionary = 0;
  }
  if (module_input->argc > 1)
  {
    value = module_input->argv[1];
    if (!Ogstricmp(value, "yes")) ctrl_tra->check_words_in_dictionary = 1;
    else if (!Ogstricmp(value, "no")) ctrl_tra->check_words_in_dictionary = 0;
  }

  /* syntax can be tra tra(yes/no) or tra(yes/no,en-fr,fr-en,de-,...)
   * but, because '-' is changed into ' or ' in LtrasFlowChartParse
   * we finally get "en or fr,fr or en,de or " */
  nb_translations = 0;
  if (module_input->argc > 2)
  {
    for (i = 2; i < module_input->argc; i++)
    {
      int ibuffer, sep = (-1);
      unsigned char *buf;
      strcpy(buffer, module_input->argv[i]);
      ibuffer = strlen(buffer);
      for (j = 0; j < ibuffer; j++)
      {
        if (j + 3 <= ibuffer && !memcmp(buffer + j, " or", 3))
        {
          sep = j;
          break;
        }
      }
      if (sep < 0) continue;
      buffer[sep] = 0;
      OgTrimString(buffer, buffer);
      translation[nb_translations].language_code = OgCodeToIso639(buffer);
      buf = buffer + sep + 3;
      OgTrimString(buf, buf);
      translation[nb_translations].target_language_code = OgCodeToIso639(buf);
      if (translation[nb_translations].language_code > 0) nb_translations++;
    }
  }

  memset(ldi_input, 0, sizeof(struct og_ldi_input));
  memset(dictionaries, 0, sizeof(struct og_ldi_dictionaries) * DOgLangMax);
  if (nb_translations <= 0)
  {
    dictionaries[0].linguistic_type = DOgLdiTypeTran;
    dictionaries[0].language = DOgLangNil;
    dictionaries[0].target_language = DOgLangNil;
    ldi_input->nb_dictionaries = 1;
  }
  else
  {
    ldi_input->nb_dictionaries = 0;
    for (i = 0; i < nb_translations; i++)
    {
      dictionaries[ldi_input->nb_dictionaries].linguistic_type = DOgLdiTypeTran;
      dictionaries[ldi_input->nb_dictionaries].language = translation[i].language_code;
      dictionaries[ldi_input->nb_dictionaries].target_language = translation[i].target_language_code;
      ldi_input->nb_dictionaries++;
    }
  }

  ldi_input->codepage = DOgCodePageUnicode;
  ldi_input->answer_codepage = DOgCodePageUnicode;
  ldi_input->dictionaries = dictionaries;

  IFE(OgLtrasTrfsDuplicate(hltras, input, &trfs));
  *output = trfs;

  /** first lemmatisation, possibly form -> root or root -> form **/
  TrfUsed1 = trfs->TrfUsed;
  for (i = 0; i < TrfUsed1; i++)
  {
    IFE(LtrasModuleTra1(module_input, trfs, i));
  }

  if (ctrl_tra->loginfo->trace & DOgLtrasTraceModuleTra)
  {
    OgMsg(ctrl_tra->hmsg, "", DOgMsgDestInLog
        , "OgLtrasModuleTra: after translation:");
    IFE(OgLtrasTrfsLog(hltras, trfs));
  }

  *elapsed = OgMicroClock() - micro_clock_start;

  DONE;
}




static int LtrasModuleTra1(struct og_ltra_module_input *module_input, struct og_ltra_trfs *trfs, int Itrf)
{
struct og_ctrl_tra *ctrl_tra = (struct og_ctrl_tra *)module_input->handle;
struct og_ldi_input *ldi_input=ctrl_tra->ldi_input,limited_ldi_input[1];
struct og_ldi_dictionaries dictionaries[DOgLangMax];
struct og_ltra_trf *trf = trfs->Trf+Itrf;
struct og_info_tra cinfo,*info=&cinfo;
struct og_ltra_word *word;

if (trf->nb_words > 1) DONE;
word = trfs->Word+trf->start_word;

memset(info,0,sizeof(struct og_info_tra));
info->module_input=module_input;
info->ctrl_tra=ctrl_tra;
info->trfs=trfs;
info->Itrf=Itrf;

if (word->language) {
  ldi_input = limited_ldi_input;
  memcpy(ldi_input,ctrl_tra->ldi_input,sizeof(struct og_ldi_input));
  ldi_input->dictionaries=dictionaries;
  ldi_input->nb_dictionaries=1;
  dictionaries[0].linguistic_type = DOgLdiTypeTran;
  dictionaries[0].language=word->language;
  dictionaries[0].target_language=DOgLangNil;
  }

ldi_input->content=trfs->Ba+word->start;
ldi_input->content_length=word->length;

IFN(ctrl_tra->hldi) CONT;

IFE(OgLdiFind(ctrl_tra->hldi,ldi_input,LtrasModuleTra2,info));

DONE;
}





static int LtrasModuleTra2(void *handle, struct og_ldi_output *output)
{
struct og_info_tra *info = (struct og_info_tra *) handle;
struct og_ltra_add_trf_input ctinput,*tinput=&ctinput;
struct og_ltra_trf *trf = info->trfs->Trf+info->Itrf;
struct og_ctrl_tra *ctrl_tra = info->ctrl_tra;
int Itrf_basic; struct og_ltra_trf *trf_basic;
int iscontent; char scontent[DPcPathSize];
struct og_ltra_trfs *trfs = info->trfs;
struct og_ltra_word *word,*word_basic;
struct og_ltra_add_trf_word *new_word;
int isword; char sword[DPcPathSize];
struct og_ltra_trf *trf_tmp, *ntrf;
double dlevenshtein_distance;
char buffer[DPcPathSize];
int i,Intrf,added,found;
double score=0;

word = trfs->Word+trf->start_word;

if (ctrl_tra->loginfo->trace & DOgLtrasTraceModuleTra) {
  IFE(OgUniToCp(output->content_length,output->content,DPcPathSize,&iscontent,scontent,DOgCodePageUTF8,0,0));
  IFE(OgUniToCp(word->length,trfs->Ba+word->start,DPcPathSize,&isword,sword,DOgCodePageUTF8,0,0));
  sprintf(buffer,"LtrasModuleTra2: found (%s) '%s' > '%s' from '%s'"
    , OgIso639ToCode(output->language)
    , OgLdiDictTypeToStr(output->linguistic_type)
    , scontent, sword);
  OgMsg(ctrl_tra->hmsg,"",DOgMsgDestInLog,"%s",buffer);
  }

Itrf_basic = (-1);
if (info->Itrf < trfs->TrfBasicUsed) Itrf_basic=info->Itrf;
else {
  for (i=0; i<trfs->TrfBasicUsed; i++) {
    trf_tmp = trfs->Trf + i;
    if (trf->start != trf_tmp->start) continue;
    if (trf->length != trf_tmp->length) continue;
    Itrf_basic = i; break;
    }
  }
/** this should not happen **/
if (Itrf_basic < 0) DONE;

trf_basic = trfs->Trf + Itrf_basic;
word_basic = trfs->Word+trf_basic->start_word;
IFE(dlevenshtein_distance=OgStmLevenshteinFast(ctrl_tra->hstm
  , word_basic->length, trfs->Ba+word_basic->start
  , output->content_length, output->content, ctrl_tra->levenshtein_costs));
/* We devide the distance by 10 so that score is high enough
 * to be above minimum score, otherwise, some lemmatisations
 * will be removed without reason */
score=1.0-(dlevenshtein_distance*0.1);

memset(tinput,0,sizeof(struct og_ltra_add_trf_input));
tinput->start = trf->start; tinput->length = trf->length;
new_word = tinput->word+tinput->nb_words;
new_word->string = output->content;
new_word->string_length = output->content_length;
new_word->start_position = trf->start;
new_word->length_position = trf->length;
new_word->language = trf->language;
if (ctrl_tra->check_words_in_dictionary) {
  IFE(found=OgLtrasTrfCalculateFrequency(ctrl_tra->hltras, new_word->string_length, new_word->string, new_word->language, &new_word->frequency));
  if (!found) DONE;
  }
else {
  new_word->frequency = ctrl_tra->no_dictionary_frequency;
  }
new_word->base_frequency = word->base_frequency;
new_word->language = output->language;
tinput->nb_words++;
tinput->module_id=info->module_input->id;
tinput->language = output->language;
tinput->score=score;
tinput->final = 1;
IFE(added=OgLtrasTrfAdd(ctrl_tra->hltras,trfs,tinput,&Intrf));
if (added) {
  ntrf = trfs->Trf + Intrf;
  ntrf->span_start_trf=Itrf_basic;
  ntrf->span_nb_trfs=1;
  IFE(OgLtrasTrfCalculateGlobal(ctrl_tra->hltras,trfs,Intrf
    ,&ntrf->global_frequency, &ntrf->global_score, &ntrf->final_score));
  }

DONE;
}


