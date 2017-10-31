/*
 * nlpdump.c
 *
 *  Created on: 26 sept. 2017
 *      Author: sebastien
 */
#include "ogm_nlp.h"

static og_status NlpPackageDump(og_nlp_th ctrl_nlp_th, package_t package, json_t *json_package);
static og_status NlpPackageInterpretationDump(og_nlp_th ctrl_nlp_th, package_t package, int Iinterpretation, json_t *dump_json);
static og_status NlpPackageExpressionDump(og_nlp_th ctrl_nlp_th, package_t package, int Iexpression, json_t *dump_json);
static og_status NlpPackageAliasDump(og_nlp_th ctrl_nlp_th, package_t package, int Ialias, json_t *json_aliases);
static og_status NlpDumpNoSync(og_nlp_th ctrl_nlp_th, struct og_nlp_dump_input *input, struct og_nlp_dump_output *output);

static gint str_compar(gconstpointer a, gconstpointer b)
{
  return strcmp((const char*) a, (const char*) b);
}

PUBLIC(int) OgNlpDump(og_nlp_th ctrl_nlp_th, struct og_nlp_dump_input *input, struct og_nlp_dump_output *output)
{
  // reset output
  memset(output, 0, sizeof(struct og_nlp_dump_output));
  ctrl_nlp_th->json_answer = json_array();

  IF(OgNlpSynchroReadLock(ctrl_nlp_th, ctrl_nlp_th->ctrl_nlp->rw_lock_packages_hash))
  {
    NlpThrowErrorTh(ctrl_nlp_th, "OgNlpDump: error on OgNlpSynchroReadLock");
    DPcErr;
  }

  og_status dump_status = NlpDumpNoSync(ctrl_nlp_th, input, output);

  IF(OgNlpSynchroReadUnLock(ctrl_nlp_th, ctrl_nlp_th->ctrl_nlp->rw_lock_packages_hash))
  {
    NlpThrowErrorTh(ctrl_nlp_th, "OgNlpDump: error on OgNlpSynchroReadUnLock");
    DPcErr;
  }

  output->json_output = ctrl_nlp_th->json_answer;

  return dump_status;
}

static og_status NlpDumpNoSyncFreeSafe(og_nlp_th ctrl_nlp_th, GList *sorted_key_kist)
{
  json_t *json_packages = ctrl_nlp_th->json_answer;

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

    og_status dump_status = NlpPackageDump(ctrl_nlp_th, package, json_package);

    NlpPackageMarkAsUnused(ctrl_nlp_th, package);

    IF(dump_status)
    {
      NlpThrowErrorTh(ctrl_nlp_th, "NlpPackageDump : NlpPackageDump failed on package '%s'", package_id);
      DPcErr;
    }

  }

  DONE;
}

static og_status NlpDumpNoSync(og_nlp_th ctrl_nlp_th, struct og_nlp_dump_input *input, struct og_nlp_dump_output *output)
{

  GList *key_list = g_hash_table_get_keys(ctrl_nlp_th->ctrl_nlp->packages_hash);

  GList *sorted_key_list = g_list_sort(key_list, str_compar);

  og_status dump_status = NlpDumpNoSyncFreeSafe(ctrl_nlp_th, sorted_key_list);

  g_list_free(sorted_key_list);

  return dump_status;
}

