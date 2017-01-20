/*
 *  The Term module for linguistic transformations
 *  Copyright (c) 2010 Pertimm by Patrick Constant
 *  Dev: January 2010
 *  Version 1.0
*/
#include <logltras.h>
#include <logheap.h>
#include <logaut.h>

#define MaxNbWords 1024

#define DOg_Unicode_DPcPathSize (DPcPathSize+4)*2

struct og_ctrl_term {
  void *herr,*hmsg,*hltras; ogmutex_t *hmutex;
  struct og_loginfo cloginfo,*loginfo;
  int max_nb_generated_solutions,nb_generated_solutions;
  int max_nb_solutions,nb_solutions;
  int maximum_transformation_length;
  int *maximum_transformation;
  double max_word_frequency_log10;
  int check_words_in_dictionary;
  int no_dictionary_frequency;
  int max_word_frequency;
  double score_factor;
  int min_frequency;
  og_heap hfrom;
  og_heap hto;
  void *ha_base;
  int TrfUsed;
  void *hstm;
  struct og_stm_levenshtein_input_param levenshtein_costs[1];
  };


static int LtrasModuleTerm1(struct og_ltra_module_input *module_input
  , struct og_ltra_trfs *input,  struct og_ltra_trfs *trfs, int Itrf);
static int LtrasModuleTerm2(struct og_ltra_module_input *module_input
  , struct og_ltra_trfs *input,  struct og_ltra_trfs *trfs, int Itrf);
static int trf_clean_cmp(const void *ptr1, const void *ptr2);
static int trf_cmp(const void *ptr1, const void *ptr2);
static int trf_final_cmp(const void *ptr1, const void *ptr2);
static int LtrasModuleTermClean(struct og_ltra_module_input *module_input
  , struct og_ltra_trfs *trfs, int Itrf);
static int LtrasModuleTermClean1(struct og_ltra_module_input *module_input
  , struct og_ltra_trfs *trfs, int Itrf_basic, int Itrf, int *pnb_solutions);
static int LtrasModuleTerm3(struct og_ltra_module_input *module_input
  , struct og_ltra_trfs *trfs, int span_start_trf, int *counter_for_safety);
static int LtrasModuleTermIsFalse(struct og_ltra_module_input *module_input
  , struct og_ltra_trfs *trfs, struct og_ltra_add_trf_input *input);
static int LtrasModuleTermStopExplosion(struct og_ctrl_term *ctrl_term);




void *OgLtrasModuleTermInit(struct og_ltra_module_param *param)
{
struct og_msg_param cmsg_param,*msg_param=&cmsg_param;
struct og_ctrl_term *ctrl_term;
char erreur[DOgErrorSize];
double dfrequency;

IFn(ctrl_term=(struct og_ctrl_term *)malloc(sizeof(struct og_ctrl_term))) {
  sprintf(erreur,"OgLtrasInit: malloc error on ctrl_term");
  OgErr(param->herr,erreur); return(0);
  }
memset(ctrl_term,0,sizeof(struct og_ctrl_term));

ctrl_term->herr = param->herr;
ctrl_term->hltras = param->hltras;
ctrl_term->hmutex = param->hmutex;
ctrl_term->cloginfo = param->loginfo;
ctrl_term->loginfo = &ctrl_term->cloginfo;

memset(msg_param,0,sizeof(struct og_msg_param));
msg_param->herr=ctrl_term->herr;
msg_param->hmutex=ctrl_term->hmutex;
msg_param->loginfo.trace = DOgMsgTraceMinimal+DOgMsgTraceMemory;
msg_param->loginfo.where = ctrl_term->loginfo->where;
msg_param->module_name="ltra_module_term";
IFn(ctrl_term->hmsg=OgMsgInit(msg_param)) return(0);
IF(OgMsgTuneInherit(ctrl_term->hmsg,param->hmsg)) return(0);

IFn(ctrl_term->hfrom=OgHeapInit(ctrl_term->hmsg,"term_from",sizeof(unsigned char),0x800)) return(0);
IFn(ctrl_term->hto=OgHeapInit(ctrl_term->hmsg,"term_to",sizeof(unsigned char),0x800)) return(0);

IFn(ctrl_term->ha_base=OgLtrasHaBase(ctrl_term->hltras)) {
  IFn(param->hmodule_to_inherit) {
    OgMsgErr(ctrl_term->hmsg,"term_module",0,0,0,DOgMsgSeverityNotice
      ,DOgErrLogFlagNoSystemError+DOgErrLogFlagNotInErr+DOgErrLogFlagNoDate);
    OgMsg(ctrl_term->hmsg,"",DOgMsgDestInLog
      , "OgLtrasModuleTermInit: going on because ltra_base.auf not mandatory");
    }
  else {
    OgErrLast(ctrl_term->herr,erreur,0);
    }
  }
OgLtrasGetFrequencyFromNormalizedFrequency(ctrl_term->hltras, 0.9, &dfrequency);
ctrl_term->no_dictionary_frequency = (int)dfrequency;

ctrl_term->max_word_frequency = OgLtrasMaxWordFrequency(ctrl_term->hltras);
ctrl_term->max_word_frequency_log10=log10(ctrl_term->max_word_frequency);

ctrl_term->hstm = OgLtrasHstm(ctrl_term->hltras);
IF(OgLtrasGetLevenshteinCosts(ctrl_term->hltras, ctrl_term->levenshtein_costs)) return(0);


IF(OgLtrasGetMaximumTransformation(ctrl_term->hltras, &ctrl_term->maximum_transformation_length, &ctrl_term->maximum_transformation))
{
  return NULL;
}

return((void *)ctrl_term);
}




