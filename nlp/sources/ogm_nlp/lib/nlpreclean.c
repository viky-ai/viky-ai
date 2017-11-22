/*
 *  Handling choice of request expressions
 *  Copyright (c) 2017 Pertimm, by Patrick Constant
 *  Dev : October 2017
 *  Version 1.0
 */
#include "ogm_nlp.h"

static int NlpRequestExpressionsCleanCmp(gconstpointer ptr_request_expression1, gconstpointer ptr_request_expression2,
    gpointer user_data);
#if 0
static int NlpRequestExpressionCleanOld(og_nlp_th ctrl_nlp_th, struct request_expression *request_expression_old);
static og_bool NlpRequestExpressionUsesOld(og_nlp_th ctrl_nlp_th, struct request_expression *request_expression,
    struct request_expression *request_expression_old);
#endif

og_status NlpRequestExpressionsClean(og_nlp_th ctrl_nlp_th)
{

  int request_expression_used = OgHeapGetCellsUsed(ctrl_nlp_th->hrequest_expression);
  struct request_expression *request_expressions = OgHeapGetCell(ctrl_nlp_th->hrequest_expression, 0);
  IFn(request_expressions) DPcErr;

  int nb_deleted_request_expressions = 0;
  for (int i = 0; i < request_expression_used; i++)
  {
    struct request_expression *request_expression = request_expressions + i;
    if (request_expression->deleted == 0) continue;
    if (i < ctrl_nlp_th->new_request_expression_start)
    {
      // Its seems we do not need to clean old expressions,
      // and the code to do it is bugged in NlpRequestExpressionCleanOld
      //IFE(NlpRequestExpressionCleanOld(ctrl_nlp_th, request_expression));
      //NlpThrowErrorTh(ctrl_nlp_th, "NlpRequestExpressionsClean: cleaning of an old expression %d not done", i);
      //DPcErr;
      request_expression->deleted = 0;
    }
    nb_deleted_request_expressions++;
  }

  if (nb_deleted_request_expressions >= 1 && request_expression_used > 1)
  {
    g_qsort_with_data(request_expressions, request_expression_used, sizeof(struct request_expression),
        NlpRequestExpressionsCleanCmp, NULL);

    int new_request_expression_used = request_expression_used;
    for (int i = request_expression_used - 1; i >= 0; i--)
    {
      struct request_expression *request_expression = request_expressions + i;
      if (request_expression->deleted) new_request_expression_used = i;
      else break;
    }

    IFE(OgHeapSetCellsUsed(ctrl_nlp_th->hrequest_expression, new_request_expression_used));
    request_expression_used = OgHeapGetCellsUsed(ctrl_nlp_th->hrequest_expression);

    for (int i = 0; i < request_expression_used; i++)
    {
      struct request_expression *request_expression = request_expressions + i;
      request_expression->self_index = i;
    }

  }

  if (ctrl_nlp_th->loginfo->trace & DOgNlpTraceMatch)
  {
    char buffer[DPcPathSize];
    snprintf(buffer, DPcPathSize, "List of all request expression after cleaning at level %d:", ctrl_nlp_th->level);
    IFE(NlpRequestExpressionsLog(ctrl_nlp_th, 0, buffer));
  }
  DONE;
}

static int NlpRequestExpressionsCleanCmp(gconstpointer ptr_request_expression1, gconstpointer ptr_request_expression2,
    gpointer user_data)
{
  struct request_expression *request_expression1 = (struct request_expression *) ptr_request_expression1;
  struct request_expression *request_expression2 = (struct request_expression *) ptr_request_expression2;

  if (request_expression1->deleted != request_expression2->deleted)
  {
    return (request_expression1->deleted - request_expression2->deleted);
  }
  // It is very important to avoid changing the order of the request expressions
  return request_expression1->self_index - request_expression2->self_index;
}

#if 0
static int NlpRequestExpressionCleanOld(og_nlp_th ctrl_nlp_th, struct request_expression *request_expression_old)
{
  struct request_expression *request_expressions = OgHeapGetCell(ctrl_nlp_th->hrequest_expression, 0);
  IFn(request_expressions) DPcErr;

  for (int i = 0; i < ctrl_nlp_th->new_request_expression_start; i++)
  {
    struct request_expression *request_expression = request_expressions + i;
    if (request_expression->deleted) continue;
    og_bool uses_old = NlpRequestExpressionUsesOld(ctrl_nlp_th, request_expression, request_expression_old);
    IFE(uses_old);
    if (uses_old)
    {
      request_expression->deleted = 1;
      IFE(NlpRequestExpressionCleanOld(ctrl_nlp_th, request_expression));
    }
  }
  DONE;
}

static og_bool NlpRequestExpressionUsesOld(og_nlp_th ctrl_nlp_th, struct request_expression *request_expression,
    struct request_expression *request_expression_old)
{
  for (int i = 0; i < request_expression->orips_nb; i++)
  {
    struct request_input_part *request_input_part = NlpGetRequestInputPart(ctrl_nlp_th, request_expression, i);
    IFN(request_input_part) DPcErr;

    if (request_input_part->type == nlp_input_part_type_Interpretation)
    {
      struct request_expression *sub_request_expression = OgHeapGetCell(ctrl_nlp_th->hrequest_expression,
          request_input_part->Irequest_expression);
      IFN(sub_request_expression) DPcErr;
      if (sub_request_expression->self_index == request_expression_old->self_index) return TRUE;
    }
  }
  return FALSE;
}
#endif
