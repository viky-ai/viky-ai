/*
 *  Main function for Natural Language Processing library
 *  Copyright (c) 2017 Pertimm, by Patrick Constant
 *  Dev : September 2017
 *  Version 1.0
 */
#include "ogm_nlp.h"

static int NlpCompilePackageInterpretations(og_nlp_th ctrl_nlp_th, struct og_nlp_compile_input *input, json_t *json_id,
    json_t *json_slug, json_t *json_interpretations);

static int NlpCompilePackageContexts(og_nlp_th ctrl_nlp_th, package_t package,
    struct interpretation_compile *interpretation, json_t *json_contexts);
static int NlpCompilePackageContext(og_nlp_th ctrl_nlp_th, package_t package,
    struct interpretation_compile *interpretation, json_t *json_context);
static int NlpCompilePackageExpressions(og_nlp_th ctrl_nlp_th, package_t package,
    struct interpretation_compile *interpretation, json_t *json_expressions);
static int NlpCompilePackageExpression(og_nlp_th ctrl_nlp_th, package_t package,
    struct interpretation_compile *interpretation, json_t *json_expression);
static int NlpCompilePackageExpressionAliases(og_nlp_th ctrl_nlp_th, package_t package,
    struct expression_compile *expression, json_t *json_aliases);
static int NlpCompilePackageExpressionAlias(og_nlp_th ctrl_nlp_th, package_t package,
    struct expression_compile *expression, json_t *json_alias);

PUBLIC(int) OgNlpCompile(og_nlp_th ctrl_nlp_th, struct og_nlp_compile_input *input,
    struct og_nlp_compile_output *output)
{

  // reset ouput
  memset(output, 0, sizeof(struct og_nlp_compile_output));

  og_char_buffer json_compile_request_string[DOgMlogMaxMessageSize / 2];
  IFE(NlpJsonToBuffer(input->json_input, json_compile_request_string, DOgMlogMaxMessageSize / 2, NULL, JSON_INDENT(2)));

  NlpLog(DOgNlpTraceCompile, "OgNlpCompile: json_compile_request_string is [\n%s]", json_compile_request_string)

  // package input can be a json object (a package) or an array (a list of package)
  if (json_is_object(input->json_input))
  {
    IFE(NlpCompilePackage(ctrl_nlp_th, input, input->json_input));
  }
  else if (json_is_array(input->json_input))
  {
    int array_size = json_array_size(input->json_input);
    for (int i = 0; i < array_size; i++)
    {
      json_t *json_package = json_array_get(input->json_input, i);
      if (json_is_object(json_package))
      {
        IFE(NlpCompilePackage(ctrl_nlp_th, input, json_package));
      }
      else
      {
        NlpThrowErrorTh(ctrl_nlp_th, "OgNlpCompile: structure error : json_package at position %d is not an object", i);
        DPcErr;
      }
    }

  }
  else
  {
    NlpThrowErrorTh(ctrl_nlp_th, "OgNlpCompile: structure error : main container must be an array or an object");
    DPcErr;
  }

  // Just saying that compilation went well
  ctrl_nlp_th->json_answer = json_object();

  IFE(json_object_set_new(ctrl_nlp_th->json_answer, "compilation", json_string("ok")));

  output->json_output = ctrl_nlp_th->json_answer;

  DONE;
}

