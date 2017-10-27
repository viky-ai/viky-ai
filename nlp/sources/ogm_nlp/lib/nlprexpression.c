/*
 *  Handling request input parts
 *  Copyright (c) 2017 Pertimm, by Patrick Constant
 *  Dev : October 2017
 *  Version 1.0
 */
#include "ogm_nlp.h"

static int NlpRequestExpressionCmp(gconstpointer ptr_request_expression1,
    gconstpointer ptr_request_expression2, gpointer user_data);
static og_status NlpRequestInterpretationBuild(og_nlp_th ctrl_nlp_th,
    struct request_expression *request_expression, json_t *json_interpretations);

og_status NlpRequestExpressionAdd(og_nlp_th ctrl_nlp_th, struct expression *expression, int level)
{
  size_t Irequest_expression;
  struct request_expression *request_expression = OgHeapNewCell(ctrl_nlp_th->hrequest_expression,
      &Irequest_expression);
  IFn(request_expression) DPcErr;
  IF(Irequest_expression) DPcErr;
  request_expression->expression = expression;
  request_expression->level = level;
  DONE;
}

og_status NlpRequestExpressionsExplicit(og_nlp_th ctrl_nlp_th)
{
  int request_expression_used = OgHeapGetCellsUsed(ctrl_nlp_th->hrequest_expression);
  struct request_expression *request_expression = OgHeapGetCell(ctrl_nlp_th->hrequest_expression, 0);
  IFN(request_expression) DPcErr;

  g_qsort_with_data(request_expression, request_expression_used, sizeof(struct request_expression),
      NlpRequestExpressionCmp, NULL);

  DONE;
}

static int NlpRequestExpressionCmp(gconstpointer ptr_request_expression1,
    gconstpointer ptr_request_expression2, gpointer user_data)
{
  struct request_expression *request_expression1 = (struct request_expression *) ptr_request_expression1;
  struct request_expression *request_expression2 = (struct request_expression *) ptr_request_expression2;

  if (request_expression1->level != request_expression2->level)
  {
    return (request_expression2->level - request_expression1->level);
  }
  // Just to make sure it is
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

static og_status NlpRequestInterpretationBuild(og_nlp_th ctrl_nlp_th,
    struct request_expression *request_expression, json_t *json_interpretations)
{
  struct expression *expression = request_expression->expression;
  struct interpretation *interpretation = expression->interpretation;

  package_t package = interpretation->package;

  if (ctrl_nlp_th->loginfo->trace & DOgNlpTraceCompile)
  {
    OgMsg(ctrl_nlp_th->hmsg, "", DOgMsgDestInLog,
        "NlpInterpretRequestInterpretation: found interpretation '%s' '%s' in package '%s' '%s'", interpretation->slug,
        interpretation->id, package->slug, package->id);
  }
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

og_status NlpRequestExpressionsLog(og_nlp_th ctrl_nlp_th)
{
  int request_expression_used = OgHeapGetCellsUsed(ctrl_nlp_th->hrequest_expression);

  OgMsg(ctrl_nlp_th->hmsg, "", DOgMsgDestInLog, "List of found expressions:");

  for (int i = 0; i < request_expression_used; i++)
  {
    struct request_expression *request_expression = OgHeapGetCell(ctrl_nlp_th->hrequest_expression, i);
    IFN(request_expression) DPcErr;

    IFE(NlpRequestExpressionLog(ctrl_nlp_th, request_expression));
  }
  DONE;
}

og_status NlpRequestExpressionLog(og_nlp_th ctrl_nlp_th, struct request_expression *request_expression)
{
  IFN(request_expression) DPcErr;

  struct expression *expression = request_expression->expression;

  OgMsg(ctrl_nlp_th->hmsg, "", DOgMsgDestInLog, "  %2d: '%.*s' in interpretation '%s' '%s'",
      request_expression->level, DPcPathSize, expression->text, expression->interpretation->slug,
      expression->interpretation->id);
  DONE;
}
