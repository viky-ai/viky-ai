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
  output->json_output = json_packages;

  GList *key_list = g_hash_table_get_keys(ctrl_nlp->packages_hash);

  GList *sorted_key_kist = g_list_sort(key_list, str_compar);
  for (GList *iter = g_list_first(sorted_key_kist); iter; iter = iter->next)
  {
    json_t *json_package = json_object();
    IF(json_array_append_new(json_packages, json_package))
    {
      NlpThrowError(ctrl_nlp, "NlpPackageDump : Error while dumping package");
      DPcErr;
    }
    og_string package_id = iter->data;

    package_t package = NlpPackageGet(ctrl_nlp, package_id);
    IFN(package) DONE;

    IFE(NlpPackageLog(package));

    IFE(NlpPackageDump(package, json_package));

  }

  size_t size = json_dumpb(json_packages, NULL, 0, 0);
  if (size == 0)
  {
    NlpThrowError(ctrl_nlp, "OgNlpDump : Empty JSon structure");
    DPcErr;
  }

  DONE;
}

og_status NlpPackageDump(package_t package, json_t *json_package)
{
  og_nlp ctrl_nlp = package->ctrl_nlp;
  const char *package_id = OgHeapGetCell(package->hba, package->id_start);
  IFN(package_id) DPcErr;

  OgMsg(ctrl_nlp->hmsg, "", DOgMsgDestInLog, "Package '%s' :", package_id);

  json_t *json_package_dump = json_string(package_id);

  IF(json_object_set_new(json_package, "id", json_package_dump))
  {
    NlpThrowError(ctrl_nlp, "NlpPackageDump : Error while dumping package %s", package_id);
    DPcErr;
  }

  int intent_used = OgHeapGetCellsUsed(package->hintent);
  if (intent_used > 0)
  {
    json_t *json_intents = json_array();
    IF(json_object_set_new(json_package, "intents", json_intents))
    {
      NlpThrowError(ctrl_nlp, "NlpPackageDump : Error while dumping intents");
      DPcErr;
    }

    for (int i = 0; i < intent_used; i++)
    {
      IFE(NlpPackageIntentDump(package, i, json_intents));
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
  IF(json_array_append_new(json_intents, json_intent))
  {
    NlpThrowError(ctrl_nlp, "NlpPackageIntentDump : Error while dumping intent");
    DPcErr;
  }

  json_t *json_intent_id = json_string(intent_id);
  IF(json_object_set_new(json_intent, "id", json_intent_id))
  {
    NlpThrowError(ctrl_nlp, "NlpPackageIntentDump : Error while dumping intent ID %s", intent_id);
    DPcErr;
  }

  if (intent->sentences_nb > 0)
  {
    json_t *json_sentences = json_array();
    IF(json_object_set_new(json_intent, "sentences", json_sentences))
    {
      NlpThrowError(ctrl_nlp, "NlpPackageIntentDump : Error while dumping sentences");
      DPcErr;
    }

    for (int i = 0; i < intent->sentences_nb; i++)
    {
      IFE(NlpPackageSentenceDump(package, intent->sentence_start + i, json_sentences));
    }

  }

  DONE;
}

og_status NlpPackageSentenceDump(package_t package, int Isentence, json_t *json_sentences)

{
  og_nlp ctrl_nlp = package->ctrl_nlp;
  struct sentence *sentence = OgHeapGetCell(package->hsentence, Isentence);
  IFN(sentence) DPcErr;

  json_t *json_sentence = json_object();
  IF(json_array_append_new(json_sentences, json_sentence))
  {
    NlpThrowError(ctrl_nlp, "NlpPackageSentenceDump : Error while dumping sentence");
    DPcErr;
  }
  og_string text = OgHeapGetCell(package->hba, sentence->text_start);
  json_t *json_text = json_string(text);

  IF(json_object_set_new(json_sentence, "sentence", json_text))
  {
    NlpThrowError(ctrl_nlp, "NlpPackageSentenceDump : Error while dumping sentence");
    DPcErr;
  }

  unsigned char string_locale[DPcPathSize];
  OgIso639_3166ToCode(sentence->locale, string_locale);

  OgMsg(ctrl_nlp->hmsg, "", DOgMsgDestInLog, "    Phrase '%s' with locale %s", text, string_locale);

  json_t *json_locale = json_string(string_locale);
  IF(json_object_set_new(json_sentence, "locale", json_locale))
  {
    NlpThrowError(ctrl_nlp, "NlpPackageSentenceDump : Error while dumping sentence locale", string_locale);
    DPcErr;
  }

  DONE;
}

