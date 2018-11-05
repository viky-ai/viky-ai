/*
 *  Handling log messages for Natural Language Processing library
 *  Copyright (c) 2017 Pertimm, by Patrick Constant
 *  Dev : September 2017
 *  Version 1.0
 */
#include "ogm_nlp.h"

static og_status NlpRequestAnyAdd(og_nlp_th ctrl_nlp_th, struct request_expression *request_expression,
    int Irequest_position_before, int Irequest_position_after);
static og_status NlpRequestAnyTunePunctuation(og_nlp_th ctrl_nlp_th, GQueue *rw_list);
static int NlpRequestAnyCmp(gconstpointer ptr_request_any1, gconstpointer ptr_request_any2, gpointer user_data);
static og_status NlpRequestAnySort(og_nlp_th ctrl_nlp_th, struct request_expression *request_expression);
static og_status NlpRequestAnyDistance(og_nlp_th ctrl_nlp_th, struct request_any *request_any,
    struct request_expression *request_expression);
static og_status NlpRequestAnyDistanceFromPositions(og_nlp_th ctrl_nlp_th, struct request_any *request_any,
    int position1, int position2);
static og_status NlpRequestAnyIsOrdered(og_nlp_th ctrl_nlp_th, struct request_any *request_any,
    struct request_expression *request_expression);
static og_status NlpRequestAnyIsOrdered1(og_nlp_th ctrl_nlp_th, struct request_any *request_any,
    struct request_expression *request_expression);
static og_status NlpRequestAnyAddPositions(og_nlp_th ctrl_nlp_th, struct request_any *request_any,
    struct request_expression *request_expression);
static og_status NlpRequestAnyDelPositions(og_nlp_th ctrl_nlp_th, struct request_expression *request_expression);
static og_status NlpRequestAnyAddPositionsInputPart(og_nlp_th ctrl_nlp_th,
    struct request_expression *sub_request_expression, struct request_expression *request_expression);
static og_status NlpRequestAnyDelPositionsInputPart(og_nlp_th ctrl_nlp_th,
    struct request_expression *sub_request_expression, struct request_expression *request_expression);
static int NlpRequestAnyRequestExpressionString(og_nlp_th ctrl_nlp_th, struct request_any *request_any, int size,
    char *string);

og_status NlpInterpretAnyFlush(og_nlp_th ctrl_nlp_th)
{
  IFE(NlpInterpretAnyReset(ctrl_nlp_th));

  IFE(OgHeapFlush(ctrl_nlp_th->hrequest_any));

  DONE;
}

og_status NlpInterpretAnyReset(og_nlp_th ctrl_nlp_th)
{
  int request_any_used = OgHeapGetCellsUsed(ctrl_nlp_th->hrequest_any);
  struct request_any *request_any = OgHeapGetCell(ctrl_nlp_th->hrequest_any, 0);
  IFN(request_any) DPcErr;

  for (int i = 0; i < request_any_used; i++)
  {
    g_queue_clear(request_any->queue_request_expression);
    g_queue_clear(request_any->queue_request_words);
  }

  IFE(OgHeapReset(ctrl_nlp_th->hrequest_any));

  DONE;
}

og_status NlpRequestAnysAdd(og_nlp_th ctrl_nlp_th, struct request_expression *request_expression)
{
  NlpLog(DOgNlpTraceMatch, "NlpRequestAnysAdd: nb_anys=%d nb_anys_attached=%d, adding anys:",
      request_expression->nb_anys, request_expression->nb_anys_attached);

  struct request_position *request_position = OgHeapGetCell(ctrl_nlp_th->hrequest_position,
      request_expression->request_position_start);
  IFN(request_position) DPcErr;

  request_expression->request_any_start = -1;
  request_expression->request_anys_nb = 0;

  IFE(NlpRequestAnyAdd(ctrl_nlp_th, request_expression, -1, request_expression->request_position_start));
  for (int i = 0; i + 1 < request_expression->request_positions_nb; i++)
  {
    int Irequest_position_before = request_expression->request_position_start + i;
    int Irequest_position_after = Irequest_position_before + 1;
    IFE(NlpRequestAnyAdd(ctrl_nlp_th, request_expression, Irequest_position_before, Irequest_position_after));
  }
  IFE(
      NlpRequestAnyAdd(ctrl_nlp_th, request_expression,
          request_expression->request_position_start + request_expression->request_positions_nb - 1, -1));

  // Sorting by decreasing positions, because we want to choose the any that are preferably after
  // the expression where the any is positioned, when randomly choosing.
  IFE(NlpRequestAnySort(ctrl_nlp_th, request_expression));

  if (ctrl_nlp_th->loginfo->trace & DOgNlpTraceMatch)
  {
    NlpLog(DOgNlpTraceMatch, "NlpRequestAnysAdd: nb_anys=%d nb_anys_attached=%d, added anys:",
        request_expression->nb_anys, request_expression->nb_anys_attached);
    IFE(NlpRequestExpressionAnysLog(ctrl_nlp_th, request_expression));
  }

  DONE;
}

