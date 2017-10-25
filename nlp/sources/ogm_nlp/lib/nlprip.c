/*
 *  Handling request input parts
 *  Copyright (c) 2017 Pertimm, by Patrick Constant
 *  Dev : October 2017
 *  Version 1.0
 */
#include "ogm_nlp.h"

static struct request_input_part *NlpRequestInputPartAdd(og_nlp_th ctrl_nlp_th, int Iinterpret_package,
    package_t package, int Iinput_part);

og_status NlpRequestInputPartAddWord(og_nlp_th ctrl_nlp_th, struct request_word *request_word, int Iinterpret_package,
    package_t package, int Iinput_part)
{
  struct request_input_part *request_input_part = NlpRequestInputPartAdd(ctrl_nlp_th, Iinterpret_package, package,
      Iinput_part);
  IFN(request_input_part) DPcErr;
  request_input_part->type = nlp_input_part_type_Word;
  request_input_part->request_word = request_word;
  DONE;
}

og_status NlpRequestInputPartAddInterpretation(og_nlp_th ctrl_nlp_th, struct interpretation *request_interpretation,
    int Iinterpret_package, package_t package, int Iinput_part)
{
  struct request_input_part *request_input_part = NlpRequestInputPartAdd(ctrl_nlp_th, Iinterpret_package, package,
      Iinput_part);
  IFN(request_input_part) DPcErr;
  request_input_part->type = nlp_input_part_type_Interpretation;
  request_input_part->request_interpretation = request_interpretation;
  DONE;
}

static struct request_input_part *NlpRequestInputPartAdd(og_nlp_th ctrl_nlp_th, int Iinterpret_package,
    package_t package, int Iinput_part)
{
  size_t Irequest_input_part;
  struct request_input_part *request_input_part = OgHeapNewCell(ctrl_nlp_th->hrequest_input_part, &Irequest_input_part);
  IFn(request_input_part) return NULL;
  IF(Irequest_input_part) return NULL;

  IFn(request_input_part->input_part=OgHeapGetCell(package->hinput_part,Iinput_part)) return NULL;
  request_input_part->Iinterpret_package = Iinterpret_package;
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
  og_string package_id = OgHeapGetCell(package->hba, package->id_start);
  IFN(package_id) DPcErr;
  og_string package_slug = OgHeapGetCell(package->hba, package->slug_start);
  IFN(package_slug) DPcErr;

  unsigned char string_input_part[DPcPathSize];
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
      package_t interpretation_package = interpretation->package;

      og_string interpretation_id = OgHeapGetCell(interpretation_package->hba, interpretation->id_start);
      IFN(interpretation_id) DPcErr;

      og_string interpretation_slug = OgHeapGetCell(interpretation_package->hba, interpretation->slug_start);
      IFN(interpretation_slug) DPcErr;
      snprintf(string_input_part, DPcPathSize, "interpretation: '%s' '%s'", interpretation_slug, interpretation_id);
      break;
    }
  }

  OgMsg(ctrl_nlp_th->hmsg, "", DOgMsgDestInLog, "%4d:%c %s %d,%d in package '%s' '%s'", Irequest_input_part,
      (request_input_part->consumed ? '-' : ' '), string_input_part, request_input_part->Iinterpret_package,
      request_input_part->Iinput_part, package_slug, package_id);

  DONE;
}
