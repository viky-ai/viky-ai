/*
 *	Rules allocation for ogm_rule functions
 *  Copyright (c) 2006 Pertimm by P.Constant,G.Logerot and L.Rigouste
 *  Dev : December 2006
 *  Version 1.0
*/
#include "ogm_pho.h"





int AllocRule(ctrl_pho,prule)
struct og_ctrl_pho *ctrl_pho;
struct rule **prule;
{
char erreur[DOgErrorSize];
struct rule *rule = 0;
int i=ctrl_pho->RuleNumber;

beginAllocRule:

if (ctrl_pho->RuleUsed < ctrl_pho->RuleNumber) {
  i = ctrl_pho->RuleUsed++; 
  }

if (i == ctrl_pho->RuleNumber) {
  unsigned a, b; struct rule *og_l;

  if (ctrl_pho->loginfo->trace & DOgPhoTraceMemory) {
    OgMsg(ctrl_pho->hmsg,"",DOgMsgDestInLog,"AllocRule: max Rule number (%d) reached", ctrl_pho->RuleNumber);
    }
  a = ctrl_pho->RuleNumber; b = a + (a>>2) + 1;
  IFn(og_l=(struct rule *)malloc(b*sizeof(struct rule))) {
    sprintf(erreur,"AllocRule: malloc error on Rule");
    OgErr(ctrl_pho->herr,erreur); DPcErr;
    }

  memcpy( og_l, ctrl_pho->Rule, a*sizeof(struct rule));
  DPcFree(ctrl_pho->Rule); ctrl_pho->Rule = og_l;
  ctrl_pho->RuleNumber = b;

  if (ctrl_pho->loginfo->trace & DOgPhoTraceMemory) {
    OgMsg(ctrl_pho->hmsg,"",DOgMsgDestInLog,"AllocRule: new Rule number is %d\n", ctrl_pho->RuleNumber);
    }

#ifdef DOgNoMainBufferReallocation
  sprintf(erreur,"AllocRule: RuleNumber reached (%d)",ctrl_pho->RuleNumber);
  OgErr(ctrl_pho->herr,erreur); DPcErr;
#endif

  goto beginAllocRule;
  }

rule = ctrl_pho->Rule + i;
memset(rule,0,sizeof(struct rule));

if (prule) *prule = rule;
return(i);
}
