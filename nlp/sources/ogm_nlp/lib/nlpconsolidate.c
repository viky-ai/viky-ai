/*
 *  Consolidating packages compilation
 *  Copyright (c) 2017 Pertimm, by Patrick Constant
 *  Dev : October 2017
 *  Version 1.0
 */
#include "ogm_nlp.h"

static og_status NlpConsolidateInterpretation(og_nlp_th ctrl_nlp_th, package_t package, int Iinterpretation);
static og_status NlpConsolidateExpression(og_nlp_th ctrl_nlp_th, package_t package,
    struct interpretation *interpretation, struct expression *expression);
static og_status NlpConsolidateAddAlias(og_nlp_th ctrl_nlp_th, package_t package, struct expression *expression,
    og_string string_alias, int length_string_alias);
static og_status NlpConsolidateAddWord(og_nlp_th ctrl_nlp_th, package_t package, struct expression *expression,
    og_string string_word, int length_string_word);
struct input_part *NlpConsolidateCreateInputPart(og_nlp_th ctrl_nlp_th, package_t package,
    struct expression *expression, size_t *pIinput_part);

static og_status NlpConsolidatePrepare(og_nlp_th ctrl_nlp_th, package_t package)
{
//  // freeze ba heap
//  IFE(OgHeapFreeze(package->hinterpretation_ba));
//  IFE(OgHeapFreeze(package->hexpression_ba));
//  IFE(OgHeapFreeze(package->halias_ba));
//  IFE(OgHeapFreeze(package->hinput_part_ba));
//
//  // freeze compile heap
//  IFE(OgHeapFreeze(package->hinterpretation_compile));
//  IFE(OgHeapFreeze(package->hexpression_compile));
//  IFE(OgHeapFreeze(package->halias_compile));

// prealloc heap to avoid realloc
  int interpretation_compile_used = OgHeapGetCellsUsed(package->hinterpretation_compile);
  IFE(interpretation_compile_used);
  if (interpretation_compile_used > 0)
  {
    IFE(OgHeapAddCells(package->hinterpretation, interpretation_compile_used));
  }

  int expression_compile_used = OgHeapGetCellsUsed(package->hexpression_compile);
  IFE(expression_compile_used);
  if (expression_compile_used > 0)
  {
    IFE(OgHeapAddCells(package->hexpression, expression_compile_used));
  }

  int alias_compile_used = OgHeapGetCellsUsed(package->halias_compile);
  IFE(alias_compile_used);
  if (alias_compile_used > 0)
  {
    IFE(OgHeapAddCells(package->halias, alias_compile_used));
  }

  // convert _compile heaps to simple one
  struct interpretation_compile *all_interpretation_compile = OgHeapGetCell(package->hinterpretation_compile, 0);
  struct interpretation *all_interpretation = OgHeapGetCell(package->hinterpretation, 0);
  for (int i = 0; i < interpretation_compile_used; i++)
  {
    struct interpretation_compile *interpretation_compile = all_interpretation_compile + i;
    struct interpretation *interpretation = all_interpretation + i;
    interpretation->package = package;

    interpretation->id = OgHeapGetCell(package->hinterpretation_ba, interpretation_compile->id_start);
    IFN(interpretation->id) DPcErr;

    interpretation->slug = OgHeapGetCell(package->hinterpretation_ba, interpretation_compile->slug_start);
    IFN(interpretation->slug) DPcErr;

    interpretation->expressions_nb = interpretation_compile->expressions_nb;
    if (interpretation->expressions_nb > 0)
    {
      interpretation->expressions = OgHeapGetCell(package->hexpression, interpretation_compile->expression_start);
      IFN(interpretation->expressions) DPcErr;
    }
    else
    {
      interpretation->expressions = NULL;
    }
  }

  // convert _compile heaps to simple one
  struct expression_compile *all_expression_compile = OgHeapGetCell(package->hexpression_compile, 0);
  struct expression *all_expression = OgHeapGetCell(package->hexpression, 0);
  for (int i = 0; i < expression_compile_used; i++)
  {
    struct expression_compile *expression_compile = all_expression_compile + i;
    struct expression *expression = all_expression + i;

    expression->text = OgHeapGetCell(package->hexpression_ba, expression_compile->text_start);
    IFN(expression->text) DPcErr;

    expression->locale = expression_compile->locale;
    expression->aliases_nb = expression_compile->aliases_nb;
    if (expression->aliases_nb > 0)
    {
      expression->aliases = OgHeapGetCell(package->halias, expression_compile->alias_start);
      IFN(expression->aliases) DPcErr;
    }
    else
    {
      expression->aliases = NULL;
    }
  }

  // convert _compile heaps to simple one
  struct alias_compile *all_alias_compile = OgHeapGetCell(package->halias_compile, 0);
  struct alias *all_alias = OgHeapGetCell(package->halias, 0);
  for (int i = 0; i < alias_compile_used; i++)
  {
    struct alias_compile *alias_compile = all_alias_compile + i;
    struct alias *alias = all_alias + i;

    alias->alias_length = alias_compile->alias_length;
    alias->alias = OgHeapGetCell(package->halias_ba, alias_compile->alias_start);
    IFN(alias->alias) DPcErr;

    alias->slug = OgHeapGetCell(package->halias_ba, alias_compile->slug_start);
    IFN(alias->slug) DPcErr;

    alias->id = OgHeapGetCell(package->halias_ba, alias_compile->id_start);
    IFN(alias->id) DPcErr;

    alias->package_id = OgHeapGetCell(package->halias_ba, alias_compile->package_id_start);
    IFN(alias->package_id) DPcErr;
  }

  // free used heap
  IFE(OgHeapFlush(package->hinterpretation_compile));
  package->hinterpretation_compile = NULL;
  IFE(OgHeapFlush(package->hexpression_compile));
  package->hexpression_compile = NULL;
  IFE(OgHeapFlush(package->halias_compile));
  package->halias_compile = NULL;

//  // freeze final heap
//  IFE(OgHeapFreeze(package->hinterpretation));
//  IFE(OgHeapFreeze(package->hexpression));
//  IFE(OgHeapFreeze(package->halias));
//  IFE(OgHeapFreeze(package->hinput_part));

  DONE;
}

