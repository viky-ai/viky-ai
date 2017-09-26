/*
 *  Main function for Natural Language Processing library
 *  Copyright (c) 2017 Pertimm, by Patrick Constant
 *  Dev : September 2017
 *  Version 1.0
 */
#include "ogm_nlp.h"

static int NlpCompilePackage(og_nlp ctrl_nlp, json_t *json_package);
static int NlpCompilePackageIntents(og_nlp ctrl_nlp, json_t *json_id, json_t *json_intents);
static int NlpCompilePackageIntent(package_t package, json_t *json_intent);
static int NlpCompilePackagePhrases(package_t package, struct intent *intent, json_t *json_phrases);
static int NlpCompilePackagePhrase(package_t package, struct intent *intent, json_t *json_phrase);

PUBLIC(int) OgNlpCompile(og_nlp ctrl_nlp, struct og_nlp_compile_input *input, struct og_nlp_compile_output *output)
{

  ctrl_nlp->json_compile_request_string = json_dumps(input->json_input, JSON_INDENT(2));
  IFN(ctrl_nlp->json_compile_request_string)
  {
    NlpThrowError(ctrl_nlp, "OgNlpCompile: json_dumps error on input->json_input");
    DPcErr;
  }
  ctrl_nlp->json_compile_request_string_length = strlen(ctrl_nlp->json_compile_request_string);
  ctrl_nlp->json_compile_request_string_length_truncated = ctrl_nlp->json_compile_request_string_length;
  if (ctrl_nlp->json_compile_request_string_length_truncated > DOgMlogMaxMessageSize / 2)
  {
    ctrl_nlp->json_compile_request_string_length_truncated = DOgMlogMaxMessageSize / 2;
  }

  OgMsg(ctrl_nlp->hmsg, "", DOgMsgDestInLog, "OgNlpCompile: json_compile_request_string is [\n%.*s]",
      ctrl_nlp->json_compile_request_string_length_truncated, ctrl_nlp->json_compile_request_string);

  if (json_is_object(input->json_input))
  {
    IFE(NlpCompilePackage(ctrl_nlp, input->json_input));
  }
  else if (json_is_array(input->json_input))
  {
    int array_size = json_array_size(input->json_input);
    for (int i = 0; i < array_size; i++)
    {
      json_t *json_package = json_array_get(input->json_input, i);
      IFN(json_package)
      {
        NlpThrowError(ctrl_nlp, "OgNlpCompile: null json_package at position %d", i);
        DPcErr;
      }
      if (json_is_object(json_package))
      {
        IFE(NlpCompilePackage(ctrl_nlp, input->json_input));
      }
      else
      {
        NlpThrowError(ctrl_nlp, "OgNlpCompile: json_package at position %d is not an object", i);
        DPcErr;
      }
    }
  }
  else
  {
    NlpThrowError(ctrl_nlp, "OgNlpCompile: input->json_input is not an object nor an array");
    DPcErr;
  }

  // Just saying that compilation went well
  json_t *json = json_object();
  output->json_output = json;

  json_t *value = json_string("ok");
  IFE(json_object_set_new(json, "compilation", value));

  DONE;
}

static int NlpCompilePackage(og_nlp ctrl_nlp, json_t *json_package)
{
  char *json_package_string = json_dumps(json_package, JSON_INDENT(2));

  OgMsg(ctrl_nlp->hmsg, "", DOgMsgDestInLog, "NlpCompilePackage: compiling package [\n%.*s]",
      strlen(json_package_string), json_package_string);

  void *package_iter = json_object_iter(json_package);
  IFN(package_iter)
  {
    NlpThrowError(ctrl_nlp, "NlpCompilePackage: package is empty");
    DPcErr;
  }
  json_t *json_id = NULL;
  json_t *json_intents = NULL;
  do
  {
    const char *key = json_object_iter_key(package_iter);
    OgMsg(ctrl_nlp->hmsg, "", DOgMsgDestInLog, "NlpCompilePackage: found key='%s'", key);
    if (Ogstricmp(key, "id") == 0)
    {
      json_id = json_object_iter_value(package_iter);
    }
    else if (Ogstricmp(key, "intents") == 0)
    {
      json_intents = json_object_iter_value(package_iter);
    }
    else
    {
      NlpThrowError(ctrl_nlp, "NlpCompilePackage: unknow key '%s'", key);
      DPcErr;
    }
  }
  while ((package_iter = json_object_iter_next(json_package, package_iter)) != NULL);

  IFN(json_id)
  {
    NlpThrowError(ctrl_nlp, "NlpCompilePackage: no key");
    DPcErr;
  }
  IFN(json_intents)
  {
    NlpThrowError(ctrl_nlp, "NlpCompilePackage: no intents");
    DPcErr;
  }

  IFE(NlpCompilePackageIntents(ctrl_nlp, json_id, json_intents));

  // A la fin, on parcours la liste des package (liste sur les pointeurs package) et on compare l'id
  // si on trouve un même id, on swap le package dans la liste (synchro) et on détruit l'ancien package
  // sinon on ajoute le package dans la liste des package (donc un tas slicé).

  DONE;
}

