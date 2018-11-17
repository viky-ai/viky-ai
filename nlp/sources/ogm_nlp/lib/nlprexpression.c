/*
 *  Handling request input parts
 *  Copyright (c) 2017 Pertimm, by Patrick Constant
 *  Dev : October 2017
 *  Version 1.0
 */
#include "ogm_nlp.h"

#define DOgNlpMaxNbAnys 2

static og_bool NlpRequestExpressionExists(og_nlp_th ctrl_nlp_th, struct request_expression_access_cache *cache,
    struct request_expression *request_expression, struct request_expression **psame_request_expression);
static og_bool NlpRequestExpressionIsIncludedInNoanyRequestExpression(og_nlp_th ctrl_nlp_th,
    struct request_expression *request_expression);
static og_bool NlpRequestExpressionSame(og_nlp_th ctrl_nlp_th, struct request_expression_access_cache *cache,
    struct request_expression *request_expression1, struct request_expression *request_expression2);
static og_bool NlpRequestExpressionIsGlued(og_nlp_th ctrl_nlp_th, struct request_expression *request_expression);
static og_status NlpRequestExpressionOverlapMark(og_nlp_th ctrl_nlp_th, struct request_expression *request_expression);
static og_status NlpRequestExpressionInputPartsOverlapMark(og_nlp_th ctrl_nlp_th,
    struct request_input_part *request_input_part1, struct request_input_part *request_input_part2, int *poverlap_mark);
static og_status NlpRequestExpressionGetSparseMark(og_nlp_th ctrl_nlp_th, struct request_expression *request_expression);
static og_status NlpRequestInterpretationBuild(og_nlp_th ctrl_nlp_th, struct request_expression *request_expression,
    json_t *json_interpretations);

og_bool NlpRequestExpressionAdd(og_nlp_th ctrl_nlp_th, struct expression *expression,
    struct match_zone_input_part *match_zone_input_part, struct request_expression **prequest_expression)
{
  int request_expression_used = OgHeapGetCellsUsed(ctrl_nlp_th->hrequest_expression);
  int request_position_used = OgHeapGetCellsUsed(ctrl_nlp_th->hrequest_position);
  int orip_used = OgHeapGetCellsUsed(ctrl_nlp_th->horip);

  size_t Irequest_expression;
  struct request_expression *request_expression = OgHeapNewCell(ctrl_nlp_th->hrequest_expression, &Irequest_expression);
  IFn(request_expression) DPcErr;
  IF(Irequest_expression) DPcErr;
  request_expression->self_index = Irequest_expression;
  request_expression->expression = expression;
  request_expression->level = ctrl_nlp_th->level;
  request_expression->Isuper_request_expression = (-1);
  request_expression->Irequest_any = (-1);
  request_expression->auto_complete_request_word = NULL;
  request_expression->keep_as_result = FALSE;
  g_queue_init(request_expression->tmp_solutions);
  g_queue_init(request_expression->sorted_flat_list);
  request_expression->overlap_mark = 0;
  memset(request_expression->score, 0, sizeof(struct request_score));
  request_expression->score->scope = 1;
  request_expression->total_score = 0.0;
  request_expression->safe_request_position_start = (-1);
  request_expression->safe_request_positions_nb = 0;
  request_expression->recursive_without_any_chosen = FALSE;
  request_expression->nb_matched_words = 0;

  request_expression->request_position_start = OgHeapGetCellsUsed(ctrl_nlp_th->hrequest_position);
  IF(request_expression->request_position_start) DPcErr;

  int nb_request_positions = 0;
  struct request_input_part *request_input_parts = OgHeapGetCell(ctrl_nlp_th->hrequest_input_part, 0);
  IFN(request_input_parts) DPcErr;
  for (int i = 0; i < expression->input_parts_nb; i++)
  {
    struct request_input_part *request_input_part = request_input_parts + match_zone_input_part[i].current;
    nb_request_positions += request_input_part->request_positions_nb;
  }
  // pre-allocation to avoid reallocation in the
  int request_position_start = OgHeapAddCells(ctrl_nlp_th->hrequest_position, nb_request_positions);
  IFE(request_position_start);
  if (request_position_start != request_expression->request_position_start)
  {
    NlpThrowErrorTh(ctrl_nlp_th,
        "NlpRequestExpressionAdd: request_position_start (%d) != request_expression->request_position_start (%d)",
        request_position_start, request_expression->request_position_start);
    DPcErr;
  }
  struct request_position *request_positions = OgHeapGetCell(ctrl_nlp_th->hrequest_position, 0);
  IFN(request_positions) DPcErr;