static og_status NlpRequestAnyAdd(og_nlp_th ctrl_nlp_th, struct request_expression *request_expression,
    int Irequest_position_before, int Irequest_position_after)
{
  struct request_position *request_position_before = NULL;
  struct request_position request_position_before_container[1];
  if (Irequest_position_before < 0)
  {
    request_position_before = request_position_before_container;
    request_position_before->start = 0;
    request_position_before->length = 0;
  }
  else
  {
    request_position_before = OgHeapGetCell(ctrl_nlp_th->hrequest_position, Irequest_position_before);
    IFN(request_position_before) DPcErr;
  }

  struct request_position *request_position_after;
  struct request_position request_position_after_container[1];
  if (Irequest_position_after < 0)
  {
    request_position_after = request_position_after_container;
    request_position_after->start = 0xfffffff;
    request_position_after->length = 0;
  }
  else
  {
    request_position_after = OgHeapGetCell(ctrl_nlp_th->hrequest_position, Irequest_position_after);
    IFN(request_position_after) DPcErr;
  }

  struct request_word *first_request_word = OgHeapGetCell(ctrl_nlp_th->hrequest_word, 0);
  IFN(first_request_word) DPcErr;

  for (struct request_word *rw = first_request_word; rw; rw = rw->next)
  {
    // ignore non basic word (build from ltras)
    if (rw->self_index >= ctrl_nlp_th->basic_request_word_used) break;

    if (rw->start_position < request_position_before->start + request_position_before->length) continue;

    GQueue rw_list[1];
    g_queue_init(rw_list);

    // fill list with all basic request word
    for (struct request_word *rw_next = rw; rw_next; rw_next = rw_next->next)
    {
      // ignore non basic word (build from ltras)
      if (rw_next->self_index >= ctrl_nlp_th->basic_request_word_used) break;

      if (rw_next->start_position + rw_next->length_position > request_position_after->start) break;

      g_queue_push_tail(rw_list, rw_next);
    }

    IFE(NlpRequestAnyTunePunctuation(ctrl_nlp_th, rw_list));
    if (rw_list->length <= 0) continue;

    size_t Irequest_any;
    struct request_any *request_any = OgHeapNewCell(ctrl_nlp_th->hrequest_any, &Irequest_any);
    IFn(request_any)
    {
      g_queue_clear(rw_list);
      DPcErr;
    }

    // copy rw list state
    *request_any->queue_request_words = *rw_list;
    request_any->is_attached = FALSE;

    g_queue_init(request_any->queue_request_expression);
    request_any->consumed = 0;

    if (request_expression->request_anys_nb == 0)
    {
      request_expression->request_any_start = Irequest_any;
    }
    request_expression->request_anys_nb++;

    if (ctrl_nlp_th->loginfo->trace & DOgNlpTraceMatch)
    {
      NlpLog(DOgNlpTraceMatch, "NlpRequestAnyAdd: added any from=%d:%d to=%d:%d:", request_position_before->start,
          request_position_before->length, request_position_after->start, request_position_after->length);
      IFE(NlpRequestExpressionAnyLog(ctrl_nlp_th, request_any));
    }
    break;
  }

  DONE;
}

static og_status NlpRequestAnyTunePunctuation(og_nlp_th ctrl_nlp_th, GQueue *rw_list)
{
  // remove last  request_word if they are punctuation
  for (GList *iter = rw_list->tail; iter; iter = rw_list->tail)
  {
    struct request_word *rw = iter->data;
    if (rw->is_punctuation)
    {
      g_queue_pop_tail(rw_list);
    }
    else
    {
      break;
    }
  }

  // remove fisrt request_word if they are punctuation
  for (GList *iter = rw_list->head; iter; iter = rw_list->head)
  {
    struct request_word *rw = iter->data;

    if (rw->is_punctuation)
    {
      g_queue_pop_head(rw_list);
    }
    else
    {
      break;
    }
  }

  DONE;
}

