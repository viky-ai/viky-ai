/*
 *  Handling log messages for Natural Language Processing library
 *  Copyright (c) 2017 Pertimm, by Patrick Constant
 *  Dev : September 2017
 *  Version 1.0
 */
#include "ogm_nlp.h"

struct punctuation_word
{
  int length;
  char *string;
};

static og_bool NlpParseIsSpace(int c);
static og_bool NlpParseIsPunctuation(int c);
static og_bool NlpParseIsSpaceOrPunctuation(int c);
static int NlpParseIsPunctuationWord(int is, og_string s);
static og_status NlpParseAddWord(og_nlp_th ctrl_nlp_th, int word_start, int word_length);

/**
 *  The request sentence is in : ctrl_nlp_th->request_sentence
 *  we want to parse the sentence. For the moment, nothing fancy
 *  we simply use space and some punctuations as separator
 */
og_status NlpParse(og_nlp_th ctrl_nlp_th)
{
  og_string s = ctrl_nlp_th->request_sentence;
  int is = strlen(s);

  for (int i = 0, state = 1, start = 0, end = 0; !end; i++)
  {
    int c = ' ';
    if (i < is)
    {
      c = s[i];
    }
    else
    {
      end = 1;
      c = ' ';
    }

    switch (state)
    {
      case 1:   // between words
      {
        int length = 0;
        if ((length = NlpParseIsPunctuationWord(is - i, s + i)))
        {
          IFE(NlpParseAddWord(ctrl_nlp_th, i, length));
          i += length - 1;
        }
        else if (NlpParseIsSpaceOrPunctuation(c))
        {
          state = 1;
        }
        else
        {
          start = i;
          state = 2;
        }
        break;
      }
      case 2:   // in word
      {
        int length = 0;
        if ((length = NlpParseIsPunctuationWord(is - i, s + i)))
        {
          IFE(NlpParseAddWord(ctrl_nlp_th, start, i - start));
          IFE(NlpParseAddWord(ctrl_nlp_th, i, length));
          i += length - 1;
          state = 1;
        }
        else if (NlpParseIsSpaceOrPunctuation(c))
        {
          IFE(NlpParseAddWord(ctrl_nlp_th, start, i - start));
          state = 1;
        }
        else
        {
          state = 2;
        }

        break;
      }
    }

  }
  DONE;
}

static og_bool NlpParseIsSpace(int c)
{
  if (OgUniIsspace(c)) return TRUE;
  return FALSE;
}

static og_bool NlpParseIsPunctuation(int c)
{
  int punctuation[] = { ',', '\'', 0 };
  for (int i = 0; punctuation[i]; i++)
  {
    if (c == punctuation[i]) return TRUE;
  }
  return FALSE;
}

static og_bool NlpParseIsSpaceOrPunctuation(int c)
{
  if (NlpParseIsSpace(c)) return TRUE;
  if (NlpParseIsPunctuation(c)) return TRUE;
  return (FALSE);
}

static int NlpParseIsPunctuationWord(int is, og_string s)
{
  struct punctuation_word punctuation_word[] = { { 1, "&" },   //
      { 1, "+" },   //
      { 1, "-" },   //
      { 1, "=" },   //
      { 1, "<" },   //
      { 2, "<=" },   //
      { 1, ">" },   //
      { 2, ">=" },   //
      { 2, "<>" },   //
      { 2, "!=" },   //
      { 1, "*" },   //
      { 1, "/" },   //
      { 0, "" }   //
  };
  for (int i = 0; punctuation_word[i].length; i++)
  {
    if (is >= punctuation_word[i].length && !memcmp(s, punctuation_word[i].string, punctuation_word[i].length)) return punctuation_word[i].length;
  }
  return 0;
}

static og_status NlpParseAddWord(og_nlp_th ctrl_nlp_th, int word_start, int word_length)
{
  og_string s = ctrl_nlp_th->request_sentence;

  if (ctrl_nlp_th->loginfo->trace & DOgNlpTraceMatch)
  {
    OgMsg(ctrl_nlp_th->hmsg, "", DOgMsgDestInLog, "NlpParseAddWord: adding word '%.*s' at start %d", word_length,
        s + word_start, word_start);
  }

  size_t Irequest_word;
  struct request_word *request_word = OgHeapNewCell(ctrl_nlp_th->hrequest_word, &Irequest_word);
  IFn(request_word) DPcErr;
  IF(Irequest_word) DPcErr;

  request_word->start = OgHeapGetCellsUsed(ctrl_nlp_th->hba);
  request_word->length = word_length;
  IFE(OgHeapAppend(ctrl_nlp_th->hba, request_word->length, s + word_start));
  IFE(OgHeapAppend(ctrl_nlp_th->hba, 1, ""));
  request_word->start_position = word_start;
  request_word->length_position = word_length;

  DONE;
}

