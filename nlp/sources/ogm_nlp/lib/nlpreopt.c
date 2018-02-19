/*
 *  Handling choice of request expressions
 *  Copyright (c) 2017 Pertimm, by Patrick Constant
 *  Dev : October 2017
 *  Version 1.0
 */
#include "ogm_nlp.h"

static og_status NlpRequestExpressionsOptimizeIncluded(og_nlp_th ctrl_nlp_th);
static og_status NlpRequestExpressionOptimizeIncluded(og_nlp_th ctrl_nlp_th,
    struct request_expression *request_expressions, int Irequest_expression, int request_expression_used);
static int NlpRequestExpressionOptimizeIncludedCmp(gconstpointer ptr_request_expression1,
    gconstpointer ptr_request_expression2, gpointer user_data);
static og_status NlpRequestExpressionOptimizeIncludedRemove(og_nlp_th ctrl_nlp_th,
    struct request_expression *request_expression_big, GList *iter_big);
static og_bool NlpRequestExpressionIsIncluded(og_nlp_th ctrl_nlp_th, struct request_expression *request_expression_big,
    struct request_expression *request_expression);
static og_bool NlpRequestPositionIsIncluded(og_nlp_th ctrl_nlp_th, int request_position_big_start,
    int request_positions_big_nb, int request_position_start, int request_positions_nb);
static og_status NlpRequestExpressionOptimizeSparseRemove(og_nlp_th ctrl_nlp_th,
    struct request_expression *request_expression_big, GList *iter_big);
static og_bool NlpRequestExpressionHasCommonSubExpression(og_nlp_th ctrl_nlp_th,
    struct request_expression *request_expression1, struct request_expression *request_expression2);
static og_status NlpRequestExpressionOptimizeGetSparseValue(og_nlp_th ctrl_nlp_th,
    struct request_expression *request_expression);

og_status NlpRequestExpressionsOptimize(og_nlp_th ctrl_nlp_th)
{
  IFE(NlpRequestExpressionsOptimizeIncluded(ctrl_nlp_th));
  if (ctrl_nlp_th->loginfo->trace & DOgNlpTraceMatch)
  {
    char buffer[DPcPathSize];
    snprintf(buffer, DPcPathSize, "List of all request expression at level %d:", ctrl_nlp_th->level);
    IFE(NlpRequestExpressionsLog(ctrl_nlp_th, 0, buffer));
  }
  IFE(NlpRequestExpressionsClean(ctrl_nlp_th));
  DONE;
}

/*
 * When there is several expressions found within the same interpretation,
 * some are more compact than others, we remove the less compact ones
 */
static og_status NlpRequestExpressionsOptimizeIncluded(og_nlp_th ctrl_nlp_th)
{
  int request_expression_used = OgHeapGetCellsUsed(ctrl_nlp_th->hrequest_expression);
  struct request_expression *request_expressions = OgHeapGetCell(ctrl_nlp_th->hrequest_expression, 0);
  IFn(request_expressions) DPcErr;

  for (int i = 0; i < request_expression_used; i++)
  {
    struct request_expression *request_expression = request_expressions + i;
    // We do not want to remove the any expressions, because they are validated or not
    // only when the complete parsing is finished
    //if (request_expression->nb_anys > 0) request_expression->analyzed = 1;
    //else request_expression->analyzed = 0;
    request_expression->analyzed = 0;
    request_expression->deleted = 0;
  }

  for (int i = 0; i < request_expression_used; i++)
  {
    struct request_expression *request_expression = request_expressions + i;
    if (request_expression->analyzed) continue;
    IFE(NlpRequestExpressionOptimizeIncluded(ctrl_nlp_th, request_expressions, i, request_expression_used));
  }

  DONE;
}

static og_status NlpRequestExpressionOptimizeIncluded(og_nlp_th ctrl_nlp_th,
    struct request_expression *request_expressions, int Irequest_expression, int request_expression_used)
{
  struct request_expression *first_request_expression = request_expressions + Irequest_expression;

  GQueue queue_request_expression[1];
  g_queue_init(queue_request_expression);

  // Creating the list of request_expression that have the same expression
  for (int i = Irequest_expression; i < request_expression_used; i++)
  {
    struct request_expression *request_expression = request_expressions + i;
    if (request_expression->analyzed) continue;
    if (request_expression->expression->interpretation != first_request_expression->expression->interpretation) continue;
    g_queue_push_tail(queue_request_expression, request_expression);
    request_expression->analyzed = 1;
  }

