/*
 *  Main function for Natural Language Processing library
 *  Copyright (c) 2017 Pertimm, by Patrick Constant
 *  Dev : September 2017
 *  Version 1.0
 */
#include "ogm_nlp.h"

static og_status NlpInterpretRequest(og_nlpi ctrl_nlpi, json_t *json_request, json_t *json_answer);
static og_status NlpInterpretRequestReset(og_nlpi ctrl_nlpi);
static og_status NlpInterpretRequestPackage(og_nlpi ctrl_nlpi, package_t package, json_t *json_intents);
static og_status NlpInterpretRequestIntent(og_nlpi ctrl_nlpi, package_t package, int Iintent, json_t *json_intents);
static og_status NlpInterpretRequestParse(og_nlpi ctrl_nlpi, json_t *json_request);
static og_status NlpInterpretRequestBuildSentence(og_nlpi ctrl_nlpi, json_t *json_sentence);
static og_status NlpInterpretRequestBuildPackages(og_nlpi ctrl_nlpi, json_t *json_packages);
static og_status NlpInterpretRequestBuildPackage(og_nlpi ctrl_nlpi, const char *package_id);
static og_status NlpInterpretRequestBuildAcceptLanguage(og_nlpi ctrl_nlpi, json_t *json_accept_language);

og_status NlpInterpretInit(og_nlpi ctrl_nlpi, struct og_nlpi_param *param)
{

  // setup request memory for future interpretation

  og_char_buffer nlpc_name[DPcPathSize];
  snprintf(nlpc_name, DPcPathSize, "%s_interpret_package", param->name);
  ctrl_nlpi->hinterpret_package = OgHeapInit(ctrl_nlpi->hmsg, nlpc_name, sizeof(struct interpret_package),
  DOgNlpiPackageNumber);
  IFN(ctrl_nlpi->hinterpret_package)
  {
    NlpiThrowError(ctrl_nlpi, "OgNlpInterpretInit : error on OgHeapInit(%s)", nlpc_name);
    DPcErr;
  }

  DONE;
}

og_status NlpInterpretReset(og_nlpi ctrl_nlpi)
{
  IFE(NlpInterpretRequestReset(ctrl_nlpi));

  DONE;
}

og_status NlpInterpretFlush(og_nlpi ctrl_nlpi)
{
  IFE(OgHeapFlush(ctrl_nlpi->hinterpret_package));
  ctrl_nlpi->hinterpret_package = NULL;

  DONE;
}

PUBLIC(og_status) OgNlpInterpret(og_nlpi ctrl_nlpi, struct og_nlp_interpret_input *input,
    struct og_nlp_interpret_output *output)
{
  og_char_buffer json_interpret_request[DOgMlogMaxMessageSize / 2];
  IFE(NlpJsonToBuffer(input->json_input, json_interpret_request, DOgMlogMaxMessageSize / 2, NULL));

  OgMsg(ctrl_nlpi->hmsg, "", DOgMsgDestInLog, "OgNlpInterpret: ctrl_nlpi->json_interpret_request_string is [\n%s]",
      json_interpret_request);

  if (json_is_object(input->json_input))
  {
    ctrl_nlpi->json_answer = json_object();
    IFE(NlpInterpretRequest(ctrl_nlpi, input->json_input, ctrl_nlpi->json_answer));
    output->json_output = ctrl_nlpi->json_answer;
  }
  else if (json_is_array(input->json_input))
  {
    ctrl_nlpi->json_answer = json_array();

    int array_size = json_array_size(input->json_input);
    for (int i = 0; i < array_size; i++)
    {
      json_t *json_request = json_array_get(input->json_input, i);
      IFN(json_request)
      {
        NlpiThrowError(ctrl_nlpi, "OgNlpInterpret: null json_request at position %d", i);
        DPcErr;
      }

      if (json_is_object(json_request))
      {
        json_t *json_answer_unit = json_object();
        IF(json_array_append_new(ctrl_nlpi->json_answer, json_answer_unit))
        {
          NlpiThrowError(ctrl_nlpi, "OgNlpInterpret: error appending json_answer to array");
          DPcErr;
        }

        IFE(NlpInterpretRequest(ctrl_nlpi, json_request, json_answer_unit));

      }
      else
      {
        NlpiThrowError(ctrl_nlpi, "OgNlpInterpret: json_request at position %d is not an object", i);
        DPcErr;
      }

      output->json_output = ctrl_nlpi->json_answer;
    }
  }
  else
  {
    NlpiThrowError(ctrl_nlpi, "OgNlpCompile: input->json_input is not an object or an array");
    DPcErr;
  }

