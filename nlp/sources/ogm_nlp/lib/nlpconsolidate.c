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
static og_status NlpConsolidateGetCurrentAliasNb(og_nlp_th ctrl_nlp_th, package_t package,
    struct expression *expression, int *palias_nb);

static og_status NlpConsolidatePrepareInterpretation(og_nlp_th ctrl_nlp_th, package_t package)
{
  // freeze ba heap
  IFE(OgHeapFreeze(package->hinterpretation_ba));

  // prealloc heap to avoid realloc
  int interpretation_compile_used = OgHeapGetCellsUsed(package->hinterpretation_compile);
  IFE(interpretation_compile_used);
  if (interpretation_compile_used > 0)
  {
    IFE(OgHeapAddCells(package->hinterpretation, interpretation_compile_used));
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

    interpretation->scope = interpretation_compile->scope;

    interpretation->json_solution = json_deep_copy(interpretation_compile->json_solution);
    interpretation_compile->json_solution = NULL;

    interpretation->contexts_nb = interpretation_compile->contexts_nb;
    if (interpretation->contexts_nb > 0)
    {
      interpretation->contexts = OgHeapGetCell(package->hcontext, interpretation_compile->context_start);
      IFN(interpretation->contexts) DPcErr;
    }
    else
    {
      interpretation->contexts = NULL;
    }

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
    interpretation->is_recursive = FALSE;
  }

  // free compile heap
  IFE(OgHeapFlush(package->hinterpretation_compile));
  package->hinterpretation_compile = NULL;

  // freeze final heap
  IFE(OgHeapFreeze(package->hinterpretation));

  DONE;
}

static og_status NlpConsolidatePrepareContext(og_nlp_th ctrl_nlp_th, package_t package)
{
  // freeze ba heap
  IFE(OgHeapFreeze(package->hcontext_ba));

  // prealloc heap to avoid realloc
  int context_compile_used = OgHeapGetCellsUsed(package->hcontext_compile);
  IFE(context_compile_used);
  if (context_compile_used > 0)
  {
    IFE(OgHeapAddCells(package->hcontext, context_compile_used));
  }

  // convert _compile heaps to simple one
  struct context_compile *all_context_compile = OgHeapGetCell(package->hcontext_compile, 0);
  struct context *all_context = OgHeapGetCell(package->hcontext, 0);
  for (int i = 0; i < context_compile_used; i++)
  {
    struct context_compile *context_compile = all_context_compile + i;
    struct context *context = all_context + i;

    context->flag = OgHeapGetCell(package->hcontext_ba, context_compile->flag_start);
    IFN(context->flag) DPcErr;
  }

  // free compile heap
  IFE(OgHeapFlush(package->hcontext_compile));
  package->hcontext_compile = NULL;

  // freeze final heap
  IFE(OgHeapFreeze(package->hcontext));

  DONE;
}

static og_status NlpConsolidatePrepareExpression(og_nlp_th ctrl_nlp_th, package_t package)
{
  // freeze ba heap
  IFE(OgHeapFreeze(package->hexpression_ba));

  // prealloc heap to avoid realloc
  int expression_compile_used = OgHeapGetCellsUsed(package->hexpression_compile);
  IFE(expression_compile_used);
  if (expression_compile_used > 0)
  {
    IFE(OgHeapAddCells(package->hexpression, expression_compile_used));
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

    expression->keep_order = expression_compile->keep_order;
    expression->glued = expression_compile->glued;

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

    // solution needs to be copied from the request
    expression->json_solution = json_deep_copy(expression_compile->json_solution);
    expression_compile->json_solution = NULL;

    expression->is_recursive = FALSE;
  }

  // free compile heap
  IFE(OgHeapFlush(package->hexpression_compile));
  package->hexpression_compile = NULL;

  // freeze final heap
  IFE(OgHeapFreeze(package->hexpression));

  DONE;
}

