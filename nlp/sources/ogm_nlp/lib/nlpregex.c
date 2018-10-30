/*
 *  Handling regex expressions for NLP
 *  Copyright (c) 2018 Pertimm, by Patrick Constant & Sebastien Manfredini
 *  Dev : November 2018
 *  Version 1.0
 */
#include "ogm_nlp.h"

static og_status NlpRegexBuildInterpretation(og_nlp_th ctrl_nlp_th, struct interpretation *interpretation);
static og_status NlpRegexBuildExpression(og_nlp_th ctrl_nlp_th, struct expression *expression);
static og_status NlpRegexBuildInputPart(og_nlp_th ctrl_nlp_th, struct input_part *input_part);
static og_status NlpRegexCompile(og_nlp_th ctrl_nlp_th, struct input_part *input_part);
static og_status NlpRegexMatchPackage(og_nlp_th ctrl_nlp_th, package_t package);
static og_status NlpRegexMatchInterpretation(og_nlp_th ctrl_nlp_th, struct interpretation *interpretation);
static og_status NlpRegexMatchExpression(og_nlp_th ctrl_nlp_th, struct expression *expression);
static og_status NlpRegexMatchInputPart(og_nlp_th ctrl_nlp_th, struct input_part *input_part);
static og_status NlpRegexAddWord(og_nlp_th ctrl_nlp_th, int word_start, int word_length, struct input_part *input_part,
    size_t sentence_word_count);
static og_status NlpRegexLogPackage(og_nlp_th ctrl_nlp_th, package_t package);
static og_status NlpRegexLogInterpretation(og_nlp_th ctrl_nlp_th, struct interpretation *interpretation);
static og_status NlpRegexLogExpression(og_nlp_th ctrl_nlp_th, struct expression *expression);
static og_status NlpRegexLogInputPart(og_nlp_th ctrl_nlp_th, struct input_part *input_part);

og_status NlpRegexBuildPackage(og_nlp_th ctrl_nlp_th, package_t package)
{
  int interpretation_used = OgHeapGetCellsUsed(package->hinterpretation);
  for (int i = 0; i < interpretation_used; i++)
  {
    struct interpretation *interpretation = OgHeapGetCell(package->hinterpretation, i);
    IFN(interpretation) DPcErr;

    IFE(NlpRegexBuildInterpretation(ctrl_nlp_th, interpretation));
  }
  DONE;
}

static og_status NlpRegexBuildInterpretation(og_nlp_th ctrl_nlp_th, struct interpretation *interpretation)
{
  IFN(interpretation) DPcErr;

  for (int i = 0; i < interpretation->expressions_nb; i++)
  {
    IFE(NlpRegexBuildExpression(ctrl_nlp_th, interpretation->expressions + i));
  }

  DONE;
}

static og_status NlpRegexBuildExpression(og_nlp_th ctrl_nlp_th, struct expression *expression)
{
  IFN(expression) DPcErr;

  for (int i = 0; i < expression->input_parts_nb; i++)
  {
    IFE(NlpRegexBuildInputPart(ctrl_nlp_th, expression->input_parts + i));
  }

  DONE;
}

static og_status NlpRegexBuildInputPart(og_nlp_th ctrl_nlp_th, struct input_part *input_part)
{
  if (input_part->type != nlp_input_part_type_Regex) DONE;
  struct alias *alias = input_part->alias;
  IFN(alias) DPcErr;

  OgMsg(ctrl_nlp_th->hmsg, "", DOgMsgDestInLog, "NlpRegexBuildAlias: found alias '%s' %s='%s'", alias->alias,
      NlpAliasTypeString(alias->type), alias->regex);

  IFE(NlpRegexCompile(ctrl_nlp_th, input_part));

  DONE;
}

static og_status NlpRegexCompile(og_nlp_th ctrl_nlp_th, struct input_part *input_part)
{
  GError *regexp_error = NULL;
  input_part->regex = g_regex_new(input_part->alias->regex, 0, 0, &regexp_error);
  if (!input_part->regex || regexp_error)
  {
    NlpThrowErrorTh(ctrl_nlp_th, "NlpRegexCompile: g_regex_new failed on main : %s", regexp_error->message);
    g_error_free(regexp_error);
    DPcErr;
  }
  DONE;
}

