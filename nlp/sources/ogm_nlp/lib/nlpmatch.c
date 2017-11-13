/*
 *  Handling log messages for Natural Language Processing library
 *  Copyright (c) 2017 Pertimm, by Patrick Constant
 *  Dev : September 2017
 *  Version 1.0
 */
#include "ogm_nlp.h"
#include <stdlib.h>

static og_status NlpMatchWords(og_nlp_th ctrl_nlp_th);
static og_status NlpMatchWord(og_nlp_th ctrl_nlp_th, int Irequest_word);
static og_status NlpMatchWordInPackage(og_nlp_th ctrl_nlp_th, struct request_word *request_word, int input_length,
    unsigned char *input, struct interpret_package *interpret_package);
static og_status NlpMatchExpressions(og_nlp_th ctrl_nlp_th);
static int NlpMatchExpressionsZone(og_nlp_th ctrl_nlp_th, int Irequest_input_part,
    struct match_zone_input_part *match_zone_input_part, int match_zone_input_part_length);
static int NlpMatchExpressionsZoneRecursive(og_nlp_th ctrl_nlp_th, int Irequest_input_part,
    struct match_zone_input_part *match_zone_input_part, int start, int length);
static int NlpMatchExpression(og_nlp_th ctrl_nlp_th, int Irequest_input_part,
    struct match_zone_input_part *match_zone_input_part);
static int NlpRequestInputPartCmp(gconstpointer ptr_request_input_part1, gconstpointer ptr_request_input_part2,
    gpointer user_data);
static og_bool NlpMatchInterpretation(og_nlp_th ctrl_nlp_th, struct request_expression *request_expression);
static og_bool NlpMatchInterpretationInPackage(og_nlp_th ctrl_nlp_th, struct request_expression *request_expression,
    int input_length, unsigned char *input, struct interpret_package *interpret_package);

/**
 * Parse the request and then working on all words of the sentence
 * to encode the matching algorithm.
 */