static og_status NlpConsolidatePrepareAlias(og_nlp_th ctrl_nlp_th, package_t package)
{
  // freeze ba heap
  IFE(OgHeapFreeze(package->halias_ba));

  // prealloc heap to avoid realloc
  int alias_compile_used = OgHeapGetCellsUsed(package->halias_compile);
  IFE(alias_compile_used);
  if (alias_compile_used > 0)
  {
    IFE(OgHeapAddCells(package->halias, alias_compile_used));
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

    alias->type = alias_compile->type;
    if (alias->type == nlp_alias_type_Interpretation)
    {
      alias->slug = OgHeapGetCell(package->halias_ba, alias_compile->slug_start);
      IFN(alias->slug) DPcErr;

      alias->id = OgHeapGetCell(package->halias_ba, alias_compile->id_start);
      IFN(alias->id) DPcErr;

      alias->package_id = OgHeapGetCell(package->halias_ba, alias_compile->package_id_start);
      IFN(alias->package_id) DPcErr;
    }
    else if (alias->type == nlp_alias_type_Regex)
    {
      alias->regex = OgHeapGetCell(package->halias_ba, alias_compile->regex_start);
    }

    else
    {
      alias->slug = NULL;
      alias->id = NULL;
      alias->package_id = NULL;
    }
  }

  // free compile heap
  IFE(OgHeapFlush(package->halias_compile));
  package->halias_compile = NULL;

  // freeze final heap
  IFE(OgHeapFreeze(package->halias));

  DONE;
}

static og_status NlpConsolidatePrepare(og_nlp_th ctrl_nlp_th, package_t package)
{
  IFE(NlpConsolidatePrepareAlias(ctrl_nlp_th, package));

  IFE(NlpConsolidatePrepareExpression(ctrl_nlp_th, package));

  IFE(NlpConsolidatePrepareContext(ctrl_nlp_th, package));

  IFE(NlpConsolidatePrepareInterpretation(ctrl_nlp_th, package));

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
    if (expression->input_part_start >= 0)
    {
      expression->input_parts = OgHeapGetCell(package->hinput_part, expression->input_part_start);
      IFN(expression->input_parts) DPcErr;
    }
    else
    {
      expression->input_parts = NULL;
    }
  }

  IFE(OgAuf(package->ha_interpretation_id, FALSE));
  IFE(OgAufClean(package->ha_interpretation_id));

  IFE(OgAuf(package->ha_word, FALSE));
  IFE(OgAufClean(package->ha_word));

  package->consolidate_done = TRUE;

  IFE(NlpLtracPackage(ctrl_nlp_th, package));
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
    IFE(NlpNumberInputPartLog(ctrl_nlp_th, package));
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
  expression->alias_any_input_part_position = -1;

  og_string s = expression->text;
  int is = strlen(s);
  og_string s_end = s + is;

  NlpLog(DOgNlpTraceConsolidate, "NlpConsolidateExpression: parsing '%s'", s);

  int state = 1;
  og_bool end = FALSE;
  int i = -1;
  int start = 0;
  while (!end)
  {
    gunichar c = ' ';
    og_string s_pos = g_utf8_find_next_char(s + i, s_end);
    if (s_pos == NULL)
    {
      end = TRUE;
      i = is;
    }
    else
    {
      i = s_pos - s;
      c = g_utf8_get_char_validated(s_pos, is - i);
      if ((c == (gunichar) -1) || (c == (gunichar) -2))
      {
        NlpThrowErrorTh(ctrl_nlp_th, "NlpConsolidateExpression : invalid UTF-8 character '%s' '%s' : '%s'",
            interpretation->slug, interpretation->id, s_pos);
        DPcErr;
      }
    }

    switch (state)
    {
      case 1:   // between words or interpretations
      {
        if (c == '@' && i + 1 < is && s[i + 1] == '{')
        {
          state = 2;
        }
        else
        {
          og_bool punct_length = 0;
          og_bool is_skipped = FALSE;
          og_bool is_punct = NlpParseIsPunctuation(ctrl_nlp_th, is - i, s + i, &is_skipped, &punct_length);
          IFE(is_punct);
          if (is_punct)
          {
            if (!is_skipped)
            {
              // add punctuation word
              IFE(NlpConsolidateAddWord(ctrl_nlp_th, package, expression, s + i, punct_length));
              state = 1;
            }
            i += punct_length - 1;
            state = 1;
          }
          else if (g_unichar_isdigit(c))
          {
            start = i;
            state = 5;
          }
          else
          {
            start = i;
            state = 4;
          }
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
        og_bool punct_length = 0;
        og_bool is_skip = FALSE;
        og_bool is_punct = NlpParseIsPunctuation(ctrl_nlp_th, is - i, s + i, &is_skip, &punct_length);
        IFE(is_punct);
        if (is_punct)
        {
          // add previously parsed word
          IFE(NlpConsolidateAddWord(ctrl_nlp_th, package, expression, s + start, i - start));

          if (!is_skip)
          {
            // add punctuation word
            IFE(NlpConsolidateAddWord(ctrl_nlp_th, package, expression, s + i, punct_length));
            state = 1;
          }
          i += punct_length - 1;
          state = 1;
        }
        else if (g_unichar_isdigit(c))
        {
          // add previously parsed word
          IFE(NlpConsolidateAddWord(ctrl_nlp_th, package, expression, s + start, i - start));

          i -= 1;
          start = i;
          state = 1;
        }
        break;
      }
      case 5:   // in digit
      {
        if (!g_unichar_isdigit(c))
        {
          // add previously parsed digit word
          IFE(NlpConsolidateAddWord(ctrl_nlp_th, package, expression, s + start, i - start));

          i -= 1;
          start = i;
          state = 1;
        }
        break;
      }
    }
  }

  if (state != 1)
  {
    NlpThrowErrorTh(ctrl_nlp_th, "NlpConsolidateExpression: error parsing expression '%s' : state %d != 1", s, state);
    DPcErr;
  }

  DONE;
}