  int length = g_queue_get_length(queue_request_expression);
  if (length > 1)
  {
    static int titi = 0;
    titi++;
    // We sort by biggest request expressions, and, for each of them we remove
    // all the expressions that are contained in them
    g_queue_sort(queue_request_expression, (GCompareDataFunc) NlpRequestExpressionOptimizeIncludedCmp, NULL);

    for (GList *iter = queue_request_expression->head; iter; iter = iter->next)
    {
      struct request_expression *request_expression = iter->data;
      IFE(NlpRequestExpressionOptimizeIncludedRemove(ctrl_nlp_th, request_expression, iter));
    }

    if (titi == 9)
    {
      int tutu = 1;
    }

    for (GList *iter = queue_request_expression->head; iter; iter = iter->next)
    {
      struct request_expression *request_expression = iter->data;
      IFE(NlpRequestExpressionOptimizeSparseRemove(ctrl_nlp_th, request_expression, iter));
    }

    if (ctrl_nlp_th->loginfo->trace & DOgNlpTraceMatch)
    {
      OgMsg(ctrl_nlp_th->hmsg, "", DOgMsgDestInLog,
          "NlpRequestExpressionOptimizeIncluded: cleaning the following request expressions: titi=%d", titi);
      for (GList *iter = queue_request_expression->head; iter; iter = iter->next)
      {
        struct request_expression *request_expression = iter->data;
        IFE(NlpRequestExpressionLog(ctrl_nlp_th, request_expression, 2));
      }
    }
  }
  g_queue_clear(queue_request_expression);

  DONE;
}

