/*
 *  The Compound module for linguistic transformations
 *  Copyright (c) 2017 Pertimm by Caroline Collet
 *  Dev: January 2017
 *  Version 1.0
 */

#include <logltras.h>
#include <logltrap.h>
#include <logldi.h>

#define DOgLtrasCompoundMaxDictionariesNumber 10
#define DOgLtrasCompoundMaxNbCandidates 100
#define DOgLtrasCompoundMaxNbSolutions 1

struct og_ctrl_compound
{
  void *herr;
  void *hmsg;
  ogmutex_t *hmutex;
  struct og_loginfo loginfo[1];

  void *hltras;
  void *hstm;
  void *hldi;
  void *hltrap;

  int new_words_length;
  unsigned char new_words[DPcPathSize];
  struct og_ltra_add_trf_input tinput[1];

  int Itrf;
  int iuni;
  unsigned char *uni;
  struct og_ltra_trfs *trfs;
  int max_nb_candidates;
  int max_nb_solutions;
  double cut_cost;

  og_bool dict_loaded_msg[DOgLangMax];

  struct og_stm_levenshtein_input_param levenshtein_costs[1];
};

static int LtrasModuleCompound1(struct og_ltra_module_input *module_input, struct og_ltra_trfs *trfs, int Itrf);
static int LtrasModuleCompound1Token(void *context, struct og_ltrap_token *token);
static og_status getStartAndLengthPositions(struct og_ctrl_compound *ctrl_compound, struct og_ltra_trfs *trfs,
    struct og_ltra_trf *trf, struct og_ltra_add_trf_word *word, int start);

void *OgLtrasModuleCompoundInit(struct og_ltra_module_param *param)
{
  struct og_ctrl_compound *ctrl_compound = (struct og_ctrl_compound *) malloc(sizeof(struct og_ctrl_compound));
  IFn(ctrl_compound)
  {
    char erreur[DOgErrorSize];
    sprintf(erreur, "OgLtrasInit: malloc error on ctrl_compound");
    OgErr(param->herr, erreur);
    return (0);
  }
  memset(ctrl_compound, 0, sizeof(struct og_ctrl_compound));

  ctrl_compound->herr = param->herr;
  ctrl_compound->hltras = param->hltras;
  ctrl_compound->hmutex = param->hmutex;
  ctrl_compound->loginfo[0] = param->loginfo;

  struct og_msg_param msg_param[1];
  memset(msg_param, 0, sizeof(struct og_msg_param));
  msg_param->herr = ctrl_compound->herr;
  msg_param->hmutex = ctrl_compound->hmutex;
  msg_param->loginfo.trace = DOgMsgTraceMinimal + DOgMsgTraceMemory;
  msg_param->loginfo.where = ctrl_compound->loginfo->where;
  msg_param->module_name = "ltra_module_compound";
  IFn(ctrl_compound->hmsg = OgMsgInit(msg_param)) return (0);
  IF(OgMsgTuneInherit(ctrl_compound->hmsg, param->hmsg)) return (0);

  struct og_aut_param aut_param[1];
  memset(aut_param, 0, sizeof(struct og_aut_param));
  aut_param->herr = ctrl_compound->herr;
  aut_param->hmutex = ctrl_compound->hmutex;
  aut_param->loginfo.trace = DOgAutTraceMinimal + DOgAutTraceMemory;
  aut_param->loginfo.where = ctrl_compound->loginfo->where;
  aut_param->state_number = 0x1000;
  sprintf(aut_param->name, "ltras_module_compound");

  struct og_ltrap_param ltrap_param[1];
  memset(ltrap_param, 0, sizeof(struct og_ltrap_param));
  ltrap_param->herr = ctrl_compound->herr;
  ltrap_param->hmsg = ctrl_compound->hmsg;
  ltrap_param->hmutex = ctrl_compound->hmutex;
  ltrap_param->loginfo.trace = DOgLtrapTraceMinimal + DOgLtrapTraceMemory;
  if (ctrl_compound->loginfo->trace & DOgLtrasTraceModuleCompound)
  {
    ltrap_param->loginfo.trace |= DOgLtrapTraceAdd + DOgLtrapTraceBuild + DOgLtrapTraceSort + DOgLtrapTraceSend;
  }
  ltrap_param->loginfo.where = ctrl_compound->loginfo->where;
  ltrap_param->max_word_frequency = OgLtrasMaxWordFrequency(ctrl_compound->hltras);
  ltrap_param->ha_false = OgLtrasHaFalse(ctrl_compound->hltras);

  ltrap_param->ha_base = OgLtrasHaBase(ctrl_compound->hltras);
  ltrap_param->ha_swap = OgLtrasHaSwap(ctrl_compound->hltras);
  ltrap_param->ha_phon = OgLtrasHaPhon(ctrl_compound->hltras);

  IFn(ctrl_compound->hltrap = OgLtrapInit(ltrap_param)) return (0);

  ctrl_compound->hldi = OgLtrasHldi(ctrl_compound->hltras);

  ctrl_compound->hstm = OgLtrasHstm(ctrl_compound->hltras);
  IF(OgLtrasGetLevenshteinCosts(ctrl_compound->hltras, ctrl_compound->levenshtein_costs)) return (0);

  ctrl_compound->max_nb_candidates = DOgLtrasCompoundMaxNbCandidates;
  ctrl_compound->max_nb_solutions = DOgLtrasCompoundMaxNbSolutions;

  IF(OgStmGetSpaceCost(ctrl_compound->hstm, 0, TRUE, &ctrl_compound->cut_cost)) return (0);

  return ctrl_compound;
}

