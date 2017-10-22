/*
 *  Handling log messages for Natural Language Processing library
 *  Copyright (c) 2017 Pertimm, by Patrick Constant
 *  Dev : September 2017
 *  Version 1.0
 */
#include "ogm_nlp.h"

static og_status NlpMatchWords(og_nlp_th ctrl_nlp_th);
static og_status NlpMatchWord(og_nlp_th ctrl_nlp_th, int Irequest_word);
static og_status NlpMatchWordInPackage(og_nlp_th ctrl_nlp_th, struct request_word *request_word, int input_length,
    unsigned char *input, package_t package);

/**
 * Parse the request and then working on all words of the sentence
 * to encode the matching algorithm.
 */
og_status NlpMatch(og_nlp_th ctrl_nlp_th, json_t *json_interpretations)
{
  // The request sentence is in : ctrl_nlp_th->request_sentence
  IFE(NlpParse(ctrl_nlp_th));
  IFE(NlpLogRequestWords(ctrl_nlp_th));

  // Scanning all the words and create the list of input parts that match the words
  IFE(NlpMatchWords(ctrl_nlp_th));
  IFE(NlpRequestInputPartsLog(ctrl_nlp_th));


  DONE;
}

static og_status NlpMatchWords(og_nlp_th ctrl_nlp_th)
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
  input[input_length++] = 1;
  input[input_length] = 0;

  OgMsg(ctrl_nlp_th->hmsg, "", DOgMsgDestInLog, "Looking for input parts for string '%s':", string_request_word);

  int interpret_package_used = OgHeapGetCellsUsed(ctrl_nlp_th->hinterpret_package);
  for (int i = 0; i < interpret_package_used; i++)
  {
    struct interpret_package *interpret_package = OgHeapGetCell(ctrl_nlp_th->hinterpret_package, i);
    IFN(interpret_package) DPcErr;
    IFE(NlpMatchWordInPackage(ctrl_nlp_th, request_word, input_length, input, interpret_package->package));
  }

  DONE;
}

static og_status NlpMatchWordInPackage(og_nlp_th ctrl_nlp_th, struct request_word *request_word, int input_length,
    unsigned char *input, package_t package)
{
  unsigned char out[DPcAutMaxBufferSize + 9];
  oindex states[DPcAutMaxBufferSize + 9];
  int retour, nstate0, nstate1, iout;

  og_string package_id = OgHeapGetCell(package->hba, package->id_start);
  IFN(package_id) DPcErr;
  og_string package_slug = OgHeapGetCell(package->hba, package->slug_start);
  IFN(package_slug) DPcErr;
  OgMsg(ctrl_nlp_th->hmsg, "", DOgMsgDestInLog, "  Looking for input parts in package '%s' '%s':", package_slug,
      package_id);

  if ((retour = OgAutScanf(package->ha_word, input_length, input, &iout, out, &nstate0, &nstate1, states)))
  {
    do
    {
      IFE(retour);
      int Iinput_part;
      unsigned char *p = out;
      IFE(DOgPnin4(ctrl_nlp_th->herr,&p,&Iinput_part));
      OgMsg(ctrl_nlp_th->hmsg, "", DOgMsgDestInLog, "    found input part %d", Iinput_part);
      IFE(NlpRequestInputPartAdd(ctrl_nlp_th, request_word, package, Iinput_part));
    }
    while ((retour = OgAutScann(package->ha_word, &iout, out, nstate0, &nstate1, states)));
  }

  DONE;
}

