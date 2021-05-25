/*
 *  Handling locale which is the user langage preferences
 *  Copyright (c) 2017 Pertimm, by Patrick Constant
 *  Dev : November 2017
 *  Version 1.0
 */
#include "ogm_nlp.h"

static og_status NlpCalculateScoreRecursive(og_nlp_th ctrl_nlp_th, struct request_expression *root_request_expression,
    struct request_expression *request_expression);
static og_status NlpCalculateTotalScore(og_nlp_th ctrl_nlp_th, struct request_expression *request_expression);
static og_status NlpCalculateScoreMatchScope(og_nlp_th ctrl_nlp_th, struct request_expression *request_expression);

og_status NlpCalculateScore(og_nlp_th ctrl_nlp_th, struct request_expression *request_expression)
{
  IFE(NlpCalculateScoreRecursive(ctrl_nlp_th, request_expression, request_expression));

  // Taking into account the punctuation words if they exist
  // but does not word with number such as 13345.678 so we remove this patch
  //request_expression->score->coverage = request_expression->request_positions_nb;
  //request_expression->score->coverage /= ctrl_nlp_th->basic_request_word_used;
  IFE(NlpCalculateTotalScore(ctrl_nlp_th, request_expression));

  DONE;
}

static og_status NlpCalculateScoreRecursive(og_nlp_th ctrl_nlp_th, struct request_expression *root_request_expression,
    struct request_expression *request_expression)
{
  struct request_score score[1];
  memset(score, 0, sizeof(struct request_score));
  score->any = 1.0;
  int nb_words = ctrl_nlp_th->basic_request_word_used;
  request_expression->nb_matched_words = 0;

  score->ordered = 0.0;

  for (int i = 0; i < request_expression->orips_nb; i++)
  {
    struct request_input_part *request_input_part = NlpGetRequestInputPart(ctrl_nlp_th, request_expression, i);
    IFN(request_input_part) DPcErr;

    if (request_input_part->type == nlp_input_part_type_Word)
    {
      struct request_word *request_word = request_input_part->request_word;
      score->coverage += (double) request_word->nb_matched_words / nb_words;
      score->spelling += (double) request_word->nb_matched_words * request_word->spelling_score
          * request_input_part->score_spelling / nb_words;
      score->locale += (double) request_word->nb_matched_words / nb_words;
      request_expression->nb_matched_words += request_word->nb_matched_words;
      score->ordered += 1;
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
      score->ordered += sub_request_expression->score->ordered;
    }
    else
    {
      score->locale += 1.0 / nb_words;
      score->spelling += 1.0 / nb_words;
      score->ordered += 1;
    }

  }

  score->ordered /= request_expression->orips_nb;
  if (!request_expression->expression->keep_order)
  {
    og_bool is_ordered = NlpRequestExpressionIsOrdered(ctrl_nlp_th, request_expression);
    IFE(is_ordered);
    if (!is_ordered) score->ordered *= 0.5;
  }

  // Should not happen, but safer to do it
  if (score->coverage != 0)
  {
    score->spelling /= score->coverage;
    score->locale /= score->coverage;
  }

  if (request_expression->expression->any_input_part_position >= 0)
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

  request_expression->score->expression = request_expression->expression->score;
  request_expression->score->ordered = score->ordered;

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
      score->spelling += request_word->spelling_score * request_input_part->score_spelling;
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

  request_expression->score->expression = request_expression->expression->score;

  IFE(NlpAdjustLocaleScore(ctrl_nlp_th, request_expression));

  IFE(NlpCalculateTotalScore(ctrl_nlp_th, request_expression));

  DONE;
}

static og_status NlpCalculateTotalScore(og_nlp_th ctrl_nlp_th, struct request_expression *request_expression)
{
  //double score_number = 6.0;
  double score_number = 8.0;
  struct request_score *score = request_expression->score;

  //double score_sum = score->coverage + score->locale + score->spelling * score->spelling + score->overlap + score->any
  //    + score->context;
  double score_sum = score->coverage + score->locale + score->spelling * score->spelling + score->overlap + score->any
      + score->context + score->expression + score->ordered;

  request_expression->total_score = score->scope * score_sum / score_number;

  DONE;
}

static og_status NlpCalculateScoreMatchScope(og_nlp_th ctrl_nlp_th, struct request_expression *request_expression)
{
  og_bool is_primary_package = NlpIsPrimaryPackage(ctrl_nlp_th,
      request_expression->expression->interpretation->package);
  IFE(is_primary_package);

  if (is_primary_package)
  {
    request_expression->score->scope = 1;
  }
  else
  {
    request_expression->score->scope = 0.9;
  }
  DONE;
}

#if 0
// used to debug a double that is the result of division by zero
// put a break point at toto = 1;
static int bad_double(double d)
{
  int toto;
  char buffer[DPcPathSize];
  sprintf(buffer, "%.2f", d);
  if (strstr(buffer, "nan"))
  {
    toto = 1;
  }
  return toto;
}
#endif