int OgLtrasModuleTermFlush(void *handle)
{
struct og_ctrl_term *ctrl_term = (struct og_ctrl_term *)handle;
IFE(OgMsgFlush(ctrl_term->hmsg));
IFE(OgHeapFlush(ctrl_term->hfrom));
IFE(OgHeapFlush(ctrl_term->hto));
DPcFree(ctrl_term);
DONE;
}





int OgLtrasModuleTerm(struct og_ltra_module_input *module_input
    , struct og_ltra_trfs *input, struct og_ltra_trfs **output, ogint64_t *elapsed)
{
  struct og_ctrl_term *ctrl_term = (struct og_ctrl_term *) module_input->handle;
  void *hltras = ctrl_term->hltras;
  struct og_ltra_trfs *trfs;
  char erreur[DOgErrorSize];
  char buffer[DPcPathSize];
  char *value;
  int i, found;

  ogint64_t micro_clock_start = OgMicroClock();

  ctrl_term->score_factor = OgLtrasScoreFactor(ctrl_term->hltras);

  /** Max number of solutions validated by the term module **/
  ctrl_term->max_nb_solutions = 10;
  IFE(found=OgLtrasGetParameterValue(ctrl_term->hltras,"term_max_nb_solutions",DPcPathSize,buffer));
  if (found)
  {
    ctrl_term->max_nb_solutions = atoi(buffer);
  }
  if (module_input->argc > 1)
  {
    ctrl_term->max_nb_solutions = atoi(module_input->argv[1]);
  }

  /* Checking words in basic dictionary for found solutions
   * when check is enabled, if words are found, solutions are validated
   * else solutions are always validated */
  ctrl_term->check_words_in_dictionary = 1;
  IFE(found=OgLtrasGetParameterValue(ctrl_term->hltras,"term_check_words_in_dictionary",DPcPathSize,buffer));
  if (found)
  {
    if (!Ogstricmp(buffer, "yes")) ctrl_term->check_words_in_dictionary = 1;
    else if (!Ogstricmp(buffer, "no")) ctrl_term->check_words_in_dictionary = 0;
  }
  if (module_input->argc > 2)
  {
    value = module_input->argv[2];
    if (!Ogstricmp(value, "yes")) ctrl_term->check_words_in_dictionary = 1;
    else if (!Ogstricmp(value, "no")) ctrl_term->check_words_in_dictionary = 0;
  }

  if (ctrl_term->loginfo->trace & DOgLtrasTraceModuleTerm)
  {
    if (!ctrl_term->check_words_in_dictionary)
    {
      if (ctrl_term->score_factor < 1.0)
      {
        OgMsg(ctrl_term->hmsg, "", DOgMsgDestInLog, "LtrasModuleTerm: Pay attention, check_words_in_dictionary is set to no and a score factor different from 1.0 is set");
      }

      double frequency_ratio = OgLtrasFrequencyRatio(ctrl_term->hltras);
      if (frequency_ratio != 0)
      {
        OgMsg(ctrl_term->hmsg, "", DOgMsgDestInLog, "LtrasModuleTerm: Pay attention, check_words_in_dictionary is set to no and a frequency ratio different from 0 is set");
      }

    }
  }


  /* Max number of solutions generated by the term module
   * beyond this number, solutions are not generated.
   * this avoids combinatory explosions */
  ctrl_term->max_nb_generated_solutions = 50;
  IFE(found=OgLtrasGetParameterValue(ctrl_term->hltras,"term_max_nb_generated_solutions",DPcPathSize,buffer));
  if (found)
  {
    ctrl_term->max_nb_generated_solutions = atoi(buffer);
  }
  if (module_input->argc > 3)
  {
    ctrl_term->max_nb_generated_solutions = atoi(module_input->argv[3]);
  }

  ctrl_term->min_frequency = 0;
  IFE(found=OgLtrasGetParameterValue(ctrl_term->hltras,"term_min_frequency",DPcPathSize,buffer));
  if (found)
  {
    ctrl_term->min_frequency = atoi(buffer);
  }
  if (module_input->argc > 4)
  {
    ctrl_term->min_frequency = atoi(module_input->argv[4]);
  }

  IFE(OgLtrasTrfsCreate(hltras, &trfs));
  IFE(OgLtrasTrfsAddText(hltras, trfs, input->length_text, input->Ba + input->start_text));
  *output = trfs;

  if (ctrl_term->loginfo->trace & DOgLtrasTraceModuleTerm)
  {
    OgMsg(ctrl_term->hmsg, "", DOgMsgDestInLog
        , "OgLtrasModuleTerm: input:");
    IFE(OgLtrasTrfsLog(hltras, input));
  }

  for (i = 0; i < input->TrfUsed; i++)
  {
    IFE(LtrasModuleTerm1(module_input, input, trfs, i));
  }

  if (ctrl_term->loginfo->trace & DOgLtrasTraceModuleTerm)
  {
    OgMsg(ctrl_term->hmsg, "", DOgMsgDestInLog
        , "OgLtrasModuleTerm: after LtrasModuleTerm1:");
    IFE(OgLtrasTrfsLog(hltras, trfs));
  }

  for (i = 0; i < trfs->TrfUsed; i++)
  {
    if (trfs->Trf[i].basic) trfs->TrfBasicUsed = i + 1;
    else break;
  }
  if (trfs->TrfBasicUsed != input->TrfBasicUsed)
  {
    sprintf(erreur, "OgLtrasModuleTerm: trfs->TrfBasicUsed (%d) != input->TrfBasicUsed (%d)"
        , trfs->TrfBasicUsed, input->TrfBasicUsed);
    OgErr(ctrl_term->herr, erreur);
    DPcErr;
  }

  /** Calculating sort information **/
  for (i = 0; i < trfs->TrfUsed; i++)
  {
    IFE(LtrasModuleTerm2(module_input, input, trfs, i));
  }

  if (ctrl_term->loginfo->trace & DOgLtrasTraceModuleTerm)
  {
    OgMsg(ctrl_term->hmsg, "", DOgMsgDestInLog
        , "OgLtrasModuleTerm: after LtrasModuleTerm2:");
    IFE(OgLtrasTrfsLog(hltras, trfs));
  }

  qsort(trfs->Trf + trfs->TrfBasicUsed, trfs->TrfUsed - trfs->TrfBasicUsed, sizeof(struct og_ltra_trf), trf_clean_cmp);

  if (ctrl_term->loginfo->trace & DOgLtrasTraceModuleTerm)
  {
    OgMsg(ctrl_term->hmsg, "", DOgMsgDestInLog
        , "OgLtrasModuleTerm: after first sort for clean:");
    IFE(OgLtrasTrfsLog(hltras, trfs));
  }

  for (i = 0; i < trfs->TrfUsed; i++)
    trfs->Trf[i].history_trf = 0;
  for (i = 0; i < trfs->TrfUsed; i++)
  {
    IFE(LtrasModuleTermClean(module_input, trfs, i));
  }

  if (ctrl_term->loginfo->trace & DOgLtrasTraceModuleTerm)
  {
    OgMsg(ctrl_term->hmsg, "", DOgMsgDestInLog
        , "OgLtrasModuleTerm: after cleaning:");
    IFE(OgLtrasTrfsLog(hltras, trfs));
  }

  qsort(trfs->Trf + trfs->TrfBasicUsed, trfs->TrfUsed - trfs->TrfBasicUsed, sizeof(struct og_ltra_trf), trf_clean_cmp);

  if (ctrl_term->loginfo->trace & DOgLtrasTraceModuleTerm)
  {
    OgMsg(ctrl_term->hmsg, "", DOgMsgDestInLog
        , "OgLtrasModuleTerm: after second sort for clean:");
    IFE(OgLtrasTrfsLog(hltras, trfs));
  }

  for (i = trfs->TrfBasicUsed; i < trfs->TrfUsed; i++)
  {
    if (trfs->Trf[i].final_score == 0)
    {
      trfs->TrfUsed = i;
      break;
    }
  }

  if (ctrl_term->loginfo->trace & DOgLtrasTraceModuleTerm)
  {
    OgMsg(ctrl_term->hmsg, "", DOgMsgDestInLog
        , "OgLtrasModuleTerm: after true cleaning :");
    IFE(OgLtrasTrfsLog(hltras, trfs));
  }

  if (ctrl_term->max_nb_solutions > 0)
  {
    /** Creating the solutions **/
    ctrl_term->nb_solutions = 0;
    ctrl_term->TrfUsed = trfs->TrfUsed;
    for (i = 0; i < trfs->TrfUsed; i++)
      trfs->Trf[i].history_trf = 0;
    int counter_for_safety = 0;
    IFE(LtrasModuleTerm3(module_input, trfs, 0, &counter_for_safety));
    if (ctrl_term->loginfo->trace & DOgLtrasTraceModuleTerm)
    {
      OgMsg(ctrl_term->hmsg, "", DOgMsgDestInLog
          , "OgLtrasModuleTerm: after LtrasModuleTerm3:");
      IFE(OgLtrasTrfsLog(hltras, trfs));
    }
    qsort(trfs->Trf, trfs->TrfUsed, sizeof(struct og_ltra_trf), trf_final_cmp);
    if (trfs->TrfUsed > ctrl_term->max_nb_solutions) trfs->TrfUsed = ctrl_term->max_nb_solutions;
  }

  *elapsed = OgMicroClock() - micro_clock_start;

  DONE;
}

