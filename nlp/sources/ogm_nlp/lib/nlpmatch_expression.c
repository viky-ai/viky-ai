/*
 *  Handling log messages for Natural Language Processing library
 *  Copyright (c) 2017 Pertimm, by Patrick Constant
 *  Dev : September 2017
 *  Version 1.0
 */
#include "ogm_nlp.h"
#include <stdlib.h>

#define DOgNlpMaxNbExpressionCombinations 10000

static int NlpMatchExpressionsZone(og_nlp_th ctrl_nlp_th, struct expression *expression, int Irequest_input_part,
    struct match_zone_input_part *match_zone_input_part, int match_zone_input_part_length);
static og_bool NlpMatchExpressionsZoneRecursive(og_nlp_th ctrl_nlp_th, struct expression *expression,
    int Irequest_input_part, struct match_zone_input_part *match_zone_input_part, int start, int length,
    int *pcombination_counter);
static og_bool NlpInputPartsTooFar(og_nlp_th ctrl_nlp_th, struct expression *expression, int Irequest_input_part,
    struct match_zone_input_part *match_zone_input_part, int start, int length);
static int NlpInputPartPositionCmp(gconstpointer ptr_input_part_position1, gconstpointer ptr_input_part_position2,
    gpointer user_data);
static int NlpMatchExpression(og_nlp_th ctrl_nlp_th, struct expression *expression, int Irequest_input_part,
    struct match_zone_input_part *match_zone_input_part);
static int NlpRequestInputPartCmp(gconstpointer ptr_request_input_part1, gconstpointer ptr_request_input_part2,
    gpointer user_data);

og_status NlpMatchExpressions(og_nlp_th ctrl_nlp_th)
{
  int request_expression_used = OgHeapGetCellsUsed(ctrl_nlp_th->hrequest_expression);
  int request_input_part_used = OgHeapGetCellsUsed(ctrl_nlp_th->hrequest_input_part);

  ctrl_nlp_th->new_request_expression_start = request_expression_used;
  ctrl_nlp_th->new_request_input_part_start = request_input_part_used;

  struct request_input_part *request_input_parts = OgHeapGetCell(ctrl_nlp_th->hrequest_input_part, 0);
  IFN(request_input_parts) DPcErr;

  g_qsort_with_data(request_input_parts, request_input_part_used, sizeof(struct request_input_part),
      NlpRequestInputPartCmp, ctrl_nlp_th);

  IFE(NlpOriginalRequestInputPartsCalculate(ctrl_nlp_th));

  if (ctrl_nlp_th->loginfo->trace & DOgNlpTraceMatch)
  {
    char buffer[DPcPathSize];
    snprintf(buffer, DPcPathSize, "Looking for expressions at level %d using the following request input parts:",
        ctrl_nlp_th->level);
    IFE(NlpRequestInputPartsLog(ctrl_nlp_th, 0, buffer));
  }

  // scan all request input parts
  for (int i = 0; i < request_input_part_used; i++)
  {
    struct request_input_part *request_input_part = request_input_parts + i;

    if (request_input_part->super_list_status == nlp_super_list_status_Part) continue;

    struct expression *expression = request_input_part->input_part->expression;

    if (expression->input_parts != request_input_part->input_part) continue;

    // not enough input parts to match the expression
    if (expression->input_parts_nb > (request_input_part_used - i)) continue;

    if (!ctrl_nlp_th->accept_any_expressions)
    {
      if (expression->alias_any_input_part_position >= 0) continue;
    }

    struct match_zone_input_part match_zone_input_part[DOgMatchZoneInputPartSize];
    if (expression->input_parts_nb > DOgMatchZoneInputPartSize)
    {
      NlpThrowErrorTh(ctrl_nlp_th,
          "NlpMatchExpressions: expression->input_parts_nb (%d) > DOgMatchZoneInputPartSize (%d)",
          expression->input_parts_nb, DOgMatchZoneInputPartSize);
      DPcErr;
    }
    og_bool expression_matches_at_least_once = FALSE;
    int Iinput_part = 0, start = i, j = i;
    for (; j < request_input_part_used; j++)
    {
      struct request_input_part *rip = request_input_parts + j;
      struct input_part *input_part = expression->input_parts;
      input_part += Iinput_part;
      if (input_part != rip->input_part)
      {
        int length = j - start;
        if (length <= 0) break;
        match_zone_input_part[Iinput_part].start = start;
        match_zone_input_part[Iinput_part].length = length;
        Iinput_part++;
        if (Iinput_part >= expression->input_parts_nb)
        {
          expression_matches_at_least_once = TRUE;
          break;
        }
        start = j;
        j--;
      }
    }
    int length = j - start;
    if (length > 0)
    {
      match_zone_input_part[Iinput_part].start = start;
      match_zone_input_part[Iinput_part].length = length;
      Iinput_part++;
      if (Iinput_part >= expression->input_parts_nb)
      {
        expression_matches_at_least_once = TRUE;
      }
    }
    if (!expression_matches_at_least_once) continue;
    IFE(NlpMatchExpressionsZone(ctrl_nlp_th, expression, i, match_zone_input_part, expression->input_parts_nb));
    i += length - 1;
  }

  IFE(OgNlpSynchroTestSleepIfTimeoutNeeded(ctrl_nlp_th, nlp_timeout_in_NlpMatchExpressions));

  if (ctrl_nlp_th->loginfo->trace & DOgNlpTraceMatch)
  {
    int new_request_expression_used = OgHeapGetCellsUsed(ctrl_nlp_th->hrequest_expression);
    if (new_request_expression_used > ctrl_nlp_th->new_request_expression_start)
    {
      char buffer[DPcPathSize];
      snprintf(buffer, DPcPathSize, "NlpMatchExpressions: list of new request expression at level %d:",
          ctrl_nlp_th->level);
      IFE(NlpRequestExpressionsLog(ctrl_nlp_th, ctrl_nlp_th->new_request_expression_start, buffer));
    }
  }

  DONE;
}

