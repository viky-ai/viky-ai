/*
 *  Handling why an expression or an interpretation has not been found
 *  Copyright (c) 2017 Pertimm, by Patrick Constant
 *  Dev : November 2017
 *  Version 1.0
 */
#include "ogm_nlp.h"

static og_status NlpWhyNotMatchingBuildGetPackage(og_nlp_th ctrl_nlp_th, json_t *json_package, package_t *ppackage);
static og_status NlpWhyNotMatchingBuildGetInterpretation(og_nlp_th ctrl_nlp_th, package_t package,
    json_t *json_interpretation, const og_string expression_text);
static og_status NlpWhyNotMatchingBuildGetInterpretationPackage(og_nlp_th ctrl_nlp_th, package_t package,
    og_string interpretation_id, const og_string expression_text);
static og_status NlpWhyNotMatchingBuildGetExpression(og_nlp_th ctrl_nlp_th, package_t package,
    const og_string expression_text);
static og_status NlpWhyNotMatchingBuildGetExpressionPackage(og_nlp_th ctrl_nlp_th, package_t package,
    const og_string expression_text);
static og_status NlpWhyNotMatchingAdd(og_nlp_th ctrl_nlp_th, struct expression *expression);

og_status NlpWhyNotMatchingInit(og_nlp_th ctrl_nlp_th, og_string name)
{
  og_char_buffer nlpc_name[DPcPathSize];

  snprintf(nlpc_name, DPcPathSize, "%s_nm_expression", name);
  ctrl_nlp_th->hnm_expression = OgHeapInit(ctrl_nlp_th->hmsg, nlpc_name, sizeof(struct nm_expression), 1);
  IFN(ctrl_nlp_th->hnm_expression)
  {
    NlpThrowErrorTh(ctrl_nlp_th, "OgNlpInterpretInit : error on OgHeapInit(%s)", nlpc_name);
    DPcErr;
  }

  snprintf(nlpc_name, DPcPathSize, "%s_m_input_part", name);
  ctrl_nlp_th->hm_input_part = OgHeapInit(ctrl_nlp_th->hmsg, nlpc_name, sizeof(struct m_input_part), 1);
  IFN(ctrl_nlp_th->hm_input_part)
  {
    NlpThrowErrorTh(ctrl_nlp_th, "OgNlpInterpretInit : error on OgHeapInit(%s)", nlpc_name);
    DPcErr;
  }

  snprintf(nlpc_name, DPcPathSize, "%s_m_expression", name);
  ctrl_nlp_th->hm_expression = OgHeapInit(ctrl_nlp_th->hmsg, nlpc_name, sizeof(struct m_expression), 1);
  IFN(ctrl_nlp_th->hm_expression)
  {
    NlpThrowErrorTh(ctrl_nlp_th, "OgNlpInterpretInit : error on OgHeapInit(%s)", nlpc_name);
    DPcErr;
  }
  DONE;
}

og_status NlpWhyNotMatchingReset(og_nlp_th ctrl_nlp_th)
{
  IFE(OgHeapReset(ctrl_nlp_th->hnm_expression));
  IFE(OgHeapReset(ctrl_nlp_th->hm_input_part));
  IFE(OgHeapReset(ctrl_nlp_th->hm_expression));
  DONE;
}

og_status NlpWhyNotMatchingFlush(og_nlp_th ctrl_nlp_th)
{
  IFE(OgHeapFlush(ctrl_nlp_th->hnm_expression));
  IFE(OgHeapFlush(ctrl_nlp_th->hm_input_part));
  IFE(OgHeapFlush(ctrl_nlp_th->hm_expression));

  DONE;
}

og_status NlpWhyNotMatchingBuild(og_nlp_th ctrl_nlp_th, json_t *json_why_not_matching)
{
  IFN(json_why_not_matching) DONE;

  if (!json_is_object(json_why_not_matching))
  {
    NlpThrowErrorTh(ctrl_nlp_th, "NlpWhyNotMatchingBuild: json_why_not_matching is not an object");
    DPcErr;
  }

  json_t *json_package = NULL;
  json_t *json_interpretation = NULL;
  json_t *json_expression = NULL;

  for (void *iter = json_object_iter(json_why_not_matching); iter;
      iter = json_object_iter_next(json_why_not_matching, iter))
  {
    og_string key = json_object_iter_key(iter);

    NlpLog(DOgNlpTraceInterpret, "NlpWhyNotMatchingBuild: found key='%s'", key)

    if (Ogstricmp(key, "package") == 0)
    {
      json_package = json_object_iter_value(iter);
    }
    else if (Ogstricmp(key, "interpretation") == 0)
    {
      json_interpretation = json_object_iter_value(iter);
    }
    else if (Ogstricmp(key, "expression") == 0)
    {
      json_expression = json_object_iter_value(iter);
    }
    else
    {
      NlpThrowErrorTh(ctrl_nlp_th, "NlpWhyNotMatchingBuild: unknow key '%s'", key);
      DPcErr;
    }
  }
  package_t package = NULL;
  IFX(json_package)
  {
    IFE(NlpWhyNotMatchingBuildGetPackage(ctrl_nlp_th, json_package, &package));
  }

  og_string expression_text = NULL;
  IFX(json_expression)
  {
    if (!json_is_string(json_expression))
    {
      NlpThrowErrorTh(ctrl_nlp_th, "NlpWhyNotMatchingBuildGetPackage: json_expression is not a string");
      DPcErr;
    }
    expression_text = json_string_value(json_expression);
  }

  IFX(json_interpretation)
  {
    IFE(NlpWhyNotMatchingBuildGetInterpretation(ctrl_nlp_th, package, json_interpretation, expression_text));
  }
  else
  {
    IFE(NlpWhyNotMatchingBuildGetExpression(ctrl_nlp_th, package, expression_text));
  }

  IFE(NlpWhyNotMatchingLog(ctrl_nlp_th));

  DONE;
}