  int request_position_current = request_position_start;
  for (int i = 0; i < expression->input_parts_nb; i++)
  {
    struct request_input_part *request_input_part = request_input_parts + match_zone_input_part[i].current;
    struct request_position *request_position_from = request_positions + request_input_part->request_position_start;
    struct request_position *request_position_to = request_positions + request_position_current;
    memcpy(request_position_to, request_position_from,
        sizeof(struct request_position) * request_input_part->request_positions_nb);
    request_position_current += request_input_part->request_positions_nb;
  }
  request_expression->request_positions_nb += request_position_current - request_position_start;

  IF(NlpRequestPositionSort(ctrl_nlp_th, request_expression->request_position_start, request_expression->request_positions_nb)) DPcErr;

  // Necessary for NlpRequestExpressionOverlapMark and NlpRequestExpressionIsOrdered
  request_expression->orip_start = (-1);
  request_expression->orips_nb = 0;
  for (int i = 0; i < expression->input_parts_nb; i++)
  {
    struct request_input_part *request_input_part = request_input_parts + match_zone_input_part[i].current;

    og_status status = NlpRequestExpressionAddOrip(ctrl_nlp_th, request_expression,
        request_input_part->Ioriginal_request_input_part);
    IFE(status);
  }

  IFE(NlpCalculateScoreDuringParsing(ctrl_nlp_th, request_expression));
  IFE(NlpSetNbAnys(ctrl_nlp_th, request_expression));

  int must_add_request_expression = TRUE;

  if (must_add_request_expression)
  {
    if (request_expression->nb_anys == 0 && ctrl_nlp_th->accept_any_expressions)
    {
      must_add_request_expression = FALSE;
    }
  }

  if (must_add_request_expression)
  {
    if (request_expression->nb_anys > DOgNlpMaxNbAnys)
    {
      must_add_request_expression = FALSE;
    }
  }

  if (must_add_request_expression)
  {
    if (request_expression->expression->keep_order)
    {
      og_bool is_ordered = NlpRequestExpressionIsOrdered(ctrl_nlp_th, request_expression);
      IFE(is_ordered);
      if (!is_ordered) must_add_request_expression = FALSE;
    }
  }

  if (must_add_request_expression)
  {
    if (request_expression->expression->glued)
    {
      og_bool is_glued = NlpRequestExpressionIsGlued(ctrl_nlp_th, request_expression);
      IFE(is_glued);
      if (!is_glued)
      {
        must_add_request_expression = FALSE;
      }
    }
  }

  if (must_add_request_expression)
  {
    og_bool request_position_overlap = NlpRequestPositionOverlap(ctrl_nlp_th,
        request_expression->request_position_start, request_expression->request_positions_nb);
    if (request_position_overlap) must_add_request_expression = FALSE;
  }

  if (must_add_request_expression)
  {
    // Must be called before NlpRequestExpressionExists
    IFE(NlpRequestExpressionOverlapMark(ctrl_nlp_th, request_expression));
    IFE(NlpRequestExpressionGetSparseMark(ctrl_nlp_th, request_expression));
  }

  if (must_add_request_expression)
  {
    og_bool context_is_valid = NlpContextIsValid(ctrl_nlp_th, request_expression);
    IFE(context_is_valid);
    if (!context_is_valid)
    {
      must_add_request_expression = FALSE;
    }
  }

