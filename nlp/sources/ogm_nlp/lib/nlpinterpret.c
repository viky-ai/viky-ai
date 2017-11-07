/*
 *  Main function for Natural Language Processing library
 *  Copyright (c) 2017 Pertimm, by Patrick Constant
 *  Dev : September 2017
 *  Version 1.0
 */
#include "ogm_nlp.h"

static og_status NlpInterpretRequest(og_nlp_th ctrl_nlp_th, json_t *json_request, json_t *json_answer);
static og_status NlpInterpretRequestReset(og_nlp_th ctrl_nlp_th);
static og_status NlpInterpretRequestParse(og_nlp_th ctrl_nlp_th, json_t *json_request);
static og_status NlpInterpretRequestBuildSentence(og_nlp_th ctrl_nlp_th, json_t *json_sentence);
static og_status NlpInterpretRequestBuildPackages(og_nlp_th ctrl_nlp_th, json_t *json_packages);
static og_status NlpInterpretRequestBuildPackage(og_nlp_th ctrl_nlp_th, const char *package_id);
static og_status NlpInterpretRequestBuildAcceptLanguage(og_nlp_th ctrl_nlp_th, json_t *json_accept_language);

og_status NlpInterpretInit(og_nlp_th ctrl_nlp_th, struct og_nlp_threaded_param *param)
{

  // setup request memory for future interpretation

  og_char_buffer nlpc_name[DPcPathSize];
  snprintf(nlpc_name, DPcPathSize, "%s_interpret_package", param->name);
  ctrl_nlp_th->hinterpret_package = OgHeapInit(ctrl_nlp_th->hmsg, nlpc_name, sizeof(struct interpret_package),
  DOgNlpPackageNumber);
  IFN(ctrl_nlp_th->hinterpret_package)
  {
    NlpThrowErrorTh(ctrl_nlp_th, "OgNlpInterpretInit : error on OgHeapInit(%s)", nlpc_name);
    DPcErr;
  }
  snprintf(nlpc_name, DPcPathSize, "%s_request_word", param->name);
  ctrl_nlp_th->hrequest_word = OgHeapInit(ctrl_nlp_th->hmsg, nlpc_name, sizeof(struct request_word),
  DOgNlpRequestWordNumber);
  IFN(ctrl_nlp_th->hrequest_word)
  {
    NlpThrowErrorTh(ctrl_nlp_th, "OgNlpInterpretInit : error on OgHeapInit(%s)", nlpc_name);
    DPcErr;
  }
  snprintf(nlpc_name, DPcPathSize, "%s_ba", param->name);
  ctrl_nlp_th->hba = OgHeapInit(ctrl_nlp_th->hmsg, nlpc_name, sizeof(unsigned char), DOgNlpBaNumber);
  IFN(ctrl_nlp_th->hba)
  {
    NlpThrowErrorTh(ctrl_nlp_th, "OgNlpInterpretInit : error on OgHeapInit(%s)", nlpc_name);
    DPcErr;
  }
  snprintf(nlpc_name, DPcPathSize, "%s_request_input_part", param->name);
  ctrl_nlp_th->hrequest_input_part = OgHeapInit(ctrl_nlp_th->hmsg, nlpc_name, sizeof(struct request_input_part),
  DOgNlpRequestInputPartNumber);
  IFN(ctrl_nlp_th->hrequest_input_part)
  {
    NlpThrowErrorTh(ctrl_nlp_th, "OgNlpInterpretInit : error on OgHeapInit(%s)", nlpc_name);
    DPcErr;
  }
  snprintf(nlpc_name, DPcPathSize, "%s_request_expression", param->name);
  ctrl_nlp_th->hrequest_expression = OgHeapInit(ctrl_nlp_th->hmsg, nlpc_name, sizeof(struct request_expression),
  DOgNlpRequestExpressionNumber);
  IFN(ctrl_nlp_th->hrequest_expression)
  {
    NlpThrowErrorTh(ctrl_nlp_th, "OgNlpInterpretInit : error on OgHeapInit(%s)", nlpc_name);
    DPcErr;
  }
  snprintf(nlpc_name, DPcPathSize, "%s_request_position", param->name);
  ctrl_nlp_th->hrequest_position = OgHeapInit(ctrl_nlp_th->hmsg, nlpc_name, sizeof(struct request_position),
  DOgNlpRequestPositionNumber);
  IFN(ctrl_nlp_th->hrequest_position)
  {
    NlpThrowErrorTh(ctrl_nlp_th, "OgNlpInterpretInit : error on OgHeapInit(%s)", nlpc_name);
    DPcErr;
  }
  snprintf(nlpc_name, DPcPathSize, "%s_original_request_input_part", param->name);
  ctrl_nlp_th->horiginal_request_input_part = OgHeapInit(ctrl_nlp_th->hmsg, nlpc_name,
      sizeof(struct original_request_input_part),
      DOgNlpRequestInputPartNumber);
  IFN(ctrl_nlp_th->horiginal_request_input_part)
  {
    NlpThrowErrorTh(ctrl_nlp_th, "OgNlpInterpretInit : error on OgHeapInit(%s)", nlpc_name);
    DPcErr;
  }
  snprintf(nlpc_name, DPcPathSize, "%s_orip", param->name);
  ctrl_nlp_th->horip = OgHeapInit(ctrl_nlp_th->hmsg, nlpc_name, sizeof(struct orip),
  DOgNlpRequestInputPartNumber);
  IFN(ctrl_nlp_th->horip)
  {
    NlpThrowErrorTh(ctrl_nlp_th, "OgNlpInterpretInit : error on OgHeapInit(%s)", nlpc_name);
    DPcErr;
  }
  snprintf(nlpc_name, DPcPathSize, "%s_request_any", param->name);
  ctrl_nlp_th->hrequest_any = OgHeapInit(ctrl_nlp_th->hmsg, nlpc_name, sizeof(struct request_any),
  DOgNlpRequestInputPartNumber);
  IFN(ctrl_nlp_th->hrequest_any)
  {
    NlpThrowErrorTh(ctrl_nlp_th, "OgNlpInterpretInit : error on OgHeapInit(%s)", nlpc_name);
    DPcErr;
  }

  ctrl_nlp_th->regular_trace = ctrl_nlp_th->loginfo->trace;

  ctrl_nlp_th->duk_context = duk_create_heap_default();

  DONE;
}

