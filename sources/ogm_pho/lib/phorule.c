/*
 *  Rules allocation for ogm_rule functions
 *  Copyright (c) 2006 Pertimm by P.Constant,G.Logerot and L.Rigouste
 *  Dev : December 2006
 *  Version 1.0
 */
#include "ogm_pho.h"

int AllocRule(struct lang_context *lang_context, struct rule **prule)
{

  struct og_ctrl_pho *ctrl_pho = lang_context->ctrl_pho;

  struct rule *rule = 0;
  int i = lang_context->RuleNumber;

  beginAllocRule:

  if (lang_context->RuleUsed < lang_context->RuleNumber)
  {
    i = lang_context->RuleUsed++;
  }

  if (i == lang_context->RuleNumber)
  {
    unsigned a, b;
    struct rule *og_l;

    if (ctrl_pho->loginfo->trace & DOgPhoTraceMemory)
    {
      OgMsg(ctrl_pho->hmsg, "", DOgMsgDestInLog, "AllocRule: max Rule number (%d) reached", lang_context->RuleNumber);
    }
    a = lang_context->RuleNumber;
    b = a + (a >> 2) + 1;
    IFn(og_l=(struct rule *)malloc(b*sizeof(struct rule)))
    {
      char erreur[DOgErrorSize];
      sprintf(erreur, "AllocRule: malloc error on Rule");
      OgErr(ctrl_pho->herr, erreur);
      DPcErr;
    }

    memcpy(og_l, lang_context->Rule, a * sizeof(struct rule));
    DPcFree(lang_context->Rule);
    lang_context->Rule = og_l;
    lang_context->RuleNumber = b;

    if (ctrl_pho->loginfo->trace & DOgPhoTraceMemory)
    {
      OgMsg(ctrl_pho->hmsg, "", DOgMsgDestInLog, "AllocRule: new Rule number is %d\n", lang_context->RuleNumber);
    }

#ifdef DOgNoMainBufferReallocation
    sprintf(erreur,"AllocRule: RuleNumber reached (%d)",lang_context->RuleNumber);
    OgErr(ctrl_pho->herr,erreur); DPcErr;
#endif

    goto beginAllocRule;
  }

  rule = lang_context->Rule + i;
  memset(rule, 0, sizeof(struct rule));

  if (prule) *prule = rule;
  return (i);
}
