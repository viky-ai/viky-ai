/*
 *  Handling log messages for Natural Language Processing library
 *  Copyright (c) 2017 Pertimm, by Patrick Constant
 *  Dev : September 2017
 *  Version 1.0
 */
#include "ogm_nlp.h"
#include <stdlib.h>

static int NlpMatchValidateListsWithoutAny(og_nlp_th ctrl_nlp_th);

/**
 * Parse the request and then working on all words of the sentence
 * to encode the matching algorithm.
 */
og_status NlpMatch(og_nlp_th ctrl_nlp_th)
{
  // The request sentence is in : ctrl_nlp_th->request_sentence
  IFE(NlpParseRequestSentence(ctrl_nlp_th));
  IFE(NlpAutoComplete(ctrl_nlp_th));
  IFE(NlpLtras(ctrl_nlp_th));
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

  ctrl_nlp_th->accept_any_expressions = FALSE;
  // Getting all matching expressions and thus interpretations and getting all new input_parts
  ctrl_nlp_th->level = 0;
  int at_least_one_input_part_added = 0;
  do
  {
    IFE(NlpMatchExpressions(ctrl_nlp_th));
    IFE(NlpRequestExpressionsOptimize(ctrl_nlp_th));
    IFE(at_least_one_input_part_added = NlpMatchInterpretations(ctrl_nlp_th));
    ctrl_nlp_th->level++;
  }
  while (at_least_one_input_part_added);

  OgMsg(ctrl_nlp_th->hmsg, "", DOgMsgDestInLog, "\nLooking for any expressions");

  IFE(NlpMatchValidateListsWithoutAny(ctrl_nlp_th));

  ctrl_nlp_th->accept_any_expressions = TRUE;
  // Getting all matching expressions and thus interpretations and getting all new input_parts
  at_least_one_input_part_added = 0;
  do
  {
    IFE(NlpMatchExpressions(ctrl_nlp_th));
    IFE(NlpRequestExpressionsOptimize(ctrl_nlp_th));
    IFE(at_least_one_input_part_added = NlpMatchInterpretations(ctrl_nlp_th));
    ctrl_nlp_th->level++;
  }
  while (at_least_one_input_part_added);

  if (ctrl_nlp_th->loginfo->trace & DOgNlpTraceMatch)
  {
    IFE(NlpRequestExpressionsLog(ctrl_nlp_th, 0, "List of request expressions finally found:"));
  }
  IFE(NlpRequestExpressionsCalculate(ctrl_nlp_th));

  IFE(NlpWhyCalculate(ctrl_nlp_th));
  DONE;
}

static int NlpMatchValidateListsWithoutAny(og_nlp_th ctrl_nlp_th)
{
  if (ctrl_nlp_th->loginfo->trace & DOgNlpTraceMatch)
  {
    char buffer[DPcPathSize];
    snprintf(buffer, DPcPathSize, "NlpMatchValidateListWithoutAny: list of all request expressions at level %d:",
        ctrl_nlp_th->level);
    IFE(NlpRequestExpressionsLog(ctrl_nlp_th, 0, buffer));
  }
  int request_expression_used = OgHeapGetCellsUsed(ctrl_nlp_th->hrequest_expression);
  struct request_expression *request_expressions = OgHeapGetCell(ctrl_nlp_th->hrequest_expression, 0);

  for (int i = request_expression_used - 1; i >= 0; i--)
  {
    struct request_expression *request_expression = request_expressions + i;
    IFN(request_expression) DPcErr;
    request_expression->recursive_without_any_chosen = FALSE;
    if (request_expression->expression->is_recursive)
    {
      // TODO : there can be several recursive lists
      request_expression->recursive_without_any_chosen = TRUE;
      break;
    }
  }
  DONE;
}

