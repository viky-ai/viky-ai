/*
 *  Handling decimal number parsing with different locale convention
 *
 *  Copyright (c) 2017 Pertimm, by Sebastien Manfedini, Brice Ruzand
 *  Dev : Janyary 2018
 *  Version 1.0
 */
#include "ogm_nlp.h"

/**
 * thousand and decimal separator configuration language agnostic
 */
struct number_sep_conf
{
  og_string thousand_sep;
  og_string decimal_sep;

  /** regex for these separator */
  GRegex *regex;

  /** regex already check */
  og_bool already_processed;
};

/**
 * Association between separator configuration and locale
 */
struct number_sep_conf_locale
{
  /** lang iso639 */
  int lang;

  /** country iso3166*/
  int country;

  /** locale : lang+ country iso639_3166 */
  int lang_country;

  /** separator configuration associate with */
  struct number_sep_conf *sep_conf;
};

#define locale_lang_country(lang, country) (lang + DOgLangMax * country)

static og_bool NlpMatchGroupNumbersParsing(og_nlp_th ctrl_nlp_th, og_string sentence,
    struct number_sep_conf* locale_conf, double *p_value);
static og_status NlpMatchGroupNumbersWithLocale(struct nlp_match_group_numbers *nmgn, int locale,
    og_bool *p_locale_found, og_bool *p_grouped_numbers);
static og_bool NlpMatchGroupNumbersByLocale(og_nlp_th ctrl_nlp_th, struct number_sep_conf* locale_conf);

static struct number_sep_conf* NlpMatchGroupNumbersInitAddSeparatorConf(og_nlp_th ctrl_nlp_th, og_string thousand_sep,
    og_string decimal_sep);
static og_status NlpMatchGroupNumbersInitAssociateLocaleWithConf(og_nlp_th ctrl_nlp_th, int lang, int country,
    struct number_sep_conf* sep_conf);

static og_bool str_remove_inplace(og_char_buffer *source, og_string to_replace);

// Shorcut for easier reading
#define addSepConf(thousand_sep, decimal_sep) NlpMatchGroupNumbersInitAddSeparatorConf(ctrl_nlp_th, thousand_sep, decimal_sep)
#define addLocaleConf(param_lang, param_country, param_sep_conf) NlpMatchGroupNumbersInitAssociateLocaleWithConf(ctrl_nlp_th, param_lang, param_country, param_sep_conf)

static og_status NlpMatchGroupNumbersInitConf(og_nlp_th ctrl_nlp_th)
{
  // create conf
  struct number_sep_conf *conf_quote_dot = addSepConf("'", ".");
  IFNE(conf_quote_dot);

  struct number_sep_conf *conf_space_coma = addSepConf(" ", ",");
  IFNE(conf_space_coma);

  struct number_sep_conf *conf_coma_dot = addSepConf(",", ".");
  IFNE(conf_coma_dot);

  struct number_sep_conf *conf_dot_coma = addSepConf(".", ",");
  IFNE(conf_dot_coma);

  // associate conf with locale

  // fr-FR, *-FR, fr-*
  IFE(addLocaleConf(DOgLangFR, DOgCountryFR, conf_space_coma));

  // en-US, *-US, en-*
  IFE(addLocaleConf(DOgLangEN, DOgCountryUS, conf_coma_dot));

  // de-DE, *-DE, de-*
  IFE(addLocaleConf(DOgLangDE, DOgCountryDE, conf_dot_coma));

  // *-CH
  IFE(addLocaleConf(DOgLangNil, DOgCountryCH, conf_quote_dot));

  // set default conf : fr-FR
  ctrl_nlp_th->group_numbers_settings->default_conf = conf_space_coma;

  DONE;
}

og_status NlpMatchGroupNumbersInit(og_nlp_th ctrl_nlp_th)
{
  struct nlp_match_group_numbers *nmgn = ctrl_nlp_th->group_numbers_settings;
  memset(nmgn, 0, sizeof(struct nlp_match_group_numbers));

  nmgn->nlpth = ctrl_nlp_th;
  nmgn->sep_conf_lang_by_lang_country = g_hash_table_new(g_direct_hash, g_direct_equal);
  nmgn->sep_conf_lang_by_country = g_hash_table_new(g_direct_hash, g_direct_equal);
  nmgn->sep_conf_lang_by_lang = g_hash_table_new(g_direct_hash, g_direct_equal);

  IFE(NlpMatchGroupNumbersInitConf(ctrl_nlp_th));

  DONE;
}

