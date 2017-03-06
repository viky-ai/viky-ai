/*
 *  All module function for Linguistic trf search
 *  Copyright (c) 2009-2010 Pertimm by Patrick Constant
 *  Dev: November 2009, January,August 2010
 *  Version 1.0
 */
#include "ogm_ltras.h"

struct ltraf_score_ctx
{
    int ibuffer;
    og_string buffer;
    og_bool same_lang;
    int language;

    int Fword;
    int Fexpr;
    double final_score;
    double global_score;
};


static int LtrasGetStringLengthFromInput(struct og_ctrl_ltras *ctrl_ltras, struct og_ltra_add_trf_input *input);
static int LtrasTrfSameAsRequest(struct og_ctrl_ltras *ctrl_ltras, struct og_ltra_trfs *trfs,
    struct og_ltra_add_trf_input *input);
static int LtrasTrfCalculateScoresFromInput(struct og_ctrl_ltras *ctrl_ltras, struct og_ltra_trfs *trfs,
    struct og_ltra_add_trf_input *input, og_bool check_words_in_dictionary, int *pword_frequency,
    int *pexpression_frequency, double *pglobal_score, double *pfinal_score);
static int LtrasTrfGet(struct og_ctrl_ltras *ctrl_ltras, struct og_ltra_trfs *trfs, struct og_ltra_add_trf_input *input,
    int *pItrf);
static int LtrasTrfEqual(struct og_ctrl_ltras *ctrl_ltras, struct og_ltra_trfs *trfs, int Itrf,
    struct og_ltra_add_trf_input *input);
static int LtrasTrfSame(void *handle, struct og_ltra_trfs *trfs1, int Itrf1, struct og_ltra_trfs *trfs2, int Itrf2);
static int LtrasTrfMerge(struct og_ctrl_ltras *ctrl_ltras, struct og_ltra_trfs *trfs, struct og_ltra_trfs *mother_trfs,
    int Itrf);
static int LtrasTrfMergeScores(struct og_ctrl_ltras *ctrl_ltras, struct og_ltra_trfs *trfs1, int Itrf1,
    struct og_ltra_trfs *trfs2, int Itrf2);
static int LtrasTrfCalculateScores(struct og_ctrl_ltras *ctrl_ltras, struct ltraf_score_ctx *ctx, og_bool check_words_in_dictionary);

PUBLIC(og_status) OgLtrasTrfsCreate(void *handle, struct og_ltra_trfs **ptrfs)
{
  struct og_ctrl_ltras *ctrl_ltras = (struct og_ctrl_ltras *) handle;
  struct og_ltra_trfs *trfs;
  char erreur[DOgErrorSize];
  int size;

  IFn(trfs=(struct og_ltra_trfs *)malloc(sizeof(struct og_ltra_trfs)))
  {
    sprintf(erreur, "OgLtrasTrfsCreate: malloc error on trfs");
    OgErr(ctrl_ltras->herr, erreur);
    DPcErr;
  }
  memset(trfs, 0, sizeof(struct og_ltra_trfs));
  *ptrfs = trfs;

  trfs->TrfNumber = DOgLtrasTrfsTrfNumber;
  size = trfs->TrfNumber * sizeof(struct og_ltra_trf);
  IFn(trfs->Trf=(struct og_ltra_trf *)malloc(size))
  {
    sprintf(erreur, "OgLtrasTrfsCreate: malloc error on Trf (%d bytes)", size);
    OgErr(ctrl_ltras->herr, erreur);
    return (0);
  }

  trfs->WordNumber = DOgLtrasTrfsWordNumber;
  size = trfs->WordNumber * sizeof(struct og_ltra_word);
  IFn(trfs->Word=(struct og_ltra_word *)malloc(size))
  {
    sprintf(erreur, "OgLtrasTrfsCreate: malloc error on Word (%d bytes)", size);
    OgErr(ctrl_ltras->herr, erreur);
    return (0);
  }

  trfs->ModuleNumber = DOgLtrasTrfsModuleNumber;
  size = trfs->ModuleNumber * sizeof(struct og_ltra_module);
  IFn(trfs->Module=(struct og_ltra_module *)malloc(size))
  {
    sprintf(erreur, "OgLtrasTrfsCreate: malloc error on Module (%d bytes)", size);
    OgErr(ctrl_ltras->herr, erreur);
    return (0);
  }

  trfs->BaSize = DOgLtrasTrfsBaSize;
  size = trfs->BaSize * sizeof(unsigned char);
  IFn(trfs->Ba=(unsigned char *)malloc(size))
  {
    sprintf(erreur, "OgLtrasTrfsCreate: malloc error on Ba (%d bytes)", size);
    OgErr(ctrl_ltras->herr, erreur);
    return (0);
  }

  IFn(trfs->htransposition=OgHeapInit(ctrl_ltras->hmsg,"ltras_htransposition",sizeof(int),DOgLtrasTrfsTransposition)) {
    size = sizeof(int) * DOgLtrasTrfsTransposition;
    sprintf(erreur, "OgLtrasTrfsCreate: OgHeapInit error on htransposition (%d bytes)", size);
    OgErr(ctrl_ltras->herr, erreur);
    return (0);
  }

  DONE;
}

