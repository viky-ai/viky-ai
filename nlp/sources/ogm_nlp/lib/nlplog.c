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
og_status NlpLogInfo(og_nlp_th ctrl_nlp_th, og_bitfield trace_component, og_string format, ...)
{
  // consistency checking
  IFN(ctrl_nlp_th) DPcErr;

  if (ctrl_nlp_th->loginfo->trace & trace_component)
  {
    // level dependent
    int levelFlag = DOgMsgDestInLog + DOgMsgSeverityInfo;
    og_char_buffer *levelText = "[INFO]";

    og_char_buffer textBuffer[DOgErrorSize];
    va_list vl;

    // prefix with lt number
    og_char_buffer format_extended[DPcPathSize];
    snprintf(format_extended, DPcPathSize, "%s: %s", ctrl_nlp_th->name, format);

    // var_args processing
    va_start(vl, format);
    vsnprintf(textBuffer, DOgMlogMaxMessageSize, format_extended, vl);
    va_end(vl);

    // log message
    IFE(OgMsg(ctrl_nlp_th->hmsg, levelText, levelFlag, textBuffer));

  }

  DONE;
}

/**
 * Log message at info level if trace_component is enable
 */
og_status NlpLogDebug(og_nlp_th ctrl_nlp_th, og_bitfield trace_component, og_string format, ...)
{
  // consistency checking
  IFN(ctrl_nlp_th) DPcErr;

  if (ctrl_nlp_th->loginfo->trace & trace_component)
  {
    // level dependent
    int levelFlag = DOgMsgDestInLog + DOgMsgSeverityDebug;
    og_char_buffer *levelText = "[DEBUG]";

    og_char_buffer textBuffer[DOgErrorSize];
    va_list vl;

    // prefix with lt number
    og_char_buffer format_extended[DPcPathSize];
    snprintf(format_extended, DPcPathSize, "%s: %s", ctrl_nlp_th->name, format);

    // var_args processing
    va_start(vl, format);
    vsnprintf(textBuffer, DOgMlogMaxMessageSize, format_extended, vl);
    va_end(vl);

    // log message
    IFE(OgMsg(ctrl_nlp_th->hmsg, levelText, levelFlag, textBuffer));

  }

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

  for (int i = 0; i < expression->input_parts_nb; i++)
  {
    IFE(NlpPackageInputPartLog(ctrl_nlp_th, package, expression->input_part_start + i));
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
  OgMsg(ctrl_nlp_th->hmsg, "", DOgMsgDestInLog, "      alias '%s' '%s' '%s' '%s'", string_alias, string_slug, string_id,
      string_package);

  DONE;
}

og_status NlpPackageInputPartLog(og_nlp_th ctrl_nlp_th, package_t package, int Iinput_part)

{
  struct input_part *input_part = OgHeapGetCell(package->hinput_part, Iinput_part);
  IFN(input_part) DPcErr;

  switch (input_part->type)
  {
    case nlp_input_part_type_Nil:
    {
      break;
    }
    case nlp_input_part_type_Word:
    {
      og_string string_word = OgHeapGetCell(package->hba, input_part->word_start);
      IFN(string_word) DPcErr;
      OgMsg(ctrl_nlp_th->hmsg, "", DOgMsgDestInLog, "      input_part word '%s'", string_word);
      break;
    }
    case nlp_input_part_type_Interpretation:
    {
      struct alias *alias = input_part->alias;

      og_string package_id = OgHeapGetCell(package->hba, alias->package_start);
      IFN(package_id) DPcErr;

      og_string interpretation_id = OgHeapGetCell(package->hba, alias->id_start);
      IFN(interpretation_id) DPcErr;
      og_string interpretation_slug = OgHeapGetCell(package->hba, alias->slug_start);
      IFN(interpretation_slug) DPcErr;
      OgMsg(ctrl_nlp_th->hmsg, "", DOgMsgDestInLog, "      input_part interpretation '%s' '%s' in package '%s'",
          interpretation_slug, interpretation_id, package_id);
      break;
    }
  }

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

