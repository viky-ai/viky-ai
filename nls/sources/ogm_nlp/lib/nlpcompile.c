/*
 *  Main function for Natural Language Processing library
 *  Copyright (c) 2017 Pertimm, by Patrick Constant
 *  Dev : September 2017
 *  Version 1.0
 */
#include "ogm_nlp.h"

static int NlpCompilePackageIntents(og_nlp_th ctrl_nlp_th, json_t *json_id, json_t *json_intents);

static int NlpCompilePackageSentences(og_nlp_th ctrl_nlp_th, package_t package, struct intent *intent,
    json_t *json_sentences);
static int NlpCompilePackageSentence(og_nlp_th ctrl_nlp_th, package_t package, struct intent *intent, json_t *json_sentence);

PUBLIC(int) OgNlpCompile(og_nlp_th ctrl_nlp_th, struct og_nlp_compile_input *input, struct og_nlp_compile_output *output)
{

  // reset ouput
  memset(output, 0, sizeof(struct og_nlp_compile_output));

  og_char_buffer json_compile_request_string[DOgMlogMaxMessageSize / 2];
  IFE(NlpJsonToBuffer(input->json_input, json_compile_request_string, DOgMlogMaxMessageSize / 2, NULL));

  OgMsg(ctrl_nlp_th->hmsg, "", DOgMsgDestInLog, "OgNlpCompile: json_compile_request_string is [\n%s]",
      json_compile_request_string);

  // package input can be a json object (a package) or an array (a list of package)
  if (json_is_object(input->json_input))
  {
    IFE(NlpCompilePackage(ctrl_nlp_th, input, input->json_input));
  }
  else if (json_is_array(input->json_input))
  {
    int array_size = json_array_size(input->json_input);
    for (int i = 0; i < array_size; i++)
    {
      json_t *json_package = json_array_get(input->json_input, i);
      if (json_is_object(json_package))
      {
        IFE(NlpCompilePackage(ctrl_nlp_th, input, json_package));
      }
      else
      {
        NlpThrowErrorTh(ctrl_nlp_th, "OgNlpCompile: structure error : json_package at position %d is not an object", i);
        DPcErr;
      }
    }

  }
  else
  {
    NlpThrowErrorTh(ctrl_nlp_th, "OgNlpCompile: structure error : main container must be an array or an object");
    DPcErr;
  }

  // Just saying that compilation went well
  json_t *json = json_object();
  output->json_output = json;

  IFE(json_object_set_new(json, "compilation", json_string("ok")));

  DONE;
}

og_status NlpCompilePackage(og_nlp_th ctrl_nlp_th, struct og_nlp_compile_input *input, json_t *json_package)
{
  og_char_buffer json_package_string[DPcPathSize];
  IFE(NlpJsonToBuffer(json_package, json_package_string, DPcPathSize, NULL));
  OgMsg(ctrl_nlp_th->hmsg, "", DOgMsgDestInLog, "NlpCompilePackage: compiling package [\n%s]", json_package_string);

  json_t *json_id = NULL;
  json_t *json_intents = NULL;

  for (void *iter = json_object_iter(json_package); iter; iter = json_object_iter_next(json_package, iter))
  {
    og_string key = json_object_iter_key(iter);
    OgMsg(ctrl_nlp_th->hmsg, "", DOgMsgDestInLog, "NlpCompilePackage: found key='%s'", key);

    if (Ogstricmp(key, "id") == 0)
    {
      json_id = json_object_iter_value(iter);
    }
    else if (Ogstricmp(key, "intents") == 0)
    {
      json_intents = json_object_iter_value(iter);
    }
    else
    {
      NlpThrowErrorTh(ctrl_nlp_th, "NlpCompilePackage: unknow key '%s'", key);
      DPcErr;
    }
  }

  IFN(json_id)
  {
    NlpThrowErrorTh(ctrl_nlp_th, "NlpCompilePackage: package without key");
    DPcErr;
  }
  if (!json_is_string(json_id))
  {
    NlpThrowErrorTh(ctrl_nlp_th, "NlpCompilePackage: package 'id' is not a string");
    DPcErr;
  }

  // check unicity
  if (!input->package_update)
  {
    og_string package_id = json_string_value(json_id);
    package_t package = NlpPackageGet(ctrl_nlp_th, package_id);
    if (package != NULL)
    {
      NlpThrowErrorTh(ctrl_nlp_th, "NlpCompilePackage: package with id='%s' already exists, init must update package",
          package_id);
      DPcErr;
    }
  }

  IFN(json_intents)
  {
    NlpThrowErrorTh(ctrl_nlp_th, "NlpCompilePackage: no 'intents' in package");
    DPcErr;
  }

  IFE(NlpCompilePackageIntents(ctrl_nlp_th, json_id, json_intents));

  DONE;
}

