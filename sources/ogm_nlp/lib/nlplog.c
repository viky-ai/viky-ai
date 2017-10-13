/*
 *  Handling log messages for Natural Language Processing library
 *  Copyright (c) 2017 Pertimm, by Patrick Constant
 *  Dev : September 2017
 *  Version 1.0
 */
#include "ogm_nlp.h"

/**
 * Write json_t in buffer. Buffer is always NUL terminated.
 *
 * @param json json_t pointer
 * @param buffer buffer to write json in
 * @param buffer_size buffer size
 * @param p_truncated flag to indicate if json is truncated or not
 * @return status
 */
og_status NlpJsonToBuffer(const json_t *json, og_char_buffer *buffer, int buffer_size, og_bool *p_truncated)
{
  og_string truncated_ends = " ... (truncated) ";
  int truncated_ends_size = strlen(truncated_ends);

  int expected_size = json_dumpb(json, buffer, buffer_size - truncated_ends_size - 1, JSON_INDENT(2));
  IF(expected_size)
  {
    DPcErr;
  }

  // truncated json
  if (expected_size >= (buffer_size - truncated_ends_size - 1))
  {
    if (p_truncated) *p_truncated = TRUE;

    if (truncated_ends_size > buffer_size)
    {
      snprintf(buffer + buffer_size - truncated_ends_size - 1, truncated_ends_size, "%s", truncated_ends);
    }

    DONE;
  }

  buffer[expected_size - 1] = 0;

  if (p_truncated) *p_truncated = TRUE;

  DONE;
}

og_status NlpPackageLog(og_nlpi ctrl_nlpi, package_t package)
{
  og_string package_id = OgHeapGetCell(package->hba, package->id_start);
  IFN(package_id) DPcErr;

  OgMsg(ctrl_nlpi->hmsg, "", DOgMsgDestInLog, "Package '%s' :", package_id);

  int intent_used = OgHeapGetCellsUsed(package->hintent);
  for (int i = 0; i < intent_used; i++)
  {
    IFE(NlpPackageIntentLog(ctrl_nlpi, package, i));
  }
  DONE;
}

og_status NlpPackageIntentLog(og_nlpi ctrl_nlpi, package_t package, int Iintent)
{
  struct intent *intent = OgHeapGetCell(package->hintent, Iintent);
  IFN(intent) DPcErr;

  og_string intent_id = OgHeapGetCell(package->hba, intent->id_start);
  IFN(intent_id) DPcErr;

  OgMsg(ctrl_nlpi->hmsg, "", DOgMsgDestInLog, "  Intent '%s' :", intent_id);

  for (int i = 0; i < intent->sentences_nb; i++)
  {
    IFE(NlpPackageSentenceLog(ctrl_nlpi, package, intent->sentence_start + i));
  }
  DONE;
}

og_status NlpPackageSentenceLog(og_nlpi ctrl_nlpi, package_t package, int Iphrase)

{
  struct sentence *phrase = OgHeapGetCell(package->hsentence, Iphrase);
  IFN(phrase) DPcErr;

  og_string text = OgHeapGetCell(package->hba, phrase->text_start);

  unsigned char string_locale[DPcPathSize];
  OgIso639_3166ToCode(phrase->locale, string_locale);

  OgMsg(ctrl_nlpi->hmsg, "", DOgMsgDestInLog, "    Phrase '%s' with locale %s", text, string_locale);

  DONE;
}

