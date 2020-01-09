/*
 *  Handling the language of a recognized expression
 *  Copyright (c) 2019 Pertimm, by Patrick Constant
 *  Dev : November 2019
 *  Version 1.0
 */
#include "ogm_nlp.h"

struct og_language_span
{
  int language;
  int span;
};

static og_status NlpSolutionCalculateLanguageRecursive(og_nlp_th ctrl_nlp_th,
    struct request_expression *request_expression);

og_status NlpSolutionCalculateLanguage(og_nlp_th ctrl_nlp_th, struct request_expression *request_expression)
{
  IFE(NlpSolutionCalculateLanguageRecursive(ctrl_nlp_th, request_expression));
  DONE;
}

static og_status NlpSolutionCalculateLanguageRecursive(og_nlp_th ctrl_nlp_th,
    struct request_expression *request_expression)
{

  IFE(NlpSolutionCalculatePositions(ctrl_nlp_th, request_expression));
  if (request_expression->expression->locale != DOgLangNil)
  {
    request_expression->language->id = request_expression->expression->locale;
    request_expression->language->score = 1.0;
    DONE;
  }

  int span, total_span = 0;
  int language_span[DOgLangMax];
  memset(language_span, 0, sizeof(int) * DOgLangMax);

  if (request_expression->sorted_flat_list->length > 0)
  {
    for (GList *iter = request_expression->sorted_flat_list->head; iter; iter = iter->next)
    {
      int Irequest_expression = GPOINTER_TO_INT(iter->data);
      struct request_expression *sub_request_expression = OgHeapGetCell(ctrl_nlp_th->hrequest_expression,
          Irequest_expression);
      IFE(NlpSolutionCalculateLanguageRecursive(ctrl_nlp_th, sub_request_expression));
      span = sub_request_expression->end_position_char - sub_request_expression->start_position_char;
      if (0 <= sub_request_expression->language->id && sub_request_expression->language->id < DOgLangMax)
      {
        language_span[sub_request_expression->language->id] += span;
      }
      total_span += span;
    }
  }
  else
  {
    for (int i = 0; i < request_expression->orips_nb; i++)
    {
      struct request_input_part *request_input_part = NlpGetRequestInputPart(ctrl_nlp_th, request_expression, i);
      IFN(request_input_part) DPcErr;

      if (request_input_part->type == nlp_input_part_type_Word)
      {
        struct request_word *request_word = request_input_part->request_word;
        og_string string_request_word = OgHeapGetCell(ctrl_nlp_th->hba, request_word->start);
        IFN(string_request_word) DPcErr;
        int start_position_char = g_utf8_pointer_to_offset(ctrl_nlp_th->request_sentence,
            ctrl_nlp_th->request_sentence + request_word->start_position);
        int end_position_char = g_utf8_pointer_to_offset(ctrl_nlp_th->request_sentence,
            ctrl_nlp_th->request_sentence + request_word->start_position + request_word->length_position);
        span = end_position_char - start_position_char;
        if (0 <= request_word->lang_id && request_word->lang_id < DOgLangMax)
        {
          language_span[request_word->lang_id] += span;
        }
        total_span += span;
      }
      else if (request_input_part->type == nlp_input_part_type_Interpretation)
      {
        struct request_expression *sub_request_expression = OgHeapGetCell(ctrl_nlp_th->hrequest_expression,
            request_input_part->Irequest_expression);
        IFN(sub_request_expression) DPcErr;
        IFE(NlpSolutionCalculateLanguageRecursive(ctrl_nlp_th, sub_request_expression));
        span = sub_request_expression->end_position_char - sub_request_expression->start_position_char;
        if (0 <= sub_request_expression->language->id && sub_request_expression->language->id < DOgLangMax)
        {
          language_span[sub_request_expression->language->id] += span;
        }
        total_span += span;
      }
    }
  }

  int max_i = (-1);
  int max_span = 0;
  for (int i = 0; i < DOgLangMax; i++)
  {
    if (max_span < language_span[i])
    {
      max_span = language_span[i];
      max_i = i;
    }
  }

  if (max_i >= 0)
  {
    request_expression->language->id = max_i;
    request_expression->language->score = max_span;
    request_expression->language->score /= total_span;
  }

  DONE;
}