og_status NlpInterpretReset(og_nlp_th ctrl_nlp_th)
{
  IFE(NlpInterpretRequestReset(ctrl_nlp_th));

  DONE;
}

og_status NlpInterpretFlush(og_nlp_th ctrl_nlp_th)
{
  g_queue_clear(ctrl_nlp_th->sorted_request_expressions);
  IFE(NlpInterpretAnyFlush(ctrl_nlp_th));
  IFE(OgHeapFlush(ctrl_nlp_th->hinterpret_package));
  IFE(OgHeapFlush(ctrl_nlp_th->hrequest_word));
  IFE(OgHeapFlush(ctrl_nlp_th->hba));
  IFE(OgHeapFlush(ctrl_nlp_th->hrequest_input_part));
  IFE(OgHeapFlush(ctrl_nlp_th->hrequest_expression));
  IFE(OgHeapFlush(ctrl_nlp_th->hrequest_position));
  IFE(OgHeapFlush(ctrl_nlp_th->horiginal_request_input_part));
  IFE(OgHeapFlush(ctrl_nlp_th->horip));
  IFE(OgHeapFlush(ctrl_nlp_th->hrequest_any));

  ctrl_nlp_th->hinterpret_package = NULL;
  ctrl_nlp_th->hrequest_word = NULL;
  ctrl_nlp_th->hba = NULL;
  ctrl_nlp_th->hrequest_input_part = NULL;
  ctrl_nlp_th->hrequest_expression = NULL;
  ctrl_nlp_th->hrequest_position = NULL;
  ctrl_nlp_th->horiginal_request_input_part = NULL;
  ctrl_nlp_th->horip = NULL;
  ctrl_nlp_th->hrequest_any = NULL;

  duk_destroy_heap(ctrl_nlp_th->duk_context);

  DONE;
}

