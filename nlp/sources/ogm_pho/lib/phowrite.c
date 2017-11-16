/*
 *  Writing Output for Phontizer functions
 *  Copyright (c) 2006 Pertimm by P.Constant,G.Logerot and L.Rigouste
 *  Dev : December 2006
 *  Version 1.0
*/
#include "ogm_pho.h"


og_status PhoWriting(struct lang_context *lang_context)
{
  int last_offset = 0;
  for (int i = 0; i < lang_context->MatchingUsed; i++)
  {
    struct matching *matching = lang_context->Matching + i;
    if (!matching->selected) continue;
    struct rule *rule = lang_context->Rule + matching->Irule;

    IFE(PhoAppendBa(lang_context, matching->offset - last_offset, lang_context->bufferIn + last_offset));
    IFE(PhoAppendBa(lang_context, rule->iphonetic, rule->phonetic));

    last_offset = matching->offset + rule->replace;
  }

  IFE(PhoAppendBa(lang_context, lang_context->ibufferIn - last_offset, lang_context->bufferIn + last_offset));

  DONE;
}