static int NlpRequestExpressionOptimizeIncludedCmp(gconstpointer ptr_request_expression1,
    gconstpointer ptr_request_expression2, gpointer user_data)
{
  struct request_expression *request_expression1 = (struct request_expression *) ptr_request_expression1;
  struct request_expression *request_expression2 = (struct request_expression *) ptr_request_expression2;

  if (request_expression1->request_positions_nb != request_expression2->request_positions_nb)
  {
    return (request_expression2->request_positions_nb - request_expression1->request_positions_nb);
  }
  if (request_expression1->overlap_mark != request_expression2->overlap_mark)
  {
    return (request_expression1->overlap_mark - request_expression2->overlap_mark);
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

static og_status NlpRequestExpressionOptimizeIncludedRemove(og_nlp_th ctrl_nlp_th,
    struct request_expression *request_expression_big, GList *iter_big)
{
  for (GList *iter = iter_big->next; iter; iter = iter->next)
  {
    struct request_expression *request_expression = iter->data;
    og_bool is_bigger = NlpRequestExpressionIsIncluded(ctrl_nlp_th, request_expression_big, request_expression);
    IFE(is_bigger);
    if (is_bigger)
    {
      if (!NlpDifferentAutoCompleteRequestWord(ctrl_nlp_th, request_expression_big, request_expression))
      {
        request_expression->deleted = 1;
      }
    }
  }
  DONE;
}

static og_bool NlpRequestExpressionIsIncluded(og_nlp_th ctrl_nlp_th, struct request_expression *request_expression_big,
    struct request_expression *request_expression)
{
  if (request_expression_big->nb_anys != request_expression->nb_anys) return FALSE;
  return NlpRequestPositionIsIncluded(ctrl_nlp_th, request_expression_big->request_position_start,
      request_expression_big->request_positions_nb, request_expression->request_position_start,
      request_expression->request_positions_nb);
}

static og_bool NlpRequestPositionIsIncluded(og_nlp_th ctrl_nlp_th, int request_position_big_start,
    int request_positions_big_nb, int request_position_start, int request_positions_nb)
{
  struct request_position *request_position_big = OgHeapGetCell(ctrl_nlp_th->hrequest_position,
      request_position_big_start);
  IFN(request_position_big) DPcErr;
  struct request_position *request_position = OgHeapGetCell(ctrl_nlp_th->hrequest_position, request_position_start);
  IFN(request_position) DPcErr;

  for (int i = 0; i < request_positions_nb; i++)
  {
    int position_included = FALSE;
    for (int j = 0; j < request_positions_big_nb; j++)
    {
      if (request_position[i].start == request_position_big[j].start)
      {
        position_included = TRUE;
        break;
      }
    }
    if (!position_included) return FALSE;
  }
  return TRUE;
}

static og_status NlpRequestExpressionOptimizeSparseRemove(og_nlp_th ctrl_nlp_th,
    struct request_expression *request_expression_big, GList *iter_big)
{
  if (request_expression_big->deleted) DONE;
  int big_sparse_value = NlpRequestExpressionOptimizeGetSparseValue(ctrl_nlp_th, request_expression_big);
  IFE(big_sparse_value);
  for (GList *iter = iter_big->next; iter; iter = iter->next)
  {
    struct request_expression *request_expression = iter->data;
    og_bool has_common_sub_expression = NlpRequestExpressionHasCommonSubExpression(ctrl_nlp_th, request_expression_big,
        request_expression);
    IFE(has_common_sub_expression);
    if (!has_common_sub_expression) continue;
    int sparse_value = NlpRequestExpressionOptimizeGetSparseValue(ctrl_nlp_th, request_expression);
    IFE(sparse_value);
    if (sparse_value > big_sparse_value)
    {
      request_expression->deleted = 1;
    }
    else if (big_sparse_value > sparse_value)
    {
      request_expression_big->deleted = 1;
      break;
    }
  }
  DONE;
}

static og_bool NlpRequestExpressionHasCommonSubExpression(og_nlp_th ctrl_nlp_th,
    struct request_expression *request_expression1, struct request_expression *request_expression2)
{
  static int tata = 0; tata++;
  og_bool has_common_sub_expression = FALSE;
  for (int i = 0; i < request_expression1->orips_nb; i++)
  {
    struct request_input_part *request_input_part1 = NlpGetRequestInputPart(ctrl_nlp_th, request_expression1, i);
    IFN(request_input_part1) DPcErr;
    for (int j = 0; j < request_expression2->orips_nb; j++)
    {
      struct request_input_part *request_input_part2 = NlpGetRequestInputPart(ctrl_nlp_th, request_expression2, j);
      IFN(request_input_part2) DPcErr;
      if (request_input_part1->input_part != request_input_part2->input_part) continue;
      og_bool same_positions = NlpRequestPositionSame(ctrl_nlp_th, request_input_part1->request_position_start,
          request_input_part1->request_positions_nb, request_input_part2->request_position_start,
          request_input_part2->request_positions_nb);
      if (same_positions)
      {
        has_common_sub_expression = TRUE;
        goto end_NlpRequestExpressionHasCommonSubExpression;
      }
    }
  }
  end_NlpRequestExpressionHasCommonSubExpression:
//  if (ctrl_nlp_th->loginfo->trace & DOgNlpTraceMatch)
//  {
//    OgMsg(ctrl_nlp_th->hmsg, "", DOgMsgDestInLog, "Comparing for common sub expression (%s) tata=%d",
//        has_common_sub_expression ? "true" : "false", tata);
//    IFE(NlpInterpretTreeLog(ctrl_nlp_th, request_expression1));
//    IFE(NlpInterpretTreeLog(ctrl_nlp_th, request_expression2));
//  }
  return has_common_sub_expression;

  DONE;
}

/*
 * We have this kind of result we only want to keep [with] [sea] [view]
 8:2 [0:4 5:8 14:4] '@{preposition_building_feature} @{building_feature}' '[with] [swimming] [pool] with golf with sea view with spa'
 9:2 [0:4 34:3 38:4] '@{preposition_building_feature} @{building_feature}''[with] swimming pool with golf with [sea] [view] with spa'
 10:2 [19:4 34:3 38:4] '@{preposition_building_feature} @{building_feature}' 'with swimming pool [with] golf with [sea] [view] with spa'
 11:2 [29:4 34:3 38:4] '@{preposition_building_feature} @{building_feature}' 'with swimming pool with golf [with] [sea] [view] with spa'
 */

static og_status NlpRequestExpressionOptimizeGetSparseValue(og_nlp_th ctrl_nlp_th,
    struct request_expression *request_expression)
{
  return NlpRequestPositionDistance(ctrl_nlp_th, request_expression->request_position_start,
      request_expression->request_positions_nb);
}

