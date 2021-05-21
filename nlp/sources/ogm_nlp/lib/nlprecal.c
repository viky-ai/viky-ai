/*
 *  Handling request input parts
 *  Copyright (c) 2017 Pertimm, by Patrick Constant
 *  Dev : October 2017
 *  Version 1.0
 */
#include "ogm_nlp.h"

static og_status NlpCalculateKeptRequestExpressions(og_nlp_th ctrl_nlp_th, GQueue *sorted_request_expressions);
static og_status NlpListsSort(og_nlp_th ctrl_nlp_th, GQueue *sorted_request_expressions);
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
    struct interpretation *interpretation = request_expression->expression->interpretation;
    og_bool is_primary_package = NlpIsPrimaryPackage(ctrl_nlp_th, interpretation->package);
    IFE(is_primary_package);

    if (interpretation->scope == nlp_interpretation_scope_type_hidden)
    {
      continue;
    }
    else if (interpretation->scope == nlp_interpretation_scope_type_private)
    {
      if (ctrl_nlp_th->show_private && is_primary_package)
      {
        // keep that interpretation
      }
      else
      {
        continue;
      }
    }
    else if (interpretation->scope == nlp_interpretation_scope_type_public)
    {
      if (!is_primary_package)
      {
        continue;
      }
    }

    request_expression->any_validate_status = 1;
    g_queue_push_tail(sorted_request_expressions, request_expressions + i);
  }

  // Score calculation for all expression, it must not take too much time
  for (GList *iter = sorted_request_expressions->head; iter; iter = iter->next)
  {
    struct request_expression *request_expression = iter->data;
    IFE(NlpCalculateScore(ctrl_nlp_th, request_expression));
  }

  IFE(NlpCalculateKeptRequestExpressions(ctrl_nlp_th, sorted_request_expressions));

  if (ctrl_nlp_th->loginfo->trace & DOgNlpTraceMatch)
  {
    IFE(NlpSortedRequestExpressionsLog(ctrl_nlp_th, "NlpRequestExpressionsCalculate: list of sorted request expressions before enable list:"));
  }

  IFE(NlpEnableList(ctrl_nlp_th, sorted_request_expressions));
  IFE(NlpListsSort(ctrl_nlp_th, sorted_request_expressions));

  if (ctrl_nlp_th->loginfo->trace & DOgNlpTraceMatch)
  {
    IFE(NlpSortedRequestExpressionsLog(ctrl_nlp_th, "NlpRequestExpressionsCalculate: list of sorted request expressions after enable list:"));
  }

  for (GList *iter = sorted_request_expressions->head; iter; iter = iter->next)
  {
    struct request_expression *request_expression = iter->data;
    if (!request_expression->keep_as_result) continue;
    IFE(NlpSolutionCalculate(ctrl_nlp_th, request_expression));
    //IFE(NlpCalculateScore(ctrl_nlp_th, request_expression));
  }

  IFE(NlpEnableListCheckOverlapAfterCalculation(ctrl_nlp_th, sorted_request_expressions));

  // sort again to take into account scores
  g_queue_sort(sorted_request_expressions, NlpRequestExpressionCmp, NULL);

  if (ctrl_nlp_th->loginfo->trace & DOgNlpTraceMatch)
  {
    if (sorted_request_expressions->length > 0)
    {
      NlpLog(DOgNlpTraceMatch, "First request expression found:")
      struct request_expression *last_request_expression = sorted_request_expressions->head->data;
      IFE(NlpRequestExpressionAnysLog(ctrl_nlp_th, last_request_expression));
      IFE(NlpInterpretTreeLog(ctrl_nlp_th, last_request_expression, 0));
      IFE(NlpSortedRequestExpressionsLog(ctrl_nlp_th, "List of sorted request expressions:"));
    }
  }

  DONE;
}

