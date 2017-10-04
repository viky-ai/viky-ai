/*
 *  Main function for Natural Language Processing library
 *  Copyright (c) 2017 Pertimm, by Patrick Constant
 *  Dev : September 2017
 *  Version 1.0
 */
#include "ogm_nlp.h"

static int NlpCompilePackage(og_nlp ctrl_nlp, json_t *json_package, struct packages_intents_hashtables *p_i_hash);
static int NlpCompilePackageIntents(og_nlp ctrl_nlp, json_t *json_id, json_t *json_intents, struct packages_intents_hashtables *p_i_hash);
static int NlpCompilePackageIntent(package_t package, json_t *json_intent, struct packages_intents_hashtables *p_i_hash);
static int NlpCompilePackageSentences(package_t package, struct intent *intent, json_t *json_sentences);
static int NlpCompilePackageSentence(package_t package, struct intent *intent, json_t *json_sentence);
// static int NlpControlImportFile(og_nlp ctrl_nlp, json_t *json, char *import_file);

PUBLIC(int) OgNlpCompile(og_nlp ctrl_nlp, struct og_nlp_compile_input *input, struct og_nlp_compile_output *output)
{

  ctrl_nlp->json_compile_request_string = json_dumps(input->json_input, JSON_INDENT(2));
  IFN(ctrl_nlp->json_compile_request_string)
  {
    NlpThrowError(ctrl_nlp, "OgNlpCompile: json_dumps error on input->json_input for file %s", input->filename);
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
    NlpThrowError(ctrl_nlp, "OgNlpCompile: structure error in file %s : main container must be an array", input->filename);
    DPcErr;
  }
  else if (json_is_array(input->json_input))
  {
    int array_size = json_array_size(input->json_input);

    struct packages_intents_hashtables p_i_hash;
    p_i_hash.package_id_hastable = g_hash_table_new(g_str_hash,g_str_equal);
    p_i_hash.intent_id_hastable = g_hash_table_new(g_str_hash,g_str_equal);
    p_i_hash.filename = input->filename;

    for (int i = 0; i < array_size; i++)
    {
      json_t *json_package = json_array_get(input->json_input, i);
      IFN(json_package)
      {
        NlpThrowError(ctrl_nlp, "OgNlpCompile: null json_package at position %d", i);
        g_hash_table_destroy(p_i_hash.package_id_hastable);
        g_hash_table_destroy(p_i_hash.intent_id_hastable);
        DPcErr;
      }
      if (json_is_object(json_package))
      {
        og_bool compiledPackage = NlpCompilePackage(ctrl_nlp, json_package, &p_i_hash);
        if(compiledPackage == ERREUR)
        {
          g_hash_table_destroy(p_i_hash.package_id_hastable);
          g_hash_table_destroy(p_i_hash.intent_id_hastable);
          DPcErr;
        }
      }
      else
      {
        NlpThrowError(ctrl_nlp, "OgNlpCompile: structure error in file %s : json_package at position %d is not an object", input->filename, i);
        g_hash_table_destroy(p_i_hash.package_id_hastable);
        g_hash_table_destroy(p_i_hash.intent_id_hastable);
        DPcErr;
      }
    }
    g_hash_table_destroy(p_i_hash.package_id_hastable);
    g_hash_table_destroy(p_i_hash.intent_id_hastable);
  }
  else
  {
    NlpThrowError(ctrl_nlp, "OgNlpCompile: structure error in file %s : main container must be an array", input->filename);
    DPcErr;
  }

  // Just saying that compilation went well
  json_t *json = json_object();
  output->json_output = json;

  json_t *value = json_string("ok");
  IFE(json_object_set_new(json, "compilation", value));

  DONE;
}

