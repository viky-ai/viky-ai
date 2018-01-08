/*
 *  Handling request input parts
 *  Copyright (c) 2017 Pertimm, by Patrick Constant
 *  Dev : October 2017
 *  Version 1.0
 */
#include "ogm_nlp.h"

static og_status NlpCalculateKeptRequestExpressions(og_nlp_th ctrl_nlp_th, GQueue *sorted_request_expressions);
static og_status NlpAnyValidate(og_nlp_th ctrl_nlp_th, GQueue *sorted_request_expressions);
static int NlpRequestExpressionCmp(gconstpointer ptr_request_expression1, gconstpointer ptr_request_expression2,
    gpointer user_data);
static og_status NlpIsSubRequestExpression(og_nlp_th ctrl_nlp_th, GQueue *sorted_request_expressions,
    struct request_expression *request_expression);
static og_status NlpIsSubExpression(og_nlp_th ctrl_nlp_th, struct request_expression *sub_request_expression,
    struct request_expression *request_expression);
static og_status NlpIsSubExpressionRecursive(og_nlp_th ctrl_nlp_th, struct expression *expression,
    struct request_expression *request_expression);

og_status NlpRequestExpressionsCalculate(og_nlp_th ctrl_nlp_th)
{
  int request_expression_used = OgHeapGetCellsUsed(ctrl_nlp_th->hrequest_expression);
  if (request_expression_used <= 0) DONE;

  struct request_expression *request_expressions = OgHeapGetCell(ctrl_nlp_th->hrequest_expression, 0);
  IFN(request_expressions) DPcErr;

  GQueue *sorted_request_expressions = ctrl_nlp_th->sorted_request_expressions;
  for (int i = 0; i < request_expression_used; i++)
  {
    struct request_expression *request_expression = request_expressions + i;

    if (request_expression->expression->interpretation->scope == nlp_interpretation_scope_type_hidden)
    {
      continue;
    }

    request_expression->any_validate_status = 1;
    g_queue_push_tail(sorted_request_expressions, request_expressions + i);
  }

  IFE(NlpCalculateKeptRequestExpressions(ctrl_nlp_th, sorted_request_expressions));

  if (ctrl_nlp_th->loginfo->trace & DOgNlpTraceMatch)
  {
    IFE(NlpSortedRequestExpressionsLog(ctrl_nlp_th, "List of sorted request expressions before any validation:"));
  }

  IFE(NlpAnyValidate(ctrl_nlp_th, sorted_request_expressions));

  for (GList *iter = sorted_request_expressions->head; iter; iter = iter->next)
  {
    struct request_expression *request_expression = iter->data;
    if (!request_expression->keep_as_result) continue;
    IFE(NlpSolutionCalculate(ctrl_nlp_th, request_expression));
    IFE(NlpCalculateScore(ctrl_nlp_th, request_expression));
  }

  // sort again to take into account scores
  g_queue_sort(sorted_request_expressions, (GCompareDataFunc) NlpRequestExpressionCmp, NULL);

  if (ctrl_nlp_th->loginfo->trace & DOgNlpTraceMatch)
  {
    NlpLog(DOgNlpTraceMatch, "First request expression found:")
    struct request_expression *last_request_expression = sorted_request_expressions->head->data;
    IFE(NlpRequestExpressionAnysLog(ctrl_nlp_th, last_request_expression));
    IFE(NlpInterpretTreeLog(ctrl_nlp_th, last_request_expression));

    IFE(NlpSortedRequestExpressionsLog(ctrl_nlp_th, "List of sorted request expressions:"));
  }

  DONE;
}

static og_status NlpCalculateKeptRequestExpressions(og_nlp_th ctrl_nlp_th, GQueue *sorted_request_expressions)
{
  g_queue_sort(sorted_request_expressions, (GCompareDataFunc) NlpRequestExpressionCmp, NULL);

  struct request_expression *first_request_expression = sorted_request_expressions->head->data;
  for (GList *iter = sorted_request_expressions->head; iter; iter = iter->next)
  {
    struct request_expression *request_expression = iter->data;
    if (request_expression->any_validate_status == 0) continue;
    if (request_expression->request_positions_nb == first_request_expression->request_positions_nb
        && request_expression->overlap_mark == first_request_expression->overlap_mark)
    {
      og_bool is_sub_re;
      IFE(is_sub_re = NlpIsSubRequestExpression(ctrl_nlp_th, sorted_request_expressions, request_expression));
      if (!is_sub_re) request_expression->keep_as_result = TRUE;
    }
  }
  DONE;
}

