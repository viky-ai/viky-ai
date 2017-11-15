/*
 * nlpdump.c
 *
 *  Created on: 26 sept. 2017
 *      Author: sebastien
 */
#include "ogm_nlp.h"

static og_status NlpPackageDump(og_nlp_th ctrl_nlp_th, package_t package, json_t *json_package);
static og_status NlpPackageInterpretationDump(og_nlp_th ctrl_nlp_th, package_t package,
    struct interpretation *interpretation, json_t *json_interpretations);
static og_status NlpPackageExpressionDump(og_nlp_th ctrl_nlp_th, package_t package, struct expression *expression,
    json_t *json_expressions);
static og_status NlpPackageAliasDump(og_nlp_th ctrl_nlp_th, package_t package, struct alias *alias,
    json_t *json_aliases);
static og_status NlpDumpNoSync(og_nlp_th ctrl_nlp_th, struct og_nlp_dump_input *input,
    struct og_nlp_dump_output *output);

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

static og_status NlpDumpNoSync(og_nlp_th ctrl_nlp_th, struct og_nlp_dump_input *input,
    struct og_nlp_dump_output *output)
{

  GList *key_list = g_hash_table_get_keys(ctrl_nlp_th->ctrl_nlp->packages_hash);

  GList *sorted_key_list = g_list_sort(key_list, str_compar);

  og_status dump_status = NlpDumpNoSyncFreeSafe(ctrl_nlp_th, sorted_key_list);

  g_list_free(sorted_key_list);

  return dump_status;
}

static og_status NlpPackageDump(og_nlp_th ctrl_nlp_th, package_t package, json_t *json_package)
{

  if (ctrl_nlp_th->loginfo->trace & DOgNlpTraceDump)
  {
    IFE(NlpPackageLog(ctrl_nlp_th, "dump", package));
  }

  NlpLog(DOgNlpTraceDump, "Package id '%s', slug '%s' :", package->id, package->slug)

  json_t *json_package_id = json_string(package->id);
  IF(json_object_set_new(json_package, "id", json_package_id))
  {
    NlpThrowErrorTh(ctrl_nlp_th, "NlpPackageDump : Error while dumping package id '%s'", package->id);
    DPcErr;
  }

  json_t *json_package_slug = json_string(package->slug);
  IF(json_object_set_new(json_package, "slug", json_package_slug))
  {
    NlpThrowErrorTh(ctrl_nlp_th, "NlpPackageDump : Error while dumping package slug '%s'", package->slug);
    DPcErr;
  }

  int interpretation_used = OgHeapGetCellsUsed(package->hinterpretation);
  if (interpretation_used > 0)
  {
    struct interpretation *all_interpretations = (struct interpretation *) OgHeapGetCell(package->hinterpretation, 0);
    IFN(all_interpretations) DPcErr;

    json_t *json_interpretations = json_array();
    IF(json_object_set_new(json_package, "interpretations", json_interpretations))
    {
      NlpThrowErrorTh(ctrl_nlp_th, "NlpPackageDump : Error while dumping interpretations");
      DPcErr;
    }

    for (int i = 0; i < interpretation_used; i++)
    {
      IFE(NlpPackageInterpretationDump(ctrl_nlp_th, package, all_interpretations + i, json_interpretations));
    }

  }

  DONE;
}

static og_status NlpPackageInterpretationDump(og_nlp_th ctrl_nlp_th, package_t package,
    struct interpretation *interpretation, json_t *json_interpretations)
{
  IFN(interpretation) DPcErr;

  NlpLog(DOgNlpTraceDump, "  Interpretation '%s' '%s' :", interpretation->slug, interpretation->id)

  json_t *json_interpretation = json_object();
  IF(json_array_append_new(json_interpretations, json_interpretation))
  {
    NlpThrowErrorTh(ctrl_nlp_th, "NlpPackageInterpretationDump : Error while dumping interpretation");
    DPcErr;
  }

  json_t *json_interpretation_id = json_string(interpretation->id);
  IF(json_object_set_new(json_interpretation, "id", json_interpretation_id))
  {
    NlpThrowErrorTh(ctrl_nlp_th, "NlpPackageInterpretationDump : Error while dumping interpretation id %s",
        interpretation->id);
    DPcErr;
  }

  json_t *json_interpretation_slug = json_string(interpretation->slug);
  IF(json_object_set_new(json_interpretation, "slug", json_interpretation_slug))
  {
    NlpThrowErrorTh(ctrl_nlp_th, "NlpPackageInterpretationDump : Error while dumping interpretation slug %s",
        interpretation->slug);
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
      IFE(NlpPackageExpressionDump(ctrl_nlp_th, package, interpretation->expressions + i, json_expressions));
    }

  }

  if(interpretation->json_solution)
  {
    IF(json_object_set(json_interpretation, "solution", interpretation->json_solution))
    {
      NlpThrowErrorTh(ctrl_nlp_th, "NlpPackageInterpretationDump : Error while dumping solutions");
      DPcErr;
    }
  }

  DONE;
}

