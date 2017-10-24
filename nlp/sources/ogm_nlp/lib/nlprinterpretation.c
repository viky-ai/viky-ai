/*
 *  Handling request input parts
 *  Copyright (c) 2017 Pertimm, by Patrick Constant
 *  Dev : October 2017
 *  Version 1.0
 */
#include "ogm_nlp.h"

static og_status NlpRequestInterpretationExplicit(og_nlp_th ctrl_nlp_th, int Irequest_interpretation);
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

  for (int i = 0; i < request_interpretation_used; i++)
  {
    IFE(NlpRequestInterpretationExplicit(ctrl_nlp_th, i));
  }

  struct request_interpretation *request_interpretation = OgHeapGetCell(ctrl_nlp_th->hrequest_interpretation, 0);
  IFN(request_interpretation) DPcErr;
  g_qsort_with_data(request_interpretation, request_interpretation_used, sizeof(struct request_interpretation),
      NlpRequestInterpretationCmp, NULL);

  DONE;
}

static og_status NlpRequestInterpretationExplicit(og_nlp_th ctrl_nlp_th, int Irequest_interpretation)
{
  struct request_interpretation *request_interpretation = OgHeapGetCell(ctrl_nlp_th->hrequest_interpretation,
      Irequest_interpretation);
  IFN(request_interpretation) DPcErr;
  struct expression *expression = request_interpretation->expression;

  size_t request_input_part_used = OgHeapGetCellsUsed(ctrl_nlp_th->hrequest_input_part);
  struct request_input_part *request_input_part = OgHeapGetCell(ctrl_nlp_th->hrequest_input_part, 0);
  IFN(request_input_part) DPcErr;

  for (size_t i = 0; i < request_input_part_used; i++)
  {
    struct interpret_package *interpret_package = OgHeapGetCell(ctrl_nlp_th->hinterpret_package,
        request_input_part[i].Iinterpret_package);
    IFN(interpret_package) DPcErr;
    if (interpret_package->package != expression->interpretation->package) continue;
    if (request_input_part[i].Iinput_part != expression->input_part_start) continue;
    for (int j = 1; j < expression->input_parts_nb; j++)
    {
      if (expression->input_part_start + j != request_input_part[i + j].Iinput_part)
      {
        NlpThrowErrorTh(ctrl_nlp_th, "NlpRequestInterpretationExplicit: incompatible expression");
        DPcErr;
      }
    }
    request_interpretation->request_input_part_start = i;
    request_interpretation->request_input_parts_nb = expression->input_parts_nb;
    break;
  }

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
  return (request_interpretation1->request_input_part_start - request_interpretation2->request_input_part_start);
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

  char *interpretation_id = OgHeapGetCell(package->hba, interpretation->id_start);
  IFN(interpretation_id) DPcErr;

  char *interpretation_slug = OgHeapGetCell(package->hba, interpretation->slug_start);
  IFN(interpretation_slug) DPcErr;

  og_string package_id = OgHeapGetCell(package->hba, package->id_start);
  if (ctrl_nlp_th->loginfo->trace & DOgNlpTraceCompile)
  {
    OgMsg(ctrl_nlp_th->hmsg, "", DOgMsgDestInLog,
        "NlpInterpretRequestInterpretation: found interpretation '%s' '%s' in package '%s'", interpretation_slug,
        interpretation_id, package_id);
  }
  json_t *json_interpretation = json_object();

  json_t *json_package_id = json_string(package_id);
  IF(json_object_set_new(json_interpretation, "package", json_package_id))
  {
    NlpThrowErrorTh(ctrl_nlp_th, "NlpInterpretRequestInterpretation: error setting json_package_id");
    DPcErr;
  }

  json_t *json_interpretation_id = json_string(interpretation_id);
  IF(json_object_set_new(json_interpretation, "id", json_interpretation_id))
  {
    NlpThrowErrorTh(ctrl_nlp_th, "NlpInterpretRequestInterpretation: error setting json_interpretation_id");
    DPcErr;
  }

  json_t *json_interpretation_slug = json_string(interpretation_slug);
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
    IFE(NlpRequestInterpretationLog(ctrl_nlp_th, i));
  }
  DONE;
}

og_status NlpRequestInterpretationLog(og_nlp_th ctrl_nlp_th, int Irequest_interpretation)
{
  struct request_interpretation *request_interpretation = OgHeapGetCell(ctrl_nlp_th->hrequest_interpretation,
      Irequest_interpretation);
  IFN(request_interpretation) DPcErr;

  struct expression *expression = request_interpretation->expression;
  og_string text = OgHeapGetCell(expression->interpretation->package->hba, expression->text_start);

  OgMsg(ctrl_nlp_th->hmsg, "", DOgMsgDestInLog, "  %2d: '%.*s' with input_parts %d:%d in request_input_parts %d:%d",
      request_interpretation->level, DPcPathSize, text, expression->input_part_start, expression->input_parts_nb,
      request_interpretation->request_input_part_start, request_interpretation->request_input_parts_nb);
  DONE;
}
