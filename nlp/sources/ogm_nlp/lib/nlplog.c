/*
 *  Handling log messages for Natural Language Processing library
 *  Copyright (c) 2017 Pertimm, by Patrick Constant
 *  Dev : September 2017
 *  Version 1.0
 */
#include "ogm_nlp.h"

/**
 * Log message at info level if trace_component is enable
 */
og_status NlpLogImplementation(og_nlp_th ctrl_nlp_th, og_string format, ...)
{
  // consistency checking
  IFN(ctrl_nlp_th) DPcErr;

  // level dependent
  int levelFlag = DOgMsgDestInLog;

  // prefix with lt number
  og_char_buffer format_extended[DPcPathSize];
  snprintf(format_extended, DPcPathSize, "%s: %s", ctrl_nlp_th->name, format);

  og_char_buffer textBuffer[DOgMlogMaxMessageSize];

  // var_args processing
  va_list vl;
  va_start(vl, format);
  vsnprintf(textBuffer, DOgMlogMaxMessageSize, format_extended, vl);
  va_end(vl);

  // log message
  IFE(OgMsg(ctrl_nlp_th->hmsg, "", levelFlag, textBuffer));

  DONE;
}

/**
 * Write json_t in buffer. Buffer is always NUL terminated.
 *
 * @param json json_t pointer
 * @param buffer buffer to write json in
 * @param buffer_size buffer size
 * @param p_truncated flag to indicate if json is truncated or not
 * @return status
 */
og_status NlpJsonToBuffer(const json_t *json, og_char_buffer *buffer, int buffer_size, og_bool *p_truncated,
    size_t flags)
{
  og_string truncated_ends = "...(truncated)";
  int truncated_ends_size = strlen(truncated_ends);

  int max_buffer_size = buffer_size - truncated_ends_size - 1;
  int expected_size = json_dumpb(json, buffer, max_buffer_size, flags | JSON_ENCODE_ANY);
  IF(expected_size)
  {
    DPcErr;
  }

  buffer[max_buffer_size] = 0;

  // truncated json
  if (expected_size > max_buffer_size)
  {
    if (p_truncated) *p_truncated = TRUE;
    snprintf(buffer + max_buffer_size, truncated_ends_size, "%s", truncated_ends);
  }
  else
  {
    buffer[expected_size] = 0;
  }

  if (p_truncated) *p_truncated = FALSE;

  DONE;
}

og_status NlpPackageCompileLog(og_nlp_th ctrl_nlp_th, package_t package)
{
  OgMsg(ctrl_nlp_th->hmsg, "", DOgMsgDestInLog, "Package compile '%s' '%s':", package->slug, package->id);

  int interpretation_used = OgHeapGetCellsUsed(package->hinterpretation_compile);
  for (int i = 0; i < interpretation_used; i++)
  {
    struct interpretation_compile *interpretation = OgHeapGetCell(package->hinterpretation_compile, i);
    IFN(interpretation) DPcErr;

    IFE(NlpPackageCompileInterpretationLog(ctrl_nlp_th, package, interpretation));
  }
  DONE;
}

og_status NlpPackageCompileInterpretationLog(og_nlp_th ctrl_nlp_th, package_t package,
    struct interpretation_compile *interpretation)
{
  IFN(interpretation) DPcErr;

  og_string interpretation_id = OgHeapGetCell(package->hinterpretation_ba, interpretation->id_start);
  IFN(interpretation_id) DPcErr;

  og_string interpretation_slug = OgHeapGetCell(package->hinterpretation_ba, interpretation->slug_start);
  IFN(interpretation_slug) DPcErr;

  OgMsg(ctrl_nlp_th->hmsg, "", DOgMsgDestInLog, "  Interpretation compile '%s' '%s':", interpretation_slug,
      interpretation_id);

  for (int i = 0; i < interpretation->expressions_nb; i++)
  {
    struct expression_compile *expression = OgHeapGetCell(package->hexpression_compile,
        interpretation->expression_start + i);
    IFN(expression) DPcErr;
    IFE(NlpPackageCompileExpressionLog(ctrl_nlp_th, package, expression));
  }

  IFE(NlpPackageCompileInterpretationSolutionLog(ctrl_nlp_th, package, interpretation));

  DONE;
}

og_status NlpPackageCompileInterpretationSolutionLog(og_nlp_th ctrl_nlp_th, package_t package,
    struct interpretation_compile *interpretation)
{
  IFN(interpretation->json_solution) DONE;

  og_char_buffer json_interpretation_string[DOgMlogMaxMessageSize / 2];
  IFE(NlpJsonToBuffer(interpretation->json_solution, json_interpretation_string, DOgMlogMaxMessageSize / 2, NULL, 0));
  OgMsg(ctrl_nlp_th->hmsg, "", DOgMsgDestInLog, "    solution: %s", json_interpretation_string);
  DONE;
}

og_status NlpPackageCompileExpressionLog(og_nlp_th ctrl_nlp_th, package_t package,
    struct expression_compile *expression)
{
  IFN(expression) DPcErr;

