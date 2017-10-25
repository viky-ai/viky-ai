/*
 *  Handling request input parts
 *  Copyright (c) 2017 Pertimm, by Patrick Constant
 *  Dev : October 2017
 *  Version 1.0
 */
#include "ogm_nlp.h"

static struct request_input_part *NlpRequestInputPartAdd(og_nlp_th ctrl_nlp_th,
    struct interpret_package *interpret_package, int Iinput_part);

og_status NlpRequestInputPartAddWord(og_nlp_th ctrl_nlp_th, struct request_word *request_word,
    struct interpret_package *interpret_package, int Iinput_part)
{
  struct request_input_part *request_input_part = NlpRequestInputPartAdd(ctrl_nlp_th, interpret_package, Iinput_part);
  IFN(request_input_part) DPcErr;
  request_input_part->type = nlp_input_part_type_Word;
  request_input_part->request_word = request_word;
  DONE;
}

og_status NlpRequestInputPartAddInterpretation(og_nlp_th ctrl_nlp_th, struct interpretation *request_interpretation,
    struct interpret_package *interpret_package, int Iinput_part)
{
  struct request_input_part *request_input_part = NlpRequestInputPartAdd(ctrl_nlp_th, interpret_package, Iinput_part);
  IFN(request_input_part) DPcErr;
  request_input_part->type = nlp_input_part_type_Interpretation;
  request_input_part->request_interpretation = request_interpretation;
  DONE;
}

static struct request_input_part *NlpRequestInputPartAdd(og_nlp_th ctrl_nlp_th,
    struct interpret_package *interpret_package, int Iinput_part)
{
  size_t Irequest_input_part;
  struct request_input_part *request_input_part = OgHeapNewCell(ctrl_nlp_th->hrequest_input_part, &Irequest_input_part);
  IFn(request_input_part) return NULL;
  IF(Irequest_input_part) return NULL;

  IFn(request_input_part->input_part = OgHeapGetCell(interpret_package->package->hinput_part, Iinput_part)) return NULL;
  request_input_part->interpret_package = interpret_package;
  request_input_part->Iinput_part = Iinput_part;

  return request_input_part;
}

og_status NlpRequestInputPartsLog(og_nlp_th ctrl_nlp_th)
{
  OgMsg(ctrl_nlp_th->hmsg, "", DOgMsgDestInLog, "list of request input parts:");
  int request_input_part_used = OgHeapGetCellsUsed(ctrl_nlp_th->hrequest_input_part);
  for (int i = 0; i < request_input_part_used; i++)
  {
    IFE(NlpRequestInputPartLog(ctrl_nlp_th, i));
  }

  DONE;
}

og_status NlpRequestInputPartLog(og_nlp_th ctrl_nlp_th, int Irequest_input_part)
{
  struct request_input_part *request_input_part = OgHeapGetCell(ctrl_nlp_th->hrequest_input_part, Irequest_input_part);
  IFN(request_input_part) DPcErr;

  package_t package = request_input_part->input_part->expression->interpretation->package;

  og_char_buffer string_input_part[DPcPathSize];
  switch (request_input_part->type)
  {
    case nlp_input_part_type_Nil:
    {
      snprintf(string_input_part, DPcPathSize, "nil");
      break;
    }
    case nlp_input_part_type_Word:
    {
      struct request_word *request_word = request_input_part->request_word;
      og_string string_request_word = OgHeapGetCell(ctrl_nlp_th->hba, request_word->start);
      IFN(string_request_word) DPcErr;
      snprintf(string_input_part, DPcPathSize, "word:%s %d:%d", string_request_word, request_word->start_position,
          request_word->length_position);
      break;
    }
    case nlp_input_part_type_Interpretation:
    {
      struct interpretation *interpretation = request_input_part->request_interpretation;

      snprintf(string_input_part, DPcPathSize, "interpretation: '%s' '%s'", interpretation->slug, interpretation->id);
      break;
    }
  }

  OgMsg(ctrl_nlp_th->hmsg, "", DOgMsgDestInLog, "%4d: %s in package '%s' '%s'", Irequest_input_part, string_input_part,
      package->slug, package->id);

  DONE;
}
