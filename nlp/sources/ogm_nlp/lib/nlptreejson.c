/*
 *  Handling parsing tree from a request expression
 *  Copyright (c) 2017 Pertimm, by Patrick Constant
 *  Dev : October 2017
 *  Version 1.0
 */
#include "ogm_nlp.h"

static og_status NlpInterpretTreeJsonRecursive(og_nlp_th ctrl_nlp_th,
    struct request_expression *root_request_expression, struct request_expression *request_expression,
    json_t *json_expressions);

/*
 * Adds the request expression in the json_interpretation
 */

og_status NlpInterpretTreeJson(og_nlp_th ctrl_nlp_th, struct request_expression *request_expression,
    json_t *json_interpretation)
{
  IFE(NlpExplainHighlightReset(ctrl_nlp_th));
  IFE(NlpSetSuperExpression(ctrl_nlp_th, request_expression));

  json_t *json_explanation = json_object();
  IF(json_object_set_new(json_interpretation, "explanation", json_explanation))
  {
    NlpThrowErrorTh(ctrl_nlp_th, "NlpInterpretTreeJson: error setting json_explanation");
    DPcErr;
  }

  json_t *json_expression = json_object();
  IF(json_object_set_new(json_expression, "pos", json_integer(request_expression->expression->pos)))
  {
    NlpThrowErrorTh(ctrl_nlp_th, "NlpInterpretTreeJson: error setting json_expression_pos");
    DPcErr;
  }

  json_t *json_expression_text = json_string(request_expression->expression->text);
  IF(json_object_set_new(json_expression, "text", json_expression_text))
  {
    NlpThrowErrorTh(ctrl_nlp_th, "NlpInterpretTreeJson: error setting json_expression_text");
    DPcErr;
  }
  struct interpretation *interpretation = request_expression->expression->interpretation;
  json_t *json_interpretation_slug = json_string(interpretation->slug);
  IF(json_object_set_new(json_expression, "slug", json_interpretation_slug))
  {
    NlpThrowErrorTh(ctrl_nlp_th, "NlpInterpretTreeJson: error setting json_interpretation_slug");
    DPcErr;
  }
  char highlight[DPcPathSize];
  NlpRequestPositionStringHighlight(ctrl_nlp_th, request_expression->request_position_start,
      request_expression->request_positions_nb, DPcPathSize, highlight);
  json_t *json_expression_highlight = json_string(highlight);
  IF(json_object_set_new(json_expression, "highlight", json_expression_highlight))
  {
    NlpThrowErrorTh(ctrl_nlp_th, "NlpInterpretTreeJson: error setting json_expression_highlight");
    DPcErr;
  }

  og_char_buffer scores[DPcPathSize];
  struct request_score *score = request_expression->score;
  sprintf(scores, "cov:%.2f loc:%.2f spell:%.2f olap:%.2f any:%.2f ctx:%0.2f scope:%.2f", score->coverage,
      score->locale, score->spelling, score->overlap, score->any, score->context, score->scope);

  json_t *json_expression_scores = json_string(scores);
  IF(json_object_set_new(json_expression, "scores", json_expression_scores))
  {
    NlpThrowErrorTh(ctrl_nlp_th, "NlpInterpretTreeJson: error setting json_expression_scores");
    DPcErr;
  }

  IFE(NlpInterpretTreeJsonRecursive(ctrl_nlp_th, request_expression, request_expression, json_expression));

  IFE(NlpExplainHighlight(ctrl_nlp_th,request_expression,json_explanation));

  IF(json_object_set_new(json_explanation, "expression", json_expression))
  {
    NlpThrowErrorTh(ctrl_nlp_th, "NlpInterpretTreeJson: error setting json_expression");
    DPcErr;
  }

  DONE;
}

static og_status NlpInterpretTreeJsonRecursiveSub(og_nlp_th ctrl_nlp_th, json_t *json_expressions,
    struct request_expression *root_request_expression, struct request_expression *request_expression,
    json_t *json_expression, struct request_expression *sub_request_expression, og_bool in_flat_list)
{
  json_t *json_sub_expression = json_object();

  IF(json_array_append_new(json_expressions, json_sub_expression))
  {
    NlpThrowErrorTh(ctrl_nlp_th, "NlpInterpretTreeJsonRecursive : error appending json_sub_expression");
    DPcErr;
  }

  json_t *json_sub_expression_text = json_string(sub_request_expression->expression->text);
  IF(json_object_set_new(json_sub_expression, "text", json_sub_expression_text))
  {
    NlpThrowErrorTh(ctrl_nlp_th, "NlpInterpretTreeJsonRecursive: error setting json_sub_expression_text");
    DPcErr;
  }
  struct interpretation *interpretation = sub_request_expression->expression->interpretation;
  json_t *json_interpretation_slug = json_string(interpretation->slug);
  IF(json_object_set_new(json_sub_expression, "slug", json_interpretation_slug))
  {
    NlpThrowErrorTh(ctrl_nlp_th, "NlpInterpretTreeJson: error setting json_interpretation_slug");
    DPcErr;
  }

  char highlight[DPcPathSize];
  NlpRequestPositionStringHighlight(ctrl_nlp_th, sub_request_expression->request_position_start,
      sub_request_expression->request_positions_nb, DPcPathSize, highlight);
  json_t *json_expression_highlight = json_string(highlight);
  IF(json_object_set_new(json_sub_expression, "highlight", json_expression_highlight))
  {
    NlpThrowErrorTh(ctrl_nlp_th, "NlpInterpretTreeJsonRecursive: error setting json_expression_highlight");
    DPcErr;
  }

  og_char_buffer scores[DPcPathSize];
  struct request_score *score = sub_request_expression->score;
  sprintf(scores, "cov:%.2f loc:%.2f spell:%.2f olap:%.2f any:%.2f ctx:%0.2f scope:%.2f", score->coverage,
      score->locale, score->spelling, score->overlap, score->any, score->context, score->scope);
  IF(json_object_set_new(json_sub_expression, "scores", json_string(scores)))
  {
    NlpThrowErrorTh(ctrl_nlp_th, "NlpInterpretTreeJsonRecursive: error setting json_expression_scores");
    DPcErr;
  }

  og_status status = NlpInterpretTreeJsonRecursive(ctrl_nlp_th, root_request_expression, sub_request_expression,
      json_sub_expression);
  IFE(status);

  DONE;
}

