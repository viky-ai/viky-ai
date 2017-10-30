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
    int request_expression_used);
static og_bool NlpRequestExpressionSame(og_nlp_th ctrl_nlp_th, struct request_expression *request_expression1,
    struct request_expression *request_expression2);
static og_status NlpRequestInterpretationBuild(og_nlp_th ctrl_nlp_th, struct request_expression *request_expression,
    json_t *json_interpretations);

og_bool NlpRequestExpressionAdd(og_nlp_th ctrl_nlp_th, struct expression *expression,
    struct match_zone_input_part *match_zone_input_part, struct request_expression **prequest_expression)
{
  int request_expression_used = OgHeapGetCellsUsed(ctrl_nlp_th->hrequest_expression);
  int request_position_used = OgHeapGetCellsUsed(ctrl_nlp_th->hrequest_position);

  size_t Irequest_expression;
  struct request_expression *request_expression = OgHeapNewCell(ctrl_nlp_th->hrequest_expression, &Irequest_expression);
  IFn(request_expression) DPcErr;
  IF(Irequest_expression) DPcErr;
  request_expression->self_index = Irequest_expression;
  request_expression->expression = expression;
  request_expression->level = ctrl_nlp_th->level;
  request_expression->request_position_start = OgHeapGetCellsUsed(ctrl_nlp_th->hrequest_position);
  IF(request_expression->request_position_start) DPcErr;

  struct request_input_part *request_input_part = OgHeapGetCell(ctrl_nlp_th->hrequest_input_part, 0);
  IFN(request_input_part) DPcErr;
  request_expression->request_positions_nb = 0;
  for (int i = 0; i < expression->input_parts_nb; i++)
  {
    struct request_position *request_position = OgHeapGetCell(ctrl_nlp_th->hrequest_position,
        request_input_part[match_zone_input_part[i].current].request_position_start);
    IFN(request_position) DPcErr;
    IF(OgHeapAppend(ctrl_nlp_th->hrequest_position,request_input_part[match_zone_input_part[i].current].request_positions_nb,request_position)) DPcErr;
    request_expression->request_positions_nb +=
        request_input_part[match_zone_input_part[i].current].request_positions_nb;
  }
  IF(NlpRequestPositionSort(ctrl_nlp_th, request_expression->request_position_start, request_expression->request_positions_nb)) DPcErr;

  int must_add_request_expression = TRUE;

  og_bool request_position_overlap = NlpRequestPositionOverlap(ctrl_nlp_th, request_expression->request_position_start,
      request_expression->request_positions_nb);
  if (request_position_overlap) must_add_request_expression = FALSE;

  if (must_add_request_expression)
  {
    og_bool request_expression_exists = NlpRequestExpressionExists(ctrl_nlp_th, request_expression,
        request_expression_used);
    IF(request_expression_exists) DPcErr;
    if (request_expression_exists) must_add_request_expression = FALSE;
  }

  og_bool request_expression_added = TRUE;
  if (must_add_request_expression)
  {
    request_expression->orip_start = (-1);
    request_expression->orips_nb = 0;
    for (int i = 0; i < expression->input_parts_nb; i++)
    {
      og_status status = NlpRequestExpressionAddOrip(ctrl_nlp_th, request_expression,
          request_input_part[match_zone_input_part[i].current].Ioriginal_request_input_part);
      IFE(status);
    }

    *prequest_expression = request_expression;
  }
  else
  {
    OgHeapSetCellsUsed(ctrl_nlp_th->hrequest_expression, request_expression_used);
    OgHeapSetCellsUsed(ctrl_nlp_th->hrequest_position, request_position_used);
    request_expression_added = FALSE;
    *prequest_expression = NULL;
  }
  return (request_expression_added);
}

static og_bool NlpRequestExpressionExists(og_nlp_th ctrl_nlp_th, struct request_expression *request_expression,
    int request_expression_used)
{
  struct request_expression *all_request_expression = OgHeapGetCell(ctrl_nlp_th->hrequest_expression, 0);
  for (int i = 0; i < request_expression_used; i++)
  {
    og_bool is_same_request_expression = NlpRequestExpressionSame(ctrl_nlp_th, request_expression,
        all_request_expression + i);
    if (is_same_request_expression) return TRUE;
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

og_status NlpRequestExpressionsExplicit(og_nlp_th ctrl_nlp_th)
{
  int request_expression_used = OgHeapGetCellsUsed(ctrl_nlp_th->hrequest_expression);
  struct request_expression *request_expression = OgHeapGetCell(ctrl_nlp_th->hrequest_expression, 0);
  IFN(request_expression) DPcErr;

  if (request_expression_used > 0)
  {
    struct request_expression *last_request_expression = request_expression + request_expression_used - 1;
    IFE(NlpRequestAnysAdd(ctrl_nlp_th, last_request_expression));
    IFE(NlpRequestExpressionAnysLog(ctrl_nlp_th, last_request_expression));
    IFE(NlpInterpretTreeLog(ctrl_nlp_th, last_request_expression));
  }

  g_qsort_with_data(request_expression, request_expression_used, sizeof(struct request_expression),
      NlpRequestExpressionCmp, NULL);

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
  // Just to make sure it is different
  return request_expression2 - request_expression1;
}

/*
 * For the moment, we build all the top level interpretations from the expressions
 */
og_status NlpRequestInterpretationsBuild(og_nlp_th ctrl_nlp_th, json_t *json_interpretations)
{
  int request_expression_used = OgHeapGetCellsUsed(ctrl_nlp_th->hrequest_expression);

  struct request_expression *request_expression = OgHeapGetCell(ctrl_nlp_th->hrequest_expression, 0);
  IFN(request_expression) DPcErr;

  int top_level = request_expression->level;

  for (int i = 0; i < request_expression_used; i++)
  {
    if (request_expression[i].level != top_level) break;
    IFE(NlpRequestInterpretationBuild(ctrl_nlp_th, request_expression + i, json_interpretations));
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

  IF(json_array_append_new(json_interpretations, json_interpretation))
  {
    NlpThrowErrorTh(ctrl_nlp_th, "NlpInterpretRequestInterpretation: error appending json_interpretation to array");
    DPcErr;
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
  NlpRequestPositionStringPretty(ctrl_nlp_th, request_expression->request_position_start,
      request_expression->request_positions_nb, DPcPathSize, highlight);

  struct expression *expression = request_expression->expression;

  OgMsg(ctrl_nlp_th->hmsg, "", DOgMsgDestInLog, "%s%2d: [%s] '%.*s' in interpretation '%s': '%s'", string_offset,
      request_expression->level, string_positions, DPcPathSize, expression->text, expression->interpretation->slug,
      highlight);
  DONE;
}