PUBLIC(og_status) OgLtrasTrfsDestroy(void *handle, struct og_ltra_trfs *trfs)
{
  IFE(OgHeapFlush(trfs->htransposition));
  DPcFree(trfs->Trf);
  DPcFree(trfs->Word);
  DPcFree(trfs->Module);
  DPcFree(trfs->Ba);
  DPcFree(trfs);
  DONE;
}

PUBLIC(og_status) OgLtrasTrfsAddText(void *handle, struct og_ltra_trfs *trfs, int length_text, unsigned char *text)
{
  struct og_ctrl_ltras *ctrl_ltras = (struct og_ctrl_ltras *) handle;
  int is;
  unsigned char *s;

  /* Trimming is not a good idea as we want to keep
   * a reference to the original string */
  trfs->start_text = trfs->BaUsed;
  IFE(LtrasTrfsAppendBa(ctrl_ltras, trfs, length_text, text));
  trfs->length_text = length_text;

  /** However, lowercase the string is necessary **/
  is = trfs->length_text;
  s = trfs->Ba + trfs->start_text;
  OgUniStrlwr(is, s, s);

  DONE;
}

/*
 *  Returns 1 if a new trf is added, zero otherwise.
 */

PUBLIC(int) OgLtrasTrfAdd(void *handle, struct og_ltra_trfs *trfs, struct og_ltra_add_trf_input *input, int *pIntrf)
{
  struct og_ctrl_ltras *ctrl_ltras = (struct og_ctrl_ltras *) handle;
  int Itrf;
  struct og_ltra_trf *trf, *from_trf;
  double original_final_score, new_final_score;
  struct og_ltra_add_trf_word *new_word;
  unsigned char string[DPcPathSize * 5];
  unsigned char v1[256], v2[256];
  int i, found, same_as_request;
  struct og_ltra_module *module;
  int string_length;
  double frequency;

  if (pIntrf) *pIntrf = (-1);

  IFE(string_length = LtrasGetStringLengthFromInput(ctrl_ltras, input));

  if (input->score < ctrl_ltras->minimum_score[string_length])
  {
    if (ctrl_ltras->loginfo->trace & DOgLtrasTraceSelection)
    {
      sprintf(v1, "%.4f", input->score);
      sprintf(v2, "%.4f", ctrl_ltras->minimum_score[string_length]);
      IFE(OgLtrasInputTrfString(ctrl_ltras,input, DPcPathSize*5, string));
      OgMsg(ctrl_ltras->hmsg, "", DOgMsgDestInLog,
          "OgLtrasTrfAdd: input->score (%s) < minimum_score[string_length=%d] (%s), discarding transformation: %s", v1,
          string_length, v2, string);
    }
    return (0);
  }

  IFE(same_as_request = LtrasTrfSameAsRequest(ctrl_ltras, trfs, input));
  if (same_as_request)
  {
    if (ctrl_ltras->loginfo->trace & DOgLtrasTraceSelection)
    {
      IFE(OgLtrasInputTrfString(ctrl_ltras,input, DPcPathSize*5, string));
      OgMsg(ctrl_ltras->hmsg, "", DOgMsgDestInLog, "OgLtrasTrfAdd: following transformation already exists: %s",
          string);
    }
    return (0);
  }

  if (ctrl_ltras->input->frequency_ratio > 0)
  {
    if (input->final)
    {
      for (i = 0; i < input->nb_words; i++)
      {
        new_word = input->word + i;
        frequency = new_word->frequency / ctrl_ltras->input->frequency_ratio;
        if (frequency < new_word->base_frequency)
        {
          if (ctrl_ltras->loginfo->trace & DOgLtrasTraceSelection)
          {
            sprintf(v1, "%.2f", frequency);
            sprintf(v2, "%.2f", ctrl_ltras->input->frequency_ratio);
            IFE(OgLtrasInputTrfString(ctrl_ltras,input, DPcPathSize*5, string));
            OgMsg(ctrl_ltras->hmsg, "", DOgMsgDestInLog,
                "OgLtrasTrfAdd: frequency (%s=%d/%s) < base_frequency (%d) on word %d, discarding transformation: %s",
                v1, new_word->frequency, v2, new_word->base_frequency, i, string);
          }
          return (0);
        }
      }
    }
  }

  IFE(found = LtrasTrfGet(ctrl_ltras, trfs, input, &Itrf));

  if (input->final && input->total)
  {
    if (found && input->from_trf == Itrf) input->from_trf = (-1);
    /* Normally, we add the scores that come from input->from_trf, but when
     * it is Itrf, we reset it with LtrasResetTrf, thus, we need to anticipate
     * this reset */
    IFE(LtrasTrfCalculateScoresFromInput(ctrl_ltras, trfs, input, TRUE, 0, 0, 0, &new_final_score));
    if (new_final_score < ctrl_ltras->minimum_final_score[string_length])
    {
      if (ctrl_ltras->loginfo->trace & DOgLtrasTraceSelection)
      {
        sprintf(v1, "%.4f", new_final_score);
        sprintf(v2, "%.4f", ctrl_ltras->minimum_final_score[string_length]);
        IFE(OgLtrasInputTrfString(ctrl_ltras,input, DPcPathSize*5, string));
        OgMsg(ctrl_ltras->hmsg, "", DOgMsgDestInLog,
            "OgLtrasTrfAdd: new_final_score (%s) < minimum_final_score[string_length=%d] (%s), discarding transformation: %s",
            v1, string_length, v2, string);
      }
      return (0);
    }
  }

  if (found)
  {
    trf = trfs->Trf + Itrf;
    if (pIntrf) *pIntrf = Itrf;
    if (input->total) trf->total = 1;
    if (trf->basic) return (0);
    IFE(OgLtrasTrfCalculateScoresFromTrf(ctrl_ltras, trfs, Itrf, TRUE, 0, 0, 0, &original_final_score));
    /* Normally, we add the scores that come from input->from_trf, but when
     * it is Itrf, we reset it with LtrasResetTrf, thus, we need to anticipate
     * this reset */
    if (input->from_trf == Itrf) input->from_trf = (-1);
    IFE(LtrasTrfCalculateScoresFromInput(ctrl_ltras, trfs, input, TRUE, 0, 0, 0, &new_final_score));
    if (new_final_score <= original_final_score)
    {
      if (ctrl_ltras->loginfo->trace & DOgLtrasTraceSelection)
      {
        sprintf(v1, "%.4f", new_final_score);
        sprintf(v2, "%.4f", original_final_score);
        IFE(OgLtrasInputTrfString(ctrl_ltras,input, DPcPathSize*5, string));
        OgMsg(ctrl_ltras->hmsg, "", DOgMsgDestInLog,
            "OgLtrasTrfAdd: new_final_score (%s) <= original_final_score (%s) for Itrf=%d, discarding transformation: %s",
            v1, v2, Itrf, string);
        if (input->total)
        {
          OgMsg(ctrl_ltras->hmsg, "", DOgMsgDestInLog, "OgLtrasTrfAdd: but setting original transformation to 'final'");
        }
      }
      return (0);
    }
    /** from here, trf's content will be rewritten **/
    IFE(LtrasResetTrf(ctrl_ltras, trfs, Itrf));
  }
  else
  {
    IFE(Itrf = LtrasAllocTrf(ctrl_ltras, trfs, &trf));
    if (pIntrf) *pIntrf = Itrf;

  }

  if (ctrl_ltras->loginfo->trace & DOgLtrasTraceSelection)
  {
    IFE(OgLtrasInputTrfString(ctrl_ltras,input, DPcPathSize*5, string));
    OgMsg(ctrl_ltras->hmsg, "", DOgMsgDestInLog, "OgLtrasTrfAdd: adding transformation with Itrf=%d: %s", Itrf, string);
  }

  if (input->from_trf >= 0)
  {
    from_trf = trfs->Trf + input->from_trf;
    for (i = 0; i < from_trf->nb_modules; i++)
    {
      module = trfs->Module + from_trf->start_module + i;
      IFE(LtrasAddModule(ctrl_ltras, trfs, trf, module->module_id));
    }
  }

  if (!input->basic)
  {
    IFE(LtrasAddModule(ctrl_ltras, trfs, trf, input->module_id));
  }

  trf->basic = input->basic;
  trf->language = input->language;
  trf->length = input->length;
  trf->start = input->start;
  trf->global_score = input->score;
  trf->start_transposition = input->start_transposition;
  trf->length_transposition = input->length_transposition;

  for (i = 0; i < input->nb_words; i++)
  {
    unsigned char *word_string;
    new_word = input->word + i;
    /* trfs->Ba can be reallocated in LtrasAddWord so we use a temporary buffer */
    IFE(OgHeapResetWithoutReduce(ctrl_ltras->hba2));
    IFE(OgHeapAppend(ctrl_ltras->hba2, new_word->string_length, new_word->string));
    IFn(word_string=OgHeapGetCell(ctrl_ltras->hba2,0)) DPcErr;
    IFE(
        LtrasAddWord(ctrl_ltras, trfs, trf, new_word->string_length, word_string, new_word->frequency,
            new_word->base_frequency, new_word->start_position, new_word->length_position, new_word->language));
  }

  if (input->final) trf->final = 1;
  if (input->total) trf->total = 1;

  return (1);
}

