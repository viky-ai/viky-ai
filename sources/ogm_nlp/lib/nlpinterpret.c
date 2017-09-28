/*
 *  Main function for Natural Language Processing library
 *  Copyright (c) 2017 Pertimm, by Patrick Constant
 *  Dev : September 2017
 *  Version 1.0
 */
#include "ogm_nlp.h"

static int NlpInterpretRequest(og_nlpi ctrl_nlpi, json_t *json_request);
static int NlpInterpretRequestPackage(og_nlpi ctrl_nlpi, package_t package);
static int NlpInterpretRequestIntent(og_nlpi ctrl_nlpi, package_t package, int Iintent);
static int NlpInterpretRequestParse(og_nlpi ctrl_nlpi, json_t *json_request);
static int NlpInterpretRequestBuildSentence(og_nlpi ctrl_nlpi, json_t *json_sentence);
static int NlpInterpretRequestBuildPackages(og_nlpi ctrl_nlpi, json_t *json_packages);
static int NlpInterpretRequestBuildPackage(og_nlpi ctrl_nlpi, const char *package_id);
static int NlpInterpretRequestBuildAcceptLanguage(og_nlpi ctrl_nlpi, json_t *json_accept_language);

PUBLIC(og_nlpi) OgNlpInterpretInit(og_nlp ctrl_nlp, struct og_nlpi_param *param)
{
  char erreur[DOgErrorSize];
  struct og_ctrl_nlpi *ctrl_nlpi = (struct og_ctrl_nlpi *) malloc(sizeof(struct og_ctrl_nlpi));
  IFn(ctrl_nlpi)
  {
    sprintf(erreur, "OgNlpInit: malloc error on ctrl_nlpi");
    OgErr(param->herr, erreur);
    return (0);
  }

  memset(ctrl_nlpi, 0, sizeof(struct og_ctrl_nlpi));
  ctrl_nlpi->ctrl_nlp = ctrl_nlp;
  ctrl_nlpi->herr = param->herr;
  ctrl_nlpi->hmutex = param->hmutex;
  memcpy(ctrl_nlpi->loginfo, &param->loginfo, sizeof(struct og_loginfo));

  struct og_msg_param msg_param[1];
  memset(msg_param, 0, sizeof(struct og_msg_param));
  msg_param->herr = ctrl_nlpi->herr;
  msg_param->hmutex = ctrl_nlpi->hmutex;
  msg_param->loginfo.trace = DOgMsgTraceMinimal + DOgMsgTraceMemory;
  msg_param->loginfo.where = ctrl_nlpi->loginfo->where;
  msg_param->module_name = "nlp";
  IFn(ctrl_nlpi->hmsg=OgMsgInit(msg_param)) return (0);
  IF(OgMsgTuneInherit(ctrl_nlpi->hmsg,param->hmsg)) return (0);

  IFn(ctrl_nlpi->hinterpret_package=OgHeapInit(ctrl_nlpi->hmsg,"interpret_package",sizeof(struct interpret_package),DOgNlpiPackageNumber)) return (0);

  return ctrl_nlpi;

}

PUBLIC(int) OgNlpInterpret(og_nlpi ctrl_nlpi, struct og_nlp_interpret_input *input,
    struct og_nlp_interpret_output *output)
{

  ctrl_nlpi->json_interpret_request_string = json_dumps(input->json_input, JSON_INDENT(2));
  IFN(ctrl_nlpi->json_interpret_request_string)
  {
    NlpiThrowError(ctrl_nlpi, "OgNlpInterpret: json_dumps error on input->json_input");
    DPcErr;
  }
  ctrl_nlpi->json_interpret_request_string_length = strlen(ctrl_nlpi->json_interpret_request_string);
  ctrl_nlpi->json_interpret_request_string_length_truncated = ctrl_nlpi->json_interpret_request_string_length;
  if (ctrl_nlpi->json_interpret_request_string_length_truncated > DOgMlogMaxMessageSize / 2)
  {
    ctrl_nlpi->json_interpret_request_string_length_truncated = DOgMlogMaxMessageSize / 2;
  }

  OgMsg(ctrl_nlpi->hmsg, "", DOgMsgDestInLog, "OgNlpInterpret: ctrl_nlpi->json_interpret_request_string is [\n%.*s]",
      ctrl_nlpi->json_interpret_request_string_length_truncated, ctrl_nlpi->json_interpret_request_string);

  if (json_is_object(input->json_input))
  {
    IFE(NlpInterpretRequest(ctrl_nlpi, input->json_input));
  }
  else if (json_is_array(input->json_input))
  {
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
        IFE(NlpInterpretRequest(ctrl_nlpi, json_request));
      }
      else
      {
        NlpiThrowError(ctrl_nlpi, "OgNlpInterpret: json_request at position %d is not an object", i);
        DPcErr;
      }
    }
  }
  else
  {
    NlpiThrowError(ctrl_nlpi, "OgNlpCompile: input->json_input is not an object nor an array");
    DPcErr;
  }

  // Just saying that compilation went well
  json_t *json = json_object();
  output->json_output = json;

  json_t *value = json_string("ok");
  IFE(json_object_set_new(json, "interpret", value));

  DONE;
}

