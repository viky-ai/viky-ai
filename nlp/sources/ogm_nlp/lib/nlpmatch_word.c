/*
 *  Handling log messages for Natural Language Processing library
 *  Copyright (c) 2017 Pertimm, by Patrick Constant
 *  Dev : September 2017
 *  Version 1.0
 */
#include "ogm_nlp.h"

static og_status NlpMatchWord(og_nlp_th ctrl_nlp_th, struct request_word *request_word);
static og_status NlpMatchWordInPackage(og_nlp_th ctrl_nlp_th, struct request_word *request_word, int input_length,
    unsigned char *input, struct interpret_package *interpret_package);

og_status NlpMatchWords(og_nlp_th ctrl_nlp_th)
{
  struct request_word *first_request_word = OgHeapGetCell(ctrl_nlp_th->hrequest_word, 0);
  IFN(first_request_word) DPcErr;

  for (struct request_word *rw = first_request_word; rw; rw = rw->next)
  {
    IFE(NlpMatchWord(ctrl_nlp_th, rw));
  }

  DONE;
}

static og_status NlpMatchWord(og_nlp_th ctrl_nlp_th, struct request_word *request_word)
{

  og_string string_request_word = OgHeapGetCell(ctrl_nlp_th->hba, request_word->start);
  IFN(string_request_word) DPcErr;

  unsigned char input[DPcAutMaxBufferSize + 9];
  int input_length = request_word->length;
  memcpy(input, string_request_word, input_length);
  input[input_length++] = '\1';
  input[input_length] = 0;

  og_char_buffer number[DPcPathSize];
  number[0] = 0;
  if (request_word->is_number)
  {
    snprintf(number, DPcPathSize, " -> " DOgPrintDouble, request_word->number_value);
  }

  if (ctrl_nlp_th->loginfo->trace & DOgNlpTraceMatch)
  {
    unsigned char regex_string[DPcPathSize];
    regex_string[0] = 0;
    if (request_word->is_regex)
    {
      snprintf(regex_string, DPcPathSize, " (regex='%.*s')", DOgNlpMaximumRegexStringSizeLogged,
          request_word->regex_input_part->alias->regex_string);

    }
    NlpLog(DOgNlpTraceMatch, "Looking for input parts for string '%s'%s%s:", string_request_word, number, regex_string);
  }

  int interpret_package_used = OgHeapGetCellsUsed(ctrl_nlp_th->hinterpret_package);
  for (int i = 0; i < interpret_package_used; i++)
  {
    struct interpret_package *interpret_package = OgHeapGetCell(ctrl_nlp_th->hinterpret_package, i);
    IFN(interpret_package) DPcErr;
    IFE(NlpMatchWordInPackage(ctrl_nlp_th, request_word, input_length, input, interpret_package));
  }

  DONE;
}

static og_status NlpMatchWordInPackage(og_nlp_th ctrl_nlp_th, struct request_word *request_word, int input_length,
    unsigned char *input, struct interpret_package *interpret_package)
{
  package_t package = interpret_package->package;

  if (request_word->is_regex)
  {
    if (package == request_word->regex_input_part->expression->interpretation->package)
    {
      og_status status = NlpRequestInputPartAddWord(ctrl_nlp_th, request_word, interpret_package,
          request_word->regex_input_part->self_index, FALSE, 1.0);
      IFE(status);
    }
    DONE;
  }

  if (request_word->is_number)
  {
    struct number_input_part *number_input_part_all = OgHeapGetCell(package->hnumber_input_part, 0);
    int number_input_part_used = OgHeapGetCellsUsed(package->hnumber_input_part);
    for (int i = 0; i < number_input_part_used; i++)
    {
      struct number_input_part *number_input_part = number_input_part_all + i;

      // There is not need to have a special input part here for number words
      og_status status = NlpRequestInputPartAddWord(ctrl_nlp_th, request_word, interpret_package,
          number_input_part->Iinput_part, TRUE, 1.0);
      IFE(status);
    }
  }

  unsigned char out[DPcAutMaxBufferSize + 9];
  oindex states[DPcAutMaxBufferSize + 9];
  int retour, nstate0, nstate1, iout;

  if ((retour = OgAufScanf(package->ha_word, input_length, input, &iout, out, &nstate0, &nstate1, states)))
  {
    do
    {
      IFE(retour);
      int Iinput_part;
      unsigned char *p = out;
      IFE(DOgPnin4(ctrl_nlp_th->herr, &p, &Iinput_part));
      struct input_part *input_part = OgHeapGetCell(package->hinput_part, Iinput_part);
      IFN(input_part) DPcErr;
      og_bool matched_case_accent = NlpMatchCaseAccent(ctrl_nlp_th, request_word, input_part);
      IFE(matched_case_accent);
      if (!matched_case_accent) continue;
      NlpLog(DOgNlpTraceMatch, "    found input part %d in request package %d", Iinput_part,
          interpret_package->self_index)
      if (request_word->lang_id != DOgLangNil)
      {
        // Lemmatisation is accepted only on expressions with same locale
        if (input_part->expression->locale != request_word->lang_id) continue;
      }

      og_status status = NlpRequestInputPartAddWord(ctrl_nlp_th, request_word, interpret_package, Iinput_part, FALSE,
          1.0);
      IFE(status);
    }
    while ((retour = OgAufScann(package->ha_word, &iout, out, nstate0, &nstate1, states)));
  }

  DONE;
}

