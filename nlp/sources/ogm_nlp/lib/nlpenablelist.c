/*
 *  Glue means whether a position is glued to another
 *  Copyright (c) 2017 Pertimm, by Patrick Constant
 *  Dev : October 2017
 *  Version 1.0
 */
#include "ogm_nlp.h"

static og_status NlpEnableListInterpretation(og_nlp_th ctrl_nlp_th, GQueue *sorted_request_expressions,
    struct interpretation *interpretation);

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
      g_hash_table_insert(ctrl_nlp_th->glue_hash, GINT_TO_POINTER(interpretation), GINT_TO_POINTER(1));
    }
    IFE(NlpSolutionCalculatePositions(ctrl_nlp_th, request_expression));
  }

  GList *key_list = g_hash_table_get_keys(ctrl_nlp_th->glue_hash);
  for (GList *iter = key_list; iter; iter = iter->next)
  {
    interpretation = iter->data;
    IFE(NlpEnableListInterpretation(ctrl_nlp_th, sorted_request_expressions, interpretation));
  }
  g_list_free(key_list);

  if (ctrl_nlp_th->loginfo->trace & DOgNlpTraceMatch)
  {
    IFE(NlpSortedRequestExpressionsLog(ctrl_nlp_th, "List of sorted request expressions after enable list:"));
  }
  DONE;
}

static og_status NlpEnableListInterpretation(og_nlp_th ctrl_nlp_th, GQueue *sorted_request_expressions,
    struct interpretation *interpretation)
{
  for (GList *iter1 = sorted_request_expressions->head; iter1; iter1 = iter1->next)
  {
    struct request_expression *request_expression1 = iter1->data;
    if (interpretation == request_expression1->expression->interpretation)
    {
      for (GList *iter2 = sorted_request_expressions->head; iter2; iter2 = iter2->next)
      {
        struct request_expression *request_expression2 = iter2->data;
        if (interpretation == request_expression2->expression->interpretation)
        {
          if (request_expression2 == request_expression1)
          {
            request_expression2->keep_as_result = TRUE;
            break;
          }
          else
          {
            // Check if request_expression2 is overlapped with request_expression1
            og_bool overlapped = TRUE;
            if (request_expression1->start_position_char <= request_expression2->start_position_char
                && request_expression1->end_position_char <= request_expression2->start_position_char) overlapped =
                FALSE;
            else if (request_expression2->start_position_char <= request_expression1->start_position_char
                && request_expression2->end_position_char <= request_expression1->start_position_char) overlapped =
                FALSE;
          if (overlapped) break;
          }
        }
      }
    }
  }
  DONE;
}