/*
 *  Copies transformation Itrf1 from trfs1 into trfs2
 *  is it safe if trfs1 is the same as trfs2
 */
PUBLIC(int) OgLtrasTrfCopy(void *handle, struct og_ltra_trfs *trfs1, int Itrf1, struct og_ltra_trfs *trfs2,
    int copy_scores)
{
  struct og_ctrl_ltras *ctrl_ltras = (struct og_ctrl_ltras *) handle;
  struct og_ltra_trf *trf1 = trfs1->Trf + Itrf1;
  int Itrf2;
  struct og_ltra_trf *trf2;
  struct og_ltra_module *module1;
  struct og_ltra_word *word1;
  unsigned char *word_string;
  int i;

  IFE(Itrf2 = LtrasAllocTrf(ctrl_ltras, trfs2, &trf2));
  trf2->basic = 0;
  trf2->start = trf1->start;
  trf2->length = trf1->length;
  trf2->final = trf1->final;
  trf2->total = trf1->total;
  trf2->language = trf1->language;

  trf2->nb_words = trf1->nb_words;
  for (i = 0; i < trf1->nb_words; i++)
  {
    word1 = trfs1->Word + trf1->start_word + i;
    if (trfs1 == trfs2)
    {
      /* trfs1->Ba can be reallocated in LtrasAddWord when trfs1 == trfs2
       * so we use a temporary buffer */
      IFE(OgHeapResetWithoutReduce(ctrl_ltras->hba1));
      IFE(OgHeapAppend(ctrl_ltras->hba1, word1->length, trfs1->Ba + word1->start));
      IFn(word_string=OgHeapGetCell(ctrl_ltras->hba1,0)) DPcErr;
    }
    else
    {
      word_string = trfs1->Ba + word1->start;
    }
    IFE(
        LtrasAddWord(ctrl_ltras, trfs2, trf2, word1->length, word_string, word1->frequency, word1->base_frequency,
            word1->start_position, word1->length_position, word1->language));
  }

  if (copy_scores)
  {
    trf2->global_score = trf1->global_score;
    for (i = 0; i < trf1->nb_modules; i++)
    {
      module1 = trfs1->Module + trf1->start_module + i;
      IFE(LtrasAddModule(ctrl_ltras, trfs2, trf2, module1->module_id));
    }
  }
  return (Itrf2);
}


