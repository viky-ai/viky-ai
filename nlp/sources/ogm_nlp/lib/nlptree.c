/*
 *  Handling parsing tree from a request expression
 *  Copyright (c) 2017 Pertimm, by Patrick Constant
 *  Dev : October 2017
 *  Version 1.0
 */
#include "ogm_nlp.h"

static og_status OgNlpInterpretTreeLogRecursive(og_nlp_th ctrl_nlp_th, struct request_expression *request_expression,
    int offset);
static og_status NlpRequestInputPartWordLog(og_nlp_th ctrl_nlp_th, struct request_input_part *request_input_part,
    int offset);

og_status OgNlpInterpretTreeLog(og_nlp_th ctrl_nlp_th, struct request_expression *request_expression)
{
  OgMsg(ctrl_nlp_th->hmsg, "", DOgMsgDestInLog, "Tree representation for expression:");
  IFE(OgNlpInterpretTreeLogRecursive(ctrl_nlp_th, request_expression, 0));
  DONE;
}

static og_status OgNlpInterpretTreeLogRecursive(og_nlp_th ctrl_nlp_th, struct request_expression *request_expression,
    int offset)
{
  IFE(NlpRequestExpressionLog(ctrl_nlp_th, request_expression, offset));

  char string_offset[DPcPathSize];
  memset(string_offset, ' ', offset);
  string_offset[offset] = 0;

  struct original_request_input_part *original_request_input_part = OgHeapGetCell(
      ctrl_nlp_th->horiginal_request_input_part, 0);
  IFN(original_request_input_part) DPcErr;

  struct orip *orip = OgHeapGetCell(ctrl_nlp_th->horip, 0);
  IFN(orip) DPcErr;

  for (int i = 0; i < request_expression->orips_nb; i++)
  {
    int Ioriginal_request_input_part = orip[request_expression->orip_start + i].Ioriginal_request_input_part;
    int Irequest_input_part = original_request_input_part[Ioriginal_request_input_part].Irequest_input_part;
    struct request_input_part *request_input_part = OgHeapGetCell(ctrl_nlp_th->hrequest_input_part,
        Irequest_input_part);
    IFN(request_input_part) DPcErr;
    if (request_input_part->type == nlp_input_part_type_Word)
    {
      struct request_word *request_word = request_input_part->request_word;
      og_string string_request_word = OgHeapGetCell(ctrl_nlp_th->hba, request_word->start);
      IFN(string_request_word) DPcErr;
      IFE(NlpRequestInputPartWordLog(ctrl_nlp_th, request_input_part, offset + 2));
    }
    else if (request_input_part->type == nlp_input_part_type_Interpretation)
    {
      struct request_expression *sub_request_expression = OgHeapGetCell(ctrl_nlp_th->hrequest_expression,
          request_input_part->Irequest_expression);
      IFN(sub_request_expression) DPcErr;
      IFE(OgNlpInterpretTreeLogRecursive(ctrl_nlp_th, sub_request_expression, offset + 2));

    }

  }

  DONE;
}

static og_status NlpRequestInputPartWordLog(og_nlp_th ctrl_nlp_th, struct request_input_part *request_input_part,
    int offset)
{
  IFN(request_input_part) DPcErr;

  char string_offset[DPcPathSize];
  memset(string_offset, ' ', offset);
  string_offset[offset] = 0;

  char string_positions[DPcPathSize];
  NlpRequestPositionString(ctrl_nlp_th, request_input_part->request_position_start,
      request_input_part->request_positions_nb, DPcPathSize, string_positions);

  struct request_word *request_word = request_input_part->request_word;
  og_string string_request_word = OgHeapGetCell(ctrl_nlp_th->hba, request_word->start);
  IFN(string_request_word) DPcErr;

  OgMsg(ctrl_nlp_th->hmsg, "", DOgMsgDestInLog, "%s%2d: [%s] '%.*s'", string_offset, request_input_part->level,
      string_positions, DPcPathSize, string_request_word);
  DONE;
}