static og_status NlpPackageDump(og_nlp_th ctrl_nlp_th, package_t package, json_t *json_package)
{
  IFE(NlpPackageLog(ctrl_nlp_th, package));

  const char *package_id = OgHeapGetCell(package->hba, package->id_start);
  IFN(package_id) DPcErr;
  OgMsg(ctrl_nlp_th->hmsg, "", DOgMsgDestInLog, "Package id '%s' :", package_id);

  const char *package_slug = OgHeapGetCell(package->hba, package->slug_start);
  IFN(package_slug) DPcErr;
  OgMsg(ctrl_nlp_th->hmsg, "", DOgMsgDestInLog, "Package slug '%s' :", package_slug);

  json_t *json_package_id = json_string(package_id);
  IF(json_object_set_new(json_package, "id", json_package_id))
  {
    NlpThrowErrorTh(ctrl_nlp_th, "NlpPackageDump : Error while dumping package id '%s'", package_id);
    DPcErr;
  }

  json_t *json_package_slug = json_string(package_slug);
  IF(json_object_set_new(json_package, "slug", json_package_slug))
  {
    NlpThrowErrorTh(ctrl_nlp_th, "NlpPackageDump : Error while dumping package slug '%s'", package_id);
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
  OgMsg(ctrl_nlp_th->hmsg, "", DOgMsgDestInLog, "  Interpretation id '%s' :", interpretation_id);

  char *interpretation_slug = OgHeapGetCell(package->hba, interpretation->slug_start);
  IFN(interpretation_slug) DPcErr;
  OgMsg(ctrl_nlp_th->hmsg, "", DOgMsgDestInLog, "  Interpretation slug '%s' :", interpretation_slug);

  json_t *json_interpretation = json_object();
  IF(json_array_append_new(json_interpretations, json_interpretation))
  {
    NlpThrowErrorTh(ctrl_nlp_th, "NlpPackageInterpretationDump : Error while dumping interpretation");
    DPcErr;
  }

  json_t *json_interpretation_id = json_string(interpretation_id);
  IF(json_object_set_new(json_interpretation, "id", json_interpretation_id))
  {
    NlpThrowErrorTh(ctrl_nlp_th, "NlpPackageInterpretationDump : Error while dumping interpretation id %s", interpretation_id);
    DPcErr;
  }

  json_t *json_interpretation_slug = json_string(interpretation_slug);
  IF(json_object_set_new(json_interpretation, "slug", json_interpretation_slug))
  {
    NlpThrowErrorTh(ctrl_nlp_th, "NlpPackageInterpretationDump : Error while dumping interpretation slug %s", interpretation_slug);
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

  if (expression->aliases_nb > 0)
  {
    json_t *json_aliases = json_array();
    IF(json_object_set_new(json_expression, "aliases", json_aliases))
    {
      NlpThrowErrorTh(ctrl_nlp_th, "NlpPackageInterpretationDump : Error while dumping aliases");
      DPcErr;
    }

    for (int i = 0; i < expression->aliases_nb; i++)
    {
      IFE(NlpPackageAliasDump(ctrl_nlp_th, package, expression->alias_start + i, json_aliases));
    }

  }

  DONE;
}

og_status NlpPackageAliasDump(og_nlp_th ctrl_nlp_th, package_t package, int Ialias, json_t *json_aliases)
{
  struct alias *alias = OgHeapGetCell(package->halias, Ialias);
  IFN(alias) DPcErr;

  json_t *json_alias = json_object();
  IF(json_array_append_new(json_aliases, json_alias))
  {
    NlpThrowErrorTh(ctrl_nlp_th, "NlpPackageExpressionDump : Error while dumping alias");
    DPcErr;
  }
  og_string string_alias = OgHeapGetCell(package->hba, alias->alias_start);
  json_t *json_alias_name = json_string(string_alias);
  IF(json_object_set_new(json_alias, "alias", json_alias_name))
  {
    NlpThrowErrorTh(ctrl_nlp_th, "NlpPackageExpressionDump : Error while dumping alias name");
    DPcErr;
  }

  og_string string_slug = OgHeapGetCell(package->hba, alias->slug_start);
  json_t *json_slug = json_string(string_slug);
  IF(json_object_set_new(json_alias, "slug", json_slug))
  {
    NlpThrowErrorTh(ctrl_nlp_th, "NlpPackageExpressionDump : Error while dumping alias slug");
    DPcErr;
  }

  og_string string_id = OgHeapGetCell(package->hba, alias->id_start);
  json_t *json_id = json_string(string_id);
  IF(json_object_set_new(json_alias, "id", json_id))
  {
    NlpThrowErrorTh(ctrl_nlp_th, "NlpPackageExpressionDump : Error while dumping alias id");
    DPcErr;
  }

  og_string string_package = OgHeapGetCell(package->hba, alias->package_start);
  json_t *json_package = json_string(string_package);
  IF(json_object_set_new(json_alias, "package", json_package))
  {
    NlpThrowErrorTh(ctrl_nlp_th, "NlpPackageExpressionDump : Error while dumping alias package");
    DPcErr;
  }

  DONE;
}