static int NlpCompilePackageIntents(og_nlp_th ctrl_nlp_th, json_t *json_id, json_t *json_intents)
{

  if (!json_is_array(json_intents))
  {
    NlpThrowErrorTh(ctrl_nlp_th, "NlpCompilePackageIntents: 'intents' is not an array");
    DPcErr;
  }

  // At that point, we can create the package structure
  og_string string_id = json_string_value(json_id);
  OgMsg(ctrl_nlp_th->hmsg, "", DOgMsgDestInLog, "NlpCompilePackageIntents: package id is '%s'", string_id);

  // We do not use a package heap because we dont want synchronization on that heap
  package_t package = NlpPackageCreate(ctrl_nlp_th, string_id);
  IFN(package) DPcErr;

  int array_size = json_array_size(json_intents);
  for (int i = 0; i < array_size; i++)
  {
    json_t *json_intent = json_array_get(json_intents, i);
    if (json_is_object(json_intent))
    {
      IFE(NlpCompilePackageIntent(ctrl_nlp_th, package, json_intent));
    }
    else
    {
      NlpThrowErrorTh(ctrl_nlp_th, "NlpCompilePackageIntents: json_intent at position %d is not an object", i);
      DPcErr;
    }
  }

  IFE(NlpPackageAddOrReplace(ctrl_nlp_th, package));
  IFE(NlpPackageLog(ctrl_nlp_th, package));

  DONE;
}

og_status NlpCompilePackageIntent(og_nlp_th ctrl_nlp_th, package_t package, json_t *json_intent)
{
  og_char_buffer json_intent_string[DPcPathSize];
  IFE(NlpJsonToBuffer(json_intent, json_intent_string, DPcPathSize, NULL));
  OgMsg(ctrl_nlp_th->hmsg, "", DOgMsgDestInLog, "NlpCompilePackageIntent: compiling intent [\n%s]", json_intent_string);

  json_t *json_id = NULL;
  json_t *json_sentences = NULL;

  for (void *iter = json_object_iter(json_intent); iter; iter = json_object_iter_next(json_intent, iter))
  {
    og_string key = json_object_iter_key(iter);
    OgMsg(ctrl_nlp_th->hmsg, "", DOgMsgDestInLog, "NlpCompilePackageIntent: found key='%s'", key);

    if (Ogstricmp(key, "id") == 0)
    {
      json_id = json_object_iter_value(iter);
    }
    else if (Ogstricmp(key, "sentences") == 0)
    {
      json_sentences = json_object_iter_value(iter);
    }
    else
    {
      NlpThrowErrorTh(ctrl_nlp_th, "NlpCompilePackageIntent: unknow key '%s'", key);
      DPcErr;
    }

  }

  // intent id is mandatory
  IFN(json_id)
  {
    NlpThrowErrorTh(ctrl_nlp_th, "NlpCompilePackageIntent: intent has no 'id'");
    DPcErr;
  }
  if (!json_is_string(json_id))
  {
    NlpThrowErrorTh(ctrl_nlp_th, "NlpCompilePackageIntent: intent 'id' is not a string");
    DPcErr;
  }

  // intent sentences is mandatory
  IFN(json_sentences)
  {
    NlpThrowErrorTh(ctrl_nlp_th, "NlpCompilePackageIntent: intent has no 'sentences'");
    DPcErr;
  }
  if (!json_is_array(json_sentences))
  {
    NlpThrowErrorTh(ctrl_nlp_th, "NlpCompilePackageIntent: intent 'sentences' is not an array");
    DPcErr;
  }

  // At that point, we can create the intent structure
  const char *string_id = json_string_value(json_id);
  OgMsg(ctrl_nlp_th->hmsg, "", DOgMsgDestInLog, "NlpCompilePackageIntent: intent id is '%s'", string_id);

  size_t Iintent;
  struct intent *intent = OgHeapNewCell(package->hintent, &Iintent);
  IFn(intent) DPcErr;
  IFE(Iintent);
  intent->id_start = OgHeapGetCellsUsed(package->hba);
  intent->id_length = strlen(string_id);
  IFE(OgHeapAppend(package->hba, intent->id_length + 1, string_id));

  IFE(NlpCompilePackageSentences(ctrl_nlp_th, package, intent, json_sentences));

  DONE;
}

