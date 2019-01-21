/*
 *  Handling locale which is the user langage preferences
 *  Copyright (c) 2017 Pertimm, by Patrick Constant
 *  Dev : November 2017
 *  Version 1.0
 */
#include "ogm_nlp.h"

static og_status NlpCalculateScoreRecursive(og_nlp_th ctrl_nlp_th, struct request_expression *root_request_expression,
    struct request_expression *request_expression);
static og_status NlpCalculateScoreGluePunctuations(og_nlp_th ctrl_nlp_th, struct request_expression *request_expression,
    struct request_score *score);
static og_status NlpCalculateTotalScore(og_nlp_th ctrl_nlp_th, struct request_expression *request_expression);
static og_status NlpCalculateScoreMatchScope(og_nlp_th ctrl_nlp_th, struct request_expression *request_expression);

og_status NlpCalculateScore(og_nlp_th ctrl_nlp_th, struct request_expression *request_expression)
{
  IFE(NlpCalculateScoreRecursive(ctrl_nlp_th, request_expression, request_expression));

  DONE;
}

static og_status NlpCalculateScoreRecursive(og_nlp_th ctrl_nlp_th, struct request_expression *root_request_expression,
    struct request_expression *request_expression)
{
  struct request_score score[1];
  memset(score, 0, sizeof(struct request_score));
  score->any = 1.0;
  int nb_words = ctrl_nlp_th->basic_group_request_word_nb;
  request_expression->nb_matched_words = 0;

  for (int i = 0; i < request_expression->orips_nb; i++)
  {
    struct request_input_part *request_input_part = NlpGetRequestInputPart(ctrl_nlp_th, request_expression, i);
    IFN(request_input_part) DPcErr;

    if (request_input_part->type == nlp_input_part_type_Word)
    {
      struct request_word *request_word = request_input_part->request_word;
      score->coverage += (double) request_word->nb_matched_words / nb_words;
      score->spelling += (double) request_word->nb_matched_words * request_word->spelling_score / nb_words;
      score->locale += (double) request_word->nb_matched_words / nb_words;
      request_expression->nb_matched_words += request_word->nb_matched_words;
    }

    else if (request_input_part->type == nlp_input_part_type_Interpretation)
    {
      struct request_expression *sub_request_expression = OgHeapGetCell(ctrl_nlp_th->hrequest_expression,
          request_input_part->Irequest_expression);
      IFN(sub_request_expression) DPcErr;
      IFE(NlpCalculateScoreRecursive(ctrl_nlp_th, root_request_expression, sub_request_expression));
      request_expression->nb_matched_words += sub_request_expression->nb_matched_words;
      score->locale += sub_request_expression->score->locale * sub_request_expression->score->coverage;
      score->spelling += sub_request_expression->score->spelling * sub_request_expression->score->coverage;
      score->coverage = (double) request_expression->nb_matched_words / nb_words;
      score->any *= sub_request_expression->score->any;
    }
    else
    {
      score->locale += 1.0 / nb_words;
      score->spelling += 1.0 / nb_words;
    }

  }
  score->spelling /= score->coverage;
  score->locale /= score->coverage;

  IFE(NlpCalculateScoreGluePunctuations(ctrl_nlp_th, request_expression, score));

  if (request_expression->expression->alias_any_input_part_position >= 0)
  {
    if (request_expression->Irequest_any >= 0)
    {
      score->any *= 0.8;
    }
    else
    {
      score->any *= 0.2;
    }
  }

  // coverage is calculated in terms of number of matched words
  request_expression->score->coverage = score->coverage;

  // we use a mean score for the locale score
  request_expression->score->locale = score->locale;

  // we use a mean score for the spelling score
  request_expression->score->spelling = score->spelling;
  IFE(NlpAdjustLocaleScore(ctrl_nlp_th, request_expression));

  // overlap_mark is 1, 2, 3 when this number of input_part is included in another input_parts
  // overlap_mark is 100, 200, 300 when this number of input_part are criss-crossing (not likely in an natural language)
  if (request_expression->overlap_mark <= 0)
  {
    request_expression->score->overlap = 1.0;
  }
  else
  {
    request_expression->score->overlap = 0.9;
    request_expression->score->overlap /= request_expression->overlap_mark;
  }

  request_expression->score->any = score->any;

  IFE(NlpContextGetScore(ctrl_nlp_th, request_expression));

  IFE(NlpCalculateScoreMatchScope(ctrl_nlp_th, root_request_expression));

