/*
 *  Matching for ogm_matching functions
 *  Copyright (c) 2006 Pertimm by P.Constant,G.Logerot and L.Rigouste
 *  Dev : December 2006
 *  Version 1.0
*/
#include "ogm_pho.h"


static int AllocMatching(struct lang_context *lang_context, struct matching **pmatching);


og_status PhoMatchingRules(struct lang_context *lang_context, int step)
{
  for (int i = 0; i < lang_context->ibufferIn; i += 2)
  {
    IFE(PhoRulesRuleGet(lang_context, i, step));
  }

  DONE;
}


og_status PhoMatchingAdd(struct lang_context *lang_context, int offset, int Irule)
{
  struct matching *matching;
  struct rule *rule = lang_context->Rule + Irule;
  int Imatching;

  IFE(Imatching = AllocMatching(lang_context, &matching));
  matching->offset = offset + rule->ileft;
  matching->Irule = Irule;

  DONE;
}


static int AllocMatching(struct lang_context *lang_context, struct matching **pmatching)
{
  struct og_ctrl_pho *ctrl_pho = lang_context->ctrl_pho;

  struct matching *matching = 0;
  int i = lang_context->MatchingNumber;

  beginAllocMatching:

  if (lang_context->MatchingUsed < lang_context->MatchingNumber)
  {
    i = lang_context->MatchingUsed++;
  }

  if (i == lang_context->MatchingNumber)
  {
    unsigned a, b;
    struct matching *og_l;

    if (ctrl_pho->loginfo->trace & DOgPhoTraceMemory)
    {
      OgMsg(ctrl_pho->hmsg, "", DOgMsgDestInLog, "AllocMatching: max Matching number (%d) reached",
          lang_context->MatchingNumber);
    }
    a = lang_context->MatchingNumber;
    b = a + (a >> 2) + 1;
    IFn(og_l=(struct matching *)malloc(b*sizeof(struct matching)))
    {
      char erreur[DOgErrorSize];
      sprintf(erreur, "AllocMatching: malloc error on Matching");
      OgErr(ctrl_pho->herr, erreur);
      DPcErr;
    }

    memcpy(og_l, lang_context->Matching, a * sizeof(struct matching));
    DPcFree(lang_context->Matching);
    lang_context->Matching = og_l;
    lang_context->MatchingNumber = b;

    if (ctrl_pho->loginfo->trace & DOgPhoTraceMemory)
    {
      OgMsg(ctrl_pho->hmsg, "", DOgMsgDestInLog, "AllocMatching: new Matching number is %d\n",
          lang_context->MatchingNumber);
    }

#ifdef DOgNoMainBufferReallocation
    sprintf(erreur,"AllocMatching: MatchingNumber reached (%d)",lang_context->MatchingNumber);
    OgErr(ctrl_pho->herr,erreur); DPcErr;
#endif

    goto beginAllocMatching;
  }

  matching = lang_context->Matching + i;
  memset(matching, 0, sizeof(struct matching));
  matching->lang_context = lang_context;

  if (pmatching) *pmatching = matching;
  return (i);
}


og_status MatchingLog(struct lang_context *lang_context, int step)
{
  struct og_ctrl_pho *ctrl_pho = lang_context->ctrl_pho;

  int iB1 = 0;
  char B1[DPcPathSize * 2];
  IFE(OgUniToCp(lang_context->ibufferIn,lang_context->bufferIn,DPcPathSize*2,&iB1,B1,DOgCodePageANSI,0,0));
  OgMsg(ctrl_pho->hmsg, "", DOgMsgDestInLog, "MatchingLog: -- *STEP %d* listing matching rules for '%.*s' --", step,
      iB1, B1);

  int last_offset = (-2);
  for (int i = 0; i < lang_context->MatchingUsed; i++)
  {
    struct matching *matching = lang_context->Matching + i;
    struct rule *rule = lang_context->Rule + matching->Irule;

    if (matching->offset > last_offset)
    {
      for (int j = last_offset + 2; j < matching->offset + 1; j += 2)
      {
        IFE(OgUniToCp(2,lang_context->bufferIn+j,DPcPathSize,&iB1,B1,DOgCodePageANSI,0,0));
        OgMsg(ctrl_pho->hmsg, "", DOgMsgDestInLog, "MatchingLog: -- OFFSET %.*s", iB1, B1);
      }
      last_offset = matching->offset;
    }

    char message[25];
    if (matching->selected) strcpy(message, " SELECTED");
    else strcpy(message, "");

    /*
     %s:%s:%s::%s
     rule->left,rule->key,rule->right,rule->phonetic
     */
    int iB = 0;
    unsigned char B[DPcPathSize];
    memcpy(B + iB, rule->left, rule->ileft);
    iB += rule->ileft;
    B[iB++] = '\0';
    B[iB++] = ':';
    memcpy(B + iB, rule->key, rule->ikey);
    iB += rule->ikey;
    B[iB++] = '\0';
    B[iB++] = ':';
    memcpy(B + iB, rule->right, rule->iright);
    iB += rule->iright;
    B[iB++] = '\0';
    B[iB++] = '-';
    memcpy(B + iB, rule->phonetic, rule->iphonetic);
    iB += rule->iphonetic;

    IFE(OgUniToCp(iB,B,DPcPathSize,&iB1,B1,DOgCodePageANSI,0,0));

    OgMsg(ctrl_pho->hmsg, "", DOgMsgDestInLog, "MatchingLog: RULE Irule=%.3d (%d*%.*s*%d) %s", matching->Irule,
        rule->step, iB1, B1, rule->replace, message);
  }

  for (int j = last_offset + 2; j < lang_context->ibufferIn; j += 2)
  {
    IFE(OgUniToCp(2,lang_context->bufferIn+j,DPcPathSize,&iB1,B1,DOgCodePageANSI,0,0));
    OgMsg(ctrl_pho->hmsg, "", DOgMsgDestInLog, "MatchingLog: -- OFFSET %.*s", iB1, B1);
  }

  DONE;
}