static int NlpCompilePackage(og_nlp ctrl_nlp, json_t *json_package, struct packages_intents_hashtables *p_i_hash)
{
  char *json_package_string = json_dumps(json_package, JSON_INDENT(2));

  OgMsg(ctrl_nlp->hmsg, "", DOgMsgDestInLog, "NlpCompilePackage: compiling package [\n%.*s]",
      strlen(json_package_string), json_package_string);

  void *package_iter = json_object_iter(json_package);
  IFN(package_iter)
  {
    NlpThrowError(ctrl_nlp, "NlpCompilePackage: package is empty in file %s", p_i_hash->filename);
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
      if(json_is_string(json_id) != FALSE)
      {
        const char* package_id_string = json_string_value(json_id);
        if(g_hash_table_lookup (p_i_hash->package_id_hastable,package_id_string) == NULL)
        {
          g_hash_table_insert(p_i_hash->package_id_hastable,g_strdup (package_id_string),"");
        }
        else
        {
          NlpThrowError(ctrl_nlp, "NlpCompilePackage: file %s, package id %s already exist", p_i_hash->filename, package_id_string);
          DPcErr;
        }
      }
      else
      {
        NlpThrowError(ctrl_nlp, "NlpCompilePackage: file %s, package id is not a string", p_i_hash->filename);
        DPcErr;
      }
    }
    else if (Ogstricmp(key, "intents") == 0)
    {
      json_intents = json_object_iter_value(package_iter);
    }
    else
    {
      NlpThrowError(ctrl_nlp, "NlpCompilePackage: file %s, unknow key '%s'", p_i_hash->filename, key);
      DPcErr;
    }
  }
  while ((package_iter = json_object_iter_next(json_package, package_iter)) != NULL);

  IFN(json_id)
  {
    NlpThrowError(ctrl_nlp, "NlpCompilePackage: file %s, package without key", p_i_hash->filename);
    DPcErr;
  }
  IFN(json_intents)
  {
    NlpThrowError(ctrl_nlp, "NlpCompilePackage: file %s, no intents in package", p_i_hash->filename);
    DPcErr;
  }

  IFE(NlpCompilePackageIntents(ctrl_nlp, json_id, json_intents, p_i_hash));

  DONE;
}

static int NlpCompilePackageIntents(og_nlp ctrl_nlp, json_t *json_id, json_t *json_intents, struct packages_intents_hashtables *p_i_hash)
{
  if (!json_is_string(json_id))
  {
    NlpThrowError(ctrl_nlp, "NlpCompilePackageIntents: file %s, id is not a string", p_i_hash->filename);
    DPcErr;
  }
  if (!json_is_array(json_intents))
  {
    NlpThrowError(ctrl_nlp, "NlpCompilePackageIntents: file %s, intents is not an array", p_i_hash->filename);
    DPcErr;
  }
  // At that point, we can create the package structure
  const char *string_id = json_string_value(json_id);
  OgMsg(ctrl_nlp->hmsg, "", DOgMsgDestInLog, "NlpCompilePackageIntents: file %s, package id is '%s'", p_i_hash->filename, string_id);

  // We do not use a package heap because we dont want synchronization on that heap
  package_t package = NlpPackageCreate(ctrl_nlp, string_id);
  IFN(package) DPcErr;

  int array_size = json_array_size(json_intents);
  for (int i = 0; i < array_size; i++)
  {
    json_t *json_intent = json_array_get(json_intents, i);
    IFN(json_intent)
    {
      NlpThrowError(ctrl_nlp, "NlpCompilePackageIntents: file %s, null json_intent at position %d", p_i_hash->filename, i);
      DPcErr;
    }
    if (json_is_object(json_intent))
    {
      IFE(NlpCompilePackageIntent(package, json_intent, p_i_hash));
    }
    else
    {
      NlpThrowError(ctrl_nlp, "NlpCompilePackageIntents: file %s, json_intent at position %d is not an object", p_i_hash->filename, i);
      DPcErr;
    }
  }

  IFE(NlpPackageAdd(ctrl_nlp, package));
  IFE(NlpPackageLog(package));

  DONE;
}

