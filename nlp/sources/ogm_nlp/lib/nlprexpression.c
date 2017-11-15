/*
 *  Handling request input parts
 *  Copyright (c) 2017 Pertimm, by Patrick Constant
 *  Dev : October 2017
 *  Version 1.0
 */
#include "ogm_nlp.h"

static int NlpRequestExpressionCmp(gconstpointer ptr_request_expression1, gconstpointer ptr_request_expression2,
    gpointer user_data);
static og_bool NlpRequestExpressionExists(og_nlp_th ctrl_nlp_th, struct request_expression *request_expression,
    int request_expression_used, struct request_expression **psame_request_expression);
static og_bool NlpRequestExpressionSame(og_nlp_th ctrl_nlp_th, struct request_expression *request_expression1,
    struct request_expression *request_expression2);
static og_bool NlpRequestExpressionIsOrdered(og_nlp_th ctrl_nlp_th, struct request_expression *request_expression);
static og_bool NlpRequestExpressionIsGlued(og_nlp_th ctrl_nlp_th, struct request_expression *request_expression);
static og_status NlpRequestExpressionOverlapMark(og_nlp_th ctrl_nlp_th, struct request_expression *request_expression);
static og_status NlpRequestExpressionInputPartsOverlapMark(og_nlp_th ctrl_nlp_th,
    struct request_input_part *request_input_part1, struct request_input_part *request_input_part2, int *poverlap_mark);
static og_status NlpRequestInterpretationBuild(og_nlp_th ctrl_nlp_th, struct request_expression *request_expression,
    json_t *json_interpretations);

og_bool NlpRequestExpressionAdd(og_nlp_th ctrl_nlp_th, struct expression *expression,
    struct match_zone_input_part *match_zone_input_part, struct request_expression **prequest_expression)
{
  int request_expression_used = OgHeapGetCellsUsed(ctrl_nlp_th->hrequest_expression);
  int request_position_used = OgHeapGetCellsUsed(ctrl_nlp_th->hrequest_position);
  int orip_used = OgHeapGetCellsUsed(ctrl_nlp_th->horip);

  size_t Irequest_expression;
  struct request_expression *request_expression = OgHeapNewCell(ctrl_nlp_th->hrequest_expression, &Irequest_expression);
  IFn(request_expression) DPcErr;
  IF(Irequest_expression) DPcErr;
  request_expression->self_index = Irequest_expression;
  request_expression->expression = expression;
  request_expression->level = ctrl_nlp_th->level;
  request_expression->Irequest_any = (-1);
  request_expression->keep_as_result = FALSE;
  g_queue_init(request_expression->tmp_solutions);
  request_expression->overlap_mark = 0;
  if (request_expression->expression->alias_any_input_part_position >= 0)
  {
    request_expression->contains_any = TRUE;
  }
  else
  {
    request_expression->contains_any = FALSE;
  }

  request_expression->request_position_start = OgHeapGetCellsUsed(ctrl_nlp_th->hrequest_position);
  IF(request_expression->request_position_start) DPcErr;
  struct request_input_part *request_input_parts = OgHeapGetCell(ctrl_nlp_th->hrequest_input_part, 0);
  IFN(request_input_parts) DPcErr;
  request_expression->request_positions_nb = 0;
  for (int i = 0; i < expression->input_parts_nb; i++)
  {
    struct request_input_part *request_input_part = request_input_parts + match_zone_input_part[i].current;

    // prealloc due to app call
    int Irequest_position_start = OgHeapAddCells(ctrl_nlp_th->hrequest_position,
        request_input_part->request_positions_nb);

    struct request_position *request_position_from = OgHeapGetCell(ctrl_nlp_th->hrequest_position,
        request_input_part->request_position_start);
    IFN(request_position_from) DPcErr;

    struct request_position *request_position_to = OgHeapGetCell(ctrl_nlp_th->hrequest_position,
        Irequest_position_start);
    IFN(request_position_to) DPcErr;

    memcpy(request_position_to, request_position_from,
        sizeof(struct request_position) * request_input_part->request_positions_nb);

    request_expression->request_positions_nb += request_input_part->request_positions_nb;

  }
  IF(NlpRequestPositionSort(ctrl_nlp_th, request_expression->request_position_start, request_expression->request_positions_nb)) DPcErr;