/*
 *  Checking in the base dictionary if the transformation exists
 *  and, if so, marking it as final. Also copies all final and basic transformations.
 */
static int LtrasModuleTerm1(struct og_ltra_module_input *module_input, struct og_ltra_trfs *input,
    struct og_ltra_trfs *trfs, int Itrf)
{
  struct og_ctrl_term *ctrl_term = (struct og_ctrl_term *) module_input->handle;

  int frequencies[MaxNbWords];

  struct og_ltra_trf *trf = input->Trf + Itrf;
  struct og_ltra_trf *trfn;
  if (trf->final)
  {
    if (ctrl_term->min_frequency > 0)
    {
      int global_frequency;
      IFE(OgLtrasTrfCalculateGlobal(ctrl_term->hltras, input, Itrf, &global_frequency, 0, 0));
      if (global_frequency < ctrl_term->min_frequency)
      {
        if (ctrl_term->loginfo->trace & DOgLtrasTraceModuleTerm)
        {
          OgMsg(ctrl_term->hmsg, "", DOgMsgDestInLog,
              "LtrasModuleTerm1: global_frequency (%d) < min_frequency (%d) for transformation:", global_frequency,
              ctrl_term->min_frequency);
          IFE(OgLtrasTrfLog(ctrl_term->hltras, input, Itrf));
        }
        DONE;
      }
    }
    int Itrfn = OgLtrasTrfCopy(ctrl_term->hltras, input, Itrf, trfs, 1);
    IFE(Itrfn);
    if (trf->basic)
    {
      trfn = trfs->Trf + Itrfn;
      trfn->basic = 1;
    }
    DONE;
  }
  og_bool found = 0;

