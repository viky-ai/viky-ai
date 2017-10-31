/*
 *  Main function for Natural Language Processing library
 *  Copyright (c) 2017 Pertimm, by Patrick Constant
 *  Dev : September 2017
 *  Version 1.0
 */
#include "ogm_nlp.h"

static og_status NlpInterpretRequest(og_nlp_th ctrl_nlp_th, json_t *json_request, json_t *json_answer);
static og_status NlpInterpretRequestReset(og_nlp_th ctrl_nlp_th);
static og_status NlpInterpretRequestPackage(og_nlp_th ctrl_nlp_th, package_t package, json_t *json_interpretations);
static og_status NlpInterpretRequestInterpretation(og_nlp_th ctrl_nlp_th, package_t package, int Iinterpretation, json_t *json_interpretations);
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

  DONE;
}

og_status NlpInterpretReset(og_nlp_th ctrl_nlp_th)
{
  IFE(NlpInterpretRequestReset(ctrl_nlp_th));

  DONE;
}

og_status NlpInterpretFlush(og_nlp_th ctrl_nlp_th)
{
  IFE(OgHeapFlush(ctrl_nlp_th->hinterpret_package));
  ctrl_nlp_th->hinterpret_package = NULL;

  DONE;
}

PUBLIC(og_status) OgNlpInterpret(og_nlp_th ctrl_nlp_th, struct og_nlp_interpret_input *input,
    struct og_nlp_interpret_output *output)
{

  // reset output
  memset(output, 0, sizeof(struct og_nlp_interpret_output));

  og_char_buffer json_interpret_request[DOgMlogMaxMessageSize / 2];
  IFE(NlpJsonToBuffer(input->json_input, json_interpret_request, DOgMlogMaxMessageSize / 2, NULL));

  if (ctrl_nlp_th->loginfo->trace & DOgNlpTraceCompile)
  {
    OgMsg(ctrl_nlp_th->hmsg, "", DOgMsgDestInLog,
        "OgNlpInterpret: ctrl_nlp_th->json_interpret_request_string is [\n%s]", json_interpret_request);
  }

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
  IFE(NlpJsonToBuffer(json_request, json_request_string, DPcPathSize, NULL));

  if (ctrl_nlp_th->loginfo->trace & DOgNlpTraceCompile)
  {
    OgMsg(ctrl_nlp_th->hmsg, "", DOgMsgDestInLog, "NlpInterpretRequest: interpreting request [\n%s]",
        json_request_string);
  }

  // parse
  IFE(NlpInterpretRequestParse(ctrl_nlp_th, json_request));

  json_t *json_interpretations = json_array();
  IF(json_object_set_new(json_answer, "interpretations", json_interpretations))
  {
    NlpThrowErrorTh(ctrl_nlp_th, "NlpInterpretRequest: error setting json_interpretations");
    DPcErr;
  }

  int package_used = OgHeapGetCellsUsed(ctrl_nlp_th->hinterpret_package);
  for (int i = 0; i < package_used; i++)
  {
    struct interpret_package *interpret_package = OgHeapGetCell(ctrl_nlp_th->hinterpret_package, i);
    IFN(interpret_package) DPcErr;

    package_t package = interpret_package->package;

    og_string package_id = OgHeapGetCell(package->hba, package->id_start);
    IFN(package_id) DPcErr;

    if (ctrl_nlp_th->loginfo->trace & DOgNlpTraceCompile)
    {
      OgMsg(ctrl_nlp_th->hmsg, "", DOgMsgDestInLog, "NlpInterpretRequest: searching package '%s'", package_id);
    }

    IFE(NlpInterpretRequestPackage(ctrl_nlp_th, package, json_interpretations));
  }

  if (ctrl_nlp_th->loginfo->trace & DOgNlpTraceCompile)
  {
    OgMsg(ctrl_nlp_th->hmsg, "", DOgMsgDestInLog, "NlpInterpretRequest: finished interpreting request [\n%s]",
        json_request_string);
  }
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

  IFE(OgHeapReset(ctrl_nlp_th->hinterpret_package));

  ctrl_nlp_th->request_sentence = NULL;

  DONE;
}

static og_status NlpInterpretRequestPackage(og_nlp_th ctrl_nlp_th, package_t package, json_t *json_interpretations)
{
  int interpretation_used = OgHeapGetCellsUsed(package->hinterpretation);
  for (int i = 0; i < interpretation_used; i++)
  {
    IFE(NlpInterpretRequestInterpretation(ctrl_nlp_th, package, i, json_interpretations));
  }

  DONE;
}

