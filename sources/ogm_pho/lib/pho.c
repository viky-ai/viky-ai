/*
 *  Main program for Phonet function
 *  Copyright (c) 2008 Pertimm by P.Constant,G.Logerot and L.Rigouste
 *  Dev : November 2006, april 2008
 *  Version 1.1
*/
#include "ogm_pho.h"



static int PhoResultSendClean(struct lang_context *lang_context, struct og_pho_output *output, int step);



PUBLIC(int) OgPhonet(void *handle, struct og_pho_input *input, struct og_pho_output *output)
{
  struct og_ctrl_pho *ctrl_pho = (struct og_ctrl_pho *) handle;
  char B1[DPcPathSize * 2], B2[DPcPathSize * 2];
  int iB1 = 0, iB2 = 0, step;

  output->iB = 0;
  IFn(ctrl_pho) DONE;

  if (input->iB > 4 * DPcPathSize)
  {
    OgMsg(ctrl_pho->hmsg, "", DOgMsgDestInLog, "OgPhonet: input string too long (%d>%d)", input->iB, 4 * DPcPathSize);
    DONE;
  }

  struct lang_context *lang_context = g_hash_table_lookup(ctrl_pho->lang_context_map, GINT_TO_POINTER(input->lang));
  unsigned char *lang = OgIso639ToCode(input->lang);
  IFn(lang_context)
  {
    OgMsg(ctrl_pho->hmsg, "", DOgMsgDestInLog, "OgPhonet: phonet has no conf for language %s, nothing done.", lang);
    DONE;
  }

  memcpy(lang_context->input, input->B, input->iB);
  lang_context->iinput = input->iB;

  IFE(PhoFormatClean(lang_context));

  if (ctrl_pho->loginfo->trace & DOgPhoTraceMain)
  {
    IFE(OgUniToCp(input->iB,input->B,DPcPathSize*2,&iB1,B1,DOgCodePageANSI,0,0));
    IFE(OgUniToCp(lang_context->iinput,lang_context->input,DPcPathSize*2,&iB2,B2,DOgCodePageANSI,0,0));
    OgMsg(ctrl_pho->hmsg, "", DOgMsgDestInLog, "OgPhonet: Entry is '%.*s' (%d) cleaned to '%.*s'", iB1, B1, iB1, iB2,
        B2);
    OgMsg(ctrl_pho->hmsg, "", DOgMsgDestInLog, "OgPhonet: number of steps : %d", lang_context->max_steps);
  }

  /* here doing the loop for rules steps */
  for (step = 1; step < lang_context->max_steps + 1; step++)
  {

    lang_context->MatchingUsed = 0;
    lang_context->BaUsed = 0;

    output->iB = 0;

    memcpy(lang_context->bufferIn, lang_context->space_character, 2);
    memcpy(lang_context->bufferIn + 2, lang_context->space_character, 2);
    memcpy(lang_context->bufferIn + 4, lang_context->input, lang_context->iinput);
    memcpy(lang_context->bufferIn + 4 + lang_context->iinput, lang_context->space_character, 2);
    memcpy(lang_context->bufferIn + 6 + lang_context->iinput, lang_context->space_character, 2);
    memcpy(lang_context->bufferIn + 8 + lang_context->iinput, lang_context->space_character, 2);
    lang_context->ibufferIn = lang_context->iinput + 10;

    if (ctrl_pho->loginfo->trace & DOgPhoTraceMain)
    {
      IFE(OgUniToCp(lang_context->ibufferIn,lang_context->bufferIn,DPcPathSize*2,&iB2,B2,DOgCodePageANSI,0,0));
      OgMsg(ctrl_pho->hmsg, "", DOgMsgDestInLog, "OgPhonet: *STEP %d* starting to phonetize '%.*s'", step, iB2, B2);
    }

    IFE(PhoMatchingRules(lang_context, step));

    IFE(PhoAnalysing(lang_context));

    if (ctrl_pho->loginfo->trace & DOgPhoTraceRulesMatch)
    {
      IFE(MatchingLog(lang_context, step));
    }

    IFE(PhoWriting(lang_context));

    IFE(PhoResultSendClean(lang_context, output, step));

    memcpy(lang_context->input, output->B, output->iB);
    lang_context->iinput = output->iB;
  }

  DONE;
}




static int PhoResultSendClean(struct lang_context *lang_context, struct og_pho_output *output, int step)
{
  int i, start = 0, end = 0;

  for (i = 0; i < lang_context->BaUsed; i += 2)
  {
    if (!memcmp(lang_context->Ba + i, lang_context->space_character, 2)) continue;
    start = i;
    break;
  }
  for (i = lang_context->BaUsed - 2; i >= 0; i -= 2)
  {
    if (!memcmp(lang_context->Ba + i, lang_context->space_character, 2)) continue;
    end = i;
    break;
  }

  if (start >= end)
  {
    output->B = lang_context->Ba;
    output->iB = 0;
  }

  if (step == lang_context->max_steps)
  {
    for (i = start; i <= end; i += 2)
    {
      if (!memcmp(lang_context->Ba + i, lang_context->space_character, 2))
      {
        lang_context->Ba[i] = '\0';
        lang_context->Ba[i + 1] = ' ';
      }
    }
  }

  output->B = lang_context->Ba + start;
  output->iB = end - start + 2;

  DONE;
}


