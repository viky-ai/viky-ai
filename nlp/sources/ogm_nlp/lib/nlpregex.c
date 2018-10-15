/*
 *  Handling regex expressions for NLP
 *  Copyright (c) 2018 Pertimm, by Patrick Constant & Sebastien Manfredini
 *  Dev : November 2018
 *  Version 1.0
 */
#include "ogm_nlp.h"

static og_status NlpRegexBuildInterpretation(og_nlp_th ctrl_nlp_th, package_t package,
    struct interpretation *interpretation);
static og_status NlpRegexBuildExpression(og_nlp_th ctrl_nlp_th, package_t package, struct expression *expression);
static og_status NlpRegexBuildAlias(og_nlp_th ctrl_nlp_th, package_t package, struct alias *alias);

og_status NlpRegexInit(og_nlp_th ctrl_nlp_th, package_t package)
{
  og_char_buffer heap_name[DPcPathSize];
  void *hmsg = ctrl_nlp_th->ctrl_nlp->hmsg;
  snprintf(heap_name, DPcPathSize, "regex_%s", package->id);
  IFn(package->hregex = OgHeapInit(hmsg, heap_name, sizeof(struct regex), 1)) DPcErr;
  DONE;
}

og_status NlpRegexFlush(package_t package)
{
  IFE(OgHeapFlush(package->hregex));
  DONE;
}

og_status NlpRegexBuildPackage(og_nlp_th ctrl_nlp_th, package_t package)
{
  int interpretation_used = OgHeapGetCellsUsed(package->hinterpretation);
  for (int i = 0; i < interpretation_used; i++)
  {
    struct interpretation *interpretation = OgHeapGetCell(package->hinterpretation, i);
    IFN(interpretation) DPcErr;

    IFE(NlpRegexBuildInterpretation(ctrl_nlp_th, package, interpretation));
  }

  NlpRegexPackageLog(ctrl_nlp_th, package);

  DONE;
}

static og_status NlpRegexBuildInterpretation(og_nlp_th ctrl_nlp_th, package_t package,
    struct interpretation *interpretation)
{
  IFN(interpretation) DPcErr;

  for (int i = 0; i < interpretation->expressions_nb; i++)
  {
    IFE(NlpRegexBuildExpression(ctrl_nlp_th, package, interpretation->expressions + i));
  }

  DONE;
}

static og_status NlpRegexBuildExpression(og_nlp_th ctrl_nlp_th, package_t package, struct expression *expression)
{
  IFN(expression) DPcErr;

  for (int i = 0; i < expression->aliases_nb; i++)
  {
    IFE(NlpRegexBuildAlias(ctrl_nlp_th, package, expression->aliases + i));
  }

  DONE;
}

static og_status NlpRegexBuildAlias(og_nlp_th ctrl_nlp_th, package_t package, struct alias *alias)
{
  IFN(alias) DPcErr;
  if (alias->type != nlp_alias_type_Regex) DONE;

  int regex_used = OgHeapGetCellsUsed(package->hregex);
  if (regex_used >= DOgNlpMaximumRegex)
  {
    NlpThrowErrorTh(ctrl_nlp_th, "NlpRegexBuildAlias: maximum regex number reached: %d", regex_used);
    DPcErr;
  }

  OgMsg(ctrl_nlp_th->hmsg, "", DOgMsgDestInLog, "NlpRegexBuildAlias: found alias '%s' %s='%s'", alias->alias,
      NlpAliasTypeString(alias->type), alias->regex);

  struct regex regex[1];
  regex->alias = alias;

  // TODO SMA Compile regex here, remonter en erreur si ça compile pas

  IFE(OgHeapAppend(package->hregex, 1, regex));

  DONE;
}

og_status NlpRegexPackageLog(og_nlp_th ctrl_nlp_th, package_t package)
{
  int regex_used = OgHeapGetCellsUsed(package->hregex);
  struct regex *regexes = OgHeapGetCell(package->hregex,0);

  for (int i = 0; i < regex_used; i++)
  {
    struct regex *regex = regexes + i;
    struct alias *alias = regex->alias;
    OgMsg(ctrl_nlp_th->hmsg, "", DOgMsgDestInLog, "regex '%s' '%s'", alias->alias, alias->regex);
  }
  DONE;
}





