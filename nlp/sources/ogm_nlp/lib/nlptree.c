/*
 *  Handling parsing tree from a request expression
 *  Copyright (c) 2017 Pertimm, by Patrick Constant
 *  Dev : October 2017
 *  Version 1.0
 */
#include "ogm_nlp.h"

static og_status NlpInterpretTreeAttachAnyRecursive(og_nlp_th ctrl_nlp_th,
    struct request_expression *root_request_expression, struct request_expression *request_expression, int offset);

static og_status NlpInterpretTreeLogRecursive(og_nlp_th ctrl_nlp_th, struct request_expression *root_request_expression,
    struct request_expression *request_expression, int offset);
static og_status NlpRequestInputPartWordLog(og_nlp_th ctrl_nlp_th, struct request_input_part *request_input_part,
    int offset);

og_status NlpInterpretTreeAttachAny(og_nlp_th ctrl_nlp_th, struct request_expression *request_expression)
{
  IFE(NlpInterpretTreeAttachAnyRecursive(ctrl_nlp_th, request_expression, request_expression, 0));
  DONE;
}

static og_status NlpInterpretTreeAttachAnyRecursive(og_nlp_th ctrl_nlp_th,
    struct request_expression *root_request_expression, struct request_expression *request_expression, int offset)
{
  struct original_request_input_part *original_request_input_part = OgHeapGetCell(
      ctrl_nlp_th->horiginal_request_input_part, 0);
  IFN(original_request_input_part) DPcErr;

  struct orip *orip = OgHeapGetCell(ctrl_nlp_th->horip, 0);
  IFN(orip) DPcErr;

  for (int i = 0; i < request_expression->orips_nb; i++)
  {
    struct request_input_part *request_input_part = NlpGetRequestInputPart(ctrl_nlp_th, request_expression, i);
    IFN(request_input_part) DPcErr;

    if (request_input_part->type == nlp_input_part_type_Word) ;
    else if (request_input_part->type == nlp_input_part_type_Interpretation)
    {
      struct request_expression *sub_request_expression = OgHeapGetCell(ctrl_nlp_th->hrequest_expression,
          request_input_part->Irequest_expression);
      IFN(sub_request_expression) DPcErr;
      IFE(NlpInterpretTreeAttachAnyRecursive(ctrl_nlp_th, root_request_expression, sub_request_expression, offset + 2));
    }

    if (request_expression->expression->alias_any_input_part_position == i + 1)
    {
      IFE(NlpRequestAnyAddClosest(ctrl_nlp_th, root_request_expression, request_expression));
    }

  }

  DONE;
}

og_status NlpInterpretTreeLog(og_nlp_th ctrl_nlp_th, struct request_expression *request_expression)
{
  OgMsg(ctrl_nlp_th->hmsg, "", DOgMsgDestInLog, "Tree representation for expression:");
  IFE(NlpInterpretTreeLogRecursive(ctrl_nlp_th, request_expression, request_expression, 0));
  DONE;
}

static og_status NlpInterpretTreeLogRecursive(og_nlp_th ctrl_nlp_th, struct request_expression *root_request_expression,
    struct request_expression *request_expression, int offset)
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
    struct request_input_part *request_input_part = NlpGetRequestInputPart(ctrl_nlp_th, request_expression, i);
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
      IFE(NlpInterpretTreeLogRecursive(ctrl_nlp_th, root_request_expression, sub_request_expression, offset + 2));
    }

    if (request_expression->expression->alias_any_input_part_position == i + 1)
    {
      if (request_expression->Irequest_any >= 0)
      {
        struct request_any *request_any = OgHeapGetCell(ctrl_nlp_th->hrequest_any, request_expression->Irequest_any);
        IFN(request_any) DPcErr;

        char string_any[DPcPathSize];
        NlpRequestAnyString(ctrl_nlp_th, request_any, DPcPathSize, string_any);

        char string_any_position[DPcPathSize];
        NlpRequestAnyPositionString(ctrl_nlp_th, request_any, DPcPathSize, string_any_position);

        char highlight[DPcPathSize];
        NlpRequestAnyStringPretty(ctrl_nlp_th, request_any, DPcPathSize, highlight);

        OgMsg(ctrl_nlp_th->hmsg, "", DOgMsgDestInLog, "  %s%2d: '%s' [%s] any: '%s'", string_offset,
            request_input_part->level, string_any, string_any_position, highlight);
      }
      else
      {
        OgMsg(ctrl_nlp_th->hmsg, "", DOgMsgDestInLog, "  %s%2d: [] any: not found", string_offset,
            request_input_part->level);
      }
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

  char digit[DPcPathSize];
  digit[0] = 0;
  if (request_word->is_digit)
  {
    snprintf(digit, DPcPathSize, " -> %d", request_word->digit_value);
  }

  OgMsg(ctrl_nlp_th->hmsg, "", DOgMsgDestInLog, "%s%2d:%d [%s] '%.*s'%s", string_offset,
      request_input_part->Ioriginal_request_input_part, request_input_part->level, string_positions, DPcPathSize,
      string_request_word, digit);
  DONE;
}