  if (must_add_request_expression)
  {
    if (request_expression->expression->is_recursive)
    {
      og_bool contains_level_minus_one_request_expression = FALSE;
      og_bool recursive_without_any_found = FALSE;
      for (int i = 0; i < request_expression->orips_nb; i++)
      {
        struct request_input_part *request_input_part = NlpGetRequestInputPart(ctrl_nlp_th, request_expression, i);
        IFN(request_input_part) DPcErr;

        if (request_input_part->type == nlp_input_part_type_Interpretation)
        {
          struct request_expression *sub_request_expression = OgHeapGetCell(ctrl_nlp_th->hrequest_expression,
              request_input_part->Irequest_expression);
          IFN(sub_request_expression) DPcErr;
          if (sub_request_expression->level + 1 == request_expression->level) contains_level_minus_one_request_expression =
          TRUE;
          if (!ctrl_nlp_th->accept_any_expressions) recursive_without_any_found = TRUE;
          if (sub_request_expression->recursive_without_any_chosen) recursive_without_any_found = TRUE;
        }
      }
      if (contains_level_minus_one_request_expression && recursive_without_any_found)
      {
        request_expression->recursive_without_any_chosen = TRUE;
      }
      else
      {
        must_add_request_expression = FALSE;
      }
    }
    else if (request_expression->expression->interpretation->is_recursive && ctrl_nlp_th->accept_any_expressions)
    {
      og_bool recursive_sub_request_expression_found = FALSE;
      for (int i = 0; i < request_expression->orips_nb; i++)
      {
        struct request_input_part *request_input_part = NlpGetRequestInputPart(ctrl_nlp_th, request_expression, i);
        IFN(request_input_part) DPcErr;

        if (request_input_part->type == nlp_input_part_type_Interpretation)
        {
          struct request_expression *sub_request_expression = OgHeapGetCell(ctrl_nlp_th->hrequest_expression,
              request_input_part->Irequest_expression);
          IFN(sub_request_expression) DPcErr;
          if (sub_request_expression->expression->is_recursive)
          {
            recursive_sub_request_expression_found = TRUE;
            break;
          }
        }
      }
      if (!recursive_sub_request_expression_found)
      {
        og_bool is_included_in_noany_expression = FALSE;
        is_included_in_noany_expression = NlpRequestExpressionIsIncludedInNoanyRequestExpression(ctrl_nlp_th,
            request_expression);
        IFE(is_included_in_noany_expression);
        if (!is_included_in_noany_expression) request_expression->recursive_without_any_chosen = TRUE;
      }
    }
  }

  if (must_add_request_expression)
  {
    IFE(NlpGetAutoCompleteRequestWord(ctrl_nlp_th, request_expression));

    struct request_expression *request_expressions = OgHeapGetCell(ctrl_nlp_th->hrequest_expression, 0);
    IFn(request_expressions) DPcErr;

    struct request_expression_access_cache cache[1];
    memset(cache, 0, sizeof(struct request_expression_access_cache));

    cache->request_expressions = request_expressions;
    cache->request_expressions_used = request_expression_used;
    cache->request_positions = request_positions;
    cache->request_positions_used = request_position_used;

    struct request_expression *same_request_expression;
    og_bool request_expression_exists = NlpRequestExpressionExists(ctrl_nlp_th, cache, request_expression,
        &same_request_expression);
    IF(request_expression_exists) DPcErr;
    if (request_expression_exists)
    {
      og_bool must_log_compared_expressions = TRUE;
      if (request_expression->request_positions_nb == 1) must_log_compared_expressions = FALSE;
      if (must_log_compared_expressions)
      {
//        OgMsg(ctrl_nlp_th->hmsg, "", DOgMsgDestInLog, "comparing new request expression with same request expression:");
//        IFE(NlpInterpretTreeLog(ctrl_nlp_th, request_expression, 0));
//        IFE(NlpInterpretTreeLog(ctrl_nlp_th, same_request_expression, 0));
      }
      if (NlpDifferentAutoCompleteRequestWord(ctrl_nlp_th, request_expression, same_request_expression))
      {
        NlpLog(DOgNlpTraceInterpret, "Keeping same request expression as different auto-complete");
      }
      else
      {
        must_add_request_expression = FALSE;
      }
    }
  }

  og_bool request_expression_added = TRUE;
  if (must_add_request_expression)
  {
    *prequest_expression = request_expression;
  }
  else
  {
    OgHeapSetCellsUsed(ctrl_nlp_th->hrequest_expression, request_expression_used);
    OgHeapSetCellsUsed(ctrl_nlp_th->hrequest_position, request_position_used);
    OgHeapSetCellsUsed(ctrl_nlp_th->horip, orip_used);
    request_expression_added = FALSE;
    *prequest_expression = NULL;
  }
  return (request_expression_added);
}

static og_bool NlpRequestExpressionExists(og_nlp_th ctrl_nlp_th, struct request_expression_access_cache *cache,
    struct request_expression *request_expression, struct request_expression **psame_request_expression)
{
  *psame_request_expression = NULL;