static og_status NlpInterpretTreeJsonRecursive(og_nlp_th ctrl_nlp_th,
    struct request_expression *root_request_expression, struct request_expression *request_expression,
    json_t *json_expression)
{
  json_t *json_expressions = json_array();

  if (request_expression->sorted_flat_list->length > 0)
  {

    IF(json_object_set_new(json_expression, "is_list", json_true()))
    {
      NlpThrowErrorTh(ctrl_nlp_th, "NlpInterpretTreeJsonRecursive: error setting is_list on json_expression");
      DPcErr;
    }

    for (GList *iter = request_expression->sorted_flat_list->head; iter; iter = iter->next)
    {
      int Irequest_expression = GPOINTER_TO_INT(iter->data);
      struct request_expression *sub_request_expression = OgHeapGetCell(ctrl_nlp_th->hrequest_expression,
          Irequest_expression);

      og_status status = NlpInterpretTreeJsonRecursiveSub(ctrl_nlp_th, json_expressions, root_request_expression,
          request_expression, json_expression, sub_request_expression, TRUE);
      IFE(status);

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

        json_t *json_sub_expression = json_object();
        IF(json_array_append_new(json_expressions, json_sub_expression))
        {
          NlpThrowErrorTh(ctrl_nlp_th, "NlpInterpretTreeJsonRecursive : error appending json_sub_expression");
          DPcErr;
        }

        json_t *json_sub_expression_text = json_string(string_request_word);
        IF(json_object_set_new(json_sub_expression, "word", json_sub_expression_text))
        {
          NlpThrowErrorTh(ctrl_nlp_th, "NlpInterpretTreeJsonRecursive: error setting json_sub_expression_text");
          DPcErr;
        }

        IFE(NlpExplainHighlightAddWord(ctrl_nlp_th, request_expression, request_word, NULL));

      }
      else if (request_input_part->type == nlp_input_part_type_Interpretation)
      {

        struct request_expression *sub_request_expression = OgHeapGetCell(ctrl_nlp_th->hrequest_expression,
            request_input_part->Irequest_expression);
        IFN(sub_request_expression) DPcErr;

        og_status status = NlpInterpretTreeJsonRecursiveSub(ctrl_nlp_th, json_expressions, root_request_expression,
            request_expression, json_expression, sub_request_expression, FALSE);
        IFE(status);
      }

    }

  }

  if (request_expression->Irequest_any >= 0)
  {
    struct request_any *request_any = OgHeapGetCell(ctrl_nlp_th->hrequest_any, request_expression->Irequest_any);
    IFN(request_any) DPcErr;

    char string_any[DPcPathSize];
    NlpRequestAnyString(ctrl_nlp_th, request_any, DPcPathSize, string_any);

    json_t *json_sub_expression = json_object();
    IF(json_array_append_new(json_expressions, json_sub_expression))
    {
      NlpThrowErrorTh(ctrl_nlp_th, "NlpInterpretTreeJsonRecursive : error appending json_sub_expression");
      DPcErr;
    }

    IF(json_object_set_new(json_sub_expression, "any", json_string(string_any)))
    {
      NlpThrowErrorTh(ctrl_nlp_th, "NlpInterpretTreeJsonRecursive: error setting json_sub_expression_any");
      DPcErr;
    }

    char highlight[DPcPathSize];
    NlpRequestAnyStringPretty(ctrl_nlp_th, request_any, DPcPathSize, highlight);
    IF(json_object_set_new(json_sub_expression, "highlight", json_string(highlight)))
    {
      NlpThrowErrorTh(ctrl_nlp_th, "NlpInterpretTreeJson: error setting json_expression_highlight");
      DPcErr;
    }
    IFE(NlpExplainHighlightAddWord(ctrl_nlp_th, request_expression, NULL, request_any));
  }

  IF(json_object_set_new(json_expression, "expressions", json_expressions))
  {
    NlpThrowErrorTh(ctrl_nlp_th, "NlpInterpretTreeJsonRecursive: error setting json_expression");
    DPcErr;
  }

  // solution explain
  json_t *json_package_solution = request_expression->expression->json_solution;
  if (json_package_solution == NULL)
  {
    json_package_solution = request_expression->expression->interpretation->json_solution;
  }

  if (json_package_solution)
  {
    IF(json_object_set_new(json_expression, "package_solution", json_deep_copy(json_package_solution)))
    {
      NlpThrowErrorTh(ctrl_nlp_th, "NlpInterpretTreeJsonRecursive: error setting package_solution on json_expression");
      DPcErr;
    }
  }

  if (request_expression->json_solution)
  {
    IF(json_object_set(json_expression, "computed_solution", request_expression->json_solution))
    {
      NlpThrowErrorTh(ctrl_nlp_th, "NlpInterpretTreeJsonRecursive: error setting computed_solution on json_expression");
      DPcErr;
    }
  }

  DONE;
}