PUBLIC(int) OgLtrasTrfCalculateFrequency(void *handle, int string_length, unsigned char *string, int language, int *pfrequency)
{
  struct og_ctrl_ltras *ctrl_ltras = (struct og_ctrl_ltras *) handle;
  int iout;
  unsigned char *p, out[DPcAutMaxBufferSize + 9];
  int ibuffer;
  unsigned char buffer[DPcPathSize * 2];
  int language_code, frequency;
  oindex states[DPcAutMaxBufferSize + 9];
  int retour, nstate0, nstate1;
  int found = 0;

  *pfrequency = 0;

  /** checks and, if necessary, initializes the base automaton **/
  IFn(OgLtrasHaBase(handle)) return (0);

  ibuffer = OgStrCpySized(buffer, (DPcPathSize * 2) - 2, string, string_length);
  if (ibuffer < string_length)
  {
    if (ctrl_ltras->loginfo->trace & DOgLtrasTraceModuleTerm)
    {
      OgMsg(ctrl_ltras->hmsg, "", DOgMsgDestInLog, "OgLtrasTrfCalculateFrequency: in string has been truncated"
          " (form size %d to %d)", string_length, ibuffer);
    }
  }

// add automation suffix
  buffer[ibuffer++] = 0;
  buffer[ibuffer++] = 1;

  if ((retour = OgAufScanf(ctrl_ltras->ha_base, ibuffer, buffer, &iout, out, &nstate0, &nstate1, states)))
  {
    do
    {
      IFE(retour);
      p = out;
      IFE(DOgPnin4(ctrl_ltras->herr,&p,&language_code));
      IFE(DOgPnin4(ctrl_ltras->herr,&p,&frequency));

      // check language frequency
      int language_lang = OgIso639_3166ToLang(language);
      if (language_lang != 0 && language_code != 0 && language_lang != language_code)
      {
        continue;
      }

      *pfrequency += frequency;
      found = 1;
    }
    while ((retour = OgAufScann(ctrl_ltras->ha_base, &iout, out, nstate0, &nstate1, states)));
  }

  return (found);
}


PUBLIC(int) OgLtrasTrfCalculateExpressionFrequency(void *handle, int string_length, unsigned char *string, int language, int *pfrequency)
{
  struct og_ctrl_ltras *ctrl_ltras = (struct og_ctrl_ltras *) handle;

  *pfrequency = 0;


  unsigned char buffer[DPcPathSize * 2];
  int ibuffer = OgStrCpySized(buffer, (DPcPathSize * 2) - 2, string, string_length);
  if (ibuffer < string_length)
  {
    if (ctrl_ltras->loginfo->trace & DOgLtrasTraceModuleTerm)
    {
      OgMsg(ctrl_ltras->hmsg, "", DOgMsgDestInLog, "OgLtrasTrfCalculateExpressionFrequency: in string has been truncated"
          " (form size %d to %d)", string_length, ibuffer);
    }
  }

  // add automation suffix
  buffer[ibuffer++] = 0;
  buffer[ibuffer++] = 1;

  int iout;
  unsigned char out[DPcAutMaxBufferSize + 9];
  oindex states[DPcAutMaxBufferSize + 9];
  int retour, nstate0, nstate1;
  og_bool found  = FALSE;
  if ((retour = OgAufScanf(ctrl_ltras->ha_base, ibuffer, buffer, &iout, out, &nstate0, &nstate1, states)))
  {
    do
    {
      IFE(retour);
      unsigned char *p = out;

      int language_code = DOgLangNil;
      IFE(DOgPnin4(ctrl_ltras->herr,&p,&language_code));

      int frequency = 0;
      IFE(DOgPnin4(ctrl_ltras->herr,&p,&frequency));

      // check language frequency
      int language_lang = OgIso639_3166ToLang(language);
      if (language_lang != 0 && language_code != 0 && language_lang != language_code)
      {
        continue;
      }

      *pfrequency += frequency;
      found = TRUE;
    }
    while ((retour = OgAufScann(ctrl_ltras->ha_base, &iout, out, nstate0, &nstate1, states)));
  }

  return (found);
}