og_status NlpCompilePackage(og_nlp_th ctrl_nlp_th, struct og_nlp_compile_input *input, json_t *json_package)
{
  og_char_buffer json_package_string[DPcPathSize];
  IFE(NlpJsonToBuffer(json_package, json_package_string, DPcPathSize, NULL, JSON_INDENT(2)));

  NlpLog(DOgNlpTraceCompile, "NlpCompilePackage: compiling package [\n%s]", json_package_string)

  json_t *json_id = NULL;
  json_t *json_slug = NULL;
  json_t *json_interpretations = NULL;

  for (void *iter = json_object_iter(json_package); iter; iter = json_object_iter_next(json_package, iter))
  {
    og_string key = json_object_iter_key(iter);

    NlpLog(DOgNlpTraceCompile, "NlpCompilePackage: found key='%s'", key)

    if (Ogstricmp(key, "id") == 0)
    {
      json_id = json_object_iter_value(iter);
    }
    else if (Ogstricmp(key, "slug") == 0)
    {
      json_slug = json_object_iter_value(iter);
    }
    else if (Ogstricmp(key, "interpretations") == 0)
    {
      json_interpretations = json_object_iter_value(iter);
    }
    else
    {
      NlpThrowErrorTh(ctrl_nlp_th, "NlpCompilePackage: unknow key '%s'", key);
      DPcErr;
    }
  }

  IFN(json_id)
  {
    NlpThrowErrorTh(ctrl_nlp_th, "NlpCompilePackage: package without id");
    DPcErr;
  }
  if (!json_is_string(json_id))
  {
    NlpThrowErrorTh(ctrl_nlp_th, "NlpCompilePackage: package 'id' is not a string");
    DPcErr;
  }

  if (json_slug)
  {
    if (!json_is_string(json_slug))
    {
      NlpThrowErrorTh(ctrl_nlp_th, "NlpCompilePackage: package 'slug' is not a string");
      DPcErr;
    }
  }
  else
  {
    json_slug = json_string(json_string_value(json_id));
  }

  // check unicity
  if (!input->package_update)
  {
    og_string package_id = json_string_value(json_id);
    package_t package = NlpPackageGet(ctrl_nlp_th, package_id);
    NlpPackageMarkAsUnused(ctrl_nlp_th, package);
    if (package != NULL)
    {
      NlpThrowErrorTh(ctrl_nlp_th, "NlpCompilePackage: package with id='%s' already exists, init must update package",
          package_id);
      DPcErr;
    }
  }

  IFN(json_interpretations)
  {
    NlpThrowErrorTh(ctrl_nlp_th, "NlpCompilePackage: no 'interpretations' in package");
    DPcErr;
  }

  IFE(NlpCompilePackageInterpretations(ctrl_nlp_th, input, json_id, json_slug, json_interpretations));

  DONE;
}

static int NlpCompilePackageInterpretations(og_nlp_th ctrl_nlp_th, struct og_nlp_compile_input *input, json_t *json_id,
    json_t *json_slug, json_t *json_interpretations)
{

  if (!json_is_array(json_interpretations))
  {
    NlpThrowErrorTh(ctrl_nlp_th, "NlpCompilePackageInterpretations: 'interpretations' is not an array");
    DPcErr;
  }

  // At that point, we can create the package structure
  og_string string_id = json_string_value(json_id);
  og_string string_slug = json_string_value(json_slug);

  NlpLog(DOgNlpTraceCompile, "NlpCompilePackageInterpretations: package id is '%s', slug is '%s'", string_id,
      string_slug)

  // We do not use a package heap because we dont want synchronization on that heap
  package_t package = NlpPackageCreate(ctrl_nlp_th, string_id, string_slug);
  if (!package)
  {
    NlpThrowErrorTh(ctrl_nlp_th, "NlpCompilePackageinterpretations: NlpPackageCreate: failed on package '%s' '%s'",
        string_slug, string_id);
    DPcErr;
  }

  int array_size = json_array_size(json_interpretations);
  for (int i = 0; i < array_size; i++)
  {
    json_t *json_interpretation = json_array_get(json_interpretations, i);
    if (json_is_object(json_interpretation))
    {
      IF(NlpCompilePackageInterpretation(ctrl_nlp_th, package, json_interpretation))
      {
        NlpThrowErrorTh(ctrl_nlp_th, "NlpCompilePackageinterpretations: NlpCompilePackageInterpretation :"
            " failed on package '%s' '%s'", package->slug, package->id);
        DPcErr;
      }
    }
    else
    {
      NlpThrowErrorTh(ctrl_nlp_th, "NlpCompilePackageinterpretations: failed on package '%s' '%s', "
          "json_interpretation at position %d is not an object", package->slug, package->id, i);
      DPcErr;
    }
  }

  if (ctrl_nlp_th->loginfo->trace & DOgNlpTraceCompile)
  {
    IFE(NlpPackageCompileLog(ctrl_nlp_th, package));
  }

  // freeze memory structure
  IF(NlpConsolidatePackage(ctrl_nlp_th, package))
  {
    NlpThrowErrorTh(ctrl_nlp_th, "NlpCompilePackageinterpretations: NlpConsolidatePackage : "
        "failed on package '%s' '%s'", package->slug, package->id);
    DPcErr;
  }

  // publish package
  IFE(NlpPackageAddOrReplace(ctrl_nlp_th, package));

  // mark temporary package done
  ctrl_nlp_th->package_in_progress = NULL;

  DONE;
}

