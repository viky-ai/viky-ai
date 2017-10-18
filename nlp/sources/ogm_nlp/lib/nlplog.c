/*
 *  Handling log messages for Natural Language Processing library
 *  Copyright (c) 2017 Pertimm, by Patrick Constant
 *  Dev : September 2017
 *  Version 1.0
 */
#include "ogm_nlp.h"

/**
 * Write json_t in buffer. Buffer is always NUL terminated.
 *
 * @param json json_t pointer
 * @param buffer buffer to write json in
 * @param buffer_size buffer size
 * @param p_truncated flag to indicate if json is truncated or not
 * @return status
 */
og_status NlpJsonToBuffer(const json_t *json, og_char_buffer *buffer, int buffer_size, og_bool *p_truncated)
{
  og_string truncated_ends = " ... (truncated) ";
  int truncated_ends_size = strlen(truncated_ends);

  int expected_size = json_dumpb(json, buffer, buffer_size - truncated_ends_size - 1, JSON_INDENT(2));
  IF(expected_size)
  {
    DPcErr;
  }

  // truncated json
  if (expected_size >= (buffer_size - truncated_ends_size - 1))
  {
    if (p_truncated) *p_truncated = TRUE;

    if (truncated_ends_size > buffer_size)
    {
      snprintf(buffer + buffer_size - truncated_ends_size - 1, truncated_ends_size, "%s", truncated_ends);
    }

    DONE;
  }

  buffer[expected_size - 1] = 0;

  if (p_truncated) *p_truncated = TRUE;

  DONE;
}

og_status NlpPackageLog(og_nlp_th ctrl_nlp_th, package_t package)
{
  og_string package_id = OgHeapGetCell(package->hba, package->id_start);
  IFN(package_id) DPcErr;
  og_string package_slug = OgHeapGetCell(package->hba, package->slug_start);
  IFN(package_slug) DPcErr;

  OgMsg(ctrl_nlp_th->hmsg, "", DOgMsgDestInLog, "Package '%s' '%s':", package_slug, package_id);

  int interpretation_used = OgHeapGetCellsUsed(package->hinterpretation);
  for (int i = 0; i < interpretation_used; i++)
  {
    IFE(NlpPackageInterpretationLog(ctrl_nlp_th, package, i));
  }
  DONE;
}

og_status NlpPackageInterpretationLog(og_nlp_th ctrl_nlp_th, package_t package, int Iinterpretation)
{
  struct interpretation *interpretation = OgHeapGetCell(package->hinterpretation, Iinterpretation);
  IFN(interpretation) DPcErr;

  og_string interpretation_id = OgHeapGetCell(package->hba, interpretation->id_start);
  IFN(interpretation_id) DPcErr;

  og_string interpretation_slug = OgHeapGetCell(package->hba, interpretation->slug_start);
  IFN(interpretation_slug) DPcErr;

  OgMsg(ctrl_nlp_th->hmsg, "", DOgMsgDestInLog, "  Interpretation '%s' '%s':", interpretation_slug, interpretation_id);

  for (int i = 0; i < interpretation->expressions_nb; i++)
  {
    IFE(NlpPackageExpressionLog(ctrl_nlp_th, package, interpretation->expression_start + i));
  }
  DONE;
}

og_status NlpPackageExpressionLog(og_nlp_th ctrl_nlp_th, package_t package, int Iexpression)

{
  struct expression *expression = OgHeapGetCell(package->hexpression, Iexpression);
  IFN(expression) DPcErr;

  og_string text = OgHeapGetCell(package->hba, expression->text_start);

  unsigned char string_locale[DPcPathSize];
  OgIso639_3166ToCode(expression->locale, string_locale);

  OgMsg(ctrl_nlp_th->hmsg, "", DOgMsgDestInLog, "    Phrase '%s' with locale %s", text, string_locale);
  for (int i = 0; i < expression->aliases_nb; i++)
  {
    IFE(NlpPackageAliasLog(ctrl_nlp_th, package, expression->alias_start + i));
  }

  DONE;
}

og_status NlpPackageAliasLog(og_nlp_th ctrl_nlp_th, package_t package, int Ialias)

{
  struct alias *alias = OgHeapGetCell(package->halias, Ialias);
  IFN(alias) DPcErr;

  og_string string_alias = OgHeapGetCell(package->hba, alias->alias_start);
  og_string string_slug = OgHeapGetCell(package->hba, alias->slug_start);
  og_string string_id = OgHeapGetCell(package->hba, alias->id_start);
  og_string string_package = OgHeapGetCell(package->hba, alias->package_start);
  OgMsg(ctrl_nlp_th->hmsg, "", DOgMsgDestInLog, "      alias '%s' '%s' '%s' '%s'", string_alias, string_slug, string_id, string_package);

  DONE;
}