static int LtrasGetStringLengthFromInput(struct og_ctrl_ltras *ctrl_ltras, struct og_ltra_add_trf_input *input)
{
  struct og_ltra_add_trf_word *new_word;
  int i, string_length = 0;

  for (i = 0; i < input->nb_words; i++)
  {
    new_word = input->word + i;
    string_length += new_word->string_length;
    if (i > 0) string_length += 2;
  }
  /** string_length is the minimum length of proposed string and original string **/
  if (string_length > input->length) string_length = input->length;
  string_length /= 2;
  if (string_length > DOgLtrasMinimumScoreLength - 1) string_length = DOgLtrasMinimumScoreLength - 1;
  return (string_length);
}

static int LtrasTrfSameAsRequest(struct og_ctrl_ltras *ctrl_ltras, struct og_ltra_trfs *trfs,
    struct og_ltra_add_trf_input *input)
{
  struct og_ltra_add_trf_word *new_word;
  struct og_ltra_word *word_basic;
  struct og_ltra_trf *trf_basic;
  int i;

  if (input->nb_words != trfs->TrfBasicUsed) return (0);

  for (i = 0; i < input->nb_words; i++)
  {
    new_word = input->word + i;
    trf_basic = trfs->Trf + i;
    if (trf_basic->nb_words != 1) return (0); /** should be always true **/
    word_basic = trfs->Word + trf_basic->start_word + 0;

    /* start_position and length position should not be used
     * because they can be different, even though all the words
     * are the same, in the same order */
    if (word_basic->length != new_word->string_length) return (0);
    if (Ogmemicmp(trfs->Ba + word_basic->start, new_word->string, word_basic->length)) return (0);
  }

  return (1);
}


PUBLIC(int) OgLtrasTrfCalculateScoresFromTrf(void *handle, struct og_ltra_trfs *trfs, int Itrf, og_bool check_words_in_dictionary, int *pword_frequency,
    int *pexpression_frequency, double *pglobal_score, double *pfinal_score)
{
  struct og_ctrl_ltras *ctrl_ltras = (struct og_ctrl_ltras *) handle;

  struct og_ltra_trf *trf = trfs->Trf + Itrf;
  char buffer[DPcPathSize];
  int ibuffer = 0;
  int Fword = 0;
  og_bool same_lang = TRUE;
  int language_code = DOgLangNil;

  for (int i = 0; i < trf->nb_words; i++)
  {
    struct og_ltra_word *word = trfs->Word + trf->start_word + i;

    memcpy(buffer + ibuffer, trfs->Ba + word->start, word->length);

    ibuffer += word->length;

    if (i != (trf->nb_words - 1))
    {
      buffer[ibuffer++] = '\0';
      buffer[ibuffer++] = ' ';
    }

    if((language_code != word->language) && (language_code != DOgLangNil))
    {
      same_lang = FALSE;
    }
    language_code = word->language;

    if (i == 0)
    {
      Fword = word->frequency;
    }
    else if (Fword > word->frequency)
    {
      Fword = word->frequency;
    }
  }

  struct ltraf_score_ctx ctx[1];
  memset(ctx, 0, sizeof(struct ltraf_score_ctx));
  ctx->global_score = trf->global_score;
  ctx->buffer = buffer;
  ctx->ibuffer = ibuffer;
  ctx->same_lang = same_lang;
  ctx->language = language_code;
  ctx->Fword = Fword;

  LtrasTrfCalculateScores(ctrl_ltras, ctx, check_words_in_dictionary);

  if (pword_frequency) *pword_frequency = Fword;
  if (pexpression_frequency) *pexpression_frequency = ctx->Fexpr;
  if (pglobal_score) *pglobal_score = trf->global_score;
  if (pfinal_score) *pfinal_score = ctx->final_score;

  DONE;
}



