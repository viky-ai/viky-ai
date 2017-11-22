/*
 *  Handling log messages for Natural Language Processing library
 *  Copyright (c) 2017 Pertimm, by Patrick Constant
 *  Dev : September 2017
 *  Version 1.0
 */
#include "ogm_nlp.h"
#include <stdlib.h>

/**
 * Parse the request and then working on all words of the sentence
 * to encode the matching algorithm.
 */
og_status NlpMatch(og_nlp_th ctrl_nlp_th)
{
  // The request sentence is in : ctrl_nlp_th->request_sentence
  IFE(NlpParseRequestSentence(ctrl_nlp_th));
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

  if (ctrl_nlp_th->loginfo->trace & DOgNlpTraceMatch)
  {
    IFE(NlpRequestExpressionsLog(ctrl_nlp_th, 0, "List of request expressions finally found:"));
  }
  IFE(NlpRequestExpressionsExplicit(ctrl_nlp_th));

  IFE(NlpWhyCalculate(ctrl_nlp_th));
  DONE;
}

