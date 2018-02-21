/*
 *  Handling decimal number parsing with different locale convention
 *
 *  Copyright (c) 2017 Pertimm, by Sebastien Manfedini, Brice Ruzand
 *  Dev : Janyary 2018
 *  Version 1.0
 */
#include "ogm_nlp.h"

struct lang_sep
{
  int lang;
  int country;
  og_string thousand_sep;
  og_string decimal_sep;
  og_bool used;
};

#define locale_lang_country(lang, country) (lang + DOgLangMax * country)

static struct lang_sep lang_sep_conf[] = {   //
    { DOgLangNil, DOgCountryCH, "'", ".", FALSE },   // *-CH
        { DOgLangFR, DOgCountryNil, " ", ",", FALSE },   // fr-*
        { DOgLangEN, DOgCountryNil, ",", ".", FALSE },   // en-*

        { DOgLangNil, DOgCountryNil, "", "", FALSE }   // End
    };
static struct lang_sep *lang_sep_default = &lang_sep_conf[1];

static og_bool NlpMatchGroupNumbersParsing(og_nlp_th ctrl_nlp_th, og_string sentence, GRegex *regular_expression,
    og_string thousand_sep, double *p_value);
static og_bool NlpMatchGroupNumbersByLanguage(og_nlp_th ctrl_nlp_th, struct lang_sep* lang_conf);
static og_bool str_remove_inplace(og_char_buffer *source, og_string to_replace);
static struct lang_sep* getNumberSeparatorsConf(struct lang_sep* lang_conf, int locale);

og_status NlpMatchGroupNumbersInit(og_nlp_th ctrl_nlp_th)
{

  DONE;
}

og_status NlpMatchGroupNumbersFlush(og_nlp_th ctrl_nlp_th)
{

  DONE;
}

og_bool NlpMatchGroupNumbers(og_nlp_th ctrl_nlp_th)
{
  og_bool numbers_grouped = FALSE;
  og_bool language_found = FALSE;

  struct lang_sep* lang_conf = lang_sep_conf;

  // check the given accepted languages
  int numberLang = OgHeapGetCellsUsed(ctrl_nlp_th->haccept_language);
  for (int i = 0; i < numberLang && !numbers_grouped; i++)
  {
    struct accept_language* accepted_language = OgHeapGetCell(ctrl_nlp_th->haccept_language, i);

    int locale = accepted_language->locale;

    struct lang_sep *conf = getNumberSeparatorsConf(lang_conf, locale);
    if (conf)
    {
      language_found = TRUE;

      numbers_grouped = NlpMatchGroupNumbersByLanguage(ctrl_nlp_th, conf);
      IF(numbers_grouped)
      {
        og_char_buffer current_lang_country[DPcPathSize];
        NlpThrowErrorTh(ctrl_nlp_th, "NlpMatchWordGroupNumbers: failed on NlpMatchNumbersGroupByLanguage "
            "with accepted-language %s (%g)", OgIso639_3166ToCode(locale, current_lang_country),
            accepted_language->quality_factor);
        DPcErr;
      }
    }
  }

  // accepted languages is not provided or not found
  if (!language_found)
  {
    struct lang_sep* default_lang_conf = lang_sep_default;

    // first check default language
    if (!numbers_grouped)
    {
      struct lang_sep *conf = default_lang_conf;
      numbers_grouped = NlpMatchGroupNumbersByLanguage(ctrl_nlp_th, conf);
      IF(numbers_grouped)
      {
        NlpThrowErrorTh(ctrl_nlp_th, "NlpMatchWordGroupNumbers: failed on NlpMatchNumbersGroupByLanguage"
            " with default language %s-%s", OgIso639ToCode(conf->lang), OgIso3166ToCode(conf->country));
        DPcErr;
      }
    }

    // then check all others language
    for (struct lang_sep *conf = lang_conf; !numbers_grouped; conf++)
    {
      // end of conf
      if (conf->lang == DOgLangNil && conf->country == DOgCountryNil) break;

      // skip default language
      if (conf == default_lang_conf) continue;

      numbers_grouped = NlpMatchGroupNumbersByLanguage(ctrl_nlp_th, conf);
      IF(numbers_grouped)
      {
        NlpThrowErrorTh(ctrl_nlp_th, "NlpMatchWordGroupNumbers: failed on NlpMatchNumbersGroupByLanguage"
            " fallback on language %s-%s", OgIso639ToCode(conf->lang), OgIso3166ToCode(conf->country));
        DPcErr;
      }
    }

  }

  return numbers_grouped;
}


