/*
 *  Handling choice of request expressions
 *  Copyright (c) 2017 Pertimm, by Patrick Constant
 *  Dev : October 2017
 *  Version 1.0
 */
#include "ogm_nlp.h"

static og_status NlpRequestExpressionsOptimizeAny(og_nlp_th ctrl_nlp_th);
static og_status NlpRequestExpressionOptimizeAny(og_nlp_th ctrl_nlp_th, struct request_expression *request_expressions,
    int Irequest_expression, int request_expression_used);
static og_status NlpRequestExpressionsOptimizeCompact(og_nlp_th ctrl_nlp_th);
static og_status NlpRequestExpressionOptimizeCompact(og_nlp_th ctrl_nlp_th,
    struct request_expression *request_expressions, int Irequest_expression, int request_expression_used);

og_status NlpRequestExpressionsOptimize(og_nlp_th ctrl_nlp_th)
{
  // Ancune de ces heuristiques n'est convaincante, mais on se garde la possibilité de les utiliser
  // dans tous les cas, ces fonctions ne font que des logs.
  if (ctrl_nlp_th->loginfo->trace & DOgNlpTraceMatch)
  {
    char buffer[DPcPathSize];
    snprintf(buffer, DPcPathSize, "List of all request expression at level %d:", ctrl_nlp_th->level);
    IFE(NlpRequestExpressionsLog(ctrl_nlp_th, 0, buffer));
  }
  DONE;
  IFE(NlpRequestExpressionsOptimizeAny(ctrl_nlp_th));
  IFE(NlpRequestExpressionsOptimizeCompact(ctrl_nlp_th));
  DONE;
}

/*
 * When there is several expressions found within the same interpretation,
 * some with any, other without any, we remove all expressions with any
 */
static og_status NlpRequestExpressionsOptimizeAny(og_nlp_th ctrl_nlp_th)
{
  int request_expression_used = OgHeapGetCellsUsed(ctrl_nlp_th->hrequest_expression);
  struct request_expression *request_expressions = OgHeapGetCell(ctrl_nlp_th->hrequest_expression, 0);
  IFn(request_expressions) DPcErr;

  for (int i = 0; i < request_expression_used; i++)
  {
    struct request_expression *request_expression = request_expressions + i;
    request_expression->analyzed = 0;
  }

  for (int i = 0; i < request_expression_used; i++)
  {
    struct request_expression *request_expression = request_expressions + i;
    if (request_expression->analyzed) continue;
    IFE(NlpRequestExpressionOptimizeAny(ctrl_nlp_th, request_expressions, i, request_expression_used));
  }

  DONE;
}

static og_status NlpRequestExpressionOptimizeAny(og_nlp_th ctrl_nlp_th, struct request_expression *request_expressions,
    int Irequest_expression, int request_expression_used)
{
  struct request_expression *first_request_expression = request_expressions + Irequest_expression;

  GQueue queue_request_expression[1];
  g_queue_init(queue_request_expression);
  g_queue_push_tail(queue_request_expression, first_request_expression);

  for (int i = Irequest_expression + 1; i < request_expression_used; i++)
  {
    struct request_expression *request_expression = request_expressions + i;
    if (request_expression->expression->interpretation != first_request_expression->expression->interpretation) continue;
    g_queue_push_tail(queue_request_expression, request_expression);
  }

  int nb_re = g_queue_get_length(queue_request_expression);
  if (nb_re <= 1) DONE;

  // Now we have several request_expression with the same interpretation
  // we find the ones with and without any
  int nb_re_any = 0;
  for (GList *iter = queue_request_expression->head; iter; iter = iter->next)
  {
    struct request_expression *request_expression = iter->data;
    if (request_expression->expression->alias_any_input_part_position >= 0)
    {
      nb_re_any++;
    }
  }
  if (nb_re_any > 0 && nb_re_any < nb_re)
  {
    OgMsg(ctrl_nlp_th->hmsg, "", DOgMsgDestInLog, "Any expression to be removed:");
    for (GList *iter = queue_request_expression->head; iter; iter = iter->next)
    {
      struct request_expression *request_expression = iter->data;
      if (request_expression->expression->alias_any_input_part_position >= 0)
      {
        IFE(NlpRequestExpressionLog(ctrl_nlp_th, request_expression, 2));
      }
    }
    OgMsg(ctrl_nlp_th->hmsg, "", DOgMsgDestInLog, "  Expression(s) to keep:");
    for (GList *iter = queue_request_expression->head; iter; iter = iter->next)
    {
      struct request_expression *request_expression = iter->data;
      if (request_expression->expression->alias_any_input_part_position < 0)
      {
        IFE(NlpRequestExpressionLog(ctrl_nlp_th, request_expression, 4));
      }
    }

  }
  for (GList *iter = queue_request_expression->head; iter; iter = iter->next)
  {
    struct request_expression *request_expression = iter->data;
    request_expression->analyzed = 1;
  }

  g_queue_clear(queue_request_expression);
  DONE;
}