static og_status NlpCalculateKeptRequestExpressions(og_nlp_th ctrl_nlp_th, GQueue *sorted_request_expressions)
{
  if (sorted_request_expressions->length == 0) CONT;

  for (GList *iter = sorted_request_expressions->head; iter; iter = iter->next)
  {
    struct request_expression *request_expression = iter->data;
    IFE(NlpSolutionCalculatePositions(ctrl_nlp_th, request_expression));
    request_expression->keep_as_result = FALSE;
  }

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


static og_status NlpListsSort(og_nlp_th ctrl_nlp_th, GQueue *sorted_request_expressions)
{
  for (GList *iter = sorted_request_expressions->head; iter; iter = iter->next)
  {
    struct request_expression *request_expression = iter->data;
    if (!request_expression->keep_as_result) continue;

    IFE(NlpRequestExpressionListsSort(ctrl_nlp_th, request_expression));
  }
  DONE;
}

og_bool NlpAnyValidateExpression(og_nlp_th ctrl_nlp_th, struct request_expression *request_expression)
{
  if (request_expression->nb_anys <= 0)
  {
    NlpLog(DOgNlpTraceMatch, "NlpAnyValidateExpression: this request with no any is validated");
    return TRUE;
  }

  if (request_expression->any_validate_status == 0) return FALSE;
  else if (request_expression->any_validate_status == 2) return TRUE;

  if (ctrl_nlp_th->loginfo->trace & DOgNlpTraceMatch)
  {
    NlpLog(DOgNlpTraceMatch, "NlpAnyValidateExpression: trying to attach anys to request expression:")
    IFE(NlpRequestExpressionAnysLog(ctrl_nlp_th, request_expression));
    IFE(NlpInterpretTreeLog(ctrl_nlp_th, request_expression, 0));
  }

  IFE(NlpInterpretTreeAttachAny(ctrl_nlp_th, request_expression));

  int nb_anys_attached = 0;
  IFE(nb_anys_attached = NlpRequestAnyOptimizeMatch(ctrl_nlp_th, request_expression,FALSE));
  if (request_expression->nb_anys != nb_anys_attached)
  {
    NlpLog(DOgNlpTraceMatch,
        "NlpAnyValidateExpression: nb_anys=%d != nb_anys_attached=%d, this request is not validated",
        request_expression->nb_anys, nb_anys_attached);
    request_expression->any_validate_status = 0;
    return FALSE;
  }

  NlpLog(DOgNlpTraceMatch, "NlpAnyValidateExpression: nb_anys=%d == nb_anys_attached=%d, this request is validated",
      request_expression->nb_anys, nb_anys_attached);
  IFE(NlpRequestAnyOptimizeMatch(ctrl_nlp_th, request_expression,TRUE));
  request_expression->any_validate_status = 2;
  return TRUE;

}



static int NlpRequestExpressionCmp(gconstpointer ptr_request_expression1, gconstpointer ptr_request_expression2,
    gpointer user_data)
{
  struct request_expression *request_expression1 = (struct request_expression *) ptr_request_expression1;
  struct request_expression *request_expression2 = (struct request_expression *) ptr_request_expression2;

  struct interpretation *interpretation1 = request_expression1->expression->interpretation;
  struct interpretation *interpretation2 = request_expression2->expression->interpretation;

  // We want to put at the end any expressions that not validated
  // but when they are validated, they can have value of 1 or 2
  int avs1 = request_expression1->any_validate_status;
  int avs2 = request_expression2->any_validate_status;
  if (avs1) avs1= 1;
  if (avs2) avs2= 1;
  if (avs1 != avs2)
  {
    return (avs2 - avs1);
  }
  if (interpretation1->scope != interpretation2->scope)
  {
    return (interpretation1->scope - interpretation2->scope);
  }
  if (request_expression1->score->scope != request_expression2->score->scope)
  {
    double cmp = request_expression2->score->scope - request_expression1->score->scope;
    if (cmp > 0) return 1;
    else return -1;
  }
  if (request_expression1->keep_as_result != request_expression2->keep_as_result)
  {
    return (request_expression2->keep_as_result - request_expression1->keep_as_result);
  }
  int length1 = request_expression1->end_position_char - request_expression1->start_position_char;
  int length2 = request_expression2->end_position_char - request_expression2->start_position_char;
  if (length1 != length2)
  {
    return (length2 - length1);
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

