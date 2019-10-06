/*
 *  Handling decimal number parsing with different locale convention
 *
 *  Copyright (c) 2017 Pertimm, by Sebastien Manfedini, Brice Ruzand, Patrick Constant
 *  Dev : January 2018, October 2019
 *  Version 1.1
 */
#include "ogm_nlp.h"

static og_status NlpNumberAddWord(og_nlp_th ctrl_nlp_th, struct request_word *rw_start, struct request_word *rw_end,
    double value);

/**
 * thousand and decimal separator configuration language agnostic
 */
struct number_sep_conf
{
  og_string thousand_sep;
  og_string decimal_sep;

  /** regex for these separator */
  GRegex *regex;

  /** Regex to guess locale without ambiguity */
  GRegex *regex_guess_locale_not_ambiguous;

  /** Regex to guess locale with some ambiguity */
  GRegex *regex_guess_locale_ambiguous;

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

static og_bool NlpMatchGroupNumbersAreThereConsecutiveNumbers(og_nlp_th ctrl_nlp_th);
static og_status NlpMatchGroupNumbersBuildLocaleList(og_nlp_th ctrl_nlp_th, GQueue *list_locale_to_try);
static struct number_sep_conf_locale *NlpMatchGroupNumbersGuessBestLocale(og_nlp_th ctrl_nlp_th,
    GQueue *list_locale_to_try);
static og_bool NlpMatchGroupNumbersWithLocale(og_nlp_th ctrl_nlp_th, struct number_sep_conf_locale* locale_conf);

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
  // list all separators except space
  ctrl_nlp_th->group_numbers_settings->all_separators = "'.,";

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

  // fr-FR, *-FR, fr-* | default (first declared)
  IFE(addLocaleConf(DOgLangFR, DOgCountryFR, conf_dot_coma));

  // en-US, *-US, en-*
  IFE(addLocaleConf(DOgLangEN, DOgCountryUS, conf_coma_dot));

  // de-DE, *-DE, de-*
  IFE(addLocaleConf(DOgLangDE, DOgCountryDE, conf_dot_coma));

  // *-CH
  IFE(addLocaleConf(DOgLangNil, DOgCountryCH, conf_quote_dot));

  DONE;
}

static void freeCountryByLang(gpointer glist)
{
  if (glist)
  {
    g_list_free((GList*) glist);
  }
}

