/*
 *  Handling log messages for Natural Language Processing library
 *  Copyright (c) 2017 Pertimm, by Patrick Constant
 *  Dev : September 2017
 *  Version 1.0
 */
#include "ogm_nlp.h"

og_status NlpPackageLog(package_t package)
{
  og_nlp ctrl_nlp = package->ctrl_nlp;
  char *package_id = OgHeapGetCell(package->hba, package->id_start);
  IFN(package_id) DPcErr;
  OgMsg(ctrl_nlp->hmsg, "", DOgMsgDestInLog, "Package '%s' :", package_id);

  int intent_used = OgHeapGetCellsUsed(package->hintent);
  for (int i = 0; i < intent_used; i++)
  {
    IFE(NlpPackageIntentLog(package, i));
  }
  DONE;
}

og_status NlpPackageIntentLog(package_t package, int Iintent)
{
  og_nlp ctrl_nlp = package->ctrl_nlp;
  struct intent *intent = OgHeapGetCell(package->hintent, Iintent);
  IFN(intent) DPcErr;
  char *intent_id = OgHeapGetCell(package->hba, intent->id_start);
  IFN(intent_id) DPcErr;
  OgMsg(ctrl_nlp->hmsg, "", DOgMsgDestInLog, "  Intent '%s' :", intent_id);

  for (int i = 0; i < intent->phrases_nb; i++)
  {
    IFE(NlpPackagePhraseLog(package, intent->phrase_start + i));
  }
  DONE;
}

og_status NlpPackagePhraseLog(package_t package, int Iphrase)

{
  og_nlp ctrl_nlp = package->ctrl_nlp;
  struct phrase *phrase = OgHeapGetCell(package->hphrase, Iphrase);
  IFN(phrase) DPcErr;

  char *text = OgHeapGetCell(package->hba, phrase->text_start);

  unsigned char string_locale[DPcPathSize];
  OgIso639_3166ToCode(phrase->locale, string_locale);

  OgMsg(ctrl_nlp->hmsg, "", DOgMsgDestInLog, "    Phrase '%s' with locale %s", text, string_locale);
  DONE;
}



