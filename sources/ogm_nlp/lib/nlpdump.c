/*
 * nlpdump.c
 *
 *  Created on: 26 sept. 2017
 *      Author: sebastien
 */
#include "ogm_nlp.h"

static gint str_compar(gconstpointer a, gconstpointer b)
{
  return strcmp((const char*) a, (const char*) b);
}

PUBLIC(int) OgNlpDump(og_nlp ctrl_nlp, struct og_nlp_dump_input *input, struct og_nlp_dump_output *output)
{

  json_t *json_packages = json_array();

  GList *key_list = g_hash_table_get_keys(ctrl_nlp->packages_hash);
  GList *sorted_key_kist = g_list_sort(key_list, str_compar);
  for (GList *iter = g_list_first(sorted_key_kist); iter; iter = iter->next)
  {
    json_t *json_package = json_object();

    og_string package_id = iter->data;

    package_t package = NlpPackageGet(ctrl_nlp, package_id);
    IFN(package) DONE;

    IFE(NlpPackageLog(package));

    IFE(NlpPackageDump(package, json_package));

    if (json_array_append(json_packages, json_package) == 0)
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

  if (intent->sentences_nb > 0)
  {
    json_t *json_sentences = json_array();
    for (int i = 0; i < intent->sentences_nb; i++)
    {
      IFE(NlpPackageSentenceDump(package, intent->sentence_start + i, json_sentences));
    }
    if (json_object_set(json_intent, "sentences", json_sentences) == 0)
    {
      json_incref(json_intent);
    }
    else
    {
      OgMsg(ctrl_nlp->hmsg, "NlpPackageIntentDump", DOgMsgDestInLog, "Error while dumping sentences");
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

og_status NlpPackageSentenceDump(package_t package, int Isentence, json_t *json_sentences)

{
  og_nlp ctrl_nlp = package->ctrl_nlp;
  struct sentence *sentence = OgHeapGetCell(package->hsentence, Isentence);
  IFN(sentence) DPcErr;

  json_t *json_sentence = json_object();

  char *text = OgHeapGetCell(package->hba, sentence->text_start);
  json_t *json_text = json_string(text);

  if (json_object_set(json_sentence, "sentence", json_text) == 0)
  {
    json_incref(json_sentence);
  }
  else
  {
    OgMsg(ctrl_nlp->hmsg, "NlpPackageSentenceDump", DOgMsgDestInLog, "Error while dumping sentence");
    DPcErr;
  }

  unsigned char string_locale[DPcPathSize];
  OgIso639_3166ToCode(sentence->locale, string_locale);

  OgMsg(ctrl_nlp->hmsg, "", DOgMsgDestInLog, "    Phrase '%s' with locale %s", text, string_locale);

  json_t *json_locale = json_string(string_locale);
  if (json_object_set(json_sentence, "locale", json_locale) == 0)
  {
    json_incref(json_sentence);
  }
  else
  {
    OgMsg(ctrl_nlp->hmsg, "NlpPackageSentenceDump", DOgMsgDestInLog, "Error while dumping sentence locale", string_locale);
    DPcErr;
  }

  if (json_array_append(json_sentences, json_sentence) == 0)
  {
    json_incref(json_sentences);
  }
  else
  {
    OgMsg(ctrl_nlp->hmsg, "NlpPackageSentenceDump", DOgMsgDestInLog, "Error while dumping sentence");
    DPcErr;
  }

  DONE;
}