  if (ctrl_term->check_words_in_dictionary)
  {

    if (trf->nb_words > MaxNbWords)
    {
      if (ctrl_term->loginfo->trace & DOgLtrasTraceInformation)
      {
        OgMsg(ctrl_term->hmsg, "", DOgMsgDestInLog,
            "LtrasModuleTerm1: number of words in transformation too big : %d (max allowed: %d)", trf->nb_words,
            MaxNbWords);
      }
    }
    else
    {
      memset(frequencies, 0, MaxNbWords * sizeof(int));

      for (int i = 0; i < trf->nb_words; i++)
      {
        int ibuffer;
        unsigned char buffer[DPcPathSize * 2];
        found = 0;
        struct og_ltra_word *word = input->Word + trf->start_word + i;
        unsigned char *in = input->Ba + word->start;
        int iin = word->length;

        if (!trf->basic)
        {
          void *hltras = ctrl_term->hltras;
          og_bool is_false = OgLtrasIsFalseTransformation(hltras, word->length_position,
              trfs->Ba + word->start_position, iin, in);
          IFE(is_false);
          if (is_false) break;
        }

        ibuffer = OgStrCpySized(buffer, (DPcPathSize * 2) - 2, in, iin);
        if (ibuffer < iin)
        {
          if (ctrl_term->loginfo->trace & DOgLtrasTraceModuleTerm)
          {
            OgMsg(ctrl_term->hmsg, "", DOgMsgDestInLog,
                "LtrasModuleTerm1: in string has been truncated (form size %d to %d)", iin, ibuffer);
            IFE(OgLtrasTrfLog(ctrl_term->hltras, input, Itrf));
          }
        }

        // add automation suffix
        buffer[ibuffer++] = 0;
        buffer[ibuffer++] = 1;

        unsigned char *p;
        unsigned char out[DPcAutMaxBufferSize + 9];
        int iout = -1;
        oindex states[DPcAutMaxBufferSize + 9];
        int retour, nstate0, nstate1;
        if ((retour = OgAufScanf(ctrl_term->ha_base, ibuffer, buffer, &iout, out, &nstate0, &nstate1, states)))
        {
          do
          {
            IFE(retour);
            p = out;

            int attribute_number = -1;
            IFE(DOgPnin4(ctrl_term->herr,&p,&attribute_number));

            int language_code = -1;
            IFE(DOgPnin4(ctrl_term->herr,&p,&language_code));

            int frequency = -1;
            IFE(DOgPnin4(ctrl_term->herr,&p,&frequency));
            /* function OgLtrasTrfCalculateGlobal calculates the frequency of a trf
             * as the minimum of each frequency, thus we use that property to avoid
             * any trf with frequency less than the minimum frequency. This is because
             * any combination with that trf will lead to a solution with that same
             * frequency or lower */
            if (frequency < ctrl_term->min_frequency)
            {
              if (ctrl_term->loginfo->trace & DOgLtrasTraceModuleTerm)
              {
                OgMsg(ctrl_term->hmsg, "", DOgMsgDestInLog,
                    "LtrasModuleTerm1: frequency (%d) < min_frequency (%d) for transformation:", frequency,
                    ctrl_term->min_frequency);
                IFE(OgLtrasTrfLog(ctrl_term->hltras, input, Itrf));
              }
              continue;
            }
            frequencies[i] += frequency;
            found = 1;
          }
          while ((retour = OgAufScann(ctrl_term->ha_base, &iout, out, nstate0, &nstate1, states)));
        }
        if (!found)
        {
          break;
        }
      }

      if (found)
      {
        // all words in trf found
        int Itrfn = OgLtrasTrfCopy(ctrl_term->hltras, input, Itrf, trfs, 1);
        IFE(Itrfn);
        trfn = trfs->Trf + Itrfn;
        trfn->final = 1;
        for (int i = 0; i < trf->nb_words; i++)
        {
          struct og_ltra_word * wordn = trfs->Word + trfn->start_word + i;
          wordn->frequency = frequencies[i];
        }
        if (trf->basic) trfn->basic = 1;
      }
    }
  }