int OgLtrasModuleCompoundFlush(void *handle)
{
  struct og_ctrl_compound *ctrl_compound = (struct og_ctrl_compound *) handle;
  IFE(OgLtrapFlush(ctrl_compound->hltrap));

  IFE(OgMsgFlush(ctrl_compound->hmsg));
  DPcFree(ctrl_compound);
  DONE;
}

/*
 * the parameter max_nb_letters can only be zero or 1
 */
int OgLtrasModuleCompound(struct og_ltra_module_input *module_input, struct og_ltra_trfs *input,
    struct og_ltra_trfs **output, ogint64_t *elapsed)
{
  struct og_ctrl_compound *ctrl_compound = (struct og_ctrl_compound *) module_input->handle;

  ogint64_t micro_clock_start = OgMicroClock();

  IFE(OgLtrasTrfsDuplicate(ctrl_compound->hltras, input, &ctrl_compound->trfs));
  *output = ctrl_compound->trfs;

  for (int i = 0; i < ctrl_compound->trfs->TrfUsed; i++)
  {
    IFE(LtrasModuleCompound1(module_input, ctrl_compound->trfs, i));
  }

  *elapsed = OgMicroClock() - micro_clock_start;

  DONE;

}

struct compound_context
{
  struct og_ctrl_compound *ctrl_compound;
  struct og_ltra_module_input *module_input;
  struct og_ltra_add_trf_input tinput[1];
  int language_code;
};