static int NlpRequestAnyCmp(gconstpointer ptr_request_any1, gconstpointer ptr_request_any2, gpointer user_data)
{
  struct request_any *request_any1 = (struct request_any *) ptr_request_any1;
  struct request_any *request_any2 = (struct request_any *) ptr_request_any2;

  if (request_any1->queue_request_words->head && request_any2->queue_request_words->head)
  {
    struct request_word *request_word1 = request_any1->queue_request_words->head->data;
    struct request_word *request_word2 = request_any2->queue_request_words->head->data;

    if (request_word1->start_position != request_word2->start_position)
    {
      return (request_word2->start_position - request_word1->start_position);
    }
  }
  return request_any1 - request_any2;
}

static og_status NlpRequestAnySort(og_nlp_th ctrl_nlp_th, struct request_expression *request_expression)
{
  if (request_expression->request_anys_nb == 0) DONE;

  struct request_any *request_any = OgHeapGetCell(ctrl_nlp_th->hrequest_any, request_expression->request_any_start);
  IFN(request_any) DPcErr;
  g_qsort_with_data(request_any, request_expression->request_anys_nb, sizeof(struct request_any), NlpRequestAnyCmp,
      ctrl_nlp_th);
  DONE;
}

og_status NlpRequestAnyAddClosest(og_nlp_th ctrl_nlp_th, struct request_expression *root_request_expression,
    struct request_expression *request_expression)
{

  struct request_any *request_anys = OgHeapGetCell(ctrl_nlp_th->hrequest_any, 0);
  IFN(request_anys) DPcErr;

  int minimum_distance = 0xfffffff;
  for (int i = 0; i < root_request_expression->request_anys_nb; i++)
  {
    struct request_any *request_any = request_anys + root_request_expression->request_any_start + i;
    if (request_any->is_attached) continue;

    og_bool is_ordered = NlpRequestAnyIsOrdered(ctrl_nlp_th, request_any, request_expression);
    IFE(is_ordered);
    if (is_ordered)
    {
      int distance = NlpRequestAnyDistance(ctrl_nlp_th, request_any, request_expression);
      IFE(distance);
      request_any->distance = distance;
      if (minimum_distance > distance)
      {
        minimum_distance = distance;
      }
    }
  }

  if (ctrl_nlp_th->loginfo->trace & DOgNlpTraceMatch)
  {
    NlpLog(DOgNlpTraceMatch, "NlpRequestAnyAddClosest: starting with expression and possible anys:");
    IFE(NlpRequestExpressionLog(ctrl_nlp_th, request_expression, 2));
    IFE(NlpRequestExpressionAnysLog(ctrl_nlp_th, root_request_expression));
  }

  for (int i = 0; i < root_request_expression->request_anys_nb; i++)
  {
    struct request_any *request_any = request_anys + root_request_expression->request_any_start + i;
    if (request_any->is_attached) continue;
    int distance = NlpRequestAnyDistance(ctrl_nlp_th, request_any, request_expression);
    IFE(distance);
    if (minimum_distance == distance)
    {
      og_bool is_ordered = NlpRequestAnyIsOrdered(ctrl_nlp_th, request_any, request_expression);
      IFE(is_ordered);
      if (is_ordered)
      {
        IFE(NlpRequestAnyAddRequestExpression(ctrl_nlp_th, request_any, request_expression));
        root_request_expression->nb_anys_attached++;
        int Irequest_any = request_any - request_anys;
        if (ctrl_nlp_th->loginfo->trace & DOgNlpTraceMatch)
        {
          NlpLog(DOgNlpTraceMatch,
              "NlpRequestAnyAddClosest: nb_anys=%d nb_anys_attached=%d, attaching Irequest_any=%d to expression:",
              root_request_expression->nb_anys, root_request_expression->nb_anys_attached, Irequest_any);
          IFE(NlpRequestExpressionLog(ctrl_nlp_th, request_expression, 2));
        }
      }
    }
  }

  if (ctrl_nlp_th->loginfo->trace & DOgNlpTraceMatch)
  {
    NlpLog(DOgNlpTraceMatch, "NlpRequestAnyAddClosest: nb_anys=%d nb_anys_attached=%d, finished with expression:",
        root_request_expression->nb_anys, root_request_expression->nb_anys_attached);
    IFE(NlpRequestExpressionLog(ctrl_nlp_th, request_expression, 2));
  }

