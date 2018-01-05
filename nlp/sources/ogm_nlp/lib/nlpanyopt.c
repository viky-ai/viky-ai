/*
 *  Handling log messages for Natural Language Processing library
 *  Copyright (c) 2017 Pertimm, by Patrick Constant
 *  Dev : September 2017
 *  Version 1.0
 */
#include "ogm_nlp.h"

static int NlpRequestAnyOptimizeMatch1(og_nlp_th ctrl_nlp_th, struct request_expression *root_request_expression,
    og_bool match_single, og_bool must_attach);
static int NlpRequestAnyOptimizeMatch2(og_nlp_th ctrl_nlp_th, struct request_expression *root_request_expression,
    og_bool match_single, og_bool must_attach);
static og_bool NlpRequestAnyOptimizeMatch3(og_nlp_th ctrl_nlp_th, struct request_expression *root_request_expression,
    int Irequest_any, og_bool match_single, og_bool must_attach);

int NlpRequestAnyOptimizeMatch(og_nlp_th ctrl_nlp_th, struct request_expression *root_request_expression,
    og_bool must_attach)
{
  int total_request_expressions_matched = 0;
  int nb_request_expressions_matched;

  for (int i = 0; i < root_request_expression->request_anys_nb; i++)
  {
    struct request_any *request_any = OgHeapGetCell(ctrl_nlp_th->hrequest_any,
        root_request_expression->request_any_start + i);
    request_any->consumed = 0;
  }

  /** We attach 'any' that match only one expression **/
  nb_request_expressions_matched = NlpRequestAnyOptimizeMatch1(ctrl_nlp_th, root_request_expression, TRUE, must_attach);
  IFE(nb_request_expressions_matched);
  total_request_expressions_matched += nb_request_expressions_matched;

  /** Then we randomly attach 'any' that match several expression: we choose the first matching expression **/
  nb_request_expressions_matched = NlpRequestAnyOptimizeMatch1(ctrl_nlp_th, root_request_expression, FALSE,
      must_attach);
  IFE(nb_request_expressions_matched);
  total_request_expressions_matched += nb_request_expressions_matched;

  if (!must_attach)
  {
    for (int i = 0; i < root_request_expression->request_anys_nb; i++)
    {
      struct request_any *request_any = OgHeapGetCell(ctrl_nlp_th->hrequest_any,
          root_request_expression->request_any_start + i);
      request_any->consumed = 0;
      for (GList *iter = request_any->queue_request_expression->head; iter; iter = iter->next)
      {
        int Irequest_expression = GPOINTER_TO_INT(iter->data);
        struct request_expression *request_expression = OgHeapGetCell(ctrl_nlp_th->hrequest_expression,
            Irequest_expression);
        IFN(request_expression) DPcErr;
        request_expression->Irequest_any = -1;
      }
    }
  }

  return total_request_expressions_matched;
}

static int NlpRequestAnyOptimizeMatch1(og_nlp_th ctrl_nlp_th, struct request_expression *root_request_expression,
    og_bool match_single, og_bool must_attach)
{
  int total_request_expressions_matched = 0;
  int nb_request_expressions_matched;
  do
  {
    nb_request_expressions_matched = NlpRequestAnyOptimizeMatch2(ctrl_nlp_th, root_request_expression, match_single,
        must_attach);
    IFE(nb_request_expressions_matched);
    total_request_expressions_matched += nb_request_expressions_matched;
  }
  while (nb_request_expressions_matched);
  return total_request_expressions_matched;
}

static int NlpRequestAnyOptimizeMatch2(og_nlp_th ctrl_nlp_th, struct request_expression *root_request_expression,
    og_bool match_single, og_bool must_attach)
{
  int nb_request_expressions_matched = 0;
  for (int i = 0; i < root_request_expression->request_anys_nb; i++)
  {
    og_bool one_request_expression_matched = NlpRequestAnyOptimizeMatch3(ctrl_nlp_th, root_request_expression,
        root_request_expression->request_any_start + i, match_single, must_attach);
    IFE(one_request_expression_matched);
    if (one_request_expression_matched) nb_request_expressions_matched++;
  }

  return nb_request_expressions_matched;
}
static og_bool NlpRequestAnyOptimizeMatch3(og_nlp_th ctrl_nlp_th, struct request_expression *root_request_expression,
    int Irequest_any, og_bool match_single, og_bool must_attach)
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
    if (!match_single) break;
  }
  {
    if (match_single)
    {
      if (nb_possible_request_expressions_to_match == 1)
      {
        single_request_expression->Irequest_any = Irequest_any;
        request_any->consumed = 1;
        return TRUE;
      }
    }
    else
    {
      if (nb_possible_request_expressions_to_match >= 1)
      {
        single_request_expression->Irequest_any = Irequest_any;
        request_any->consumed = 1;
        return TRUE;
      }
    }
  }
  return FALSE;
}

