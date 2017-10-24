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
    unsigned char *input, int Iinterpret_package, package_t package);
static og_status NlpMatchExpressions(og_nlp_th ctrl_nlp_th, int level);
static int NlpRequestInputPartCmp(const void *void_request_input_part1, const void *void_request_input_part2);
static og_status NlpMatchInterpretation(og_nlp_th ctrl_nlp_th, struct interpretation *interpretation);
static og_status NlpMatchInterpretationInPackage(og_nlp_th ctrl_nlp_th, struct interpretation *interpretation,
    og_string interpretation_id, int Iinterpret_package, package_t package);

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
    IFE(at_least_one_input_part_added = NlpMatchExpressions(ctrl_nlp_th,level));
    if (ctrl_nlp_th->loginfo->trace & DOgNlpTraceMatch)
    {
      IFE(NlpRequestInputPartsLog(ctrl_nlp_th));
    }
    level++;
  }
  while (at_least_one_input_part_added);

  IFE(NlpRequestInterpretationsExplicit(ctrl_nlp_th));
  IFE(NlpRequestInterpretationsLog(ctrl_nlp_th));
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

  if (ctrl_nlp_th->loginfo->trace & DOgNlpTraceMatch)
  {
    OgMsg(ctrl_nlp_th->hmsg, "", DOgMsgDestInLog, "Looking for input parts for string '%s':", string_request_word);
  }

  int interpret_package_used = OgHeapGetCellsUsed(ctrl_nlp_th->hinterpret_package);
  for (int i = 0; i < interpret_package_used; i++)
  {
    struct interpret_package *interpret_package = OgHeapGetCell(ctrl_nlp_th->hinterpret_package, i);
    IFN(interpret_package) DPcErr;
    IFE(NlpMatchWordInPackage(ctrl_nlp_th, request_word, input_length, input, i, interpret_package->package));
  }

  DONE;
}

static og_status NlpMatchWordInPackage(og_nlp_th ctrl_nlp_th, struct request_word *request_word, int input_length,
    unsigned char *input, int Iinterpret_package, package_t package)
{
  unsigned char out[DPcAutMaxBufferSize + 9];
  oindex states[DPcAutMaxBufferSize + 9];
  int retour, nstate0, nstate1, iout;

  if (ctrl_nlp_th->loginfo->trace & DOgNlpTraceMatch)
  {
    og_string package_id = OgHeapGetCell(package->hba, package->id_start);
    IFN(package_id) DPcErr;
    og_string package_slug = OgHeapGetCell(package->hba, package->slug_start);
    IFN(package_slug) DPcErr;
    OgMsg(ctrl_nlp_th->hmsg, "", DOgMsgDestInLog, "  Looking for input parts in package '%s' '%s':", package_slug,
        package_id);
  }

  if ((retour = OgAutScanf(package->ha_word, input_length, input, &iout, out, &nstate0, &nstate1, states)))
  {
    do
    {
      IFE(retour);
      int Iinput_part;
      unsigned char *p = out;
      IFE(DOgPnin4(ctrl_nlp_th->herr,&p,&Iinput_part));
      if (ctrl_nlp_th->loginfo->trace & DOgNlpTraceMatch)
      {
        OgMsg(ctrl_nlp_th->hmsg, "", DOgMsgDestInLog, "    found input part %d", Iinput_part);
      }
      IFE(NlpRequestInputPartAddWord(ctrl_nlp_th, request_word, Iinterpret_package, package, Iinput_part));
    }
    while ((retour = OgAutScann(package->ha_word, &iout, out, nstate0, &nstate1, states)));
  }

  DONE;
}