static int LtrasTrfCalculateScoresFromInput(struct og_ctrl_ltras *ctrl_ltras, struct og_ltra_trfs *trfs,
    struct og_ltra_add_trf_input *input, og_bool check_words_in_dictionary, int *pword_frequency,
    int *pexpression_frequency, double *pglobal_score, double *pfinal_score)
{
  char buffer[DPcPathSize];
  int ibuffer = 0;
  int Fword = 0;
  og_bool same_lang = TRUE;
  int language_code = DOgLangNil;

  for (int i = 0; i < input->nb_words; i++)
  {
    struct og_ltra_add_trf_word *new_word = input->word + i;
    memcpy(buffer + ibuffer, new_word->string, new_word->string_length);
    ibuffer += new_word->string_length;

    if (i != (input->nb_words - 1))
    {
      buffer[ibuffer++] = '\0';
      buffer[ibuffer++] = ' ';
    }

    if((language_code != new_word->language) && (language_code != DOgLangNil))
    {
      same_lang = FALSE;
    }
    language_code = new_word->language;

    if (i == 0)
    {
      Fword = new_word->frequency;
    }
    else if (Fword > new_word->frequency)
    {
      Fword = new_word->frequency;
    }
  }

  struct ltraf_score_ctx ctx[1];
  memset(ctx, 0, sizeof(struct ltraf_score_ctx));
  ctx->global_score = input->score;
  ctx->buffer = buffer;
  ctx->ibuffer = ibuffer;
  ctx->same_lang = same_lang;
  ctx->language = language_code;
  ctx->Fword = Fword;

  LtrasTrfCalculateScores(ctrl_ltras, ctx, check_words_in_dictionary);

  if (pword_frequency) *pword_frequency = Fword;
  if (pexpression_frequency) *pexpression_frequency = ctx->Fexpr;
  if (pglobal_score) *pglobal_score = input->score;
  if (pfinal_score) *pfinal_score = ctx->final_score;

  DONE;
}



static int LtrasTrfCalculateScores(struct og_ctrl_ltras *ctrl_ltras, struct ltraf_score_ctx *ctx, og_bool check_words_in_dictionary)
{
    int Fexpr = 0;
    og_bool found = FALSE;
    if(ctx->same_lang && check_words_in_dictionary)
    {
      found = OgLtrasTrfCalculateFrequency(ctrl_ltras, ctx->ibuffer, (unsigned char *)ctx->buffer, ctx->language, &Fexpr);
    }
    if(!found)
    {
      Fexpr = 0;
    }

   double normalized_frequency = 0;

   if ((Fexpr != 0) || (ctx->Fword != 0))
   {
     //We use double type, otherwise following operations have interger type and can exceed integer
     //max number, then we have a log10 of a negative number which is not possible and we obtain a nan
     double Fmax = ctrl_ltras->max_word_frequency;

     double Ftotal = log10((double)Fexpr * (Fmax + 1) + ctx->Fword);
     double Fmax_norm = log10(Fmax * (Fmax + 1) + Fmax);

     normalized_frequency = Ftotal / Fmax_norm;
   }

   double score_factor = ctrl_ltras->input->score_factor;
   double global_score = ctx->global_score;
   double final_score = score_factor * global_score + (1 - score_factor) * normalized_frequency;

   ctx->Fexpr = Fexpr;
   ctx->final_score = final_score;

   DONE;
}

static int LtrasTrfGet(struct og_ctrl_ltras *ctrl_ltras, struct og_ltra_trfs *trfs, struct og_ltra_add_trf_input *input,
    int *pItrf)
{
  int i, equal;

  if (pItrf) *pItrf = (-1);

  for (i = 0; i < trfs->TrfUsed; i++)
  {
    IFE(equal = LtrasTrfEqual(ctrl_ltras, trfs, i, input));
    if (equal)
    {
      if (pItrf) *pItrf = i;
      return (1);
    }
  }
  return (0);
}

static int LtrasTrfEqual(struct og_ctrl_ltras *ctrl_ltras, struct og_ltra_trfs *trfs, int Itrf,
    struct og_ltra_add_trf_input *input)
{
  struct og_ltra_trf *trf = trfs->Trf + Itrf;
  struct og_ltra_add_trf_word *new_word;
  struct og_ltra_word *word;
  int i;

  if (trf->start != input->start) return (0);
  if (trf->length != input->length) return (0);

  if (trf->nb_words != input->nb_words) return (0);
  for (i = 0; i < input->nb_words; i++)
  {
    new_word = input->word + i;
    word = trfs->Word + trf->start_word + i;
    if (new_word->string_length != word->length) return (0);
    if (memcmp(new_word->string, trfs->Ba + word->start, word->length)) return (0);
  }

  return (1);
}