static og_status NlpWhyNotMatchingBuildGetPackage(og_nlp_th ctrl_nlp_th, json_t *json_package, package_t *ppackage)
{
  *ppackage = NULL;
  if (!json_is_string(json_package))
  {
    NlpThrowErrorTh(ctrl_nlp_th, "NlpWhyNotMatchingBuildGetPackage: package is not a string");
    DPcErr;
  }
  const char *package_id = json_string_value(json_package);
  package_t package = NlpPackageGet(ctrl_nlp_th, package_id);
  IFN(package)
  {
    NlpThrowErrorTh(ctrl_nlp_th, "NlpWhyNotMatchingBuildGetPackage: unknown package '%s'", package_id);
    DPcErr;
  }
  int found_request_package = FALSE;
  int package_used = OgHeapGetCellsUsed(ctrl_nlp_th->hinterpret_package);
  struct interpret_package *interpret_packages = OgHeapGetCell(ctrl_nlp_th->hinterpret_package, 0);
  for (int i = 0; i < package_used; i++)
  {
    if (interpret_packages[i].package != package) continue;
    found_request_package = TRUE;
    break;
  }
  if (!found_request_package)
  {
    NlpThrowErrorTh(ctrl_nlp_th, "NlpWhyNotMatchingBuildGetPackage: package '%s' is not in the request", package_id);
    DPcErr;
  }
  *ppackage = package;
  DONE;
}

static og_status NlpWhyNotMatchingBuildGetInterpretation(og_nlp_th ctrl_nlp_th, package_t package,
    json_t *json_interpretation, const og_string expression_text)
{
  if (!json_is_string(json_interpretation))
  {
    NlpThrowErrorTh(ctrl_nlp_th, "NlpWhyNotMatchingBuildGetPackage: interpretation is not a string");
    DPcErr;
  }
  og_string interpretation_id = json_string_value(json_interpretation);

  IFX(package)
  {
    IFE(NlpWhyNotMatchingBuildGetInterpretationPackage(ctrl_nlp_th, package, interpretation_id, expression_text));
  }
  else
  {
    int interpret_package_used = OgHeapGetCellsUsed(ctrl_nlp_th->hinterpret_package);
    struct interpret_package *interpret_packages = OgHeapGetCell(ctrl_nlp_th->hinterpret_package, 0);
    IFN(interpret_packages) DPcErr;
    for (int i = 0; i < interpret_package_used; i++)
    {
      struct interpret_package *interpret_package = interpret_packages + i;
      IFE(
          NlpWhyNotMatchingBuildGetInterpretationPackage(ctrl_nlp_th, interpret_package->package, interpretation_id,
              expression_text));
    }
  }

  DONE;
}

static og_status NlpWhyNotMatchingBuildGetInterpretationPackage(og_nlp_th ctrl_nlp_th, package_t package,
    og_string interpretation_id, const og_string expression_text)
{
  int interpretation_used = OgHeapGetCellsUsed(package->hinterpretation);
  struct interpretation *interpretations = OgHeapGetCell(package->hinterpretation, 0);
  IFN(interpretations) DPcErr;
  struct interpretation *found_interpretation = NULL;
  for (int i = 0; i < interpretation_used; i++)
  {
    struct interpretation *interpretation = interpretations + i;
    if (strcmp(interpretation->id, interpretation_id)) continue;
    found_interpretation = interpretation;
    break;
  }

  IFN(found_interpretation) DONE;

  for (int i = 0; i < found_interpretation->expressions_nb; i++)
  {
    struct expression *expression = found_interpretation->expressions + i;
    if (expression_text)
    {
      if (!strcmp(expression->text, expression_text))
      {
        IFE(NlpWhyNotMatchingAdd(ctrl_nlp_th, expression));
      }
    }
    else
    {
      IFE(NlpWhyNotMatchingAdd(ctrl_nlp_th, expression));
    }
  }

  DONE;
}