  if (!found && trf->basic)
  {
    int Itrfn = OgLtrasTrfCopy(ctrl_term->hltras, input, Itrf, trfs, 1);
    IFE(Itrfn);
    trfn = trfs->Trf + Itrfn;
    trfn->basic = 1;
    struct og_ltra_word *word = trfs->Word + trfn->start_word;
    /** We need to calculate the basic word frequency so that they are sorted correctly **/
    if (ctrl_term->check_words_in_dictionary)
    {
      IFE(OgLtrasTrfCalculateFrequency(ctrl_term->hltras, word->length, trfs->Ba + word->start, word->language, &word->frequency));
    }
    else
    {
      word->frequency = ctrl_term->no_dictionary_frequency;
    }
  }
  else if (!ctrl_term->check_words_in_dictionary)
  {
    int Itrfn = OgLtrasTrfCopy(ctrl_term->hltras, input, Itrf, trfs, 1);
    IFE(Itrfn);
    trfn = trfs->Trf + Itrfn;
    trfn->final = ctrl_term->no_dictionary_frequency;
    struct og_ltra_word *word = trfs->Word + trfn->start_word;
    word->frequency = 1;
    og_status status = OgLtrasTrfCalculateGlobal(ctrl_term->hltras, trfs, Itrfn, &trfn->global_frequency,
        &trfn->global_score, &trfn->final_score);
    IFE(status);
  }
  DONE;
}





static int LtrasModuleTerm2(struct og_ltra_module_input *module_input
  , struct og_ltra_trfs *input,  struct og_ltra_trfs *trfs, int Itrf)
{
struct og_ctrl_term *ctrl_term = (struct og_ctrl_term *)module_input->handle;
struct og_ltra_trf *trf = trfs->Trf + Itrf;
int i,first,end,bend,must_combine=0;
struct og_ltra_trf *btrf;


if (trf->final) must_combine=1;
if (!ctrl_term->check_words_in_dictionary && trf->basic) must_combine=1;
if (!must_combine) DONE;

for (first=1,i=0; i<trfs->TrfBasicUsed; i++) {
  btrf = trfs->Trf + i;
  end = trf->start + trf->length;
  bend = btrf->start + btrf->length;
  if (trf->start <= btrf->start && bend <= end) {
    if (first) { trf->span_start_trf=i; first=0; }
    trf->span_nb_trfs++;
    }
  }

IFE(OgLtrasTrfCalculateGlobal(ctrl_term->hltras,trfs,Itrf
  ,&trf->global_frequency, &trf->global_score, &trf->final_score));

DONE;
}