PUBLIC(og_status) OgLtrasTrfsDuplicate(void *handle, struct og_ltra_trfs *input, struct og_ltra_trfs **output)
{
  struct og_ctrl_ltras *ctrl_ltras = (struct og_ctrl_ltras *) handle;
  struct og_ltra_trfs *trfs;
  char erreur[DOgErrorSize];
  int size;

  IFn(trfs=(struct og_ltra_trfs *)malloc(sizeof(struct og_ltra_trfs)))
  {
    sprintf(erreur, "OgLtrasTrfsDuplicate: malloc error on trfs");
    OgErr(ctrl_ltras->herr, erreur);
    DPcErr;
  }
  memcpy(trfs, input, sizeof(struct og_ltra_trfs));

  trfs->TrfNumber = input->TrfNumber;
  size = trfs->TrfNumber * sizeof(struct og_ltra_trf);
  IFn(trfs->Trf=(struct og_ltra_trf *)malloc(size))
  {
    sprintf(erreur, "OgLtrasTrfsDuplicate: malloc error on Trf (%d bytes)", size);
    OgErr(ctrl_ltras->herr, erreur);
    return (0);
  }
  trfs->TrfUsed = input->TrfUsed;
  size = trfs->TrfUsed * sizeof(struct og_ltra_trf);
  memcpy(trfs->Trf, input->Trf, size);

  trfs->WordNumber = input->WordNumber;
  size = trfs->WordNumber * sizeof(struct og_ltra_word);
  IFn(trfs->Word=(struct og_ltra_word *)malloc(size))
  {
    sprintf(erreur, "OgLtrasTrfsDuplicate: malloc error on Word (%d bytes)", size);
    OgErr(ctrl_ltras->herr, erreur);
    return (0);
  }
  trfs->WordUsed = input->WordUsed;
  size = trfs->WordUsed * sizeof(struct og_ltra_word);
  memcpy(trfs->Word, input->Word, size);

  trfs->ModuleNumber = input->ModuleNumber;
  size = trfs->ModuleNumber * sizeof(struct og_ltra_module);
  IFn(trfs->Module=(struct og_ltra_module *)malloc(size))
  {
    sprintf(erreur, "OgLtrasTrfsDuplicate: malloc error on Module (%d bytes)", size);
    OgErr(ctrl_ltras->herr, erreur);
    return (0);
  }
  trfs->ModuleUsed = input->ModuleUsed;
  size = trfs->ModuleUsed * sizeof(struct og_ltra_module);
  memcpy(trfs->Module, input->Module, size);

  trfs->BaSize = input->BaSize;
  size = trfs->BaSize * sizeof(unsigned char);
  IFn(trfs->Ba=(unsigned char *)malloc(size))
  {
    sprintf(erreur, "OgLtrasTrfsDuplicate: malloc error on Ba (%d bytes)", size);
    OgErr(ctrl_ltras->herr, erreur);
    return (0);
  }
  trfs->BaUsed = input->BaUsed;
  size = trfs->BaUsed * sizeof(unsigned char);
  memcpy(trfs->Ba, input->Ba, size);

  IFn(trfs->htransposition=OgHeapInit(ctrl_ltras->hmsg,"ltras_htransposition",sizeof(int),DOgLtrasTrfsTransposition)) {
    size = sizeof(int) * DOgLtrasTrfsTransposition;
    sprintf(erreur, "OgLtrasTrfsDuplicate: OgHeapInit error on htransposition (%d bytes)", size);
    OgErr(ctrl_ltras->herr, erreur);
    return (0);
  }
  IF(OgHeapCopy(input->htransposition, trfs->htransposition)) return(0);

  *output = trfs;

  DONE;
}

PUBLIC(char *) OgLtrasModuleName(void *handle,int module_id)
{
  struct og_ctrl_ltras *ctrl_ltras = (struct og_ctrl_ltras *)handle;
  struct module *module = ctrl_ltras->Module + module_id;
  IFn(module_id) return("basic");
  return(ctrl_ltras->Ba+module->start_name);
}

PUBLIC(int) OgLtrasModuleId(void *handle, char *module_name, int *pmodule_id)
{
  struct og_ctrl_ltras *ctrl_ltras = (struct og_ctrl_ltras *) handle;
  struct module *module;
  int i;

  for (i = 0; i < ctrl_ltras->ModuleUsed; i++)
  {
    module = ctrl_ltras->Module + i;
    if (strcmp(module_name, ctrl_ltras->Ba + module->start_name)) continue;
    *pmodule_id = i;
    return (1);
  }

  return (0);
}

PUBLIC(og_status) OgLtrasTrfsMerge(void *handle, struct og_ltra_trfs *trfs, struct og_ltra_trfs *mother_trfs)
{
  int i;

  for (i = trfs->TrfBasicUsed; i < trfs->TrfUsed; i++)
  {
    IFE(LtrasTrfMerge(handle, trfs, mother_trfs, i));
  }

  DONE;
}

static int LtrasTrfMerge(struct og_ctrl_ltras *ctrl_ltras, struct og_ltra_trfs *trfs, struct og_ltra_trfs *mother_trfs,
    int Itrf)
{
  int i, same_trf = 0;

  for (i = mother_trfs->TrfBasicUsed; i < mother_trfs->TrfUsed; i++)
  {
    IFE(same_trf = LtrasTrfSame(ctrl_ltras, trfs, Itrf, mother_trfs, i));
    if (!same_trf) continue;
    IFE(LtrasTrfMergeScores(ctrl_ltras, trfs, Itrf, mother_trfs, i));
    break;
  }

  if (!same_trf)
  {
    IFE(OgLtrasTrfCopy(ctrl_ltras, trfs, Itrf, mother_trfs, 1));
  }

  DONE;
}