og_status NlpCompilePackageInterpretation(og_nlp_th ctrl_nlp_th, package_t package, json_t *json_interpretation)
{
  og_char_buffer json_interpretation_string[DPcPathSize];
  IFE(NlpJsonToBuffer(json_interpretation, json_interpretation_string, DPcPathSize, NULL, JSON_INDENT(2)));

  NlpLog(DOgNlpTraceCompile, "NlpCompilePackageInterpretation: compiling interpretation [\n%s]",
      json_interpretation_string)

  json_t *json_id = NULL;
  json_t *json_slug = NULL;
  json_t *json_contexts = NULL;
  json_t *json_expressions = NULL;
  json_t *json_solution = NULL;
  json_t *json_scope = NULL;

  for (void *iter = json_object_iter(json_interpretation); iter;
      iter = json_object_iter_next(json_interpretation, iter))
  {
    og_string key = json_object_iter_key(iter);
    NlpLog(DOgNlpTraceCompile, "NlpCompilePackageInterpretation: found key='%s'", key)

    if (Ogstricmp(key, "id") == 0)
    {
      json_id = json_object_iter_value(iter);
    }
    else if (Ogstricmp(key, "slug") == 0)
    {
      json_slug = json_object_iter_value(iter);
    }
    else if (Ogstricmp(key, "contexts") == 0)
    {
      json_contexts = json_object_iter_value(iter);
    }
    else if (Ogstricmp(key, "expressions") == 0)
    {
      json_expressions = json_object_iter_value(iter);
    }
    else if (Ogstricmp(key, "solution") == 0)
    {
      json_solution = json_object_iter_value(iter);
    }
    else if (Ogstricmp(key, "scope") == 0)
    {
      json_scope = json_object_iter_value(iter);
    }
    else
    {
      NlpThrowErrorTh(ctrl_nlp_th, "NlpCompilePackageInterpretation: unknow key '%s'", key);
      DPcErr;
    }

  }

  IFN(json_id)
  {
    NlpThrowErrorTh(ctrl_nlp_th, "NlpCompilePackageInterpretation: interpretation has no 'id'");
    DPcErr;
  }
  if (!json_is_string(json_id))
  {
    NlpThrowErrorTh(ctrl_nlp_th, "NlpCompilePackageInterpretation: interpretation 'id' is not a string");
    DPcErr;
  }
  const char *string_id = json_string_value(json_id);
  const char *string_slug;

  // interpretation id is mandatory
  IFN(json_slug)
  {
    string_slug = json_string_value(json_id);
  }
  else if (!json_is_string(json_slug))
  {
    NlpThrowErrorTh(ctrl_nlp_th, "NlpCompilePackageInterpretation: interpretation 'slug' is not a string");
    DPcErr;
  }
  else
  {
    string_slug = json_string_value(json_slug);
  }
  // interpretation expressions is mandatory
  IFN(json_expressions)
  {
    NlpThrowErrorTh(ctrl_nlp_th, "NlpCompilePackageInterpretation: interpretation has no 'expressions'");
    DPcErr;
  }
  if (!json_is_array(json_expressions))
  {
    NlpThrowErrorTh(ctrl_nlp_th, "NlpCompilePackageInterpretation: interpretation 'expressions' is not an array");
    DPcErr;
  }

  IFX(json_contexts)
  {
    if (!json_is_array(json_contexts))
    {
      NlpThrowErrorTh(ctrl_nlp_th, "NlpCompilePackageInterpretation: interpretation 'context' is not an array");
      DPcErr;
    }
  }

  enum nlp_interpretation_scope_type scope = nlp_interpretation_scope_type_public;
  IFX(json_scope)
  {
    if (!json_is_string(json_scope))
    {
      NlpThrowErrorTh(ctrl_nlp_th, "NlpCompilePackageInterpretation: interpretation 'scope' is not a string");
      DPcErr;
    }
    else
    {
      og_string scope_string = json_string_value(json_scope);
      if (Ogstricmp(scope_string, "public") == 0)
      {
        scope = nlp_interpretation_scope_type_public;
      }
      else if (Ogstricmp(scope_string, "private") == 0)
      {
        scope = nlp_interpretation_scope_type_private;
      }
      else if (Ogstricmp(scope_string, "hidden") == 0)
      {
        scope = nlp_interpretation_scope_type_hidden;
      }
      else
      {
        NlpThrowErrorTh(ctrl_nlp_th, "NlpCompilePackageInterpretation: interpretation 'scope' "
            "with value '%s' is not supported", scope_string);
        DPcErr;
      }
    }
  }

  // solution can be of any json type and can be non existant

  // At that point, we can create the interpretation structure
  NlpLog(DOgNlpTraceCompile, "NlpCompilePackageInterpretation: interpretation id is '%s', slug is '%s'", string_id,
      string_slug)

  size_t Iinterpretation;
  struct interpretation_compile *interpretation = OgHeapNewCell(package->hinterpretation_compile, &Iinterpretation);
  IFn(interpretation) DPcErr;
  IFE(Iinterpretation);

  interpretation->id_start = OgHeapGetCellsUsed(package->hinterpretation_ba);
  interpretation->id_length = strlen(string_id);
  IFE(OgHeapAppend(package->hinterpretation_ba, interpretation->id_length + 1, string_id));

  interpretation->slug_start = OgHeapGetCellsUsed(package->hinterpretation_ba);
  interpretation->slug_length = strlen(string_slug);
  IFE(OgHeapAppend(package->hinterpretation_ba, interpretation->slug_length + 1, string_slug));

  interpretation->json_solution = json_solution;

  interpretation->scope = scope;

  IFE(NlpCompilePackageContexts(ctrl_nlp_th, package, interpretation, json_contexts));
  IFE(NlpCompilePackageExpressions(ctrl_nlp_th, package, interpretation, json_expressions));

  DONE;
}

