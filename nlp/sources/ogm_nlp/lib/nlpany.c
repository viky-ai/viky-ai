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

  int request_word_used = OgHeapGetCellsUsed(ctrl_nlp_th->hrequest_word);
  struct request_word *request_word = OgHeapGetCell(ctrl_nlp_th->hrequest_word, 0);
  IFN(request_word) DPcErr;

  for (int i = 0; i < request_word_used; i++)
  {
    if (request_word[i].start < request_position_before->start + request_position_before->length) continue;
    int j = i;
    for (; j < request_word_used; j++)
    {
      if (request_word[j].start + request_word[j].length > request_position_after->start) break;
    }
    if (j == i) continue;
    size_t Irequest_any;
    struct request_any *request_any = OgHeapNewCell(ctrl_nlp_th->hrequest_any, &Irequest_any);
    IFn(request_any) DPcErr;
    request_any->request_word_start = i;
    request_any->request_words_nb = j - i;
    if (request_expression->request_anys_nb == 0)
    {
      request_expression->request_any_start = Irequest_any;
    }
    request_expression->request_anys_nb++;
    i = j;
  }

  DONE;
}

og_bool NlpRequestAnyGetClosest(og_nlp_th ctrl_nlp_th, struct request_expression *root_request_expression,
    struct request_expression *request_expression, int *pIrequest_any)
{
  struct request_any *request_any = OgHeapGetCell(ctrl_nlp_th->hrequest_any, 0);
  IFN(request_any) DPcErr;

  int Iclosest_request_any = (-1);
  int minimum_distance = 0xfffffff;
  for (int i = 0; i < root_request_expression->request_anys_nb; i++)
  {
    int Irequest_any = root_request_expression->request_any_start + i;
    int distance = NlpRequestAnyDistance(ctrl_nlp_th, request_expression, request_any + Irequest_any);
    IFE(distance);
    if (minimum_distance >= distance)
    {
      minimum_distance = distance;
      Iclosest_request_any = Irequest_any;
    }
  }

  *pIrequest_any = Iclosest_request_any;

  if (Iclosest_request_any >= 0) return TRUE;
  return FALSE;
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

      int pos_rw1 = request_word[j].start;
      int pos_rw2 = request_word[j].start + request_word[j].length;

      int distance = abs(pos_rp1 - pos_rw1);
      if (distance > abs(pos_rp1 - pos_rw2)) distance = abs(pos_rp1 - pos_rw2);
      if (distance > abs(pos_rp2 - pos_rw1)) distance = abs(pos_rp2 - pos_rw1);
      if (distance > abs(pos_rp2 - pos_rw2)) distance = abs(pos_rp2 - pos_rw2);

      if (minimum_distance > distance) minimum_distance = distance;
    }

  }
  return (minimum_distance);
}

int NlpRequestExpressionAnysLog(og_nlp_th ctrl_nlp_th, struct request_expression *request_expression)
{
  OgMsg(ctrl_nlp_th->hmsg, "", DOgMsgDestInLog, "list of possible any for expression:");

  struct request_any *request_any = OgHeapGetCell(ctrl_nlp_th->hrequest_any, 0);

  for (int i = 0; i < request_expression->request_anys_nb; i++)
  {
    char string_any[DPcPathSize];
    NlpRequestAnyString(ctrl_nlp_th, request_any + request_expression->request_any_start + i, DPcPathSize, string_any);
    OgMsg(ctrl_nlp_th->hmsg, "", DOgMsgDestInLog, " [%s]", string_any);
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
    length = strlen(string);
    snprintf(string + length, size - length, "%s%d:%d", (i ? " " : ""),
        request_word[request_any->request_word_start + i].start,
        request_word[request_any->request_word_start + i].length);
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
    if (position < request_word[irw].start)
    {
      length = strlen(string);
      snprintf(string + length, size - length, "%.*s",
          request_word[irw].start - position, s + position);
      position = request_word[irw].start;

    }
    length = strlen(string);
    snprintf(string + length, size - length, "[%.*s]", request_word[irw].length,
        s + request_word[irw].start);
    position = request_word[irw].start
        + request_word[irw].length;
  }

  if (position < is)
  {
    length = strlen(string);
    snprintf(string + length, size - length, "%.*s", is - position, s + position);
    position = is;
  }

  DONE;
}

