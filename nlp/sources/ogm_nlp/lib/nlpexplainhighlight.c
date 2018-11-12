/*
 *  Handling explanation for highlighted words
 *  Copyright (c) 2018 Pertimm, by Patrick Constant
 *  Dev : November 2018
 *  Version 1.0
 */
#include "ogm_nlp.h"

static og_status NlpExplainHighlightJson(og_nlp_th ctrl_nlp_th, struct request_expression *request_expression,
    json_t *json_explanation);
static og_status NlpExplainHighlightAddBasicWord(og_nlp_th ctrl_nlp_th, int Irequest_word);
static og_status NlpExplainHighlightSort(og_nlp_th ctrl_nlp_th);
static int NlpExplainHighlightCmp(gconstpointer ptr_highlight_word1, gconstpointer ptr_highlight_word2,
    gpointer user_data);
static og_status NlpInterpretTreeMatchJson(og_nlp_th ctrl_nlp_th, struct request_expression *request_expression,
    json_t *json_expression);
static og_status NlpInterpretTreeMatchJsonRecursive(og_nlp_th ctrl_nlp_th,
    struct request_expression *request_expression, json_t *json_expression, og_bool is_root);
static og_status NlpInterpretTreeMatchJsonExpression(og_nlp_th ctrl_nlp_th,
    struct request_expression *request_expression, json_t *json_expression);

og_status NlpExplainHighlightInit(og_nlp_th ctrl_nlp_th, og_string name)
{
  og_char_buffer nlpc_name[DPcPathSize];
  snprintf(nlpc_name, DPcPathSize, "%s_highlight_word", name);
  ctrl_nlp_th->hhighlight_word = OgHeapInit(ctrl_nlp_th->hmsg, nlpc_name, sizeof(struct highlight_word), 1);
  IFN(ctrl_nlp_th->hhighlight_word)
  {
    NlpThrowErrorTh(ctrl_nlp_th, "NlpExplainHighlightInit : error on OgHeapInit(%s)", nlpc_name);
    DPcErr;
  }
  DONE;
}

og_status NlpExplainHighlightReset(og_nlp_th ctrl_nlp_th)
{
  IFE(OgHeapReset(ctrl_nlp_th->hhighlight_word));
  DONE;
}

og_status NlpExplainHighlightFlush(og_nlp_th ctrl_nlp_th)
{
  IFE(OgHeapFlush(ctrl_nlp_th->hhighlight_word));
  DONE;
}

og_status NlpExplainHighlightAddWord(og_nlp_th ctrl_nlp_th, struct request_expression *request_expression,
    struct request_word *request_word, struct request_any *request_any)
{
  struct highlight_word highlight_word[1];
  highlight_word->request_expression = request_expression;

  IFN(request_any)
  {
    highlight_word->is_any = FALSE;
    highlight_word->request_word = request_word;
  }
  else
  {
    highlight_word->is_any = TRUE;
    highlight_word->request_any = request_any;
  }

  IFE(OgHeapAppend(ctrl_nlp_th->hhighlight_word, 1, highlight_word));
  DONE;
}

og_status NlpExplainHighlight(og_nlp_th ctrl_nlp_th, struct request_expression *request_expression,
    json_t *json_explanation)
{
  if (ctrl_nlp_th->basic_request_word_used <= 0) DONE;
  int nb_basic_request_word = ctrl_nlp_th->basic_request_word_used;
  for (int i = 0; i < nb_basic_request_word; i++)
  {
    IFE(NlpExplainHighlightAddBasicWord(ctrl_nlp_th, i));
  }
  IFE(NlpExplainHighlightSort(ctrl_nlp_th));
  IFE(NlpExplainHighlightLog(ctrl_nlp_th));

  IFE(NlpExplainHighlightJson(ctrl_nlp_th, request_expression, json_explanation));
  DONE;
}