static int NlpMatchExpressionsZone(og_nlp_th ctrl_nlp_th, struct expression *expression, int Irequest_input_part,
    struct match_zone_input_part *match_zone_input_part, int match_zone_input_part_length)
{
  int nb_combinations = 1;
  for (int i = 0; i < match_zone_input_part_length; i++)
  {
    nb_combinations *= match_zone_input_part[i].length;
  }

  if (nb_combinations > DOgNlpMaxNbExpressionCombinations)
  {
    if (ctrl_nlp_th->accept_any_expressions == FALSE)
    {
      NlpLog(DOgNlpTraceMinimal,
          "NlpMatchExpressionsZone at Irequest_input_part=%d nb_combinations=%d > %d, avoiding expression '%s'",
          Irequest_input_part, nb_combinations, DOgNlpMaxNbExpressionCombinations, expression->text);
    }
  }
  int combination_counter = DOgNlpMaxNbExpressionCombinations;
  return (NlpMatchExpressionsZoneRecursive(ctrl_nlp_th, expression, Irequest_input_part, match_zone_input_part, 0,
      match_zone_input_part_length, &combination_counter));
}

static og_bool NlpMatchExpressionsZoneRecursive(og_nlp_th ctrl_nlp_th, struct expression *expression,
    int Irequest_input_part, struct match_zone_input_part *match_zone_input_part, int start, int length,
    int *pcombination_counter)
{
  (*pcombination_counter)--;
  if (*pcombination_counter <= 0)
  {
    if (ctrl_nlp_th->accept_any_expressions == FALSE)
    {
      NlpLog(DOgNlpTraceMinimal,
          "NlpMatchExpressionsZone at Irequest_input_part=%d nb_combinations > %d, avoiding expression '%s'",
          Irequest_input_part, DOgNlpMaxNbExpressionCombinations, expression->text);
      NlpWarningAdd(ctrl_nlp_th,
          "NlpMatchExpressionsZone at Irequest_input_part=%d nb_combinations > %d, avoiding expression '%s'",
          Irequest_input_part, DOgNlpMaxNbExpressionCombinations, expression->text);
    }
    return TRUE;
  }
  if (length == 0)
  {
    return (NlpMatchExpression(ctrl_nlp_th, expression, Irequest_input_part, match_zone_input_part));
  }
  for (int i = 0; i < match_zone_input_part[start].length; i++)
  {
    match_zone_input_part[start].current = match_zone_input_part[start].start + i;
    og_bool too_far = NlpInputPartsTooFar(ctrl_nlp_th, expression, Irequest_input_part, match_zone_input_part, start,
        length);
    IFE(too_far);
    if (too_far) continue;
    og_bool must_stop = NlpMatchExpressionsZoneRecursive(ctrl_nlp_th, expression, Irequest_input_part, match_zone_input_part, start + 1,
            length - 1, pcombination_counter);
    IFE(must_stop);
    if (must_stop) return TRUE;
  }
  return FALSE;
}