  for (int i = 0; i < cache->request_expressions_used; i++)
  {
    og_bool is_same_request_expression = NlpRequestExpressionSame(ctrl_nlp_th, cache, request_expression,
        cache->request_expressions + i);
    if (is_same_request_expression)
    {
//      OgMsg(ctrl_nlp_th->hmsg, "", DOgMsgDestInLog,
//          "NlpRequestExpressionExists compared equal new request expression with same request expression:");
//      IFE(NlpInterpretTreeLog(ctrl_nlp_th, request_expression, 0));
//      IFE(NlpInterpretTreeLog(ctrl_nlp_th, all_request_expression + i, 0));
      *psame_request_expression = cache->request_expressions + i;
      return TRUE;
    }
  }
  return FALSE;
}

static og_bool NlpRequestExpressionIsIncludedInNoanyRequestExpression(og_nlp_th ctrl_nlp_th,
    struct request_expression *request_expression)
{
  int request_expression_used = OgHeapGetCellsUsed(ctrl_nlp_th->hrequest_expression);
  struct request_expression *request_expressions = OgHeapGetCell(ctrl_nlp_th->hrequest_expression, 0);
  IFN(request_expressions) DPcErr;

  struct request_position *request_position;

  request_position = OgHeapGetCell(ctrl_nlp_th->hrequest_position, request_expression->request_position_start);
  IFN(request_position) DPcErr;
  int start_position = request_position->start;
  request_position = OgHeapGetCell(ctrl_nlp_th->hrequest_position,
      request_expression->request_position_start + request_expression->request_positions_nb - 1);
  IFN(request_position) DPcErr;
  int end_position = request_position->start + request_position->length;

  for (int i = 0; i < request_expression_used; i++)
  {
    struct request_expression *no_any_request_expression = request_expressions + i;
    IFN(no_any_request_expression) DPcErr;
    if (no_any_request_expression->nb_anys != 0) continue;
    if (no_any_request_expression->expression != request_expression->expression) continue;

    request_position = OgHeapGetCell(ctrl_nlp_th->hrequest_position, no_any_request_expression->request_position_start);
    IFN(request_position) DPcErr;
    int no_any_start_position = request_position->start;
    request_position = OgHeapGetCell(ctrl_nlp_th->hrequest_position,
        no_any_request_expression->request_position_start + no_any_request_expression->request_positions_nb - 1);
    IFN(request_position) DPcErr;
    int no_any_end_position = request_position->start + request_position->length;

    if (no_any_start_position <= start_position && end_position <= no_any_end_position) return TRUE;

  }
  return FALSE;
}

static og_bool NlpRequestExpressionSame(og_nlp_th ctrl_nlp_th, struct request_expression_access_cache *cache,
    struct request_expression *request_expression1, struct request_expression *request_expression2)
{
  if (request_expression1->expression != request_expression2->expression) return FALSE;
  if (request_expression1->overlap_mark != request_expression2->overlap_mark) return FALSE;
  if (request_expression1->nb_anys != request_expression2->nb_anys) return FALSE;
  if (request_expression1->total_score != request_expression2->total_score) return FALSE;

  if (request_expression1->expression->is_recursive)
  {
    if (request_expression1->level == request_expression2->level && request_expression1->nb_anys == 0) return TRUE;
  }

  og_bool same_positions = NlpRequestPositionSame(ctrl_nlp_th, cache, request_expression1->request_position_start,
      request_expression1->request_positions_nb, request_expression2->request_position_start,
      request_expression2->request_positions_nb);
  return (same_positions);
}

og_bool NlpRequestExpressionIsOrdered(og_nlp_th ctrl_nlp_th, struct request_expression *request_expression)
{

  if (ctrl_nlp_th->loginfo->trace & DOgNlpTraceMatchExpression)
  {
    static int nb_calls = 0;
    nb_calls++;
    NlpLog(DOgNlpTraceMatchExpression,
        "NlpRequestExpressionIsOrdered: starting on expression (%d): alias_any_input_part_position=%d any_input_part_position=%d orips=%d",
        nb_calls, request_expression->expression->alias_any_input_part_position,
        request_expression->expression->any_input_part_position, request_expression->orips_nb);
    IFE(NlpInterpretTreeLog(ctrl_nlp_th, request_expression, 2));
  }


  og_bool is_ordered = TRUE;

  for (int i = 0; i + 1 < request_expression->orips_nb; i++)
  {
    struct request_input_part *request_input_part1 = NlpGetRequestInputPart(ctrl_nlp_th, request_expression, i);
    IFN(request_input_part1) DPcErr;
    struct request_input_part *request_input_part2 = NlpGetRequestInputPart(ctrl_nlp_th, request_expression, i + 1);
    IFN(request_input_part2) DPcErr;

    is_ordered = NlpRequestInputPartsAreOrdered(ctrl_nlp_th, request_input_part1, request_input_part2);
    IFE(is_ordered);
    if (!is_ordered)
    {
      break;
    }
  }
  NlpLog(DOgNlpTraceMatchExpression,"NlpRequestExpressionIsOrdered: expression %s",(is_ordered?"is ordered":"is not ordered"));

  return is_ordered;
}

