/*
 *  The Reference module for linguistic transformations
 *  Copyright (c) 2014 Pertimm by Patrick Constant
 *  Dev: December 2014
 *  Version 1.1
 */
#include <logltras.h>
#include <logheap.h>

#define DOgLtrasModuleReferenceMaxNbLettersForPart  3

struct og_ctrl_reference
{
  void *herr, *hmsg, *hltras;
  ogmutex_t *hmutex;
  struct og_loginfo cloginfo, *loginfo;
  struct og_stm_levenshtein_input_param levenshtein_costs[1];
  void *hstm;
  og_heap hba;
  og_heap hitrf;
  og_heap hposition;
  int no_dictionary_frequency;
  int TrfUsed;
};

static int LtrasModuleReferenceSingle(struct og_ltra_module_input *module_input, struct og_ltra_trfs *trfs, int Itrf);
static int LtrasModuleReference1(struct og_ltra_module_input *module_input, struct og_ltra_trfs *trfs, int Itrf);
static int LtrasModuleReference2(struct og_ltra_module_input *module_input, struct og_ltra_trfs *trfs, int Itrf_next);
static int LtrasModuleReference3(struct og_ltra_module_input *module_input, struct og_ltra_trfs *trfs);
static int LtrasModuleReferenceMarkTrf(struct og_ltra_trfs *trfs, int Itrf);
static int LtrasModuleReferenceIsMarkedTrf(struct og_ltra_trfs *trfs, int Itrf);
static int LtrasModuleReferenceIsAdjacentTrf(struct og_ctrl_reference *ctrl_reference, struct og_ltra_trfs *trfs,
    int Itrf1, int Itrf2);
static int LtrasModuleReferenceIsValidTrf(struct og_ltra_trfs *trfs, int Itrf);
static int LtrasModuleReferenceIsValidWord(struct og_ltra_trfs *trfs, struct og_ltra_trf *trf,
    struct og_ltra_word *word);
static int LtrasModuleReferenceAdjustLength(struct og_ltra_module_input *module_input, struct og_ltra_trfs *trfs,
    struct og_ltra_add_trf_word *new_word);

void *OgLtrasModuleReferenceInit(struct og_ltra_module_param *param)
{
  struct og_msg_param cmsg_param, *msg_param = &cmsg_param;
  struct og_ctrl_reference *ctrl_reference;
  char erreur[DOgErrorSize];
  double dfrequency;

  IFn(ctrl_reference=(struct og_ctrl_reference *)malloc(sizeof(struct og_ctrl_reference)))
  {
    sprintf(erreur, "OgLtrasInit: malloc error on ctrl_reference");
    OgErr(param->herr, erreur);
    return (0);
  }
  memset(ctrl_reference, 0, sizeof(struct og_ctrl_reference));

  ctrl_reference->herr = param->herr;
  ctrl_reference->hltras = param->hltras;
  ctrl_reference->hmutex = param->hmutex;
  ctrl_reference->cloginfo = param->loginfo;
  ctrl_reference->loginfo = &ctrl_reference->cloginfo;

  memset(msg_param, 0, sizeof(struct og_msg_param));
  msg_param->herr = ctrl_reference->herr;
  msg_param->hmutex = ctrl_reference->hmutex;
  msg_param->loginfo.trace = DOgMsgTraceMinimal + DOgMsgTraceMemory;
  msg_param->loginfo.where = ctrl_reference->loginfo->where;
  msg_param->module_name = "ltra_module_reference";
  IFn(ctrl_reference->hmsg=OgMsgInit(msg_param)) return (0);
  IF(OgMsgTuneInherit(ctrl_reference->hmsg,param->hmsg)) return (0);

  /** needs to be called so that we have the right max frequency **/
  OgLtrasHaBase(ctrl_reference->hltras);
  OgLtrasGetFrequencyFromNormalizedFrequency(ctrl_reference->hltras, 0.9, &dfrequency);
  ctrl_reference->no_dictionary_frequency = (int) dfrequency;

  ctrl_reference->hstm = OgLtrasHstm(ctrl_reference->hltras);
  IF(OgLtrasGetLevenshteinCosts(ctrl_reference->hltras, ctrl_reference->levenshtein_costs)) return (0);

  IFn(ctrl_reference->hitrf=OgHeapInit(ctrl_reference->hmsg,"ref_itrf",sizeof(int),0x20)) return (0);
  IFn(ctrl_reference->hba=OgHeapInit(ctrl_reference->hmsg,"ref_ba",sizeof(unsigned char),0x100)) return (0);
  IFn(ctrl_reference->hposition=OgHeapInit(ctrl_reference->hmsg,"ref_position",sizeof(int),0x100)) return (0);

  return ((void *) ctrl_reference);
}