static og_status NlpConsolidateAddAlias(og_nlp_th ctrl_nlp_th, package_t package, struct expression *expression,
    og_string string_alias, int length_string_alias)
{
  NlpLog(DOgNlpTraceConsolidate, "NlpConsolidateAddAlias: adding alias '%.*s' as input_part", length_string_alias,
      string_alias)

  og_bool alias_added = FALSE;
  for (int i = 0; i < expression->aliases_nb; i++)
  {
    struct alias *alias = expression->aliases + i;

    if (length_string_alias != alias->alias_length) continue;
    if (memcmp(string_alias, alias->alias, length_string_alias)) continue;

    if (alias->type == nlp_alias_type_Interpretation)
    {
      size_t Iinput_part;
      struct input_part *input_part = NlpConsolidateCreateInputPart(ctrl_nlp_th, package, expression, &Iinput_part);
      IFN(input_part) DPcErr;
      input_part->type = nlp_input_part_type_Interpretation;
      input_part->alias = alias;

      if (!strcmp(alias->id, expression->interpretation->id))
      {
        expression->is_recursive = TRUE;
        expression->interpretation->is_recursive = TRUE;
        // Keep order on a recursive list in not necessary
        // as sorting a a a into a a a does not mean anything
        expression->keep_order = FALSE;
      }

      IFE(NlpInputPartAliasAdd(ctrl_nlp_th, package, alias->id, Iinput_part));
      alias_added = TRUE;
    }
    else if (alias->type == nlp_alias_type_Any)
    {
      // this says that the any alias is before the input_part position expression->input_parts_nb
      // a value of -1 means no alias, a value of zero means before the first input_part
      // a value of n means before the nth input_part (or at the end if it is equal to expression->input_parts_nb
      if (expression->alias_any_input_part_position >= 0)
      {
        NlpLog(DOgNlpTraceConsolidate, "NlpConsolidateAddAlias: alias '%.*s' is a second alias of type any, while"
            " only one alias per expression is allowed in interpretation '%s' '%s'", length_string_alias, string_alias,
            expression->text, expression->interpretation->slug, expression->interpretation->id);
      }

      og_status current_alias_status = NlpConsolidateGetCurrentAliasNb(ctrl_nlp_th, package, expression,
          &expression->alias_any_input_part_position);
      IFE(current_alias_status);

      alias_added = TRUE;
    }
    else if (alias->type == nlp_alias_type_Number)
    {
      size_t Iinput_part;
      struct input_part *input_part = NlpConsolidateCreateInputPart(ctrl_nlp_th, package, expression, &Iinput_part);
      IFN(input_part) DPcErr;
      input_part->type = nlp_input_part_type_Number;
      input_part->alias = alias;

      IFE(NlpInputPartAliasNumberAdd(ctrl_nlp_th, package, Iinput_part));
      alias_added = TRUE;
    }
    else if (alias->type == nlp_alias_type_Regex)
    {
      size_t Iinput_part;
      struct input_part *input_part = NlpConsolidateCreateInputPart(ctrl_nlp_th, package, expression, &Iinput_part);
      IFN(input_part) DPcErr;
      input_part->type = nlp_input_part_type_Regex;
      input_part->alias = alias;

      IFE(NlpInputPartAliasNumberAdd(ctrl_nlp_th, package, Iinput_part));
      alias_added = TRUE;
    }
    break;
  }

  if (!alias_added)
  {
    NlpThrowErrorTh(ctrl_nlp_th, "NlpConsolidateAddAlias: alias '%.*s' not found in expression '%s'"
        " in interpretation '%s' '%s'", length_string_alias, string_alias, expression->text,
        expression->interpretation->slug, expression->interpretation->id);
    DPcErr;
  }

  DONE;
}