PUBLIC(og_status) OgNlpInterpret(og_nlp_th ctrl_nlp_th, struct og_nlp_interpret_input *input,
    struct og_nlp_interpret_output *output)
{

  // reset output
  memset(output, 0, sizeof(struct og_nlp_interpret_output));

  og_char_buffer json_interpret_request[DOgMlogMaxMessageSize / 2];
  IFE(NlpJsonToBuffer(input->json_input, json_interpret_request, DOgMlogMaxMessageSize / 2, NULL, JSON_INDENT(2)));

  NlpLog(DOgNlpTraceInterpret, "OgNlpInterpret: ctrl_nlp_th->json_interpret_request_string is [\n%s]",
      json_interpret_request)

  if (json_is_object(input->json_input))
  {
    ctrl_nlp_th->json_answer = json_object();
    IFE(NlpInterpretRequest(ctrl_nlp_th, input->json_input, ctrl_nlp_th->json_answer));
    output->json_output = ctrl_nlp_th->json_answer;
  }
  else if (json_is_array(input->json_input))
  {
    ctrl_nlp_th->json_answer = json_array();

    int array_size = json_array_size(input->json_input);
    for (int i = 0; i < array_size; i++)
    {
      json_t *json_request = json_array_get(input->json_input, i);
      IFN(json_request)
      {
        NlpThrowErrorTh(ctrl_nlp_th, "OgNlpInterpret: null json_request at position %d", i);
        DPcErr;
      }

      if (json_is_object(json_request))
      {
        json_t *json_answer_unit = json_object();
        IF(json_array_append_new(ctrl_nlp_th->json_answer, json_answer_unit))
        {
          NlpThrowErrorTh(ctrl_nlp_th, "OgNlpInterpret: error appending json_answer to array");
          DPcErr;
        }

        IFE(NlpInterpretRequest(ctrl_nlp_th, json_request, json_answer_unit));

      }
      else
      {
        NlpThrowErrorTh(ctrl_nlp_th, "OgNlpInterpret: json_request at position %d is not an object", i);
        DPcErr;
      }

      output->json_output = ctrl_nlp_th->json_answer;
    }
  }
  else
  {
    NlpThrowErrorTh(ctrl_nlp_th, "OgNlpCompile: input->json_input is not an object or an array");
    DPcErr;
  }

  DONE;
}

static int NlpInterpretRequest(og_nlp_th ctrl_nlp_th, json_t *json_request, json_t *json_answer)
{
  // reset previews interpretations
  IFE(NlpInterpretRequestReset(ctrl_nlp_th));

  og_char_buffer json_request_string[DPcPathSize];
  IFE(NlpJsonToBuffer(json_request, json_request_string, DPcPathSize, NULL, JSON_INDENT(2)));

  NlpLog(DOgNlpTraceInterpret, "NlpInterpretRequest: interpreting request [\n%s]", json_request_string)

  // parse
  IFE(NlpInterpretRequestParse(ctrl_nlp_th, json_request));

  json_t *json_interpretations = json_array();
  IF(json_object_set_new(json_answer, "interpretations", json_interpretations))
  {
    NlpThrowErrorTh(ctrl_nlp_th, "NlpInterpretRequest: error setting json_interpretations");
    DPcErr;
  }

  IFE(NlpMatch(ctrl_nlp_th));
  IFE(NlpRequestInterpretationsBuild(ctrl_nlp_th, json_interpretations));

  NlpLog(DOgNlpTraceInterpret, "NlpInterpretRequest: finished interpreting request [\n%s]", json_request_string)

  DONE;
}

