/*
 *  Creating auto-complete request
 *  Copyright (c) 2017 Pertimm, by Patrick Constant
 *  Dev : November 2017
 *  Version 1.0
 */
#include "ogm_nlp.h"

static og_status NlpAutoCompletePackage(og_nlp_th ctrl_nlp_th, package_t package, int Ilast_request_word);
static og_status NlpAutoCompleteAddWord(og_nlp_th ctrl_nlp_th, int Ilast_request_word, int complete_word_length,
    og_string complete_word, double spelling_score);

og_status NlpAutoComplete(og_nlp_th ctrl_nlp_th)
{
  if (!ctrl_nlp_th->auto_complete) DONE;
  if (ctrl_nlp_th->basic_request_word_used <= 0) DONE;
  int Ilast_request_word = ctrl_nlp_th->basic_request_word_used - 1;
  int package_used = OgHeapGetCellsUsed(ctrl_nlp_th->hinterpret_package);
  struct interpret_package *interpret_packages = OgHeapGetCell(ctrl_nlp_th->hinterpret_package, 0);
  for (int i = 0; i < package_used; i++)
  {
    IFE(NlpAutoCompletePackage(ctrl_nlp_th, interpret_packages[i].package, Ilast_request_word));
  }
  DONE;
}

static og_status NlpAutoCompletePackage(og_nlp_th ctrl_nlp_th, package_t package, int Ilast_request_word)
{
  struct request_word *last_request_word = OgHeapGetCell(ctrl_nlp_th->hrequest_word, Ilast_request_word);
  IFN(last_request_word) DPcErr;

  unsigned char *last_word = OgHeapGetCell(ctrl_nlp_th->hba, last_request_word->start);
  IFN(last_word) DPcErr;
  int last_word_length = last_request_word->length;

  NlpLog(DOgNlpTraceInterpret, "NlpAutoCompletePackage: looking for complete word from '%s' in package '%s'", last_word,
      package->slug)

  unsigned char out[DPcAutMaxBufferSize + 9];
  oindex states[DPcAutMaxBufferSize + 9];
  int retour, nstate0, nstate1, iout;

  if ((retour = OgAufScanf(package->ha_word, last_word_length, last_word, &iout, out, &nstate0, &nstate1, states)))
  {
    do
    {
      IFE(retour);
      int sep = (-1);
      for (int i = 0; i < iout; i++)
      {
        if (out[i] == '\1')
        {
          sep = i;
          break;
        }
      }
      if (sep < 0)
      {
        NlpThrowErrorTh(ctrl_nlp_th, "NlpAutoCompletePackage: no sep in ha_word in package '%s'", package->slug);
        DPcErr;
      }
      if (sep > 0)
      {
        unsigned char complete_word[DPcPathSize];
        snprintf(complete_word, DPcPathSize, "%.*s%.*s", last_word_length, last_word, sep, out);
        int complete_word_length = strlen(complete_word);
        double spelling_score = last_word_length;
        spelling_score /= sep + last_word_length;
        NlpLog(DOgNlpTraceInterpret, "    found complete word '%s'", complete_word)
        og_status status = NlpAutoCompleteAddWord(ctrl_nlp_th, Ilast_request_word, complete_word_length, complete_word,
            spelling_score);
        IFE(status);
      }
    }
    while ((retour = OgAufScann(package->ha_word, &iout, out, nstate0, &nstate1, states)));
  }

  DONE;
}

static og_status NlpAutoCompleteAddWord(og_nlp_th ctrl_nlp_th, int Ilast_request_word, int complete_word_length,
    og_string complete_word, double spelling_score)
{
  struct request_word *last_request_word = OgHeapGetCell(ctrl_nlp_th->hrequest_word, Ilast_request_word);
  IFN(last_request_word) DPcErr;

  if (ctrl_nlp_th->loginfo->trace & DOgNlpTraceLtras)
  {
    og_string string_request_word = OgHeapGetCell(ctrl_nlp_th->hba, last_request_word->start);
    IFN(string_request_word) DPcErr;
    NlpLog(DOgNlpTraceLtras, "NlpLtrasAddWord: adding corrected word '%s' for basic word '%s' at position %d:%d",
        complete_word, string_request_word, last_request_word->start_position, last_request_word->length_position)
  }

  size_t Irequest_word;
  struct request_word *request_word = OgHeapNewCell(ctrl_nlp_th->hrequest_word, &Irequest_word);
  IFn(request_word) DPcErr;
  IF(Irequest_word) DPcErr;

  request_word->self_index = Irequest_word;
  request_word->start = OgHeapGetCellsUsed(ctrl_nlp_th->hba);
  request_word->length = complete_word_length;
  IFE(OgHeapAppend(ctrl_nlp_th->hba, request_word->length, complete_word));
  IFE(OgHeapAppend(ctrl_nlp_th->hba, 1, ""));

  request_word->raw_start = OgHeapGetCellsUsed(ctrl_nlp_th->hba);
  request_word->raw_length = complete_word_length;
  IFE(OgHeapAppend(ctrl_nlp_th->hba, request_word->raw_length, complete_word));
  IFE(OgHeapAppend(ctrl_nlp_th->hba, 1, ""));

  // Necessary because of possible reallocation of hrequest_word
  last_request_word = OgHeapGetCell(ctrl_nlp_th->hrequest_word, Ilast_request_word);
  IFN(last_request_word) DPcErr;
  request_word->start_position = last_request_word->start_position;
  request_word->length_position = last_request_word->length_position;

  request_word->is_number = FALSE;
  request_word->is_punctuation = FALSE;
  request_word->is_auto_complete_word = TRUE;
  request_word->is_regex = FALSE;
  request_word->regex_input_part = NULL;
  request_word->nb_matched_words = 1;


  request_word->spelling_score = spelling_score;

  DONE;
}

og_status NlpGetAutoCompleteRequestWord(og_nlp_th ctrl_nlp_th, struct request_expression *request_expression)
{
  if (!ctrl_nlp_th->auto_complete) DONE;

  for (int i = 0; i < request_expression->orips_nb; i++)
  {
    struct request_input_part *request_input_part = NlpGetRequestInputPart(ctrl_nlp_th, request_expression, i);
    IFN(request_input_part) DPcErr;

    if (request_input_part->type == nlp_input_part_type_Word)
    {
      struct request_word *request_word = request_input_part->request_word;
      if (request_word->is_auto_complete_word)
      {
        request_expression->auto_complete_request_word = request_word;
        DONE;
      }
    }
    else if (request_input_part->type == nlp_input_part_type_Interpretation)
    {
      struct request_expression *sub_request_expression = OgHeapGetCell(ctrl_nlp_th->hrequest_expression,
          request_input_part->Irequest_expression);
      IFN(sub_request_expression) DPcErr;
      IFX(sub_request_expression->auto_complete_request_word)
      {
        request_expression->auto_complete_request_word = sub_request_expression->auto_complete_request_word;
        DONE;
      }
      else
      {
        IFE(NlpGetAutoCompleteRequestWord(ctrl_nlp_th, sub_request_expression));
      }
    }
  }
  DONE;
}

og_bool NlpDifferentAutoCompleteRequestWord(og_nlp_th ctrl_nlp_th, struct request_expression *request_expression1,
    struct request_expression *request_expression2)
{
  if (request_expression1->auto_complete_request_word != NULL && request_expression2->auto_complete_request_word != NULL
      && request_expression1->auto_complete_request_word != request_expression2->auto_complete_request_word) return TRUE;
  return FALSE;
}