static int NlpCompilePackageContexts(og_nlp_th ctrl_nlp_th, package_t package,
    struct interpretation_compile *interpretation, json_t *json_contexts)
{
  interpretation->context_start = OgHeapGetCellsUsed(package->hcontext_compile);

  int array_size = json_array_size(json_contexts);
  for (int i = 0; i < array_size; i++)
  {
    json_t *json_context = json_array_get(json_contexts, i);
    IFN(json_context)
    {
      NlpThrowErrorTh(ctrl_nlp_th, "NlpCompilePackagecontexts: null json_context at position %d", i);
      DPcErr;
    }
    if (json_is_string(json_context))
    {
      IFE(NlpCompilePackageContext(ctrl_nlp_th, package, interpretation, json_context));
    }
    else
    {
      NlpThrowErrorTh(ctrl_nlp_th, "NlpCompilePackagecontexts: json_context at position %d is not string", i);
      DPcErr;
    }
  }

  int contexts_used = OgHeapGetCellsUsed(package->hcontext_compile);
  interpretation->contexts_nb = contexts_used - interpretation->context_start;
  DONE;
}

static int NlpCompilePackageContext(og_nlp_th ctrl_nlp_th, package_t package,
    struct interpretation_compile *interpretation, json_t *json_context)
{
  size_t Icontext;
  struct context_compile *context = OgHeapNewCell(package->hcontext_compile, &Icontext);
  IFn(context) DPcErr;
  IFE(Icontext);

  if (json_is_string(json_context))
  {
    const char *string_flag = json_string_value(json_context);
    context->flag_start = OgHeapGetCellsUsed(package->hcontext_ba);
    int flag_length = strlen(string_flag);
    if (flag_length > DOgNlpInterpretationContextFlagMaxLength)
    {
      NlpThrowErrorTh(ctrl_nlp_th, "NlpCompilePackageContext: json_context is too long");
      DPcErr;
    }
    IFE(OgHeapAppend(package->hcontext_ba, flag_length + 1, string_flag));
  }
  else
  {
    NlpThrowErrorTh(ctrl_nlp_th, "NlpCompilePackageContext: json_context is not a string");
    DPcErr;
  }
  DONE;
}

static int NlpCompilePackageExpressions(og_nlp_th ctrl_nlp_th, package_t package,
    struct interpretation_compile *interpretation, json_t *json_expressions)
{
  interpretation->expression_start = OgHeapGetCellsUsed(package->hexpression_compile);

