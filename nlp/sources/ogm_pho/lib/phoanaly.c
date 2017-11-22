/*
 *  Analysing matching for Phonetizer functions
 *  Copyright (c) 2006 Pertimm by P.Constant,G.Logerot and L.Rigouste
 *  Dev : December 2006
 *  Version 1.0
*/
#include "ogm_pho.h"


STATICF(int) matching_cmp(pr_(const void *) pr(const void *));


og_status PhoAnalysing(struct lang_context *lang_context)
{
  qsort(lang_context->Matching, lang_context->MatchingUsed, sizeof(struct matching), matching_cmp);

  int last_offset = (-1);
  for (int i = 0; i < lang_context->MatchingUsed; i++)
  {
    struct matching *matching = lang_context->Matching + i;
    struct rule *rule = lang_context->Rule + matching->Irule;

    if (matching->offset > last_offset)
    {
      matching->selected = 1;
      /** adjust last_offset to skip as many characters as replace says **/
      last_offset = matching->offset + rule->replace - 1;
    }
  }

  DONE;
}


static int matching_cmp(const void *ptr1, const void *ptr2)
{
  struct matching *matching1 = (struct matching *) ptr1;
  struct matching *matching2 = (struct matching *) ptr2;
  struct lang_context *lang_context = matching1->lang_context;
  struct rule *rule1 = lang_context->Rule + matching1->Irule;
  struct rule *rule2 = lang_context->Rule + matching2->Irule;
  int cmp;
  cmp = matching1->offset - matching2->offset;
  if (cmp) return (cmp);
  cmp = rule2->context_size - rule1->context_size;
  if (cmp) return (cmp);
  return (matching1->Irule - matching2->Irule);
}
