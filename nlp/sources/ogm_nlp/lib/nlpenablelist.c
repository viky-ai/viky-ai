/*
 *  Selecting a list of interpretations
 *  Copyright (c) 2010 Pertimm, by Patrick Constant
 *  Dev : July 2019
 *  Version 1.0
 */
#include <loggen.h>

#include "ogm_nlp.h"

static og_status NlpEnableListInterpretation(og_nlp_th ctrl_nlp_th, GQueue *sorted_request_expressions);

og_status NlpEnableListInit(og_nlp_th ctrl_nlp_th)
{
  ctrl_nlp_th->interpretation_hash = g_hash_table_new(g_direct_hash, g_direct_equal);
  DONE;
}

og_status NlpEnableListFlush(og_nlp_th ctrl_nlp_th)
{
  g_hash_table_destroy(ctrl_nlp_th->interpretation_hash);
  DONE;
}

og_status NlpEnableListReset(og_nlp_th ctrl_nlp_th)
{
  ctrl_nlp_th->enable_list = FALSE;
  g_hash_table_remove_all(ctrl_nlp_th->interpretation_hash);
  DONE;
}

og_status NlpEnableList(og_nlp_th ctrl_nlp_th, GQueue *sorted_request_expressions)
{
  if (!ctrl_nlp_th->enable_list) DONE;

  // TODO: make a hashtable of selected interpretations
  // then for each of those interpretations get the one that do not overlap
  // with already selected ones
  // Getting expression that will be sent
  struct interpretation *interpretation = NULL;
  for (GList *iter = sorted_request_expressions->head; iter; iter = iter->next)
  {
    struct request_expression *request_expression = iter->data;
    if (request_expression->keep_as_result)
    {
      interpretation = request_expression->expression->interpretation;
      g_hash_table_insert(ctrl_nlp_th->interpretation_hash, GINT_TO_POINTER(interpretation), GINT_TO_POINTER(1));
    }
    else if (interpretation->scope == nlp_interpretation_scope_type_public)
    {
      if (interpretation->package == ctrl_nlp_th->primary_package)
      {
        interpretation = request_expression->expression->interpretation;
        g_hash_table_insert(ctrl_nlp_th->interpretation_hash, GINT_TO_POINTER(interpretation), GINT_TO_POINTER(1));
      }
    }

    IFE(NlpSolutionCalculatePositions(ctrl_nlp_th, request_expression));
  }

  IFE(NlpEnableListInterpretation(ctrl_nlp_th, sorted_request_expressions));

  if (ctrl_nlp_th->loginfo->trace & DOgNlpTraceMatch)
  {
    IFE(NlpSortedRequestExpressionsLog(ctrl_nlp_th, "List of sorted request expressions after enable list:"));
  }
  DONE;
}

static og_status NlpEnableListInterpretation(og_nlp_th ctrl_nlp_th, GQueue *sorted_request_expressions)
{
  for (GList *iter1 = sorted_request_expressions->head; iter1; iter1 = iter1->next)
  {
    struct request_expression *request_expression = iter1->data;
    request_expression->overlapped = FALSE;
  }

  // Removing overlapped expressions whatever their interpretation is
  // Since the expressions are sorted by pertinence, the removed expressions are the one less pertinent
  // and we keep only those interpretations that have been selected
  for (GList *iter1 = sorted_request_expressions->head; iter1; iter1 = iter1->next)
  {
    struct request_expression *request_expression1 = iter1->data;
    {
      if (request_expression1->overlapped) continue;
      gpointer exists = g_hash_table_lookup(ctrl_nlp_th->interpretation_hash,
          request_expression1->expression->interpretation);
      IFN(exists) continue;
      request_expression1->keep_as_result = TRUE;
      for (GList *iter2 = iter1->next; iter2; iter2 = iter2->next)
      {
        struct request_expression *request_expression2 = iter2->data;
        {
          og_bool local_overlapped = TRUE;
          // Check if request_expression2 is overlapped with request_expression1
          if (request_expression1->start_position_char <= request_expression2->start_position_char
              && request_expression1->end_position_char <= request_expression2->start_position_char) local_overlapped =
          FALSE;
          else if (request_expression2->start_position_char <= request_expression1->start_position_char
              && request_expression2->end_position_char <= request_expression1->start_position_char) local_overlapped =
          FALSE;
          if (local_overlapped)
          {
            request_expression2->keep_as_result = FALSE;
            request_expression2->overlapped = TRUE;
            if (ctrl_nlp_th->loginfo->trace & DOgNlpTraceMatch)
            {
              OgMsg(ctrl_nlp_th->hmsg, "", DOgMsgDestInLog, "expression overlapped");
              IFE(NlpRequestExpressionLog(ctrl_nlp_th, request_expression2, 2));
            }
          }
        }
      }
      if (ctrl_nlp_th->loginfo->trace & DOgNlpTraceMatch)
      {
        OgMsg(ctrl_nlp_th->hmsg, "", DOgMsgDestInLog, "expression overlapped=%d keep_as_result=%d",
            request_expression1->overlapped, request_expression1->keep_as_result);
        IFE(NlpRequestExpressionLog(ctrl_nlp_th, request_expression1, 2));
      }
    }
  }
  DONE;
}