static og_status NlpAnyValidate(og_nlp_th ctrl_nlp_th, GQueue *sorted_request_expressions)
{
  og_bool some_expressions_kept = FALSE;
  while (!some_expressions_kept)
  {
    for (GList *iter = sorted_request_expressions->head; iter; iter = iter->next)
    {
      struct request_expression *request_expression = iter->data;
      if (!request_expression->keep_as_result) continue;
      IFE(NlpInterpretTreeAttachAny(ctrl_nlp_th, request_expression));

      int nb_anys_attached;
      IFE(nb_anys_attached = NlpRequestAnyOptimizeMatch(ctrl_nlp_th, request_expression,FALSE));

      if (request_expression->nb_anys != nb_anys_attached)
      {
        NlpLog(DOgNlpTraceMatch, "NlpAnyValidate: nb_anys=%d != nb_anys_attached=%d, this request is not validated:",
            request_expression->nb_anys, nb_anys_attached);
        request_expression->any_validate_status = 0;
        request_expression->keep_as_result = FALSE;
      }
      else
      {
        IFE(NlpRequestAnyOptimizeMatch(ctrl_nlp_th, request_expression,TRUE));
        request_expression->any_validate_status = 2;
        some_expressions_kept = TRUE;
      }
    }
    IFE(NlpCalculateKeptRequestExpressions(ctrl_nlp_th, sorted_request_expressions));
  }
  DONE;
}

static int NlpRequestExpressionCmp(gconstpointer ptr_request_expression1, gconstpointer ptr_request_expression2,
    gpointer user_data)
{
  struct request_expression *request_expression1 = (struct request_expression *) ptr_request_expression1;
  struct request_expression *request_expression2 = (struct request_expression *) ptr_request_expression2;

  if (request_expression1->any_validate_status != request_expression2->any_validate_status)
  {
    return (request_expression2->any_validate_status - request_expression1->any_validate_status);
  }
  if (request_expression1->keep_as_result != request_expression2->keep_as_result)
  {
    return (request_expression2->keep_as_result - request_expression1->keep_as_result);
  }
  if (request_expression1->request_positions_nb != request_expression2->request_positions_nb)
  {
    return (request_expression2->request_positions_nb - request_expression1->request_positions_nb);
  }
  if (request_expression1->overlap_mark != request_expression2->overlap_mark)
  {
    return (request_expression1->overlap_mark - request_expression2->overlap_mark);
  }
  if (request_expression1->nb_anys != request_expression2->nb_anys)
  {
    return (request_expression2->nb_anys - request_expression1->nb_anys);
  }
  if (request_expression1->total_score != request_expression2->total_score)
  {
    double cmp = request_expression2->total_score - request_expression1->total_score;
    if (cmp > 0) return 1;
    else return -1;
  }
  if (request_expression1->level != request_expression2->level)
  {
    return (request_expression2->level - request_expression1->level);
  }
// Just to make sure it is different
  return request_expression1 - request_expression2;
}

static og_status NlpIsSubRequestExpression(og_nlp_th ctrl_nlp_th, GQueue *sorted_request_expressions,
    struct request_expression *sub_request_expression)
{
  for (GList *iter = sorted_request_expressions->head; iter; iter = iter->next)
  {
    struct request_expression *request_expression = iter->data;
    if (!request_expression->keep_as_result) continue;

    og_bool is_sub_expression = FALSE;
    IFE(is_sub_expression = NlpIsSubExpression(ctrl_nlp_th, sub_request_expression, request_expression));
    if (is_sub_expression)
    {
      if (NlpDifferentAutoCompleteRequestWord(ctrl_nlp_th, sub_request_expression, request_expression)) return FALSE;
      return TRUE;
    }
  }
  return FALSE;
}

static og_status NlpIsSubExpression(og_nlp_th ctrl_nlp_th, struct request_expression *sub_request_expression,
    struct request_expression *request_expression)
{
  return NlpIsSubExpressionRecursive(ctrl_nlp_th, sub_request_expression->expression, request_expression);
}

static og_status NlpIsSubExpressionRecursive(og_nlp_th ctrl_nlp_th, struct expression *expression,
    struct request_expression *request_expression)
{
  if (request_expression->expression == expression) return TRUE;
  for (int i = 0; i < request_expression->orips_nb; i++)
  {
    struct request_input_part *request_input_part = NlpGetRequestInputPart(ctrl_nlp_th, request_expression, i);
    IFN(request_input_part) DPcErr;

    if (request_input_part->type == nlp_input_part_type_Word) ;
    else if (request_input_part->type == nlp_input_part_type_Interpretation)
    {
      struct request_expression *sub_request_expression = OgHeapGetCell(ctrl_nlp_th->hrequest_expression,
          request_input_part->Irequest_expression);
      IFN(sub_request_expression) DPcErr;
      og_bool is_sub_expression = FALSE;
      if (sub_request_expression->expression == expression) return TRUE;
      is_sub_expression = NlpIsSubExpressionRecursive(ctrl_nlp_th, expression, sub_request_expression);
      IFE(is_sub_expression);
      if (is_sub_expression) return TRUE;
    }
  }

  return FALSE;
}

