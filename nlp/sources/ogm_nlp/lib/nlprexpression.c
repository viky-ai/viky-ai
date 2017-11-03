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

  request_expression->input_parts_compacity = 0;
  for (int i = 0; i < expression->input_parts_nb; i++)
  {
    request_expression->input_parts_compacity +=
        request_input_parts[match_zone_input_part[i].current].request_position_distance;
  }

  int must_add_request_expression = TRUE;

  og_bool request_position_overlap = NlpRequestPositionOverlap(ctrl_nlp_th, request_expression->request_position_start,
      request_expression->request_positions_nb);
  if (request_position_overlap) must_add_request_expression = FALSE;

  if (must_add_request_expression)
  {
    request_expression->orip_start = (-1);
    request_expression->orips_nb = 0;
    for (int i = 0; i < expression->input_parts_nb; i++)
    {
      struct request_input_part *request_input_part = request_input_parts + match_zone_input_part[i].current;

      og_status status = NlpRequestExpressionAddOrip(ctrl_nlp_th, request_expression,
          request_input_part->Ioriginal_request_input_part);
      IFE(status);
    }

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
//        OgMsg(ctrl_nlp_th->hmsg, "", DOgMsgDestInLog,
//            "comparing new request expression %d with same request expression %d:",
//            request_expression->input_parts_compacity, same_request_expression->input_parts_compacity);
//        IFE(NlpInterpretTreeLog(ctrl_nlp_th, request_expression));
//        IFE(NlpInterpretTreeLog(ctrl_nlp_th, same_request_expression));
      }
      must_add_request_expression = FALSE;
    }
  }

  if (must_add_request_expression)
  {
    if (request_expression->expression->keep_order)
    {
      og_bool is_ordered = NlpRequestExpressionIsOrdered(ctrl_nlp_th, request_expression);
      IFE(is_ordered);
      if (!is_ordered) must_add_request_expression = FALSE;
    }
  }

  og_bool request_expression_added = TRUE;
  if (must_add_request_expression)
  {
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
      *psame_request_expression = all_request_expression + i;
      return TRUE;
    }
  }
  return FALSE;
}

static og_bool NlpRequestExpressionSame(og_nlp_th ctrl_nlp_th, struct request_expression *request_expression1,
    struct request_expression *request_expression2)
{
  if (request_expression1->expression != request_expression2->expression) return FALSE;
  return (NlpRequestPositionSame(ctrl_nlp_th, request_expression1->request_position_start,
      request_expression1->request_positions_nb, request_expression2->request_position_start,
      request_expression2->request_positions_nb));
}

static og_bool NlpRequestExpressionIsOrdered(og_nlp_th ctrl_nlp_th, struct request_expression *request_expression)
{
  struct original_request_input_part *original_request_input_part = OgHeapGetCell(
      ctrl_nlp_th->horiginal_request_input_part, 0);
  IFN(original_request_input_part) DPcErr;

  struct orip *orip = OgHeapGetCell(ctrl_nlp_th->horip, 0);
  IFN(orip) DPcErr;

  for (int i = 0; i + 1 < request_expression->orips_nb; i++)
  {
    int Ioriginal_request_input_part1 = orip[request_expression->orip_start + i].Ioriginal_request_input_part;
    int Irequest_input_part1 = original_request_input_part[Ioriginal_request_input_part1].Irequest_input_part;
    struct request_input_part *request_input_part1 = OgHeapGetCell(ctrl_nlp_th->hrequest_input_part,
        Irequest_input_part1);
    IFN(request_input_part1) DPcErr;

    int Ioriginal_request_input_part2 = orip[request_expression->orip_start + i + 1].Ioriginal_request_input_part;
    int Irequest_input_part2 = original_request_input_part[Ioriginal_request_input_part2].Irequest_input_part;
    struct request_input_part *request_input_part2 = OgHeapGetCell(ctrl_nlp_th->hrequest_input_part,
        Irequest_input_part2);
    IFN(request_input_part2) DPcErr;

    og_bool is_ordered = NlpRequestInputPartsAreOrdered(ctrl_nlp_th, request_input_part1, request_input_part2);
    IFE(is_ordered);
    if (!is_ordered) return FALSE;
  }

  return TRUE;
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
        && request_expression->request_positions_nb == first_request_expression->request_positions_nb)
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

  if (request_expression1->level != request_expression2->level)
  {
    return (request_expression2->level - request_expression1->level);
  }
  if (request_expression1->request_positions_nb != request_expression2->request_positions_nb)
  {
    return (request_expression2->request_positions_nb - request_expression1->request_positions_nb);
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

  IFE(OgNlpSynchroTestSleepIfTimeoutNeeded(ctrl_nlp_th, nlp_timeout_in_NlpRequestInterpretationBuild));

  json_t *json_interpretation_slug = json_string(interpretation->slug);
  IF(json_object_set_new(json_interpretation, "slug", json_interpretation_slug))
  {
    NlpThrowErrorTh(ctrl_nlp_th, "NlpInterpretRequestInterpretation: error setting json_interpretation_slug");
    DPcErr;
  }

  json_t *json_score = json_real(1.0);
  IF(json_object_set_new(json_interpretation, "score", json_score))
  {
    NlpThrowErrorTh(ctrl_nlp_th, "NlpInterpretRequestInterpretation: error setting json_score");
    DPcErr;
  }

  if (ctrl_nlp_th->show_explanation)
  {
    IFE(NlpInterpretTreeJson(ctrl_nlp_th, request_expression, json_interpretation));
  }

  IF(json_array_append_new(json_interpretations, json_interpretation))
  {
    NlpThrowErrorTh(ctrl_nlp_th, "NlpInterpretRequestInterpretation: error appending json_interpretation to array");
    DPcErr;
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

  char string_positions[DPcPathSize];
  NlpRequestPositionString(ctrl_nlp_th, request_expression->request_position_start,
      request_expression->request_positions_nb, DPcPathSize, string_positions);

  char highlight[DPcPathSize];
  NlpRequestPositionStringHighlight(ctrl_nlp_th, request_expression->request_position_start,
      request_expression->request_positions_nb, DPcPathSize, highlight);

  char solution[DPcPathSize];
  NlpSolutionString(ctrl_nlp_th, request_expression, DPcPathSize, solution);

  struct expression *expression = request_expression->expression;

  OgMsg(ctrl_nlp_th->hmsg, "", DOgMsgDestInLog, "%s%2d:%d%s [%s] '%.*s' in interpretation '%s': '%s'%s%s", string_offset,
      request_expression->self_index, request_expression->level, (request_expression->keep_as_result ? "*" : ""),
      string_positions, DPcPathSize, expression->text, expression->interpretation->slug, highlight, (solution[0]?" ":""), solution);
  DONE;
}
