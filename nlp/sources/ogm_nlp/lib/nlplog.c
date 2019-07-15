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
  memset(buffer, 0, buffer_size);
  og_string truncated_ends = "...(truncated)";
  int truncated_ends_size = strlen(truncated_ends);

  if (json == NULL)
  {
    snprintf(buffer, buffer_size, "null");
    DONE;
  }

  int max_buffer_size = buffer_size - truncated_ends_size - 1;
  int expected_size = json_dumpb(json, buffer, max_buffer_size, flags | JSON_ENCODE_ANY);
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

  for (int i = 0; i < interpretation->contexts_nb; i++)
  {
    struct context_compile *context = OgHeapGetCell(package->hcontext_compile, interpretation->context_start + i);
    IFN(context) DPcErr;
    IFE(NlpPackageCompileContextLog(ctrl_nlp_th, package, context));
  }

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

og_status NlpPackageCompileContextLog(og_nlp_th ctrl_nlp_th, package_t package, struct context_compile *context)
{
  IFN(context) DPcErr;
  og_string flag = OgHeapGetCell(package->hcontext_ba, context->flag_start);
  OgMsg(ctrl_nlp_th->hmsg, "", DOgMsgDestInLog, "    Context compile '%s'", flag);
  DONE;
}

og_status NlpPackageCompileExpressionLog(og_nlp_th ctrl_nlp_th, package_t package,
    struct expression_compile *expression)
{
  IFN(expression) DPcErr;

  char pos_string[DPcPathSize];
  sprintf(pos_string," pos: %d", expression->pos);

  og_string text = OgHeapGetCell(package->hexpression_ba, expression->text_start);

  unsigned char string_locale[DPcPathSize];
  OgIso639_3166ToCode(expression->locale, string_locale);

  OgMsg(ctrl_nlp_th->hmsg, "", DOgMsgDestInLog, "    Expression compile '%s' with locale %s%s", text, string_locale,
      pos_string);
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