  IFE(NlpCalculateTotalScore(ctrl_nlp_th, request_expression));

  DONE;
}

static og_status NlpCalculateScoreGluePunctuations(og_nlp_th ctrl_nlp_th, struct request_expression *request_expression,
    struct request_score *score)
{
  if (request_expression->expression->glue_strength == nlp_glue_strength_Punctuation)
  {
    int nb_words = ctrl_nlp_th->basic_group_request_word_nb;

    struct request_position *request_position_start = OgHeapGetCell(ctrl_nlp_th->hrequest_position,
        request_expression->request_position_start);
    IFN(request_position_start) DPcErr;

    struct request_position *request_position_end = OgHeapGetCell(ctrl_nlp_th->hrequest_position,
        request_expression->request_position_start + request_expression->request_positions_nb - 1);
    IFN(request_position_end) DPcErr;

    int re_start_position = request_position_start->start + request_position_start->length;
    int re_end_position = request_position_end->start;

    struct request_word *request_words = OgHeapGetCell(ctrl_nlp_th->hrequest_word, 0);
    IFN(request_words) DPcErr;
    int basic_request_word_used = ctrl_nlp_th->basic_request_word_used;
    int Irequest_word_start;
    og_bool found = NlpRequestWordGet(ctrl_nlp_th, re_start_position, &Irequest_word_start);
    IFE(found);
    if (found)
    {
      for (int w = Irequest_word_start; w < basic_request_word_used; w++)
      {
        struct request_word *request_word = request_words + w;
        if (request_word->start_position + request_word->length_position <= re_end_position)
        {
          if (request_word->is_expression_punctuation)
          {
            score->coverage += (double) request_word->nb_matched_words / nb_words;
          }
        }
      }
    }
  }
  DONE;
}

/*
 * It is necessary to do the calculation during the parsing phase
 * to be able to choose some expressions that contain no spelling mistakes
 * over expressions that contain spelling mistakes.
 */

og_status NlpCalculateScoreDuringParsing(og_nlp_th ctrl_nlp_th, struct request_expression *request_expression)
{
  struct request_score score[1];
  memset(score, 0, sizeof(struct request_score));

  for (int i = 0; i < request_expression->orips_nb; i++)
  {
    struct request_input_part *request_input_part = NlpGetRequestInputPart(ctrl_nlp_th, request_expression, i);
    IFN(request_input_part) DPcErr;

    if (request_input_part->type == nlp_input_part_type_Word)
    {
      struct request_word *request_word = request_input_part->request_word;
      score->locale += 1;
      score->spelling += request_word->spelling_score;
    }

    else if (request_input_part->type == nlp_input_part_type_Interpretation)
    {
      struct request_expression *sub_request_expression = OgHeapGetCell(ctrl_nlp_th->hrequest_expression,
          request_input_part->Irequest_expression);
      IFN(sub_request_expression) DPcErr;
      score->locale += sub_request_expression->score->locale;
      score->spelling += sub_request_expression->score->spelling;
    }
    else
    {
      score->locale += 1;
      score->spelling += 1;
    }

  }
  // we use a mean score for the locale score
  request_expression->score->locale = score->locale / request_expression->orips_nb;
  // we use a mean score for the spelling score
  request_expression->score->spelling = score->spelling / request_expression->orips_nb;

  IFE(NlpAdjustLocaleScore(ctrl_nlp_th, request_expression));

  IFE(NlpCalculateTotalScore(ctrl_nlp_th, request_expression));

  DONE;
}

static og_status NlpCalculateTotalScore(og_nlp_th ctrl_nlp_th, struct request_expression *request_expression)
{
  double score_number = 6.0;
  struct request_score *score = request_expression->score;

  double score_sum = score->coverage + score->locale + score->spelling * score->spelling + score->overlap + score->any
      + score->context;

  request_expression->total_score = score->scope * score_sum / score_number;

  DONE;
}

static og_status NlpCalculateScoreMatchScope(og_nlp_th ctrl_nlp_th, struct request_expression *request_expression)
{
  if (ctrl_nlp_th->primary_package != NULL)
  {
    if (ctrl_nlp_th->primary_package == request_expression->expression->interpretation->package)
    {
      request_expression->score->scope = 1;
    }
    else
    {
      request_expression->score->scope = 0.9;
    }
  }
  else
  {
    request_expression->score->scope = 1;
  }
  DONE;
}