og_status NlpMatch(og_nlp_th ctrl_nlp_th)
{
  // The request sentence is in : ctrl_nlp_th->request_sentence
  IFE(NlpParseRequestSentence(ctrl_nlp_th));
  if (ctrl_nlp_th->loginfo->trace & DOgNlpTraceMatch)
  {
    IFE(NlpLogRequestWords(ctrl_nlp_th));
  }
  IFE(NlpGlueBuild(ctrl_nlp_th));

  // Scanning all the words and create the list of input parts that match the words
  IFE(NlpMatchWords(ctrl_nlp_th));
  if (ctrl_nlp_th->loginfo->trace & DOgNlpTraceMatch)
  {
    IFE(NlpRequestInputPartsLog(ctrl_nlp_th, 0, "List of request input parts after NlpMatchWords:"));
  }

  // Getting all matching expressions and thus interpretations and getting all new input_parts
  ctrl_nlp_th->level = 0;
  int at_least_one_input_part_added = 0;
  do
  {
    IFE(at_least_one_input_part_added = NlpMatchExpressions(ctrl_nlp_th));
    IFE(NlpRequestExpressionsOptimize(ctrl_nlp_th));
    ctrl_nlp_th->level++;
  }
  while (at_least_one_input_part_added);

  if (ctrl_nlp_th->loginfo->trace & DOgNlpTraceMatch)
  {
    IFE(NlpRequestExpressionsLog(ctrl_nlp_th, 0, "List of request expressions finally found:"));
  }
  IFE(NlpRequestExpressionsExplicit(ctrl_nlp_th));
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

static og_status NlpMatchExpressions(og_nlp_th ctrl_nlp_th)
{
  og_bool at_least_one_input_part_added = 0;

  int request_expression_used = OgHeapGetCellsUsed(ctrl_nlp_th->hrequest_expression);
  int request_input_part_used = OgHeapGetCellsUsed(ctrl_nlp_th->hrequest_input_part);

  ctrl_nlp_th->new_request_expression_start = request_expression_used;
  ctrl_nlp_th->new_request_input_part_start = request_input_part_used;

  {
    struct request_input_part *all_request_input_part = OgHeapGetCell(ctrl_nlp_th->hrequest_input_part, 0);
    IFN(all_request_input_part) DPcErr;

    g_qsort_with_data(all_request_input_part, request_input_part_used, sizeof(struct request_input_part),
        NlpRequestInputPartCmp, ctrl_nlp_th);

    IFE(NlpOriginalRequestInputPartsCalculate(ctrl_nlp_th));
  }

  if (ctrl_nlp_th->loginfo->trace & DOgNlpTraceMatch)
  {
    char buffer[DPcPathSize];
    snprintf(buffer, DPcPathSize, "Looking for expressions at level %d using the following input parts:",
        ctrl_nlp_th->level);
    IFE(NlpRequestInputPartsLog(ctrl_nlp_th, 0, buffer));
  }

  // scan all request input parts
  for (int i = 0; i < request_input_part_used; i++)
  {
    struct request_input_part *request_input_part = OgHeapGetCell(ctrl_nlp_th->hrequest_input_part, i);
    struct expression *expression = request_input_part->input_part->expression;

    if (expression->input_parts != request_input_part->input_part) continue;

    // not enough input parts to match the expression
    if (expression->input_parts_nb > (request_input_part_used - i)) continue;

    struct match_zone_input_part match_zone_input_part[DOgMatchZoneInputPartSize];
    if (expression->input_parts_nb > DOgMatchZoneInputPartSize)
    {
      NlpThrowErrorTh(ctrl_nlp_th,
          "NlpMatchExpressions: expression->input_parts_nb (%d) > DOgMatchZoneInputPartSize (%d)",
          expression->input_parts_nb, DOgMatchZoneInputPartSize);
      DPcErr;
    }
    og_bool expression_matches_at_least_once = FALSE;
    int Iinput_part = 0, start = i, j = i;
    for (; j < request_input_part_used; j++)
    {
      struct request_input_part *rip = OgHeapGetCell(ctrl_nlp_th->hrequest_input_part, j);
      struct input_part *input_part = expression->input_parts;
      input_part += Iinput_part;
      if (input_part != rip->input_part)
      {
        int length = j - start;
        if (length <= 0) break;
        match_zone_input_part[Iinput_part].start = start;
        match_zone_input_part[Iinput_part].length = length;
        Iinput_part++;
        if (Iinput_part >= expression->input_parts_nb)
        {
          expression_matches_at_least_once = TRUE;
          break;
        }
        start = j;
        j--;
      }
    }
    int length = j - start;
    if (length > 0)
    {
      match_zone_input_part[Iinput_part].start = start;
      match_zone_input_part[Iinput_part].length = length;
      Iinput_part++;
      if (Iinput_part >= expression->input_parts_nb)
      {
        expression_matches_at_least_once = TRUE;
      }
    }
    if (!expression_matches_at_least_once) continue;
    int at_least_one_input_part_added_here = NlpMatchExpressionsZone(ctrl_nlp_th, i, match_zone_input_part,
        expression->input_parts_nb);
    IFE(at_least_one_input_part_added_here);
    if (at_least_one_input_part_added_here) at_least_one_input_part_added = 1;
    //i = j - 1;
  }

  if (at_least_one_input_part_added)
  {
    if (ctrl_nlp_th->loginfo->trace & DOgNlpTraceMatch)
    {
      char buffer[DPcPathSize];
      snprintf(buffer, DPcPathSize, "List of new request expression at level %d:", ctrl_nlp_th->level);
      IFE(NlpRequestExpressionsLog(ctrl_nlp_th, request_expression_used, buffer));

      snprintf(buffer, DPcPathSize, "List of new request input parts deducted at level %d:", ctrl_nlp_th->level);
      IFE(NlpRequestInputPartsLog(ctrl_nlp_th, request_input_part_used, buffer));
    }

  }

  IFE(OgNlpSynchroTestSleepIfTimeoutNeeded(ctrl_nlp_th, nlp_timeout_in_NlpMatchExpressions));

  return (at_least_one_input_part_added);
}

static int NlpMatchExpressionsZone(og_nlp_th ctrl_nlp_th, int Irequest_input_part,
    struct match_zone_input_part *match_zone_input_part, int match_zone_input_part_length)
{
  return (NlpMatchExpressionsZoneRecursive(ctrl_nlp_th, Irequest_input_part, match_zone_input_part, 0,
      match_zone_input_part_length));
}

static int NlpMatchExpressionsZoneRecursive(og_nlp_th ctrl_nlp_th, int Irequest_input_part,
    struct match_zone_input_part *match_zone_input_part, int start, int length)
{
  if (length == 0)
  {
    return (NlpMatchExpression(ctrl_nlp_th, Irequest_input_part, match_zone_input_part));
  }
  int at_least_one_input_part_added = 0;
  for (int i = 0; i < match_zone_input_part[start].length; i++)
  {
    match_zone_input_part[start].current = match_zone_input_part[start].start + i;
    int at_least_one_input_part_added_here;
    at_least_one_input_part_added_here = NlpMatchExpressionsZoneRecursive(ctrl_nlp_th, Irequest_input_part,
        match_zone_input_part, start + 1, length - 1);
    IFE(at_least_one_input_part_added_here);
    if (at_least_one_input_part_added_here) at_least_one_input_part_added = 1;
  }
  return (at_least_one_input_part_added);
}

static int NlpMatchExpression(og_nlp_th ctrl_nlp_th, int Irequest_input_part,
    struct match_zone_input_part *match_zone_input_part)
{
  struct request_input_part *request_input_part = OgHeapGetCell(ctrl_nlp_th->hrequest_input_part, Irequest_input_part);
  IFN(request_input_part) DPcErr;
  struct expression *expression = request_input_part->input_part->expression;
  struct request_expression *request_expression = NULL;
  og_bool request_expression_added = NlpRequestExpressionAdd(ctrl_nlp_th, expression, match_zone_input_part,
      &request_expression);
  IF(request_expression_added) DPcErr;

  int at_least_one_input_part_added = 0;

  if (request_expression_added)
  {
    char string_input_parts[DPcPathSize];
    string_input_parts[0] = 0;
    for (int i = 0; i < expression->input_parts_nb; i++)
    {
      sprintf(string_input_parts + strlen(string_input_parts), "%s%d", (i ? ":" : ""),
          match_zone_input_part[i].current);
    }

    if (ctrl_nlp_th->loginfo->trace & DOgNlpTraceMatch)
    {
      char string_positions[DPcPathSize];
      NlpRequestPositionString(ctrl_nlp_th, request_expression->request_position_start,
          request_expression->request_positions_nb, DPcPathSize, string_positions);

      char highlight[DPcPathSize];
      NlpRequestPositionStringHighlight(ctrl_nlp_th, request_expression->request_position_start,
          request_expression->request_positions_nb, DPcPathSize, highlight);

      NlpLog(DOgNlpTraceMatch, "found expression '%.*s' [%s] with request_input_parts %s at level %d : '%s'",
          DPcPathSize, expression->text, string_positions, string_input_parts, ctrl_nlp_th->level, highlight)
    }
    IFE(at_least_one_input_part_added = NlpMatchInterpretation(ctrl_nlp_th, request_expression));
  }

  return at_least_one_input_part_added;
}

static int NlpRequestInputPartCmp(gconstpointer ptr_request_input_part1, gconstpointer ptr_request_input_part2,
    gpointer user_data)
{
  struct request_input_part *request_input_part1 = (struct request_input_part *) ptr_request_input_part1;
  struct request_input_part *request_input_part2 = (struct request_input_part *) ptr_request_input_part2;
  og_nlp_th ctrl_nlp_th = user_data;

  if (request_input_part1->interpret_package != request_input_part2->interpret_package)
  {
    return (request_input_part1->interpret_package - request_input_part2->interpret_package);
  }
  if (request_input_part1->Iinput_part != request_input_part2->Iinput_part)
  {
    return (request_input_part1->Iinput_part - request_input_part2->Iinput_part);
  }

  struct request_position *request_position1 = OgHeapGetCell(ctrl_nlp_th->hrequest_position,
      request_input_part1->request_position_start);
  struct request_position *request_position2 = OgHeapGetCell(ctrl_nlp_th->hrequest_position,
      request_input_part2->request_position_start);

  return (request_position1->start - request_position2->start);
}

static og_bool NlpMatchInterpretation(og_nlp_th ctrl_nlp_th, struct request_expression *request_expression)
{
  struct interpretation *interpretation = request_expression->expression->interpretation;
  NlpLog(DOgNlpTraceMatch, "Looking for input parts for interpretation '%s' '%s' containing expression '%.*s':",
      interpretation->slug, interpretation->id, DPcPathSize, request_expression->expression->text);

  unsigned char input[DPcAutMaxBufferSize + 9];
  int input_length = strlen(interpretation->id);
  memcpy(input, interpretation->id, input_length);
  input[input_length++] = '\1';
  input[input_length] = 0;

  og_bool at_least_one_input_part_added = FALSE;
  int interpret_package_used = OgHeapGetCellsUsed(ctrl_nlp_th->hinterpret_package);
  for (int i = 0; i < interpret_package_used; i++)
  {
    struct interpret_package *interpret_package = OgHeapGetCell(ctrl_nlp_th->hinterpret_package, i);
    IFN(interpret_package) DPcErr;

    og_bool input_part_added = NlpMatchInterpretationInPackage(ctrl_nlp_th, request_expression, input_length, input,
        interpret_package);
    IFE(input_part_added);
    if (input_part_added) at_least_one_input_part_added = TRUE;
  }

  return at_least_one_input_part_added;
}

static og_bool NlpMatchInterpretationInPackage(og_nlp_th ctrl_nlp_th, struct request_expression *request_expression,
    int input_length, unsigned char *input, struct interpret_package *interpret_package)
{
  package_t package = interpret_package->package;

  unsigned char out[DPcAutMaxBufferSize + 9];
  oindex states[DPcAutMaxBufferSize + 9];
  int retour, nstate0, nstate1, iout;

  int found_input_part = 0;
  if ((retour = OgAufScanf(package->ha_interpretation_id, input_length, input, &iout, out, &nstate0, &nstate1, states)))
  {
    do
    {
      IFE(retour);
      int Iinput_part;
      unsigned char *p = out;
      IFE(DOgPnin4(ctrl_nlp_th->herr,&p,&Iinput_part));
      NlpLog(DOgNlpTraceMatch, "    found input part %d in interpret package %d", Iinput_part,
          interpret_package->self_index)
      IFE(NlpRequestInputPartAddInterpretation(ctrl_nlp_th, request_expression, interpret_package, Iinput_part));
      found_input_part = 1;

    }
    while ((retour = OgAufScann(package->ha_interpretation_id, &iout, out, nstate0, &nstate1, states)));
  }

  return (found_input_part);
}