  if (alias->type == nlp_alias_type_Interpretation)
  {
    og_string string_slug = OgHeapGetCell(package->halias_ba, alias->slug_start);
    og_string string_id = OgHeapGetCell(package->halias_ba, alias->id_start);
    og_string string_package = OgHeapGetCell(package->halias_ba, alias->package_id_start);
    OgMsg(ctrl_nlp_th->hmsg, "", DOgMsgDestInLog, "      alias compile '%s' '%s' '%s' '%s'", string_alias, string_slug,
        string_id, string_package);
  }
  else if (alias->type == nlp_alias_type_Regex)
  {
    og_string string_regex = OgHeapGetCell(package->halias_ba, alias->regex_start);
    OgMsg(ctrl_nlp_th->hmsg, "", DOgMsgDestInLog, "      alias compile '%s' %s='%s'", string_alias,
        NlpAliasTypeString(alias->type), string_regex);
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

  unsigned char recursive[DPcPathSize];
  recursive[0]=0;
  if (interpretation->is_recursive) sprintf(recursive," recursive");

  unsigned char super_list[DPcPathSize];
  super_list[0]=0;
  if (interpretation->is_super_list) sprintf(super_list," super_list");

  OgMsg(ctrl_nlp_th->hmsg, "", DOgMsgDestInLog, "  Interpretation '%s' '%s'%s%s:", interpretation->slug,
      interpretation->id, recursive, super_list);

  for (int i = 0; i < interpretation->contexts_nb; i++)
  {
    IFE(NlpPackageContextLog(ctrl_nlp_th, package, interpretation->contexts + i));
  }

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

og_status NlpPackageContextLog(og_nlp_th ctrl_nlp_th, package_t package, struct context *context)
{
  IFN(context) DPcErr;
  og_string flag = context->flag;
  OgMsg(ctrl_nlp_th->hmsg, "", DOgMsgDestInLog, "    context '%s'", flag);
  DONE;
}

og_status NlpPackageExpressionLog(og_nlp_th ctrl_nlp_th, package_t package, struct expression *expression)
{
  IFN(expression) DPcErr;

  unsigned char recursive[DPcPathSize];
  recursive[0]=0;
  if (expression->is_recursive) sprintf(recursive," recursive");

  unsigned char super_list[DPcPathSize];
  super_list[0]=0;
  if (expression->is_super_list) sprintf(super_list," super_list");

  char pos_string[DPcPathSize];
  sprintf(pos_string," pos: %d", expression->pos);

  og_string text = expression->text;

  unsigned char string_locale[DPcPathSize];
  OgIso639_3166ToCode(expression->locale, string_locale);

  unsigned char string_glue_distance[DPcPathSize];
  sprintf(string_glue_distance," glue_distance=%d",expression->glue_distance);

  unsigned char string_glue_strength[DPcPathSize];
  string_glue_strength[0]=0;
  if (expression->glue_strength != nlp_glue_strength_Total)
  {
    sprintf(string_glue_strength," glue_strength=%s",NlpGlueStrengthString(expression->glue_strength));
  }

  OgMsg(ctrl_nlp_th->hmsg, "", DOgMsgDestInLog,
      "    Expression '%s' with locale %s%s%s%s%s%s%s alias_any_input_part_position=%d any_input_part_position=%d" , text, string_locale, pos_string,
      expression->keep_order ? " keep-order" : "", string_glue_distance,  string_glue_strength, recursive,
      super_list, expression->alias_any_input_part_position, expression->any_input_part_position);

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
  if (alias->type == nlp_alias_type_Interpretation)
  {
    OgMsg(ctrl_nlp_th->hmsg, "", DOgMsgDestInLog, "      alias '%s' '%s' '%s' '%s'", alias->alias, alias->slug,
        alias->id, alias->package_id);
  }
  else if (alias->type == nlp_alias_type_Regex)
  {
    OgMsg(ctrl_nlp_th->hmsg, "", DOgMsgDestInLog, "      alias '%s' %s='%.*s'", alias->alias,
        NlpAliasTypeString(alias->type), DOgNlpMaximumRegexStringSizeLogged, alias->regex_string);
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
      OgMsg(ctrl_nlp_th->hmsg, "", DOgMsgDestInLog, "      %4d input_part interpretation '%s'",
          Iinput_part, alias->slug);
      break;
    }
    case nlp_input_part_type_Number:
    {
      struct alias *alias = input_part->alias;
      OgMsg(ctrl_nlp_th->hmsg, "", DOgMsgDestInLog, "      %4d input_part %s", Iinput_part,
          NlpAliasTypeString(alias->type));
      break;
    }
    case nlp_input_part_type_Regex:
    {
      struct alias *alias = input_part->alias;
      OgMsg(ctrl_nlp_th->hmsg, "", DOgMsgDestInLog, "      %4d input_part %s", Iinput_part,
          NlpAliasTypeString(alias->type));
      break;
    }
  }

  DONE;
}


og_status NlpPackageInputPartExpressionLog(og_nlp_th ctrl_nlp_th, package_t package, int Iinput_part, char *label)

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
      og_string string_word = OgHeapGetCell(package->hinput_part_ba, input_part->word->word_start);
      IFN(string_word) DPcErr;
      OgMsg(ctrl_nlp_th->hmsg, "", DOgMsgDestInLog, "%s %4d input_part word '%s' in expression '%s'", label,
          Iinput_part, string_word, input_part->expression->text);
      break;
    }
    case nlp_input_part_type_Interpretation:
    {
      struct alias *alias = input_part->alias;
      OgMsg(ctrl_nlp_th->hmsg, "", DOgMsgDestInLog, "%s %4d input_part interpretation '%s' in expression '%s'", label,
          Iinput_part, alias->slug, input_part->expression->text);
      break;
    }
    case nlp_input_part_type_Number:
    {
      struct alias *alias = input_part->alias;
      OgMsg(ctrl_nlp_th->hmsg, "", DOgMsgDestInLog, "%s %4d input_part %s in expression '%s'", label, Iinput_part,
          NlpAliasTypeString(alias->type), input_part->expression->text);
      break;
    }
    case nlp_input_part_type_Regex:
    {
      struct alias *alias = input_part->alias;
      OgMsg(ctrl_nlp_th->hmsg, "", DOgMsgDestInLog, "%s %4d input_part %s in expression '%s'", label, Iinput_part,
          NlpAliasTypeString(alias->type), input_part->expression->text);
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

  struct request_word *first_request_word = OgHeapGetCell(ctrl_nlp_th->hrequest_word, 0);
  IFN(first_request_word) DPcErr;

  for (struct request_word *rw = first_request_word; rw; rw = rw->next)
  {
    IFE(NlpLogRequestWord(ctrl_nlp_th, rw));
  }
  DONE;
}