  DONE;
}

static int NlpInterpretRequest(og_nlpi ctrl_nlpi, json_t *json_request, json_t *json_answer)
{
  // reset previews interpretations
  IFE(NlpInterpretRequestReset(ctrl_nlpi));

  og_char_buffer json_request_string[DPcPathSize];
  IFE(NlpJsonToBuffer(json_request, json_request_string, DPcPathSize, NULL));

  OgMsg(ctrl_nlpi->hmsg, "", DOgMsgDestInLog, "NlpInterpretRequest: interpreting request [\n%s]", json_request_string);

  // parse
  IFE(NlpInterpretRequestParse(ctrl_nlpi, json_request));

  json_t *json_intents = json_array();
  IF(json_object_set_new(json_answer, "intents", json_intents))
  {
    NlpiThrowError(ctrl_nlpi, "NlpInterpretRequest: error setting json_intents");
    DPcErr;
  }

  int package_used = OgHeapGetCellsUsed(ctrl_nlpi->hinterpret_package);
  for (int i = 0; i < package_used; i++)
  {
    struct interpret_package *interpret_package = OgHeapGetCell(ctrl_nlpi->hinterpret_package, i);
    IFN(interpret_package) DPcErr;

    package_t package = interpret_package->package;

    og_string package_id = OgHeapGetCell(package->hba, package->id_start);
    IFN(package_id) DPcErr;

    OgMsg(ctrl_nlpi->hmsg, "", DOgMsgDestInLog, "NlpInterpretRequest: searching package '%s'", package_id);

    IFE(NlpInterpretRequestPackage(ctrl_nlpi, package, json_intents));
  }

  OgMsg(ctrl_nlpi->hmsg, "", DOgMsgDestInLog, "NlpInterpretRequest: finished interpreting request [\n%s]",
      json_request_string);
  DONE;
}

static og_status NlpInterpretRequestReset(og_nlpi ctrl_nlpi)
{
  IFE(OgHeapReset(ctrl_nlpi->hinterpret_package));

  ctrl_nlpi->request_sentence = NULL;

  DONE;
}

static og_status NlpInterpretRequestPackage(og_nlpi ctrl_nlpi, package_t package, json_t *json_intents)
{
  int intent_used = OgHeapGetCellsUsed(package->hintent);
  for (int i = 0; i < intent_used; i++)
  {
    IFE(NlpInterpretRequestIntent(ctrl_nlpi, package, i, json_intents));
  }

  DONE;
}

static og_status NlpInterpretRequestIntent(og_nlpi ctrl_nlpi, package_t package, int Iintent, json_t *json_intents)
{
  struct intent *intent = OgHeapGetCell(package->hintent, Iintent);
  IFN(intent) DPcErr;

  char *intent_id = OgHeapGetCell(package->hba, intent->id_start);
  IFN(intent_id) DPcErr;

  for (int i = 0; i < intent->sentences_nb; i++)
  {
    struct sentence *sentence = OgHeapGetCell(package->hsentence, intent->sentence_start + i);
    IFN(sentence) DPcErr;

    og_string string_sentence = OgHeapGetCell(package->hba, sentence->text_start);
    if (Ogstricmp(string_sentence, ctrl_nlpi->request_sentence) == 0)
    {
      og_string package_id = OgHeapGetCell(package->hba, package->id_start);
      OgMsg(ctrl_nlpi->hmsg, "", DOgMsgDestInLog, "NlpInterpretRequestIntent: found intent '%s' in package '%s'",
          intent_id, package_id);
      json_t *json_intent = json_object();

      json_t *json_package_id = json_string(package_id);
      IF(json_object_set_new(json_intent, "package", json_package_id))
      {
        NlpiThrowError(ctrl_nlpi, "NlpInterpretRequestIntent: error setting json_package_id");
        DPcErr;
      }

      json_t *json_intent_id = json_string(intent_id);
      IF(json_object_set_new(json_intent, "id", json_intent_id))
      {
        NlpiThrowError(ctrl_nlpi, "NlpInterpretRequestIntent: error setting json_intent_id");
        DPcErr;
      }

      json_t *json_score = json_real(1.0);
      IF(json_object_set_new(json_intent, "score", json_score))
      {
        NlpiThrowError(ctrl_nlpi, "NlpInterpretRequestIntent: error setting json_score");
        DPcErr;
      }

      IF(json_array_append_new(json_intents, json_intent))
      {
        NlpiThrowError(ctrl_nlpi, "NlpInterpretRequestIntent: error appending json_intent to array");
        DPcErr;
      }

    }
  }

  DONE;
}