  int array_size = json_array_size(json_expressions);
  for (int i = 0; i < array_size; i++)
  {
    json_t *json_expression = json_array_get(json_expressions, i);
    IFN(json_expression)
    {
      NlpThrowErrorTh(ctrl_nlp_th, "NlpCompilePackageExpressions: null json_expression at position %d", i);
      DPcErr;
    }
    if (json_is_object(json_expression))
    {
      IFE(NlpCompilePackageExpression(ctrl_nlp_th, package, interpretation, json_expression));
    }
    else
    {
      NlpThrowErrorTh(ctrl_nlp_th, "NlpCompilePackageExpressions: json_expression at position %d is not an object", i);
      DPcErr;
    }
  }

  int expressions_used = OgHeapGetCellsUsed(package->hexpression_compile);
  interpretation->expressions_nb = expressions_used - interpretation->expression_start;

  DONE;
}

static int NlpCompilePackageExpression(og_nlp_th ctrl_nlp_th, package_t package,
    struct interpretation_compile *interpretation, json_t *json_expression)
{
  og_char_buffer json_expression_string[DPcPathSize];
  IFE(NlpJsonToBuffer(json_expression, json_expression_string, DPcPathSize, NULL, JSON_INDENT(2)));
  NlpLog(DOgNlpTraceCompile, "NlpCompilePackageExpression: compiling expression [\n%s]", json_expression_string)

  json_t *json_id = NULL;
  json_t *json_text = NULL;
  json_t *json_keep_order = NULL;
  json_t *json_glue_strength = NULL;
  json_t *json_glue_distance = NULL;
  json_t *json_aliases = NULL;
  json_t *json_locale = NULL;
  json_t *json_solution = NULL;

  for (void *iter = json_object_iter(json_expression); iter; iter = json_object_iter_next(json_expression, iter))
  {
    const char *key = json_object_iter_key(iter);
    NlpLog(DOgNlpTraceCompile, "NlpCompilePackageExpression: found key='%s'", key)

    if (Ogstricmp(key, "id") == 0)
    {
      json_id = json_object_iter_value(iter);
    }
    else if (Ogstricmp(key, "expression") == 0)
    {
      json_text = json_object_iter_value(iter);
    }
    else if (Ogstricmp(key, "keep-order") == 0)
    {
      json_keep_order = json_object_iter_value(iter);
    }
    else if (Ogstricmp(key, "glue-strength") == 0)
    {
      json_glue_strength = json_object_iter_value(iter);
    }
    else if (Ogstricmp(key, "glue-distance") == 0)
    {
      json_glue_distance = json_object_iter_value(iter);
    }
    else if (Ogstricmp(key, "aliases") == 0)
    {
      json_aliases = json_object_iter_value(iter);
    }
    else if (Ogstricmp(key, "locale") == 0)
    {
      json_locale = json_object_iter_value(iter);
    }
    else if (Ogstricmp(key, "solution") == 0)
    {
      json_solution = json_object_iter_value(iter);
    }
    else
    {
      NlpThrowErrorTh(ctrl_nlp_th, "NlpCompilePackageExpression: unknown key '%s'", key);
      DPcErr;
    }
  }

  const char *string_id = NULL;

  IFN(json_id)
  {
    string_id = NULL;
  }
  else if (!json_is_string(json_id))
  {
    NlpThrowErrorTh(ctrl_nlp_th, "NlpCompilePackageExpression: interpretation 'id' is not a string");
    DPcErr;
  }
  string_id = json_string_value(json_id);

  IFN(json_text)
  {
    NlpThrowErrorTh(ctrl_nlp_th, "NlpCompilePackageExpression: no text");
    DPcErr;
  }

  size_t Iexpression;
  struct expression_compile *expression = OgHeapNewCell(package->hexpression_compile, &Iexpression);
  IFn(expression) DPcErr;
  IFE(Iexpression);

  IFN(string_id)
  {
    expression->id_start = -1;
  }
  else
  {
    expression->id_start = OgHeapGetCellsUsed(package->hexpression_ba);
    IFE(OgHeapAppend(package->hexpression_ba, strlen(string_id) + 1, string_id));
  }