og_status NlpLogRequestWord(og_nlp_th ctrl_nlp_th, struct request_word *request_word)
{
  og_string string_request_word = OgHeapGetCell(ctrl_nlp_th->hba, request_word->start);
  IFN(string_request_word) DPcErr;

  unsigned char is_punctuation[DPcPathSize];
  is_punctuation[0] = 0;
  if (request_word->is_punctuation)
  {
    if (request_word->is_expression_punctuation)
    {
      snprintf(is_punctuation, DPcPathSize, " (expression punctuation)");
    }
    else
    {
      snprintf(is_punctuation, DPcPathSize, " (punctuation)");
    }
  }

  unsigned char is_regex[DPcPathSize];
  is_regex[0] = 0;
  if (request_word->is_regex)
  {
    snprintf(is_regex, DPcPathSize, " (regex='%.*s')", DOgNlpMaximumRegexStringSizeLogged,
        request_word->regex_input_part->alias->regex_string);
  }

  OgMsg(ctrl_nlp_th->hmsg, "", DOgMsgDestInLog, "%4d: '%s' at %d:%d%s%s", request_word->self_index, string_request_word,
      request_word->start_position, request_word->length_position, is_regex, is_punctuation);
  DONE;
}

const char *NlpAliasTypeString(enum nlp_alias_type type)
{

  switch (type)
  {
    case nlp_alias_type_Nil:
      return "nil";
    case nlp_alias_type_Interpretation:
      return "interpretation";
    case nlp_alias_type_Any:
      return "any";
    case nlp_alias_type_Number:
      return "number";
    case nlp_alias_type_Regex:
      return "regex";

  }
  return "alias_unknown";
}

const char *NlpGlueStrengthString(enum nlp_glue_strength glue_strength)
{

  switch (glue_strength)
  {
    case nlp_glue_strength_Nil:
      return "nil";
    case nlp_glue_strength_Total:
      return "total";
    case nlp_glue_strength_Punctuation:
      return "punctuation";
  }
  return "glue_strength_unknown";
}

static og_status NlpLogJanssonRefCounterRecursive(og_nlp_th ctrl_nlp_th, og_string key, json_t *json_value, int depth);
static og_status NlpLogJanssonRefCounterRecursiveObject(og_nlp_th ctrl_nlp_th, og_string key, json_t *json_object,
    int depth);
static og_status NlpLogJanssonRefCounterRecursiveArray(og_nlp_th ctrl_nlp_th, og_string key, json_t *json_array,
    int depth);
static og_status NlpLogJanssonRefCounterSimple(og_nlp_th ctrl_nlp_th, og_string key, json_t *json_value, int depth);

PUBLIC(og_status) OgNlpLogJanssonRefCounter(og_nlp_th ctrl_nlp_th, og_string function_name, json_t *json)
{
  NlpLog(DOgNlpTraceMinimal, "OgNlpLogJanssonRefCounter %s:", function_name);
  IFE(NlpLogJanssonRefCounterRecursive(ctrl_nlp_th, "root", json, 0));

  DONE;
}

static og_status NlpLogJanssonRefCounterRecursive(og_nlp_th ctrl_nlp_th, og_string key, json_t *json_value, int depth)
{
  if (json_value == NULL) CONT;

  switch (json_typeof(json_value))
  {
    case JSON_OBJECT:
      IFE(NlpLogJanssonRefCounterRecursiveObject(ctrl_nlp_th, key, json_value, depth));
      break;
    case JSON_ARRAY:
      IFE(NlpLogJanssonRefCounterRecursiveArray(ctrl_nlp_th, key, json_value, depth));
      break;
    default:
      IFE(NlpLogJanssonRefCounterSimple(ctrl_nlp_th, key, json_value, depth));
      break;
  }

  DONE;
}

