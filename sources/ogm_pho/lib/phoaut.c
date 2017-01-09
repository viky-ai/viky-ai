/*
 *  Automaton for ogm_pho functions
 *  Copyright (c) 2006 Pertimm by P.Constant,G.Logerot and L.Rigouste
 *  Dev : November 2006
 *  Version 1.0
*/
#include "ogm_pho.h"






og_status PhoRulesAutRuleAdd(struct lang_context *lang_context, int iword, char *word, int Irule)
{
  struct rule *rule = lang_context->Rule + Irule;

  /* step:word:Irule tout en Unicode */
  unsigned char buffer[DPcPathSize * 2];
  int ibuffer = 0;
  unsigned char *p = buffer;
  OggNout(rule->step, &p);
  memcpy(p, word, iword);
  p += iword;
  *p++ = 0;
  *p++ = 1;
  OggNout(Irule, &p);
  ibuffer = p - buffer;
  IFE(OgAutAdd(lang_context->ha_rules, ibuffer, buffer));

  DONE;
}