  og_string text = OgHeapGetCell(package->hexpression_ba, expression->text_start);

  unsigned char string_locale[DPcPathSize];
  OgIso639_3166ToCode(expression->locale, string_locale);

  OgMsg(ctrl_nlp_th->hmsg, "", DOgMsgDestInLog, "    Expression compile '%s' with locale %s", text, string_locale);
  for (int i = 0; i < expression->aliases_nb; i++)
  {
    struct alias_compile *alias = OgHeapGetCell(package->halias_compile, expression->alias_start + i);
    IFN(alias) DPcErr;

    IFE(NlpPackageCompileAliasLog(ctrl_nlp_th, package, alias));
  }

  IFE(NlpPackageCompileExpressionSolutionLog(ctrl_nlp_th, package, expression));

  DONE;
}

og_status NlpPackageCompileAliasLog(og_nlp_th ctrl_nlp_th, package_t package, struct alias_compile *alias)
{

  IFN(alias) DPcErr;

  og_string string_alias = OgHeapGetCell(package->halias_ba, alias->alias_start);

  if (alias->type == nlp_alias_type_type_Interpretation)
  {
    og_string string_slug = OgHeapGetCell(package->halias_ba, alias->slug_start);
    og_string string_id = OgHeapGetCell(package->halias_ba, alias->id_start);
    og_string string_package = OgHeapGetCell(package->halias_ba, alias->package_id_start);
    OgMsg(ctrl_nlp_th->hmsg, "", DOgMsgDestInLog, "      alias compile '%s' '%s' '%s' '%s'", string_alias, string_slug,
        string_id, string_package);
  }
  else
  {
    OgMsg(ctrl_nlp_th->hmsg, "", DOgMsgDestInLog, "      alias compile '%s' %s", string_alias,
        NlpAliasTypeString(alias->type));
  }
  DONE;
}

og_status NlpPackageCompileExpressionSolutionLog(og_nlp_th ctrl_nlp_th, package_t package,
    struct expression_compile *expression)
{
  IFN(expression->json_solution) DONE;

  og_char_buffer json_solution_string[DOgMlogMaxMessageSize / 2];
  IFE(NlpJsonToBuffer(expression->json_solution, json_solution_string, DOgMlogMaxMessageSize / 2, NULL, 0));
  OgMsg(ctrl_nlp_th->hmsg, "", DOgMsgDestInLog, "      solution: %s", json_solution_string);
  DONE;
}

og_status NlpPackageLog(og_nlp_th ctrl_nlp_th, og_string label, package_t package)
{
  OgMsg(ctrl_nlp_th->hmsg, "", DOgMsgDestInLog, "Package %s '%s' '%s' :", label, package->slug, package->id);

  int interpretation_used = OgHeapGetCellsUsed(package->hinterpretation);
  for (int i = 0; i < interpretation_used; i++)
  {
    struct interpretation *interpretation = OgHeapGetCell(package->hinterpretation, i);
    IFN(interpretation) DPcErr;

    IFE(NlpPackageInterpretationLog(ctrl_nlp_th, package, interpretation));
  }
  DONE;
}

og_status NlpPackageInterpretationLog(og_nlp_th ctrl_nlp_th, package_t package, struct interpretation *interpretation)
{
  IFN(interpretation) DPcErr;

  OgMsg(ctrl_nlp_th->hmsg, "", DOgMsgDestInLog, "  Interpretation '%s' '%s':", interpretation->slug,
      interpretation->id);

  for (int i = 0; i < interpretation->expressions_nb; i++)
  {
    IFE(NlpPackageExpressionLog(ctrl_nlp_th, package, interpretation->expressions + i));
  }

  IFE(NlpPackageInterpretationSolutionLog(ctrl_nlp_th, package, interpretation));

  DONE;
}

og_status NlpPackageInterpretationSolutionLog(og_nlp_th ctrl_nlp_th, package_t package,
    struct interpretation *interpretation)
{
  IFN(interpretation->json_solution) DONE;

  og_char_buffer json_interpretation_string[DOgMlogMaxMessageSize / 2];
  IFE(NlpJsonToBuffer(interpretation->json_solution, json_interpretation_string, DOgMlogMaxMessageSize / 2, NULL, 0));
  OgMsg(ctrl_nlp_th->hmsg, "", DOgMsgDestInLog, "    solution: %s", json_interpretation_string);
  DONE;
}