static og_status NlpExplainHighlightJson(og_nlp_th ctrl_nlp_th, struct request_expression *request_expression,
    json_t *json_explanation)
{
  og_char_buffer scores[DPcPathSize];
  struct request_score *score = request_expression->score;
  sprintf(scores, "cov:%.2f loc:%.2f spell:%.2f olap:%.2f any:%.2f ctx:%0.2f scope:%.2f", score->coverage,
      score->locale, score->spelling, score->overlap, score->any, score->context, score->scope);

  json_t *json_expression_scores = json_string(scores);
  IF(json_object_set_new(json_explanation, "scores", json_expression_scores))
  {
    NlpThrowErrorTh(ctrl_nlp_th, "NlpExplainHighlightJson: error setting json_expression_scores");
    DPcErr;
  }

  json_t *json_highlight = json_object();
  char highlight[DPcPathSize];
  NlpRequestPositionStringHighlight(ctrl_nlp_th, request_expression->request_position_start,
      request_expression->request_positions_nb, DPcPathSize, highlight);
  json_t *json_expression_highlight = json_string(highlight);
  IF(json_object_set_new(json_highlight, "summary", json_expression_highlight))
  {
    NlpThrowErrorTh(ctrl_nlp_th, "NlpExplainHighlightJson: error setting json_expression_highlight");
    DPcErr;
  }
  IF(json_object_set_new(json_explanation, "highlight", json_highlight))
  {
    NlpThrowErrorTh(ctrl_nlp_th, "NlpExplainHighlightJson: error setting json_expression_highlight");
    DPcErr;
  }

  json_t *json_words = json_array();
  int highlight_word_used = OgHeapGetCellsUsed(ctrl_nlp_th->hhighlight_word);
  struct highlight_word *highlight_words = OgHeapGetCell(ctrl_nlp_th->hhighlight_word, 0);
  IFN(highlight_words) DPcErr;
  for (int i = 0; i < highlight_word_used; i++)
  {
    struct highlight_word *highlight_word = highlight_words + i;
    json_t *json_word = json_object();
    og_string string_highlight_word = NULL;
    char string_any[DPcPathSize];

    if (highlight_word->is_any)
    {
      struct request_any *request_any = highlight_word->request_any;
      NlpRequestAnyString(ctrl_nlp_th, request_any, DPcPathSize, string_any);
      string_highlight_word = string_any;
      IF(json_object_set_new(json_word, "is_any", json_true()))
      {
        NlpThrowErrorTh(ctrl_nlp_th, "NlpExplainHighlightJson: error setting json_word is_any to true");
        DPcErr;
      }
    }
    else
    {
      struct request_word *request_word = highlight_word->request_word;
      string_highlight_word = OgHeapGetCell(ctrl_nlp_th->hba, request_word->start);
      IFN(string_highlight_word) DPcErr;
    }

    json_t *json_highlight_word_text = json_string(string_highlight_word);
    IF(json_object_set_new(json_word, "word", json_highlight_word_text))
    {
      NlpThrowErrorTh(ctrl_nlp_th, "NlpExplainHighlightJson: error setting json_highlight_word_text");
      DPcErr;
    }
    IF(json_array_append_new(json_words, json_word))
    {
      NlpThrowErrorTh(ctrl_nlp_th, "NlpExplainHighlightJson : error appending json_word");
      DPcErr;
    }

    IFN(highlight_word->request_expression)
    {
      IF(json_object_set_new(json_word, "match", json_null()))
      {
        NlpThrowErrorTh(ctrl_nlp_th, "NlpExplainHighlightJson: error setting json_word to null");
        DPcErr;
      }

    }
    else
    {
      IFE(NlpInterpretTreeMatchJson(ctrl_nlp_th, highlight_word->request_expression, json_word));
    }

  }

  IF(json_object_set_new(json_highlight, "words", json_words))
  {
    NlpThrowErrorTh(ctrl_nlp_th, "NlpExplainHighlightJson: error setting json_expression_highlight");
    DPcErr;
  }

  DONE;
}

