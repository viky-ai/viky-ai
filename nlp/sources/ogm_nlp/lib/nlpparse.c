/*
 *  Handling log messages for Natural Language Processing library
 *  Copyright (c) 2017 Pertimm, by Patrick Constant
 *  Dev : September 2017
 *  Version 1.0
 */
#include "ogm_nlp.h"

static inline og_bool NlpParseUnicharIsSkipPunctuation(struct og_nlp_parse_conf *parse_conf, gunichar c);
static inline int NlpParseIsPunctuationInternal(struct og_nlp_parse_conf *parse_conf, int max_word_size,
    og_string current_word, og_bool *p_skip);

static og_status NlpParseAddWord(og_nlp_th ctrl_nlp_th, int word_start, int word_length);

/**
 *  The request sentence is in : ctrl_nlp_th->request_sentence
 *  we want to parse the sentence. For the moment, nothing fancy
 *  we simply use space and some punctuations as separator
 */
og_status NlpParseRequestSentence(og_nlp_th ctrl_nlp_th)
{
  struct og_nlp_parse_conf *parse_conf = ctrl_nlp_th->ctrl_nlp->parse_conf;

  og_string s = ctrl_nlp_th->request_sentence;
  int is = strlen(s);
  og_string s_end = s + is;

  int state = 1;
  og_bool end = FALSE;
  int i = -1;
  int start = 0;
  while (!end)
  {
    gunichar c = ' ';
    og_string s_pos = g_utf8_find_next_char(s + i, s_end);
    if (s_pos == NULL)
    {
      end = TRUE;
      i = is;
    }
    else
    {
      i = s_pos - s;
      c = g_utf8_get_char_validated(s_pos, is - i);
      if ((c == (gunichar) -1) || (c == (gunichar) -2))
      {
        NlpThrowErrorTh(ctrl_nlp_th, "NlpParseRequestSentence : invalid UTF-8 character '%s'",
            ctrl_nlp_th->request_sentence);
        DPcErr;
      }
    }

    switch (state)
    {
      case 1:   // between words
      {

        og_bool is_skipped = FALSE;
        int length = NlpParseIsPunctuationInternal(parse_conf, is - i, s + i, &is_skipped);
        IFE(length);
        if (length > 0)
        {
          if (!is_skipped)
          {
            // add punctuation word
            IFE(NlpParseAddWord(ctrl_nlp_th, i, length));
          }
          state = 1;
          i += length - 1;
        }
        else if (g_unichar_isdigit(c))
        {
          start = i;
          state = 3;
        }
        else
        {
          start = i;
          state = 2;
        }
        break;
      }
      case 2:   // in word not digit
      {
        og_bool is_skipped = FALSE;
        int length = NlpParseIsPunctuationInternal(parse_conf, is - i, s + i, &is_skipped);
        IFE(length);
        if (length > 0)
        {
          // add previously word
          IFE(NlpParseAddWord(ctrl_nlp_th, start, i - start));

          if (!is_skipped)
          {
            // add punctuation word
            IFE(NlpParseAddWord(ctrl_nlp_th, i, length));
          }
          state = 1;
          i += length - 1;
        }
        else if (g_unichar_isdigit(c))
        {
          // add previously word
          IFE(NlpParseAddWord(ctrl_nlp_th, start, i - start));

          i -= 1;
          start = i;
          state = 1;
        }
        break;
      }
      case 3:   // in digit word
      {
        if (!g_unichar_isdigit(c))
        {
          // add previously digit word
          IFE(NlpParseAddWord(ctrl_nlp_th, start, i - start));

          i -= 1;
          start = i;
          state = 1;
        }
        break;
      }
    }

  }

  // This is necessary to keep this information as we add word to the list through ltras
  ctrl_nlp_th->basic_request_word_used = OgHeapGetCellsUsed(ctrl_nlp_th->hrequest_word);

  DONE;
}