  DONE;
}

#if 1
static og_status NlpRequestAnyDistance(og_nlp_th ctrl_nlp_th, struct request_any *request_any,
    struct request_expression *request_expression)
{
  struct request_position *request_positions = OgHeapGetCell(ctrl_nlp_th->hrequest_position,
      request_expression->request_position_start);
  IFN(request_positions) DPcErr;

  int minimum_distance = 0xfffffff;

  if (request_expression->expression->keep_order)
  {
    struct expression *expression = request_expression->expression;
    int alias_any_input_part_position = expression->alias_any_input_part_position;
    int any_input_part_position = expression->any_input_part_position;

    int position1 = 0;
    int position2 = 0xfffffff;

    if (any_input_part_position - 1 >= 0)
    {
      int Irequest_input_part1 = any_input_part_position - 1;
      struct request_input_part *request_input_part = NlpGetRequestInputPart(ctrl_nlp_th, request_expression,
          Irequest_input_part1);
      IFN(request_input_part) DPcErr;
      if (request_input_part->type == nlp_input_part_type_Word)
      {
        struct request_word *request_word = request_input_part->request_word;
        position1 = request_word->start_position + request_word->length_position;
      }
      else if (request_input_part->type == nlp_input_part_type_Interpretation)
      {
        struct request_expression *sub_request_expression = OgHeapGetCell(ctrl_nlp_th->hrequest_expression,
            request_input_part->Irequest_expression);
        IFN(sub_request_expression) DPcErr;
        struct request_position *request_position;
        request_position = OgHeapGetCell(ctrl_nlp_th->hrequest_position,
            sub_request_expression->request_position_start + sub_request_expression->request_positions_nb - 1);
        IFN(request_position) DPcErr;
        position1 = request_position->start + request_position->length;
      }
    }

    if (any_input_part_position < request_expression->orips_nb)
    {
      int Irequest_input_part2 = any_input_part_position;
      struct request_input_part *request_input_part = NlpGetRequestInputPart(ctrl_nlp_th, request_expression,
          Irequest_input_part2);
      IFN(request_input_part) DPcErr;
      if (request_input_part->type == nlp_input_part_type_Word)
      {
        struct request_word *request_word = request_input_part->request_word;
        position2 = request_word->start_position;

      }
      else if (request_input_part->type == nlp_input_part_type_Interpretation)
      {
        struct request_expression *sub_request_expression = OgHeapGetCell(ctrl_nlp_th->hrequest_expression,
            request_input_part->Irequest_expression);
        IFN(sub_request_expression) DPcErr;
        struct request_position *request_position;
        request_position = OgHeapGetCell(ctrl_nlp_th->hrequest_position,
            sub_request_expression->request_position_start);
        IFN(request_position) DPcErr;
        position2 = request_position->start;

      }
    }

    if (ctrl_nlp_th->loginfo->trace & DOgNlpTraceMatch)
    {
      NlpLog(DOgNlpTraceMatch,
          "NlpRequestAnyDistance: expression is keep order, p1=%d p2=%d alias_any_input_part_position=%d, any_input_part_position=%d:",
          position1, position2, alias_any_input_part_position, any_input_part_position);
      IFE(NlpRequestExpressionLog(ctrl_nlp_th, request_expression, 2));
    }

    struct request_word *first_request_word = request_any->queue_request_words->head->data;
    struct request_word *last_request_word = request_any->queue_request_words->tail->data;
    if (position1 <= first_request_word->start_position
        && last_request_word->start_position + last_request_word->length_position <= position2)
    {
      minimum_distance = NlpRequestAnyDistanceFromPositions(ctrl_nlp_th, request_any, position1, position2);
      IFE(minimum_distance);
    }
  }
  else
  {
    for (int i = 0; i < request_expression->request_positions_nb; i++)
    {
      struct request_position *request_position = request_positions + i;

      int pos_rp1 = request_position->start;
      int pos_rp2 = request_position->start + request_position->length;

      int distance = NlpRequestAnyDistanceFromPositions(ctrl_nlp_th, request_any, pos_rp1, pos_rp2);
      IFE(distance);
      if (minimum_distance > distance) minimum_distance = distance;
    }
  }
  return (minimum_distance);
}