static int trf_clean_cmp(const void *ptr1, const void *ptr2)
{
struct og_ltra_trf *trf1 = (struct og_ltra_trf *)ptr1;
struct og_ltra_trf *trf2 = (struct og_ltra_trf *)ptr2;
double dcmp;
int cmp;

if (trf1->final_score==0 &&  trf2->final_score==0) return(trf_cmp(ptr1,ptr2));
else if (trf1->final_score!=0 &&  trf2->final_score==0) return(-1);
else if (trf1->final_score==0 &&  trf2->final_score!=0) return(1);

cmp = trf1->start - trf2->start;
if (cmp) return(cmp);
cmp = trf1->length - trf2->length;
if (cmp) return(cmp);

dcmp = trf2->final_score - trf1->final_score;
if (dcmp > 0) return(1);
else if (dcmp < 0) return(-1);
return(trf_cmp(ptr1,ptr2));
}





static int trf_cmp(const void *ptr1, const void *ptr2)
{
struct og_ltra_trf *trf1 = (struct og_ltra_trf *)ptr1;
struct og_ltra_trf *trf2 = (struct og_ltra_trf *)ptr2;
int isword1; unsigned char sword1[DPcPathSize];
int isword2; unsigned char sword2[DPcPathSize];
struct og_ltra_word *word;
int i,iswordmin,cmp;
double dcmp;

dcmp = trf2->final_score - trf1->final_score;
if (dcmp > 0) return(1);
else if (dcmp < 0) return(-1);

dcmp = trf2->global_score - trf1->global_score;
if (dcmp > 0) return(1);
else if (dcmp < 0) return(-1);

cmp = trf2->global_frequency - trf1->global_frequency;
if (cmp) return(cmp);

/* This is to make sure we are always in the same order
 * Is it not too costly, because it is only done when
 * scoring and frequency are the same */
for (isword1=0,i=0; i<trf1->nb_words; i++) {
  word = trf1->trfs->Word + trf1->start_word + i;
  memcpy(sword1+isword1,trf1->trfs->Ba+word->start,word->length);
  isword1 += word->length;
  }
for (isword2=0,i=0; i<trf2->nb_words; i++) {
  word = trf2->trfs->Word + trf2->start_word + i;
  memcpy(sword2+isword2,trf2->trfs->Ba+word->start,word->length);
  isword2 += word->length;
  }
if (isword1<isword2) iswordmin=isword1;
else iswordmin=isword2;

cmp=Ogmemicmp(sword1,sword2,iswordmin);
if (cmp) return(cmp);

cmp=isword1-isword2;

return(cmp);
}




static int trf_final_cmp(const void *ptr1, const void *ptr2)
{
struct og_ltra_trf *trf1 = (struct og_ltra_trf *)ptr1;
struct og_ltra_trf *trf2 = (struct og_ltra_trf *)ptr2;
int final_total1=0,final_total2=0;

if (trf1->final && trf1->total) final_total1=1;
if (trf2->final && trf2->total) final_total2=1;

if (final_total1 && final_total2) return(trf_cmp(ptr1,ptr2));
else if (final_total1) return(-1);
else if (final_total2) return(1);
else return(trf_cmp(ptr1,ptr2));
}



/*
 * The goal is to get all trfs with same start and length
 * and limit the number of such list using the maximum_transformation
 * parameter. This avoids combinatory explosion, for example with
 * lemmatisation (but not only). We use history_trf as a marker.
*/


static int LtrasModuleTermClean(struct og_ltra_module_input *module_input
  , struct og_ltra_trfs *trfs, int Itrf)
{
struct og_ltra_trf *trf_seed = trfs->Trf + Itrf;
int i,nb_solutions;

if (trf_seed->history_trf) DONE;
if (trf_seed->final_score == 0) {
  trf_seed->history_trf=1;
  DONE;
  }

nb_solutions=1;
trf_seed->history_trf=1;
for (i=0; i<trfs->TrfUsed; i++) {
  IFE(LtrasModuleTermClean1(module_input,trfs,Itrf,i,&nb_solutions));
  }

DONE;
}




static int LtrasModuleTermClean1(struct og_ltra_module_input *module_input
  , struct og_ltra_trfs *trfs, int Itrf_seed, int Itrf, int *pnb_solutions)
{
struct og_ctrl_term *ctrl_term = (struct og_ctrl_term *)module_input->handle;
struct og_ltra_trf *trf_seed = trfs->Trf + Itrf_seed;
struct og_ltra_trf *trf = trfs->Trf + Itrf;
int nb_solutions = *pnb_solutions;
int max_nb_solutions,nb_request_words;

if (trf->history_trf) DONE;
if (trf->start != trf_seed->start) DONE;
if (trf->length != trf_seed->length) DONE;
if (trf->final_score == 0) DONE;
if (trf->length <= 0) DONE;

nb_request_words = trfs->TrfBasicUsed + trf->nb_words - 1;

if (nb_request_words > ctrl_term->maximum_transformation_length-1) max_nb_solutions
  = ctrl_term->maximum_transformation[ctrl_term->maximum_transformation_length-1];
else max_nb_solutions = ctrl_term->maximum_transformation[nb_request_words];

if (nb_solutions >= max_nb_solutions) trf->final_score = 0;
else nb_solutions++;

*pnb_solutions = nb_solutions;
trf->history_trf=1;

DONE;
}



