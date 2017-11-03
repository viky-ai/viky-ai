/*
 *  Handling log messages for Natural Language Processing library
 *  Copyright (c) 2017 Pertimm, by Patrick Constant
 *  Dev : September 2017
 *  Version 1.0
 */
#include "ogm_nlp.h"

static int NlpRequestAnyOptimizeMatchSingle(og_nlp_th ctrl_nlp_th, struct request_expression *root_request_expression);
static og_bool NlpRequestAnyOptimizeMatchSingle1(og_nlp_th ctrl_nlp_th,
    struct request_expression *root_request_expression, int Irequest_any);

/*
 * One case is not implemented : the case when several 'any' are possible for all request_expression
 * if this is the case we would need to choose randomly a request_expression for a given 'any'
 */
int NlpRequestAnyOptimizeMatch(og_nlp_th ctrl_nlp_th, struct request_expression *root_request_expression)
{
  og_bool some_request_expression_matched;
  do
  {
    some_request_expression_matched = NlpRequestAnyOptimizeMatchSingle(ctrl_nlp_th, root_request_expression);
    IFE(some_request_expression_matched);
  }
  while (some_request_expression_matched);
  DONE;
}

static int NlpRequestAnyOptimizeMatchSingle(og_nlp_th ctrl_nlp_th, struct request_expression *root_request_expression)
{
  og_bool some_request_expression_matched = FALSE;
  for (int i = 0; i < root_request_expression->request_anys_nb; i++)
  {
    og_bool at_least_one_request_expression_matched = NlpRequestAnyOptimizeMatchSingle1(ctrl_nlp_th,
        root_request_expression, root_request_expression->request_any_start + i);
    IFE(at_least_one_request_expression_matched);
    if (at_least_one_request_expression_matched) some_request_expression_matched = TRUE;
  }

  return some_request_expression_matched;
}
static og_bool NlpRequestAnyOptimizeMatchSingle1(og_nlp_th ctrl_nlp_th,
    struct request_expression *root_request_expression, int Irequest_any)
{
  struct request_any *request_any = OgHeapGetCell(ctrl_nlp_th->hrequest_any, Irequest_any);
  if (request_any->consumed) return FALSE;

  struct request_expression *single_request_expression = NULL;

  int nb_possible_request_expressions_to_match = 0;
  for (GList *iter = request_any->queue_request_expression->head; iter; iter = iter->next)
  {
    int Irequest_expression = GPOINTER_TO_INT(iter->data);
    struct request_expression *request_expression = OgHeapGetCell(ctrl_nlp_th->hrequest_expression,
        Irequest_expression);
    IFN(request_expression) DPcErr;
    if (request_expression->Irequest_any >= 0) continue;
    nb_possible_request_expressions_to_match++;
    single_request_expression = request_expression;
  }
  if (nb_possible_request_expressions_to_match == 1)
  {
    single_request_expression->Irequest_any = Irequest_any;
    request_any->consumed = 1;
    return TRUE;
  }
  return FALSE;
}

