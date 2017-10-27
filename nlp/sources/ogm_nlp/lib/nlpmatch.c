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
static og_status NlpMatchExpressions(og_nlp_th ctrl_nlp_th, int level);
static int NlpRequestInputPartCmp(const void *void_request_input_part1, const void *void_request_input_part2);
static og_bool NlpMatchInterpretation(og_nlp_th ctrl_nlp_th, struct interpretation *interpretation);
static og_bool NlpMatchInterpretationInPackage(og_nlp_th ctrl_nlp_th, struct interpretation *interpretation,
    int input_length, unsigned char *input, struct interpret_package *interpret_package);

/**
 * Parse the request and then working on all words of the sentence
 * to encode the matching algorithm.
 */
og_status NlpMatch(og_nlp_th ctrl_nlp_th)
{
  // The request sentence is in : ctrl_nlp_th->request_sentence
  IFE(NlpParse(ctrl_nlp_th));
  if (ctrl_nlp_th->loginfo->trace & DOgNlpTraceMatch)
  {
    IFE(NlpLogRequestWords(ctrl_nlp_th));
  }

  // Scanning all the words and create the list of input parts that match the words
  IFE(NlpMatchWords(ctrl_nlp_th));
  if (ctrl_nlp_th->loginfo->trace & DOgNlpTraceMatch)
  {
    IFE(NlpRequestInputPartsLog(ctrl_nlp_th));
  }

  // Getting all matching expressions and thus interpretations and getting all new input_parts
  int level = 0;
  int at_least_one_input_part_added = 0;
  do
  {
    IFE(at_least_one_input_part_added = NlpMatchExpressions(ctrl_nlp_th, level));
    if (ctrl_nlp_th->loginfo->trace & DOgNlpTraceMatch)
    {
      IFE(NlpRequestInputPartsLog(ctrl_nlp_th));
    }
    level++;
  }
  while (at_least_one_input_part_added);

  IFE(NlpRequestExpressionsExplicit(ctrl_nlp_th));
  IFE(NlpRequestExpressionsLog(ctrl_nlp_th));
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

  if (ctrl_nlp_th->loginfo->trace & DOgNlpTraceMatch)
  {
    OgMsg(ctrl_nlp_th->hmsg, "", DOgMsgDestInLog, "Looking for input parts for string '%s':", string_request_word);
  }

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

  unsigned char out[DPcAutMaxBufferSize + 9];
  oindex states[DPcAutMaxBufferSize + 9];
  int retour, nstate0, nstate1, iout;

  NlpLog(DOgNlpTraceMatch, "  Looking for input parts in package '%s' '%s' from word:", package->slug, package->id)

  if ((retour = OgAufScanf(package->ha_word, input_length, input, &iout, out, &nstate0, &nstate1, states)))
  {
    do
    {
      IFE(retour);
      int Iinput_part;
      unsigned char *p = out;
      IFE(DOgPnin4(ctrl_nlp_th->herr,&p,&Iinput_part));
      NlpLog(DOgNlpTraceMatch, "    found input part %d", Iinput_part)
      IFE(NlpRequestInputPartAddWord(ctrl_nlp_th, request_word, interpret_package, Iinput_part));
    }
    while ((retour = OgAufScann(package->ha_word, &iout, out, nstate0, &nstate1, states)));
  }

  DONE;
}