static int LtrasModuleTerm3(struct og_ltra_module_input *module_input
  , struct og_ltra_trfs *trfs, int span_start_trf, int *counter_for_safety)
{
struct og_ctrl_term *ctrl_term = (struct og_ctrl_term *)module_input->handle;
struct og_ltra_add_trf_input ctinput,*tinput=&ctinput;
int words_length; unsigned char words[DPcPathSize];
struct og_ltra_add_trf_word *new_word;
unsigned char string[DPcPathSize*5];
unsigned char buffer[DPcPathSize];
void *hltras = ctrl_term->hltras;
int stopped,added,all_basic_trf;
struct og_ltra_trf *trf,*ntrf;
int nb_scores; double score;
struct og_ltra_word *word;
int i,j,Iotrf,Itrf,Intrf;
int is_false;

// Sometimes there is a bug and it loop infinitely (bug has not been understood)
// So we have a counter to avoid infinity loops
*counter_for_safety = *counter_for_safety + 1;
if(*counter_for_safety >= 2000) return(1);

IFn(ctrl_term->TrfUsed) return(0);

if (span_start_trf == trfs->TrfBasicUsed) {
  /* We have a solution, we use span_nb_trfs to store the current list
   * of ordered trfs that are the building trfs of the solution */
  if (ctrl_term->loginfo->trace & DOgLtrasTraceModuleTerm) {
    buffer[0]=0;
    for (i=0; i<trfs->TrfBasicUsed; i++) {
      sprintf(buffer+strlen(buffer),"%s%d",i?" ":"",trfs->Trf[i].span_nb_trfs);
      }
    OgMsg(ctrl_term->hmsg,"",DOgMsgDestInLog
      , "LtrasModuleTerm3: found solution [%s]", buffer);
    }
  if (LtrasModuleTermStopExplosion(ctrl_term)) return(1);
  score=0.0; nb_scores=0; words_length=0;
  memset(tinput,0,sizeof(struct og_ltra_add_trf_input));
  all_basic_trf=1;
  for (Iotrf=(-1),i=0; i<trfs->TrfBasicUsed; i++) {
    Itrf=trfs->Trf[i].history_trf;
    if (Itrf==Iotrf) continue;
    trf = trfs->Trf + Itrf;
    if (!trf->basic) all_basic_trf=0;
    for (j=0; j<trf->nb_words; j++) {
      word = trfs->Word + trf->start_word + j;
      new_word = tinput->word+tinput->nb_words;
      if (words_length+word->length >= DPcPathSize/2) {
        if (ctrl_term->loginfo->trace & DOgLtrasTraceInformation) {
          OgMsg(ctrl_term->hmsg,"",DOgMsgDestInLog
            , "LtrasModuleTerm3: solution too long (>=%d), discarding solution", DPcPathSize/2);
          }
        return(0);
        }
      memcpy(words+words_length,trfs->Ba+word->start,word->length);
      new_word->string = words+words_length;
      new_word->string_length = word->length;
      new_word->frequency = word->frequency;
      new_word->base_frequency = word->base_frequency;
      new_word->start_position = word->start_position;
      new_word->length_position = word->length_position;
      new_word->language = word->language;
      words_length+=word->length;
      if (tinput->nb_words >= DOgLtrasAddTrfMaxNbWords) {
        if (ctrl_term->loginfo->trace & DOgLtrasTraceInformation) {
          OgMsg(ctrl_term->hmsg,"",DOgMsgDestInLog
            , "LtrasModuleTerm3: tinput->nb_words (%d) >= DOgLtrasAddTrfMaxNbWords (%d), discarding solution"
            , tinput->nb_words, DOgLtrasAddTrfMaxNbWords);
          }
        return(0);
        }
      tinput->nb_words++;
      }
    score+= (1-trf->global_score); nb_scores++;
    Iotrf=Itrf;
    }
  if (all_basic_trf) {
    if (ctrl_term->loginfo->trace & DOgLtrasTraceModuleTerm) {
      IFE(OgLtrasInputTrfString(hltras, tinput, DPcPathSize*5, string));
      OgMsg(ctrl_term->hmsg,"",DOgMsgDestInLog
        , "LtrasModuleTerm3: discarding solution because it is basic: '%s'", string);
      }
    return(0);
    }
  trf = trfs->Trf + trfs->Trf[0].history_trf;
  tinput->start = trf->start;
  trf = trfs->Trf + Itrf;
  tinput->length = trf->start + trf->length - tinput->start;
  tinput->module_id = module_input->id;
  tinput->score = (1-score);
  tinput->from_trf = trfs->Trf[0].history_trf;
  tinput->final = 1; tinput->total = 1;
  /** Checks for false expression **/
  IFE(is_false=LtrasModuleTermIsFalse(module_input,trfs,tinput));
  if (is_false) {
    if (ctrl_term->loginfo->trace & DOgLtrasTraceModuleTerm) {
      IFE(OgLtrasInputTrfString(hltras, tinput, DPcPathSize*5, string));
      OgMsg(ctrl_term->hmsg,"",DOgMsgDestInLog
        , "LtrasModuleTerm3: discarding solution because it is false: '%s'", string);
      }
    return(0);
    }
  IFE(added=OgLtrasTrfAdd(hltras,trfs,tinput,&Intrf));
  if (added) {
    ntrf = trfs->Trf + Intrf;
    ntrf->span_start_trf=trfs->Trf[0].history_trf;
    ntrf->span_nb_trfs=trfs->TrfBasicUsed;
    IFE(OgLtrasTrfCalculateGlobal(ctrl_term->hltras,trfs,Intrf
      ,&ntrf->global_frequency, &ntrf->global_score, &ntrf->final_score));
    if (ctrl_term->loginfo->trace & DOgLtrasTraceModuleTerm) {
      IFE(OgLtrasInputTrfString(hltras, tinput, DPcPathSize*5, string));
      OgMsg(ctrl_term->hmsg,"",DOgMsgDestInLog
        , "LtrasModuleTerm3: solution added: '%s'", string);
      }
    }
  return(0);
  }

for (i=0; i<ctrl_term->TrfUsed; i++) {
  trf = trfs->Trf + i;
  if (trf->span_start_trf != span_start_trf) continue;
  for (j=0; j<trf->span_nb_trfs; j++) {
    trfs->Trf[span_start_trf+j].history_trf = i;
    }
  IFE(stopped=LtrasModuleTerm3(module_input,trfs,trf->span_start_trf+trf->span_nb_trfs, counter_for_safety));
  if (stopped) return(1);
  }
return(0);
}