/*
 * When there is several expressions found within the same interpretation,
 * some are more compact than others, we remove the less compact ones
 */
static og_status NlpRequestExpressionsOptimizeCompact(og_nlp_th ctrl_nlp_th)
{
  int request_expression_used = OgHeapGetCellsUsed(ctrl_nlp_th->hrequest_expression);
  struct request_expression *request_expressions = OgHeapGetCell(ctrl_nlp_th->hrequest_expression, 0);
  IFn(request_expressions) DPcErr;

  for (int i = 0; i < request_expression_used; i++)
  {
    struct request_expression *request_expression = request_expressions + i;
    request_expression->analyzed = 0;
  }

  for (int i = 0; i < request_expression_used; i++)
  {
    struct request_expression *request_expression = request_expressions + i;
    if (request_expression->analyzed) continue;
    IFE(NlpRequestExpressionOptimizeCompact(ctrl_nlp_th, request_expressions, i, request_expression_used));
  }

  DONE;
}

static og_status NlpRequestExpressionOptimizeCompact(og_nlp_th ctrl_nlp_th,
    struct request_expression *request_expressions, int Irequest_expression, int request_expression_used)
{
  struct request_expression *first_request_expression = request_expressions + Irequest_expression;

  GQueue queue_request_expression[1];
  g_queue_init(queue_request_expression);
  g_queue_push_tail(queue_request_expression, first_request_expression);

  for (int i = Irequest_expression + 1; i < request_expression_used; i++)
  {
    struct request_expression *request_expression = request_expressions + i;
    if (request_expression->expression->interpretation != first_request_expression->expression->interpretation) continue;
    g_queue_push_tail(queue_request_expression, request_expression);
  }

  int nb_re = g_queue_get_length(queue_request_expression);
  if (nb_re <= 1) DONE;

  // If all expression have only one position, we stop
  int nb_re_with_several_positions = 0;
  for (GList *iter = queue_request_expression->head; iter; iter = iter->next)
  {
    struct request_expression *request_expression = iter->data;
    if (request_expression->request_positions_nb > 1) nb_re_with_several_positions++;
  }

  if (nb_re_with_several_positions <= 1) DONE;

  OgMsg(ctrl_nlp_th->hmsg, "", DOgMsgDestInLog, "Expression(s) to study for compactness:");
  for (GList *iter = queue_request_expression->head; iter; iter = iter->next)
  {
    struct request_expression *request_expression = iter->data;
    IFE(NlpRequestExpressionLog(ctrl_nlp_th, request_expression, 2));
  }

  for (GList *iter = queue_request_expression->head; iter; iter = iter->next)
  {
    struct request_expression *request_expression = iter->data;
    request_expression->analyzed = 1;
  }

  g_queue_clear(queue_request_expression);
  DONE;
}