static og_status NlpWhyNotMatchingBuildGetExpression(og_nlp_th ctrl_nlp_th, package_t package,
    const og_string expression_text)
{
  IFN(expression_text) DONE;

  IFX(package)
  {
    IFE(NlpWhyNotMatchingBuildGetExpressionPackage(ctrl_nlp_th, package, expression_text));
  }
  else
  {
    int interpret_package_used = OgHeapGetCellsUsed(ctrl_nlp_th->hinterpret_package);
    struct interpret_package *interpret_packages = OgHeapGetCell(ctrl_nlp_th->hinterpret_package, 0);
    IFN(interpret_packages) DPcErr;
    for (int i = 0; i < interpret_package_used; i++)
    {
      struct interpret_package *interpret_package = interpret_packages + i;
      IFE(NlpWhyNotMatchingBuildGetExpressionPackage(ctrl_nlp_th, interpret_package->package, expression_text));
    }
  }

  DONE;
}

static og_status NlpWhyNotMatchingBuildGetExpressionPackage(og_nlp_th ctrl_nlp_th, package_t package,
    const og_string expression_text)
{

  int interpretation_used = OgHeapGetCellsUsed(package->hinterpretation);
  struct interpretation *interpretations = OgHeapGetCell(package->hinterpretation, 0);
  IFN(interpretations) DPcErr;
  for (int i = 0; i < interpretation_used; i++)
  {
    struct interpretation *interpretation = interpretations + i;
    for (int j = 0; j < interpretation->expressions_nb; j++)
    {
      struct expression *expression = interpretation->expressions + j;
      if (!strcmp(expression->text, expression_text))
      {
        IFE(NlpWhyNotMatchingAdd(ctrl_nlp_th, expression));
      }
    }
  }
  DONE;
}

static og_status NlpWhyNotMatchingAdd(og_nlp_th ctrl_nlp_th, struct expression *expression)
{

  struct nm_expression nm_expression[1];
  memset(nm_expression, 0, sizeof(struct nm_expression));
  nm_expression->expression = expression;
  nm_expression->m_input_part_start = (-1);

  int nm_expression_used = OgHeapGetCellsUsed(ctrl_nlp_th->hnm_expression);
  IFE(OgHeapAppend(ctrl_nlp_th->hnm_expression, 1, nm_expression));
  struct nm_expression *nm_expression_verif = OgHeapGetCell(ctrl_nlp_th->hnm_expression, nm_expression_used);
  NlpLog(DOgNlpTraceInterpret, "NlpWhyNotMatchingAdd adding Inm_expression=%d m_input_part_start=%d=%d: ",
      nm_expression_used, nm_expression->m_input_part_start, nm_expression_verif->m_input_part_start)

  DONE;
}

og_status NlpWhyNotMatchingLog(og_nlp_th ctrl_nlp_th)
{
  int nm_expression_used = OgHeapGetCellsUsed(ctrl_nlp_th->hnm_expression);
  struct nm_expression *nm_expressions = OgHeapGetCell(ctrl_nlp_th->hnm_expression, 0);
  IFN(nm_expressions) DPcErr;

  NlpLog(DOgNlpTraceInterpret, "List of all expressions for why-not-match:")

  for (int i = 0; i < nm_expression_used; i++)
  {
    IFE(NlpWhyNotMatchingExpressionLog(ctrl_nlp_th, nm_expressions + i));
  }
  DONE;
}

og_status NlpWhyNotMatchingExpressionLog(og_nlp_th ctrl_nlp_th, struct nm_expression *nm_expression)
{
  NlpPackageExpressionLog(ctrl_nlp_th, nm_expression->expression->interpretation->package, nm_expression->expression);
  struct expression *expression = nm_expression->expression;
  int input_parts_nb = expression->input_parts_nb;

  if (nm_expression->m_input_part_start < 0) DONE;

  struct m_input_part *m_input_parts = OgHeapGetCell(ctrl_nlp_th->hm_input_part, 0);
  struct m_expression *m_expressions = OgHeapGetCell(ctrl_nlp_th->hm_expression, 0);

  for (int i = 0; i < input_parts_nb; i++)
  {
    struct m_input_part *m_input_part = m_input_parts + nm_expression->m_input_part_start + i;
    NlpLog(DOgNlpTraceInterpret, "List of matched expression for input_part %d:", m_input_part->input_part->self_index)
    for (int j = 0; j < m_input_part->m_expressions_nb; j++)
    {
      struct m_expression *m_expression = m_expressions + m_input_part->m_expression_start + j;
      IFE(NlpRequestInputPartLog(ctrl_nlp_th, m_expression->Irequest_input_part));
    }
  }
  DONE;
}

