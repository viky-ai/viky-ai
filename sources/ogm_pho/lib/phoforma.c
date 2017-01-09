/*
 *  Format function for Phonet function
 *  Copyright (c) 2006 Pertimm by P.Constant,G.Logerot and L.Rigouste
 *  Dev : December 2006
 *  Version 1.0
*/
#include "ogm_pho.h"



og_status PhoFormatClean(struct lang_context *lang_context)
{
  struct og_ctrl_pho *ctrl_pho = lang_context->ctrl_pho;

  OgUniStrlwr(lang_context->iinput, lang_context->input, lang_context->input);

  if (ctrl_pho->loginfo->trace & DOgPhoTraceMain)
  {
    OgMsg(ctrl_pho->hmsg, "", DOgMsgDestInLog, "OgPhonet: non_alpha_to_space = %d", lang_context->non_alpha_to_space);
  }

  for (int i = 0; i < lang_context->iinput; i += 2)
  {

    for (int k = 0; k < lang_context->appending_characters_number; k++)
    {
      if (lang_context->input[i] == lang_context->appending_characters[k * 2]
          && lang_context->input[i + 1] == lang_context->appending_characters[k * 2 + 1])
      {
        for (int j = i; j < lang_context->iinput - 2; j++)
        {
          lang_context->input[j] = lang_context->input[j + 2];
        }
        lang_context->iinput -= 2;
      }
    }

    unsigned char c = (lang_context->input[i] << 8) + lang_context->input[i + 1];

    // We replace some characters by a space character
    if (lang_context->non_alpha_to_space)
    {
      if(OgUniIsdigit(c))
      {
        if(!lang_context->keep_digit)
        {
          memcpy(lang_context->input + i, lang_context->space_character, 2);
        }
      }
      else if(!OgUniIsalpha(c))
      {
        memcpy(lang_context->input + i, lang_context->space_character, 2);
      }
    }

  }

  DONE;
}


og_status PhoFormatAppendingCharAdd(struct lang_context *lang_context, int ib, unsigned char *b)
{
  if (ib < 2)
  {

    DONE;
  }

  memcpy(lang_context->appending_characters + lang_context->appending_characters_number * 2, b, 2);
  lang_context->appending_characters_number++;

  DONE;
}