static og_status NlpRequestAnyDistanceFromPositions(og_nlp_th ctrl_nlp_th, struct request_any *request_any,
    int position1, int position2)
{
  int minimum_distance = 0xfffffff;

  for (GList * rw_iter = request_any->queue_request_words->head; rw_iter; rw_iter = rw_iter->next)
  {
    struct request_word *rw = rw_iter->data;

    int pos_rw1 = rw->start_position;
    int pos_rw2 = rw->start_position + rw->length_position;

    int distance = abs(position1 - pos_rw1);
    if (distance > abs(position1 - pos_rw2)) distance = abs(position1 - pos_rw2);
    if (distance > abs(position2 - pos_rw1)) distance = abs(position2 - pos_rw1);
    if (distance > abs(position2 - pos_rw2)) distance = abs(position2 - pos_rw2);

    if (minimum_distance > distance) minimum_distance = distance;
  }
  return (minimum_distance);
}



#else

static og_status NlpRequestAnyDistance(og_nlp_th ctrl_nlp_th, struct request_any *request_any, struct request_expression *request_expression)
{
  struct request_position *request_positions = OgHeapGetCell(ctrl_nlp_th->hrequest_position,
      request_expression->request_position_start);
  IFN(request_positions) DPcErr;

  int minimum_distance = 0xfffffff;
  for (int i = 0; i < request_expression->request_positions_nb; i++)
  {
    struct request_position *request_position = request_positions + i;

    for (GList * rw_iter = request_any->queue_request_words->head; rw_iter; rw_iter = rw_iter->next)
    {
      struct request_word *rw = rw_iter->data;

      int pos_rp1 = request_position->start;
      int pos_rp2 = request_position->start + request_position->length;

      int pos_rw1 = rw->start_position;
      int pos_rw2 = rw->start_position + rw->length_position;

      int distance = abs(pos_rp1 - pos_rw1);
      if (distance > abs(pos_rp1 - pos_rw2)) distance = abs(pos_rp1 - pos_rw2);
      if (distance > abs(pos_rp2 - pos_rw1)) distance = abs(pos_rp2 - pos_rw1);
      if (distance > abs(pos_rp2 - pos_rw2)) distance = abs(pos_rp2 - pos_rw2);

      if (minimum_distance > distance) minimum_distance = distance;
    }

  }
  return (minimum_distance);
}
#endif

static og_status NlpRequestAnyIsOrdered(og_nlp_th ctrl_nlp_th, struct request_any *request_any,
    struct request_expression *request_expression)
{
  og_bool is_ordered = NlpRequestAnyIsOrdered1(ctrl_nlp_th, request_any, request_expression);
  IFE(is_ordered);
  if (!is_ordered) return FALSE;

  og_bool global_is_ordered = TRUE;

  IFE(NlpRequestAnyAddPositions(ctrl_nlp_th, request_any, request_expression));

  if (ctrl_nlp_th->loginfo->trace & DOgNlpTraceMatch)
  {
    NlpLog(DOgNlpTraceMatch, "NlpRequestAnyIsOrdered: expression where any is found:")
    IFE(NlpRequestExpressionLog(ctrl_nlp_th, request_expression, 2));
    NlpLog(DOgNlpTraceMatch, "NlpRequestAnyIsOrdered: any to check:")
    NlpRequestExpressionAnyLog(ctrl_nlp_th, request_any);
  }

  struct request_expression *sub_re = NULL;
  struct request_expression *re = request_expression;
  while (re->Isuper_request_expression >= 0)
  {
    sub_re = re;
    re = OgHeapGetCell(ctrl_nlp_th->hrequest_expression, re->Isuper_request_expression);
    IFN(re) DPcErr;

    IFE(NlpRequestAnyAddPositions(ctrl_nlp_th, request_any, re));
    IFE(NlpRequestAnyAddPositionsInputPart(ctrl_nlp_th, sub_re, re));

    if (ctrl_nlp_th->loginfo->trace & DOgNlpTraceMatch)
    {
      NlpLog(DOgNlpTraceMatch, "NlpRequestAnyIsOrdered: checking order of expression:")
      IFE(NlpInterpretTreeLog(ctrl_nlp_th, re, 0));
    }
    if (re->expression->keep_order)
    {
      is_ordered = NlpRequestExpressionIsOrdered(ctrl_nlp_th, re);
      IFE(is_ordered);
    }
    else is_ordered = TRUE;
    if (!is_ordered)
    {
      global_is_ordered = FALSE;
      break;
    }
  }