struct input_part_position
{
  int start, end;
};

static og_bool NlpInputPartsTooFar(og_nlp_th ctrl_nlp_th, struct expression *expression, int Irequest_input_part,
    struct match_zone_input_part *match_zone_input_part, int start, int length)
{
  og_bool too_far = FALSE;
  if (start < 1) return (FALSE);
  if (expression->is_recursive) return (FALSE);
  if (ctrl_nlp_th->loginfo->trace & DOgNlpTraceMatchExpressionZone)
  {
    NlpLog(DOgNlpTraceMatchExpressionZone, "NlpInputPartsTooFar at %d/%d: checking following request_input_parts:",
        start + 1, start + length);
    for (int i = 0; i <= start; i++)
    {
      IFE(NlpRequestInputPartLog(ctrl_nlp_th, match_zone_input_part[i].current));
    }
  }
  struct input_part_position input_part_position[DOgMatchZoneInputPartSize];
  struct request_input_part *request_input_parts = OgHeapGetCell(ctrl_nlp_th->hrequest_input_part, 0);
  struct request_position *request_positions = OgHeapGetCell(ctrl_nlp_th->hrequest_position, 0);
  for (int i = 0; i <= start; i++)
  {
    struct request_input_part *request_input_part = request_input_parts + match_zone_input_part[i].current;
    struct request_position *request_position = request_positions + request_input_part->request_position_start;
    input_part_position[i].start = request_position->start;
    request_position = request_positions + request_input_part->request_position_start
        + request_input_part->request_positions_nb - 1;
    input_part_position[i].end = request_position->start + request_position->length;
  }
  g_qsort_with_data(input_part_position, start + 1, sizeof(struct input_part_position), NlpInputPartPositionCmp,
      ctrl_nlp_th);
  int max_distance = 0;
  for (int i = 0; i + 1 <= start; i++)
  {
    int distance = input_part_position[i + 1].start - input_part_position[i].end;
    if (max_distance < distance) max_distance = distance;
  }
  if (max_distance > expression->glue_distance) too_far = TRUE;
  if (ctrl_nlp_th->loginfo->trace & DOgNlpTraceMatchExpressionZone)
  {
    char string_too_far[DPcPathSize];
    if (too_far) sprintf(string_too_far, " (too far, max is %d)", expression->glue_distance);
    else sprintf(string_too_far, " (correct distance, max is %d)", expression->glue_distance);
    NlpLog(DOgNlpTraceMatchExpressionZone, "NlpInputPartsTooFar at %d/%d: max_distance is %d%s", start + 1,
        start + length, max_distance, string_too_far);
  }
  return (too_far);
}

