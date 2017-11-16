/*
 *  Rules automaton for ogm_pho functions
 *  Copyright (c) 2006 Pertimm by P.Constant,G.Logerot and L.Rigouste
 *  Dev : November 2006
 *  Version 1.0
*/
#include "ogm_pho.h"



og_status PhoRulesRuleAdd(struct og_xml_info *info)
{
  struct og_ctrl_pho *ctrl_pho = info->ctrl_pho;
  struct lang_context *lang_context = info->lang_context;

  struct rule *rule;
  int Irule = AllocRule(lang_context, &rule);
  IFE(Irule);

  memcpy(rule->left, info->left, info->ileft);
  rule->ileft = info->ileft;
  memcpy(rule->key, info->key, info->ikey);
  rule->ikey = info->ikey;
  memcpy(rule->right, info->right, info->iright);
  rule->iright = info->iright;
  memcpy(rule->phonetic, info->phonetic, info->iphonetic);
  rule->iphonetic = info->iphonetic;
  rule->replace = info->replace;
  rule->step = info->step;
  rule->context_size = rule->ileft + rule->ikey + rule->iright;

  if (rule->context_size > DPcPathSize * 2)
  {
    OgMsg(ctrl_pho->hmsg, "", DOgMsgDestInLog, "PhoRulesRuleAdd: too big left+key+right (context_size=%d)",
        rule->context_size);
    lang_context->RuleUsed--;
    DONE;
  }

  char word[DPcPathSize * 2];
  int iword = 0;
  memcpy(word + iword, rule->left, rule->ileft);
  iword += rule->ileft;
  memcpy(word + iword, rule->key, rule->ikey);
  iword += rule->ikey;
  memcpy(word + iword, rule->right, rule->iright);
  iword += rule->iright;

  if (ctrl_pho->loginfo->trace & DOgPhoTraceRules)
  {
    char B1[DPcPathSize];
    int iB1 = 0;
    IFE(OgUniToCp(iword,word,DPcPathSize,&iB1,B1,DOgCodePageANSI,0,0));
    OgMsg(ctrl_pho->hmsg, "", DOgMsgDestInLog, "PhoRulesRuleAdd: Irule = '%d', Expanding context [[%.*s]]", Irule, iB1,
        B1);
  }

  IFE(PhoRulesRuleAddExpense(lang_context, iword, word, 0, Irule));

  DONE;
}


og_status PhoRulesRuleGet(struct lang_context *lang_context, int offset, int step)
{
  unsigned char buffer[DPcAutMaxBufferSize];
  unsigned char *p = buffer;
  OggNout(step, &p);
  int iunibansi = p - buffer;
  memcpy(p, lang_context->bufferIn + offset, lang_context->ibufferIn - offset);
  p += lang_context->ibufferIn - offset;
  int ibuffer = p - buffer;

  unsigned char out[DPcAutMaxBufferSize + 9];
  oindex states[DPcAutMaxBufferSize + 9];
  int retour = 0, nstate0 = 0, nstate1 = 0, iout = 0;
  IFE(retour = OgAufScanf(lang_context->ha_rules, ibuffer, buffer, &iout, out, &nstate0, &nstate1, states));

  if (nstate0 <= 0) return (0);

  for (int i = nstate0 - 1; i > iunibansi; i--)
  {
    buffer[i] = 0;
    buffer[i + 1] = 1;

    if ((retour = OgAufScanf(lang_context->ha_rules, i + 2, buffer, &iout, out, &nstate0, &nstate1, states)))
    {
      do
      {
        IFE(retour);
        if (retour)
        {
          p = out;
          int Irule = OggNin4(&p);
          IFE(PhoMatchingAdd(lang_context, offset, Irule));
        }
      }
      while ((retour = OgAufScann(lang_context->ha_rules, &iout, out, nstate0, &nstate1, states)));
    }

  }

  return (1);
}



og_status RulesLog(struct lang_context *lang_context, char *filename)
{
  struct og_ctrl_pho *ctrl_pho = lang_context->ctrl_pho;

  FILE *fd=fopen(filename,"wb");
  IFn(fd)
  {
    OgMsg(ctrl_pho->hmsg, "", DOgMsgDestInLog, "RulesLog: impossible to fopen '%s'", filename);
    DONE;
  }

  /* code uni (2) + "STEP::Number::[[left::key::right]]::phonetic::replace\n" (45) */
  fwrite(
      "\xFE\xFF\0S\0T\0E\0P\0:\0:\0N\0u\0m\0b\0e\0r\0:\0:\0[\0[\0l\0e\0f\0t\0:\0:\0k\0e\0y\0:\0:\0r\0i\0g\0h\0t\0]\0]\0:\0:\0p\0h\0o\0n\0e\0t\0i\0c\0\n",
      sizeof(unsigned char), 92, fd);

  for (int i = 0; i < lang_context->RuleUsed; i++)
  {
    struct rule *rule = lang_context->Rule + i;
    /* step and i */

    char B1[DPcPathSize];
    int iB1 = sprintf(B1, "%d::%d::[[", rule->step, i);

    char B2[DPcPathSize];
    int iB2 = 0;
    IFE(OgCpToUni(iB1,B1,DPcPathSize,&iB2,B2,DOgCodePageANSI,0,0));
    fwrite(B2, sizeof(unsigned char), iB2, fd);
    /* left, key and right */
    fwrite(rule->left, sizeof(unsigned char), rule->ileft, fd);
    fwrite("\0:\0:", sizeof(unsigned char), 4, fd);
    fwrite(rule->key, sizeof(unsigned char), rule->ikey, fd);
    fwrite("\0:\0:", sizeof(unsigned char), 4, fd);
    fwrite(rule->right, sizeof(unsigned char), rule->iright, fd);
    fwrite("\0]\0]\0:\0:", sizeof(unsigned char), 8, fd);
    /* phonetic */
    fwrite(rule->phonetic, sizeof(unsigned char), rule->iphonetic, fd);
    /* replace */
    iB1 = sprintf(B1, "::%d\n", rule->replace);
    IFE(OgCpToUni(iB1,B1,DPcPathSize,&iB2,B2,DOgCodePageANSI,0,0));
    fwrite(B2, sizeof(unsigned char), iB2, fd);
  }

  fclose(fd);

  DONE;
}

