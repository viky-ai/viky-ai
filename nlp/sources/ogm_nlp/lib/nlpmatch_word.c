/*
 *  Handling log messages for Natural Language Processing library
 *  Copyright (c) 2017 Pertimm, by Patrick Constant
 *  Dev : September 2017
 *  Version 1.0
 */
#include "ogm_nlp.h"
#include <stdlib.h>

static og_status NlpMatchWord(og_nlp_th ctrl_nlp_th, int Irequest_word);
static og_status NlpMatchWordInPackage(og_nlp_th ctrl_nlp_th, struct request_word *request_word, int input_length,
    unsigned char *input, struct interpret_package *interpret_package);


og_status NlpMatchWords(og_nlp_th ctrl_nlp_th)
{
  int request_word_used = OgHeapGetCellsUsed(ctrl_nlp_th->hrequest_word);
  for (int i = 0; i < request_word_used; i++)
  {
    IFE(NlpMatchWord(ctrl_nlp_th, i));
  }
  DONE;
}

static og_status NlpMatchWord(og_nlp_th ctrl_nlp_th, int Irequest_word)
{
  struct request_word *request_word = OgHeapGetCell(ctrl_nlp_th->hrequest_word, Irequest_word);
  IFN(request_word) DPcErr;

  og_string string_request_word = OgHeapGetCell(ctrl_nlp_th->hba, request_word->start);
  IFN(string_request_word) DPcErr;

  unsigned char input[DPcAutMaxBufferSize + 9];
  int input_length = request_word->length;
  memcpy(input, string_request_word, input_length);
  input[input_length++] = '\1';
  input[input_length] = 0;

  char digit[DPcPathSize];
  digit[0] = 0;
  if (request_word->is_digit)
  {
    snprintf(digit, DPcPathSize, " -> %d", request_word->digit_value);
  }

  NlpLog(DOgNlpTraceMatch, "Looking for input parts for string '%s'%s:", string_request_word, digit);

  int interpret_package_used = OgHeapGetCellsUsed(ctrl_nlp_th->hinterpret_package);
  for (int i = 0; i < interpret_package_used; i++)
  {
    struct interpret_package *interpret_package = OgHeapGetCell(ctrl_nlp_th->hinterpret_package, i);
    IFN(interpret_package) DPcErr;
    IFE(NlpMatchWordInPackage(ctrl_nlp_th, request_word, input_length, input, interpret_package));
  }

  DONE;
}

static og_status NlpMatchWordInPackage(og_nlp_th ctrl_nlp_th, struct request_word *request_word, int input_length,
    unsigned char *input, struct interpret_package *interpret_package)
{
  package_t package = interpret_package->package;

  if (request_word->is_digit)
  {
    struct digit_input_part *digit_input_part_all = OgHeapGetCell(package->hdigit_input_part, 0);
    int digit_input_part_used = OgHeapGetCellsUsed(package->hdigit_input_part);
    for (int i = 0; i < digit_input_part_used; i++)
    {
      struct digit_input_part *digit_input_part = digit_input_part_all + i;
      // There is not need to have a special input part here for digit words
      IFE(NlpRequestInputPartAddWord(ctrl_nlp_th, request_word, interpret_package, digit_input_part->Iinput_part,TRUE));
    }
  }

  unsigned char out[DPcAutMaxBufferSize + 9];
  oindex states[DPcAutMaxBufferSize + 9];
  int retour, nstate0, nstate1, iout;

  if ((retour = OgAufScanf(package->ha_word, input_length, input, &iout, out, &nstate0, &nstate1, states)))
  {
    do
    {
      IFE(retour);
      int Iinput_part;
      unsigned char *p = out;
      IFE(DOgPnin4(ctrl_nlp_th->herr,&p,&Iinput_part));
      NlpLog(DOgNlpTraceMatch, "    found input part %d in request package %d", Iinput_part,
          interpret_package->self_index)
      IFE(NlpRequestInputPartAddWord(ctrl_nlp_th, request_word, interpret_package, Iinput_part,FALSE));
    }
    while ((retour = OgAufScann(package->ha_word, &iout, out, nstate0, &nstate1, states)));
  }

  DONE;
}