og_status NlpRegexMatch(og_nlp_th ctrl_nlp_th)
{
  int interpret_package_used = OgHeapGetCellsUsed(ctrl_nlp_th->hinterpret_package);
  for (int i = 0; i < interpret_package_used; i++)
  {
    struct interpret_package *interpret_package = OgHeapGetCell(ctrl_nlp_th->hinterpret_package, i);
    IFN(interpret_package) DPcErr;

    IFE(NlpRegexMatchPackage(ctrl_nlp_th, interpret_package->package));
  }
  DONE;
}

static og_status NlpRegexMatchPackage(og_nlp_th ctrl_nlp_th, package_t package)
{
  int interpretation_used = OgHeapGetCellsUsed(package->hinterpretation);
  for (int i = 0; i < interpretation_used; i++)
  {
    struct interpretation *interpretation = OgHeapGetCell(package->hinterpretation, i);
    IFN(interpretation) DPcErr;

    IFE(NlpRegexMatchInterpretation(ctrl_nlp_th, interpretation));
  }
  DONE;
}

static og_status NlpRegexMatchInterpretation(og_nlp_th ctrl_nlp_th, struct interpretation *interpretation)
{
  IFN(interpretation) DPcErr;

  for (int i = 0; i < interpretation->expressions_nb; i++)
  {
    IFE(NlpRegexMatchExpression(ctrl_nlp_th, interpretation->expressions + i));
  }

  DONE;
}

static og_status NlpRegexMatchExpression(og_nlp_th ctrl_nlp_th, struct expression *expression)
{
  IFN(expression) DPcErr;

  for (int i = 0; i < expression->input_parts_nb; i++)
  {
    IFE(NlpRegexMatchInputPart(ctrl_nlp_th, expression->input_parts + i));
  }

  DONE;
}

static og_status NlpRegexMatchInputPart(og_nlp_th ctrl_nlp_th, struct input_part *input_part)
{
  if (input_part->type != nlp_input_part_type_Regex) DONE;
  og_string sentence = ctrl_nlp_th->request_sentence;
  size_t sentence_word_count = OgHeapGetCellsUsed(ctrl_nlp_th->hrequest_word);

  int start_match_position = 0;
  int end_match_position = 0;

  if (input_part->regex)
  {
    // match the regular expression
    GMatchInfo *match_info = NULL;
    GError *regexp_error = NULL;
    og_bool match = g_regex_match_full(input_part->regex, sentence, -1, 0, 0, &match_info, &regexp_error);
    if (regexp_error)
    {
      NlpThrowErrorTh(ctrl_nlp_th, "NlpMatchRegexes: g_regex_match_all_full failed on execution : %s",
          regexp_error->message);
      g_error_free(regexp_error);

      if (match_info != NULL)
      {
        g_match_info_free(match_info);
      }

      DPcErr;
    }

    if (match)
    {
      while (g_match_info_matches(match_info))
      {
        g_match_info_fetch_pos(match_info, 0, &start_match_position, &end_match_position);

        if (ctrl_nlp_th->loginfo->trace & DOgNlpTraceMatch)
        {
          OgMsg(ctrl_nlp_th->hmsg, "", DOgMsgDestInLog,
              "NlpMatchRegexes: matched sentence with regex '%s' start=%d end=%d", sentence, start_match_position,
              end_match_position);
        }
        IFE(
            NlpRegexAddWord(ctrl_nlp_th, start_match_position, end_match_position - start_match_position, input_part,
                sentence_word_count));

        g_match_info_next(match_info, &regexp_error);

        if (regexp_error)
        {
          NlpThrowErrorTh(ctrl_nlp_th, "NlpMatchRegexes: g_match_info_next failed on execution : %s",
              regexp_error->message);
          g_error_free(regexp_error);

          if (match_info != NULL)
          {
            g_match_info_free(match_info);
          }

          DPcErr;
        }
      }

      g_match_info_free(match_info);
      match_info = NULL;
    }

    // ensure match_info freeing
    if (match_info != NULL)
    {
      g_match_info_free(match_info);
    }

  }

  DONE;
}