  if (json_is_string(json_text))
  {
    const char *string_text = json_string_value(json_text);
    expression->text_start = OgHeapGetCellsUsed(package->hexpression_ba);
    int text_length = strlen(string_text);
    if (text_length > DOgNlpInterpretationExpressionMaxLength)
    {
      NlpThrowErrorTh(ctrl_nlp_th, "NlpCompilePackageExpression: text is too long");
      DPcErr;
    }
    IFE(OgHeapAppend(package->hexpression_ba, text_length + 1, string_text));
  }
  else
  {
    NlpThrowErrorTh(ctrl_nlp_th, "NlpCompilePackageExpression: text is not a string");
    DPcErr;
  }

  expression->locale = DOgLangNil;
  if (json_locale == NULL)
  {
    expression->locale = DOgLangNil;
  }
  else if (json_is_string(json_locale))
  {
    const char *string_locale = json_string_value(json_locale);
    IFE(expression->locale = OgCodeToIso639_3166(ctrl_nlp_th->herr, (char * )string_locale));
  }
  else if (json_is_null(json_locale))
  {
    expression->locale = DOgLangNil;
  }
  else
  {
    NlpThrowErrorTh(ctrl_nlp_th, "NlpCompilePackageExpression: locale is not a string");
    DPcErr;
  }

  expression->keep_order = FALSE;
  if (json_keep_order == NULL)
  {
    expression->keep_order = FALSE;
  }
  else if (json_is_boolean(json_keep_order))
  {
    expression->keep_order = json_boolean_value(json_keep_order);
  }
  else if (json_is_null(json_keep_order))
  {
    expression->keep_order = FALSE;
  }
  else
  {
    NlpThrowErrorTh(ctrl_nlp_th, "NlpCompilePackageExpression: keep-order is not a boolean");
    DPcErr;
  }

  expression->glue_strength = nlp_glue_strength_Total;
  if (json_glue_strength == NULL)
  {
    expression->glue_strength = nlp_glue_strength_Total;
  }
  else if (json_is_string(json_glue_strength))
  {
    const char *string_glue_strength = json_string_value(json_glue_strength);
    if (!Ogstricmp(string_glue_strength,"total")) expression->glue_strength = nlp_glue_strength_Total;
    else if (!Ogstricmp(string_glue_strength,"punctuation")) expression->glue_strength = nlp_glue_strength_Punctuation;
    else
    {
      NlpThrowErrorTh(ctrl_nlp_th, "NlpCompilePackageExpression: glue_strength value '%s' is not valid",string_glue_strength);
      DPcErr;
    }
  }
  else if (json_is_null(json_glue_strength))
  {
    expression->glue_strength = nlp_glue_strength_Total;
  }
  else
  {
    NlpThrowErrorTh(ctrl_nlp_th, "NlpCompilePackageExpression: glue_strength is not a string");
    DPcErr;
  }


  if (json_glue_distance == NULL)
  {
    expression->glue_distance = DOgNlpDefaultGlueDistance;
  }
  else if (json_is_integer(json_glue_distance))
  {
    expression->glue_distance = json_integer_value(json_glue_distance);
  }
  else
  {
    NlpThrowErrorTh(ctrl_nlp_th, "NlpCompilePackageExpression: glue_distance is not an integer");
    DPcErr;
  }

  IFN(json_aliases)
  {
    expression->alias_start = (-1);
    expression->aliases_nb = 0;
  }
  else
  {
    if (json_is_array(json_aliases))
    {
      IFE(NlpCompilePackageExpressionAliases(ctrl_nlp_th, package, expression, json_aliases));
    }
    else
    {
      NlpThrowErrorTh(ctrl_nlp_th, "NlpCompilePackageExpression: expression 'aliases' is not an array");
      DPcErr;
    }

  }

  expression->json_solution = json_solution;

  DONE;
}

static int NlpCompilePackageExpressionAliases(og_nlp_th ctrl_nlp_th, package_t package,
    struct expression_compile *expression, json_t *json_aliases)
{

  expression->alias_start = (-1);
  expression->aliases_nb = 0;

  int array_size = json_array_size(json_aliases);
  for (int i = 0; i < array_size; i++)
  {
    json_t *json_alias = json_array_get(json_aliases, i);
    if (json_is_object(json_alias))
    {
      IFE(NlpCompilePackageExpressionAlias(ctrl_nlp_th, package, expression, json_alias));
    }
    else
    {
      NlpThrowErrorTh(ctrl_nlp_th,
          "NlpCompilePackageExpressionAliases: structure error : json_alias at position %d is not an object", i);
      DPcErr;
    }
  }

  DONE;
}

