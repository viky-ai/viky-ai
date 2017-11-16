/*
 *  The Split module for linguistic transformations
 *  Copyright (c) 2010 Pertimm by Lo�s Rigouste
 *  Dev: October 2010
 *  Version 1.0
 */
#include "ltrasmsplit.h"

static int LtrasModuleSplit1(struct og_ltra_module_input *module_input, struct og_ltra_trfs *trfs, int nb_splits);
static int LtrasModuleSplit2(struct og_ltra_module_input *module_input, struct og_ltra_trfs *trfs, int nb_splits,
    int Itrf);
static og_status getStartAndLengthPositions(struct og_ctrl_split *ctrl_split, struct og_ltra_trfs *trfs,
    struct og_ltra_trf *trf, struct og_ltra_add_trf_word *word, int start);

void *OgLtrasModuleSplitInit(struct og_ltra_module_param *param)
{
  struct og_msg_param cmsg_param, *msg_param = &cmsg_param;
  struct og_aut_param caut_param, *aut_param = &caut_param;
  struct og_ctrl_split *ctrl_split;
  char ltras_split[DPcPathSize];
  char erreur[DOgErrorSize];
  char *WorkingDirectory;

  IFn(ctrl_split = (struct og_ctrl_split *) malloc(sizeof(struct og_ctrl_split)))
  {
    sprintf(erreur, "OgLtrasInit: malloc error on ctrl_split");
    OgErr(param->herr, erreur);
    return (0);
  }
  memset(ctrl_split, 0, sizeof(struct og_ctrl_split));

  ctrl_split->herr = param->herr;
  ctrl_split->hltras = param->hltras;
  ctrl_split->hmutex = param->hmutex;
  ctrl_split->cloginfo = param->loginfo;
  ctrl_split->loginfo = &ctrl_split->cloginfo;

  memset(msg_param, 0, sizeof(struct og_msg_param));
  msg_param->herr = ctrl_split->herr;
  msg_param->hmutex = ctrl_split->hmutex;
  msg_param->loginfo.trace = DOgMsgTraceMinimal + DOgMsgTraceMemory;
  msg_param->loginfo.where = ctrl_split->loginfo->where;
  msg_param->module_name = "ltra_module_split";
  IFn(ctrl_split->hmsg = OgMsgInit(msg_param))
  return (0);
  IF(OgMsgTuneInherit(ctrl_split->hmsg, param->hmsg))
  return (0);

  memset(aut_param, 0, sizeof(struct og_aut_param));
  aut_param->herr = ctrl_split->herr;
  aut_param->hmutex = ctrl_split->hmutex;
  aut_param->loginfo.trace = DOgAutTraceMinimal + DOgAutTraceMemory;
  aut_param->loginfo.where = ctrl_split->loginfo->where;
  aut_param->state_number = 0x1000;
  sprintf(aut_param->name, "ltras_module_cut");
  IFn(ctrl_split->ha_split = OgAutInit(aut_param))
  return (0);
  ctrl_split->max_small_word_length = 3;
  WorkingDirectory = OgLtrasWorkingDirectory(ctrl_split->hltras);
  if (WorkingDirectory[0]) sprintf(ltras_split, "%s/ling/ltras_cut.xml", WorkingDirectory);
  else strcpy(ltras_split, "ling/ltras_cut.xml");
  IF (LtrasModuleSplitReadConf( ctrl_split, ltras_split))
  return (0);

  ctrl_split->hstm = OgLtrasHstm(ctrl_split->hltras);
  IF(OgLtrasGetLevenshteinCosts(ctrl_split->hltras, ctrl_split->levenshtein_costs))
  return (0);

  return ((void *) ctrl_split);
}

int OgLtrasModuleSplitFlush(void *handle)
{
  struct og_ctrl_split *ctrl_split = (struct og_ctrl_split *) handle;
  IFE(OgMsgFlush(ctrl_split->hmsg));
  DPcFree(ctrl_split);
  DONE;
}

