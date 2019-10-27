/*
 *  Handling request input parts
 *  Copyright (c) 2017 Pertimm, by Patrick Constant
 *  Dev : October 2017
 *  Version 1.0
 */
#include "ogm_nlp.h"

static struct request_input_part *NlpRequestInputPartAdd(og_nlp_th ctrl_nlp_th,
    struct interpret_package *interpret_package, int Iinput_part);
static og_status NlpRequestInputPartGetSparseMark(og_nlp_th ctrl_nlp_th, struct request_input_part *request_input_part);

og_status NlpRequestInputPartAddWord(og_nlp_th ctrl_nlp_th, struct request_word *request_word,
    struct interpret_package *interpret_package, int Iinput_part, og_bool interpret_word_as_number,
    double score_spelling)
{
  struct request_input_part *request_input_part = NlpRequestInputPartAdd(ctrl_nlp_th, interpret_package, Iinput_part);
  IFN(request_input_part) DPcErr;
  request_input_part->type = nlp_input_part_type_Word;
  request_input_part->request_word = request_word;

  size_t Irequest_position;
  IFE(
      NlpRequestPositionAdd(ctrl_nlp_th, request_word->start_position, request_word->length_position,
          &Irequest_position));
  request_input_part->request_position_start = Irequest_position;
  request_input_part->request_positions_nb = 1;
  request_input_part->request_position_distance = 0;
  request_input_part->interpret_word_as_number = interpret_word_as_number;
  request_input_part->score_spelling = score_spelling;

  IFE(NlpRequestInputPartGetSparseMark(ctrl_nlp_th, request_input_part));

  DONE;
}

og_status NlpRequestInputPartAddInterpretation(og_nlp_th ctrl_nlp_th, struct request_expression *request_expression,
    struct interpret_package *interpret_package, int Iinput_part)
{
  struct request_input_part *request_input_part = NlpRequestInputPartAdd(ctrl_nlp_th, interpret_package, Iinput_part);
  IFN(request_input_part) DPcErr;
  request_input_part->type = nlp_input_part_type_Interpretation;
  request_input_part->Irequest_expression = request_expression->self_index;

  // prealloc due to app call
  request_input_part->request_position_start = OgHeapAddCells(ctrl_nlp_th->hrequest_position,
      request_expression->request_positions_nb);

  struct request_position *request_position_from = OgHeapGetCell(ctrl_nlp_th->hrequest_position,
      request_expression->request_position_start);
  IFN(request_position_from) DPcErr;

  struct request_position *request_position_to = OgHeapGetCell(ctrl_nlp_th->hrequest_position,
      request_input_part->request_position_start);
  IFN(request_position_to) DPcErr;

  memcpy(request_position_to, request_position_from,
      sizeof(struct request_position) * request_expression->request_positions_nb);

  request_input_part->request_positions_nb = request_expression->request_positions_nb;

  IFE(NlpRequestInputPartGetSparseMark(ctrl_nlp_th, request_input_part));

  return (request_input_part->Ioriginal_request_input_part);
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
  request_input_part->Ioriginal_request_input_part = Irequest_input_part;
  request_input_part->level = ctrl_nlp_th->level;
  request_input_part->safe_request_position_start = (-1);
  request_input_part->safe_request_positions_nb = 0;
  request_input_part->super_list_status = nlp_super_list_status_Nil;

  return request_input_part;
}

struct request_input_part *NlpGetRequestInputPart(og_nlp_th ctrl_nlp_th, struct request_expression *request_expression,
    int Iorip)
{
  struct orip *orip = OgHeapGetCell(ctrl_nlp_th->horip, request_expression->orip_start + Iorip);
  IFN(orip) return NULL;
  struct original_request_input_part *original_request_input_part = OgHeapGetCell(
      ctrl_nlp_th->horiginal_request_input_part, orip->Ioriginal_request_input_part);
  IFN(original_request_input_part) return NULL;
  struct request_input_part *request_input_part = OgHeapGetCell(ctrl_nlp_th->hrequest_input_part,
      original_request_input_part->Irequest_input_part);
  IFN(request_input_part) return NULL;
  return request_input_part;
}

og_bool NlpRequestInputPartsAreOrdered(og_nlp_th ctrl_nlp_th, struct request_input_part *request_input_part1,
    struct request_input_part *request_input_part2)
{
  return NlpRequestPositionsAreOrdered(ctrl_nlp_th, request_input_part1->request_position_start,
      request_input_part1->request_positions_nb, request_input_part2->request_position_start,
      request_input_part2->request_positions_nb);
}

og_bool NlpRequestInputPartsAreGlued(og_nlp_th ctrl_nlp_th, struct request_input_part *request_input_part1,
    struct request_input_part *request_input_part2, og_bool keep_order)
{
  if (request_input_part1->type == nlp_input_part_type_Interpretation)
  {
    struct request_expression *sub_request_expression = OgHeapGetCell(ctrl_nlp_th->hrequest_expression,
        request_input_part1->Irequest_expression);
    IFN(sub_request_expression) DPcErr;
    int any_topology = DOgNlpAnyTopologyBothSide;
    if (keep_order) any_topology = DOgNlpAnyTopologyRight;
    if (sub_request_expression->any_topology & any_topology) return (TRUE);
  }

  if (request_input_part2->type == nlp_input_part_type_Interpretation)
  {
    struct request_expression *sub_request_expression = OgHeapGetCell(ctrl_nlp_th->hrequest_expression,
        request_input_part2->Irequest_expression);
    IFN(sub_request_expression) DPcErr;
    int any_topology = DOgNlpAnyTopologyBothSide;
    if (keep_order) any_topology = DOgNlpAnyTopologyLeft;
    if (sub_request_expression->any_topology & any_topology) return (TRUE);
  }