static int NlpCompilePackageIntent(package_t package, json_t *json_intent, struct packages_intents_hashtables *p_i_hash)
{
  og_nlp ctrl_nlp = package->ctrl_nlp;
  char *json_intent_string = json_dumps(json_intent, JSON_INDENT(2));

  OgMsg(ctrl_nlp->hmsg, "", DOgMsgDestInLog, "NlpCompilePackageIntent: file %s, compiling intent [\n%.*s]",
      p_i_hash->filename, strlen(json_intent_string), json_intent_string);

  void *intent_iter = json_object_iter(json_intent);
  IFN(intent_iter)
  {
    NlpThrowError(ctrl_nlp, "NlpCompilePackageIntent: file %s, intent is empty", p_i_hash->filename);
    DPcErr;
  }
  json_t *json_id = NULL;
  json_t *json_sentences = NULL;
  do
  {
    const char *key = json_object_iter_key(intent_iter);
    OgMsg(ctrl_nlp->hmsg, "", DOgMsgDestInLog, "NlpCompilePackageIntent: file %s, found key='%s'", p_i_hash->filename, key);
    if (Ogstricmp(key, "id") == 0)
    {
      json_id = json_object_iter_value(intent_iter);

      if(json_is_string(json_id) != FALSE)
      {
        const char* intent_id_string = json_string_value(json_id);
        if(g_hash_table_lookup (p_i_hash->intent_id_hastable,intent_id_string) == NULL)
        {
          g_hash_table_insert(p_i_hash->intent_id_hastable,g_strdup (intent_id_string),"");
        }
        else
        {
          NlpThrowError(ctrl_nlp, "NlpCompilePackageIntent: file %s, intent id %s already exist", p_i_hash->filename, intent_id_string);
          DPcErr;
        }
      }
      else
      {
        NlpThrowError(ctrl_nlp, "NlpCompilePackageIntent: file %s, intent id is not a string", p_i_hash->filename);
        DPcErr;
      }


    }
    else if (Ogstricmp(key, "sentences") == 0)
    {
      json_sentences = json_object_iter_value(intent_iter);
    }
    else
    {
      NlpThrowError(ctrl_nlp, "NlpCompilePackageIntent: file %s, unknow key '%s'", p_i_hash->filename, key);
      DPcErr;
    }
  }
  while ((intent_iter = json_object_iter_next(json_intent, intent_iter)) != NULL);

  IFN(json_id)
  {
    NlpThrowError(ctrl_nlp, "NlpCompilePackageIntent: file %s, no id", p_i_hash->filename);
    DPcErr;
  }
  IFN(json_sentences)
  {
    NlpThrowError(ctrl_nlp, "NlpCompilePackageIntent: file %s, no sentences", p_i_hash->filename);
    DPcErr;
  }

  if (!json_is_string(json_id))
  {
    NlpThrowError(ctrl_nlp, "NlpCompilePackageIntent: file %s, id is not a string", p_i_hash->filename);
    DPcErr;
  }
  if (!json_is_array(json_sentences))
  {
    NlpThrowError(ctrl_nlp, "NlpCompilePackageIntent: file %s, sentences is not an array", p_i_hash->filename);
    DPcErr;
  }
  // At that point, we can create the intent structure
  const char *string_id = json_string_value(json_id);
  OgMsg(ctrl_nlp->hmsg, "", DOgMsgDestInLog, "NlpCompilePackageIntent: file %s, intent id is '%s'", p_i_hash->filename, string_id);

  size_t Iintent;
  struct intent *intent = OgHeapNewCell(package->hintent, &Iintent);
  IFn(intent) DPcErr;
  IFE(Iintent);
  intent->id_start = OgHeapGetCellsUsed(package->hba);
  intent->id_length = strlen(string_id);
  IFE(OgHeapAppend(package->hba, intent->id_length + 1, string_id));

  IFE(NlpCompilePackageSentences(package, intent, json_sentences));

  DONE;
}