static int NlpCompilePackageIntents(og_nlp ctrl_nlp, json_t *json_id, json_t *json_intents)
{
  if (!json_is_string(json_id))
  {
    NlpThrowError(ctrl_nlp, "NlpCompilePackageIntents: id is not a string");
    DPcErr;
  }
  if (!json_is_array(json_intents))
  {
    NlpThrowError(ctrl_nlp, "NlpCompilePackageIntents: intents is not an array");
    DPcErr;
  }
  // At that point, we can create the package structure
  const char *string_id = json_string_value(json_id);
  OgMsg(ctrl_nlp->hmsg, "", DOgMsgDestInLog, "NlpCompilePackageIntents: package id is '%s'", string_id);

  // We do not use a package heap because we dont want synchronization on that heap
  package_t package = NlpPackageCreate(ctrl_nlp, string_id);
  IFN(package) DPcErr;

  int array_size = json_array_size(json_intents);
  for (int i = 0; i < array_size; i++)
  {
    json_t *json_intent = json_array_get(json_intents, i);
    IFN(json_intent)
    {
      NlpThrowError(ctrl_nlp, "NlpCompilePackageIntents: null json_intent at position %d", i);
      DPcErr;
    }
    if (json_is_object(json_intent))
    {
      IFE(NlpCompilePackageIntent(package, json_intent));
    }
    else
    {
      NlpThrowError(ctrl_nlp, "NlpCompilePackageIntents: json_intent at position %d is not an object", i);
      DPcErr;
    }
  }

  IFE(NlpPackageLog(package));

  DONE;
}

static int NlpCompilePackageIntent(package_t package, json_t *json_intent)
{
  og_nlp ctrl_nlp = package->ctrl_nlp;
  char *json_intent_string = json_dumps(json_intent, JSON_INDENT(2));

  OgMsg(ctrl_nlp->hmsg, "", DOgMsgDestInLog, "NlpCompilePackageIntent: compiling intent [\n%.*s]",
      strlen(json_intent_string), json_intent_string);

  void *intent_iter = json_object_iter(json_intent);
  IFN(intent_iter)
  {
    NlpThrowError(ctrl_nlp, "NlpCompilePackageIntent: intent is empty");
    DPcErr;
  }
  json_t *json_id = NULL;
  json_t *json_phrases = NULL;
  do
  {
    const char *key = json_object_iter_key(intent_iter);
    OgMsg(ctrl_nlp->hmsg, "", DOgMsgDestInLog, "NlpCompilePackageIntent: found key='%s'", key);
    if (Ogstricmp(key, "id") == 0)
    {
      json_id = json_object_iter_value(intent_iter);
    }
    else if (Ogstricmp(key, "phrases") == 0)
    {
      json_phrases = json_object_iter_value(intent_iter);
    }
    else
    {
      NlpThrowError(ctrl_nlp, "NlpCompilePackageIntent: unknow key '%s'", key);
      DPcErr;
    }
  }
  while ((intent_iter = json_object_iter_next(json_intent, intent_iter)) != NULL);

  IFN(json_id)
  {
    NlpThrowError(ctrl_nlp, "NlpCompilePackageIntent: no id");
    DPcErr;
  }
  IFN(json_phrases)
  {
    NlpThrowError(ctrl_nlp, "NlpCompilePackageIntent: no phrases");
    DPcErr;
  }

  if (!json_is_string(json_id))
  {
    NlpThrowError(ctrl_nlp, "NlpCompilePackageIntent: id is not a string");
    DPcErr;
  }
  if (!json_is_array(json_phrases))
  {
    NlpThrowError(ctrl_nlp, "NlpCompilePackageIntent: phrases is not an array");
    DPcErr;
  }
  // At that point, we can create the intent structure
  const char *string_id = json_string_value(json_id);
  OgMsg(ctrl_nlp->hmsg, "", DOgMsgDestInLog, "NlpCompilePackageIntent: intent id is '%s'", string_id);

  size_t Iintent;
  struct intent *intent = OgHeapNewCell(package->hintent, &Iintent);
  IFn(intent) DPcErr;
  IFE(Iintent);
  intent->id_start = OgHeapGetCellsUsed(package->hba);
  intent->id_length = strlen(string_id);
  IFE(OgHeapAppend(package->hba, intent->id_length + 1, string_id));

  IFE(NlpCompilePackagePhrases(package, intent, json_phrases));

  DONE;
}

