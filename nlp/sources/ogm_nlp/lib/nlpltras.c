/*
 *  Handling spell checking for Natural Language Processing library
 *  Copyright (c) 2017 Pertimm, by Patrick Constant
 *  Dev : November 2017
 *  Version 1.0
 */
#include "ogm_nlp.h"
#include <logltras.h>

static og_status NlpLtrasWord(og_nlp_th ctrl_nlp_th, int Irequest_word);
static og_status NlpLtrasWordPackage(og_nlp_th ctrl_nlp_th, int Irequest_word,
    struct interpret_package *interpret_package);
static og_status NlpLtrasAddWord(og_nlp_th ctrl_nlp_th, int Irequest_word_basic, int length_corrected_word,
    og_string corrected_word, double spelling_score);

og_status NlpLtrasInit(og_nlp_th ctrl_nlp_th)
{
  struct og_ltras_param param[1];
  memset(param, 0, sizeof(struct og_ltras_param));
  param->herr = ctrl_nlp_th->herr;
  param->hmsg = ctrl_nlp_th->hmsg;
  param->hmutex = ctrl_nlp_th->hmutex;

  param->loginfo.trace = DOgLtrasTraceMinimal;
  if (ctrl_nlp_th->loginfo->trace & DOgNlpTraceLtrasDetail)
  {
    param->loginfo.trace |= DOgLtrasTraceMemory + DOgLtrasTraceModuleFlowChart + DOgLtrasTraceModuleCalls
        + DOgLtrasTraceSelection;
  }
  param->loginfo.where = ctrl_nlp_th->loginfo->where;
  sprintf(param->caller_label, "%s_ltras", ctrl_nlp_th->name);
  strcpy(param->WorkingDirectory, ctrl_nlp_th->ctrl_nlp->WorkingDirectory);
  /* TODO: all lt inherit from Lt[0], thus Lt[0] is the only one
   * to actually do the automatons initializations */
  //ltras_param->hltras_to_inherit = ctrl_ssrv->Lt[0].hltras;
  strcpy(param->WorkingDirectory, ctrl_nlp_th->ctrl_nlp->WorkingDirectory);
  IFN(ctrl_nlp_th->hltras = OgLtrasInit(param)) DPcErr;
  DONE;
}

og_status NlpLtrasFlush(og_nlp_th ctrl_nlp_th)
{
  if (ctrl_nlp_th->hltras == NULL) CONT;

  IFE(OgLtrasFlush(ctrl_nlp_th->hltras));
  ctrl_nlp_th->hltras = NULL;

  DONE;
}

/**
 * Do a spell checking on each words and also a spell checking on the whole sentence
 */
og_status NlpLtras(og_nlp_th ctrl_nlp_th)
{
  if (ctrl_nlp_th->hltras == NULL) CONT;

  int request_word_used = OgHeapGetCellsUsed(ctrl_nlp_th->hrequest_word);
  for (int i = 0; i < request_word_used; i++)
  {
    IFE(NlpLtrasWord(ctrl_nlp_th, i));
  }
  DONE;
}

static og_status NlpLtrasWord(og_nlp_th ctrl_nlp_th, int Irequest_word)
{
  int interpret_package_used = OgHeapGetCellsUsed(ctrl_nlp_th->hinterpret_package);
  struct interpret_package *interpret_packages = OgHeapGetCell(ctrl_nlp_th->hinterpret_package, 0);
  IFN(interpret_packages) DPcErr;
  for (int i = 0; i < interpret_package_used; i++)
  {
    struct interpret_package *interpret_package = interpret_packages + i;
    og_status status = NlpLtrasWordPackage(ctrl_nlp_th, Irequest_word, interpret_package);
    IFE(status);
  }
  DONE;
}

static og_status NlpLtrasWordPackage(og_nlp_th ctrl_nlp_th, int Irequest_word,
    struct interpret_package *interpret_package)
{
  package_t package = interpret_package->package;
  IFE(OgLtrasHaBaseSet(ctrl_nlp_th->hltras, package->ltra_dictionaries->ha_base));
  IFE(OgLtrasHaSwapSet(ctrl_nlp_th->hltras, package->ltra_dictionaries->ha_swap));
  IFE(OgLtrasHaPhonSet(ctrl_nlp_th->hltras, package->ltra_dictionaries->ha_phon));

  struct og_ltra_trfs *trfs;
  struct og_ltras_input input[1];
  memset(input, 0, sizeof(struct og_ltras_input));