static og_status NlpMatchExpressions(og_nlp_th ctrl_nlp_th, int level)
{
  int at_least_one_input_part_added = 0;
  size_t request_input_part_used = OgHeapGetCellsUsed(ctrl_nlp_th->hrequest_input_part);
  struct request_input_part *all_request_input_part = OgHeapGetCell(ctrl_nlp_th->hrequest_input_part, 0);
  IFN(all_request_input_part) DPcErr;

  qsort(all_request_input_part, request_input_part_used, sizeof(struct request_input_part), NlpRequestInputPartCmp);

  if (ctrl_nlp_th->loginfo->trace & DOgNlpTraceMatch)
  {
    IFE(NlpRequestInputPartsLog(ctrl_nlp_th));
  }

  // scan all request input parts
  for (size_t i = 0; i < request_input_part_used; i++)
  {
    struct request_input_part *request_input_part = OgHeapGetCell(ctrl_nlp_th->hrequest_input_part, i);
    IFN(request_input_part) DPcErr;
    if (request_input_part->consumed) continue;
    struct expression *expression = request_input_part->input_part->expression;
    if (expression->input_part_start != request_input_part->Iinput_part) continue;
    if (i + expression->input_parts_nb > request_input_part_used) continue;
    int found_expression = 1;
    for (int j = 1; j < expression->input_parts_nb; j++)
    {
      if (expression->input_part_start + j != request_input_part[j].Iinput_part)
      {
        found_expression = 0;
        break;
      }
    }
    if (!found_expression) continue;
    // here we have found an expression, thus an interpretation
    og_string text = OgHeapGetCell(expression->interpretation->package->hba, expression->text_start);
    if (ctrl_nlp_th->loginfo->trace & DOgNlpTraceMatch)
    {
      OgMsg(ctrl_nlp_th->hmsg, "", DOgMsgDestInLog, "found expression '%.*s' with input_parts %d:%d at level %d", DPcPathSize, text,
          expression->input_part_start, expression->input_parts_nb, level);
    }
    IFE(NlpRequestInterpretationAdd(ctrl_nlp_th, expression, level));
    i += expression->input_parts_nb - 1;
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
  }
  return (at_least_one_input_part_added);
}

static int NlpRequestInputPartCmp(const void *void_request_input_part1, const void *void_request_input_part2)
{
  struct request_input_part *request_input_part1 = (struct request_input_part *) void_request_input_part1;
  struct request_input_part *request_input_part2 = (struct request_input_part *) void_request_input_part2;

  if (request_input_part1->Iinterpret_package != request_input_part2->Iinterpret_package)
  {
    return (request_input_part1->Iinterpret_package - request_input_part2->Iinterpret_package);
  }
  return (request_input_part1->Iinput_part - request_input_part2->Iinput_part);
}

static og_status NlpMatchInterpretation(og_nlp_th ctrl_nlp_th, struct interpretation *interpretation)
{
  package_t package = interpretation->package;
  og_string interpretation_id = OgHeapGetCell(package->hba, interpretation->id_start);
  IFN(interpretation_id) DPcErr;

  if (ctrl_nlp_th->loginfo->trace & DOgNlpTraceMatch)
  {
    og_string interpretation_slug = OgHeapGetCell(package->hba, interpretation->slug_start);
    IFN(interpretation_slug) DPcErr;
    OgMsg(ctrl_nlp_th->hmsg, "", DOgMsgDestInLog, "Looking for input parts for interpretation '%s' '%s':",
        interpretation_slug, interpretation_id);
  }

  int at_least_one_input_part_added = 0;
  int interpret_package_used = OgHeapGetCellsUsed(ctrl_nlp_th->hinterpret_package);
  for (int i = 0; i < interpret_package_used; i++)
  {
    struct interpret_package *interpret_package = OgHeapGetCell(ctrl_nlp_th->hinterpret_package, i);
    IFN(interpret_package) DPcErr;
    int input_part_added;
    IFE(
        input_part_added = NlpMatchInterpretationInPackage(ctrl_nlp_th, interpretation, interpretation_id, i,
            interpret_package->package));
    if (input_part_added) at_least_one_input_part_added = 1;
  }

  return at_least_one_input_part_added;
}

static og_status NlpMatchInterpretationInPackage(og_nlp_th ctrl_nlp_th, struct interpretation *interpretation,
    og_string interpretation_id, int Iinterpret_package, package_t package)
{
  int Iinput_part;
  int found = g_hash_table_lookup_extended(package->interpretation_id_hash, interpretation_id, NULL,
      (void **) (&Iinput_part));
  if (!found) return (0);

  if (ctrl_nlp_th->loginfo->trace & DOgNlpTraceMatch)
  {
    og_string package_id = OgHeapGetCell(package->hba, package->id_start);
    IFN(package_id) DPcErr;
    og_string package_slug = OgHeapGetCell(package->hba, package->slug_start);
    IFN(package_slug) DPcErr;
    OgMsg(ctrl_nlp_th->hmsg, "", DOgMsgDestInLog, "Found input part %d in package '%s' '%s':", Iinput_part,
        package_slug, package_id);
  }

  IFE(NlpRequestInputPartAddInterpretation(ctrl_nlp_th, interpretation, Iinterpret_package, package, Iinput_part));

  return (1);
}