static struct number_sep_conf* NlpMatchGroupNumbersInitAddSeparatorConf(og_nlp_th ctrl_nlp_th, og_string thousand_sep,
    og_string decimal_sep)
{
  struct nlp_match_group_numbers *nmgn = ctrl_nlp_th->group_numbers_settings;

  struct number_sep_conf* conf = g_slice_new0(struct number_sep_conf);
  conf->thousand_sep = g_strdup(thousand_sep);
  conf->decimal_sep = g_strdup(decimal_sep);

  // escape regex operator in thousand and decimal separator
  gchar *escaped_thousand_sep = g_regex_escape_string(conf->thousand_sep, -1);
  gchar *escaped_decimal_sep = g_regex_escape_string(conf->decimal_sep, -1);

  og_char_buffer pattern[DPcPathSize];
  snprintf(pattern, DPcPathSize, "^((?:(?:\\d{1,3}(?:%s\\d{3})+)|\\d+))(?:%s(\\d*))?$", escaped_thousand_sep,
      escaped_decimal_sep);

  g_free(escaped_thousand_sep);
  g_free(escaped_decimal_sep);

  GError *regexp_error = NULL;
  GRegex *regular_expression = g_regex_new(pattern, 0, 0, &regexp_error);
  if (!regular_expression || regexp_error)
  {
    NlpThrowErrorTh(nmgn->nlpth, "NlpMatchGroupNumbersInitAddSeparatorConf: g_regex_new failed : %s",
        regexp_error->message);
    return NULL;
  }

  // build regex
  conf->regex = regular_expression;

  // add conf in list
  g_queue_push_tail(nmgn->sep_conf, conf);

  return conf;
}

static og_status NlpMatchGNInitHashInsert(GHashTable *hash_table, struct number_sep_conf_locale* conf_lang, int key)
{
  GList *list = g_hash_table_lookup(hash_table, GINT_TO_POINTER(key));
  if (!list)
  {
    list = g_list_append(list, conf_lang);
    g_hash_table_insert(hash_table, GINT_TO_POINTER(key), list);
  }
  else
  {
    list = g_list_append(list, conf_lang);
  }

  DONE;
}

static og_status NlpMatchGroupNumbersInitAssociateLocaleWithConf(og_nlp_th ctrl_nlp_th, int lang, int country,
    struct number_sep_conf* sep_conf)
{
  struct nlp_match_group_numbers *nmgn = ctrl_nlp_th->group_numbers_settings;

  struct number_sep_conf_locale* conf_lang = g_slice_new0(struct number_sep_conf_locale);
  conf_lang->lang = lang;
  conf_lang->country = country;
  conf_lang->lang_country = locale_lang_country(lang, country);
  conf_lang->sep_conf = sep_conf;

  // add conf in list
  g_queue_push_tail(nmgn->sep_conf_lang, conf_lang);

  // add conf in lookup table for fast access

  // lookup by lang_country
  IFE(NlpMatchGNInitHashInsert(nmgn->sep_conf_lang_by_lang_country, conf_lang, conf_lang->lang_country));

  // lookup by lang
  IFE(NlpMatchGNInitHashInsert(nmgn->sep_conf_lang_by_lang, conf_lang, conf_lang->lang));

  // lookup by country
  IFE(NlpMatchGNInitHashInsert(nmgn->sep_conf_lang_by_country, conf_lang, conf_lang->country));

  DONE;
}

og_status NlpMatchGroupNumbersFlush(og_nlp_th ctrl_nlp_th)
{

  struct nlp_match_group_numbers *nmgn = ctrl_nlp_th->group_numbers_settings;

  // free all lookup hash
  g_hash_table_destroy(nmgn->sep_conf_lang_by_lang_country);
  nmgn->sep_conf_lang_by_lang_country = NULL;

  g_hash_table_destroy(nmgn->sep_conf_lang_by_country);
  nmgn->sep_conf_lang_by_country = NULL;

  g_hash_table_destroy(nmgn->sep_conf_lang_by_lang);
  nmgn->sep_conf_lang_by_lang = NULL;

  // free sep_conf_lang
  for (GList *iter = nmgn->sep_conf_lang->head; iter; iter = iter->next)
  {
    struct number_sep_conf_locale *conf = iter->data;

    g_slice_free(struct number_sep_conf_locale, conf);
    iter->data = NULL;
  }
  g_queue_clear(nmgn->sep_conf_lang);

  // free sep_conf
  for (GList *iter = nmgn->sep_conf->head; iter; iter = iter->next)
  {
    struct number_sep_conf *conf = iter->data;

    g_free((gchar*) conf->decimal_sep);
    conf->decimal_sep = NULL;

    g_free((gchar*) conf->thousand_sep);
    conf->thousand_sep = NULL;

    g_regex_unref(conf->regex);
    conf->regex = NULL;

    g_slice_free(struct number_sep_conf, conf);
    iter->data = NULL;

  }
  g_queue_clear(nmgn->sep_conf);

  DONE;
}