static og_status NlpInterpretRequestReset(og_nlp_th ctrl_nlp_th)
{
  // mark InterpretRequest package as unused
  int package_used = OgHeapGetCellsUsed(ctrl_nlp_th->hinterpret_package);
  for (int i = 0; i < package_used; i++)
  {
    struct interpret_package *interpret_package = OgHeapGetCell(ctrl_nlp_th->hinterpret_package, i);
    NlpPackageMarkAsUnused(ctrl_nlp_th, interpret_package->package);
  }

  g_queue_clear(ctrl_nlp_th->sorted_request_expressions);
  IFE(NlpInterpretAnyReset(ctrl_nlp_th));
  IFE(OgHeapReset(ctrl_nlp_th->hinterpret_package));
  IFE(OgHeapReset(ctrl_nlp_th->hrequest_word));
  IFE(OgHeapReset(ctrl_nlp_th->hba));
  IFE(OgHeapReset(ctrl_nlp_th->hrequest_input_part));

  IFE(OgHeapReset(ctrl_nlp_th->hrequest_expression));
  IFE(OgHeapReset(ctrl_nlp_th->hrequest_position));
  IFE(OgHeapReset(ctrl_nlp_th->horiginal_request_input_part));
  IFE(OgHeapReset(ctrl_nlp_th->horip));
  IFE(OgHeapReset(ctrl_nlp_th->hrequest_any));

  ctrl_nlp_th->request_sentence = NULL;
  ctrl_nlp_th->show_explanation = FALSE;
  ctrl_nlp_th->loginfo->trace = ctrl_nlp_th->regular_trace;

  DONE;
}

static og_status NlpInterpretRequestParse(og_nlp_th ctrl_nlp_th, json_t *json_request)
{
  json_t *json_packages = NULL;
  json_t *json_sentence = NULL;
  json_t *json_accept_language = NULL;
  json_t *json_show_explanation = NULL;
  json_t *json_trace = NULL;

  for (void *iter = json_object_iter(json_request); iter; iter = json_object_iter_next(json_request, iter))
  {
    og_string key = json_object_iter_key(iter);

    NlpLog(DOgNlpTraceInterpret, "NlpInterpretRequestParse: found key='%s'", key)

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
    else if (Ogstricmp(key, "show-explanation") == 0)
    {
      json_show_explanation = json_object_iter_value(iter);
    }
    else if (Ogstricmp(key, "trace") == 0)
    {
      json_trace = json_object_iter_value(iter);
    }
    else
    {
      NlpThrowErrorTh(ctrl_nlp_th, "NlpInterpretRequestParse: unknow key '%s'", key);
      DPcErr;
    }

  }

  IFN(json_packages)
  {
    NlpThrowErrorTh(ctrl_nlp_th, "NlpInterpretRequestParse: no packages");
    DPcErr;
  }

  IFN(json_sentence)
  {
    NlpThrowErrorTh(ctrl_nlp_th, "NlpInterpretRequestParse: no sentence");
    DPcErr;
  }

  IFX(json_show_explanation)
  {
    if (json_is_string(json_show_explanation))
    {
      og_string string_show_explanation = json_string_value(json_show_explanation);
      if (!Ogstricmp(string_show_explanation, "true"))
      {
        ctrl_nlp_th->show_explanation = TRUE;
      }

      NlpLog(DOgNlpTraceInterpret, "NlpInterpretRequestParse: showing explanation in answer")
    }
    else
    {
      NlpThrowErrorTh(ctrl_nlp_th, "NlpInterpretRequestParse: json_show_explanation is not a string");
      DPcErr;
    }
  }

  IFX(json_trace)
  {
    if (json_is_string(json_trace))
    {
      og_string string_trace = json_string_value(json_trace);
      char *nil;
      ctrl_nlp_th->regular_trace = ctrl_nlp_th->loginfo->trace;
      ctrl_nlp_th->loginfo->trace = strtol(string_trace, &nil, 16);

      NlpLog(DOgNlpTraceInterpret, "NlpInterpretRequestParse: trace changed from %x to %x", ctrl_nlp_th->regular_trace,
          ctrl_nlp_th->loginfo->trace)
    }
    else
    {
      NlpThrowErrorTh(ctrl_nlp_th, "NlpInterpretRequestParse: json_show_explanation is not a string");
      DPcErr;
    }
  }

  // The Accept-Language string can be non extant

  IFE(OgNlpSynchroTestSleepIfTimeoutNeeded(ctrl_nlp_th, nlp_timeout_in_NlpInterpretRequestParse));

  IFE(NlpInterpretRequestBuildSentence(ctrl_nlp_th, json_sentence));
  IFE(NlpInterpretRequestBuildPackages(ctrl_nlp_th, json_packages));
  IFE(NlpInterpretRequestBuildAcceptLanguage(ctrl_nlp_th, json_accept_language));

  DONE;
}