static int NlpInputPartPositionCmp(gconstpointer ptr_input_part_position1, gconstpointer ptr_input_part_position2,
    gpointer user_data)
{
  struct input_part_position *input_part_position1 = (struct input_part_position *) ptr_input_part_position1;
  struct input_part_position *input_part_position2 = (struct input_part_position *) ptr_input_part_position2;
  if (input_part_position1->start != input_part_position2->start)
  {
    return (input_part_position1->start - input_part_position2->start);
  }
  return (input_part_position1->end - input_part_position2->end);
}

static int NlpMatchExpression(og_nlp_th ctrl_nlp_th, struct expression *expression, int Irequest_input_part,
    struct match_zone_input_part *match_zone_input_part)
{
  struct request_input_part *request_input_part = OgHeapGetCell(ctrl_nlp_th->hrequest_input_part, Irequest_input_part);
  IFN(request_input_part) DPcErr;
  struct request_expression *request_expression = NULL;

  if (expression != request_input_part->input_part->expression)
  {
    NlpThrowErrorTh(ctrl_nlp_th, "NlpMatchExpression: incoherent expression");
    DPcErr;
  }

  og_bool request_expression_added = NlpRequestExpressionAdd(ctrl_nlp_th, expression, match_zone_input_part,
      &request_expression, FALSE);
  IF(request_expression_added) DPcErr;

  if (request_expression_added)
  {
    char string_input_parts[DPcPathSize];
    string_input_parts[0] = 0;
    for (int i = 0; i < expression->input_parts_nb; i++)
    {
      sprintf(string_input_parts + strlen(string_input_parts), "%s%d", (i ? ":" : ""),
          match_zone_input_part[i].current);
    }

    if (ctrl_nlp_th->loginfo->trace & DOgNlpTraceMatch)
    {
      char string_positions[DPcPathSize];
      NlpRequestPositionString(ctrl_nlp_th, request_expression->request_position_start,
          request_expression->request_positions_nb, DPcPathSize, string_positions);

      char highlight[DPcPathSize];
      NlpRequestPositionStringHighlight(ctrl_nlp_th, request_expression->request_position_start,
          request_expression->request_positions_nb, DPcPathSize, highlight);

      NlpLog(DOgNlpTraceMatch, "found expression '%.*s' [%s] with request_input_parts %s at level %d : '%s'",
          DPcPathSize, expression->text, string_positions, string_input_parts, ctrl_nlp_th->level, highlight)
    }
  }

  DONE;
}

static int NlpRequestInputPartCmp(gconstpointer ptr_request_input_part1, gconstpointer ptr_request_input_part2,
    gpointer user_data)
{
  struct request_input_part *request_input_part1 = (struct request_input_part *) ptr_request_input_part1;
  struct request_input_part *request_input_part2 = (struct request_input_part *) ptr_request_input_part2;
  og_nlp_th ctrl_nlp_th = user_data;

  if (request_input_part1->interpret_package != request_input_part2->interpret_package)
  {
    return (request_input_part1->interpret_package - request_input_part2->interpret_package);
  }
  if (request_input_part1->Iinput_part != request_input_part2->Iinput_part)
  {
    return (request_input_part1->Iinput_part - request_input_part2->Iinput_part);
  }
  if (request_input_part1->request_positions_nb != request_input_part2->request_positions_nb)
  {
    return (request_input_part2->request_positions_nb - request_input_part1->request_positions_nb);
  }
  if (request_input_part1->sparse_mark != request_input_part2->sparse_mark)
  {
    return (request_input_part1->sparse_mark - request_input_part2->sparse_mark);
  }

  struct request_position *request_position1 = OgHeapGetCell(ctrl_nlp_th->hrequest_position,
      request_input_part1->request_position_start);
  struct request_position *request_position2 = OgHeapGetCell(ctrl_nlp_th->hrequest_position,
      request_input_part2->request_position_start);

  return (request_position1->start - request_position2->start);
}

