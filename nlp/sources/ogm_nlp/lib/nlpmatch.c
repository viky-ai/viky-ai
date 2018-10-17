/*
 *  Handling log messages for Natural Language Processing library
 *  Copyright (c) 2017 Pertimm, by Patrick Constant
 *  Dev : September 2017
 *  Version 1.0
 */
#include "ogm_nlp.h"
#include <stdlib.h>

static int NlpMatchValidateListsWithoutAny(og_nlp_th ctrl_nlp_th);
static og_status NlpMatchRegexs(og_nlp_th ctrl_nlp_th);
static og_status NlpRegexAddWordsPackage(og_nlp_th ctrl_nlp_th, struct interpret_package *interpret_package);


/**
 * Parse the request and then working on all words of the sentence
 * to encode the matching algorithm.
 */
og_status NlpMatch(og_nlp_th ctrl_nlp_th)
{
  // The request sentence is in : ctrl_nlp_th->request_sentence
  IFE(NlpParseRequestSentence(ctrl_nlp_th));

  // matching the sentence on regular expressions
  IFE(NlpMatchRegexs(ctrl_nlp_th));

  IFE(NlpAutoComplete(ctrl_nlp_th));

  IFE(NlpLtras(ctrl_nlp_th));

  // function to chain words in order to re-order them if needed
  IFE(NlpMatchWordChainRequestWords(ctrl_nlp_th));

  // function to regroup numbers
  IFE(NlpMatchGroupNumbers(ctrl_nlp_th));

  // if number has been groupes update basic_group_request_word_nb
  IFE(NlpMatchWordChainUpdateWordCount(ctrl_nlp_th));

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
    snprintf(buffer, DPcPathSize, "NlpMatchValidateListWithoutAny: list of all request expressions at level %d before:",
        ctrl_nlp_th->level);
    IFE(NlpRequestExpressionsLog(ctrl_nlp_th, 0, buffer));
  }

  GQueue chosen_recursive_interpretation[1];
  g_queue_init(chosen_recursive_interpretation);

  int request_expression_used = OgHeapGetCellsUsed(ctrl_nlp_th->hrequest_expression);
  struct request_expression *request_expressions = OgHeapGetCell(ctrl_nlp_th->hrequest_expression, 0);

  for (int i = request_expression_used - 1; i >= 0; i--)
  {
    struct request_expression *request_expression = request_expressions + i;
    IFN(request_expression) DPcErr;
    request_expression->recursive_without_any_chosen = FALSE;
    if (request_expression->expression->interpretation->is_recursive)
    {
      og_bool found_interpretation = FALSE;
      for (GList *iter = chosen_recursive_interpretation->head; iter; iter = iter->next)
      {
        struct interpretation *interpretation = iter->data;
        if (interpretation == request_expression->expression->interpretation)
        {
          found_interpretation = TRUE;
          break;
        }
      }
      if (!found_interpretation)
      {
        g_queue_push_tail(chosen_recursive_interpretation, request_expression->expression->interpretation);
        request_expression->recursive_without_any_chosen = TRUE;
      }
    }
  }

  g_queue_clear(chosen_recursive_interpretation);

  if (ctrl_nlp_th->loginfo->trace & DOgNlpTraceMatch)
  {
    char buffer[DPcPathSize];
    snprintf(buffer, DPcPathSize, "NlpMatchValidateListWithoutAny: list of all request expressions at level %d after:",
        ctrl_nlp_th->level);
    IFE(NlpRequestExpressionsLog(ctrl_nlp_th, 0, buffer));
  }


  DONE;
}

static og_status NlpMatchRegexs(og_nlp_th ctrl_nlp_th)
{

  // TODO SMA Fonction NlpRegexAddWords
  // boucle sur les regex, et pour chaque regex
  //   - travailler sur la string complete de la phrase -> start et end du match 0
  //   - construit un word de type regex (comme type number) basé sur NlpParseAddWord
  /* Dans une fonction, parcourir la liste des package pour aller cherchez les regex
  NlpRegexAddWords(ctrl_nlp_th)
  {
  int interpret_package_used = OgHeapGetCellsUsed(ctrl_nlp_th->hinterpret_package);
  struct interpret_package *interpret_packages = OgHeapGetCell(ctrl_nlp_th->hinterpret_package, 0);
  IFN(interpret_packages) DPcErr;status
  for (int i = 0; i < interpret_package_used; i++)
  {
    struct interpret_package *interpret_package = interpret_packages + i;
    og_status status = NlpRegexAddWordsPackage(ctrl_nlp_th, Irequest_word, interpret_package);
    IFE(status);
  }
  }
  et dans  NlpRegexAddWordsPackage
  package_t package = interpret_package->package;
  */


  int interpret_package_used = OgHeapGetCellsUsed(ctrl_nlp_th->hinterpret_package);
  struct interpret_package *interpret_packages = OgHeapGetCell(ctrl_nlp_th->hinterpret_package, 0);
  IFN(interpret_packages) DPcErr;
  for (int i = 0; i < interpret_package_used; i++)
  {
    struct interpret_package *interpret_package = interpret_packages + i;
    og_bool status = NlpRegexAddWordsPackage(ctrl_nlp_th, interpret_package);
    IFE(status);
  }


  DONE;
}

static og_status NlpRegexAddWordsPackage(og_nlp_th ctrl_nlp_th, struct interpret_package *interpret_package)
{
  og_string sentence = ctrl_nlp_th->request_sentence;

  og_bool found = FALSE;
  og_string *matchedSentence = NULL;
  int start_match_position = 0;
  int end_match_position = 0;

  int regexNumber = OgHeapGetCellsUsed(interpret_package->package->hregex);
  for(int i=0; i<regexNumber; i++)
  {
    struct regex *regex = OgHeapGetCell(interpret_package->package->hregex, i);
    if(regex->regex)
    {
      // match the regular expression
      GMatchInfo *match_info = NULL;
      GError *regexp_error = NULL;
      og_bool match = g_regex_match_all_full(regex->regex, sentence, -1, 0, 0, &match_info, &regexp_error);
      if (regexp_error)
      {
        NlpThrowErrorTh(ctrl_nlp_th, "NlpRegexAddWordsPackage: g_regex_match_all_full failed on execution : %s",
            regexp_error->message);
        g_error_free(regexp_error);

        if (match_info != NULL)
        {
          g_match_info_free(match_info);
        }

        DPcErr;
      }

      if (match)
      {
        matchedSentence = (og_string *)g_match_info_get_string(match_info);
        g_match_info_fetch_pos (match_info, 0, &start_match_position, &end_match_position);

        g_match_info_free(match_info);
        match_info = NULL;
        found = TRUE;
      }

      // ensure match_info freeing
      if (match_info != NULL)
      {
        g_match_info_free(match_info);
      }

    }
  }

  if(found)
  {
    // ajouter à la réponse
  }

  DONE;
}