static og_bool NlpMatchGroupNumbersParsingEnsureFree(og_nlp_th ctrl_nlp_th, og_string sentence, GMatchInfo *match_info,
    og_string thousand_sep, double *p_value)
{
  // getting the integer part position on sentence
  int integerPartStart = 0;
  int integerPartLength = 0;
  og_bool integerPartMatch = g_match_info_fetch_pos(match_info, 1, &integerPartStart, &integerPartLength);
  if (!integerPartMatch) return FALSE;

  // getting the decimal part position on sentence
  int decimalPartStart = 0;
  int decimalPartLength = 0;
  og_bool decimalPartMatch = g_match_info_fetch_pos(match_info, 2, &decimalPartStart, &decimalPartLength);

  // bluid number value

  // append integer part removing thousand separators
  og_char_buffer value_buffer[DPcPathSize];
  snprintf(value_buffer, DPcPathSize, "%.*s", integerPartLength, sentence + integerPartStart);
  str_remove_inplace(value_buffer, thousand_sep);

  // append decimal part
  if (decimalPartMatch && decimalPartLength >= 0)
  {
    snprintf(value_buffer + strlen(value_buffer), DPcPathSize, ".%.*s", decimalPartLength, sentence + decimalPartStart);
  }
  double tmp_value = atof(value_buffer);

  if (p_value) *p_value = tmp_value;

  return TRUE;
}

static og_bool NlpMatchGroupNumbersParsing(og_nlp_th ctrl_nlp_th, og_string sentence, GRegex *regular_expression,
    og_string thousand_sep, double *p_value)
{
  if (p_value) *p_value = 0;

  og_bool found = FALSE;

  // match the regular expression
  GMatchInfo *match_info = NULL;
  og_bool match = g_regex_match(regular_expression, sentence, 0, &match_info);
  if (match)
  {
    found = NlpMatchGroupNumbersParsingEnsureFree(ctrl_nlp_th, sentence, match_info, thousand_sep, p_value);
  }

  // ensure match_info freeing
  if (match_info != NULL)
  {
    g_match_info_free(match_info);
  }

  return found;
}

