/*
 *  Handling regex expressions for NLP
 *  Copyright (c) 2018 Pertimm, by Patrick Constant & Sebastien Manfredini
 *  Dev : November 2018
 *  Version 1.0
 */
#include "ogm_nlp.h"

static og_status NlpRegexBuildInterpretation(og_nlp_th ctrl_nlp_th, package_t package,
    struct interpretation *interpretation);
static og_status NlpRegexBuildExpression(og_nlp_th ctrl_nlp_th, package_t package, struct expression *expression);
static og_status NlpRegexBuildAlias(og_nlp_th ctrl_nlp_th, package_t package, struct alias *alias);
static og_status NlpRegexCompile(og_nlp_th ctrl_nlp_th, struct regex *regex);
static og_status NlpRegexAddWordsPackage(og_nlp_th ctrl_nlp_th, struct interpret_package *interpret_package);
static og_status NlpRegexAddWord(og_nlp_th ctrl_nlp_th, int word_start, int word_length, int Iregex, package_t package);

og_status NlpRegexInit(og_nlp_th ctrl_nlp_th, package_t package)
{
  og_char_buffer heap_name[DPcPathSize];
  void *hmsg = ctrl_nlp_th->ctrl_nlp->hmsg;
  snprintf(heap_name, DPcPathSize, "regex_%s", package->id);
  IFn(package->hregex = OgHeapInit(hmsg, heap_name, sizeof(struct regex), 1)) DPcErr;
  DONE;
}

og_status NlpRegexFlush(package_t package)
{
  // flusher les regex avant de flusher la heap sinon fuite mémoire
  int regexNumber = OgHeapGetCellsUsed(package->hregex);
  for(int i=0;i<regexNumber; i++)
  {
    struct regex *regex = OgHeapGetCell(package->hregex, i);
    if(regex->regex)
    {
      g_regex_unref(regex->regex);
    }
  }
  IFE(OgHeapFlush(package->hregex));
  DONE;
}

og_status NlpRegexBuildPackage(og_nlp_th ctrl_nlp_th, package_t package)
{
  int interpretation_used = OgHeapGetCellsUsed(package->hinterpretation);
  for (int i = 0; i < interpretation_used; i++)
  {
    struct interpretation *interpretation = OgHeapGetCell(package->hinterpretation, i);
    IFN(interpretation) DPcErr;

    IFE(NlpRegexBuildInterpretation(ctrl_nlp_th, package, interpretation));
  }

  NlpRegexPackageLog(ctrl_nlp_th, package);

  DONE;
}

static og_status NlpRegexBuildInterpretation(og_nlp_th ctrl_nlp_th, package_t package,
    struct interpretation *interpretation)
{
  IFN(interpretation) DPcErr;

  for (int i = 0; i < interpretation->expressions_nb; i++)
  {
    IFE(NlpRegexBuildExpression(ctrl_nlp_th, package, interpretation->expressions + i));
  }

  DONE;
}

static og_status NlpRegexBuildExpression(og_nlp_th ctrl_nlp_th, package_t package, struct expression *expression)
{
  IFN(expression) DPcErr;

  for (int i = 0; i < expression->aliases_nb; i++)
  {
    IFE(NlpRegexBuildAlias(ctrl_nlp_th, package, expression->aliases + i));
  }

  DONE;
}

static og_status NlpRegexBuildAlias(og_nlp_th ctrl_nlp_th, package_t package, struct alias *alias)
{
  IFN(alias) DPcErr;
  if (alias->type != nlp_alias_type_Regex) DONE;

  int regex_used = OgHeapGetCellsUsed(package->hregex);
  if (regex_used >= DOgNlpMaximumRegex)
  {
    NlpThrowErrorTh(ctrl_nlp_th, "NlpRegexBuildAlias: maximum regex number reached: %d", regex_used);
    DPcErr;
  }

  OgMsg(ctrl_nlp_th->hmsg, "", DOgMsgDestInLog, "NlpRegexBuildAlias: found alias '%s' %s='%s'", alias->alias,
      NlpAliasTypeString(alias->type), alias->regex);

  struct regex regex[1];
  regex->alias = alias;
  regex->regex = NULL;

  IFE(NlpRegexCompile(ctrl_nlp_th, regex));

  IFE(OgHeapAppend(package->hregex, 1, regex));

  DONE;
}