static int LtrasModuleTermIsFalse(struct og_ltra_module_input *module_input
  , struct og_ltra_trfs *trfs, struct og_ltra_add_trf_input *input)
{
struct og_ctrl_term *ctrl_term = (struct og_ctrl_term *)module_input->handle;
struct og_ltra_add_trf_word *new_word;
int from_length; unsigned char *from;
int to_length; unsigned char *to;
void *hltras = ctrl_term->hltras;
int i,n,j,is_false;

for (n=1; n<=input->nb_words; n++) {
  for (i=0; i<=input->nb_words-n; i++){
    IFE(OgHeapResetWithoutReduce(ctrl_term->hfrom));
    IFE(OgHeapResetWithoutReduce(ctrl_term->hto));
    for (j=0; j<n; j++) {
      new_word = input->word + i+j;
      if (j > 0) {
        IFE(OgHeapAppend(ctrl_term->hfrom,2,"\0 "));
        IFE(OgHeapAppend(ctrl_term->hto,2,"\0 "));
        }
      IFE(OgHeapAppend(ctrl_term->hfrom,new_word->length_position,trfs->Ba+new_word->start_position));
      IFE(OgHeapAppend(ctrl_term->hto,new_word->string_length,new_word->string));
      }
    from_length=OgHeapGetCellsUsed(ctrl_term->hfrom);
    IFn(from=OgHeapGetCell(ctrl_term->hfrom,0)) DPcErr;

    IFn(to=OgHeapGetCell(ctrl_term->hto,0)) DPcErr;
    to_length=OgHeapGetCellsUsed(ctrl_term->hto);

    IFE(is_false=OgLtrasIsFalseTransformation(hltras, from_length, from, to_length, to));
    if (is_false) return(1);
    }
  }

return(0);
}




static int LtrasModuleTermStopExplosion(struct og_ctrl_term *ctrl_term)
{
ctrl_term->nb_solutions++;
/** stopping to avoid combinatory explosion **/
if (ctrl_term->max_nb_generated_solutions > 0 && ctrl_term->nb_solutions >= ctrl_term->max_nb_generated_solutions) {
  if (ctrl_term->loginfo->trace & DOgLtrasTraceInformation) {
    OgMsg(ctrl_term->hmsg,"",DOgMsgDestInLog
      , "LtrasModuleTerm3: nb_solutions (%d) >= max_nb_generated_solutions (%d), stopping recombination of solutions"
      , ctrl_term->nb_solutions, ctrl_term->max_nb_generated_solutions);
    }
  return(1);
  }
return(0);
}