og_status NlpPackageExpressionLog(og_nlp_th ctrl_nlp_th, package_t package, struct expression *expression)
{
  IFN(expression) DPcErr;

  og_string text = expression->text;

  unsigned char string_locale[DPcPathSize];
  OgIso639_3166ToCode(expression->locale, string_locale);

  OgMsg(ctrl_nlp_th->hmsg, "", DOgMsgDestInLog, "    Expression '%s' with locale %s%s%s", text, string_locale,
      expression->keep_order ? " keep-order" : "", expression->glued ? " glued" : "");
  for (int i = 0; i < expression->aliases_nb; i++)
  {
    IFE(NlpPackageAliasLog(ctrl_nlp_th, package, expression->aliases + i));
  }

  for (int i = 0; i < expression->input_parts_nb; i++)
  {
    if (package->consolidate_done)
    {

      IFE(NlpPackageInputPartLog(ctrl_nlp_th, package, expression->input_parts + i));
    }
    else
    {
      struct input_part *input_parts = OgHeapGetCell(package->hinput_part, expression->input_part_start);
      IFN(input_parts) DPcErr;

      IFE(NlpPackageInputPartLog(ctrl_nlp_th, package, input_parts + i));
    }
  }

  IFE(NlpPackageExpressionSolutionLog(ctrl_nlp_th, package, expression));

  DONE;
}

og_status NlpPackageAliasLog(og_nlp_th ctrl_nlp_th, package_t package, struct alias *alias)

{
  IFN(alias) DPcErr;
  if (alias->type == nlp_alias_type_type_Interpretation)
  {
    OgMsg(ctrl_nlp_th->hmsg, "", DOgMsgDestInLog, "      alias '%s' '%s' '%s' '%s'", alias->alias, alias->slug,
        alias->id, alias->package_id);
  }
  else
  {
    OgMsg(ctrl_nlp_th->hmsg, "", DOgMsgDestInLog, "      alias '%s' %s", alias->alias, NlpAliasTypeString(alias->type));
  }
  DONE;
}

og_status NlpPackageInputPartLog(og_nlp_th ctrl_nlp_th, package_t package, struct input_part *input_part)

{
  IFN(input_part) DPcErr;

  struct input_part *all_input_part = OgHeapGetCell(package->hinput_part, 0);
  IFN(all_input_part) DPcErr;
  int Iinput_part = (input_part - all_input_part);

  switch (input_part->type)
  {
    case nlp_input_part_type_Nil:
    {
      break;
    }
    case nlp_input_part_type_Word:
    {
      og_string string_word = OgHeapGetCell(package->hinput_part_ba, input_part->word->word_start);
      IFN(string_word) DPcErr;
      OgMsg(ctrl_nlp_th->hmsg, "", DOgMsgDestInLog, "      %4d input_part word '%s'", Iinput_part, string_word);
      break;
    }
    case nlp_input_part_type_Interpretation:
    {
      struct alias *alias = input_part->alias;
      OgMsg(ctrl_nlp_th->hmsg, "", DOgMsgDestInLog, "      %4d input_part interpretation '%s' '%s' in package '%s'",
          Iinput_part, alias->slug, alias->id, alias->package_id);
      break;
    }
    case nlp_input_part_type_Digit:
    {
      struct alias *alias = input_part->alias;
      OgMsg(ctrl_nlp_th->hmsg, "", DOgMsgDestInLog, "      %4d input_part %s", Iinput_part,
          NlpAliasTypeString(alias->type));
      break;
    }
  }

  DONE;
}

og_status NlpPackageExpressionSolutionLog(og_nlp_th ctrl_nlp_th, package_t package, struct expression *expression)
{
  IFN(expression->json_solution) DONE;

  og_char_buffer json_solution_string[DOgMlogMaxMessageSize / 2];
  IFE(NlpJsonToBuffer(expression->json_solution, json_solution_string, DOgMlogMaxMessageSize / 2, NULL, 0));
  OgMsg(ctrl_nlp_th->hmsg, "", DOgMsgDestInLog, "      solution: %s", json_solution_string);
  DONE;
}

og_status NlpLogRequestWords(og_nlp_th ctrl_nlp_th)
{
  OgMsg(ctrl_nlp_th->hmsg, "", DOgMsgDestInLog, "list of request words:");
  int request_word_used = OgHeapGetCellsUsed(ctrl_nlp_th->hrequest_word);
  for (int i = 0; i < request_word_used; i++)
  {
    IFE(NlpLogRequestWord(ctrl_nlp_th, i));
  }
  DONE;
}

og_status NlpLogRequestWord(og_nlp_th ctrl_nlp_th, int Irequest_word)
{
  struct request_word *request_word = OgHeapGetCell(ctrl_nlp_th->hrequest_word, Irequest_word);
  IFN(request_word) DPcErr;

  og_string string_request_word = OgHeapGetCell(ctrl_nlp_th->hba, request_word->start);
  IFN(string_request_word) DPcErr;

  OgMsg(ctrl_nlp_th->hmsg, "", DOgMsgDestInLog, "%4d: '%s' at %d:%d", Irequest_word, string_request_word,
      request_word->start_position, request_word->length_position);
  DONE;
}

const char *NlpAliasTypeString(enum nlp_alias_type type)
{

  switch (type)
  {
    case nlp_alias_type_Nil:
      return "nil";
    case nlp_alias_type_type_Interpretation:
      return "interpretation";
    case nlp_alias_type_Any:
      return "any";
    case nlp_alias_type_Digit:
      return "digit";

  }
  return "alias_unknown";
}