static og_bool NlpRequestExpressionIsGlued(og_nlp_th ctrl_nlp_th, struct request_expression *request_expression)
{

  if (ctrl_nlp_th->loginfo->trace & DOgNlpTraceMatchExpression)
  {
    static int nb_calls = 0;
    nb_calls++;
    NlpLog(DOgNlpTraceMatchExpression,
        "NlpRequestExpressionIsGlued: starting on expression (%d): alias_any_input_part_position=%d any_input_part_position=%d orips=%d",
        nb_calls, request_expression->expression->alias_any_input_part_position,
        request_expression->expression->any_input_part_position, request_expression->orips_nb);
    IFE(NlpInterpretTreeLog(ctrl_nlp_th, request_expression, 2));
  }

  og_bool is_glued = TRUE;
  int any_input_part_position = request_expression->expression->any_input_part_position;

  for (int i = 0; i + 1 < request_expression->orips_nb; i++)
  {
    struct request_input_part *request_input_part1 = NlpGetRequestInputPart(ctrl_nlp_th, request_expression, i);
    IFN(request_input_part1) DPcErr;
    if (request_expression->expression->keep_order)
    {
      if (i == any_input_part_position - 1) continue;
      struct request_input_part *request_input_part2 = NlpGetRequestInputPart(ctrl_nlp_th, request_expression, i + 1);
      IFN(request_input_part2) DPcErr;
      if (request_expression->expression->glue_strength == nlp_glue_strength_Punctuation)
      {
        is_glued = NlpRequestInputPartsAreGlued(ctrl_nlp_th, request_input_part1, request_input_part2);
        IFE(is_glued);
        if (!is_glued)
        {
          is_glued = NlpRequestInputPartsAreExpressionGlued(ctrl_nlp_th, request_input_part1, request_input_part2);
          IFE(is_glued);
          if (!is_glued) break;
        }
      }
      else
      {
        is_glued = NlpRequestInputPartsAreGlued(ctrl_nlp_th, request_input_part1, request_input_part2);
        IFE(is_glued);
        if (!is_glued) break;
      }
    }
    else
    {
      is_glued = FALSE;
      for (int j = i + 1; j < request_expression->orips_nb; j++)
      {
        if (i == any_input_part_position - 1 && j == any_input_part_position)
        {
          is_glued = TRUE;
          break;
        }
        else
        {
          struct request_input_part *request_input_part2 = NlpGetRequestInputPart(ctrl_nlp_th, request_expression, j);
          IFN(request_input_part2) DPcErr;
          is_glued = NlpRequestInputPartsAreGlued(ctrl_nlp_th, request_input_part1, request_input_part2);
          IFE(is_glued);
          if (is_glued) break;
          if (request_expression->expression->glue_strength == nlp_glue_strength_Punctuation)
          {
            is_glued = NlpRequestInputPartsAreExpressionGlued(ctrl_nlp_th, request_input_part1, request_input_part2);
            IFE(is_glued);
            if (is_glued) break;
          }
        }
      }
      if (!is_glued) break;
    }
  }

  NlpLog(DOgNlpTraceMatchExpression,"NlpRequestExpressionIsGlued: expression %s",(is_glued?"is glued":"is not glued"));

  return is_glued;
}