int OgLtrasModuleReferenceFlush(void *handle)
{
  struct og_ctrl_reference *ctrl_reference = (struct og_ctrl_reference *) handle;
  IFE(OgHeapFlush(ctrl_reference->hitrf));
  IFE(OgHeapFlush(ctrl_reference->hba));
  IFE(OgHeapFlush(ctrl_reference->hposition));
  IFE(OgMsgFlush(ctrl_reference->hmsg));
  DPcFree(ctrl_reference);
  DONE;
}

int OgLtrasModuleReference(struct og_ltra_module_input *module_input, struct og_ltra_trfs *input,
    struct og_ltra_trfs **output, ogint64_t *elapsed)
{
  struct og_ctrl_reference *ctrl_reference = (struct og_ctrl_reference *) module_input->handle;
  void *hltras = ctrl_reference->hltras;
  struct og_ltra_trfs *trfs;
  int i;

  ogint64_t micro_clock_start = OgMicroClock();

  IFE(OgLtrasTrfsDuplicate(hltras, input, &trfs));
  *output = trfs;
  ctrl_reference->TrfUsed = trfs->TrfUsed;

  for (i = 0; i < ctrl_reference->TrfUsed; i++)
  {
    IFE(LtrasModuleReferenceSingle(module_input, trfs, i));
  }

  for (i = 0; i < ctrl_reference->TrfUsed; i++)
  {
    IFE(LtrasModuleReference1(module_input, trfs, i));
  }

  *elapsed = OgMicroClock() - micro_clock_start;

  DONE;
}

/*
 * In the case of example such as "reference mx mv23km du produit"
 * we want to separate "mv23km" into "mv 23 km" so that reference mv 23 km can still be found
 * otherwise we would get only  mxmv 23 kmdu, which might not be the right reference
 */
static int LtrasModuleReferenceSingle(struct og_ltra_module_input *module_input, struct og_ltra_trfs *trfs, int Itrf)
{
  struct og_ctrl_reference *ctrl_reference = (struct og_ctrl_reference *) module_input->handle;
  int is_valid_trf;
  IFE(is_valid_trf = LtrasModuleReferenceIsValidTrf(trfs, Itrf));
  if (!is_valid_trf) DONE;

  IFE(OgHeapResetWithoutReduce(ctrl_reference->hitrf));
  IFE(OgHeapAppend(ctrl_reference->hitrf, 1, &Itrf));

  IFE(LtrasModuleReference3(module_input, trfs));
  DONE;
}

/*
 * We try to find a list of trf starting with Itrf_start, that are a candidate to a reference
 * This list is build into the htrf heap.
 */
static int LtrasModuleReference1(struct og_ltra_module_input *module_input, struct og_ltra_trfs *trfs, int Itrf_start)
{
  struct og_ctrl_reference *ctrl_reference = (struct og_ctrl_reference *) module_input->handle;
  int i, is_valid_trf, itrf_used;

  if (LtrasModuleReferenceIsMarkedTrf(trfs, Itrf_start)) DONE;

  IFE(is_valid_trf = LtrasModuleReferenceIsValidTrf(trfs, Itrf_start));
  if (!is_valid_trf) DONE;

  IFE(OgHeapResetWithoutReduce(ctrl_reference->hitrf));
  IFE(OgHeapAppend(ctrl_reference->hitrf, 1, &Itrf_start));
  LtrasModuleReferenceMarkTrf(trfs, Itrf_start);

  if (Itrf_start + 1 >= ctrl_reference->TrfUsed)
  {
    IFE(LtrasModuleReference3(module_input, trfs));
    DONE;
  }

  for (i = Itrf_start + 1; i < ctrl_reference->TrfUsed; i++)
  {
    itrf_used = OgHeapGetCellsUsed(ctrl_reference->hitrf);
    IFE(LtrasModuleReference2(module_input, trfs, i));
    IFE(OgHeapSetCellsUsed(ctrl_reference->hitrf, itrf_used));
  }

  DONE;
}