static og_status NlpInterpretRequestParse(og_nlpi ctrl_nlpi, json_t *json_request)
{
  json_t *json_packages = NULL;
  json_t *json_sentence = NULL;
  json_t *json_accept_language = NULL;

  for (void *iter = json_object_iter(json_request); iter; iter = json_object_iter_next(json_request, iter))
  {
    og_string key = json_object_iter_key(iter);

    OgMsg(ctrl_nlpi->hmsg, "", DOgMsgDestInLog, "NlpInterpretRequestParse: found key='%s'", key);

    if (Ogstricmp(key, "packages") == 0)
    {
      json_packages = json_object_iter_value(iter);
    }
    else if (Ogstricmp(key, "sentence") == 0)
    {
      json_sentence = json_object_iter_value(iter);
    }
    else if (Ogstricmp(key, "Accept-Language") == 0)
    {
      json_accept_language = json_object_iter_value(iter);
    }
    else
    {
      NlpiThrowError(ctrl_nlpi, "NlpInterpretRequestParse: unknow key '%s'", key);
      DPcErr;
    }

  }

  IFN(json_packages)
  {
    NlpiThrowError(ctrl_nlpi, "NlpInterpretRequestParse: no packages");
    DPcErr;
  }

  IFN(json_sentence)
  {
    NlpiThrowError(ctrl_nlpi, "NlpInterpretRequestParse: no sentence");
    DPcErr;
  }

  // The Accept-Language string can be non extant

  IFE(NlpInterpretRequestBuildSentence(ctrl_nlpi, json_sentence));
  IFE(NlpInterpretRequestBuildPackages(ctrl_nlpi, json_packages));
  IFE(NlpInterpretRequestBuildAcceptLanguage(ctrl_nlpi, json_accept_language));

  DONE;
}

static og_status NlpInterpretRequestBuildSentence(og_nlpi ctrl_nlpi, json_t *json_sentence)
{
  if (json_is_string(json_sentence))
  {
    ctrl_nlpi->request_sentence = json_string_value(json_sentence);

    OgMsg(ctrl_nlpi->hmsg, "", DOgMsgDestInLog, "NlpInterpretRequestBuildSentence: sentence '%s'",
        ctrl_nlpi->request_sentence);
  }
  else
  {
    NlpiThrowError(ctrl_nlpi, "NlpInterpretRequestBuildSentence: json_sentence is not a string");
    DPcErr;
  }
  DONE;
}

static og_status NlpInterpretRequestBuildPackages(og_nlpi ctrl_nlpi, json_t *json_packages)
{
  int array_size = json_array_size(json_packages);
  for (int i = 0; i < array_size; i++)
  {
    json_t *json_package = json_array_get(json_packages, i);
    IFN(json_package)
    {
      NlpiThrowError(ctrl_nlpi, "NlpInterpretRequestBuildPackages: null json_package at position %d", i);
      DPcErr;
    }

    if (json_is_string(json_package))
    {
      const char *package_id = json_string_value(json_package);
      OgMsg(ctrl_nlpi->hmsg, "", DOgMsgDestInLog, "NlpInterpretRequestBuildPackages: package id is '%s'", package_id);

      IFE(NlpInterpretRequestBuildPackage(ctrl_nlpi, package_id));
    }
    else
    {
      NlpiThrowError(ctrl_nlpi, "NlpCompilePackageIntents: json_intent at position %d is not a string", i);
      DPcErr;
    }

  }

  DONE;
}

static og_status NlpInterpretRequestBuildPackage(og_nlpi ctrl_nlpi, const char *package_id)
{
  package_t package = NlpPackageGet(ctrl_nlpi, package_id);
  IFN(package)
  {
    NlpiThrowError(ctrl_nlpi, "NlpInterpretRequestBuildPackage: unknown package '%s'", package_id);
    DPcErr;
  }

  struct interpret_package interpret_package[1];
  interpret_package->package = package;

  IFE(OgHeapAppend(ctrl_nlpi->hinterpret_package, 1, interpret_package));

  DONE;
}

// TODO : mettre dans nlplang.c
static og_status NlpInterpretRequestBuildAcceptLanguage(og_nlpi ctrl_nlpi, json_t *json_accept_language)
{

  DONE;
}

