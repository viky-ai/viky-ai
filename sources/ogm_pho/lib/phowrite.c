/*
 *	Writing Output for Phontizer functions
 *  Copyright (c) 2006 Pertimm by P.Constant,G.Logerot and L.Rigouste
 *  Dev : December 2006
 *  Version 1.0
*/
#include "ogm_pho.h"






int PhoWriting(ctrl_pho)
struct og_ctrl_pho *ctrl_pho;
{
struct matching *matching;
struct rule *rule;
int i,last_offset=0;

for(i=0;i<ctrl_pho->MatchingUsed;i++) {
  matching=ctrl_pho->Matching+i;
  if(!matching->selected) continue;
  rule=ctrl_pho->Rule+matching->Irule;
  
  IFE(PhoAppendBa(ctrl_pho,matching->offset-last_offset,ctrl_pho->bufferIn+last_offset));
  IFE(PhoAppendBa(ctrl_pho,rule->iphonetic,rule->phonetic));

  last_offset=matching->offset+rule->replace;
  }

IFE(PhoAppendBa(ctrl_pho,ctrl_pho->ibufferIn-last_offset,ctrl_pho->bufferIn+last_offset));

DONE;
}