static og_status NlpExplainHighlightAddBasicWord(og_nlp_th ctrl_nlp_th, int Irequest_word)
{
  struct request_word *request_word = OgHeapGetCell(ctrl_nlp_th->hrequest_word, Irequest_word);
  IFN(request_word) DPcErr;

  int highlight_word_used = OgHeapGetCellsUsed(ctrl_nlp_th->hhighlight_word);
  struct highlight_word *highlight_words = OgHeapGetCell(ctrl_nlp_th->hhighlight_word, 0);
  IFN(highlight_words) DPcErr;

  for (int i = 0; i < highlight_word_used; i++)
  {
    struct highlight_word *highlight_word = highlight_words + i;
    if (highlight_word->is_any)
    {
      struct request_word *first_request_word_any = highlight_word->request_any->queue_request_words->head->data;
      int start_position = first_request_word_any->start_position;

      struct request_word *last_request_word_any = highlight_word->request_any->queue_request_words->tail->data;
      int end_position = last_request_word_any->start_position + last_request_word_any->length_position;

      if (start_position <= request_word->start_position
          && request_word->start_position + request_word->length_position <= end_position)
      {
        DONE;
      }
    }
    else
    {
      if (highlight_word->request_word->start_position <= request_word->start_position
          && request_word->start_position + request_word->length_position
              <= highlight_word->request_word->start_position + highlight_word->request_word->length_position)
      {
        DONE;
      }
    }

  }
  IFE(NlpExplainHighlightAddWord(ctrl_nlp_th, NULL, request_word, NULL));

  DONE;
}

static og_status NlpExplainHighlightSort(og_nlp_th ctrl_nlp_th)
{
  int highlight_word_used = OgHeapGetCellsUsed(ctrl_nlp_th->hhighlight_word);
  struct highlight_word *highlight_words = OgHeapGetCell(ctrl_nlp_th->hhighlight_word, 0);
  IFN(highlight_words) DPcErr;

  g_qsort_with_data(highlight_words, highlight_word_used, sizeof(struct highlight_word), NlpExplainHighlightCmp,
      ctrl_nlp_th);
  DONE;
}

static int NlpExplainHighlightCmp(gconstpointer ptr_highlight_word1, gconstpointer ptr_highlight_word2,
    gpointer user_data)
{
  struct highlight_word *highlight_word1 = (struct highlight_word *) ptr_highlight_word1;
  struct highlight_word *highlight_word2 = (struct highlight_word *) ptr_highlight_word2;

  int start_position1;
  if (highlight_word1->is_any)
  {
    struct request_word *first_request_word_any = highlight_word1->request_any->queue_request_words->head->data;
    start_position1 = first_request_word_any->start_position;
  }
  else
  {
    start_position1 = highlight_word1->request_word->start_position;
  }

  int start_position2;
  if (highlight_word2->is_any)
  {
    struct request_word *first_request_word_any = highlight_word2->request_any->queue_request_words->head->data;
    start_position2 = first_request_word_any->start_position;
  }
  else
  {
    start_position2 = highlight_word2->request_word->start_position;
  }


  if (start_position1 != start_position2)
  {
    return (start_position1 - start_position2);
  }

  return (highlight_word1 - highlight_word2);
}

static og_status NlpInterpretTreeMatchJson(og_nlp_th ctrl_nlp_th, struct request_expression *request_expression,
    json_t *json_expression)
{
  IFE(NlpInterpretTreeMatchJsonRecursive(ctrl_nlp_th, request_expression, json_expression, TRUE));
  DONE;
}

static og_status NlpInterpretTreeMatchJsonRecursive(og_nlp_th ctrl_nlp_th,
    struct request_expression *request_expression, json_t *json_expression, og_bool is_root)
{
  og_string name = "parent";
  if (is_root) name = "match";