static int NlpCompilePackageSentences(package_t package, struct intent *intent, json_t *json_sentences)
{
  og_nlp ctrl_nlp = package->ctrl_nlp;

  intent->sentence_start = OgHeapGetCellsUsed(package->hsentence);

  int array_size = json_array_size(json_sentences);
  for (int i = 0; i < array_size; i++)
  {
    json_t *json_sentence = json_array_get(json_sentences, i);
    IFN(json_sentence)
    {
      NlpThrowError(ctrl_nlp, "NlpCompilePackageSentences: null json_sentence at position %d", i);
      DPcErr;
    }
    if (json_is_object(json_sentence))
    {
      IFE(NlpCompilePackageSentence(package, intent, json_sentence));
    }
    else
    {
      NlpThrowError(ctrl_nlp, "NlpCompilePackageSentences: json_sentence at position %d is not an object", i);
      DPcErr;
    }
  }

  intent->sentences_nb = OgHeapGetCellsUsed(package->hsentence) - intent->sentence_start;

  DONE;
}

static int NlpCompilePackageSentence(package_t package, struct intent *intent, json_t *json_sentence)
{
  og_nlp ctrl_nlp = package->ctrl_nlp;
  char *json_phrase_string = json_dumps(json_sentence, JSON_INDENT(2));

  OgMsg(ctrl_nlp->hmsg, "", DOgMsgDestInLog, "NlpCompilePackagesentence: compiling phrase [\n%.*s]",
      strlen(json_phrase_string), json_phrase_string);

  void *intent_iter = json_object_iter(json_sentence);
  IFN(intent_iter)
  {
    NlpThrowError(ctrl_nlp, "NlpCompilePackageSentence: phrase is empty");
    DPcErr;
  }
  json_t *json_text = NULL;
  json_t *json_locale = NULL;
  do
  {
    const char *key = json_object_iter_key(intent_iter);
    OgMsg(ctrl_nlp->hmsg, "", DOgMsgDestInLog, "NlpCompilePackageSentence: found key='%s'", key);
    if (Ogstricmp(key, "sentence") == 0)
    {
      json_text = json_object_iter_value(intent_iter);
    }
    else if (Ogstricmp(key, "locale") == 0)
    {
      json_locale = json_object_iter_value(intent_iter);
    }
    else
    {
      NlpThrowError(ctrl_nlp, "NlpCompilePackageSentence: unknow key '%s'", key);
      DPcErr;
    }
  }
  while ((intent_iter = json_object_iter_next(json_sentence, intent_iter)) != NULL);

  IFN(json_text)
  {
    NlpThrowError(ctrl_nlp, "NlpCompilePackageSentence: no text");
    DPcErr;
  }
  IFN(json_locale)
  {
    NlpThrowError(ctrl_nlp, "NlpCompilePackageSentence: no locale");
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
    if(sentence->text_length > DOgNlpIntentPhraseMaxLength)
    {
      NlpThrowError(ctrl_nlp, "NlpCompilePackageSentence: text is too long");
      DPcErr;
    }
    IFE(OgHeapAppend(package->hba, sentence->text_length + 1, string_text));
  }
  else
  {
    NlpThrowError(ctrl_nlp, "NlpCompilePackageSentence: text is not a string");
    DPcErr;
  }

  sentence->locale = 0;
  if (json_is_string(json_locale))
  {
    const char *string_locale = json_string_value(json_locale);
    IFE(sentence->locale=OgCodeToIso639_3166(ctrl_nlp->herr,(char *)string_locale));
  }
  else if (json_is_null(json_locale))
  {
    sentence->locale = 0;
  }
  else
  {
    NlpThrowError(ctrl_nlp, "NlpCompilePackageIntent: locale is not a string");
    DPcErr;
  }

  DONE;
}