static og_status NlpConsolidateAddWord(og_nlp_th ctrl_nlp_th, package_t package, struct expression *expression,
    og_string string_word, int length_string_word)
{

  NlpLog(DOgNlpTraceConsolidate, "NlpConsolidateAddWord: adding word '%.*s' as input_part", length_string_word,
      string_word)

  size_t Iinput_part;
  struct input_part *input_part = NlpConsolidateCreateInputPart(ctrl_nlp_th, package, expression, &Iinput_part);
  IFN(input_part) DPcErr;

  char normalized_string_word[DPcPathSize];
  int length_normalized_string_word = OgUtf8Normalize(length_string_word, string_word, DPcPathSize,
      normalized_string_word);
  NlpLog(DOgNlpTraceConsolidate, "NlpConsolidateAddWord: normalized word '%.*s'", length_normalized_string_word,
      normalized_string_word)

  input_part->type = nlp_input_part_type_Word;

  input_part->word->word_start = OgHeapGetCellsUsed(package->hinput_part_ba);
  IFE(OgHeapAppend(package->hinput_part_ba, length_normalized_string_word, normalized_string_word));
  IFE(OgHeapAppend(package->hinput_part_ba, 1, ""));

  input_part->word->raw_word_start = OgHeapGetCellsUsed(package->hinput_part_ba);
  IFE(OgHeapAppend(package->hinput_part_ba, length_string_word, string_word));
  IFE(OgHeapAppend(package->hinput_part_ba, 1, ""));

  IFE(NlpInputPartWordAdd(ctrl_nlp_th, package, normalized_string_word, length_normalized_string_word, Iinput_part));

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
  input_part->self_index = Iinput_part;
  input_part->expression = expression;

  if (expression->input_parts_nb == 0)
  {
    expression->input_part_start = Iinput_part;
  }
  expression->input_parts_nb++;

  *pIinput_part = Iinput_part;
  return (input_part);
}

static og_status NlpConsolidateGetCurrentAliasNb(og_nlp_th ctrl_nlp_th, package_t package,
    struct expression *expression, int *palias_nb)
{
  *palias_nb = 0;
  for (int i = 0; i < expression->input_parts_nb; i++)
  {
    struct input_part *input_part = OgHeapGetCell(package->hinput_part, expression->input_part_start + i);
    IFN(input_part) DPcErr;
    if (input_part->type == nlp_input_part_type_Interpretation || input_part->type == nlp_input_part_type_Number)
    {
      (*palias_nb)++;
    }
  }
  DONE;
}