og_status NlpMatchGroupNumbersInit(og_nlp_th ctrl_nlp_th)
{
  struct nlp_match_group_numbers *nmgn = ctrl_nlp_th->group_numbers_settings;
  memset(nmgn, 0, sizeof(struct nlp_match_group_numbers));

  nmgn->nlpth = ctrl_nlp_th;
  nmgn->sep_conf_lang_by_lang_country = g_hash_table_new_full(g_direct_hash, g_direct_equal, NULL, freeCountryByLang);

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
  gchar *escaped_all_sperators = g_regex_escape_string(nmgn->all_separators, -1);

  // match pattern in en-US (thousand_sep="," decimal_sep=".")
  // https://jex.im/regulex/#!flags=&re=%5E((%3F%3A(%3F%3A%5Cd%7B1%2C3%7D(%3F%3A%2C%5Cd%7B3%7D)%2B)%7C%5Cd%2B))(%3F%3A%5C.(%5Cd%2B))%3F%24
  og_char_buffer pattern[DPcPathSize];
  snprintf(pattern, DPcPathSize, "^((?:(?:\\d{1,3}(?:[%s ]\\d{3})+)|\\d+))(?:%s(\\d+))?$", escaped_thousand_sep,
      escaped_decimal_sep);

  // not_ambiguous pattern in en-US (thousand_sep="," decimal_sep=".")
  // https://jex.im/regulex/#!flags=&re=(%3F%3A%5E%7C%5B%5E%5Cd'.%2C%5D)(%3F%3A%5Cd%7B1%2C3%7D(%3F%3A%20%5Cd%7B3%7D)%2B%2C%5Cd%2B%7C%5Cd%7B1%2C3%7D(%3F%3A%20%5Cd%7B3%7D)%2B%20%5Cd%7B3%7D%7C%5Cd%2B%2C(%3F%3A%5Cd%7B1%2C2%7D%7C%5Cd%7B4%2C%7D))(%3F%3A%24%7C%5B%5E%5Cd'.%2C%5D)
  og_char_buffer guest_locale_not_ambiguous_pattern[DPcPathSize];
  snprintf(guest_locale_not_ambiguous_pattern, DPcPathSize, "(?:^|[^\\d%s])(?:\\d{1,3}(?:[%s ]\\d{3})+%s\\d+|"
      "\\d{1,3}(?:[%s ]\\d{3})+[%s ]\\d{3}|\\d+%s(?:\\d{1,2}|\\d{4,}))(?:$|[^\\d%s])", escaped_all_sperators,
      escaped_thousand_sep, escaped_decimal_sep, escaped_thousand_sep, escaped_thousand_sep, escaped_decimal_sep,
      escaped_all_sperators);

  // ambiguous pattern in en-US (thousand_sep="," decimal_sep=".")
  // https://jex.im/regulex/#!flags=&re=(%3F%3A%5E%7C%5B%5E%5Cd'%5C.%2C%5D)(%3F%3A(%3F%3A(%3F%3A%5Cd%7B1%2C3%7D(%3F%3A%2C%5Cd%7B3%7D)%2B)%7C%5Cd%2B)(%3F%3A%5C.%5Cd%2B)%3F)(%3F%3A%24%7C%5B%5E%5Cd'%5C.%2C%5D)
  og_char_buffer guest_locale_ambiguous_pattern[DPcPathSize];
  snprintf(guest_locale_ambiguous_pattern, DPcPathSize,
      "(?:^|[^\\d%s])(?:(?:(?:\\d{1,3}(?:[%s ]\\d{3})+)|\\d+)(?:%s\\d+)?)(?:$|[^\\d%s])", escaped_all_sperators,
      escaped_thousand_sep, escaped_decimal_sep, escaped_all_sperators);

  g_free(escaped_thousand_sep);
  g_free(escaped_decimal_sep);
  g_free(escaped_all_sperators);

  NlpLog(DOgNlpTraceGroupNumbers, "NlpMatchGroupNumbersInitAddSeparatorConf: "
      "thousand_sep=\"%s\", decimal_sep=\"%s\" :\n"
      "          match_pattern: %s\n"
      "          guest_locale_not_ambiguous_pattern: %s\n"
      "          guest_locale_ambiguous_pattern: %s", thousand_sep, decimal_sep, pattern,
      guest_locale_not_ambiguous_pattern, guest_locale_ambiguous_pattern);

  GError *regexp_error = NULL;
  conf->regex = g_regex_new(pattern, 0, 0, &regexp_error);
  if (!conf->regex || regexp_error)
  {
    NlpThrowErrorTh(nmgn->nlpth, "NlpMatchGroupNumbersInitAddSeparatorConf: g_regex_new failed on main : %s",
        regexp_error->message);
    g_error_free(regexp_error);
    return NULL;
  }

  conf->regex_guess_locale_not_ambiguous = g_regex_new(guest_locale_not_ambiguous_pattern, 0, 0, &regexp_error);
  if (!conf->regex_guess_locale_not_ambiguous || regexp_error)
  {
    NlpThrowErrorTh(nmgn->nlpth, "NlpMatchGroupNumbersInitAddSeparatorConf: g_regex_new failed on not_ambiguous : %s",
        regexp_error->message);
    g_error_free(regexp_error);
    return NULL;
  }

  conf->regex_guess_locale_ambiguous = g_regex_new(guest_locale_ambiguous_pattern, 0, 0, &regexp_error);
  if (!conf->regex_guess_locale_ambiguous || regexp_error)
  {
    NlpThrowErrorTh(nmgn->nlpth, "NlpMatchGroupNumbersInitAddSeparatorConf: g_regex_new failed on ambiguous : %s",
        regexp_error->message);
    g_error_free(regexp_error);
    return NULL;
  }

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

  struct number_sep_conf_locale* conf_locale = g_slice_new0(struct number_sep_conf_locale);
  conf_locale->lang = lang;
  conf_locale->country = country;
  conf_locale->lang_country = locale_lang_country(lang, country);
  conf_locale->sep_conf = sep_conf;

  // add conf in list
  g_queue_push_tail(nmgn->sep_conf_lang, conf_locale);

  // add conf in lookup table for fast access

  // lookup by lang_country
  if (lang != DOgLangNil && country != DOgCountryNil)
  {
    int key = locale_lang_country(lang, country);
    IFE(NlpMatchGNInitHashInsert(nmgn->sep_conf_lang_by_lang_country, conf_locale, key));

    if (ctrl_nlp_th->loginfo->trace & DOgNlpTraceGroupNumbers)
    {
      og_char_buffer current_lang_country[DPcPathSize];
      NlpLog(DOgNlpTraceGroupNumbers, "NlpMatchGroupNumbersInitAssociateLocaleWithConf: %s "
          "(thousand_sep=\"%s\", decimal_sep=\"%s\")", OgIso639_3166ToCode(key, current_lang_country),
          sep_conf->thousand_sep, sep_conf->decimal_sep);
    }
  }

  // lookup by lang
  if (lang != DOgLangNil)
  {
    int key = locale_lang_country(lang, DOgCountryNil);
    IFE(NlpMatchGNInitHashInsert(nmgn->sep_conf_lang_by_lang_country, conf_locale, key));

    if (ctrl_nlp_th->loginfo->trace & DOgNlpTraceGroupNumbers)
    {
      og_char_buffer current_lang_country[DPcPathSize];
      NlpLog(DOgNlpTraceGroupNumbers, "NlpMatchGroupNumbersInitAssociateLocaleWithConf: %s    "
          "(thousand_sep=\"%s\", decimal_sep=\"%s\")", OgIso639_3166ToCode(key, current_lang_country),
          sep_conf->thousand_sep, sep_conf->decimal_sep);
    }
  }

  // lookup by country
  if (country != DOgCountryNil)
  {
    int key = locale_lang_country(DOgLangNil, country);
    IFE(NlpMatchGNInitHashInsert(nmgn->sep_conf_lang_by_lang_country, conf_locale, key));

    if (ctrl_nlp_th->loginfo->trace & DOgNlpTraceGroupNumbers)
    {
      og_char_buffer current_lang_country[DPcPathSize];
      NlpLog(DOgNlpTraceGroupNumbers, "NlpMatchGroupNumbersInitAssociateLocaleWithConf: %s "
          "(thousand_sep=\"%s\", decimal_sep=\"%s\")", OgIso639_3166ToCode(key, current_lang_country),
          sep_conf->thousand_sep, sep_conf->decimal_sep);
    }
  }

  DONE;
}