  if (request_expression->Isuper_request_expression < 0)
  {
    json_t *json_super_expression = json_object();
    IF(json_object_set_new(json_expression, name, json_super_expression))
    {
      NlpThrowErrorTh(ctrl_nlp_th, "NlpInterpretTreeMatchJsonExpression: error setting parent");
      DPcErr;
    }
    IFE(NlpInterpretTreeMatchJsonExpression(ctrl_nlp_th, request_expression, json_super_expression));
  }
  else
  {
    struct request_expression *super_request_expression;
    super_request_expression = OgHeapGetCell(ctrl_nlp_th->hrequest_expression,
        request_expression->Isuper_request_expression);
    IFN(super_request_expression) DPcErr;

    json_t *json_super_expression = json_object();
    IF(json_object_set_new(json_expression, name, json_super_expression))
    {
      NlpThrowErrorTh(ctrl_nlp_th, "NlpInterpretTreeMatchJsonExpression: error setting parent");
      DPcErr;
    }
    IFE(NlpInterpretTreeMatchJsonExpression(ctrl_nlp_th, request_expression, json_super_expression));

    IFE(NlpInterpretTreeMatchJsonRecursive(ctrl_nlp_th, super_request_expression, json_super_expression, FALSE));
  }
  DONE;
}

static og_status NlpInterpretTreeMatchJsonExpression(og_nlp_th ctrl_nlp_th,
    struct request_expression *request_expression, json_t *json_expression)
{
  int start_position, end_position, length_position;
  struct request_position *request_position1 = OgHeapGetCell(ctrl_nlp_th->hrequest_position,
      request_expression->request_position_start);
  IFn(request_position1) DPcErr;
  struct request_position *request_position2 = OgHeapGetCell(ctrl_nlp_th->hrequest_position,
      request_expression->request_position_start + request_expression->request_positions_nb - 1);
  IFn(request_position2) DPcErr;

  start_position = request_position1->start;
  end_position = request_position2->start + request_position2->length;
  length_position = end_position - start_position;

  og_string s = ctrl_nlp_th->request_sentence;
  unsigned char string_expression[DPcPathSize];
  snprintf(string_expression, DPcPathSize, "%.*s", length_position, s + start_position);

  IF(json_object_set_new(json_expression, "expression", json_string(string_expression)))
  {
    NlpThrowErrorTh(ctrl_nlp_th, "NlpInterpretTreeMatchJsonExpression: error setting expression");
    DPcErr;
  }
  IF(json_object_set_new(json_expression, "interpretation_slug", json_string(request_expression->expression->interpretation->slug)))
  {
    NlpThrowErrorTh(ctrl_nlp_th, "NlpInterpretTreeMatchJsonExpression: error setting interpretation_slug");
    DPcErr;
  }
  IF(json_object_set_new(json_expression, "interpretation_id", json_string(request_expression->expression->interpretation->id)))
  {
    NlpThrowErrorTh(ctrl_nlp_th, "NlpInterpretTreeMatchJsonExpression: error setting interpretation_id");
    DPcErr;
  }
  IFX(request_expression->expression->id)
  {
    IF(json_object_set_new(json_expression, "expression_id", json_string(request_expression->expression->id)))
    {
      NlpThrowErrorTh(ctrl_nlp_th, "NlpInterpretTreeMatchJsonExpression: error setting expression_id");
      DPcErr;
    }
  }
  DONE;
}

og_status NlpExplainHighlightLog(og_nlp_th ctrl_nlp_th)
{
  int highlight_word_used = OgHeapGetCellsUsed(ctrl_nlp_th->hhighlight_word);
  struct highlight_word *highlight_words = OgHeapGetCell(ctrl_nlp_th->hhighlight_word, 0);
  IFN(highlight_words) DPcErr;
  OgMsg(ctrl_nlp_th->hmsg, "", DOgMsgDestInLog, "NlpExplainHighlightLog: list of highlight words:");
  for (int i = 0; i < highlight_word_used; i++)
  {
    struct highlight_word *highlight_word = highlight_words + i;
    OgMsg(ctrl_nlp_th->hmsg, "", DOgMsgDestInLog, "highlight word %d%s:", i, ((highlight_word->is_any)?" (any)":""));
    if (highlight_word->is_any)
    {
      NlpRequestExpressionAnyLog(ctrl_nlp_th, highlight_word->request_any);
    }
    else
    {
      NlpLogRequestWord(ctrl_nlp_th, highlight_word->request_word);
    }
    IFX(highlight_word->request_expression)
    {
      NlpRequestExpressionLog(ctrl_nlp_th, highlight_word->request_expression, 0);
    }
  }
  DONE;
}

