/*
 *  Handling request input parts
 *  Copyright (c) 2017 Pertimm, by Patrick Constant
 *  Dev : October 2017
 *  Version 1.0
 */
#include "ogm_nlp.h"

og_status NlpRequestInputPartAdd(og_nlp_th ctrl_nlp_th, struct request_word *request_word, package_t package,
    int Iinput_part)
{
  size_t Irequest_input_part;
  struct request_input_part *request_input_part = OgHeapNewCell(ctrl_nlp_th->hrequest_input_part, &Irequest_input_part);
  IFn(request_input_part) DPcErr;
  IF(Irequest_input_part) DPcErr;

  request_input_part->request_word = request_word;
  IFn(request_input_part->input_part=OgHeapGetCell(package->hinput_part,Iinput_part)) DPcErr;

  DONE;
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

  struct request_word *request_word = request_input_part->request_word;
  og_string string_request_word = OgHeapGetCell(ctrl_nlp_th->hba, request_word->start);
  IFN(string_request_word) DPcErr;

  package_t package = request_input_part->input_part->package;
  og_string package_id = OgHeapGetCell(package->hba, package->id_start);
  IFN(package_id) DPcErr;
  og_string package_slug = OgHeapGetCell(package->hba, package->slug_start);
  IFN(package_slug) DPcErr;

  OgMsg(ctrl_nlp_th->hmsg, "", DOgMsgDestInLog, "%4d: '%s' at %d:%d in package '%s' '%s'", Irequest_input_part,
      string_request_word, request_word->start_position, request_word->length_position, package_slug, package_id);

  DONE;
}
