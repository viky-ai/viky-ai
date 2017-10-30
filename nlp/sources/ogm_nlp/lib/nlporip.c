/*
 *  Handling original request input parts
 *  Copyright (c) 2017 Pertimm, by Patrick Constant
 *  Dev : October 2017
 *  Version 1.0
 */
#include "ogm_nlp.h"


og_status NlpOriginalRequestInputPartsCalculate(og_nlp_th ctrl_nlp_th)
{
  int request_input_part_used = OgHeapGetCellsUsed(ctrl_nlp_th->hrequest_input_part);
  struct request_input_part *request_input_part = OgHeapGetCell(ctrl_nlp_th->hrequest_input_part, 0);
  IFN(request_input_part) DPcErr;

  IFE(OgHeapReset(ctrl_nlp_th->horiginal_request_input_part));
  IFE(OgHeapAddCells(ctrl_nlp_th->horiginal_request_input_part, request_input_part_used));

  struct original_request_input_part *original_request_input_part = OgHeapGetCell(ctrl_nlp_th->horiginal_request_input_part, 0);
  IFN(original_request_input_part) DPcErr;

  for (int i=0; i<request_input_part_used; i++)
  {
    original_request_input_part[request_input_part[i].Ioriginal_request_input_part].Irequest_input_part = i;
  }
DONE;
}

og_status NlpRequestExpressionAddOrip(og_nlp_th ctrl_nlp_th, struct request_expression *request_expression,
    int Ioriginal_request_input_part)
{
  size_t Iorip;
  struct orip *orip = OgHeapNewCell(ctrl_nlp_th->horip, &Iorip);
  IFn(orip) DPcErr;
  IF(Iorip) DPcErr;

  if (request_expression->orips_nb == 0)
  {
    request_expression->orip_start = Iorip;
  }
  request_expression->orips_nb++;
  orip->Ioriginal_request_input_part = Ioriginal_request_input_part;

  DONE;
}