  struct request_word *request_word = OgHeapGetCell(ctrl_nlp_th->hrequest_word, Irequest_word);
  IFN(request_word) DPcErr;
  og_string word = OgHeapGetCell(ctrl_nlp_th->hba, request_word->start);
  IFN(word) DPcErr;
  int word_length = request_word->length;
  int uni_length;
  unsigned char uni[DPcPathSize];
  IFE(OgCpToUni(word_length, word , DPcPathSize, &uni_length, uni, DOgCodePageUTF8, 0, 0));

  input->request_length = uni_length;
  input->request = uni;
  input->language_code = DOgLangNil;
  input->frequency_ratio = 1.0;
  input->score_factor = 1.0;
  input->flow_chart = "(del-add/phon/swap)-exc-term";
  input->max_nb_solutions = 3;
  IFE(OgLtras(ctrl_nlp_th->hltras, input, &trfs));

  if (ctrl_nlp_th->loginfo->trace & DOgNlpTraceLtras)
  {
    OgMsg(ctrl_nlp_th->hmsg, "ltras", DOgMsgDestInLog, "NlpLtrasWord: list of linguistic transformations:");
    IFE(OgLtrasTrfsLog(ctrl_nlp_th->hltras, trfs));
  }

  unsigned char words[DPcPathSize];
  int isword;
  unsigned char sword[DPcPathSize];

  for (int Itrf = 0; Itrf < trfs->TrfUsed; Itrf++)
  {
    struct og_ltra_trf *trf = trfs->Trf + Itrf;
    if (trf->basic) continue;
    if (!trf->total) continue;
    int i;
    for (i = 0, words[0] = 0; i < trf->nb_words; i++)
    {
      struct og_ltra_word *ltra_word = trfs->Word + trf->start_word + i;
      IFE(OgUniToCp(ltra_word->length,trfs->Ba+ltra_word->start,DPcPathSize,&isword,sword,DOgCodePageUTF8,0,0));
      sprintf(words + strlen(words), "%s%s", (i ? " " : ""), sword);
    }
    double score_spelling = pow(trf->final_score,4);
    og_status status = NlpLtrasAddWord(ctrl_nlp_th, Irequest_word, strlen(words), words, score_spelling);
    IFE(status);
  }

  IFE(OgLtrasTrfsDestroy(ctrl_nlp_th->hltras, trfs));
  DONE;
}

static og_status NlpLtrasAddWord(og_nlp_th ctrl_nlp_th, int Irequest_word_basic, int length_corrected_word,
    og_string corrected_word, double spelling_score)
{
  struct request_word *request_word_basic = OgHeapGetCell(ctrl_nlp_th->hrequest_word, Irequest_word_basic);
  IFN(request_word_basic) DPcErr;

  if (ctrl_nlp_th->loginfo->trace & DOgNlpTraceLtras)
  {
    og_string string_request_word = OgHeapGetCell(ctrl_nlp_th->hba, request_word_basic->start);
    IFN(string_request_word) DPcErr;
    NlpLog(DOgNlpTraceLtras, "NlpLtrasAddWord: adding corrected word '%s' for basic word '%s' at position %d:%d",
        corrected_word, string_request_word, request_word_basic->start_position, request_word_basic->length_position)
  }

  size_t Irequest_word;
  struct request_word *request_word = OgHeapNewCell(ctrl_nlp_th->hrequest_word, &Irequest_word);
  IFn(request_word) DPcErr;
  IF(Irequest_word) DPcErr;

  request_word->start = OgHeapGetCellsUsed(ctrl_nlp_th->hba);
  request_word->length = length_corrected_word;
  IFE(OgHeapAppend(ctrl_nlp_th->hba, request_word->length, corrected_word));
  IFE(OgHeapAppend(ctrl_nlp_th->hba, 1, ""));

  request_word->raw_start = OgHeapGetCellsUsed(ctrl_nlp_th->hba);
  request_word->raw_length = length_corrected_word;
  IFE(OgHeapAppend(ctrl_nlp_th->hba, request_word->raw_length, corrected_word));
  IFE(OgHeapAppend(ctrl_nlp_th->hba, 1, ""));

  // Necessary because of possible reallocation of hrequest_word
  request_word_basic = OgHeapGetCell(ctrl_nlp_th->hrequest_word, Irequest_word_basic);
  IFN(request_word_basic) DPcErr;
  request_word->start_position = request_word_basic->start_position;
  request_word->length_position = request_word_basic->length_position;

  request_word->is_digit = FALSE;

  request_word->spelling_score = spelling_score;

  DONE;
}