  // cleaning positions
  re = request_expression;
  IFE(NlpRequestAnyDelPositions(ctrl_nlp_th, re));
  while (re->Isuper_request_expression >= 0)
  {
    sub_re = re;
    re = OgHeapGetCell(ctrl_nlp_th->hrequest_expression, re->Isuper_request_expression);
    IFN(re) DPcErr;

    if (re->expression->keep_order)
    {
      is_ordered = NlpRequestExpressionIsOrdered(ctrl_nlp_th, re);
      IFE(is_ordered);
    }
    else is_ordered = TRUE;
    IFE(NlpRequestAnyDelPositionsInputPart(ctrl_nlp_th, sub_re, re));
    IFE(NlpRequestAnyDelPositions(ctrl_nlp_th, re));
    if (!is_ordered) break;
  }

  if (ctrl_nlp_th->loginfo->trace & DOgNlpTraceMatch)
  {
    NlpLog(DOgNlpTraceMatch, "NlpRequestAnyIsOrdered: expression is %s.",
        (global_is_ordered ? "ordered" : "not ordered"))
  }
  return global_is_ordered;
}

static og_status NlpRequestAnyIsOrdered1(og_nlp_th ctrl_nlp_th, struct request_any *request_any,
    struct request_expression *request_expression)
{
  if (!request_expression->expression->keep_order) return TRUE;

  int Ialias = 0;
  for (int i = 0; i < request_expression->orips_nb; i++)
  {
    struct request_input_part *request_input_part = NlpGetRequestInputPart(ctrl_nlp_th, request_expression, i);
    IFN(request_input_part) DPcErr;
    if (request_input_part->type == nlp_input_part_type_Interpretation)
    {
      Ialias++;
      if (request_expression->expression->alias_any_input_part_position == Ialias)
      {
        struct request_position *request_position = OgHeapGetCell(ctrl_nlp_th->hrequest_position,
            request_input_part->request_position_start + request_input_part->request_positions_nb - 1);
        IFN(request_position) DPcErr;
        struct request_word *first_request_word = request_any->queue_request_words->head->data;
        if (request_position->start + request_position->length <= first_request_word->start_position)
        {
          if (i + 1 < request_expression->orips_nb)
          {
            struct request_input_part *request_input_part_next = NlpGetRequestInputPart(ctrl_nlp_th, request_expression,
                i + 1);
            IFN(request_input_part_next) DPcErr;
            if (request_input_part_next->type == nlp_input_part_type_Interpretation)
            {
              struct request_position *request_position_next = OgHeapGetCell(ctrl_nlp_th->hrequest_position,
                  request_input_part_next->request_position_start);
              IFN(request_position) DPcErr;

              struct request_word *request_word_last = request_any->queue_request_words->tail->data;

              if (request_word_last->start_position + request_word_last->length_position
                  <= request_position_next->start) return TRUE;
              else return FALSE;
            }
            else return TRUE;

          }
          else return TRUE;
        }
        else return FALSE;
      }
    }
  }
  return TRUE;
}

static og_status NlpRequestAnyAddPositions(og_nlp_th ctrl_nlp_th, struct request_any *request_any,
    struct request_expression *request_expression)
{

  request_expression->safe_request_position_start = request_expression->request_position_start;
  request_expression->safe_request_positions_nb = request_expression->request_positions_nb;

  int nb_request_positions = request_expression->request_positions_nb + request_any->queue_request_words->length;

  // pre-allocation to avoid reallocation
  int request_position_start = OgHeapAddCells(ctrl_nlp_th->hrequest_position, nb_request_positions);
  IFE(request_position_start);

  struct request_position *request_positions = OgHeapGetCell(ctrl_nlp_th->hrequest_position, 0);
  IFN(request_positions) DPcErr;

  struct request_position *request_position_from = request_positions + request_expression->request_position_start;
  struct request_position *request_position_to = request_positions + request_position_start;
  memcpy(request_position_to, request_position_from,
      request_expression->request_positions_nb * sizeof(struct request_position));

  request_position_from = request_positions + request_expression->request_position_start
      + request_expression->request_positions_nb;
  request_position_to = request_positions + request_position_start + request_expression->request_positions_nb;

  int i = 0;
  for (GList *iter = request_any->queue_request_words->head; iter; iter = iter->next, i++)
  {
    struct request_word *rw = iter->data;

    request_position_to[i].start = rw->start_position;
    request_position_to[i].length = rw->length_position;
  }

  request_expression->request_position_start = request_position_start;
  request_expression->request_positions_nb = nb_request_positions;
  IF(NlpRequestPositionSort(ctrl_nlp_th, request_expression->request_position_start, request_expression->request_positions_nb))
  {
    DPcErr;
  }