static int NlpCompilePackageExpressionAlias(og_nlp_th ctrl_nlp_th, package_t package,
    struct expression_compile *expression, json_t *json_alias)
{

  og_char_buffer json_alias_string[DPcPathSize];
  IFE(NlpJsonToBuffer(json_alias, json_alias_string, DPcPathSize, NULL, JSON_INDENT(2)));

  NlpLog(DOgNlpTraceCompile, "NlpCompilePackageExpressionAlias: compiling alias [\n%s]", json_alias_string)

  size_t Ialias;
  struct alias_compile *alias = OgHeapNewCell(package->halias_compile, &Ialias);
  IFn(alias) DPcErr;
  IFE(Ialias);

  if (expression->aliases_nb == 0)
  {
    expression->alias_start = Ialias;
  }
  expression->aliases_nb++;

  json_t *json_alias_name = NULL;
  json_t *json_slug = NULL;
  json_t *json_id = NULL;
  json_t *json_package = NULL;
  json_t *json_alias_type = NULL;
  json_t *json_regex = NULL;

  for (void *iter = json_object_iter(json_alias); iter; iter = json_object_iter_next(json_alias, iter))
  {
    const char *key = json_object_iter_key(iter);
    NlpLog(DOgNlpTraceCompile, "NlpCompilePackageExpressionAlias: found key='%s'", key)
    if (Ogstricmp(key, "alias") == 0)
    {
      json_alias_name = json_object_iter_value(iter);
    }
    else if (Ogstricmp(key, "slug") == 0)
    {
      json_slug = json_object_iter_value(iter);
    }
    else if (Ogstricmp(key, "id") == 0)
    {
      json_id = json_object_iter_value(iter);
    }
    else if (Ogstricmp(key, "package") == 0)
    {
      json_package = json_object_iter_value(iter);
    }
    else if (Ogstricmp(key, "type") == 0)
    {
      json_alias_type = json_object_iter_value(iter);
    }
    else if (Ogstricmp(key, "regex") == 0)
    {
      json_regex = json_object_iter_value(iter);
    }
    else
    {
      NlpThrowErrorTh(ctrl_nlp_th, "NlpCompilePackageExpressionAlias: unknown key '%s'", key);
      DPcErr;
    }
  }

  if (json_is_string(json_alias_name))
  {
    const char *string_alias = json_string_value(json_alias_name);
    alias->alias_start = OgHeapGetCellsUsed(package->halias_ba);
    alias->alias_length = strlen(string_alias);
    if (alias->alias_length > DOgNlpInterpretationExpressionMaxLength)
    {
      NlpThrowErrorTh(ctrl_nlp_th, "NlpCompilePackageExpressionAlias: alias is too long");
      DPcErr;
    }
    IFE(OgHeapAppend(package->halias_ba, alias->alias_length + 1, string_alias));
  }
  else
  {
    NlpThrowErrorTh(ctrl_nlp_th, "NlpCompilePackageExpressionAlias: text is not a string");
    DPcErr;
  }

  alias->type = nlp_alias_type_Interpretation;
  if (json_alias_type != NULL)
  {
    if (json_is_string(json_alias_type))
    {
      const char *string_alias_type = json_string_value(json_alias_type);
      if (!Ogstricmp(string_alias_type, "any")) alias->type = nlp_alias_type_Any;
      else if (!Ogstricmp(string_alias_type, "number")) alias->type = nlp_alias_type_Number;
      else if (!Ogstricmp(string_alias_type, "regex")) alias->type = nlp_alias_type_Regex;
      else
      {
        NlpThrowErrorTh(ctrl_nlp_th, "NlpCompilePackageExpressionAlias: unknown type '%s'", string_alias_type);
        DPcErr;
      }
    }
    else
    {
      NlpThrowErrorTh(ctrl_nlp_th, "NlpCompilePackageExpressionAlias: type is not a string");
      DPcErr;
    }
  }

  if (json_regex != NULL)
  {
    if (json_is_string(json_regex))
    {
      const char *string_regex = json_string_value(json_regex);
      alias->regex_start = OgHeapGetCellsUsed(package->halias_ba);
      alias->regex_length = strlen(string_regex);
      if (alias->regex_length > DOgNlpInterpretationRegexMaxLength)
      {
        NlpThrowErrorTh(ctrl_nlp_th, "NlpCompilePackageExpressionAlias: regex is too long");
        DPcErr;
      }
      IFE(OgHeapAppend(package->halias_ba, alias->regex_length + 1, string_regex));
    }
    else
    {
      NlpThrowErrorTh(ctrl_nlp_th, "NlpCompilePackageExpressionAlias: regex is not a string");
      DPcErr;
    }
  }