static int LtrasModuleCompound1(struct og_ltra_module_input *module_input, struct og_ltra_trfs *trfs, int Itrf)
{
  struct og_ctrl_compound *ctrl_compound = (struct og_ctrl_compound *) module_input->handle;

  int ibuffer;
  unsigned char buffer[DPcPathSize];
  struct og_ltrap_input cinput, *input = &cinput;
  struct og_ltra_trf *trf = trfs->Trf + Itrf;

  /** we do not compound a transformation that is already compound **/
  if (trf->nb_words > 1) DONE;
  struct og_ltra_word *word = trfs->Word + trf->start_word;

  int language = DOgLangNil;
  if (module_input->language_code != 0)
  {
    language = module_input->language_code;
  }
  else if (word->language != 0)
  {
    language = word->language;
  }

  // get aut handle of matching dictionaries
  int haut_external_dictionaries_used = 0;
  int haut_external_dictionaries_size = DOgLtrasCompoundMaxDictionariesNumber;
  void *haut_external_dictionaries[haut_external_dictionaries_size];
  memset(haut_external_dictionaries, 0, sizeof(void *) * haut_external_dictionaries_size);

  int language_lang = OgIso639_3166ToLang(language);
  og_status status = OgLdiGetDictionaryAut(ctrl_compound->hldi, language_lang, DOgLdiTypeFreq,
      haut_external_dictionaries_size, haut_external_dictionaries, &haut_external_dictionaries_used);
  IFE(status);

  if (haut_external_dictionaries_used == 0)
  {
    if (!ctrl_compound->dict_loaded_msg[language_lang])
    {
      ctrl_compound->dict_loaded_msg[language_lang] = TRUE;

      og_string lang = (language_lang ? OgIso639ToCode(language_lang) : (unsigned char *) "");
      OgMsg(ctrl_compound->hmsg, "", DOgMsgDestInLog, "LtrasModuleCompound1: disable not such freq dictionary %s",
          lang);
    }
    CONT;
  }
  else
  {
    if (!ctrl_compound->dict_loaded_msg[language_lang])
    {
      ctrl_compound->dict_loaded_msg[language_lang] = TRUE;

      og_string lang = (language_lang ? OgIso639ToCode(language_lang) : (unsigned char *) "");
      OgMsg(ctrl_compound->hmsg, "", DOgMsgDestInLog, "LtrasModuleCompound1: freq dictionary %s loaded",
          lang);
    }
  }

  for (int i = 0; i < haut_external_dictionaries_used; i++)
  {
    void *haut_external_dictionary = haut_external_dictionaries[i];

    if (ctrl_compound->loginfo->trace & DOgLtrasTraceModuleCompound)
    {
      IFE(OgUniToCp(word->length, trfs->Ba + word->start, DPcPathSize, &ibuffer, buffer, DOgCodePageUTF8, 0, 0));
      OgMsg(ctrl_compound->hmsg, "", DOgMsgDestInLog, "LtrasModuleCompound1: working on '%s' with Itrf=%d on aut=%s",
          buffer, Itrf, OgAutName(haut_external_dictionary));
    }

    struct compound_context ctx[1];
    memset(ctx, 0, sizeof(struct compound_context));
    ctx->ctrl_compound = ctrl_compound;
    ctx->module_input = module_input;
    ctx->language_code = language;

    memset(input, 0, sizeof(struct og_ltrap_input));
    input->haut_external_dictionary = haut_external_dictionary;
    input->lexicon_type = DOgLtrapLexiconTypeCompound;
    input->language = language;
    input->iuni = word->length;
    input->uni = trfs->Ba + word->start;
    input->send_token = LtrasModuleCompound1Token;
    input->context = ctx;
    input->synchronization_length = DOgLtrapSynchronizationLength;
    input->max_candidates = ctrl_compound->max_nb_candidates;
    input->max_solutions = ctrl_compound->max_nb_solutions;
    input->score_factor = OgLtrasScoreFactor(ctrl_compound->hltras);
    input->cut_cost = ctrl_compound->cut_cost;

    ctrl_compound->iuni = input->iuni;
    ctrl_compound->uni = input->uni;
    ctrl_compound->Itrf = Itrf;

    IFE(OgLtrap(ctrl_compound->hltrap, input));

  }

  DONE;
}