static og_status NlpConsolidateFinalize(og_nlp_th ctrl_nlp_th, package_t package)
{
  int expression_used = OgHeapGetCellsUsed(package->hexpression);
  struct expression *all_expression = OgHeapGetCell(package->hexpression, 0);
  for (int i = 0; i < expression_used; i++)
  {
    struct expression *expression = all_expression + i;

    // resolve input_part_start to pointer
    expression->input_parts = OgHeapGetCell(package->hinput_part, expression->input_part_start);
    IFN(expression->input_parts) DPcErr;
  }

  package->consolidate_done = TRUE;

  DONE;
}

og_status NlpConsolidatePackage(og_nlp_th ctrl_nlp_th, package_t package)
{

  IFE(NlpConsolidatePrepare(ctrl_nlp_th, package));

  if (ctrl_nlp_th->loginfo->trace & DOgNlpTraceConsolidate)
  {
    IFE(NlpPackageLog(ctrl_nlp_th, "before consolidate", package));
  }

  int interpretation_used = OgHeapGetCellsUsed(package->hinterpretation);
  for (int i = 0; i < interpretation_used; i++)
  {
    IFE(NlpConsolidateInterpretation(ctrl_nlp_th, package, i));
  }

  IFE(NlpConsolidateFinalize(ctrl_nlp_th, package));

  if (ctrl_nlp_th->loginfo->trace & DOgNlpTraceConsolidate)
  {
    IFE(NlpPackageLog(ctrl_nlp_th, "after consolidate", package));
    IFE(NlpInputPartWordLog(ctrl_nlp_th, package));
    IFE(NlpInputPartAliasLog(ctrl_nlp_th, package));
  }

  DONE;
}

static og_status NlpConsolidateInterpretation(og_nlp_th ctrl_nlp_th, package_t package, int Iinterpretation)
{
  struct interpretation *interpretation = OgHeapGetCell(package->hinterpretation, Iinterpretation);
  IFN(interpretation) DPcErr;
  interpretation->package = package;

  for (int i = 0; i < interpretation->expressions_nb; i++)
  {
    struct expression *expression = interpretation->expressions + i;

    IFE(NlpConsolidateExpression(ctrl_nlp_th, package, interpretation, expression));
  }

  DONE;
}

static og_status NlpConsolidateExpression(og_nlp_th ctrl_nlp_th, package_t package,
    struct interpretation *interpretation, struct expression *expression)
{
  IFN(expression) DPcErr;

  expression->interpretation = interpretation;
  expression->input_parts_nb = 0;
  expression->input_parts = NULL;
  expression->input_part_start = -1;

  og_string s = expression->text;
  int is = strlen(s);