  if (alias->type == nlp_alias_type_Interpretation)
  {
    if (json_slug == NULL)
    {
      const char *string_slug = json_string_value(json_id);
      alias->slug_start = OgHeapGetCellsUsed(package->halias_ba);
      alias->slug_length = strlen(string_slug);
      if (alias->slug_length > DOgNlpInterpretationExpressionMaxLength)
      {
        NlpThrowErrorTh(ctrl_nlp_th, "NlpCompilePackageExpressionAlias: slug is too long");
        DPcErr;
      }
      IFE(OgHeapAppend(package->halias_ba, alias->slug_length + 1, string_slug));

    }
    else if (json_is_string(json_slug))
    {
      const char *string_slug = json_string_value(json_slug);
      alias->slug_start = OgHeapGetCellsUsed(package->halias_ba);
      alias->slug_length = strlen(string_slug);
      if (alias->slug_length > DOgNlpInterpretationExpressionMaxLength)
      {
        NlpThrowErrorTh(ctrl_nlp_th, "NlpCompilePackageExpressionAlias: slug is too long");
        DPcErr;
      }
      IFE(OgHeapAppend(package->halias_ba, alias->slug_length + 1, string_slug));
    }
    else
    {
      NlpThrowErrorTh(ctrl_nlp_th, "NlpCompilePackageExpressionAlias: slug is not a string");
      DPcErr;
    }

    if (json_is_string(json_id))
    {
      const char *string_id = json_string_value(json_id);
      alias->id_start = OgHeapGetCellsUsed(package->halias_ba);
      alias->id_length = strlen(string_id);
      if (alias->id_length > DOgNlpInterpretationExpressionMaxLength)
      {
        NlpThrowErrorTh(ctrl_nlp_th, "NlpCompilePackageExpressionAlias: id is too long");
        DPcErr;
      }
      IFE(OgHeapAppend(package->halias_ba, alias->id_length + 1, string_id));
    }
    else
    {
      NlpThrowErrorTh(ctrl_nlp_th, "NlpCompilePackageExpressionAlias: id is not a string");
      DPcErr;
    }

    if (json_is_string(json_package) || json_package == NULL)
    {
      const char *string_package = NULL;
      if (json_package == NULL) string_package = package->id;
      else string_package = json_string_value(json_package);
      alias->package_id_start = OgHeapGetCellsUsed(package->halias_ba);
      alias->package_id_length = strlen(string_package);
      if (alias->package_id_length > DOgNlpInterpretationExpressionMaxLength)
      {
        NlpThrowErrorTh(ctrl_nlp_th, "NlpCompilePackageExpressionAlias: package is too long");
        DPcErr;
      }
      IFE(OgHeapAppend(package->halias_ba, alias->package_id_length + 1, string_package));
    }
    else
    {
      NlpThrowErrorTh(ctrl_nlp_th, "NlpCompilePackageExpressionAlias: package is not a string");
      DPcErr;
    }
  }
  else
  {
    if (json_slug != NULL)
    {
      NlpThrowErrorTh(ctrl_nlp_th, "NlpCompilePackageExpressionAlias: alias of type '%s' should not have a slug",
          NlpAliasTypeString(alias->type));
      DPcErr;
    }
    if (json_id != NULL)
    {
      NlpThrowErrorTh(ctrl_nlp_th,
          "NlpCompilePackageExpressionAlias: alias of type '%s' should not have an interpretation id",
          NlpAliasTypeString(alias->type));
      DPcErr;
    }
    if (json_package != NULL)
    {
      NlpThrowErrorTh(ctrl_nlp_th,
          "NlpCompilePackageExpressionAlias: alias of type '%s' should not have an interpretation package",
          NlpAliasTypeString(alias->type));
      DPcErr;
    }
    alias->slug_start = (-1);
    alias->slug_length = 0;
    alias->id_start = (-1);
    alias->id_length = 0;
    alias->package_id_start = (-1);
    alias->package_id_length = 0;

  }
  DONE;
}