  return NlpRequestPositionsAreGlued(ctrl_nlp_th, request_input_part1->request_position_start,
      request_input_part1->request_positions_nb, request_input_part2->request_position_start,
      request_input_part2->request_positions_nb);
}

og_bool NlpRequestInputPartsAreExpressionGlued(og_nlp_th ctrl_nlp_th, struct request_input_part *request_input_part1,
    struct request_input_part *request_input_part2)
{
  og_bool is_glued = FALSE;
  // All words between end of request_input_part1 and start of request_input_part2
  // must be expression punctuations
  struct request_position *request_position = OgHeapGetCell(ctrl_nlp_th->hrequest_position,
      request_input_part1->request_position_start + request_input_part1->request_positions_nb - 1);
  IFN(request_position) DPcErr;
  int end_request_input_part1 = request_position->start + request_position->length;
  request_position = OgHeapGetCell(ctrl_nlp_th->hrequest_position, request_input_part2->request_position_start);
  IFN(request_position) DPcErr;
  int start_request_input_part2 = request_position->start;

  struct request_word *request_words = OgHeapGetCell(ctrl_nlp_th->hrequest_word, 0);
  IFN(request_words) DPcErr;
  int basic_request_word_used = ctrl_nlp_th->basic_request_word_used;
  og_bool all_words_in_between_are_expression_punctuations = TRUE;
  int Irequest_word_start;
  og_bool found = NlpRequestWordGet(ctrl_nlp_th, end_request_input_part1, &Irequest_word_start);
  IFE(found);
  if (found)
  {
    struct request_word *request_word_before = request_words + Irequest_word_start - 1;
    int end_request_word_before = request_word_before->start_position + request_word_before->length_position;
    NlpLog(DOgNlpTraceMatchExpression,
        "NlpRequestInputPartsAreExpressionGlued: end_request_word_before=%d end_request_input_part1=%d Irequest_word_start=%d",
        end_request_word_before, end_request_input_part1, Irequest_word_start);
    // This can happen with regexes that match the beginning of the word, in that case, there are some chars
    // between the two words which are not punctuations.
    if (end_request_word_before > end_request_input_part1) return(FALSE);

    for (int w = Irequest_word_start; w < basic_request_word_used; w++)
    {
      struct request_word *request_word = request_words + w;
      if (request_word->start_position + request_word->length_position <= start_request_input_part2)
      {
        if (!request_word->is_expression_punctuation)
        {
          all_words_in_between_are_expression_punctuations = FALSE;
          break;
        }
      }
      break;
    }
  }
  if (all_words_in_between_are_expression_punctuations) is_glued = TRUE;
  return (is_glued);
}

static og_status NlpRequestInputPartGetSparseMark(og_nlp_th ctrl_nlp_th, struct request_input_part *request_input_part)
{
  request_input_part->sparse_mark = NlpRequestPositionDistance(ctrl_nlp_th, request_input_part->request_position_start,
      request_input_part->request_positions_nb);
  return (request_input_part->sparse_mark);
}

og_status NlpRequestInputPartsLog(og_nlp_th ctrl_nlp_th, int request_input_part_start, char *title)
{
  OgMsg(ctrl_nlp_th->hmsg, "", DOgMsgDestInLog, "%s", title);
  int request_input_part_used = OgHeapGetCellsUsed(ctrl_nlp_th->hrequest_input_part);
  for (int i = request_input_part_start; i < request_input_part_used; i++)
  {
    IFE(NlpRequestInputPartLog(ctrl_nlp_th, i));
  }

  DONE;
}

og_status NlpRequestInputPartLog(og_nlp_th ctrl_nlp_th, int Irequest_input_part)
{
  struct request_input_part *request_input_part = OgHeapGetCell(ctrl_nlp_th->hrequest_input_part, Irequest_input_part);
  IFN(request_input_part) DPcErr;

  char string_positions[DPcPathSize];
  NlpRequestPositionString(ctrl_nlp_th, request_input_part->request_position_start,
      request_input_part->request_positions_nb, DPcPathSize, string_positions);

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
      char number[DPcPathSize];
      number[0] = 0;
      if (request_word->is_number)
      {
        snprintf(number, DPcPathSize, " -> " DOgPrintDouble, request_word->number_value);
      }
      snprintf(string_input_part, DPcPathSize, "[%s] '%s' word:%s%s %d:%d", string_positions,
          request_input_part->input_part->expression->text, string_request_word, number, request_word->start_position,
          request_word->length_position);
      break;
    }
    case nlp_input_part_type_Interpretation:
    {
      struct request_expression *request_expression = OgHeapGetCell(ctrl_nlp_th->hrequest_expression,
          request_input_part->Irequest_expression);
      IFN(request_expression) DPcErr;
      struct interpretation *interpretation = request_expression->expression->interpretation;
      snprintf(string_input_part, DPcPathSize, "[%s] '%s' interpretation: '%s'", string_positions,
          request_input_part->input_part->expression->text, interpretation->slug);
      break;
    }
    case nlp_input_part_type_Number:
    {
      // should not be used
      snprintf(string_input_part, DPcPathSize, "number");
      break;
    }
    case nlp_input_part_type_Regex:
    {
      // should not be used
      snprintf(string_input_part, DPcPathSize, "regex");
      break;
    }

  }

  OgMsg(ctrl_nlp_th->hmsg, "", DOgMsgDestInLog, "%2d %4d %4d:%d %s", request_input_part->interpret_package->self_index,
      Irequest_input_part, request_input_part->Iinput_part, request_input_part->level, string_input_part);

  DONE;
}
