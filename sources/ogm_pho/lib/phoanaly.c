/*
 *	Analysing matching for Phonetizer functions
 *  Copyright (c) 2006 Pertimm by P.Constant,G.Logerot and L.Rigouste
 *  Dev : December 2006
 *  Version 1.0
*/
#include "ogm_pho.h"




STATICF(int) matching_cmp(pr_(const void *) pr(const void *));  




int PhoAnalysing(ctrl_pho)
struct og_ctrl_pho *ctrl_pho;
{
struct matching *matching;
int i,last_offset=(-1);
struct rule *rule;

qsort(ctrl_pho->Matching,ctrl_pho->MatchingUsed,sizeof(struct matching),matching_cmp);

for(i=0;i<ctrl_pho->MatchingUsed;i++) {
  matching=ctrl_pho->Matching+i;
  rule=ctrl_pho->Rule+matching->Irule;

  if(matching->offset>last_offset) {
    matching->selected=1;
    /** adjust last_offset to skip as many characters as replace says **/
    last_offset=matching->offset+rule->replace-1;
    }
  }

DONE;
}





STATICF(int) matching_cmp(const void *ptr1, const void *ptr2)
{
struct matching *matching1 = (struct matching *)ptr1;
struct matching *matching2 = (struct matching *)ptr2;
struct og_ctrl_pho *ctrl_pho = matching1->ctrl_pho;
struct rule *rule1 = ctrl_pho->Rule+matching1->Irule;
struct rule *rule2 = ctrl_pho->Rule+matching2->Irule;
int cmp;
cmp = matching1->offset - matching2->offset;
if (cmp) return(cmp);
cmp = rule2->context_size - rule1->context_size;
if (cmp) return(cmp);
return(matching1->Irule - matching2->Irule);
}