static og_status NlpParseAddPunctWord(og_nlp ctrl_nlp, og_string utf8_word)
{
  struct og_nlp_parse_conf *parse_conf = ctrl_nlp->parse_conf;

  int utf8_word_size = strlen(utf8_word);
  if (g_utf8_find_next_char(utf8_word, utf8_word + utf8_word_size) == NULL)
  {
    // single unicode char treated as word

    if (parse_conf->punct_char_word_used >= DOgNlpParsePunctCharMaxNb)
    {
      NlpThrowError(ctrl_nlp, "NlpParseAddPunctWord : too many punctuation char treated as word %d",
      DOgNlpParsePunctCharMaxNb);
      DPcErr;
    }

    parse_conf->punct_char_word[parse_conf->punct_char_word_used++] = g_utf8_get_char(utf8_word);

  }
  else
  {
    // multiple char word
    if (parse_conf->punct_word_used >= DOgNlpParsePunctWordMaxNb)
    {
      NlpThrowError(ctrl_nlp, "NlpParseAddPunctWord : too many punctuation word %d", DOgNlpParsePunctWordMaxNb);
      DPcErr;
    }

    struct og_nlp_punctuation_word *punct_word = parse_conf->punct_word + parse_conf->punct_word_used++;
    punct_word->length = strlen(utf8_word);
    punct_word->string = strdup(utf8_word);
  }

  DONE;
}

static int NlpParseConfPuncWordSort(const void *void_word1, const void *void_word2)
{
  const struct og_nlp_punctuation_word *word1 = void_word1;
  const struct og_nlp_punctuation_word *word2 = void_word2;

  return word2->length - word1->length;
}

og_status NlpParseConfInit(og_nlp ctrl_nlp)
{

  // add punct treated as word
  IFE(NlpParseAddPunctWord(ctrl_nlp, "<="));
  IFE(NlpParseAddPunctWord(ctrl_nlp, ">="));
  IFE(NlpParseAddPunctWord(ctrl_nlp, "<>"));
  IFE(NlpParseAddPunctWord(ctrl_nlp, "!="));

  // sort longer word fist
  qsort(ctrl_nlp->parse_conf->punct_word, DOgNlpParsePunctWordMaxNb, sizeof(struct og_nlp_punctuation_word),
      NlpParseConfPuncWordSort);

  DONE;
}

og_status NlpParseConfFlush(og_nlp ctrl_nlp)
{
  struct og_nlp_parse_conf *parse_conf = ctrl_nlp->parse_conf;

  for (int i = 0; i < parse_conf->punct_word_used; i++)
  {
    struct og_nlp_punctuation_word *punct_word = parse_conf->punct_word + i;
    unsigned char *word = (unsigned char *) punct_word->string;
    punct_word->string = NULL;
    DPcFree(word);
  }

  DONE;
}

static inline og_bool NlpParseUnicharIsSkipPunctuation(struct og_nlp_parse_conf *parse_conf, gunichar c)
{
  // space char
  if (g_unichar_isspace(c))
  {
    return TRUE;
  }

  // unicode define punctuation word
  og_bool skip_punct = FALSE;
  GUnicodeType type = g_unichar_type(c);
  switch (type)
  {
    case G_UNICODE_CONTROL:
    case G_UNICODE_FORMAT:
    case G_UNICODE_UNASSIGNED:
    case G_UNICODE_PRIVATE_USE:
    case G_UNICODE_SURROGATE:
      skip_punct = TRUE;
      break;
    default:
      break;
  }
  if (skip_punct)
  {
    return TRUE;
  }

  return FALSE;
}