static int NlpCompilePackageSentences(og_nlp_th ctrl_nlp_th, package_t package, struct intent *intent,
    json_t *json_sentences)
{
  intent->sentence_start = OgHeapGetCellsUsed(package->hsentence);

  int array_size = json_array_size(json_sentences);
  for (int i = 0; i < array_size; i++)
  {
    json_t *json_sentence = json_array_get(json_sentences, i);
    IFN(json_sentence)
    {
      NlpThrowErrorTh(ctrl_nlp_th, "NlpCompilePackageSentences: null json_sentence at position %d", i);
      DPcErr;
    }
    if (json_is_object(json_sentence))
    {
      IFE(NlpCompilePackageSentence(ctrl_nlp_th, package, intent, json_sentence));
    }
    else
    {
      NlpThrowErrorTh(ctrl_nlp_th, "NlpCompilePackageSentences: json_sentence at position %d is not an object", i);
      DPcErr;
    }
  }

  intent->sentences_nb = OgHeapGetCellsUsed(package->hsentence) - intent->sentence_start;

  DONE;
}

static int NlpCompilePackageSentence(og_nlp_th ctrl_nlp_th, package_t package, struct intent *intent, json_t *json_sentence)
{
  og_char_buffer json_sentence_string[DPcPathSize];
  IFE(NlpJsonToBuffer(json_sentence, json_sentence_string, DPcPathSize, NULL));
  OgMsg(ctrl_nlp_th->hmsg, "", DOgMsgDestInLog, "NlpCompilePackageSentence: compiling sentence [\n%s]",
      json_sentence_string);

  json_t *json_text = NULL;
  json_t *json_locale = NULL;

  for (void *iter = json_object_iter(json_sentence); iter; iter = json_object_iter_next(json_sentence, iter))
  {
    const char *key = json_object_iter_key(iter);
    OgMsg(ctrl_nlp_th->hmsg, "", DOgMsgDestInLog, "NlpCompilePackageSentence: found key='%s'", key);
    if (Ogstricmp(key, "sentence") == 0)
    {
      json_text = json_object_iter_value(iter);
    }
    else if (Ogstricmp(key, "locale") == 0)
    {
      json_locale = json_object_iter_value(iter);
    }
    else
    {
      NlpThrowErrorTh(ctrl_nlp_th, "NlpCompilePackageSentence: unknow key '%s'", key);
      DPcErr;
    }
  }

  IFN(json_text)
  {
    NlpThrowErrorTh(ctrl_nlp_th, "NlpCompilePackageSentence: no text");
    DPcErr;
  }
  IFN(json_locale)
  {
    NlpThrowErrorTh(ctrl_nlp_th, "NlpCompilePackageSentence: no locale");
    DPcErr;
  }

  size_t Iphrase;
  struct sentence *sentence = OgHeapNewCell(package->hsentence, &Iphrase);
  IFn(sentence) DPcErr;
  IFE(Iphrase);

  if (json_is_string(json_text))
  {
    const char *string_text = json_string_value(json_text);
    sentence->text_start = OgHeapGetCellsUsed(package->hba);
    sentence->text_length = strlen(string_text);
    if (sentence->text_length > DOgNlpIntentPhraseMaxLength)
    {
      NlpThrowErrorTh(ctrl_nlp_th, "NlpCompilePackageSentence: text is too long");
      DPcErr;
    }
    IFE(OgHeapAppend(package->hba, sentence->text_length + 1, string_text));
  }
  else
  {
    NlpThrowErrorTh(ctrl_nlp_th, "NlpCompilePackageSentence: text is not a string");
    DPcErr;
  }

  sentence->locale = 0;
  if (json_is_string(json_locale))
  {
    const char *string_locale = json_string_value(json_locale);
    IFE(sentence->locale = OgCodeToIso639_3166(ctrl_nlp_th->herr, (char * )string_locale));
  }
  else if (json_is_null(json_locale))
  {
    sentence->locale = 0;
  }
  else
  {
    NlpThrowErrorTh(ctrl_nlp_th, "NlpCompilePackageIntent: locale is not a string");
    DPcErr;
  }

  DONE;
}