  DONE;
}

static og_status NlpRequestAnyDelPositions(og_nlp_th ctrl_nlp_th, struct request_expression *request_expression)
{
  request_expression->request_position_start = request_expression->safe_request_position_start;
  request_expression->request_positions_nb = request_expression->safe_request_positions_nb;
  request_expression->safe_request_position_start = (-1);
  request_expression->safe_request_positions_nb = 0;
  DONE;
}

static og_status NlpRequestAnyAddPositionsInputPart(og_nlp_th ctrl_nlp_th,
    struct request_expression *sub_request_expression, struct request_expression *request_expression)
{
  for (int i = 0; i < request_expression->orips_nb; i++)
  {
    struct request_input_part *request_input_part = NlpGetRequestInputPart(ctrl_nlp_th, request_expression, i);
    IFN(request_input_part) DPcErr;

    if (request_input_part->type == nlp_input_part_type_Interpretation)
    {
      if (request_input_part->Irequest_expression == sub_request_expression->self_index)
      {
        request_input_part->safe_request_position_start = request_input_part->request_position_start;
        request_input_part->safe_request_positions_nb = request_input_part->request_positions_nb;

        request_input_part->request_position_start = sub_request_expression->request_position_start;
        request_input_part->request_positions_nb = sub_request_expression->request_positions_nb;
      }
    }
  }
  DONE;
}

static og_status NlpRequestAnyDelPositionsInputPart(og_nlp_th ctrl_nlp_th,
    struct request_expression *sub_request_expression, struct request_expression *request_expression)
{
  for (int i = 0; i < request_expression->orips_nb; i++)
  {
    struct request_input_part *request_input_part = NlpGetRequestInputPart(ctrl_nlp_th, request_expression, i);
    IFN(request_input_part) DPcErr;

    if (request_input_part->type == nlp_input_part_type_Interpretation)
    {
      if (request_input_part->Irequest_expression == sub_request_expression->self_index)
      {
        request_input_part->request_position_start = request_input_part->safe_request_position_start;
        request_input_part->request_positions_nb = request_input_part->safe_request_positions_nb;
        request_input_part->safe_request_position_start = (-1);
        request_input_part->safe_request_positions_nb = 0;
      }
    }
  }
  DONE;
}

og_status NlpRequestAnyAddRequestExpression(og_nlp_th ctrl_nlp_th, struct request_any *request_any,
    struct request_expression *request_expression)
{
  g_queue_push_tail(request_any->queue_request_expression, GINT_TO_POINTER(request_expression->self_index));
  DONE;
}

og_status NlpSetNbAnys(og_nlp_th ctrl_nlp_th, struct request_expression *request_expression)
{
  request_expression->nb_anys = 0;
  request_expression->nb_anys_attached = 0;
  for (int i = 0; i < request_expression->orips_nb; i++)
  {
    struct request_input_part *request_input_part = NlpGetRequestInputPart(ctrl_nlp_th, request_expression, i);
    IFN(request_input_part) DPcErr;

    if (request_input_part->type == nlp_input_part_type_Word) ;
    else if (request_input_part->type == nlp_input_part_type_Interpretation)
    {
      struct request_expression *sub_request_expression = OgHeapGetCell(ctrl_nlp_th->hrequest_expression,
          request_input_part->Irequest_expression);
      IFN(sub_request_expression) DPcErr;
      request_expression->nb_anys += sub_request_expression->nb_anys;
    }
  }

  if (request_expression->expression->alias_any_input_part_position >= 0)
  {
    request_expression->nb_anys++;
  }
  DONE;
}

og_status NlpGetNbAnysAttached(og_nlp_th ctrl_nlp_th, struct request_expression *request_expression)
{
  request_expression->nb_anys_attached = 0;
  if (request_expression->request_any_start < 0) DONE;

  struct request_any *request_anys = OgHeapGetCell(ctrl_nlp_th->hrequest_any, request_expression->request_any_start);
  for (int i = 0; i < request_expression->request_anys_nb; i++)
  {
    struct request_any *request_any = request_anys + i;
    if (request_any->queue_request_expression->length > 0) request_expression->nb_anys_attached++;
  }
  DONE;
}