static int NlpInterpretRequest(og_nlpi ctrl_nlpi, json_t *json_request)
{
  char *json_request_string = json_dumps(json_request, JSON_INDENT(2));

  OgMsg(ctrl_nlpi->hmsg, "", DOgMsgDestInLog, "NlpInterpretRequest: interpreting request [\n%.*s]",
      strlen(json_request_string), json_request_string);

  IFE(NlpInterpretRequestParse(ctrl_nlpi, json_request));

  int package_used = OgHeapGetCellsUsed(ctrl_nlpi->hinterpret_package);
  for (int i = 0; i < package_used; i++)
  {
    struct interpret_package *interpret_package = OgHeapGetCell(ctrl_nlpi->hinterpret_package, i);
    IFN(interpret_package) DPcErr;
    package_t package = interpret_package->package;
    char *package_id = OgHeapGetCell(package->hba, package->id_start);
    IFN(package_id) DPcErr;
    OgMsg(ctrl_nlpi->hmsg, "", DOgMsgDestInLog, "NlpInterpretRequest: searching package '%s'", package_id);
    IFE(NlpInterpretRequestPackage(ctrl_nlpi, package));
  }

  OgMsg(ctrl_nlpi->hmsg, "", DOgMsgDestInLog, "NlpInterpretRequest: finished interpreting request [\n%.*s]",
      strlen(json_request_string), json_request_string);
  DONE;
}

static int NlpInterpretRequestPackage(og_nlpi ctrl_nlpi, package_t package)
{
  int intent_used = OgHeapGetCellsUsed(package->hintent);
  for (int i = 0; i < intent_used; i++)
  {
    IFE(NlpInterpretRequestIntent(ctrl_nlpi, package, i));
  }

  DONE;
}

static int NlpInterpretRequestIntent(og_nlpi ctrl_nlpi, package_t package, int Iintent)
{
  struct intent *intent = OgHeapGetCell(package->hintent, Iintent);
  IFN(intent) DPcErr;
  char *intent_id = OgHeapGetCell(package->hba, intent->id_start);
  IFN(intent_id) DPcErr;
  for (int i = 0; i < intent->sentences_nb; i++)
  {
    struct sentence *sentence = OgHeapGetCell(package->hsentence, intent->sentence_start + i);
    IFN(sentence) DPcErr;

    char *string_sentence = OgHeapGetCell(package->hba, sentence->text_start);
    if (Ogstricmp(string_sentence, ctrl_nlpi->request_sentence) == 0)
    {
      char *package_id = OgHeapGetCell(package->hba, package->id_start);
      OgMsg(ctrl_nlpi->hmsg, "", DOgMsgDestInLog, "NlpInterpretRequestIntent: found intent '%s' in package '%s'",
          intent_id, package_id);
    }
  }
  DONE;
}

static int NlpInterpretRequestParse(og_nlpi ctrl_nlpi, json_t *json_request)
{

  void *request_iter = json_object_iter(json_request);
  IFN(request_iter)
  {
    NlpiThrowError(ctrl_nlpi, "NlpInterpretRequestParse: package is empty");
    DPcErr;
  }
  json_t *json_packages = NULL;
  json_t *json_sentence = NULL;
  json_t *json_accept_language = NULL;
  do
  {
    const char *key = json_object_iter_key(request_iter);
    OgMsg(ctrl_nlpi->hmsg, "", DOgMsgDestInLog, "NlpInterpretRequestParse: found key='%s'", key);
    if (Ogstricmp(key, "packages") == 0)
    {
      json_packages = json_object_iter_value(request_iter);
    }
    else if (Ogstricmp(key, "sentence") == 0)
    {
      json_sentence = json_object_iter_value(request_iter);
    }
    else if (Ogstricmp(key, "Accept-Language") == 0)
    {
      json_accept_language = json_object_iter_value(request_iter);
    }
    else
    {
      NlpiThrowError(ctrl_nlpi, "NlpInterpretRequestParse: unknow key '%s'", key);
      DPcErr;
    }
  }
  while ((request_iter = json_object_iter_next(json_request, request_iter)) != NULL);

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

static int NlpInterpretRequestBuildSentence(og_nlpi ctrl_nlpi, json_t *json_sentence)
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

static int NlpInterpretRequestBuildPackages(og_nlpi ctrl_nlpi, json_t *json_packages)
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

static int NlpInterpretRequestBuildPackage(og_nlpi ctrl_nlpi, const char *package_id)
{
  package_t package = NlpPackageGet(ctrl_nlpi->ctrl_nlp, package_id);
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
static int NlpInterpretRequestBuildAcceptLanguage(og_nlpi ctrl_nlpi, json_t *json_accept_language)
{

  DONE;
}