static og_status NlpMatchExpressions(og_nlp_th ctrl_nlp_th, int level)
{
  og_bool at_least_one_input_part_added = 0;

  int request_input_part_used = OgHeapGetCellsUsed(ctrl_nlp_th->hrequest_input_part);
  {
    struct request_input_part *all_request_input_part = OgHeapGetCell(ctrl_nlp_th->hrequest_input_part, 0);
    IFN(all_request_input_part) DPcErr;

    qsort(all_request_input_part, request_input_part_used, sizeof(struct request_input_part), NlpRequestInputPartCmp);
  }

  if (ctrl_nlp_th->loginfo->trace & DOgNlpTraceMatch)
  {
    IFE(NlpRequestInputPartsLog(ctrl_nlp_th));
  }

  // scan all request input parts
  for (int i = 0; i < request_input_part_used; i++)
  {
    struct request_input_part *request_input_part = OgHeapGetCell(ctrl_nlp_th->hrequest_input_part, i);
    if (request_input_part->consumed == 2) continue;

    struct expression *expression = request_input_part->input_part->expression;

    if (expression->input_parts != request_input_part->input_part) continue;

    // matched expression is shorter than request
    if (expression->input_parts_nb > (request_input_part_used - i)) continue;

    // check next request words in expression
    og_bool found_expression = TRUE;
    for (int j = 1; j < expression->input_parts_nb; j++)
    {
      struct request_input_part *next_request_input_part = request_input_part + j;
      if ((expression->input_parts + j) != next_request_input_part->input_part)
      {
        found_expression = FALSE;
        break;
      }
    }
    if (!found_expression) continue;

    // here we have found an expression, thus an interpretation
    NlpLog(DOgNlpTraceMatch, "found expression '%.*s' with input_parts %d:%d at level %d", DPcPathSize,
        expression->text, i, expression->input_parts_nb, level)
    IFE(NlpRequestExpressionAdd(ctrl_nlp_th, expression, level));

    int at_least_one_input_part_added_here = 0;
    IFE(at_least_one_input_part_added_here = NlpMatchInterpretation(ctrl_nlp_th, expression->interpretation));
    if (at_least_one_input_part_added_here) at_least_one_input_part_added = 1;

    // Can be reallocated by NlpMatchInterpretation
    request_input_part = OgHeapGetCell(ctrl_nlp_th->hrequest_input_part, i);
    IFN(request_input_part) DPcErr;
    for (int j = 0; j < expression->input_parts_nb; j++)
    {
      request_input_part[j].consumed = 1;
    }

    i += expression->input_parts_nb - 1;
  }

  struct request_input_part *request_input_part = OgHeapGetCell(ctrl_nlp_th->hrequest_input_part, 0);
  for (int i = 0; i < request_input_part_used; i++)
  {
    if (request_input_part[i].consumed == 1) request_input_part[i].consumed = 2;
  }

  return (at_least_one_input_part_added);
}

static int NlpRequestInputPartCmp(const void *void_request_input_part1, const void *void_request_input_part2)
{
  struct request_input_part *request_input_part1 = (struct request_input_part *) void_request_input_part1;
  struct request_input_part *request_input_part2 = (struct request_input_part *) void_request_input_part2;

  if (request_input_part1->interpret_package != request_input_part2->interpret_package)
  {
    return (request_input_part1->interpret_package - request_input_part2->interpret_package);
  }
  return (request_input_part1->Iinput_part - request_input_part2->Iinput_part);
}

static og_bool NlpMatchInterpretation(og_nlp_th ctrl_nlp_th, struct interpretation *interpretation)
{
  NlpLog(DOgNlpTraceMatch, "Looking for input parts for interpretation '%s' '%s':", interpretation->slug,
      interpretation->id)

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

    og_bool input_part_added = NlpMatchInterpretationInPackage(ctrl_nlp_th, interpretation, input_length, input,
        interpret_package);
    IFE(input_part_added);
    if (input_part_added) at_least_one_input_part_added = TRUE;
  }

  return at_least_one_input_part_added;
}

static og_bool NlpMatchInterpretationInPackage(og_nlp_th ctrl_nlp_th, struct interpretation *interpretation,
    int input_length, unsigned char *input, struct interpret_package *interpret_package)
{
  package_t package = interpret_package->package;

  unsigned char out[DPcAutMaxBufferSize + 9];
  oindex states[DPcAutMaxBufferSize + 9];
  int retour, nstate0, nstate1, iout;

  NlpLog(DOgNlpTraceMatch, "  Looking for input parts in package '%s' '%s' from interpretation:", package->slug,
      package->id)

  int found_input_part = 0;
  if ((retour = OgAufScanf(package->ha_interpretation_id, input_length, input, &iout, out, &nstate0, &nstate1, states)))
  {
    do
    {
      IFE(retour);
      int Iinput_part;
      unsigned char *p = out;
      IFE(DOgPnin4(ctrl_nlp_th->herr,&p,&Iinput_part));
      NlpLog(DOgNlpTraceMatch, "    found input part %d", Iinput_part)
      IFE(NlpRequestInputPartAddInterpretation(ctrl_nlp_th, interpretation, interpret_package, Iinput_part));
      found_input_part = 1;

    }
    while ((retour = OgAufScann(package->ha_interpretation_id, &iout, out, nstate0, &nstate1, states)));
  }

  return (found_input_part);
}

