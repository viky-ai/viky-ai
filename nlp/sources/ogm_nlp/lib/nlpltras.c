/*
 *  Handling spell checking for Natural Language Processing library
 *  Copyright (c) 2017 Pertimm, by Patrick Constant
 *  Dev : November 2017
 *  Version 1.0
 */
#include "ogm_nlp.h"

static og_status NlpLtrasWord(og_nlp_th ctrl_nlp_th, struct request_word *request_word);
static og_status NlpLtrasAddWord(og_nlp_th ctrl_nlp_th, struct request_word *request_word_basic,
    int length_corrected_word, og_string corrected_word);

/**
 * Do a spell checking on each words and also a spell checking on the whole sentence
 */

og_status NlpLtras(og_nlp_th ctrl_nlp_th)
{
  OgMsg(ctrl_nlp_th->hmsg, "", DOgMsgDestInLog, "list of request words:");
  int request_word_used = OgHeapGetCellsUsed(ctrl_nlp_th->hrequest_word);
  struct request_word *request_words = OgHeapGetCell(ctrl_nlp_th->hrequest_word, 0);
  for (int i = 0; i < request_word_used; i++)
  {
    IFE(NlpLtrasWord(ctrl_nlp_th, request_words + i));
  }
  DONE;
}

static og_status NlpLtrasWord(og_nlp_th ctrl_nlp_th, struct request_word *request_word)
{
  og_string string_request_word = OgHeapGetCell(ctrl_nlp_th->hba, request_word->start);
  IFN(string_request_word) DPcErr;

  // Fake corrections to test the ltras integration
  og_string string_request_word_corrected = NULL;
  if (!strcmp(string_request_word, "pol")) string_request_word_corrected = "pool";

  IFN(string_request_word_corrected) DONE;

  IFE(NlpLtrasAddWord(ctrl_nlp_th, request_word, strlen(string_request_word_corrected), string_request_word_corrected));
  DONE;
}

static og_status NlpLtrasAddWord(og_nlp_th ctrl_nlp_th, struct request_word *request_word_basic,
    int length_corrected_word, og_string corrected_word)
{

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

  request_word->start_position = request_word_basic->start_position;
  request_word->length_position = request_word_basic->length_position;

  request_word->is_digit = FALSE;

  DONE;
}