static int LtrasModuleReference2(struct og_ltra_module_input *module_input, struct og_ltra_trfs *trfs, int Itrf_next)
{
  struct og_ctrl_reference *ctrl_reference = (struct og_ctrl_reference *) module_input->handle;
  int *itrf, itrf_used, Itrf_last;
  int i, is_adjacent_trf, is_valid_trf;

  itrf_used = OgHeapGetCellsUsed(ctrl_reference->hitrf);
  IFn(itrf=OgHeapGetCell(ctrl_reference->hitrf,0)) DPcErr;
  Itrf_last = itrf[itrf_used - 1];

  if (LtrasModuleReferenceIsMarkedTrf(trfs, Itrf_next)) DONE;

  IFE(is_adjacent_trf = LtrasModuleReferenceIsAdjacentTrf(ctrl_reference, trfs, Itrf_last, Itrf_next));
  if (!is_adjacent_trf) DONE;

  IFE(is_valid_trf = LtrasModuleReferenceIsValidTrf(trfs, Itrf_next));
  if (!is_valid_trf)
  {
    IFE(LtrasModuleReference3(module_input, trfs));
    DONE;
  }
  IFE(OgHeapAppend(ctrl_reference->hitrf, 1, &Itrf_next));
  LtrasModuleReferenceMarkTrf(trfs, Itrf_next);

  if (Itrf_next + 1 >= ctrl_reference->TrfUsed)
  {
    IFE(LtrasModuleReference3(module_input, trfs));
    DONE;
  }
  for (i = Itrf_next + 1; i < ctrl_reference->TrfUsed; i++)
  {
    itrf_used = OgHeapGetCellsUsed(ctrl_reference->hitrf);
    IFE(LtrasModuleReference2(module_input, trfs, i));
    IFE(OgHeapSetCellsUsed(ctrl_reference->hitrf, itrf_used));
  }
  DONE;
}

/*
 * Solution goes from Itrf_start included to Itrf_last included
 * we concatenate all the string from all the words, and then we cut it at each change between letters and digits
 * and build a solution according to those cuts
 */