int OgLtrasModuleSplit(struct og_ltra_module_input *module_input, struct og_ltra_trfs *input,
    struct og_ltra_trfs **output, ogint64_t *elapsed)
{
  struct og_ctrl_split *ctrl_split = (struct og_ctrl_split *) module_input->handle;
  void *hltras = ctrl_split->hltras;
  struct og_ltra_trfs *trfs;
  char buffer[DPcPathSize];
  int i, found;

  ogint64_t micro_clock_start = OgMicroClock();

  ctrl_split->max_nb_splits = 1;
  IFE(found = OgLtrasGetParameterValue(ctrl_split->hltras, "split_max_nb_splits", DPcPathSize, buffer));
  if (found)
  {
    ctrl_split->max_nb_splits = atoi(buffer);
  }
  if (module_input->argc > 1)
  {
    ctrl_split->max_nb_splits = atoi(module_input->argv[1]);
  }

  IFE(OgLtrasTrfsDuplicate(hltras, input, &trfs));
  *output = trfs;

  ctrl_split->start_trf = 0;
  for (i = 1; i <= ctrl_split->max_nb_splits; i++)
  {
    IFE(LtrasModuleSplit1(module_input, trfs, i));
  }

  *elapsed = OgMicroClock() - micro_clock_start;

  DONE;
}

static int LtrasModuleSplit1(struct og_ltra_module_input *module_input, struct og_ltra_trfs *trfs, int nb_splits)
{
  struct og_ctrl_split *ctrl_split = (struct og_ctrl_split *) module_input->handle;
  int i, start_trf, TrfUsed;

  start_trf = ctrl_split->start_trf;
  ctrl_split->start_trf = trfs->TrfUsed;
  TrfUsed = trfs->TrfUsed;

  for (i = start_trf; i < TrfUsed; i++)
  {
    IFE(LtrasModuleSplit2(module_input, trfs, nb_splits, i));
  }

  DONE;
}

