/*
 *  Utility functions on request words
 *  Copyright (c) 2019 Pertimm, by Patrick Constant
 *  Dev : January 2019
 *  Version 1.0
 */
#include "ogm_nlp.h"

/*
 * Gets the rword which is closest the position, after or at this position (thus >=)
 * this is a dichotomy algorithm
 */
og_bool NlpRequestWordGet(og_nlp_th ctrl_nlp_th, int position, int *pIrequest_word)
{
  struct request_word *request_words = OgHeapGetCell(ctrl_nlp_th->hrequest_word, 0);
  IFN(request_words) DPcErr;
  int basic_request_word_used = ctrl_nlp_th->basic_request_word_used;

  *pIrequest_word = (-1);

  int start = 0;
  int end = basic_request_word_used;

  while (1)
  {
    int difference = (end - start) / 2;
    int current = start + difference;
    struct request_word *request_word = request_words + current;
    if (difference <= 1)
    {
      int Irequest_word;
      if (request_word->start_position < position) Irequest_word = current + 1;
      else Irequest_word = current;
      if (0 <= Irequest_word && Irequest_word < basic_request_word_used)
      {
        *pIrequest_word = Irequest_word;
        return TRUE;
      }
      return FALSE;
    }
    if (request_word->start_position < position)
    {
      start = current;
    }
    else
    {
      end = current;
    }
  }

  return FALSE;
}

