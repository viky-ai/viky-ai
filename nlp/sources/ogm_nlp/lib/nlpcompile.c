/*
 *  Main function for Natural Language Processing library
 *  Copyright (c) 2017 Pertimm, by Patrick Constant
 *  Dev : September 2017
 *  Version 1.0
 */
#include "ogm_nlp.h"

static int NlpCompilePackageInterpretations(og_nlp_th ctrl_nlp_th, struct og_nlp_compile_input *input, json_t *json_id,
    json_t *json_slug, json_t *json_interpretations);

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
  IFE(NlpJsonToBuffer(input->json_input, json_compile_request_string, DOgMlogMaxMessageSize / 2, NULL));

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
  json_t *json = json_object();
  output->json_output = json;

  IFE(json_object_set_new(json, "compilation", json_string("ok")));

  DONE;
}

og_status NlpCompilePackage(og_nlp_th ctrl_nlp_th, struct og_nlp_compile_input *input, json_t *json_package)
{
  og_char_buffer json_package_string[DPcPathSize];
  IFE(NlpJsonToBuffer(json_package, json_package_string, DPcPathSize, NULL));

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
    NlpThrowErrorTh(ctrl_nlp_th, "NlpCompilePackage: package without is");
    DPcErr;
  }
  if (!json_is_string(json_id))
  {
    NlpThrowErrorTh(ctrl_nlp_th, "NlpCompilePackage: package 'id' is not a string");
    DPcErr;
  }

  IFN(json_slug)
  {
    NlpThrowErrorTh(ctrl_nlp_th, "NlpCompilePackage: package without slug");
    DPcErr;
  }
  if (!json_is_string(json_slug))
  {
    NlpThrowErrorTh(ctrl_nlp_th, "NlpCompilePackage: package 'slug' is not a string");
    DPcErr;
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
  IFN(package) DPcErr;

  int array_size = json_array_size(json_interpretations);
  for (int i = 0; i < array_size; i++)
  {
    json_t *json_interpretation = json_array_get(json_interpretations, i);
    if (json_is_object(json_interpretation))
    {
      IFE(NlpCompilePackageInterpretation(ctrl_nlp_th, package, json_interpretation));
    }
    else
    {
      NlpThrowErrorTh(ctrl_nlp_th,
          "NlpCompilePackageinterpretations: json_interpretation at position %d is not an object", i);
      DPcErr;
    }
  }

  if (ctrl_nlp_th->loginfo->trace & DOgNlpTraceCompile)
  {
    IFE(NlpPackageCompileLog(ctrl_nlp_th, package));
  }

  // freeze memory structure
  IFE(NlpConsolidatePackage(ctrl_nlp_th, package));

  // publish package
  IFE(NlpPackageAddOrReplace(ctrl_nlp_th, package));

  DONE;
}

og_status NlpCompilePackageInterpretation(og_nlp_th ctrl_nlp_th, package_t package, json_t *json_interpretation)
{
  og_char_buffer json_interpretation_string[DPcPathSize];
  IFE(NlpJsonToBuffer(json_interpretation, json_interpretation_string, DPcPathSize, NULL));

  NlpLog(DOgNlpTraceCompile, "NlpCompilePackageInterpretation: compiling interpretation [\n%s]",
      json_interpretation_string)

  json_t *json_id = NULL;
  json_t *json_slug = NULL;
  json_t *json_expressions = NULL;
  json_t *json_solution = NULL;

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
    else if (Ogstricmp(key, "expressions") == 0)
    {
      json_expressions = json_object_iter_value(iter);
    }
    else if (Ogstricmp(key, "solution") == 0)
    {
      json_solution = json_object_iter_value(iter);
    }
    else
    {
      NlpThrowErrorTh(ctrl_nlp_th, "NlpCompilePackageInterpretation: unknow key '%s'", key);
      DPcErr;
    }

  }

  // interpretation id is mandatory
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

  IFN(json_slug)
  {
    NlpThrowErrorTh(ctrl_nlp_th, "NlpCompilePackageInterpretation: interpretation has no 'slug'");
    DPcErr;
  }
  if (!json_is_string(json_slug))
  {
    NlpThrowErrorTh(ctrl_nlp_th, "NlpCompilePackageInterpretation: interpretation 'slug' is not a string");
    DPcErr;
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

  // solution can be of any json type and can be non existant

  // At that point, we can create the interpretation structure
  const char *string_id = json_string_value(json_id);
  const char *string_slug = json_string_value(json_slug);

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

  IFE(NlpCompilePackageExpressions(ctrl_nlp_th, package, interpretation, json_expressions));

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
  IFE(NlpJsonToBuffer(json_expression, json_expression_string, DPcPathSize, NULL));
  NlpLog(DOgNlpTraceCompile, "NlpCompilePackageExpression: compiling expression [\n%s]", json_expression_string)

  json_t *json_text = NULL;
  json_t *json_aliases = NULL;
  json_t *json_locale = NULL;
  json_t *json_solution = NULL;

  for (void *iter = json_object_iter(json_expression); iter; iter = json_object_iter_next(json_expression, iter))
  {
    const char *key = json_object_iter_key(iter);
    NlpLog(DOgNlpTraceCompile, "NlpCompilePackageExpression: found key='%s'", key)

    if (Ogstricmp(key, "expression") == 0)
    {
      json_text = json_object_iter_value(iter);
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
      NlpThrowErrorTh(ctrl_nlp_th, "NlpCompilePackageExpression: unknow key '%s'", key);
      DPcErr;
    }
  }

  IFN(json_text)
  {
    NlpThrowErrorTh(ctrl_nlp_th, "NlpCompilePackageExpression: no text");
    DPcErr;
  }

  size_t Iphrase;
  struct expression_compile *expression = OgHeapNewCell(package->hexpression_compile, &Iphrase);
  IFn(expression) DPcErr;
  IFE(Iphrase);

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

  expression->locale = 0;
  if (json_locale == NULL)
  {
    expression->locale = 0;
  }
  else if (json_is_string(json_locale))
  {
    const char *string_locale = json_string_value(json_locale);
    IFE(expression->locale = OgCodeToIso639_3166(ctrl_nlp_th->herr, (char * )string_locale));
  }
  else if (json_is_null(json_locale))
  {
    expression->locale = 0;
  }
  else
  {
    NlpThrowErrorTh(ctrl_nlp_th, "NlpCompilePackageExpression: locale is not a string");
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
  IFE(NlpJsonToBuffer(json_alias, json_alias_string, DPcPathSize, NULL));

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

  if (json_is_string(json_slug))
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

  if (json_is_string(json_package))
  {
    const char *string_package = json_string_value(json_package);
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

  DONE;
}