static int NlpCompilePackagePhrases(package_t package, struct intent *intent, json_t *json_phrases)
{
  og_nlp ctrl_nlp = package->ctrl_nlp;

  intent->phrase_start = OgHeapGetCellsUsed(package->hphrase);

  int array_size = json_array_size(json_phrases);
  for (int i = 0; i < array_size; i++)
  {
    json_t *json_phrase = json_array_get(json_phrases, i);
    IFN(json_phrase)
    {
      NlpThrowError(ctrl_nlp, "NlpCompilePackagePhrases: null json_phrase at position %d", i);
      DPcErr;
    }
    if (json_is_object(json_phrase))
    {
      IFE(NlpCompilePackagePhrase(package, intent, json_phrase));
    }
    else
    {
      NlpThrowError(ctrl_nlp, "NlpCompilePackagePhrases: json_phrase at position %d is not an object", i);
      DPcErr;
    }
  }

  intent->phrases_nb = OgHeapGetCellsUsed(package->hphrase) - intent->phrase_start;

  DONE;
}

static int NlpCompilePackagePhrase(package_t package, struct intent *intent, json_t *json_phrase)
{
  og_nlp ctrl_nlp = package->ctrl_nlp;
  char *json_phrase_string = json_dumps(json_phrase, JSON_INDENT(2));

  OgMsg(ctrl_nlp->hmsg, "", DOgMsgDestInLog, "NlpCompilePackagePhrase: compiling phrase [\n%.*s]",
      strlen(json_phrase_string), json_phrase_string);

  void *intent_iter = json_object_iter(json_phrase);
  IFN(intent_iter)
  {
    NlpThrowError(ctrl_nlp, "NlpCompilePackagePhrase: phrase is empty");
    DPcErr;
  }
  json_t *json_text = NULL;
  json_t *json_locale = NULL;
  do
  {
    const char *key = json_object_iter_key(intent_iter);
    OgMsg(ctrl_nlp->hmsg, "", DOgMsgDestInLog, "NlpCompilePackagePhrase: found key='%s'", key);
    if (Ogstricmp(key, "text") == 0)
    {
      json_text = json_object_iter_value(intent_iter);
    }
    else if (Ogstricmp(key, "locale") == 0)
    {
      json_locale = json_object_iter_value(intent_iter);
    }
    else
    {
      NlpThrowError(ctrl_nlp, "NlpCompilePackagePhrase: unknow key '%s'", key);
      DPcErr;
    }
  }
  while ((intent_iter = json_object_iter_next(json_phrase, intent_iter)) != NULL);

  IFN(json_text)
  {
    NlpThrowError(ctrl_nlp, "NlpCompilePackagePhrase: no text");
    DPcErr;
  }
  IFN(json_locale)
  {
    NlpThrowError(ctrl_nlp, "NlpCompilePackagePhrase: no locale");
    DPcErr;
  }

  size_t Iphrase;
  struct phrase *phrase = OgHeapNewCell(package->hphrase, &Iphrase);
  IFn(phrase) DPcErr;
  IFE(Iphrase);

  if (json_is_string(json_text))
  {
    const char *string_text = json_string_value(json_text);
    phrase->text_start = OgHeapGetCellsUsed(package->hba);
    phrase->text_length = strlen(string_text);

    if(phrase->text_length > DOgNlpIntentPhraseMaxLength)
    {
      NlpThrowError(ctrl_nlp, "NlpCompilePackagePhrase: text is too long");
      DPcErr;
    }


    IFE(OgHeapAppend(package->hba, package->id_length + 1, string_text));
  }
  else
  {
    NlpThrowError(ctrl_nlp, "NlpCompilePackagePhrase: text is not a string");
    DPcErr;
  }

  phrase->locale = 0;
  if (json_is_string(json_locale))
  {
    const char *string_locale = json_string_value(json_locale);
    IFE(phrase->locale=OgCodeToIso639_3166(ctrl_nlp->herr,(char *)string_locale));
  }
  else if (json_is_null(json_locale))
  {
    phrase->locale = 0;
  }
  else
  {
    NlpThrowError(ctrl_nlp, "NlpCompilePackageIntent: locale is not a string");
    DPcErr;
  }

  DONE;
}