static int LtrasModuleReference3(struct og_ltra_module_input *module_input, struct og_ltra_trfs *trfs)
{
  struct og_ctrl_reference *ctrl_reference = (struct og_ctrl_reference *) module_input->handle;
  void *hltras = ctrl_reference->hltras;
  char erreur[DOgErrorSize];
  int *itrf, itrf_used;
  int w, p, last_position, *position;

  itrf_used = OgHeapGetCellsUsed(ctrl_reference->hitrf);
  IFn(itrf=OgHeapGetCell(ctrl_reference->hitrf,0)) DPcErr;

  IFE(OgHeapResetWithoutReduce(ctrl_reference->hba));
  IFE(OgHeapResetWithoutReduce(ctrl_reference->hposition));

  for (int i = 0; i < itrf_used; i++)
  {
    struct og_ltra_trf *trf = trfs->Trf + itrf[i];
    for (w = 0; w < trf->nb_words; w++)
    {
      struct og_ltra_word *word = trfs->Word + trf->start_word + w;
      IFE(OgHeapAppend(ctrl_reference->hba, word->length, trfs->Ba + word->start));
      for (p = 0; p < word->length; p += 2)
      {
        int pos = p + word->start_position;
        IFE(OgHeapAppend(ctrl_reference->hposition, 1, &pos));
        last_position = pos;
      }
    }
  }
  last_position += 2;
  IFE(OgHeapAppend(ctrl_reference->hposition, 1, &last_position));

  int ba_used = OgHeapGetCellsUsed(ctrl_reference->hba);
  unsigned char *ba = OgHeapGetCell(ctrl_reference->hba, 0);

  /** Do not take into account string without any digit : "on est" "il le" etc. **/
  int digit_exist = 0;
  for (int i = 0; i < ba_used; i += 2)
  {
    int c = (ba[i] << 8) + ba[i + 1];
    if (OgUniIsdigit(c))
    {
      digit_exist = 1;
      break;
    }
  }
  if (!digit_exist) DONE;

  if (ctrl_reference->loginfo->trace & DOgLtrasTraceModuleRef)
  {
    int ibuffer;
    unsigned char buffer[DPcPathSize];
    IFn(ba) DPcErr;
    IFE(OgUniToCp(ba_used,ba,DPcPathSize,&ibuffer,buffer,DOgCodePageUTF8,0,0));
    OgMsg(ctrl_reference->hmsg, "", DOgMsgDestInLog, "LtrasModuleReference3: found '%s'", buffer);
  }

  struct og_ltra_add_trf_input tinput[1];
  memset(tinput, 0, sizeof(struct og_ltra_add_trf_input));
  struct og_ltra_trf *trf = trfs->Trf + itrf[0];
  tinput->start = trf->start;
  trf = trfs->Trf + itrf[itrf_used - 1];
  tinput->length = (trf->start + trf->length) - tinput->start;
  tinput->module_id = module_input->id;
  tinput->from_trf = itrf[0];
  tinput->final = 1;

  IFn(position=OgHeapGetCell(ctrl_reference->hposition,0)) DPcErr;
  struct og_ltra_add_trf_word *new_word;

  int state = 1, start = 0, end = 0;
  for (int i = 0; !end; i += 2)
  {
    // Check if can add more words
    if (tinput->nb_words >= DOgLtrasAddTrfMaxNbWords)
    {
      break;
    }

    int c = (ba[i] << 8) + ba[i + 1];
    int is_digit = 0, is_alpha = 0;
    if (i >= ba_used) end = 1;
    if (OgUniIsdigit(c)) is_digit = 1;
    else if (OgUniIsalpha(c)) is_alpha = 1;
    else if (!end)
    {
      sprintf(erreur, "LtrasModuleReference3: char %d is not a digit nor a letter", c);
      OgErr(ctrl_reference->herr, erreur);
      DPcErr;
    }
    switch (state)
    {
      case 1: /* start of word */
        if (is_digit) state = 2;
        else if (is_alpha) state = 3;
        break;

      case 2: /* in digit */
        if (is_alpha || end)
        {
          new_word = tinput->word + tinput->nb_words;
          new_word->string = ba + start;
          new_word->string_length = i - start;
          new_word->frequency = ctrl_reference->no_dictionary_frequency;
          new_word->base_frequency = ctrl_reference->no_dictionary_frequency;
          new_word->start_position = position[start / 2];
          new_word->length_position = position[i / 2] - position[start / 2];
          if (new_word->length_position < 0) new_word->length_position = 0;
          IFE(LtrasModuleReferenceAdjustLength(module_input, trfs, new_word));
          tinput->nb_words++;
          start = i;
          state = 3;
        }
        break;

      case 3: /* in letters */
        if (is_digit || end)
        {
          new_word = tinput->word + tinput->nb_words;
          new_word->string = ba + start;
          new_word->string_length = i - start;
          new_word->frequency = ctrl_reference->no_dictionary_frequency;
          new_word->base_frequency = ctrl_reference->no_dictionary_frequency;
          new_word->start_position = position[start / 2];
          new_word->length_position = position[i / 2] - position[start / 2];
          if (new_word->length_position < 0) new_word->length_position = 0;
          IFE(LtrasModuleReferenceAdjustLength(module_input, trfs, new_word));
          tinput->nb_words++;
          start = i;
          state = 2;
        }
        break;
    }
  }

  unsigned char *origin = ba;
  int origin_length = ba_used;

  unsigned char transformed[DPcPathSize];
  int transformed_length = 0;

  //Create the transformed string to calculate levenshtein distance with original string
  for (int k = 0; k < tinput->nb_words; k++)
  {
    memcpy(transformed + transformed_length, tinput->word[k].string, tinput->word[k].string_length);
    transformed_length += tinput->word[k].string_length;
    if (k < (tinput->nb_words - 1))
    {
      memcpy(transformed + transformed_length, "\0 ", 2);
      transformed_length += 2;
    }
  }

  double dlevenshtein_distance = OgStmLevenshteinFast(ctrl_reference->hstm, origin_length, origin, transformed_length,
      transformed, ctrl_reference->levenshtein_costs);
  IFE(dlevenshtein_distance);
  tinput->score = 1.0 - dlevenshtein_distance;
  IFE(OgLtrasTrfAdd(hltras, trfs, tinput, 0));

  DONE;
}

static int LtrasModuleReferenceMarkTrf(struct og_ltra_trfs *trfs, int Itrf)
{
  struct og_ltra_trf *trf = trfs->Trf + Itrf;
  trf->marker = 1;
  DONE;
}

static int LtrasModuleReferenceIsMarkedTrf(struct og_ltra_trfs *trfs, int Itrf)
{
  struct og_ltra_trf *trf = trfs->Trf + Itrf;
  if (trf->marker) return (1);
  return (0);
}