static og_status NlpPackageExpressionDump(og_nlp_th ctrl_nlp_th, package_t package, struct expression *expression,
    json_t *json_expressions)
{
  IFN(expression) DPcErr;

  json_t *json_expression = json_object();
  IF(json_array_append_new(json_expressions, json_expression))
  {
    NlpThrowErrorTh(ctrl_nlp_th, "NlpPackageExpressionDump : Error while dumping expression");
    DPcErr;
  }

  json_t *json_text = json_string(expression->text);
  IF(json_object_set_new(json_expression, "expression", json_text))
  {
    NlpThrowErrorTh(ctrl_nlp_th, "NlpPackageExpressionDump : Error while dumping expression");
    DPcErr;
  }

  og_char_buffer string_locale[DPcPathSize];
  OgIso639_3166ToCode(expression->locale, string_locale);

  if(strcmp(string_locale, "--") != 0)
  {
  NlpLog(DOgNlpTraceDump, "    Expression '%s' with locale %s", expression->text, string_locale)

  json_t *json_locale = json_string(string_locale);
  IF(json_object_set_new(json_expression, "locale", json_locale))
  {
    NlpThrowErrorTh(ctrl_nlp_th, "NlpPackageExpressionDump : Error while dumping expression locale", string_locale);
    DPcErr;
  }
  }

  if (expression->aliases_nb > 0)
  {
    json_t *json_aliases = json_array();
    IF(json_object_set_new(json_expression, "aliases", json_aliases))
    {
      NlpThrowErrorTh(ctrl_nlp_th, "NlpPackageExpressionDump : Error while dumping aliases");
      DPcErr;
    }

    for (int i = 0; i < expression->aliases_nb; i++)
    {
      IFE(NlpPackageAliasDump(ctrl_nlp_th, package, expression->aliases + i, json_aliases));
    }
  }

  if (expression->keep_order)
  {
    json_t *json_keep_order = json_true();
    IF(json_object_set_new(json_expression, "keep-order", json_keep_order))
    {
      NlpThrowErrorTh(ctrl_nlp_th, "NlpPackageExpressionDump : Error while dumping keep_order");
      DPcErr;
  }
  }

  if (expression->glued)
  {
    json_t *json_glued = json_true();
    IF(json_object_set_new(json_expression, "glued", json_glued))
    {
      NlpThrowErrorTh(ctrl_nlp_th, "NlpPackageExpressionDump : Error while dumping glued");
      DPcErr;
    }
  }

  if(expression->json_solution)
  {
    IF(json_object_set(json_expression, "solution", expression->json_solution))
    {
      NlpThrowErrorTh(ctrl_nlp_th, "NlpPackageExpressionDump : Error while dumping solution");
      DPcErr;
    }
  }

  DONE;
}

static og_status NlpPackageAliasDump(og_nlp_th ctrl_nlp_th, package_t package, struct alias *alias,
    json_t *json_aliases)
{
  IFN(alias) DPcErr;

  json_t *json_alias = json_object();
  IF(json_array_append_new(json_aliases, json_alias))
  {
    NlpThrowErrorTh(ctrl_nlp_th, "NlpPackageExpressionDump : Error while dumping alias");
    DPcErr;
  }
  json_t *json_alias_name = json_string(alias->alias);
  IF(json_object_set_new(json_alias, "alias", json_alias_name))
  {
    NlpThrowErrorTh(ctrl_nlp_th, "NlpPackageExpressionDump : Error while dumping alias name '%s'", alias->alias);
    DPcErr;
  }

  if(alias->type == nlp_alias_type_Any)
  {
    json_t *json_type_any = json_string("any");
    IF(json_object_set_new(json_alias, "type", json_type_any))
    {
      NlpThrowErrorTh(ctrl_nlp_th, "NlpPackageExpressionDump : Error while dumping alias type 'any'");
      DPcErr;
    }
  }
  else if(alias->type == nlp_alias_type_Digit)
  {
    json_t *json_type_digit = json_string("digit");
    IF(json_object_set_new(json_alias, "type", json_type_digit))
    {
      NlpThrowErrorTh(ctrl_nlp_th, "NlpPackageExpressionDump : Error while dumping alias type 'digit'");
      DPcErr;
    }
  }
  else if(alias->type == nlp_alias_type_type_Interpretation)
  {
    json_t *json_slug = json_string(alias->slug);
    IF(json_object_set_new(json_alias, "slug", json_slug))
    {
      NlpThrowErrorTh(ctrl_nlp_th, "NlpPackageExpressionDump : Error while dumping alias slug '%s'", alias->slug);
      DPcErr;
    }

    json_t *json_id = json_string(alias->id);
    IF(json_object_set_new(json_alias, "id", json_id))
    {
      NlpThrowErrorTh(ctrl_nlp_th, "NlpPackageExpressionDump : Error while dumping alias id '%s'", alias->id);
      DPcErr;
    }

    json_t *json_package = json_string(alias->package_id);
    IF(json_object_set_new(json_alias, "package", json_package))
    {
      NlpThrowErrorTh(ctrl_nlp_th, "NlpPackageExpressionDump : Error while dumping alias package", alias->package_id);
      DPcErr;
    }
  }

  DONE;
}