static og_status NlpRequestExpressionOverlapMark(og_nlp_th ctrl_nlp_th, struct request_expression *request_expression)
{
  request_expression->overlap_mark = 0;

  if (request_expression->orips_nb <= 1) DONE;

  for (int i = 0; i + 1 < request_expression->orips_nb; i++)
  {
    struct request_input_part *request_input_part1 = NlpGetRequestInputPart(ctrl_nlp_th, request_expression, i);
    IFN(request_input_part1) DPcErr;

    for (int j = i + 1; j < request_expression->orips_nb; j++)
    {
      struct request_input_part *request_input_part2 = NlpGetRequestInputPart(ctrl_nlp_th, request_expression, j);
      IFN(request_input_part2) DPcErr;

      int input_parts_overlap_mark;
      IFE(
          NlpRequestExpressionInputPartsOverlapMark(ctrl_nlp_th, request_input_part1, request_input_part2,
              &input_parts_overlap_mark));
      request_expression->overlap_mark += input_parts_overlap_mark;
    }
  }

  DONE;
}

static og_status NlpRequestExpressionInputPartsOverlapMark(og_nlp_th ctrl_nlp_th,
    struct request_input_part *request_input_part1, struct request_input_part *request_input_part2, int *poverlap_mark)
{
  struct request_position *request_position;

  request_position = OgHeapGetCell(ctrl_nlp_th->hrequest_position, request_input_part1->request_position_start);
  IFN(request_position) DPcErr;
  int start_position1 = request_position->start;
  request_position = OgHeapGetCell(ctrl_nlp_th->hrequest_position,
      request_input_part1->request_position_start + request_input_part1->request_positions_nb - 1);
  IFN(request_position) DPcErr;
  int end_position1 = request_position->start + request_position->length;

  request_position = OgHeapGetCell(ctrl_nlp_th->hrequest_position, request_input_part2->request_position_start);
  IFN(request_position) DPcErr;
  int start_position2 = request_position->start;
  request_position = OgHeapGetCell(ctrl_nlp_th->hrequest_position,
      request_input_part2->request_position_start + request_input_part2->request_positions_nb - 1);
  IFN(request_position) DPcErr;
  int end_position2 = request_position->start + request_position->length;

  int overlap_mark = 0;
  // a b a b -> +100 (full overlap)
  if (start_position1 < start_position2 && start_position2 < end_position1 && end_position1 < end_position2) overlap_mark +=
      100;
  // a b b a -> +1   (included)
  if (start_position1 < start_position2 && end_position2 < end_position1) overlap_mark += 1;
  // b a a b -> +1   (included)
  if (start_position2 < start_position1 && end_position1 < end_position2) overlap_mark += 1;
  // b a b a -> +100 (full overlap)
  if (start_position2 < start_position1 && start_position1 < end_position2 && end_position2 < end_position1) overlap_mark +=
      100;
  *poverlap_mark = overlap_mark;

  return FALSE;
}

static og_status NlpRequestExpressionGetSparseMark(og_nlp_th ctrl_nlp_th, struct request_expression *request_expression)
{
  request_expression->sparse_mark = NlpRequestPositionDistance(ctrl_nlp_th, request_expression->request_position_start,
      request_expression->request_positions_nb);
  return (request_expression->sparse_mark);
}

/*
 * For the moment, we build all the top level interpretations from the expressions
 */
og_status NlpRequestInterpretationsBuild(og_nlp_th ctrl_nlp_th, json_t *json_interpretations)
{
  GQueue *sorted_request_expressions = ctrl_nlp_th->sorted_request_expressions;
  for (GList *iter = sorted_request_expressions->head; iter; iter = iter->next)
  {
    struct request_expression *request_expression = iter->data;
    if (!request_expression->keep_as_result) continue;
    IFE(NlpRequestInterpretationBuild(ctrl_nlp_th, request_expression, json_interpretations));
  }
  DONE;
}