og_status NlpMatchWordChainRequestWords(og_nlp_th ctrl_nlp_th)
{

  // /!\ WARN /!\ you cannot add other request_word after chaining

  int request_word_used = OgHeapGetCellsUsed(ctrl_nlp_th->hrequest_word);
  struct request_word *all_words = OgHeapGetCell(ctrl_nlp_th->hrequest_word, 0);
  IFN(all_words) DPcErr;

  struct request_word *previous_word = NULL;

  for (int i = 0; i < request_word_used; i++)
  {
    struct request_word *current_word = all_words + i;
    current_word->next = NULL;

    if (previous_word)
    {
      previous_word->next = current_word;
    }

    previous_word = current_word;
  }

  DONE;
}

og_status NlpMatchWordChainUpdateWordCount(og_nlp_th ctrl_nlp_th)
{
  // replace basic_request_word_used
  ctrl_nlp_th->basic_group_request_word_nb = 0;

  struct request_word *first_request_word = OgHeapGetCell(ctrl_nlp_th->hrequest_word, 0);
  IFN(first_request_word) DPcErr;

  for (struct request_word *rw = first_request_word; rw; rw = rw->next)
  {
    // ignore non basic word (build from ltras)
    if (rw->self_index >= ctrl_nlp_th->basic_request_word_used) break;

    ctrl_nlp_th->basic_group_request_word_nb++;
  }

  DONE;
}

og_bool NlpMatchCaseAccent(og_nlp_th ctrl_nlp_th, struct request_word *request_word, struct input_part *input_part)
{
  struct expression *expression = input_part->expression;
  if (!expression->case_sensitive && !expression->accent_sensitive) return TRUE;

  og_string request_raw_word = OgHeapGetCell(ctrl_nlp_th->hba, request_word->raw_start);
  IFN(request_raw_word) DPcErr;

  package_t package = input_part->expression->interpretation->package;
  og_string raw_word = OgHeapGetCell(package->hinput_part_ba, input_part->word->raw_word_start);
  IFN(raw_word) DPcErr;

  int request_raw_word_length = strlen(request_raw_word);
  int raw_word_length = strlen(raw_word);

  if (expression->case_sensitive || expression->accent_sensitive)
  {
    if (expression->case_sensitive && expression->accent_sensitive)
    {
      if (request_raw_word_length != raw_word_length) return FALSE;
      if (strcmp(request_raw_word, raw_word)) return FALSE;
    }
    else if (expression->case_sensitive)
    {
      int iw1, iw2;
      unsigned char w1[DPcPathSize], w2[DPcPathSize];
      IFE(OgCpToUni(request_raw_word_length, request_raw_word, DPcPathSize, &iw1, w1, DOgCodePageUTF8, 0, 0));
      IFE(OgCpToUni(raw_word_length, raw_word, DPcPathSize, &iw2, w2, DOgCodePageUTF8, 0, 0));
      if (iw1 != iw2) return FALSE;
      OgUniStruna(iw1, w1, w1);
      OgUniStruna(iw2, w2, w2);
      if (memcmp(w1,w2,iw1)) return FALSE;
    }
    else   // expression->accent_sensitive == TRUE
    {
      int iw1, iw2;
      unsigned char w1[DPcPathSize], w2[DPcPathSize];
      IFE(OgCpToUni(request_raw_word_length, request_raw_word, DPcPathSize, &iw1, w1, DOgCodePageUTF8, 0, 0));
      IFE(OgCpToUni(raw_word_length, raw_word, DPcPathSize, &iw2, w2, DOgCodePageUTF8, 0, 0));
      if (iw1 != iw2) return FALSE;
      OgUniStrlwr(iw1, w1, w1);
      OgUniStrlwr(iw2, w2, w2);
      if (memcmp(w1,w2,iw1)) return FALSE;
    }

  }

  return TRUE;
}