static int LtrasModuleCompound1Token(void *context, struct og_ltrap_token *token)
{
  struct compound_context *ctx = (struct compound_context *) context;
  struct og_ltra_module_input *module_input = ctx->module_input;
  struct og_ctrl_compound *ctrl_compound = ctx->ctrl_compound;
  void *hltras = ctrl_compound->hltras;

  struct og_ltra_add_trf_input *tinput = ctx->tinput;

  if (!token->solution_complete) DONE;

  if (token->starting)
  {
    memset(tinput, 0, sizeof(struct og_ltra_add_trf_input));
    ctrl_compound->new_words_length = 0;
  }

  // Check if can add more words
  if ((tinput->nb_words + 1) >= DOgLtrasAddTrfMaxNbWords) DONE;

  // Keep original string of trfs to compare with transformation string and
  // to calculate a levenchtein score
  unsigned char origin[DPcPathSize];
  int origin_length = 0;

  og_status status = OgLtrasTrfGetOriginalString(hltras, ctrl_compound->trfs, ctrl_compound->Itrf, origin, DPcPathSize, &origin_length);
  IFE(status);

  struct og_ltra_trf *trf = ctrl_compound->trfs->Trf + ctrl_compound->Itrf;
  struct og_ltra_word *word = ctrl_compound->trfs->Word + trf->start_word;

  memcpy(ctrl_compound->new_words + ctrl_compound->new_words_length, token->string, token->length_string);
  struct og_ltra_add_trf_word *new_word = tinput->word + tinput->nb_words;
  new_word->string = ctrl_compound->new_words + ctrl_compound->new_words_length;
  new_word->string_length = token->length_string;
  new_word->frequency = token->frequency;
  new_word->base_frequency = word->base_frequency;
  new_word->start_position = token->position;
  new_word->length_position = token->length;
  new_word->language = ctx->language_code;

  struct og_ltra_trfs *trfs = ctrl_compound->trfs;
  IFE(getStartAndLengthPositions(ctrl_compound, trfs, trf, new_word, word->start_position));

  ctrl_compound->new_words_length += token->length_string;
  tinput->nb_words++;

  if (token->ending)
  {
    int to_length = 0;
    unsigned char to[DPcPathSize];

    // IFE(LtrasModuleCompound1Adjust(module_input));
    tinput->start = trf->start;
    tinput->length = trf->length;
    tinput->module_id = module_input->id;
    tinput->from_trf = ctrl_compound->Itrf;
    tinput->final = 1;
    for (int i = 0; i < tinput->nb_words; i++)
    {
      new_word = tinput->word + i;
      if (i > 0)
      {
        memcpy(to + to_length, "\0 ", 2);
        to_length += 2;
      }
      memcpy(to + to_length, new_word->string, new_word->string_length);
      to_length += new_word->string_length;
    }

    og_bool is_false = OgLtrasIsFalseTransformation(hltras, word->length_position, trfs->Ba + word->start_position,
        to_length, to);
    IFE(is_false);
    if (!is_false)
    {
      double dlevenshtein_distance = OgStmLevenshteinFast(ctrl_compound->hstm, origin_length, origin, to_length, to,
          ctrl_compound->levenshtein_costs);
      IFE(dlevenshtein_distance);
      tinput->score = 1.0 - dlevenshtein_distance;
      IFE(OgLtrasTrfAdd(hltras, ctrl_compound->trfs, tinput, 0));
    }
  }

  DONE;
}

// Fill start and length positions. If previous trf is a paste transformation, we use the transposition heap to
//compute start_position (start in the original string) and length_position (length in the original string)
static og_status getStartAndLengthPositions(struct og_ctrl_compound *ctrl_compound, struct og_ltra_trfs *trfs,
    struct og_ltra_trf *trf, struct og_ltra_add_trf_word *word, int start)
{
  int transposition_used = OgHeapGetCellsUsed(trfs->htransposition);
  int *transposition = OgHeapGetCell(trfs->htransposition, trf->start_transposition);
  IFn (transposition) DPcErr;

  int end_position = word->start_position + word->length_position - 2;
  if (trf->length_transposition > 0)
  {
    if ((end_position / 2) > transposition_used)
    {
      char erreur[DOgErrorSize];
      sprintf(erreur, "getStartAndLengthPositions: end_position %d is greater than transposition_used %d", end_position,
          transposition_used);
      OgErr(ctrl_compound->herr, erreur);
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
