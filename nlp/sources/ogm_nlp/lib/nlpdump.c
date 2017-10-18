/*
 * nlpdump.c
 *
 *  Created on: 26 sept. 2017
 *      Author: sebastien
 */
#include "ogm_nlp.h"

static gint str_compar(gconstpointer a, gconstpointer b)
{
  return strcmp((const char*) a, (const char*) b);
}

PUBLIC(int) OgNlpDump(og_nlp_th ctrl_nlp_th, struct og_nlp_dump_input *input, struct og_nlp_dump_output *output)
{
  // reset output
  memset(output, 0, sizeof(struct og_nlp_dump_output));

  ctrl_nlp_th->json_answer = json_array();
  json_t *json_packages = ctrl_nlp_th->json_answer;

  GList *key_list = g_hash_table_get_keys(ctrl_nlp_th->ctrl_nlp->packages_hash);

  GList *sorted_key_kist = g_list_sort(key_list, str_compar);

  for (GList *iter = sorted_key_kist; iter; iter = iter->next)
  {
    json_t *json_package = json_object();
    IF(json_array_append_new(json_packages, json_package))
    {
      NlpThrowErrorTh(ctrl_nlp_th, "NlpPackageDump : Error while dumping package");
      DPcErr;
    }
    og_string package_id = iter->data;

    package_t package = NlpPackageGet(ctrl_nlp_th, package_id);
    IFN(package) DONE;

    IFE(NlpPackageLog(ctrl_nlp_th, package));

    IFE(NlpPackageDump(ctrl_nlp_th, package, json_package));

  }

  output->json_output = json_packages;

  DONE;
}

og_status NlpPackageDump(og_nlp_th ctrl_nlp_th, package_t package, json_t *json_package)
{
  const char *package_id = OgHeapGetCell(package->hba, package->id_start);
  IFN(package_id) DPcErr;

  OgMsg(ctrl_nlp_th->hmsg, "", DOgMsgDestInLog, "Package '%s' :", package_id);

  json_t *json_package_dump = json_string(package_id);

  IF(json_object_set_new(json_package, "id", json_package_dump))
  {
    NlpThrowErrorTh(ctrl_nlp_th, "NlpPackageDump : Error while dumping package %s", package_id);
    DPcErr;
  }

  int interpretation_used = OgHeapGetCellsUsed(package->hinterpretation);
  if (interpretation_used > 0)
  {
    json_t *json_interpretations = json_array();
    IF(json_object_set_new(json_package, "interpretations", json_interpretations))
    {
      NlpThrowErrorTh(ctrl_nlp_th, "NlpPackageDump : Error while dumping interpretations");
      DPcErr;
    }

    for (int i = 0; i < interpretation_used; i++)
    {
      IFE(NlpPackageInterpretationDump(ctrl_nlp_th, package, i, json_interpretations));
    }

  }

  DONE;
}

og_status NlpPackageInterpretationDump(og_nlp_th ctrl_nlp_th, package_t package, int Iinterpretation, json_t *json_interpretations)
{
  struct interpretation *interpretation = OgHeapGetCell(package->hinterpretation, Iinterpretation);
  IFN(interpretation) DPcErr;
  char *interpretation_id = OgHeapGetCell(package->hba, interpretation->id_start);
  IFN(interpretation_id) DPcErr;

  OgMsg(ctrl_nlp_th->hmsg, "", DOgMsgDestInLog, "  Interpretation '%s' :", interpretation_id);

  json_t *json_interpretation = json_object();
  IF(json_array_append_new(json_interpretations, json_interpretation))
  {
    NlpThrowErrorTh(ctrl_nlp_th, "NlpPackageInterpretationDump : Error while dumping interpretation");
    DPcErr;
  }

  json_t *json_interpretation_id = json_string(interpretation_id);
  IF(json_object_set_new(json_interpretation, "id", json_interpretation_id))
  {
    NlpThrowErrorTh(ctrl_nlp_th, "NlpPackageInterpretationDump : Error while dumping interpretation ID %s", interpretation_id);
    DPcErr;
  }

  if (interpretation->expressions_nb > 0)
  {
    json_t *json_expressions = json_array();
    IF(json_object_set_new(json_interpretation, "expressions", json_expressions))
    {
      NlpThrowErrorTh(ctrl_nlp_th, "NlpPackageInterpretationDump : Error while dumping expressions");
      DPcErr;
    }

    for (int i = 0; i < interpretation->expressions_nb; i++)
    {
      IFE(NlpPackageExpressionDump(ctrl_nlp_th, package, interpretation->expression_start + i, json_expressions));
    }

  }

  DONE;
}

og_status NlpPackageExpressionDump(og_nlp_th ctrl_nlp_th, package_t package, int Iexpression, json_t *json_expressions)

{
  struct expression *expression = OgHeapGetCell(package->hexpression, Iexpression);
  IFN(expression) DPcErr;

  json_t *json_expression = json_object();
  IF(json_array_append_new(json_expressions, json_expression))
  {
    NlpThrowErrorTh(ctrl_nlp_th, "NlpPackageExpressionDump : Error while dumping expression");
    DPcErr;
  }
  og_string text = OgHeapGetCell(package->hba, expression->text_start);
  json_t *json_text = json_string(text);

  IF(json_object_set_new(json_expression, "expression", json_text))
  {
    NlpThrowErrorTh(ctrl_nlp_th, "NlpPackageExpressionDump : Error while dumping expression");
    DPcErr;
  }

  unsigned char string_locale[DPcPathSize];
  OgIso639_3166ToCode(expression->locale, string_locale);

  OgMsg(ctrl_nlp_th->hmsg, "", DOgMsgDestInLog, "    Phrase '%s' with locale %s", text, string_locale);

  json_t *json_locale = json_string(string_locale);
  IF(json_object_set_new(json_expression, "locale", json_locale))
  {
    NlpThrowErrorTh(ctrl_nlp_th, "NlpPackageExpressionDump : Error while dumping expression locale", string_locale);
    DPcErr;
  }

  DONE;
}

