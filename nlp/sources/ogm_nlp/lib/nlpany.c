/*
 *  Handling log messages for Natural Language Processing library
 *  Copyright (c) 2017 Pertimm, by Patrick Constant
 *  Dev : September 2017
 *  Version 1.0
 */
#include "ogm_nlp.h"

static og_status NlpRequestAnyAdd(og_nlp_th ctrl_nlp_th, struct request_expression *request_expression,
    int Irequest_position_before);
static og_status NlpRequestAnyDistance(og_nlp_th ctrl_nlp_th, struct request_expression *request_expression,
    struct request_any *request_any);
og_status NlpRequestAnyIsOrdered(og_nlp_th ctrl_nlp_th, struct request_any *request_any,
    struct request_expression *request_expression);
static int NlpRequestAnyRequestExpressionString(og_nlp_th ctrl_nlp_th, struct request_any *request_any, int size,
    char *string);

og_status NlpInterpretAnyFlush(og_nlp_th ctrl_nlp_th)
{
  return NlpInterpretAnyReset(ctrl_nlp_th);
}

og_status NlpInterpretAnyReset(og_nlp_th ctrl_nlp_th)
{
  int request_any_used = OgHeapGetCellsUsed(ctrl_nlp_th->hrequest_any);
  struct request_any *request_any = OgHeapGetCell(ctrl_nlp_th->hrequest_any, 0);
  IFN(request_any) DPcErr;
  for (int i = 0; i < request_any_used; i++)
  {
    g_queue_clear(request_any->queue_request_expression);
  }
  DONE;
}

og_status NlpRequestAnysAdd(og_nlp_th ctrl_nlp_th, struct request_expression *request_expression)
{
  struct request_position *request_position = OgHeapGetCell(ctrl_nlp_th->hrequest_position,
      request_expression->request_position_start);
  IFN(request_position) DPcErr;

  request_expression->request_any_start = -1;
  request_expression->request_anys_nb = 0;

  for (int i = 0; i < request_expression->request_positions_nb; i++)
  {
    IFE(NlpRequestAnyAdd(ctrl_nlp_th, request_expression, request_expression->request_position_start + i));
  }
  DONE;
}

static og_status NlpRequestAnyAdd(og_nlp_th ctrl_nlp_th, struct request_expression *request_expression,
    int Irequest_position_before)
{
  struct request_position *request_position_before = OgHeapGetCell(ctrl_nlp_th->hrequest_position,
      Irequest_position_before);
  IFN(request_position_before) DPcErr;
  struct request_position *request_position_after;
  struct request_position request_position_after_container[1];
  if (Irequest_position_before
      == request_expression->request_position_start + request_expression->request_positions_nb - 1)
  {
    request_position_after = request_position_after_container;
    request_position_after->start = 0xfffffff;
  }
  else
  {
    request_position_after = request_position_before + 1;
  }

  struct request_word *request_word = OgHeapGetCell(ctrl_nlp_th->hrequest_word, 0);
  IFN(request_word) DPcErr;

  for (int i = 0; i < ctrl_nlp_th->basic_request_word_used; i++)
  {
    if (request_word[i].start_position < request_position_before->start + request_position_before->length) continue;
    int j = i;
    for (; j < ctrl_nlp_th->basic_request_word_used; j++)
    {
      if (request_word[j].start_position + request_word[j].length_position > request_position_after->start) break;
    }
    if (j == i) continue;
    size_t Irequest_any;
    struct request_any *request_any = OgHeapNewCell(ctrl_nlp_th->hrequest_any, &Irequest_any);
    IFn(request_any) DPcErr;

    request_any->request_word_start = i;
    request_any->request_words_nb = j - i;

    g_queue_init(request_any->queue_request_expression);
    request_any->consumed = 0;

    if (request_expression->request_anys_nb == 0)
    {
      request_expression->request_any_start = Irequest_any;
    }
    request_expression->request_anys_nb++;
    i = j;
  }

  DONE;
}

og_status NlpRequestAnyAddClosest(og_nlp_th ctrl_nlp_th, struct request_expression *root_request_expression,
    struct request_expression *request_expression)
{
  struct request_any *request_any = OgHeapGetCell(ctrl_nlp_th->hrequest_any, 0);
  IFN(request_any) DPcErr;

  int minimum_distance = 0xfffffff;
  for (int i = 0; i < root_request_expression->request_anys_nb; i++)
  {
    int Irequest_any = root_request_expression->request_any_start + i;
    int distance = NlpRequestAnyDistance(ctrl_nlp_th, request_expression, request_any + Irequest_any);
    IFE(distance);
    if (minimum_distance > distance)
    {
      minimum_distance = distance;
    }
  }

  for (int i = 0; i < root_request_expression->request_anys_nb; i++)
  {
    int Irequest_any = root_request_expression->request_any_start + i;
    int distance = NlpRequestAnyDistance(ctrl_nlp_th, request_expression, request_any + Irequest_any);
    IFE(distance);
    if (minimum_distance == distance)
    {
      request_any[Irequest_any].distance = distance;
      if (NlpRequestAnyIsOrdered(ctrl_nlp_th, request_any + Irequest_any, request_expression))
      {
        IFE(NlpRequestAnyAddRequestExpression(ctrl_nlp_th, request_any + Irequest_any, request_expression));
      }
    }
  }

  DONE;
}

static og_status NlpRequestAnyDistance(og_nlp_th ctrl_nlp_th, struct request_expression *request_expression,
    struct request_any *request_any)
{
  struct request_position *request_position = OgHeapGetCell(ctrl_nlp_th->hrequest_position,
      request_expression->request_position_start);
  IFN(request_position) DPcErr;
  struct request_word *request_word = OgHeapGetCell(ctrl_nlp_th->hrequest_word, request_any->request_word_start);