static int LtrasTrfSame(void *handle, struct og_ltra_trfs *trfs1, int Itrf1, struct og_ltra_trfs *trfs2, int Itrf2)
{
  struct og_ltra_trf *trf1 = trfs1->Trf + Itrf1;
  struct og_ltra_trf *trf2 = trfs2->Trf + Itrf2;
  struct og_ltra_word *word1, *word2;
  int i;

  if (trf1->basic != trf2->basic) return (0);
  if (trf1->start != trf2->start) return (0);
  if (trf1->length != trf2->length) return (0);
  if (trf1->nb_words != trf2->nb_words) return (0);
  for (i = 0; i < trf1->nb_words; i++)
  {
    word1 = trfs1->Word + trf1->start_word + i;
    word2 = trfs2->Word + trf2->start_word + i;
    if (word1->length != word2->length) return (0);
    if (memcmp(trfs1->Ba + word1->start, trfs2->Ba + word2->start, word1->length)) return (0);
  }
  return (1);
}

/*
 *  Merging score from trf1 into trf2
 *  We chose the best score
 */

static int LtrasTrfMergeScores(struct og_ctrl_ltras *ctrl_ltras, struct og_ltra_trfs *trfs1, int Itrf1,
    struct og_ltra_trfs *trfs2, int Itrf2)
{
  struct og_ltra_trf *trf1 = trfs1->Trf + Itrf1;
  struct og_ltra_trf *trf2 = trfs2->Trf + Itrf2;
  struct og_ltra_module *module;

  if (trf1->global_score > trf2->global_score)
  {
    trf2->global_score = trf1->global_score;
    trf2->start_module = (-1);
    for (int i = 0; i < trf1->nb_modules; i++)
    {
      module = trfs1->Module + trf1->start_module + i;
      IFE(LtrasAddModule(ctrl_ltras, trfs2, trf2, module->module_id));
    }
  }

  DONE;
}

PUBLIC(int) OgLtrasTrfGetBoundaries(struct og_ltra_trfs *trfs, int Itrf, int *ptrf_start_position,
    int *ptrf_end_position)
{
  struct og_ltra_trf *trf = trfs->Trf + Itrf;
  struct og_ltra_word *word;

  IFx(ptrf_start_position) *ptrf_start_position = 0;
  IFx(ptrf_end_position) *ptrf_end_position = 0;

  if (trf->nb_words <= 0) return (0);

  IFx(ptrf_start_position)
  {
    word = trfs->Word + trf->start_word;
    *ptrf_start_position = word->start_position;
  }

  IFx(ptrf_end_position)
  {
    word = trfs->Word + trf->start_word + trf->nb_words - 1;
    *ptrf_end_position = word->start_position + word->length_position;
  }

  return (1);
}

PUBLIC(int) OgLtrasTrfGetOriginalString(void *handle, struct og_ltra_trfs *trfs, int Itrf, unsigned char *origin, int origine_size,
    int *porigin_length)
{
  struct og_ctrl_ltras *ctrl_ltras = (struct og_ctrl_ltras *) handle;
  int found, trf_start_position, trf_end_position;
  IFE(found = OgLtrasTrfGetBoundaries(trfs, Itrf, &trf_start_position, &trf_end_position));
  *porigin_length = trf_end_position - trf_start_position;
  if(*porigin_length>origine_size)
  {
    char erreur[DOgErrorSize];
    sprintf(erreur,"OgLtrasTrfGetOriginalString: original string size = %d is greater than buffer size = %d",
        *porigin_length, origine_size);
    OgErr(ctrl_ltras->herr,erreur);
    DPcErr;
  }
  memcpy(origin,trfs->Ba + trf_start_position,*porigin_length);

  return (found);
}


PUBLIC(int) OgLtrasTrfInitTinput(struct og_ltra_trfs *trfs, int Itrf, struct og_ltra_add_trf_input *tinput, int module_id, unsigned char *words)
{
  struct og_ltra_trf *trf = trfs->Trf + Itrf;
  int word_length = 0;

  for (int i = 0; i < trf->nb_words; i++)
  {
    // To many words, solution skipped
    if (tinput->nb_words >= DOgLtrasAddTrfMaxNbWords)
    {
      break;
    }

    struct og_ltra_word *word = trfs->Word + trf->start_word + i;
    struct og_ltra_add_trf_word *new_word = tinput->word + tinput->nb_words;
    memcpy(words + word_length, trfs->Ba + word->start, word->length);
    new_word->string = words + word_length;
    new_word->string_length = word->length;
    word_length += new_word->string_length;
    new_word->frequency = word->frequency;
    new_word->base_frequency = word->base_frequency;
    new_word->start_position = word->start_position;
    new_word->length_position = word->length_position;
    new_word->language = word->language;
    tinput->nb_words++;
  }

  tinput->start = trf->start;
  tinput->length = trf->length;
  tinput->module_id = module_id;
  tinput->from_trf = Itrf;

  DONE;
}
