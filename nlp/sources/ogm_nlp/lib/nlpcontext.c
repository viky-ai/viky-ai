/*
 *  Handling request context
 *  Copyright (c) 2017 Pertimm, by Patrick Constant
 *  Dev : November 2017
 *  Version 1.0
 */
#include "ogm_nlp.h"

/*
 * request_expression is matched if all contexts from the request are included in the context of the expression
 */
og_status NlpContextIsValid(og_nlp_th ctrl_nlp_th, struct request_expression *request_expression)
{
  int request_context_used = OgHeapGetCellsUsed(ctrl_nlp_th->hrequest_context);
  IFn(request_context_used) return TRUE;
  struct request_context *request_contexts = OgHeapGetCell(ctrl_nlp_th->hrequest_context, 0);
  IFN(request_contexts) DPcErr;

  struct interpretation *interpretation = request_expression->expression->interpretation;
  IFn(interpretation->contexts_nb) return FALSE;

  for (int i = 0; i < request_context_used; i++)
  {
    struct request_context *request_context = request_contexts + i;
    og_string flag = OgHeapGetCell(ctrl_nlp_th->hba, request_context->flag_start);
    IFN(flag) DPcErr;
    og_bool matched_context = FALSE;
    for (int j = 0; j < interpretation->contexts_nb; j++)
    {
      if (!strcmp(flag, interpretation->contexts[j].flag))
      {
        matched_context = TRUE;
      }
    }
    if (!matched_context) return FALSE;
  }
  return TRUE;
}

og_status NlpContextGetScore(og_nlp_th ctrl_nlp_th, struct request_expression *request_expression)
{
  request_expression->score->context = 1.0;
  int request_context_used = OgHeapGetCellsUsed(ctrl_nlp_th->hrequest_context);

  struct interpretation *interpretation = request_expression->expression->interpretation;
  IFn(request_context_used)
  {
    IFx(interpretation->contexts_nb)
    {
      double cost = 0.1;
      double total_cost = interpretation->contexts_nb * cost;
      if (total_cost > 1.0) total_cost = 1.0;
      request_expression->score->context -= total_cost;
    }
    DONE;
  }

  IFn(interpretation->contexts_nb)
  {
    double cost = 0.1;
    double total_cost = request_context_used * cost;
    if (total_cost > 1.0) total_cost = 1.0;
    request_expression->score->context -= total_cost;
    DONE;
  }

  struct request_context *request_contexts = OgHeapGetCell(ctrl_nlp_th->hrequest_context, 0);

  int matched_contexts_nb = 0;
  for (int i = 0; i < request_context_used; i++)
  {
    struct request_context *request_context = request_contexts + i;
    og_string flag = OgHeapGetCell(ctrl_nlp_th->hba, request_context->flag_start);
    IFN(flag) DPcErr;
    for (int j = 0; j < interpretation->contexts_nb; j++)
    {
      if (!strcmp(flag, interpretation->contexts[j].flag))
      {
        matched_contexts_nb++;
      }
    }
  }
  double cost = 0.1;
  double total_cost = 0.0;
  total_cost += (request_context_used - matched_contexts_nb) * cost;
  total_cost += (interpretation->contexts_nb - matched_contexts_nb) * cost;
  if (total_cost > 1.0) total_cost = 1.0;
  request_expression->score->context -= total_cost;
  DONE;
}