  if (ctrl_nlp_th->loginfo->trace & DOgNlpTraceConsolidate)
  {
    OgMsg(ctrl_nlp_th->hmsg, "", DOgMsgDestInLog, "NlpConsolidateExpression: parsing '%s'", s);
  }

  for (int i = 0, state = 1, start = 0, end = 0; !end; i++)
  {
    int c = ' ';
    if (i < is)
    {
      c = s[i];
    }
    else
    {
      end = 1;
      c = ' ';
    }

    switch (state)
    {
      case 1:   // between words or interpretations
      {
        if (c == ' ')
        {
          state = 1;
        }
        else if (c == '@' && i + 1 < is && s[i + 1] == '{')
        {
          state = 2;
        }
        else
        {
          start = i;
          state = 4;
        }
        break;
      }
      case 2:   // beginning of interpretation interpretation
      {
        if (c == '{')
        {
          start = i + 1;
          state = 3;
        }
        else
        {
          NlpThrowErrorTh(ctrl_nlp_th, "NlpConsolidateExpression: error at position %d in expression '%s'", i, s);
          DPcErr;
        }
        break;
      }
      case 3:   // middle or end of interpretation
      {
        if (c == '}')
        {
          IFE(NlpConsolidateAddAlias(ctrl_nlp_th, package, expression, s + start, i - start));
          state = 1;
        }
        break;
      }
      case 4:   // middle or end of word
      {
        if (c == ' ')
        {
          IFE(NlpConsolidateAddWord(ctrl_nlp_th, package, expression, s + start, i - start));
          state = 1;
        }
        break;
      }
    }
  }

  DONE;
}

static og_status NlpConsolidateAddAlias(og_nlp_th ctrl_nlp_th, package_t package, struct expression *expression,
    og_string string_alias, int length_string_alias)
{
  if (ctrl_nlp_th->loginfo->trace & DOgNlpTraceConsolidate)
  {
    OgMsg(ctrl_nlp_th->hmsg, "", DOgMsgDestInLog, "NlpConsolidateAddAlias: adding alias '%s' as input_part",
        string_alias);
  }

  for (int i = 0; i < expression->aliases_nb; i++)
  {
    struct alias *alias = expression->aliases + i;

    if (length_string_alias != alias->alias_length) continue;
    if (memcmp(string_alias, alias->alias, length_string_alias)) continue;

    size_t Iinput_part;
    struct input_part *input_part = NlpConsolidateCreateInputPart(ctrl_nlp_th, package, expression, &Iinput_part);
    IFN(input_part) DPcErr;
    input_part->type = nlp_input_part_type_Interpretation;
    input_part->alias = alias;

    IFE(NlpInputPartAliasAdd(ctrl_nlp_th, package, alias->id, Iinput_part));

  }

  DONE;
}

static og_status NlpConsolidateAddWord(og_nlp_th ctrl_nlp_th, package_t package, struct expression *expression,
    og_string string_word, int length_string_word)
{
  if (ctrl_nlp_th->loginfo->trace & DOgNlpTraceConsolidate)
  {
    OgMsg(ctrl_nlp_th->hmsg, "", DOgMsgDestInLog, "NlpConsolidateAddWord: adding word '%.*s' as input_part",
        length_string_word, string_word);
  }

  size_t Iinput_part;
  struct input_part *input_part = NlpConsolidateCreateInputPart(ctrl_nlp_th, package, expression, &Iinput_part);
  IFN(input_part) DPcErr;
  input_part->type = nlp_input_part_type_Word;
  input_part->word_start = OgHeapGetCellsUsed(package->hinput_part_ba);
  IFE(OgHeapAppend(package->hinput_part_ba, length_string_word, string_word));
  IFE(OgHeapAppend(package->hinput_part_ba, 1, ""));

  IFE(NlpInputPartWordAdd(ctrl_nlp_th, package, string_word, length_string_word, Iinput_part));

  DONE;
}

struct input_part *NlpConsolidateCreateInputPart(og_nlp_th ctrl_nlp_th, package_t package,
    struct expression *expression, size_t *pIinput_part)
{
  *pIinput_part = (-1);
  size_t Iinput_part;
  struct input_part *input_part = OgHeapNewCell(package->hinput_part, &Iinput_part);
  IFn(input_part) return (NULL);
  IF(Iinput_part) return (NULL);
  input_part->expression = expression;

  if (expression->input_parts_nb == 0)
  {
    expression->input_part_start = Iinput_part;
  }
  expression->input_parts_nb++;

  *pIinput_part = Iinput_part;
  return (input_part);
}

