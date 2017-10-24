/*
 *  Consolidating packages compilation
 *  Copyright (c) 2017 Pertimm, by Patrick Constant
 *  Dev : October 2017
 *  Version 1.0
 */
#include "ogm_nlp.h"

static og_status NlpConsolidateInterpretation(og_nlp_th ctrl_nlp_th, package_t package, int Iinterpretation);
static og_status NlpConsolidateExpression(og_nlp_th ctrl_nlp_th, package_t package, int Iexpression);
static og_status NlpConsolidateAddAlias(og_nlp_th ctrl_nlp_th, package_t package, struct expression *expression,
    og_string string_alias, int length_string_alias);
static og_status NlpConsolidateAddWord(og_nlp_th ctrl_nlp_th, package_t package, struct expression *expression,
    og_string string_word, int length_string_word);
struct input_part *NlpConsolidateCreateInputPart(og_nlp_th ctrl_nlp_th, package_t package,
    struct expression *expression);

PUBLIC(int) OgNlpConsolidate(og_nlp_th ctrl_nlp_th)
{
  GList *key_list = g_hash_table_get_keys(ctrl_nlp_th->ctrl_nlp->packages_hash);

  for (GList *iter = key_list; iter; iter = iter->next)
  {
    og_string package_id = iter->data;

    package_t package = NlpPackageGet(ctrl_nlp_th, package_id);
    IFN(package) DPcErr;

    IFE(NlpConsolidatePackage(ctrl_nlp_th, package));

  }

  DONE;
}

og_status NlpConsolidatePackage(og_nlp_th ctrl_nlp_th, package_t package)
{
  int interpretation_used = OgHeapGetCellsUsed(package->hinterpretation);
  for (int i = 0; i < interpretation_used; i++)
  {
    IFE(NlpConsolidateInterpretation(ctrl_nlp_th, package, i));
  }
  IFE(NlpPackageLog(ctrl_nlp_th, package));
  DONE;
}

static og_status NlpConsolidateInterpretation(og_nlp_th ctrl_nlp_th, package_t package, int Iinterpretation)
{
  struct interpretation *interpretation = OgHeapGetCell(package->hinterpretation, Iinterpretation);
  IFN(interpretation) DPcErr;

  for (int i = 0; i < interpretation->expressions_nb; i++)
  {
    IFE(NlpConsolidateExpression(ctrl_nlp_th, package, interpretation->expression_start + i));
  }

  DONE;
}

static og_status NlpConsolidateExpression(og_nlp_th ctrl_nlp_th, package_t package, int Iexpression)
{
  struct expression *expression = OgHeapGetCell(package->hexpression, Iexpression);
  IFN(expression) DPcErr;

  int is = expression->text_length;
  og_string s = OgHeapGetCell(package->hba, expression->text_start);

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
    OgMsg(ctrl_nlp_th->hmsg, "", DOgMsgDestInLog, "NlpConsolidateAddAlias: adding alias '%.*s' as input_part",
        length_string_alias, string_alias);
  }

  og_string package_id = OgHeapGetCell(package->hba, package->id_start);

  // Fist we check the availability of the alias
  int found = 0;
  for (int i = 0; i < expression->aliases_nb; i++)
  {
    struct alias *alias = OgHeapGetCell(package->halias, expression->alias_start + i);
    IFN(alias) DPcErr;
    og_string string_alias_name = OgHeapGetCell(package->hba, alias->alias_start);
    IFN(string_alias_name) DPcErr;
    if (length_string_alias != alias->alias_length) continue;
    if (memcmp(string_alias, string_alias_name, length_string_alias)) continue;
    // Getting package and interpretation for further use
    og_string alias_package_id = OgHeapGetCell(package->hba, alias->package_start);

    package_t alias_package = 0;
    if (!strcmp(package_id, alias_package_id))
    {
      alias_package = package;
    }
    else
    {
      alias_package = NlpPackageGet(ctrl_nlp_th, alias_package_id);
    }
    IFN(alias_package)
    {
      og_string text = OgHeapGetCell(package->hba, expression->text_start);
      OgMsg(ctrl_nlp_th->hmsg, "", DOgMsgDestInLog,
          "NlpConsolidateAddAlias: alias '%.*s' for expression '%s', cannot be found, expression is disactivated",
          length_string_alias, string_alias, text);
      expression->activated = 0;
      DONE;
    }
    og_string alias_interpretation_id = OgHeapGetCell(package->hba, alias->id_start);
    int interpretation_used = OgHeapGetCellsUsed(alias_package->hinterpretation);
    for (int j = 0; j < interpretation_used; j++)
    {
      struct interpretation *interpretation = OgHeapGetCell(package->hinterpretation, j);
      IFN(interpretation) DPcErr;
      if (interpretation->id_length != alias->id_length) continue;
      og_string interpretation_id = OgHeapGetCell(package->hba, interpretation->id_start);
      IFN(interpretation_id) DPcErr;
      if (memcmp(alias_interpretation_id, interpretation_id, interpretation->id_length)) continue;
      struct input_part *input_part = NlpConsolidateCreateInputPart(ctrl_nlp_th, package, expression);
      IFN(input_part) DPcErr;
      input_part->type = nlp_input_part_type_Interpretation;
      input_part->interpretation_package = alias_package;
      input_part->Iinterpretation = j;
      expression->activated = 1;
      found = 1;
    }
    if (!found)
    {
      og_string text = OgHeapGetCell(package->hba, expression->text_start);
      OgMsg(ctrl_nlp_th->hmsg, "", DOgMsgDestInLog,
          "NlpConsolidateAddAlias: alias '%.*s' for expression '%s', cannot be found, expression is disactivated",
          length_string_alias, string_alias, text);
      expression->activated = 0;
      DONE;
    }

    break;
  }
  if (!found)
  {
    NlpThrowErrorTh(ctrl_nlp_th, "NlpConsolidateAddAlias: alias '%.*s' not found", length_string_alias, string_alias);
    DPcErr;
  }

  DONE;
}

static og_status NlpConsolidateAddWord(og_nlp_th ctrl_nlp_th, package_t package, struct expression *expression,
    og_string string_word, int length_string_word)
{
  if (ctrl_nlp_th->loginfo->trace & DOgNlpTraceConsolidate)
  {
    OgMsg(ctrl_nlp_th->hmsg, "", DOgMsgDestInLog, "NlpConsolidateAddAlias: adding word '%.*s' as input_part",
        length_string_word, string_word);
  }

  struct input_part *input_part = NlpConsolidateCreateInputPart(ctrl_nlp_th, package, expression);
  IFN(input_part) DPcErr;
  input_part->type = nlp_input_part_type_Word;
  input_part->word_start = OgHeapGetCellsUsed(package->hba);
  input_part->word_length = length_string_word;
  IFE(OgHeapAppend(package->hba, input_part->word_length, string_word));
  IFE(OgHeapAppend(package->hba, 1, ""));

  DONE;
}

struct input_part *NlpConsolidateCreateInputPart(og_nlp_th ctrl_nlp_th, package_t package,
    struct expression *expression)
{
  size_t Iinput_part;
  struct input_part *input_part = OgHeapNewCell(package->hinput_part, &Iinput_part);
  IFn(input_part) return (NULL);
  IF(Iinput_part) return (NULL);

  if (expression->input_parts_nb == 0)
  {
    expression->input_part_start = Iinput_part;
  }
  expression->input_parts_nb++;
  return (input_part);
}

