/*
 *  Handling request input parts
 *  Copyright (c) 2017 Pertimm, by Patrick Constant
 *  Dev : October 2017
 *  Version 1.0
 */
#include "ogm_nlp.h"

static int NlpRequestInterpretationCmp(gconstpointer ptr_request_interpretation1,
    gconstpointer ptr_request_interpretation2, gpointer user_data);
static og_status NlpRequestInterpretationBuild(og_nlp_th ctrl_nlp_th,
    struct request_interpretation *request_interpretation, json_t *json_interpretations);

og_status NlpRequestInterpretationAdd(og_nlp_th ctrl_nlp_th, struct expression *expression, int level)
{
  size_t Irequest_interpretation;
  struct request_interpretation *request_interpretation = OgHeapNewCell(ctrl_nlp_th->hrequest_interpretation,
      &Irequest_interpretation);
  IFn(request_interpretation) DPcErr;
  IF(Irequest_interpretation) DPcErr;
  request_interpretation->expression = expression;
  request_interpretation->level = level;
  DONE;
}

og_status NlpRequestInterpretationsExplicit(og_nlp_th ctrl_nlp_th)
{
  int request_interpretation_used = OgHeapGetCellsUsed(ctrl_nlp_th->hrequest_interpretation);
  struct request_interpretation *request_interpretation = OgHeapGetCell(ctrl_nlp_th->hrequest_interpretation, 0);
  IFN(request_interpretation) DPcErr;

  g_qsort_with_data(request_interpretation, request_interpretation_used, sizeof(struct request_interpretation),
      NlpRequestInterpretationCmp, NULL);

  DONE;
}

static int NlpRequestInterpretationCmp(gconstpointer ptr_request_interpretation1,
    gconstpointer ptr_request_interpretation2, gpointer user_data)
{
  struct request_interpretation *request_interpretation1 = (struct request_interpretation *) ptr_request_interpretation1;
  struct request_interpretation *request_interpretation2 = (struct request_interpretation *) ptr_request_interpretation2;

  if (request_interpretation1->level != request_interpretation2->level)
  {
    return (request_interpretation2->level - request_interpretation1->level);
  }
  // Just to make sure it is
  return request_interpretation2 - request_interpretation1;
}

/*
 * For the moment, we build all the top level interpretations
 */
og_status NlpRequestInterpretationsBuild(og_nlp_th ctrl_nlp_th, json_t *json_interpretations)
{
  int request_interpretation_used = OgHeapGetCellsUsed(ctrl_nlp_th->hrequest_interpretation);

  struct request_interpretation *request_interpretation = OgHeapGetCell(ctrl_nlp_th->hrequest_interpretation, 0);
  IFN(request_interpretation) DPcErr;

  int top_level = request_interpretation->level;

  for (int i = 0; i < request_interpretation_used; i++)
  {
    if (request_interpretation[i].level != top_level) break;
    IFE(NlpRequestInterpretationBuild(ctrl_nlp_th, request_interpretation + i, json_interpretations));
  }

  DONE;
}

static og_status NlpRequestInterpretationBuild(og_nlp_th ctrl_nlp_th,
    struct request_interpretation *request_interpretation, json_t *json_interpretations)
{
  struct expression *expression = request_interpretation->expression;
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

og_status NlpRequestInterpretationsLog(og_nlp_th ctrl_nlp_th)
{
  int request_interpretation_used = OgHeapGetCellsUsed(ctrl_nlp_th->hrequest_interpretation);

  OgMsg(ctrl_nlp_th->hmsg, "", DOgMsgDestInLog, "List of found expressions:");

  for (int i = 0; i < request_interpretation_used; i++)
  {
    struct request_interpretation *request_interpretation = OgHeapGetCell(ctrl_nlp_th->hrequest_interpretation, i);
    IFN(request_interpretation) DPcErr;

    IFE(NlpRequestInterpretationLog(ctrl_nlp_th, request_interpretation));
  }
  DONE;
}

og_status NlpRequestInterpretationLog(og_nlp_th ctrl_nlp_th, struct request_interpretation *request_interpretation)
{
  IFN(request_interpretation) DPcErr;

  struct expression *expression = request_interpretation->expression;

  OgMsg(ctrl_nlp_th->hmsg, "", DOgMsgDestInLog, "  %2d: '%.*s' in interpretation '%s' '%s'",
      request_interpretation->level, DPcPathSize, expression->text, expression->interpretation->slug,
      expression->interpretation->id);
  DONE;
}