static og_status NlpRequestInterpretationBuild(og_nlp_th ctrl_nlp_th, struct request_expression *request_expression,
    json_t *json_interpretations)
{
  struct expression *expression = request_expression->expression;
  struct interpretation *interpretation = expression->interpretation;

  package_t package = interpretation->package;

  NlpLog(DOgNlpTraceInterpret, "NlpInterpretRequestInterpretation: found interpretation '%s' '%s' in package '%s' '%s'",
      interpretation->slug, interpretation->id, package->slug, package->id)

  json_t *json_interpretation = json_object();

  IF(json_array_append_new(json_interpretations, json_interpretation))
  {
    NlpThrowErrorTh(ctrl_nlp_th, "NlpInterpretRequestInterpretation: error appending json_interpretation to array");
    DPcErr;
  }

  json_t *json_package_id = json_string(package->id);
  IF(json_object_set_new(json_interpretation, "package", json_package_id))
  {
    NlpThrowErrorTh(ctrl_nlp_th, "NlpInterpretRequestInterpretation: error setting json_package_id");
    DPcErr;
  }

  json_t *json_interpretation_id = json_string(interpretation->id);
  IF(json_object_set_new(json_interpretation, "id", json_interpretation_id))
  {
    NlpThrowErrorTh(ctrl_nlp_th, "NlpInterpretRequestInterpretation: error setting json_interpretation_id");
    DPcErr;
  }

  // simulate a timeout right here
  IFE(OgNlpSynchroTestSleepIfTimeoutNeeded(ctrl_nlp_th, nlp_timeout_in_NlpRequestInterpretationBuild));

  json_t *json_interpretation_slug = json_string(interpretation->slug);
  IF(json_object_set_new(json_interpretation, "slug", json_interpretation_slug))
  {
    NlpThrowErrorTh(ctrl_nlp_th, "NlpInterpretRequestInterpretation: error setting json_interpretation_slug");
    DPcErr;
  }

  // round the score to 2 digit after dot
  double rounded_total_score = floor(request_expression->total_score * 100) / 100;
  json_t *json_score = json_real(rounded_total_score);
  IF(json_object_set_new(json_interpretation, "score", json_score))
  {
    NlpThrowErrorTh(ctrl_nlp_th, "NlpInterpretRequestInterpretation: error setting json_score");
    DPcErr;
  }

  if (request_expression->json_solution)
  {
    IF(json_object_set_new(json_interpretation, "solution", json_deep_copy(request_expression->json_solution)))
    {
      NlpThrowErrorTh(ctrl_nlp_th, "NlpInterpretRequestInterpretation: error setting json_solution");
      DPcErr;
    }
  }

  if (ctrl_nlp_th->show_private)
  {
    json_t *scope = NULL;
    switch (interpretation->scope)
    {
      case nlp_interpretation_scope_type_public:
        scope = json_string("public");
        break;
      case nlp_interpretation_scope_type_private:
        scope = json_string("private");
        break;
      default:
        scope = NULL;
        break;
    }

    if (scope != NULL)
    {
      IF(json_object_set_new(json_interpretation, "scope", scope))
      {
        NlpThrowErrorTh(ctrl_nlp_th, "NlpRequestInterpretationBuild : Error while setting interpretation scope %d",
            interpretation->scope);
        DPcErr;
      }
    }
  }

  if (ctrl_nlp_th->show_explanation)
  {
    IFE(NlpInterpretTreeJson(ctrl_nlp_th, request_expression, json_interpretation));
  }

  DONE;
}

og_status NlpSortedRequestExpressionsLog(og_nlp_th ctrl_nlp_th, char *title)
{
  OgMsg(ctrl_nlp_th->hmsg, "", DOgMsgDestInLog, "%s", title);

  GQueue *sorted_request_expressions = ctrl_nlp_th->sorted_request_expressions;
  for (GList *iter = sorted_request_expressions->head; iter; iter = iter->next)
  {
    struct request_expression *request_expression = iter->data;
    IFE(NlpRequestExpressionLog(ctrl_nlp_th, request_expression, 2));
  }
  DONE;
}

og_status NlpRequestExpressionsLog(og_nlp_th ctrl_nlp_th, int request_expression_start, char *title)
{
  OgMsg(ctrl_nlp_th->hmsg, "", DOgMsgDestInLog, "%s", title);
  int request_expression_used = OgHeapGetCellsUsed(ctrl_nlp_th->hrequest_expression);
  for (int i = request_expression_start; i < request_expression_used; i++)
  {
    struct request_expression *request_expression = OgHeapGetCell(ctrl_nlp_th->hrequest_expression, i);
    IFN(request_expression) DPcErr;

    IFE(NlpRequestExpressionLog(ctrl_nlp_th, request_expression, 2));
  }
  DONE;
}