og_status NlpMatchGroupNumbersFlush(og_nlp_th ctrl_nlp_th)
{

  struct nlp_match_group_numbers *nmgn = ctrl_nlp_th->group_numbers_settings;

  // free all lookup hash
  g_hash_table_destroy(nmgn->sep_conf_lang_by_lang_country);
  nmgn->sep_conf_lang_by_lang_country = NULL;

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

    g_regex_unref(conf->regex_guess_locale_not_ambiguous);
    conf->regex_guess_locale_not_ambiguous = NULL;

    g_regex_unref(conf->regex_guess_locale_ambiguous);
    conf->regex_guess_locale_ambiguous = NULL;

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

og_status NlpMatchGroupNumbers(og_nlp_th ctrl_nlp_th)
{
  og_bool need_to_check_group_number = NlpMatchGroupNumbersAreThereConsecutiveNumbers(ctrl_nlp_th);
  IF(need_to_check_group_number)
  {
    NlpThrowErrorTh(ctrl_nlp_th, "NlpMatchGroupNumbers: failed on NlpMatchGroupNumbersAreThereConsecutiveNumbers");
    DPcErr;
  }

  // skip GroupNumbers process if not needed
  if (!need_to_check_group_number) CONT;

  // look for all available locale
  IFE(NlpMatchGroupNumbersMatchReset(ctrl_nlp_th));

  og_bool numbers_grouped = FALSE;

  if (ctrl_nlp_th->loginfo->trace & DOgNlpTraceMatch)
  {
    IFE(NlpLogRequestWords(ctrl_nlp_th));
  }

  /** list of struct number_sep_conf_locale*/
  GQueue list_locale_to_try[1];
  g_queue_init(list_locale_to_try);

  NlpMatchGroupNumbersBuildLocaleList(ctrl_nlp_th, list_locale_to_try);
  struct number_sep_conf_locale *conf_locale = NlpMatchGroupNumbersGuessBestLocale(ctrl_nlp_th, list_locale_to_try);

  // free list
  g_queue_clear(list_locale_to_try);

  if (ctrl_nlp_th->loginfo->trace & DOgNlpTraceMatch)
  {
    IFE(NlpLogRequestWords(ctrl_nlp_th));
  }

  if (conf_locale)
  {

    numbers_grouped = NlpMatchGroupNumbersWithLocale(ctrl_nlp_th, conf_locale);
    IF(numbers_grouped)
    {
      og_char_buffer current_lang_country[DPcPathSize];
      NlpThrowErrorTh(ctrl_nlp_th, "NlpMatchGroupNumbers: failed on NlpMatchGroupNumbersWithLocale "
          "with locale %s", OgIso639_3166ToCode(conf_locale->lang_country, current_lang_country));
      DPcErr;
    }

  }

  return numbers_grouped;
}

static og_bool NlpMatchGroupNumbersAreThereConsecutiveNumbers(og_nlp_th ctrl_nlp_th)
{

  int request_word_used = OgHeapGetCellsUsed(ctrl_nlp_th->hrequest_word);
  if (request_word_used > 0)
  {

    struct request_word *fisrt_rw = OgHeapGetCell(ctrl_nlp_th->hrequest_word, 0);
    IFN(fisrt_rw) DPcErr;

    int nb_consecutive_number = 0;
    for (struct request_word *rw = fisrt_rw; rw; rw = rw->next)
    {
      if (rw->is_number)
      {
        nb_consecutive_number += 1;
      }
      else if (rw->is_punctuation)
      {
        // do nothing
      }
      else
      {
        nb_consecutive_number = 0;
      }

      if (nb_consecutive_number > 1)
      {
        return TRUE;
      }

    }

  }

  return FALSE;
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
  str_remove_inplace(value_buffer, " ");

  // append decimal part
  if (decimalPartMatch && decimalPartLength > 0)
  {
    snprintf(value_buffer + strlen(value_buffer), DPcPathSize, ".%.*s", decimalPartLength, sentence + decimalPartStart);
  }

  errno = 0;
  double tmp_value = strtod(value_buffer, NULL);
  if (errno > 0)
  {
    // tmp_value will be HUGE_VALF processed as null by jansson

    // integer part is already warn in nlpparse.c
    if (decimalPartLength > 0)
    {
      NlpWarningAdd(ctrl_nlp_th, "NlpMatchGroupNumbersParsing: number \"%s\" (at %d:%d) cannot be parsed as a decimal number (%s),"
          " interpretation's solution may be wrong.", value_buffer, integerPartStart, decimalPartLength, strerror(errno));
    }
  }

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

static og_bool NlpMatchGroupNumbersWithLocale(og_nlp_th ctrl_nlp_th, struct number_sep_conf_locale* locale_conf)
{

  og_string request_sentence = ctrl_nlp_th->request_sentence;

  // parsing the numbers
  double value = 0.0;
  int request_word_used = OgHeapGetCellsUsed(ctrl_nlp_th->hrequest_word);
  if (request_word_used == 0) return FALSE;

  struct request_word *rw_start_number = NULL;
  struct request_word *rw_end_number = NULL;
  double value_number = 0.0;

  struct request_word *rw_start = OgHeapGetCell(ctrl_nlp_th->hrequest_word, 0);
  IFN(rw_start)
  {
    NlpThrowErrorTh(ctrl_nlp_th, "NlpMatchGroupNumbersWithLocale: "
        "OgHeapGetCell(ctrl_nlp_th->hrequest_word, 0) failed");
    DPcErr;
  }

  for (struct request_word *rw_end = rw_start; rw_end; rw_end = rw_end->next)
  {

    // ignore non basic word (build from ltras)
    if (rw_end->self_index >= ctrl_nlp_th->basic_request_word_used) continue;

    og_string string_request_word = OgHeapGetCell(ctrl_nlp_th->hba, rw_end->start);
    if (!string_request_word)
    {
      NlpThrowErrorTh(ctrl_nlp_th, "NlpMatchGroupNumbersWithLocale: failed "
          "OgHeapGetCell(ctrl_nlp_th->hba, rw_end->start)");
      DPcErr;
    }

    // on ne prend pas en compte les séparateurs en fin de nombre
    if (!strcmp(string_request_word, locale_conf->sep_conf->thousand_sep)
        || !strcmp(string_request_word, locale_conf->sep_conf->decimal_sep))
    {
      continue;
    }

    // recuperer la string entre NumberGroupStart et NumberGroupEnd
    og_char_buffer expression_string[DPcPathSize];
    int start_position = rw_start->start_position;
    int end_position = rw_end->start_position + rw_end->length_position;
    int length = end_position - start_position;
    snprintf(expression_string, DPcPathSize, "%.*s", length, request_sentence + start_position);

    og_bool number_match = NlpMatchGroupNumbersParsing(ctrl_nlp_th, expression_string, locale_conf->sep_conf, &value);
    IFE(number_match);

    if (number_match)
    {
      if (ctrl_nlp_th->loginfo->trace & DOgNlpTraceGroupNumbers)
      {
        og_char_buffer current_lang_country[DPcPathSize];
        NlpLog(DOgNlpTraceGroupNumbers,
            "NlpMatchGroupNumbersWithLocale: \"%s\" " "find group number " DOgPrintDouble " (locale=\"%s\" thousand_sep=\"%s\", decimal_sep=\"%s\")",
            expression_string, value, OgIso639_3166ToCode(locale_conf->lang_country, current_lang_country),
            locale_conf->sep_conf->thousand_sep, locale_conf->sep_conf->decimal_sep);
      }

      rw_start_number = rw_start;
      rw_end_number = rw_end;
      value_number = value;
    }
    else
    {
      if (rw_start_number != NULL)
      {
        IFE(NlpNumberAddWord(ctrl_nlp_th, rw_start_number, rw_end_number, value_number));
        rw_start_number = NULL;
      }
      rw_start = rw_end;
    }
  }

  if (rw_start_number != NULL)
  {
    IFE(NlpNumberAddWord(ctrl_nlp_th, rw_start_number, rw_end_number, value_number));
  }

  DONE;
}

static og_status NlpMatchGroupNumbersBuildLocaleList(og_nlp_th ctrl_nlp_th, GQueue *list_locale_to_try)
{

  struct nlp_match_group_numbers *nmgn = ctrl_nlp_th->group_numbers_settings;

  og_bool look_for_all_language = FALSE;

  // check the given accepted languages
  int numberLang = OgHeapGetCellsUsed(ctrl_nlp_th->haccept_language);

  if (numberLang <= 0)
  {
    look_for_all_language = TRUE;
  }
  else
  {

    // try to guess best locale in accepted languages
    for (int i = 0; i < numberLang; i++)
    {
      struct accept_language* accepted_language = OgHeapGetCell(ctrl_nlp_th->haccept_language, i);

      if (accepted_language->locale == DOgLangNil)
      {
        look_for_all_language = TRUE;
        break;
      }

      int lang_country = accepted_language->locale;
      int lang = OgIso639_3166ToLang(lang_country);
      int country = OgIso639_3166ToCountry(lang_country);

      og_bool locale_found = FALSE;

      // lookfor lang + country
      if (lang != DOgLangNil && country != DOgCountryNil && !locale_found)
      {
        gconstpointer key = GINT_TO_POINTER(lang_country);
        GList *list_lang_country = g_hash_table_lookup(nmgn->sep_conf_lang_by_lang_country, key);
        for (GList *iter = list_lang_country; iter; iter = iter->next)
        {
          struct number_sep_conf_locale *conf = iter->data;

          if (conf->sep_conf->already_processed) continue;
          conf->sep_conf->already_processed = TRUE;

          g_queue_push_tail(list_locale_to_try, conf);

          locale_found = TRUE;
          break;
        }
      }

      // lookfor country only
      if (country != DOgCountryNil && !locale_found)
      {
        gconstpointer key = GINT_TO_POINTER(locale_lang_country(DOgLangNil, country));
        GList *list_country = g_hash_table_lookup(nmgn->sep_conf_lang_by_lang_country, key);
        for (GList *iter = list_country; iter; iter = iter->next)
        {
          struct number_sep_conf_locale *conf = iter->data;

          if (conf->sep_conf->already_processed) continue;
          conf->sep_conf->already_processed = TRUE;

          g_queue_push_tail(list_locale_to_try, conf);

          locale_found = TRUE;
          break;
        }
      }

      // lookfor lang only
      if (lang != DOgLangNil && !locale_found)
      {
        gconstpointer key = GINT_TO_POINTER(locale_lang_country(lang, DOgCountryNil));
        GList *list_lang = g_hash_table_lookup(nmgn->sep_conf_lang_by_lang_country, key);
        for (GList *iter = list_lang; iter; iter = iter->next)
        {
          struct number_sep_conf_locale *conf = iter->data;

          if (conf->sep_conf->already_processed) continue;
          conf->sep_conf->already_processed = TRUE;

          g_queue_push_tail(list_locale_to_try, conf);

          locale_found = TRUE;
          break;
        }
      }
    }

  }

  if (look_for_all_language)
  {
    // add all others conf
    for (GList *iter = nmgn->sep_conf_lang->head; iter; iter = iter->next)
    {
      struct number_sep_conf_locale *conf = iter->data;
      if (conf->sep_conf->already_processed) continue;
      conf->sep_conf->already_processed = TRUE;

      g_queue_push_tail(list_locale_to_try, conf);
    }
  }

  DONE;
}

static struct number_sep_conf_locale *NlpMatchGroupNumbersGuessBestLocale(og_nlp_th ctrl_nlp_th,
    GQueue *list_locale_to_try)
{
  og_string request_sentence = ctrl_nlp_th->request_sentence;

  struct number_sep_conf_locale *best_locale_conf = NULL;

  og_bool not_ambiguous = FALSE;

  for (GList *iter = list_locale_to_try->head; iter && !best_locale_conf; iter = iter->next)
  {
    struct number_sep_conf_locale *conf = iter->data;

    og_bool match = g_regex_match(conf->sep_conf->regex_guess_locale_not_ambiguous, request_sentence, 0, NULL);
    if (match)
    {
      best_locale_conf = conf;
      not_ambiguous = TRUE;
    }
  }

  for (GList *iter = list_locale_to_try->head; iter && !best_locale_conf; iter = iter->next)
  {
    struct number_sep_conf_locale *conf = iter->data;

    og_bool match = g_regex_match(conf->sep_conf->regex_guess_locale_ambiguous, request_sentence, 0, NULL);
    if (match)
    {
      best_locale_conf = conf;
    }
  }

  if (ctrl_nlp_th->loginfo->trace & DOgNlpTraceGroupNumbers && best_locale_conf)
  {
    unsigned char found_accept_language[DPcPathSize];
    NlpAcceptLanguageString(ctrl_nlp_th, DPcPathSize, found_accept_language);

    og_char_buffer current_lang_country[DPcPathSize];
    NlpLog(DOgNlpTraceGroupNumbers, "NlpMatchGroupNumbersGuessBestLocale: sentence: \"%s\" accept_language: \"%s\" "
        "find group number %s ambiguous conf (locale=\"%s\" thousand_sep=\"%s\", decimal_sep=\"%s\")", request_sentence,
        found_accept_language, not_ambiguous ? "NOT" : "",
        OgIso639_3166ToCode(best_locale_conf->lang_country, current_lang_country),
        best_locale_conf->sep_conf->thousand_sep, best_locale_conf->sep_conf->decimal_sep);
  }

  return best_locale_conf;
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

static og_status NlpNumberAddWord(og_nlp_th ctrl_nlp_th, struct request_word *rw_start, struct request_word *rw_end,
    double value)
{
  og_string request_sentence = ctrl_nlp_th->request_sentence;

  for (struct request_word *rw = rw_start; rw != rw_end; rw = rw->next)
  {
    rw->is_number = FALSE;
  }
  rw_end->is_number = FALSE;

  og_char_buffer expression_string[DPcPathSize];
  int start_position = rw_start->start_position;
  int end_position = rw_end->start_position + rw_end->length_position;
  int length_position = end_position - start_position;
  snprintf(expression_string, DPcPathSize, "%.*s", length_position, request_sentence + start_position);
  int iexpression_string = strlen(expression_string);

  NlpLog(DOgNlpTraceMatch, "NlpNumberAddWord: adding word '%.*s' at start %d", iexpression_string, expression_string, start_position)

  og_char_buffer normalized_word[DPcPathSize];
  snprintf(normalized_word, DPcPathSize, DOgPrintDouble, value);
  int inormalized_word = strlen(normalized_word);
  NlpLog(DOgNlpTraceMatch, "NlpNumberAddWord: normalized word '%.*s'", inormalized_word, normalized_word)

  size_t Irequest_word;
  struct request_word *request_word = OgHeapNewCell(ctrl_nlp_th->hrequest_word, &Irequest_word);
  IFn(request_word) DPcErr;
  IF(Irequest_word) DPcErr;

  request_word->self_index = Irequest_word;

  request_word->start = OgHeapGetCellsUsed(ctrl_nlp_th->hba);
  request_word->length = inormalized_word;
  IFE(OgHeapAppend(ctrl_nlp_th->hba, request_word->length, normalized_word));
  IFE(OgHeapAppend(ctrl_nlp_th->hba, 1, ""));

  request_word->raw_start = OgHeapGetCellsUsed(ctrl_nlp_th->hba);
  request_word->raw_length = iexpression_string;
  IFE(OgHeapAppend(ctrl_nlp_th->hba, request_word->raw_length, expression_string));
  IFE(OgHeapAppend(ctrl_nlp_th->hba, 1, ""));

  request_word->start_position = start_position;
  request_word->length_position = length_position;

  request_word->is_number = TRUE;
  request_word->number_value = value;
  request_word->is_punctuation = FALSE;
  request_word->is_auto_complete_word = FALSE;
  request_word->is_regex = FALSE;
  request_word->regex_input_part = NULL;
  request_word->spelling_score = 1.0;
  request_word->lang_id = DOgLangNil;

  int nb_matched_words = 1;
  for (struct request_word *rw = rw_start; rw != rw_end; rw = rw->next)
  {
    nb_matched_words++;
  }

  request_word->nb_matched_words = nb_matched_words;

  DONE;
}