  // Necessary for NlpRequestExpressionOverlapMark and NlpRequestExpressionIsOrdered
  request_expression->orip_start = (-1);
  request_expression->orips_nb = 0;
  for (int i = 0; i < expression->input_parts_nb; i++)
  {
    struct request_input_part *request_input_part = request_input_parts + match_zone_input_part[i].current;

    og_status status = NlpRequestExpressionAddOrip(ctrl_nlp_th, request_expression,
        request_input_part->Ioriginal_request_input_part);
    IFE(status);
  }

  int must_add_request_expression = TRUE;

  if (must_add_request_expression)
  {
    if (request_expression->expression->keep_order)
    {
      og_bool is_ordered = NlpRequestExpressionIsOrdered(ctrl_nlp_th, request_expression);
      IFE(is_ordered);
      if (!is_ordered) must_add_request_expression = FALSE;
    }
  }

  if (must_add_request_expression)
  {
    if (request_expression->expression->glued)
    {
      og_bool is_glued = NlpRequestExpressionIsGlued(ctrl_nlp_th, request_expression);
      IFE(is_glued);
      if (!is_glued) must_add_request_expression = FALSE;
    }
  }

  if (must_add_request_expression)
  {
    og_bool request_position_overlap = NlpRequestPositionOverlap(ctrl_nlp_th,
        request_expression->request_position_start, request_expression->request_positions_nb);
    if (request_position_overlap) must_add_request_expression = FALSE;
  }

  if (must_add_request_expression)
  {
    // Must be called before NlpRequestExpressionExists
    IFE(NlpRequestExpressionOverlapMark(ctrl_nlp_th, request_expression));
    // When there is a recursive expression, any overlapping should be remove
    // as it adds nothing to the results, because this is a repetition of the same object
    if (request_expression->overlap_mark > 0 && request_expression->expression->interpretation->is_recursive)
    {
      must_add_request_expression = FALSE;
    }
  }

  if (must_add_request_expression)
  {
    struct request_expression *same_request_expression;
    og_bool request_expression_exists = NlpRequestExpressionExists(ctrl_nlp_th, request_expression,
        request_expression_used, &same_request_expression);
    IF(request_expression_exists) DPcErr;
    if (request_expression_exists)
    {
      og_bool must_log_compared_expressions = TRUE;
      if (request_expression->request_positions_nb == 1) must_log_compared_expressions = FALSE;
      if (must_log_compared_expressions)
      {
//          OgMsg(ctrl_nlp_th->hmsg, "", DOgMsgDestInLog,
//              "comparing new request expression %d with same request expression %d:",
//              request_expression->input_parts_compacity, same_request_expression->input_parts_compacity);
//          IFE(NlpInterpretTreeLog(ctrl_nlp_th, request_expression));
//          IFE(NlpInterpretTreeLog(ctrl_nlp_th, same_request_expression));
      }
      must_add_request_expression = FALSE;
    }
  }

  og_bool request_expression_added = TRUE;
  if (must_add_request_expression)
  {
    if (!request_expression->contains_any)
    {
      for (int i = 0; i < request_expression->orips_nb; i++)
      {
        struct request_input_part *request_input_part = NlpGetRequestInputPart(ctrl_nlp_th, request_expression, i);
        IFN(request_input_part) DPcErr;

        if (request_input_part->type == nlp_input_part_type_Interpretation)
        {
          struct request_expression *sub_request_expression = OgHeapGetCell(ctrl_nlp_th->hrequest_expression,
              request_input_part->Irequest_expression);
          IFN(sub_request_expression) DPcErr;
          if (sub_request_expression->contains_any)
          {
            request_expression->contains_any = TRUE;
          }
        }
      }
    }
    *prequest_expression = request_expression;
  }
  else
  {
    OgHeapSetCellsUsed(ctrl_nlp_th->hrequest_expression, request_expression_used);
    OgHeapSetCellsUsed(ctrl_nlp_th->hrequest_position, request_position_used);
    OgHeapSetCellsUsed(ctrl_nlp_th->horip, orip_used);
    request_expression_added = FALSE;
    *prequest_expression = NULL;
  }
  return (request_expression_added);
}