int NlpRequestExpressionAnysLog(og_nlp_th ctrl_nlp_th, struct request_expression *request_expression)
{
  if (request_expression->request_anys_nb == 0)
  {
    OgMsg(ctrl_nlp_th->hmsg, "", DOgMsgDestInLog, "No any for expression");
    DONE;
  }
  OgMsg(ctrl_nlp_th->hmsg, "", DOgMsgDestInLog, "List of possible any for expression:");

  if (request_expression->request_any_start >= 0)
  {
    struct request_any *request_any = OgHeapGetCell(ctrl_nlp_th->hrequest_any, request_expression->request_any_start);
    for (int i = 0; i < request_expression->request_anys_nb; i++)
    {
      IFE(NlpRequestExpressionAnyLog(ctrl_nlp_th, request_any + i));
    }
  }

  DONE;
}

int NlpRequestExpressionAnyLog(og_nlp_th ctrl_nlp_th, struct request_any *request_any)
{
  char string_any[DPcPathSize];
  NlpRequestAnyString(ctrl_nlp_th, request_any, DPcPathSize, string_any);

  char string_any_position[DPcPathSize];
  NlpRequestAnyPositionString(ctrl_nlp_th, request_any, DPcPathSize, string_any_position);

  char string_request_expression[DPcPathSize];
  NlpRequestAnyRequestExpressionString(ctrl_nlp_th, request_any, DPcPathSize, string_request_expression);

  char highlight[DPcPathSize];
  NlpRequestAnyStringPretty(ctrl_nlp_th, request_any, DPcPathSize, highlight);

  OgMsg(ctrl_nlp_th->hmsg, "", DOgMsgDestInLog, " '%s' [%s] -> [%s]: '%s', distance=%d", string_any,
      string_any_position, string_request_expression, highlight, request_any->distance);
  DONE;
}

static int NlpRequestAnyRequestExpressionString(og_nlp_th ctrl_nlp_th, struct request_any *request_any, int size,
    char *string)
{
  int length = 0;
  string[length] = 0;

  for (GList *iter = request_any->queue_request_expression->head; iter; iter = iter->next)
  {
    int Irequest_expression = GPOINTER_TO_INT(iter->data);
    length = strlen(string);
    snprintf(string + length, size - length, "%s%d", (iter != request_any->queue_request_expression->head ? " " : ""),
        Irequest_expression);
  }

  DONE;
}

int NlpRequestAnyString(og_nlp_th ctrl_nlp_th, struct request_any *request_any, int size, char *string)
{
  og_string request_sentence = ctrl_nlp_th->request_sentence;

  int start_position = -1;
  int end_position = -1;

  for (GList *iter = request_any->queue_request_words->head; iter; iter = iter->next)
  {
    struct request_word *rw = iter->data;

    if (start_position < 0)
    {
      start_position = rw->start_position;
    }
    end_position = rw->start_position + rw->length_position;
  }

  int length = end_position - start_position;

  if (start_position < 0 || length < 0)
  {
    start_position = 0;
    length = 0;
  }

  snprintf(string, size, "%.*s", length, request_sentence + start_position);

  DONE;
}

int NlpRequestAnyPositionString(og_nlp_th ctrl_nlp_th, struct request_any *request_any, int size, char *string)
{
  int length = 0;
  string[length] = 0;

  og_bool is_first = TRUE;
  for (GList *iter = request_any->queue_request_words->head; iter; iter = iter->next)
  {
    struct request_word *rw = iter->data;

    length = strlen(string);
    snprintf(string + length, size - length, "%s%d:%d", (is_first ? "" : " "), rw->start_position,
        rw->length_position);

    is_first = FALSE;
  }

  DONE;
}

int NlpRequestAnyStringPretty(og_nlp_th ctrl_nlp_th, struct request_any *request_any, int size, char *string)
{
  og_string s = ctrl_nlp_th->request_sentence;
  int is = strlen(s);

  int position = 0;

  int length = 0;
  string[length] = 0;

  for (GList *iter = request_any->queue_request_words->head; iter; iter = iter->next)
  {
    struct request_word *rw = iter->data;

    if (position < rw->start_position)
    {
      length = strlen(string);
      snprintf(string + length, size - length, "%.*s", rw->start_position - position, s + position);
      position = rw->start_position;

    }

    length = strlen(string);
    snprintf(string + length, size - length, "[%.*s]", rw->length_position, s + rw->start_position);
    position = rw->start_position + rw->length_position;
  }

  if (position < is)
  {
    length = strlen(string);
    snprintf(string + length, size - length, "%.*s", is - position, s + position);
    position = is;
  }

  DONE;
}