static inline int NlpParseIsPunctuationInternal(struct og_nlp_parse_conf *parse_conf, int max_word_size,
    og_string current_word, og_bool *p_skip)
{
  if (p_skip) *p_skip = FALSE;

  // treat end of string as skipped punct
  if (max_word_size <= 0 || current_word[0] == '\0')
  {
    if (p_skip) *p_skip = TRUE;
    return 1;
  }

  for (int i = 0; i < parse_conf->punct_word_used; i++)
  {
    struct og_nlp_punctuation_word *punct_word = parse_conf->punct_word + i;
    if (max_word_size >= punct_word->length && !memcmp(current_word, punct_word->string, punct_word->length))
    {
      return punct_word->length;
    }
  }

  gunichar first_char = g_utf8_get_char_validated(current_word, max_word_size);
  if (first_char > 0)
  {
    og_bool single_punct_word_found = FALSE;

    // check skipped puntation
    if (NlpParseUnicharIsSkipPunctuation(parse_conf, first_char))
    {
      single_punct_word_found = TRUE;
      if (p_skip) *p_skip = TRUE;
    }

    if (!single_punct_word_found)
    {
      // code define single char punctuation word
      for (int i = 0; i < parse_conf->punct_char_word_used; i++)
      {
        if (first_char == parse_conf->punct_char_word[i])
        {
          single_punct_word_found = TRUE;
          break;
        }
      }
    }

    if (!single_punct_word_found)
    {
      // unicode define punctuation word
      GUnicodeType type = g_unichar_type(first_char);
      switch (type)
      {
        case G_UNICODE_OTHER_NUMBER:
        case G_UNICODE_CONNECT_PUNCTUATION:
        case G_UNICODE_DASH_PUNCTUATION:
        case G_UNICODE_OPEN_PUNCTUATION:
        case G_UNICODE_CLOSE_PUNCTUATION:
        case G_UNICODE_INITIAL_PUNCTUATION:
        case G_UNICODE_FINAL_PUNCTUATION:
        case G_UNICODE_OTHER_PUNCTUATION:
        case G_UNICODE_CURRENCY_SYMBOL:
        case G_UNICODE_MODIFIER_SYMBOL:
        case G_UNICODE_MATH_SYMBOL:
        case G_UNICODE_OTHER_SYMBOL:
          single_punct_word_found = TRUE;
          break;
        default:
          break;
      }
    }

    if (!single_punct_word_found)
    {
      GUnicodeBreakType break_type = g_unichar_break_type(first_char);
      switch (break_type)
      {
        case G_UNICODE_BREAK_IDEOGRAPHIC:
        case G_UNICODE_BREAK_EMOJI_BASE:
        case G_UNICODE_BREAK_EMOJI_MODIFIER:
          single_punct_word_found = TRUE;
          break;
        default:
          break;
      }
    }

    if (single_punct_word_found)
    {
      og_string current_word_pos = g_utf8_find_next_char(current_word, current_word + max_word_size);
      if (current_word_pos == NULL)
      {
        return max_word_size;
      }
      else
      {
        return current_word_pos - current_word;
      }
    }

  }

  return 0;
}

og_bool NlpParseIsPunctuation(og_nlp_th ctrl_nlp_th, int max_word_size, og_string current_word, og_bool *p_skip,
    int *p_punct_length_bytes)
{
  if (p_punct_length_bytes) *p_punct_length_bytes = 0;

  struct og_nlp_parse_conf *parse_conf = ctrl_nlp_th->ctrl_nlp->parse_conf;

  int word_length_bytes = NlpParseIsPunctuationInternal(parse_conf, max_word_size, current_word, p_skip);
  IFE(word_length_bytes);
  if (word_length_bytes > 0)
  {
    if (p_punct_length_bytes) *p_punct_length_bytes = word_length_bytes;
    return TRUE;
  }

  return FALSE;
}

static og_status NlpParseAddWord(og_nlp_th ctrl_nlp_th, int word_start, int word_length)
{
  og_string s = ctrl_nlp_th->request_sentence;

  NlpLog(DOgNlpTraceMatch, "NlpParseAddWord: adding word '%.*s' at start %d", word_length, s + word_start, word_start)

  char normalized_string_word[DPcPathSize];
  int length_normalized_string_word = OgUtf8Normalize(word_length, s + word_start, DPcPathSize, normalized_string_word);
  NlpLog(DOgNlpTraceConsolidate, "NlpConsolidateAddWord: normalized word '%.*s'", length_normalized_string_word,
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

  request_word->is_auto_complete_word = FALSE;
  request_word->is_regex = FALSE;
  request_word->Iregex = -1;

  request_word->is_number = TRUE;
  for (int i = 0; i < length_normalized_string_word; i++)
  {
    if (!OgUniIsdigit(normalized_string_word[i]))
    {
      request_word->is_number = FALSE;
      break;
    }
  }
  if (request_word->is_number)
  {
    request_word->number_value = atoi(normalized_string_word);
  }
  request_word->is_auto_complete_word = FALSE;

  request_word->spelling_score = 1.0;

  request_word->is_punctuation = FALSE;
  og_bool punct_length = 0;
  og_bool is_skipped = FALSE;
  og_bool is_punct = NlpParseIsPunctuation(ctrl_nlp_th, word_length, s + word_start, &is_skipped, &punct_length);
  IFE(is_punct);
  if (is_punct)
  {
    request_word->is_punctuation = TRUE;
  }
  request_word->nb_matched_words = 1;

  DONE;
}

