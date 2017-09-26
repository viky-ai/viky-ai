/*
 * nlpdump.c
 *
 *  Created on: 26 sept. 2017
 *      Author: sebastien
 */
#include "ogm_nlp.h"

gint compar(gpointer a, gpointer b)
{ return strcasecmp( (char*)a, (char*)b ); }

PUBLIC(int) OgNlpDump(og_nlp ctrl_nlp, struct og_nlp_dump_input *input, struct og_nlp_dump_output *output)
{

  json_t *json_packages = json_array();

  GList *key_list = g_hash_table_get_keys(ctrl_nlp->packages_hash);
  GList *sorted_key_kist = g_list_sort(key_list,compar);
  for (GList *iter = g_list_first(sorted_key_kist); iter; iter = iter->next)
  {
    json_t *json_package = json_object();

    og_string package_id = iter->data;

    package_t package = NlpPackageGet(ctrl_nlp, package_id);
    IFN(package) DONE;

    IFE(NlpPackageLog(package));

    IFE(NlpPackageDump(package, json_package));

    if(json_array_append(json_packages,json_package) == 0)
    {
      json_incref(json_packages);
    }
    else
    {
      OgMsg(ctrl_nlp->hmsg, "NlpPackageDump", DOgMsgDestInLog, "Error while dumping package");
          DPcErr;
    }
  }

  size_t size = json_dumpb(json_packages, NULL, 0, 0);
  if (size == 0)
  {
    OgMsg(ctrl_nlp->hmsg, "OgNlpDump", DOgMsgDestInLog, "Empty JSon structure");
    DPcErr;
  }


  output->json_output = json_packages;

  DONE;
}

og_status NlpPackageDump(package_t package, json_t *json_package)
{
  og_nlp ctrl_nlp = package->ctrl_nlp;
  const char *package_id = OgHeapGetCell(package->hba, package->id_start);
  IFN(package_id) DPcErr;

  OgMsg(ctrl_nlp->hmsg, "", DOgMsgDestInLog, "Package '%s' :", package_id);

  json_t *json_package_dump = json_string(package_id);
  if (json_object_set(json_package, "id", json_package_dump) == 0)
  {
    json_incref(json_package);
  }
  else
  {
    OgMsg(ctrl_nlp->hmsg, "NlpPackageDump", DOgMsgDestInLog, "Error while dumping package %s", package_id);
    DPcErr;
  }

  int intent_used = OgHeapGetCellsUsed(package->hintent);
  if (intent_used > 0)
  {
    json_t *json_intents = json_array();

    for (int i = 0; i < intent_used; i++)
    {
      IFE(NlpPackageIntentDump(package, i, json_intents));
    }
    if (json_object_set(json_package, "intents", json_intents) == 0)
    {
      json_incref(json_package);
    }
    else
    {
      OgMsg(ctrl_nlp->hmsg, "NlpPackageDump", DOgMsgDestInLog, "Error while dumping intents");
          DPcErr;
    }
  }
  DONE;
}

og_status NlpPackageIntentDump(package_t package, int Iintent, json_t *json_intents)
{
  og_nlp ctrl_nlp = package->ctrl_nlp;
  struct intent *intent = OgHeapGetCell(package->hintent, Iintent);
  IFN(intent) DPcErr;
  char *intent_id = OgHeapGetCell(package->hba, intent->id_start);
  IFN(intent_id) DPcErr;

  OgMsg(ctrl_nlp->hmsg, "", DOgMsgDestInLog, "  Intent '%s' :", intent_id);

  json_t *json_intent = json_object();

  json_t *json_intent_id = json_string(intent_id);
  if (json_object_set(json_intent, "id", json_intent_id) == 0)
  {
    json_incref(json_intent);
  }
  else
  {
    OgMsg(ctrl_nlp->hmsg, "NlpPackageIntentDump", DOgMsgDestInLog, "Error while dumping intent ID %s", intent_id);
    DPcErr;
  }

  if (intent->phrases_nb > 0)
  {
    json_t *json_phrases = json_array();
    for (int i = 0; i < intent->phrases_nb; i++)
    {
      IFE(NlpPackagePhraseDump(package, intent->phrase_start + i, json_phrases));
    }
    if (json_object_set(json_intent, "phrases", json_phrases) == 0)
    {
      json_incref(json_intent);
    }
    else
    {
      OgMsg(ctrl_nlp->hmsg, "NlpPackageIntentDump", DOgMsgDestInLog, "Error while dumping phrases");
      DPcErr;
    }
  }

  if (json_array_append(json_intents, json_intent) == 0)
  {
    json_incref(json_intents);
  }
  else
  {
    OgMsg(ctrl_nlp->hmsg, "NlpPackageIntentDump", DOgMsgDestInLog, "Error while dumping intent");
    DPcErr;
  }

  DONE;
}

og_status NlpPackagePhraseDump(package_t package, int Iphrase, json_t *json_phrases)

{
  og_nlp ctrl_nlp = package->ctrl_nlp;
  struct phrase *phrase = OgHeapGetCell(package->hphrase, Iphrase);
  IFN(phrase) DPcErr;

  json_t *json_phrase = json_object();

  char *text = OgHeapGetCell(package->hba, phrase->text_start);
  json_t *json_text = json_string(text);

  if (json_object_set(json_phrase, "text", json_text) == 0)
  {
    json_incref(json_phrase);
  }
  else
  {
    OgMsg(ctrl_nlp->hmsg, "NlpPackagePhraseDump", DOgMsgDestInLog, "Error while dumping phrase text");
    DPcErr;
  }

  unsigned char string_locale[DPcPathSize];
  OgIso639_3166ToCode(phrase->locale, string_locale);

  OgMsg(ctrl_nlp->hmsg, "", DOgMsgDestInLog, "    Phrase '%s' with locale %s", text, string_locale);

  json_t *json_locale = json_string(string_locale);
  if (json_object_set(json_phrase, "locale", json_locale) == 0)
  {
    json_incref(json_phrase);
  }
  else
  {
    OgMsg(ctrl_nlp->hmsg, "NlpPackagePhraseDump", DOgMsgDestInLog, "Error while dumping phrase locale", string_locale);
    DPcErr;
  }

  if (json_array_append(json_phrases, json_phrase) == 0)
  {
    json_incref(json_phrases);
  }
  else
  {
    OgMsg(ctrl_nlp->hmsg, "NlpPackagePhraseDump", DOgMsgDestInLog, "Error while dumping phrase");
    DPcErr;
  }

  DONE;
}