static og_status NlpInterpretRequestBuildSentence(og_nlp_th ctrl_nlp_th, json_t *json_sentence)
{
  if (json_is_string(json_sentence))
  {
    ctrl_nlp_th->request_sentence = json_string_value(json_sentence);

    if (!g_utf8_validate(ctrl_nlp_th->request_sentence, -1, NULL))
    {
      NlpThrowErrorTh(ctrl_nlp_th, "NlpInterpretRequestBuildSentence: sentence contain invalid UTF-8 : '%s'",
          ctrl_nlp_th->request_sentence);
      DPcErr;
    }

    NlpLog(DOgNlpTraceInterpret, "NlpInterpretRequestBuildSentence: sentence '%s'", ctrl_nlp_th->request_sentence)
  }
  else
  {
    NlpThrowErrorTh(ctrl_nlp_th, "NlpInterpretRequestBuildSentence: json_sentence is not a string");
    DPcErr;
  }
  DONE;
}

static og_status NlpInterpretRequestBuildPackages(og_nlp_th ctrl_nlp_th, json_t *json_packages)
{
  int array_size = json_array_size(json_packages);
  for (int i = 0; i < array_size; i++)
  {
    json_t *json_package = json_array_get(json_packages, i);
    IFN(json_package)
    {
      NlpThrowErrorTh(ctrl_nlp_th, "NlpInterpretRequestBuildPackages: null json_package at position %d", i);
      DPcErr;
    }

    if (json_is_string(json_package))
    {
      const char *package_id = json_string_value(json_package);
      NlpLog(DOgNlpTraceInterpret, "NlpInterpretRequestBuildPackages: package id is '%s'", package_id)
      IFE(NlpInterpretRequestBuildPackage(ctrl_nlp_th, package_id));
    }
    else
    {
      NlpThrowErrorTh(ctrl_nlp_th,
          "NlpCompilePackageInterpretations: json_interpretation at position %d is not a string", i);
      DPcErr;
    }

  }

  DONE;
}

static og_status NlpInterpretRequestBuildPackage(og_nlp_th ctrl_nlp_th, const char *package_id)
{
  package_t package = NlpPackageGet(ctrl_nlp_th, package_id);
  IFN(package)
  {
    NlpThrowErrorTh(ctrl_nlp_th, "NlpInterpretRequestBuildPackage: unknown package '%s'", package_id);
    DPcErr;
  }

  int self_index = OgHeapGetCellsUsed(ctrl_nlp_th->hinterpret_package);
  struct interpret_package interpret_package[1];
  interpret_package->self_index = self_index;
  interpret_package->package = package;

  IFE(OgHeapAppend(ctrl_nlp_th->hinterpret_package, 1, interpret_package));
  DONE;
}

// TODO : mettre dans nlplang.c
static og_status NlpInterpretRequestBuildAcceptLanguage(og_nlp_th ctrl_nlp_th, json_t *json_accept_language)
{

  DONE;
}