static og_bool NlpRequestExpressionExists(og_nlp_th ctrl_nlp_th, struct request_expression *request_expression,
    int request_expression_used, struct request_expression **psame_request_expression)
{
  *psame_request_expression = NULL;
  struct request_expression *all_request_expression = OgHeapGetCell(ctrl_nlp_th->hrequest_expression, 0);
  for (int i = 0; i < request_expression_used; i++)
  {
    og_bool is_same_request_expression = NlpRequestExpressionSame(ctrl_nlp_th, request_expression,
        all_request_expression + i);
    if (is_same_request_expression)
    {
//      OgMsg(ctrl_nlp_th->hmsg, "", DOgMsgDestInLog,
//          "NlpRequestExpressionExists compared equal new request expression with same request expression:");
//      IFE(NlpInterpretTreeLog(ctrl_nlp_th, request_expression));
//      IFE(NlpInterpretTreeLog(ctrl_nlp_th, all_request_expression + i));
      return TRUE;
    }
  }
  return FALSE;
}

static og_bool NlpRequestExpressionSame(og_nlp_th ctrl_nlp_th, struct request_expression *request_expression1,
    struct request_expression *request_expression2)
{
  if (request_expression1->expression != request_expression2->expression) return FALSE;
  if (request_expression1->overlap_mark != request_expression2->overlap_mark) return FALSE;
  og_bool same_positions = NlpRequestPositionSame(ctrl_nlp_th, request_expression1->request_position_start,
      request_expression1->request_positions_nb, request_expression2->request_position_start,
      request_expression2->request_positions_nb);
  return (same_positions);
}

static og_bool NlpRequestExpressionIsOrdered(og_nlp_th ctrl_nlp_th, struct request_expression *request_expression)
{
  for (int i = 0; i + 1 < request_expression->orips_nb; i++)
  {
    struct request_input_part *request_input_part1 = NlpGetRequestInputPart(ctrl_nlp_th, request_expression, i);
    IFN(request_input_part1) DPcErr;
    struct request_input_part *request_input_part2 = NlpGetRequestInputPart(ctrl_nlp_th, request_expression, i + 1);
    IFN(request_input_part2) DPcErr;

    og_bool is_ordered = NlpRequestInputPartsAreOrdered(ctrl_nlp_th, request_input_part1, request_input_part2);
    IFE(is_ordered);
    if (!is_ordered) return FALSE;
  }

  return TRUE;
}

static og_bool NlpRequestExpressionIsGlued(og_nlp_th ctrl_nlp_th, struct request_expression *request_expression)
{
  for (int i = 0; i + 1 < request_expression->orips_nb; i++)
  {
    struct request_input_part *request_input_part1 = NlpGetRequestInputPart(ctrl_nlp_th, request_expression, i);
    IFN(request_input_part1) DPcErr;
    struct request_input_part *request_input_part2 = NlpGetRequestInputPart(ctrl_nlp_th, request_expression, i + 1);
    IFN(request_input_part2) DPcErr;

    og_bool is_glued = NlpRequestInputPartsAreGlued(ctrl_nlp_th, request_input_part1, request_input_part2);
    IFE(is_glued);
    if (!is_glued) return FALSE;
  }

  return TRUE;
}

static og_status NlpRequestExpressionOverlapMark(og_nlp_th ctrl_nlp_th, struct request_expression *request_expression)
{
  request_expression->overlap_mark = 0;

  if (request_expression->orips_nb <= 1) DONE;

  for (int i = 0; i + 1 < request_expression->orips_nb; i++)
  {
    struct request_input_part *request_input_part1 = NlpGetRequestInputPart(ctrl_nlp_th, request_expression, i);
    IFN(request_input_part1) DPcErr;

    for (int j = i + 1; j < request_expression->orips_nb; j++)
    {
      struct request_input_part *request_input_part2 = NlpGetRequestInputPart(ctrl_nlp_th, request_expression, j);
      IFN(request_input_part2) DPcErr;

      int inputr_parts_overlap_mark;
      IFE(
          NlpRequestExpressionInputPartsOverlapMark(ctrl_nlp_th, request_input_part1, request_input_part2,
              &inputr_parts_overlap_mark));
      request_expression->overlap_mark += inputr_parts_overlap_mark;
    }
  }

  DONE;
}

