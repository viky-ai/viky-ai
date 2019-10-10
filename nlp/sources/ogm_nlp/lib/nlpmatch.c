/*
 *  Handling log messages for Natural Language Processing library
 *  Copyright (c) 2017 Pertimm, by Patrick Constant
 *  Dev : September 2017
 *  Version 1.0
 */
#include "ogm_nlp.h"
#include <stdlib.h>

static int NlpMatchCreate(og_nlp_th ctrl_nlp_th);

/**
 * Parse the request and then working on all words of the sentence
 * to encode the matching algorithm.
 */
og_status NlpMatch(og_nlp_th ctrl_nlp_th)
{
  // The request sentence is in : ctrl_nlp_th->request_sentence
  IFE(NlpParseRequestSentence(ctrl_nlp_th));

  // matching the sentence on regular expressions
  IFE(NlpRegexMatch(ctrl_nlp_th));

  IFE(NlpAutoComplete(ctrl_nlp_th));

  IFE(NlpLem(ctrl_nlp_th));

  IFE(NlpLtras(ctrl_nlp_th));

  // function to chain words in order to re-order them if needed
  IFE(NlpMatchWordChainRequestWords(ctrl_nlp_th));

  // function to regroup numbers
  IFE(NlpMatchGroupNumbers(ctrl_nlp_th));

  // if number has been groupes update basic_group_request_word_nb
  IFE(NlpMatchWordChainUpdateWordCount(ctrl_nlp_th));
  IFE(NlpMatchWordChainRequestWords(ctrl_nlp_th));

  if (ctrl_nlp_th->loginfo->trace & DOgNlpTraceMatch)
  {
    IFE(NlpLogRequestWords(ctrl_nlp_th));
  }

  IFE(NlpGlueBuild(ctrl_nlp_th));
  if (ctrl_nlp_th->loginfo->trace & DOgNlpTraceMatch)
  {
    IFE(NlpGlueLog(ctrl_nlp_th));
  }

  // Scanning all the words and create the list of input parts that match the words
  IFE(NlpMatchWords(ctrl_nlp_th));
  IFE(NlpMatchEntities(ctrl_nlp_th));

  if (ctrl_nlp_th->loginfo->trace & DOgNlpTraceMatch)
  {
    IFE(NlpRequestInputPartsLog(ctrl_nlp_th, 0, "List of request input parts after NlpMatchWords and NlpMatchEntities:"));
  }

  IFE(NlpMatchCreate(ctrl_nlp_th));

  if (ctrl_nlp_th->loginfo->trace & DOgNlpTraceMatch)
  {
    IFE(NlpRequestExpressionsLog(ctrl_nlp_th, 0, "List of request expressions finally found:"));
  }

  IFE(NlpRequestExpressionsCalculate(ctrl_nlp_th));

  IFE(NlpWhyCalculate(ctrl_nlp_th));

  DONE;
}

static int NlpMatchCreate(og_nlp_th ctrl_nlp_th)
{
  ctrl_nlp_th->level = 0;
  og_bool super_list_created = FALSE;
  do
  {
    ctrl_nlp_th->accept_any_expressions = FALSE;
    int at_least_one_input_part_added = 0;
    do
    {
      IFE(NlpMatchExpressions(ctrl_nlp_th));
      IFE(NlpRequestExpressionsOptimize(ctrl_nlp_th));
      IFE(at_least_one_input_part_added = NlpMatchInterpretations(ctrl_nlp_th));
      ctrl_nlp_th->level++;
    }
    while (at_least_one_input_part_added);

    ctrl_nlp_th->accept_any_expressions = TRUE;
    at_least_one_input_part_added = 0;
    do
    {
      IFE(NlpMatchExpressions(ctrl_nlp_th));
      IFE(NlpRequestExpressionsOptimize(ctrl_nlp_th));
      IFE(at_least_one_input_part_added = NlpMatchInterpretations(ctrl_nlp_th));
      ctrl_nlp_th->level++;
    }
    while (at_least_one_input_part_added);

    IFE(super_list_created = NlpSuperListsCreate(ctrl_nlp_th));
    if (super_list_created)
    {
      IFE(at_least_one_input_part_added = NlpMatchInterpretations(ctrl_nlp_th));
      ctrl_nlp_th->level++;
    }
  }
  while (super_list_created);
  DONE;
}

