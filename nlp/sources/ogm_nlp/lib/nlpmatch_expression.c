/*
 *  Handling log messages for Natural Language Processing library
 *  Copyright (c) 2017 Pertimm, by Patrick Constant
 *  Dev : September 2017
 *  Version 1.0
 */
#include "ogm_nlp.h"
#include <stdlib.h>

static int NlpMatchExpressionsZone(og_nlp_th ctrl_nlp_th, int Irequest_input_part,
    struct match_zone_input_part *match_zone_input_part, int match_zone_input_part_length);
static int NlpMatchExpressionsZoneRecursive(og_nlp_th ctrl_nlp_th, int Irequest_input_part,
    struct match_zone_input_part *match_zone_input_part, int start, int length);
static int NlpMatchExpression(og_nlp_th ctrl_nlp_th, int Irequest_input_part,
    struct match_zone_input_part *match_zone_input_part);
static int NlpRequestInputPartCmp(gconstpointer ptr_request_input_part1, gconstpointer ptr_request_input_part2,
    gpointer user_data);

og_status NlpMatchExpressions(og_nlp_th ctrl_nlp_th)
{
  int request_expression_used = OgHeapGetCellsUsed(ctrl_nlp_th->hrequest_expression);
  int request_input_part_used = OgHeapGetCellsUsed(ctrl_nlp_th->hrequest_input_part);

  ctrl_nlp_th->new_request_expression_start = request_expression_used;
  ctrl_nlp_th->new_request_input_part_start = request_input_part_used;

  {
    struct request_input_part *all_request_input_part = OgHeapGetCell(ctrl_nlp_th->hrequest_input_part, 0);
    IFN(all_request_input_part) DPcErr;

    g_qsort_with_data(all_request_input_part, request_input_part_used, sizeof(struct request_input_part),
        NlpRequestInputPartCmp, ctrl_nlp_th);

    IFE(NlpOriginalRequestInputPartsCalculate(ctrl_nlp_th));
  }

  if (ctrl_nlp_th->loginfo->trace & DOgNlpTraceMatch)
  {
    char buffer[DPcPathSize];
    snprintf(buffer, DPcPathSize, "Looking for expressions at level %d using the following input parts:",
        ctrl_nlp_th->level);
    IFE(NlpRequestInputPartsLog(ctrl_nlp_th, 0, buffer));
  }

  // scan all request input parts
  for (int i = 0; i < request_input_part_used; i++)
  {
    struct request_input_part *request_input_part = OgHeapGetCell(ctrl_nlp_th->hrequest_input_part, i);
    struct expression *expression = request_input_part->input_part->expression;

    if (expression->input_parts != request_input_part->input_part) continue;

    // not enough input parts to match the expression
    if (expression->input_parts_nb > (request_input_part_used - i)) continue;

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
      struct request_input_part *rip = OgHeapGetCell(ctrl_nlp_th->hrequest_input_part, j);
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
    IFE(NlpMatchExpressionsZone(ctrl_nlp_th, i, match_zone_input_part, expression->input_parts_nb));
    //i = j - 1;
  }

  IFE(OgNlpSynchroTestSleepIfTimeoutNeeded(ctrl_nlp_th, nlp_timeout_in_NlpMatchExpressions));

  if (ctrl_nlp_th->loginfo->trace & DOgNlpTraceMatch)
  {
    int new_request_expression_used = OgHeapGetCellsUsed(ctrl_nlp_th->hrequest_expression);
    if (new_request_expression_used > ctrl_nlp_th->new_request_expression_start)
    {
      char buffer[DPcPathSize];
      snprintf(buffer, DPcPathSize, "List of new request expression at level %d:", ctrl_nlp_th->level);
      IFE(NlpRequestExpressionsLog(ctrl_nlp_th, ctrl_nlp_th->new_request_expression_start, buffer));
    }
  }

  DONE;
}

static int NlpMatchExpressionsZone(og_nlp_th ctrl_nlp_th, int Irequest_input_part,
    struct match_zone_input_part *match_zone_input_part, int match_zone_input_part_length)
{
  return (NlpMatchExpressionsZoneRecursive(ctrl_nlp_th, Irequest_input_part, match_zone_input_part, 0,
      match_zone_input_part_length));
}

static int NlpMatchExpressionsZoneRecursive(og_nlp_th ctrl_nlp_th, int Irequest_input_part,
    struct match_zone_input_part *match_zone_input_part, int start, int length)
{
  if (length == 0)
  {
    return (NlpMatchExpression(ctrl_nlp_th, Irequest_input_part, match_zone_input_part));
  }
  for (int i = 0; i < match_zone_input_part[start].length; i++)
  {
    match_zone_input_part[start].current = match_zone_input_part[start].start + i;
    IFE(
        NlpMatchExpressionsZoneRecursive(ctrl_nlp_th, Irequest_input_part, match_zone_input_part, start + 1,
            length - 1));
  }
  DONE;
}

static int NlpMatchExpression(og_nlp_th ctrl_nlp_th, int Irequest_input_part,
    struct match_zone_input_part *match_zone_input_part)
{
  struct request_input_part *request_input_part = OgHeapGetCell(ctrl_nlp_th->hrequest_input_part, Irequest_input_part);
  IFN(request_input_part) DPcErr;
  struct expression *expression = request_input_part->input_part->expression;
  struct request_expression *request_expression = NULL;
  og_bool request_expression_added = NlpRequestExpressionAdd(ctrl_nlp_th, expression, match_zone_input_part,
      &request_expression);
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

  struct request_position *request_position1 = OgHeapGetCell(ctrl_nlp_th->hrequest_position,
      request_input_part1->request_position_start);
  struct request_position *request_position2 = OgHeapGetCell(ctrl_nlp_th->hrequest_position,
      request_input_part2->request_position_start);

  return (request_position1->start - request_position2->start);
}