static og_status NlpRegexCompile(og_nlp_th ctrl_nlp_th, struct regex *regex)
{
  GError *regexp_error = NULL;
  regex->regex = g_regex_new(regex->alias->regex, 0, 0, &regexp_error);
  if (!regex->regex || regexp_error)
  {
    NlpThrowErrorTh(ctrl_nlp_th, "NlpMatchGroupNumbersInitAddSeparatorConf: g_regex_new failed on main : %s",
        regexp_error->message);
    g_error_free(regexp_error);
    DPcErr;
  }
  DONE;
}



og_status NlpMatchRegexes(og_nlp_th ctrl_nlp_th)
{
  int interpret_package_used = OgHeapGetCellsUsed(ctrl_nlp_th->hinterpret_package);
  struct interpret_package *interpret_packages = OgHeapGetCell(ctrl_nlp_th->hinterpret_package, 0);
  IFN(interpret_packages) DPcErr;
  for (int i = 0; i < interpret_package_used; i++)
  {
    struct interpret_package *interpret_package = interpret_packages + i;
    og_bool status = NlpRegexAddWordsPackage(ctrl_nlp_th, interpret_package);
    IFE(status);
  }


  DONE;
}

static og_status NlpRegexAddWordsPackage(og_nlp_th ctrl_nlp_th, struct interpret_package *interpret_package)
{
  og_string sentence = ctrl_nlp_th->request_sentence;

  og_bool found = FALSE;
  og_string *matchedSentence = NULL;
  int start_match_position = 0;
  int end_match_position = 0;

  int regexNumber = OgHeapGetCellsUsed(interpret_package->package->hregex);
  for(int i=0; i<regexNumber; i++)
  {
    struct regex *regex = OgHeapGetCell(interpret_package->package->hregex, i);
    if(regex->regex)
    {
      // match the regular expression
      GMatchInfo *match_info = NULL;
      GError *regexp_error = NULL;
      og_bool match = g_regex_match_all_full(regex->regex, sentence, -1, 0, 0, &match_info, &regexp_error);
      if (regexp_error)
      {
        NlpThrowErrorTh(ctrl_nlp_th, "NlpRegexAddWordsPackage: g_regex_match_all_full failed on execution : %s",
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
        matchedSentence = (og_string *)g_match_info_get_string(match_info);
        g_match_info_fetch_pos (match_info, 0, &start_match_position, &end_match_position);

        if (ctrl_nlp_th->loginfo->trace & DOgNlpTraceMatch)
        {
          OgMsg(ctrl_nlp_th->hmsg, "", DOgMsgDestInLog, "NlpRegexAddWordsPackage: matched sentence with regex '%s' start=%d end=%d",matchedSentence,start_match_position,end_match_position);
        }
        IFE(NlpRegexAddWord(ctrl_nlp_th, start_match_position, end_match_position-start_match_position, i, interpret_package->package));

        g_match_info_free(match_info);
        match_info = NULL;
        found = TRUE;
      }

      // ensure match_info freeing
      if (match_info != NULL)
      {
        g_match_info_free(match_info);
      }

    }
  }

  if(found)
  {
    // ajouter à la réponse
  }

  DONE;
}



static og_status NlpRegexAddWord(og_nlp_th ctrl_nlp_th, int word_start, int word_length, int Iregex, package_t package)
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
  request_word->Iregex = Iregex;
  request_word->regex_package = package;

  DONE;
}






og_status NlpRegexPackageLog(og_nlp_th ctrl_nlp_th, package_t package)
{
  int regex_used = OgHeapGetCellsUsed(package->hregex);
  struct regex *regexes = OgHeapGetCell(package->hregex,0);

  for (int i = 0; i < regex_used; i++)
  {
    struct regex *regex = regexes + i;
    struct alias *alias = regex->alias;
    OgMsg(ctrl_nlp_th->hmsg, "", DOgMsgDestInLog, "regex '%s' '%s'", alias->alias, alias->regex);
  }
  DONE;
}