static og_status NlpRequestExpressionInputPartsOverlapMark(og_nlp_th ctrl_nlp_th,
    struct request_input_part *request_input_part1, struct request_input_part *request_input_part2, int *poverlap_mark)
{
  struct request_position *request_position;

  request_position = OgHeapGetCell(ctrl_nlp_th->hrequest_position, request_input_part1->request_position_start);
  IFN(request_position) DPcErr;
  int start_position1 = request_position->start;
  request_position = OgHeapGetCell(ctrl_nlp_th->hrequest_position,
      request_input_part1->request_position_start + request_input_part1->request_positions_nb - 1);
  IFN(request_position) DPcErr;
  int end_position1 = request_position->start + request_position->length;

  request_position = OgHeapGetCell(ctrl_nlp_th->hrequest_position, request_input_part2->request_position_start);
  IFN(request_position) DPcErr;
  int start_position2 = request_position->start;
  request_position = OgHeapGetCell(ctrl_nlp_th->hrequest_position,
      request_input_part2->request_position_start + request_input_part2->request_positions_nb - 1);
  IFN(request_position) DPcErr;
  int end_position2 = request_position->start + request_position->length;

  int overlap_mark = 0;
  // a b a b -> +100 (full overlap)
  if (start_position1 < start_position2 && start_position2 < end_position1 && end_position1 < end_position2) overlap_mark +=
      100;
  // a b b a -> +1   (included)
  if (start_position1 < start_position2 && end_position2 < end_position1) overlap_mark += 1;
  // b a a b -> +1   (included)
  if (start_position2 < start_position1 && end_position1 < end_position2) overlap_mark += 1;
  // b a b a -> +100 (full overlap)
  if (start_position2 < start_position1 && start_position1 < end_position2 && end_position2 < end_position1) overlap_mark +=
      100;

//  if (start_position1 < start_position2 && start_position2 < end_position1) return TRUE;
//  if (start_position1 < end_position2 && end_position2 < end_position1) return TRUE;
//
//  if (start_position2 < start_position1 && start_position1 < end_position2) return TRUE;
//  if (start_position2 < end_position1 && end_position1 < end_position2) return TRUE;

  *poverlap_mark = overlap_mark;

  return FALSE;
}