static og_status NlpMatchGroupNumbersMatchReset(og_nlp_th ctrl_nlp_th)
{
  struct nlp_match_group_numbers *nmgn = ctrl_nlp_th->group_numbers_settings;

  for (GList *iter = nmgn->sep_conf->head; iter; iter = iter->next)
  {
    struct number_sep_conf *conf = iter->data;
    conf->already_processed = FALSE;
  }

  DONE;
}

og_bool NlpMatchGroupNumbers(og_nlp_th ctrl_nlp_th)
{
  struct nlp_match_group_numbers *nmgn = ctrl_nlp_th->group_numbers_settings;

  // look for all available locale
  IFE(NlpMatchGroupNumbersMatchReset(ctrl_nlp_th));

  og_bool numbers_grouped = FALSE;
  og_bool language_found = FALSE;

  // check the given accepted languages
  int numberLang = OgHeapGetCellsUsed(ctrl_nlp_th->haccept_language);
  for (int i = 0; i < numberLang && !numbers_grouped; i++)
  {
    struct accept_language* accepted_language = OgHeapGetCell(ctrl_nlp_th->haccept_language, i);

    int locale = accepted_language->locale;

    og_status status = NlpMatchGroupNumbersWithLocale(nmgn, locale, &language_found, &numbers_grouped);
    IF(status)
    {
      og_char_buffer current_lang_country[DPcPathSize];
      NlpThrowErrorTh(ctrl_nlp_th, "NlpMatchGroupNumbers: failed on NlpMatchGroupNumbersWithLocale "
          "with accepted-language %s (%g)", OgIso639_3166ToCode(locale, current_lang_country),
          accepted_language->quality_factor);
      DPcErr;
    }
  }

  // accepted languages is not provided or not found and no groups found
  if (!language_found && !numbers_grouped)
  {
    // first check default language
    struct number_sep_conf* default_sep_conf = (struct number_sep_conf*) nmgn->default_conf;
    numbers_grouped = NlpMatchGroupNumbersByLocale(ctrl_nlp_th, default_sep_conf);
    IF(numbers_grouped)
    {
      NlpThrowErrorTh(ctrl_nlp_th, "NlpMatchWordGroupNumbers: failed on NlpMatchNumbersGroupByLanguage"
          " with default: thousand_sep=\"%s\", decimal_sep=\"%s\"", default_sep_conf->thousand_sep,
          default_sep_conf->decimal_sep);
      DPcErr;
    }

    // then check all others language
    for (GList *iter = nmgn->sep_conf->head; iter && !numbers_grouped; iter = iter->next)
    {
      struct number_sep_conf* sep_conf = iter->data;

      numbers_grouped = NlpMatchGroupNumbersByLocale(ctrl_nlp_th, sep_conf);
      IF(numbers_grouped)
      {
        NlpThrowErrorTh(ctrl_nlp_th, "NlpMatchWordGroupNumbers: failed on NlpMatchNumbersGroupByLanguage"
            " with all: thousand_sep=\"%s\", decimal_sep=\"%s\"", sep_conf->thousand_sep, sep_conf->decimal_sep);
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

  // build number value

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

static og_bool NlpMatchGroupNumbersParsing(og_nlp_th ctrl_nlp_th, og_string sentence,
    struct number_sep_conf* locale_conf, double *p_value)
{
  if (p_value) *p_value = 0;

  og_bool found = FALSE;

  // match the regular expression
  GMatchInfo *match_info = NULL;
  og_bool match = g_regex_match(locale_conf->regex, sentence, 0, &match_info);
  if (match)
  {
    found = NlpMatchGroupNumbersParsingEnsureFree(ctrl_nlp_th, sentence, match_info, locale_conf->thousand_sep,
        p_value);
  }

  // ensure match_info freeing
  if (match_info != NULL)
  {
    g_match_info_free(match_info);
  }

  return found;
}

static og_bool NlpMatchGroupNumbersByLocale(og_nlp_th ctrl_nlp_th, struct number_sep_conf* locale_conf)
{

  // optimize several language with same conf
  if (locale_conf->already_processed) return FALSE;
  locale_conf->already_processed = TRUE;

  og_string request_sentence = ctrl_nlp_th->request_sentence;

  // parsing the numbers
  double value = 0;
  int request_word_used = OgHeapGetCellsUsed(ctrl_nlp_th->hrequest_word);
  if (request_word_used == 0) return FALSE;

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
    if (!strcmp(string_request_word, locale_conf->thousand_sep)
        || !strcmp(string_request_word, locale_conf->decimal_sep))
    {
      continue;
    }

    // recuperer la string entre NumberGroupStart et NumberGroupEnd
    og_char_buffer expression_string[DPcPathSize];
    int start_position = rw_start->start_position;
    int end_position = rw_end->start_position + rw_end->length_position;
    int length = end_position - start_position;
    snprintf(expression_string, DPcPathSize, "%.*s", length, request_sentence + start_position);

    og_bool number_match = NlpMatchGroupNumbersParsing(ctrl_nlp_th, expression_string, locale_conf, &value);
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

        number_match = NlpMatchGroupNumbersParsing(ctrl_nlp_th, expression_string, locale_conf, &value);
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

  return numbers_grouped;
}

static og_status NlpMatchGroupNumbersWithLocale(struct nlp_match_group_numbers *nmgn, int locale,
    og_bool *p_locale_found, og_bool *p_grouped_numbers)
{
  struct og_ctrl_nlp_threaded *ctrl_nlp_th = nmgn->nlpth;

  // reset output variables
  if (p_locale_found) *p_locale_found = FALSE;
  if (p_grouped_numbers) *p_grouped_numbers = FALSE;

  og_bool locale_found = FALSE;
  og_bool grouped_numbers = FALSE;

  int lang_country = locale;
  int lang = OgIso639_3166ToLang(lang_country);
  int country = OgIso639_3166ToCountry(lang_country);

  {
    // lookfor lang + country
    GList *list_lang_country = g_hash_table_lookup(nmgn->sep_conf_lang_by_lang_country, GINT_TO_POINTER(lang_country));
    for (GList *iter = list_lang_country; iter && !locale_found && !grouped_numbers; iter = iter->next)
    {
      struct number_sep_conf_locale *conf = iter->data;
      locale_found = TRUE;

      grouped_numbers = NlpMatchGroupNumbersByLocale(ctrl_nlp_th, conf->sep_conf);
      IF(grouped_numbers)
      {
        og_char_buffer current_lang_country[DPcPathSize];
        NlpThrowErrorTh(ctrl_nlp_th, "NlpMatchGroupNumbersWithLocale: failed on NlpMatchGroupNumbersByLocale "
            "with locale %s (lang + country)", OgIso639_3166ToCode(conf->lang_country, current_lang_country));
        DPcErr;
      }
    }
  }

  {
    // lookfor country only
    GList *list_country = g_hash_table_lookup(nmgn->sep_conf_lang_by_country, GINT_TO_POINTER(country));
    for (GList *iter = list_country; iter && !locale_found && !grouped_numbers; iter = iter->next)
    {
      struct number_sep_conf_locale *conf = iter->data;
      locale_found = TRUE;

      grouped_numbers = NlpMatchGroupNumbersByLocale(ctrl_nlp_th, conf->sep_conf);
      IF(grouped_numbers)
      {
        NlpThrowErrorTh(ctrl_nlp_th, "NlpMatchGroupNumbersWithLocale: failed on NlpMatchGroupNumbersByLocale "
            "with locale %s (country only)", OgIso3166ToCode(conf->country));
        DPcErr;
      }
    }
  }

  {
    // lookfor lang only
    GList *list_lang = g_hash_table_lookup(nmgn->sep_conf_lang_by_lang, GINT_TO_POINTER(lang));
    for (GList *iter = list_lang; iter && !locale_found && !grouped_numbers; iter = iter->next)
    {
      struct number_sep_conf_locale *conf = iter->data;
      locale_found = TRUE;

      grouped_numbers = NlpMatchGroupNumbersByLocale(ctrl_nlp_th, conf->sep_conf);
      IF(grouped_numbers)
      {
        NlpThrowErrorTh(ctrl_nlp_th, "NlpMatchGroupNumbersWithLocale: failed on NlpMatchGroupNumbersByLocale "
            "with locale %s (lang)", OgIso639ToCode(conf->lang));
        DPcErr;
      }
    }
  }

  if (p_locale_found) *p_locale_found = locale_found;
  if (p_grouped_numbers) *p_grouped_numbers = grouped_numbers;

  DONE;
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