  int minimum_distance = 0xfffffff;
  for (int i = 0; i < request_expression->request_positions_nb; i++)
  {
    for (int j = 0; j < request_any->request_words_nb; j++)
    {
      int pos_rp1 = request_position[i].start;
      int pos_rp2 = request_position[i].start + request_position[i].length;

      int pos_rw1 = request_word[j].start_position;
      int pos_rw2 = request_word[j].start_position + request_word[j].length_position;

      int distance = abs(pos_rp1 - pos_rw1);
      if (distance > abs(pos_rp1 - pos_rw2)) distance = abs(pos_rp1 - pos_rw2);
      if (distance > abs(pos_rp2 - pos_rw1)) distance = abs(pos_rp2 - pos_rw1);
      if (distance > abs(pos_rp2 - pos_rw2)) distance = abs(pos_rp2 - pos_rw2);

      if (minimum_distance > distance) minimum_distance = distance;
    }

  }
  return (minimum_distance);
}

og_status NlpRequestAnyIsOrdered(og_nlp_th ctrl_nlp_th, struct request_any *request_any,
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
        struct request_word *request_word = OgHeapGetCell(ctrl_nlp_th->hrequest_word, request_any->request_word_start);
        IFN(request_word) DPcErr;
        if (request_position->start + request_position->length < request_word->start_position)
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
              struct request_word *request_word_last = OgHeapGetCell(ctrl_nlp_th->hrequest_word,
                  request_any->request_word_start + request_any->request_words_nb - 1);
              IFN(request_word_last) DPcErr;
              if (request_word_last->start_position + request_word_last->length_position < request_position_next->start) return TRUE;
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

og_status NlpRequestAnyAddRequestExpression(og_nlp_th ctrl_nlp_th, struct request_any *request_any,
    struct request_expression *request_expression)
{
  g_queue_push_tail(request_any->queue_request_expression, GINT_TO_POINTER(request_expression->self_index));
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

  struct request_any *request_any = OgHeapGetCell(ctrl_nlp_th->hrequest_any, request_expression->request_any_start);

  for (int i = 0; i < request_expression->request_anys_nb; i++)
  {
    char string_any[DPcPathSize];
    NlpRequestAnyString(ctrl_nlp_th, request_any + i, DPcPathSize, string_any);

    char string_any_position[DPcPathSize];
    NlpRequestAnyPositionString(ctrl_nlp_th, request_any + i, DPcPathSize, string_any_position);

    char string_request_expression[DPcPathSize];
    NlpRequestAnyRequestExpressionString(ctrl_nlp_th, request_any + i, DPcPathSize, string_request_expression);

    char highlight[DPcPathSize];
    NlpRequestAnyStringPretty(ctrl_nlp_th, request_any + i, DPcPathSize, highlight);

    OgMsg(ctrl_nlp_th->hmsg, "", DOgMsgDestInLog, " '%s' [%s] -> [%s]: '%s', distance=%d", string_any,
        string_any_position, string_request_expression, highlight, request_any->distance);
  }

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
  struct request_word *request_word = OgHeapGetCell(ctrl_nlp_th->hrequest_word, 0);
  IFN(request_word) DPcErr;

  int length = 0;
  string[length] = 0;
  for (int i = 0; i < request_any->request_words_nb; i++)
  {
    int Irequest_word = request_any->request_word_start + i;
    og_string string_request_word = OgHeapGetCell(ctrl_nlp_th->hba, request_word[Irequest_word].raw_start);
    IFN(string_request_word) DPcErr;

    length = strlen(string);
    snprintf(string + length, size - length, "%s%s", (i ? " " : ""), string_request_word);
  }

  DONE;
}

int NlpRequestAnyPositionString(og_nlp_th ctrl_nlp_th, struct request_any *request_any, int size, char *string)
{
  struct request_word *request_word = OgHeapGetCell(ctrl_nlp_th->hrequest_word, 0);
  IFN(request_word) DPcErr;

  int length = 0;
  string[length] = 0;
  for (int i = 0; i < request_any->request_words_nb; i++)
  {
    length = strlen(string);
    snprintf(string + length, size - length, "%s%d:%d", (i ? " " : ""),
        request_word[request_any->request_word_start + i].start_position,
        request_word[request_any->request_word_start + i].length_position);
  }

  DONE;
}

int NlpRequestAnyStringPretty(og_nlp_th ctrl_nlp_th, struct request_any *request_any, int size, char *string)
{
  struct request_word *request_word = OgHeapGetCell(ctrl_nlp_th->hrequest_word, 0);
  IFN(request_word) DPcErr;

  og_string s = ctrl_nlp_th->request_sentence;
  int is = strlen(s);

  int position = 0;

  int length = 0;
  string[length] = 0;
  for (int i = 0; i < request_any->request_words_nb; i++)
  {
    int irw = request_any->request_word_start + i;
    if (position < request_word[irw].start_position)
    {
      length = strlen(string);
      snprintf(string + length, size - length, "%.*s", request_word[irw].start_position - position, s + position);
      position = request_word[irw].start_position;

    }
    length = strlen(string);
    snprintf(string + length, size - length, "[%.*s]", request_word[irw].length_position,
        s + request_word[irw].start_position);
    position = request_word[irw].start_position + request_word[irw].length_position;
  }

  if (position < is)
  {
    length = strlen(string);
    snprintf(string + length, size - length, "%.*s", is - position, s + position);
    position = is;
  }

  DONE;
}