og_status NlpRequestExpressionLog(og_nlp_th ctrl_nlp_th, struct request_expression *request_expression, int offset)
{
  IFN(request_expression) DPcErr;

  char string_offset[DPcPathSize];
  memset(string_offset, ' ', offset);
  string_offset[offset] = 0;

  if (request_expression->deleted) string_offset[offset - 1] = '-';

  char string_positions[DPcPathSize];
  NlpRequestPositionString(ctrl_nlp_th, request_expression->request_position_start,
      request_expression->request_positions_nb, DPcPathSize, string_positions);

  char highlight[DPcPathSize];
  NlpRequestPositionStringHighlight(ctrl_nlp_th, request_expression->request_position_start,
      request_expression->request_positions_nb, DPcPathSize, highlight);

  char solution[DPcPathSize];
  NlpRequestSolutionString(ctrl_nlp_th, request_expression, DPcPathSize, solution);

  char overlap_mark[DPcPathSize];
  overlap_mark[0] = 0;
  if (request_expression->overlap_mark) sprintf(overlap_mark, " overlap_mark=%d", request_expression->overlap_mark);

  char any[DPcPathSize];
  any[0] = 0;
  if (request_expression->nb_anys > 0) sprintf(any, " nb_anys=%d", request_expression->nb_anys);

  char scores[DPcPathSize];
  scores[0] = 0;
  struct request_score *score = request_expression->score;
  if (score->coverage != 0.0 || score->locale != 0.0 || score->spelling != 0.0 || score->overlap != 0.0
      || score->any != 0.0)
  {
    sprintf(scores, " scores=[%.2f %.2f %.2f %.2f %.2f %.2f]", score->coverage, score->locale, score->spelling,
        score->overlap, score->any, score->scope);
  }

//  char sortdata[DPcPathSize];
//  sortdata[0] = 0;
//  sprintf(sortdata, " sortdata=[inter-scope=%d scope=%.2f avs=%d kar=%d rpn=%d om=%d na=%d total_score=%.2f level=%d]",
//      request_expression->expression->interpretation->scope, request_expression->score->scope,
//      request_expression->any_validate_status, request_expression->keep_as_result,
//      request_expression->request_positions_nb, request_expression->overlap_mark, request_expression->nb_anys,
//      request_expression->total_score, request_expression->level);
//
  char ac_request_word[DPcPathSize];
  ac_request_word[0] = 0;
  if (request_expression->auto_complete_request_word)
  {
    og_string acrw = OgHeapGetCell(ctrl_nlp_th->hba, request_expression->auto_complete_request_word->start);
    IFN(acrw) DPcErr;
    snprintf(ac_request_word, DPcPathSize, " acrw='%s'", acrw);
  }

  char alias_name[DPcPathSize];
  alias_name[0] = 0;
//  struct original_request_input_part *original_request_input_part = OgHeapGetCell(
//      ctrl_nlp_th->horiginal_request_input_part, request_expression->Isuper_original_request_input_part);
//  IFN(original_request_input_part) DPcErr;
//  struct request_input_part *request_input_part = OgHeapGetCell(ctrl_nlp_th->hrequest_input_part,
//      original_request_input_part->Irequest_input_part);
//  IFNE(request_input_part);
//  struct alias *alias = request_input_part->input_part->alias;
//  snprintf(alias_name, DPcPathSize, " alias='%s'", alias->alias);

  char rwac[DPcPathSize];
  rwac[0] = 0;
  if (request_expression->recursive_without_any_chosen)
  {
    sprintf(rwac, " rwac");
  }

  struct expression *expression = request_expression->expression;
  OgMsg(ctrl_nlp_th->hmsg, "", DOgMsgDestInLog, "%s%2d:%d%s [%s] '%.*s' in interpretation '%s': '%s'%s%s%s%s%s%s%s%s%s",
      string_offset, request_expression->self_index, request_expression->level,
      (request_expression->keep_as_result ? "*" : ""), string_positions, DPcPathSize, expression->text,
      expression->interpretation->slug, highlight, (solution[0] ? " " : ""), solution, any, overlap_mark, scores,
      ac_request_word, alias_name, rwac);
  DONE;
}

/*
 * This function is mainly for debugging
 */
og_status NlpRequestExpressionShowTree(og_nlp_th ctrl_nlp_th, int Irequest_expression, og_string label)
{
  static int counter = 0;
  counter++;
  OgMsg(ctrl_nlp_th->hmsg, "", DOgMsgDestInLog, "Showing expression %d, counter=%d %s", Irequest_expression, counter,
      label);
  struct request_expression *request_expression = OgHeapGetCell(ctrl_nlp_th->hrequest_expression, Irequest_expression);
  IFE(NlpInterpretTreeLog(ctrl_nlp_th, request_expression, 0));
  DONE;
}