static og_status NlpInterpretRequestInterpretation(og_nlp_th ctrl_nlp_th, package_t package, int Iinterpretation, json_t *json_interpretations)
{
  struct interpretation *interpretation = OgHeapGetCell(package->hinterpretation, Iinterpretation);
  IFN(interpretation) DPcErr;

  char *interpretation_id = OgHeapGetCell(package->hba, interpretation->id_start);
  IFN(interpretation_id) DPcErr;

  char *interpretation_slug = OgHeapGetCell(package->hba, interpretation->slug_start);
  IFN(interpretation_slug) DPcErr;

  for (int i = 0; i < interpretation->expressions_nb; i++)
  {
    struct expression *expression = OgHeapGetCell(package->hexpression, interpretation->expression_start + i);
    IFN(expression) DPcErr;

    og_string string_expression = OgHeapGetCell(package->hba, expression->text_start);
    if (Ogstricmp(string_expression, ctrl_nlp_th->request_sentence) == 0)
    {
      og_string package_id = OgHeapGetCell(package->hba, package->id_start);
      if (ctrl_nlp_th->loginfo->trace & DOgNlpTraceCompile)
      {
        OgMsg(ctrl_nlp_th->hmsg, "", DOgMsgDestInLog,
            "NlpInterpretRequestInterpretation: found interpretation '%s' '%s' in package '%s'", interpretation_slug,
            interpretation_id, package_id);
      }
      json_t *json_interpretation = json_object();

      json_t *json_package_id = json_string(package_id);
      IF(json_object_set_new(json_interpretation, "package", json_package_id))
      {
        NlpThrowErrorTh(ctrl_nlp_th, "NlpInterpretRequestInterpretation: error setting json_package_id");
        DPcErr;
      }

      json_t *json_interpretation_id = json_string(interpretation_id);
      IF(json_object_set_new(json_interpretation, "id", json_interpretation_id))
      {
        NlpThrowErrorTh(ctrl_nlp_th, "NlpInterpretRequestInterpretation: error setting json_interpretation_id");
        DPcErr;
      }

      json_t *json_interpretation_slug = json_string(interpretation_slug);
      IF(json_object_set_new(json_interpretation, "slug", json_interpretation_slug))
      {
        NlpThrowErrorTh(ctrl_nlp_th, "NlpInterpretRequestInterpretation: error setting json_interpretation_slug");
        DPcErr;
      }

      json_t *json_score = json_real(1.0);
      IF(json_object_set_new(json_interpretation, "score", json_score))
      {
        NlpThrowErrorTh(ctrl_nlp_th, "NlpInterpretRequestInterpretation: error setting json_score");
        DPcErr;
      }

      IF(json_array_append_new(json_interpretations, json_interpretation))
      {
        NlpThrowErrorTh(ctrl_nlp_th, "NlpInterpretRequestInterpretation: error appending json_interpretation to array");
        DPcErr;
      }

    }
  }

  DONE;
}

static og_status NlpInterpretRequestParse(og_nlp_th ctrl_nlp_th, json_t *json_request)
{
  json_t *json_packages = NULL;
  json_t *json_sentence = NULL;
  json_t *json_accept_language = NULL;

  for (void *iter = json_object_iter(json_request); iter; iter = json_object_iter_next(json_request, iter))
  {
    og_string key = json_object_iter_key(iter);

    if (ctrl_nlp_th->loginfo->trace & DOgNlpTraceCompile)
    {
      OgMsg(ctrl_nlp_th->hmsg, "", DOgMsgDestInLog, "NlpInterpretRequestParse: found key='%s'", key);
    }

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

  // The Accept-Language string can be non extant

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

    if (ctrl_nlp_th->loginfo->trace & DOgNlpTraceCompile)
    {
      OgMsg(ctrl_nlp_th->hmsg, "", DOgMsgDestInLog, "NlpInterpretRequestBuildSentence: sentence '%s'",
          ctrl_nlp_th->request_sentence);
    }
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
      if (ctrl_nlp_th->loginfo->trace & DOgNlpTraceCompile)
      {
        OgMsg(ctrl_nlp_th->hmsg, "", DOgMsgDestInLog, "NlpInterpretRequestBuildPackages: package id is '%s'",
            package_id);
      }

      IFE(NlpInterpretRequestBuildPackage(ctrl_nlp_th, package_id));
    }
    else
    {
      NlpThrowErrorTh(ctrl_nlp_th, "NlpCompilePackageInterpretations: json_interpretation at position %d is not a string", i);
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

  struct interpret_package interpret_package[1];
  interpret_package->package = package;

  IFE(OgHeapAppend(ctrl_nlp_th->hinterpret_package, 1, interpret_package));

  DONE;
}

// TODO : mettre dans nlplang.c
static og_status NlpInterpretRequestBuildAcceptLanguage(og_nlp_th ctrl_nlp_th, json_t *json_accept_language)
{

  DONE;
}