static og_status NlpRegexAddWord(og_nlp_th ctrl_nlp_th, int word_start, int word_length, struct input_part *input_part,
    size_t sentence_word_count)
{
  og_string s = ctrl_nlp_th->request_sentence;

  NlpLog(DOgNlpTraceMatch, "NlpRegexAddWord: adding word '%.*s' at start %d", word_length, s + word_start, word_start)

  char normalized_string_word[DPcPathSize];
  int length_normalized_string_word = OgUtf8Normalize(word_length, s + word_start, DPcPathSize, normalized_string_word);
  NlpLog(DOgNlpTraceConsolidate, "NlpRegexAddWord: normalized word '%.*s'", length_normalized_string_word,
      normalized_string_word)

  size_t Irequest_word;
  struct request_word *request_word = OgHeapNewCell(ctrl_nlp_th->hrequest_word, &Irequest_word);
  IFn(request_word) DPcErr;
  IF(Irequest_word) DPcErr;

  request_word->self_index = Irequest_word;
  request_word->start = OgHeapGetCellsUsed(ctrl_nlp_th->hba);
  request_word->length = length_normalized_string_word;
  IFE(OgHeapAppend(ctrl_nlp_th->hba, request_word->length, normalized_string_word));
  IFE(OgHeapAppend(ctrl_nlp_th->hba, 1, ""));

  request_word->raw_start = OgHeapGetCellsUsed(ctrl_nlp_th->hba);
  request_word->raw_length = word_length;
  IFE(OgHeapAppend(ctrl_nlp_th->hba, request_word->raw_length, s + word_start));
  IFE(OgHeapAppend(ctrl_nlp_th->hba, 1, ""));

  request_word->start_position = word_start;
  request_word->length_position = word_length;

  request_word->is_number = FALSE;
  request_word->is_punctuation = FALSE;
  request_word->is_auto_complete_word = FALSE;
  request_word->is_regex = TRUE;
  request_word->regex_input_part = input_part;
  request_word->spelling_score = 1.0;

  int wordCount = 0;
  for (size_t i = 0; i < sentence_word_count; i++)
  {
    struct request_word *sentence_word = OgHeapGetCell(ctrl_nlp_th->hrequest_word, i);
    if (sentence_word->start_position >= request_word->start_position
        && sentence_word->start_position + sentence_word->length <= request_word->start_position + request_word->length) wordCount++;
  }
  request_word->nb_matched_words = wordCount;

  DONE;
}

og_status NlpRegexLog(og_nlp_th ctrl_nlp_th)
{
  int interpret_package_used = OgHeapGetCellsUsed(ctrl_nlp_th->hinterpret_package);
  for (int i = 0; i < interpret_package_used; i++)
  {
    struct interpret_package *interpret_package = OgHeapGetCell(ctrl_nlp_th->hinterpret_package, i);
    IFN(interpret_package) DPcErr;

    IFE(NlpRegexLogPackage(ctrl_nlp_th, interpret_package->package));
  }
  DONE;
}

static og_status NlpRegexLogPackage(og_nlp_th ctrl_nlp_th, package_t package)
{
  int interpretation_used = OgHeapGetCellsUsed(package->hinterpretation);
  for (int i = 0; i < interpretation_used; i++)
  {
    struct interpretation *interpretation = OgHeapGetCell(package->hinterpretation, i);
    IFN(interpretation) DPcErr;

    IFE(NlpRegexLogInterpretation(ctrl_nlp_th, interpretation));
  }
  DONE;
}

static og_status NlpRegexLogInterpretation(og_nlp_th ctrl_nlp_th, struct interpretation *interpretation)
{
  IFN(interpretation) DPcErr;

  for (int i = 0; i < interpretation->expressions_nb; i++)
  {
    IFE(NlpRegexLogExpression(ctrl_nlp_th, interpretation->expressions + i));
  }

  DONE;
}

static og_status NlpRegexLogExpression(og_nlp_th ctrl_nlp_th, struct expression *expression)
{
  IFN(expression) DPcErr;

  for (int i = 0; i < expression->input_parts_nb; i++)
  {
    IFE(NlpRegexLogInputPart(ctrl_nlp_th, expression->input_parts + i));
  }

  DONE;
}

static og_status NlpRegexLogInputPart(og_nlp_th ctrl_nlp_th, struct input_part *input_part)
{
  if (input_part->type != nlp_input_part_type_Regex) DONE;
  OgMsg(ctrl_nlp_th->hmsg, "", DOgMsgDestInLog, "regex '%s' '%s'", input_part->alias->alias, input_part->alias->regex);
  DONE;
}