static int LtrasModuleSplit2(struct og_ltra_module_input *module_input, struct og_ltra_trfs *trfs, int nb_splits,
    int Itrf)
{
  struct og_ctrl_split *ctrl_split = (struct og_ctrl_split *) module_input->handle;
  struct og_ltra_add_trf_input ctinput, *tinput = &ctinput;
  int words_length;
  unsigned char words[DPcPathSize];
  struct og_ltra_trf *trf = trfs->Trf + Itrf;
  int i, j, retour;

  int language_code = 0;
  if (module_input->language_code != 0)
  {
    language_code = module_input->language_code;
  }

  // Keep original string of trfs to compare with transmormation string and
  // to calculate a levenchtein score
  unsigned char origin[DPcPathSize];
  int origin_length = 0;
  void *hltras = ctrl_split->hltras;
  IFE(OgLtrasTrfGetOriginalString(hltras, trfs, Itrf, origin, DPcPathSize, &origin_length));

  words_length = 0;
  memset(tinput, 0, sizeof(struct og_ltra_add_trf_input));
  tinput->nb_words++;
  for (i = 0; (i < trf->nb_words && tinput->nb_words < DOgLtrasAddTrfMaxNbWords); i++)
  {
    // To many word, solution skipped
    if (tinput->nb_words >= DOgLtrasAddTrfMaxNbWords)
    {
      break;
    }

    struct og_ltra_word *word = trfs->Word + trf->start_word + i;
    if (language_code == 0)
    {
      language_code = word->language;
    }

    struct og_ltra_add_trf_word *new_word = tinput->word + tinput->nb_words;
    memcpy(words + words_length, trfs->Ba + word->start, word->length);
    new_word->string = words + words_length;
    new_word->string_length = word->length;
    new_word->frequency = word->frequency;
    new_word->base_frequency = word->base_frequency;
    new_word->start_position = word->start_position;
    new_word->length_position = word->length_position;
    new_word->language = language_code;
    words_length += word->length;
    tinput->nb_words++;
  }

  tinput->start = trf->start;
  tinput->length = trf->length;
  tinput->module_id = module_input->id;
  tinput->from_trf = Itrf;

  for (i = 1; i < tinput->nb_words; i++)
  {
    struct og_ltra_add_trf_word *previous_new_word = tinput->word + i - 1;
    struct og_ltra_add_trf_word *new_word = tinput->word + i;
    int start = new_word->start_position;
    int string_length = new_word->string_length;
    unsigned char * string = new_word->string;
    memcpy(tinput->word + i - 1, tinput->word + i, sizeof(struct og_ltra_add_trf_word));
    for (j = 2; j < string_length; j += 2)
    {
      // update *trf in inner loop to protect against reallocation
      trf = trfs->Trf + Itrf;

      previous_new_word->string = string;
      previous_new_word->string_length = j;
      previous_new_word->start_position = 0;
      previous_new_word->length_position = j;
      previous_new_word->language = language_code;

      IFE(getStartAndLengthPositions(ctrl_split, trfs, trf, previous_new_word, start));

      if (j < 2 * (ctrl_split->max_small_word_length + 1))
      {
        // word is too small, check in the exception automaton
        IFE(retour = OgAutTry(ctrl_split->ha_split, previous_new_word->string_length, previous_new_word->string));
        if (retour != 2) continue;
      }

      new_word->string = string + j;
      new_word->string_length = string_length - j;
      new_word->start_position = j;
      new_word->length_position = string_length - j;

      IFE(getStartAndLengthPositions(ctrl_split, trfs, trf, new_word, start));

      if (string_length - j < 2 * (ctrl_split->max_small_word_length + 1))
      {
        // word is too small, check in the exception automaton
        IFE(retour = OgAutTry(ctrl_split->ha_split, new_word->string_length, new_word->string));
        if (retour != 2) continue;
      }

      unsigned char tranformed[DPcPathSize];
      int tranformed_length = 0;

      //Create the transformed string to calculate levenshtein distance with original string
      for (int k = 0; k < tinput->nb_words; k++)
      {
        memcpy(tranformed + tranformed_length, tinput->word[k].string, tinput->word[k].string_length);
        tranformed_length += tinput->word[k].string_length;
        if (k < (tinput->nb_words - 1))
        {
          memcpy(tranformed + tranformed_length, "\0 ", 2);
          tranformed_length += 2;
        }
      }
      double dlevenshtein_distance = OgStmLevenshteinFast(ctrl_split->hstm, origin_length, origin, tranformed_length,
          tranformed, ctrl_split->levenshtein_costs);
      IFE(dlevenshtein_distance);
      tinput->score = 1.0 - dlevenshtein_distance;
      IFE(OgLtrasTrfAdd(hltras, trfs, tinput, 0));
    }

    // at previous word location, restore original word
    previous_new_word->string = string;
    previous_new_word->string_length = string_length;
    previous_new_word->start_position = start;
    previous_new_word->length_position = string_length;
    new_word->string_length = string_length;
    new_word->string = string;
  }

  DONE;
}

// Fill start and length positions. If previous trf is a paste transformation, we use the transposition heap to
//compute start_position (start in the original string) and length_position (length in the original string)
static og_status getStartAndLengthPositions(struct og_ctrl_split *ctrl_split, struct og_ltra_trfs *trfs,
    struct og_ltra_trf *trf, struct og_ltra_add_trf_word *word, int start)
{
  int transposition_used = OgHeapGetCellsUsed(trfs->htransposition);
  int *transposition = OgHeapGetCell(trfs->htransposition, trf->start_transposition);
  IFn (transposition)
  DPcErr;

  int end_position = word->start_position + word->length_position - 2;
  if (trf->length_transposition > 0)
  {
    if ((end_position / 2) > transposition_used)
    {
      char erreur[DOgErrorSize];
      sprintf(erreur, "getStartAndLengthPositions: end_position %d is greater than transposition_used %d", end_position,
          transposition_used);
      OgErr(ctrl_split->herr, erreur);
      DPcErr;
    }
    word->start_position = transposition[word->start_position / 2] * 2;
    int end_transposition = transposition[end_position / 2] * 2 + 2;
    word->length_position = end_transposition - word->start_position;
  }
  else
  {
    word->start_position += start;
  }

  DONE;
}