static og_status NlpLogJanssonRefCounterRecursiveObject(og_nlp_th ctrl_nlp_th, og_string key, json_t *json_object,
    int depth)
{
  if (json_object == NULL) CONT;
  if (!json_is_object(json_object))
  {
    NlpThrowErrorTh(ctrl_nlp_th, "NlpLogJanssonRefCounterRecursiveObject is not an object");
    DPcErr;
  }

  og_char_buffer header[DPcPathSize];
  int i = 0;
  for (i = 0; i < (depth * 2) && DPcPathSize; i++)
  {
    header[i] = ' ';

  }
  header[i] = '\0';

  if (key)
  {
    NlpLog(DOgNlpTraceMinimal, "%s \"%s\": {   #refcount: %zu", header, key, json_object->refcount);
  }
  else
  {
    NlpLog(DOgNlpTraceMinimal, "%s {   #refcount: %zu", header, json_object->refcount);
  }

  for (void *iter = json_object_iter(json_object); iter; iter = json_object_iter_next(json_object, iter))
  {
    og_string value_key = json_object_iter_key(iter);
    json_t *value = json_object_iter_value(iter);

    IFE(NlpLogJanssonRefCounterRecursive(ctrl_nlp_th, value_key, value, depth + 1));
  }

  NlpLog(DOgNlpTraceMinimal, "%s },", header);

  DONE;
}

static og_status NlpLogJanssonRefCounterRecursiveArray(og_nlp_th ctrl_nlp_th, og_string key, json_t *json_array,
    int depth)
{
  if (json_array == NULL) CONT;
  if (!json_is_array(json_array))
  {
    NlpThrowErrorTh(ctrl_nlp_th, "NlpLogJanssonRefCounterRecursiveArray is not an array");
    DPcErr;
  }

  og_char_buffer header[DPcPathSize];
  int i = 0;
  for (i = 0; i < (depth * 2) && DPcPathSize; i++)
  {
    header[i] = ' ';

  }
  header[i] = '\0';

  if (key)
  {
    NlpLog(DOgNlpTraceMinimal, "%s \"%s\": [   #refcount: %zu", header, key, json_array->refcount);
  }
  else
  {
    NlpLog(DOgNlpTraceMinimal, "%s [   #refcount: %zu", header, json_array->refcount);
  }

  for (size_t j = 0; j < json_array_size(json_array); j++)
  {
    json_t *value = json_array_get(json_array, j);

    IFE(NlpLogJanssonRefCounterRecursive(ctrl_nlp_th, NULL, value, depth + 1));
  }

  NlpLog(DOgNlpTraceMinimal, "%s ],", header);

  DONE;
}

static og_status NlpLogJanssonRefCounterSimple(og_nlp_th ctrl_nlp_th, og_string key, json_t *json_value, int depth)
{
  if (json_value == NULL) CONT;

  og_char_buffer json_string_value[DPcPathSize];
  IFE(NlpJsonToBuffer(json_value, json_string_value, DPcPathSize, NULL, 0));

  og_char_buffer header[DPcPathSize];
  int i = 0;
  for (i = 0; i < (depth * 2) && DPcPathSize; i++)
  {
    header[i] = ' ';

  }
  header[i] = '\0';

  og_char_buffer refcount[DPcPathSize];
  if (json_is_boolean(json_value) || json_is_null(json_value))
  {
    snprintf(refcount, DPcPathSize, "N/A");
  }
  else
  {
    snprintf(refcount, DPcPathSize, "%zu", json_value->refcount);
  }

  if (key)
  {
    NlpLog(DOgNlpTraceMinimal, "%s \"%s\": %s   #refcount: %s", header, key, json_string_value, refcount);
  }
  else
  {
    NlpLog(DOgNlpTraceMinimal, "%s %s   #refcount: %s", header, json_string_value, refcount);
  }

  DONE;
}