static int LtrasModuleReferenceIsAdjacentTrf(struct og_ctrl_reference *ctrl_reference, struct og_ltra_trfs *trfs,
    int Itrf1, int Itrf2)
{
  int i, trf1_end_position, trf2_start_position;

  IFE(OgLtrasTrfGetBoundaries(trfs, Itrf1, 0, &trf1_end_position));
  IFE(OgLtrasTrfGetBoundaries(trfs, Itrf2, &trf2_start_position, 0));
  if (!(trf1_end_position <= trf2_start_position)) return (0);

  for (i = 0; i < ctrl_reference->TrfUsed; i++)
  {
    if (i == Itrf1 || i == Itrf2) continue;
    int trf_start_position, trf_end_position;
    IFE(OgLtrasTrfGetBoundaries(trfs, i, &trf_start_position, &trf_end_position));
    if (trf1_end_position <= trf_start_position && trf_end_position <= trf2_start_position) return (0);
  }
  return (1);
}

/*
 * A valid part is as follows, each word must :
 * contains only digits
 * contains digits and letters
 * contain letters but less than DOgLtrasModuleReferenceMaxNbLettersForPart letters
 * if one of the words of the trf does match those criteria, the trf is discarded
 */

static int LtrasModuleReferenceIsValidTrf(struct og_ltra_trfs *trfs, int Itrf)
{
  struct og_ltra_trf *trf = trfs->Trf + Itrf;
  struct og_ltra_word *word;
  int i, is_valid_word;

  for (i = 0; i < trf->nb_words; i++)
  {
    word = trfs->Word + trf->start_word + i;
    IFE(is_valid_word = LtrasModuleReferenceIsValidWord(trfs, trf, word));
    if (!is_valid_word) return (0);
  }

  return (1);
}

static int LtrasModuleReferenceIsValidWord(struct og_ltra_trfs *trfs, struct og_ltra_trf *trf,
    struct og_ltra_word *word)
{
  int i, is = word->length;
  unsigned char *s = trfs->Ba + word->start;
  int is_digit = 0, is_not_digit = 0, is_letter = 0, is_not_letter = 0, is_neither_digit_nor_letter = 0;

  for (i = 0; i < is; i += 2)
  {
    int c = (s[i] << 8) + s[i + 1];
    if (OgUniIsdigit(c))
    {
      is_digit = 1;
      is_not_letter = 1;
    }
    else if (OgUniIsalpha(c))
    {
      is_letter = 1;
      is_not_digit = 1;
    }
    else
    {
      is_not_digit = 1;
      is_not_letter = 1;
      is_neither_digit_nor_letter = 1;
    }
  }

  if (is_digit && !is_not_digit) return (1);
  if (is_digit && is_letter && !is_neither_digit_nor_letter) return (1);
  if (is_letter && !is_not_letter)
  {
    if (is / 2 <= DOgLtrasModuleReferenceMaxNbLettersForPart) return (1);
  }

  return (0);
}

static int LtrasModuleReferenceAdjustLength(struct og_ltra_module_input *module_input, struct og_ltra_trfs *trfs,
    struct og_ltra_add_trf_word *new_word)
{
  struct og_ctrl_reference *ctrl_reference = (struct og_ctrl_reference *) module_input->handle;
  int ibuffer;
  unsigned char buffer[DPcPathSize];
  unsigned char *uni;
  uni = trfs->Ba + trfs->start_text;

  int iword = new_word->length_position;
  unsigned char *word = uni + new_word->start_position;

  int new_length_position = new_word->length_position;
  for (int i = iword - 2; i >= 0; i -= 2)
  {
    int c = (word[i] << 8) + word[i + 1];
    if (OgUniIsdigit(c))
    {
      new_length_position = i + 2;
      break;
    }
    if (OgUniIsalpha(c))
    {
      new_length_position = i + 2;
      break;
    }
  }

  if (new_length_position != new_word->length_position)
  {
    IFE(OgUniToCp(new_word->length_position,uni+new_word->start_position,DPcPathSize,&ibuffer,buffer,DOgCodePageUTF8,0,0));
    if (ctrl_reference->loginfo->trace & DOgLtrasTraceModuleRef)
    {
      OgMsg(ctrl_reference->hmsg, "", DOgMsgDestInLog, "LtrasModuleReferenceAdjustLength: adjusting length of '%s' from %d to %d", buffer,
          new_word->length_position, new_length_position);
    }
    new_word->length_position = new_length_position;
  }

  DONE;
}