static og_bool NlpMatchGroupNumbersByLanguage(og_nlp_th ctrl_nlp_th, struct lang_sep* lang_conf)
{
  og_string request_sentence = ctrl_nlp_th->request_sentence;

  // parsing the numbers
  double value = 0;
  int request_word_used = OgHeapGetCellsUsed(ctrl_nlp_th->hrequest_word);
  if (request_word_used == 0) return FALSE;

  // getting thousand and decimal separator
  og_string thousand_sep = g_regex_escape_string(lang_conf->thousand_sep, -1);
  og_string decimal_sep = g_regex_escape_string(lang_conf->decimal_sep, -1);

  og_char_buffer pattern[DPcPathSize];
  snprintf(pattern, DPcPathSize, "^((?:(?:\\d{1,3}(?:%s\\d{3})+)|\\d+))(?:%s(\\d*))?$", thousand_sep, decimal_sep);

  GError *regexp_error = NULL;
  GRegex *regular_expression = g_regex_new(pattern, 0, 0, &regexp_error);

  struct request_word *rw_start = OgHeapGetCell(ctrl_nlp_th->hrequest_word, 0);
  IFN(rw_start)
  {
    NlpThrowErrorTh(ctrl_nlp_th, "NlpMatchNumbersGroupByLanguage: "
        "OgHeapGetCell(ctrl_nlp_th->hrequest_word, 0) failed");
    DPcErr;
  }

  og_bool numbers_grouped = FALSE;
  og_bool previous_match = FALSE;
  struct request_word *rw_end = NULL;
  for (rw_end = rw_start; rw_end && rw_end->self_index < ctrl_nlp_th->basic_request_word_used; rw_end = rw_end->next)
  {
    og_string string_request_word = OgHeapGetCell(ctrl_nlp_th->hba, rw_end->start);
    if (!string_request_word)
    {
      NlpThrowErrorTh(ctrl_nlp_th, "NlpMatchNumbersGroupByLanguage: failed "
          "OgHeapGetCell(ctrl_nlp_th->hba, rw_end->start)");
      DPcErr;
    }

    // on ne prend pas en compte les séparateurs en fin de nombre
    if (!strcmp(string_request_word, lang_conf->thousand_sep) || !strcmp(string_request_word, lang_conf->decimal_sep))
    {
      continue;
    }

    // recuperer la string entre NumberGroupStart et NumberGroupEnd
    og_char_buffer expression_string[DPcPathSize];
    int start_position = rw_start->start_position;
    int end_position = rw_end->start_position + rw_end->length_position;
    int length = end_position - start_position;
    snprintf(expression_string, DPcPathSize, "%.*s", length, request_sentence + start_position);

    og_bool number_match = NlpMatchGroupNumbersParsing(ctrl_nlp_th, expression_string, regular_expression,
        lang_conf->thousand_sep, &value);
    IFE(number_match);

    if (number_match)
    {
      if (rw_start != rw_end)
      {
        numbers_grouped = TRUE;
      }
      rw_start->next = rw_end->next;
      rw_start->length_position = rw_end->start_position - rw_start->start_position + rw_end->length_position;
      rw_start->raw_length = rw_end->raw_start - rw_start->raw_start + rw_end->raw_length;
      rw_start->length = rw_end->start - rw_start->start + rw_end->length;
      rw_start->is_number = TRUE;
      rw_start->number_value = value;
      previous_match = TRUE;
    }
    else
    {
      if (previous_match)
      {
        // si on a 123.456 789 XXXX, on a un match à false, mais il faut reconsidérer 789 comme un nombre

        // recuperer la string entre NumberGroupStart et NumberGroupEnd
        start_position = rw_end->start_position;
        end_position = rw_end->start_position + rw_end->length_position;
        length = end_position - start_position;
        snprintf(expression_string, DPcPathSize, "%.*s", length, request_sentence + start_position);

        number_match = NlpMatchGroupNumbersParsing(ctrl_nlp_th, expression_string, regular_expression, thousand_sep,
            &value);
        IFE(number_match);

        if (number_match)
        {
          // on est dans le cas 123.456 789 XXXX, il faut considérer le 789 comme un nombre

          rw_start = rw_end;
          rw_start->is_number = TRUE;
          rw_start->number_value = value;
        }
        else
        {
          // on est dans le cas 123.456 aa XXXX, il faut recommencer le parsing au mot suivant

          if (rw_end->next)
          {
            rw_start = rw_end->next;
          }
          previous_match = FALSE;
        }

      }
      else
      {
        if (rw_end->next)
        {
          rw_start = rw_end->next;
        }
      }
    }
  }

  g_regex_unref(regular_expression);
  return numbers_grouped;
}

static struct lang_sep* getNumberSeparatorsConf(struct lang_sep* lang_conf, int locale)
{
  struct lang_sep* found = NULL;

  int req_lang = OgIso639_3166ToLang(locale);
  int req_country = OgIso639_3166ToCountry(locale);

  // lookfor lang + country
  for (struct lang_sep *conf = lang_conf; !found; conf++)
  {
    // end of conf
    if (conf->lang == DOgLangNil && conf->country == DOgCountryNil) break;

    if (conf->lang == req_lang && conf->country == req_country)
    {
      found = conf;
    }
  }

  // lookfor country only
  for (struct lang_sep *conf = lang_conf; !found; conf++)
  {
    // end of conf
    if (conf->lang == DOgLangNil && conf->country == DOgCountryNil) break;

    if (conf->lang == DOgLangNil && conf->country == req_country)
    {
      found = conf;
    }
  }

  // lookfor lang only
  for (struct lang_sep *conf = lang_conf; !found; conf++)
  {
    // end of conf
    if (conf->lang == DOgLangNil && conf->country == DOgCountryNil) break;

    if (conf->lang == req_lang && conf->country == DOgCountryNil)
    {
      found = conf;
    }
  }

  return found;
}


static og_bool str_remove_inplace(og_char_buffer *source, og_string to_replace)
{
  int to_replace_size = strlen(to_replace);
  int source_size = strlen(source);
  og_char_buffer *p = strstr(source, to_replace);
  if (p)
  {
    memmove(p, p + to_replace_size, source_size - to_replace_size - (source - p) + 1);
    IFE(str_remove_inplace(p, to_replace));
    return TRUE;
  }
  return FALSE;
}
