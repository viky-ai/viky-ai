/*
 *  Rules expansion
 *  Copyright (c) 2008 Pertimm by G.Logerot
 *  Dev : May 2008
 *  Version 1.0
*/
#include "ogm_pho.h"

og_status PhoRulesRuleAddExpense(struct lang_context *lang_context, int iword, char *word, int indice, int Irule)
{

  char buffer[DPcPathSize * 2];
  memcpy(buffer, word, iword);

  int expanded = 0;
  int i = 0;
  for (i = indice; i < iword && !expanded; i += 2)
  {
    for (int j = 0; j < lang_context->Char_classUsed; j++)
    {
      struct char_class *char_class = lang_context->Char_class + j;
      if (!memcmp(char_class->character, word + i, 2))
      {
        for (int k = 0; k < char_class->number; k++)
        {
          memcpy(buffer + i, lang_context->BaClass + char_class->start + k * 2, 2);
          IFE(PhoRulesRuleAddExpense(lang_context, iword, buffer, i + 2, Irule));
          expanded = 1;
        }
        break;
      }
    }
  }

  if (i >= iword && !expanded)
  {
    IFE(PhoRulesAutRuleAdd(lang_context, iword, word, Irule));
    DONE;
  }

  DONE;
}