og_status NlpRequestExpressionsExplicit(og_nlp_th ctrl_nlp_th)
{
  int request_expression_used = OgHeapGetCellsUsed(ctrl_nlp_th->hrequest_expression);
  if (request_expression_used <= 0) DONE;

  struct request_expression *request_expressions = OgHeapGetCell(ctrl_nlp_th->hrequest_expression, 0);
  IFN(request_expressions) DPcErr;

  GQueue *sorted_request_expressions = ctrl_nlp_th->sorted_request_expressions;
  for (int i = 0; i < request_expression_used; i++)
  {
    g_queue_push_tail(sorted_request_expressions, request_expressions + i);
  }
  g_queue_sort(sorted_request_expressions, (GCompareDataFunc) NlpRequestExpressionCmp, NULL);

  struct request_expression *first_request_expression = sorted_request_expressions->head->data;
  for (GList *iter = sorted_request_expressions->head; iter; iter = iter->next)
  {
    struct request_expression *request_expression = iter->data;
    if (request_expression->level == first_request_expression->level
        && request_expression->request_positions_nb == first_request_expression->request_positions_nb
        && request_expression->overlap_mark == first_request_expression->overlap_mark)
    {
      request_expression->keep_as_result = TRUE;
    }
  }

  for (GList *iter = sorted_request_expressions->head; iter; iter = iter->next)
  {
    struct request_expression *request_expression = iter->data;
    if (!request_expression->keep_as_result) break;
    IFE(NlpRequestAnysAdd(ctrl_nlp_th, request_expression));
    IFE(NlpInterpretTreeAttachAny(ctrl_nlp_th, request_expression));
    IFE(NlpRequestAnyOptimizeMatch(ctrl_nlp_th, request_expression));
    IFE(NlpSolutionCalculate(ctrl_nlp_th, request_expression));
    IFE(NlpCalculateLocaleScore(ctrl_nlp_th, request_expression));
  }

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

static int NlpRequestExpressionCmp(gconstpointer ptr_request_expression1, gconstpointer ptr_request_expression2,
    gpointer user_data)
{
  struct request_expression *request_expression1 = (struct request_expression *) ptr_request_expression1;
  struct request_expression *request_expression2 = (struct request_expression *) ptr_request_expression2;

  if (request_expression1->request_positions_nb != request_expression2->request_positions_nb)
  {
    return (request_expression2->request_positions_nb - request_expression1->request_positions_nb);
  }
  if (request_expression1->level != request_expression2->level)
  {
    return (request_expression2->level - request_expression1->level);
  }
  if (request_expression1->overlap_mark != request_expression2->overlap_mark)
  {
    return (request_expression1->overlap_mark - request_expression2->overlap_mark);
  }
// Just to make sure it is different
  return request_expression1 - request_expression2;
}

/*
 * For the moment, we build all the top level interpretations from the expressions
 */
og_status NlpRequestInterpretationsBuild(og_nlp_th ctrl_nlp_th, json_t *json_interpretations)
{
  GQueue *sorted_request_expressions = ctrl_nlp_th->sorted_request_expressions;
  for (GList *iter = sorted_request_expressions->head; iter; iter = iter->next)
  {
    struct request_expression *request_expression = iter->data;
    if (!request_expression->keep_as_result) break;
    IFE(NlpRequestInterpretationBuild(ctrl_nlp_th, request_expression, json_interpretations));
  }
  DONE;
}

static og_status NlpRequestInterpretationBuild(og_nlp_th ctrl_nlp_th, struct request_expression *request_expression,
    json_t *json_interpretations)
{
  struct expression *expression = request_expression->expression;
  struct interpretation *interpretation = expression->interpretation;

  package_t package = interpretation->package;

  NlpLog(DOgNlpTraceInterpret, "NlpInterpretRequestInterpretation: found interpretation '%s' '%s' in package '%s' '%s'",
      interpretation->slug, interpretation->id, package->slug, package->id)

  json_t *json_interpretation = json_object();

  IF(json_array_append_new(json_interpretations, json_interpretation))
  {
    NlpThrowErrorTh(ctrl_nlp_th, "NlpInterpretRequestInterpretation: error appending json_interpretation to array");
    DPcErr;
  }

  json_t *json_package_id = json_string(package->id);
  IF(json_object_set_new(json_interpretation, "package", json_package_id))
  {
    NlpThrowErrorTh(ctrl_nlp_th, "NlpInterpretRequestInterpretation: error setting json_package_id");
    DPcErr;
  }

  json_t *json_interpretation_id = json_string(interpretation->id);
  IF(json_object_set_new(json_interpretation, "id", json_interpretation_id))
  {
    NlpThrowErrorTh(ctrl_nlp_th, "NlpInterpretRequestInterpretation: error setting json_interpretation_id");
    DPcErr;
  }

  // simulate a timeout right here
  IFE(OgNlpSynchroTestSleepIfTimeoutNeeded(ctrl_nlp_th, nlp_timeout_in_NlpRequestInterpretationBuild));

  json_t *json_interpretation_slug = json_string(interpretation->slug);
  IF(json_object_set_new(json_interpretation, "slug", json_interpretation_slug))
  {
    NlpThrowErrorTh(ctrl_nlp_th, "NlpInterpretRequestInterpretation: error setting json_interpretation_slug");
    DPcErr;
  }

  // For the moment, the only score is a locale score
  // Later we will add other calculation including level, coverage, spellchecking
  json_t *json_score = json_real(request_expression->locale_score);
  IF(json_object_set_new(json_interpretation, "score", json_score))
  {
    NlpThrowErrorTh(ctrl_nlp_th, "NlpInterpretRequestInterpretation: error setting json_score");
    DPcErr;
  }

  if (request_expression->json_solution)
  {
    IF(json_object_set(json_interpretation, "solution", request_expression->json_solution))
    {
      NlpThrowErrorTh(ctrl_nlp_th, "NlpInterpretRequestInterpretation: error setting json_solution");
      DPcErr;
    }
  }

  if (ctrl_nlp_th->show_explanation)
  {
    IFE(NlpInterpretTreeJson(ctrl_nlp_th, request_expression, json_interpretation));
  }

  DONE;
}

og_status NlpSortedRequestExpressionsLog(og_nlp_th ctrl_nlp_th, char *title)
{
  OgMsg(ctrl_nlp_th->hmsg, "", DOgMsgDestInLog, "%s", title);

  GQueue *sorted_request_expressions = ctrl_nlp_th->sorted_request_expressions;
  for (GList *iter = sorted_request_expressions->head; iter; iter = iter->next)
  {
    struct request_expression *request_expression = iter->data;
    IFE(NlpRequestExpressionLog(ctrl_nlp_th, request_expression, 2));
  }
  DONE;
}

og_status NlpRequestExpressionsLog(og_nlp_th ctrl_nlp_th, int request_expression_start, char *title)
{
  OgMsg(ctrl_nlp_th->hmsg, "", DOgMsgDestInLog, "%s", title);
  int request_expression_used = OgHeapGetCellsUsed(ctrl_nlp_th->hrequest_expression);
  for (int i = request_expression_start; i < request_expression_used; i++)
  {
    struct request_expression *request_expression = OgHeapGetCell(ctrl_nlp_th->hrequest_expression, i);
    IFN(request_expression) DPcErr;

    IFE(NlpRequestExpressionLog(ctrl_nlp_th, request_expression, 2));
  }
  DONE;
}

og_status NlpRequestExpressionLog(og_nlp_th ctrl_nlp_th, struct request_expression *request_expression, int offset)
{
  IFN(request_expression) DPcErr;

  char string_offset[DPcPathSize];
  memset(string_offset, ' ', offset);
  string_offset[offset] = 0;

  if (request_expression->deleted) string_offset[offset - 1] = '-';

  char string_positions[DPcPathSize];
  NlpRequestPositionString(ctrl_nlp_th, request_expression->request_position_start,
      request_expression->request_positions_nb, DPcPathSize, string_positions);

  char highlight[DPcPathSize];
  NlpRequestPositionStringHighlight(ctrl_nlp_th, request_expression->request_position_start,
      request_expression->request_positions_nb, DPcPathSize, highlight);

  char solution[DPcPathSize];
  NlpRequestSolutionString(ctrl_nlp_th, request_expression, DPcPathSize, solution);

  char overlap_mark[DPcPathSize];
  overlap_mark[0] = 0;
  if (request_expression->overlap_mark) sprintf(overlap_mark, " overlap_mark=%d", request_expression->overlap_mark);

  char any[DPcPathSize];
  any[0] = 0;
  if (request_expression->contains_any) sprintf(any, " any");

  char locale_score[DPcPathSize];
  locale_score[0] = 0;
  if (request_expression->locale_score > 0.0) sprintf(locale_score, " locale_score=%.2f",
      request_expression->locale_score);

  struct expression *expression = request_expression->expression;

  OgMsg(ctrl_nlp_th->hmsg, "", DOgMsgDestInLog, "%s%2d:%d%s [%s] '%.*s' in interpretation '%s': '%s'%s%s%s%s%s",
      string_offset, request_expression->self_index, request_expression->level,
      (request_expression->keep_as_result ? "*" : ""), string_positions, DPcPathSize, expression->text,
      expression->interpretation->slug, highlight, (solution[0] ? " " : ""), solution, any, overlap_mark, locale_score);
  DONE;
}

