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
static og_status NlpInterpretRequestBuildPrimaryPackage(og_nlp_th ctrl_nlp_th, json_t *json_primary_package);
static og_status NlpInterpretRequestBuildContexts(og_nlp_th ctrl_nlp_th, json_t *json_contexts);
static og_status NlpInterpretRequestBuildContext(og_nlp_th ctrl_nlp_th, const char *flag);

og_status NlpInterpretInit(og_nlp_th ctrl_nlp_th, struct og_nlp_threaded_param *param)
{

  // setup request memory for future interpretation
  ctrl_nlp_th->regular_trace = ctrl_nlp_th->loginfo->trace;

  og_char_buffer nlpc_name[DPcPathSize];
  snprintf(nlpc_name, DPcPathSize, "%s_interpret_package", param->name);
  ctrl_nlp_th->hinterpret_package = OgHeapInit(ctrl_nlp_th->hmsg, nlpc_name, sizeof(struct interpret_package),
  DOgNlpPackageNumber);
  IFN(ctrl_nlp_th->hinterpret_package)
  {
    NlpThrowErrorTh(ctrl_nlp_th, "OgNlpInterpretInit : error on OgHeapInit(%s)", nlpc_name);
    DPcErr;
  }
  snprintf(nlpc_name, DPcPathSize, "%s_request_context", param->name);
  ctrl_nlp_th->hrequest_context = OgHeapInit(ctrl_nlp_th->hmsg, nlpc_name, sizeof(struct request_context),
  DOgNlpRequestContextNumber);
  IFN(ctrl_nlp_th->hrequest_context)
  {
    NlpThrowErrorTh(ctrl_nlp_th, "OgNlpInterpretInit : error on OgHeapInit(%s)", nlpc_name);
    DPcErr;
  }
  snprintf(nlpc_name, DPcPathSize, "%s_accept_language", param->name);
  ctrl_nlp_th->haccept_language = OgHeapInit(ctrl_nlp_th->hmsg, nlpc_name, sizeof(struct accept_language),
  DOgNlpAcceptLanguageNumber);
  IFN(ctrl_nlp_th->haccept_language)
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

  IFE(NlpGlueInit(ctrl_nlp_th));
  IFE(NlpWhyNotMatchingInit(ctrl_nlp_th, param->name));
  IFE(NlpMatchGroupNumbersInit(ctrl_nlp_th));
  IFE(NlpRequestExpressionListsSortInit(ctrl_nlp_th, param->name));

  DONE;
}

og_status NlpInterpretReset(og_nlp_th ctrl_nlp_th)
{
  IFE(NlpInterpretRequestReset(ctrl_nlp_th));

  DONE;
}

og_status NlpInterpretFlush(og_nlp_th ctrl_nlp_th)
{
  int request_expression_used = OgHeapGetCellsUsed(ctrl_nlp_th->hrequest_expression);
  if (request_expression_used > 0)
  {
    struct request_expression *request_expressions = OgHeapGetCell(ctrl_nlp_th->hrequest_expression, 0);
    for (int i = 0; i < request_expression_used; i++)
    {
      struct request_expression *request_expression = request_expressions + i;
      for (GList *iter = request_expression->tmp_solutions->head; iter; iter = iter->next)
      {
        struct alias_solution *alias_solution = iter->data;
        json_decrefp(&alias_solution->json_solution);
        g_slice_free(struct alias_solution, alias_solution);
        iter->data = NULL;
      }
      g_queue_clear(request_expression->tmp_solutions);
      g_queue_clear(request_expression->sorted_flat_list);
    }
  }

  IFE(NlpMatchGroupNumbersFlush(ctrl_nlp_th));
  IFE(NlpGlueFlush(ctrl_nlp_th));
  IFE(NlpWhyNotMatchingFlush(ctrl_nlp_th));
  IFE(NlpRequestExpressionListsSortFlush(ctrl_nlp_th));

  g_queue_clear(ctrl_nlp_th->sorted_request_expressions);
  IFE(NlpInterpretAnyFlush(ctrl_nlp_th));
  IFE(OgHeapFlush(ctrl_nlp_th->hinterpret_package));
  IFE(OgHeapFlush(ctrl_nlp_th->hrequest_context));
  IFE(OgHeapFlush(ctrl_nlp_th->haccept_language));
  IFE(OgHeapFlush(ctrl_nlp_th->hrequest_word));
  IFE(OgHeapFlush(ctrl_nlp_th->hba));
  IFE(OgHeapFlush(ctrl_nlp_th->hrequest_input_part));
  IFE(OgHeapFlush(ctrl_nlp_th->hrequest_expression));
  IFE(OgHeapFlush(ctrl_nlp_th->hrequest_position));
  IFE(OgHeapFlush(ctrl_nlp_th->horiginal_request_input_part));
  IFE(OgHeapFlush(ctrl_nlp_th->horip));


  ctrl_nlp_th->hinterpret_package = NULL;
  ctrl_nlp_th->hrequest_context = NULL;
  ctrl_nlp_th->hrequest_word = NULL;
  ctrl_nlp_th->hba = NULL;
  ctrl_nlp_th->hrequest_input_part = NULL;
  ctrl_nlp_th->hrequest_expression = NULL;
  ctrl_nlp_th->hrequest_position = NULL;
  ctrl_nlp_th->horiginal_request_input_part = NULL;
  ctrl_nlp_th->horip = NULL;
  ctrl_nlp_th->hrequest_any = NULL;

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

  ctrl_nlp_th->json_answer_unit = json_answer;

  og_char_buffer json_request_string[DPcPathSize];
  IFE(NlpJsonToBuffer(json_request, json_request_string, DPcPathSize, NULL, JSON_INDENT(2)));

  NlpLog(DOgNlpTraceInterpret, "NlpInterpretRequest: interpreting request [\n%s]", json_request_string)

  // parse
  IFE(NlpInterpretRequestParse(ctrl_nlp_th, json_request));

  // ====================================
  // look for matching interpretation
  // ====================================
  IFE(NlpMatch(ctrl_nlp_th));

  json_t *json_interpretations = json_array();
  IF(json_object_set_new(json_answer, "interpretations", json_interpretations))
  {
    NlpThrowErrorTh(ctrl_nlp_th, "NlpInterpretRequest: error setting json_interpretations");
    DPcErr;
  }

  // build response
  IFE(NlpRequestInterpretationsBuild(ctrl_nlp_th, json_interpretations));

  int nm_expression_used = OgHeapGetCellsUsed(ctrl_nlp_th->hnm_expression);
  if (nm_expression_used > 0)
  {
    IFE(NlpWhyJson(ctrl_nlp_th, json_answer));
  }

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

  int request_expression_used = OgHeapGetCellsUsed(ctrl_nlp_th->hrequest_expression);
  if (request_expression_used > 0)
  {
    struct request_expression *request_expressions = OgHeapGetCell(ctrl_nlp_th->hrequest_expression, 0);
    for (int i = 0; i < request_expression_used; i++)
    {
      struct request_expression *request_expression = request_expressions + i;
      for (GList *iter = request_expression->tmp_solutions->head; iter; iter = iter->next)
      {
        struct alias_solution *alias_solution = iter->data;
        json_decrefp(&alias_solution->json_solution);
        g_slice_free(struct alias_solution, alias_solution);
        iter->data = NULL;
      }
      g_queue_clear(request_expression->tmp_solutions);
      g_queue_clear(request_expression->sorted_flat_list);

      json_decrefp(&request_expression->json_solution);

    }
  }

  IFE(NlpGlueReset(ctrl_nlp_th));
  IFE(NlpWhyNotMatchingReset(ctrl_nlp_th));
  IFE(NlpWarningReset(ctrl_nlp_th));

  g_queue_clear(ctrl_nlp_th->sorted_request_expressions);
  IFE(NlpInterpretAnyReset(ctrl_nlp_th));
  IFE(OgHeapReset(ctrl_nlp_th->hinterpret_package));
  IFE(OgHeapReset(ctrl_nlp_th->hrequest_context));
  IFE(OgHeapReset(ctrl_nlp_th->haccept_language));
  IFE(OgHeapReset(ctrl_nlp_th->hrequest_word));
  IFE(OgHeapReset(ctrl_nlp_th->hba));
  IFE(OgHeapReset(ctrl_nlp_th->hrequest_input_part));

  IFE(OgHeapReset(ctrl_nlp_th->hrequest_expression));
  IFE(OgHeapReset(ctrl_nlp_th->hrequest_position));
  IFE(OgHeapReset(ctrl_nlp_th->horiginal_request_input_part));
  IFE(OgHeapReset(ctrl_nlp_th->horip));

  ctrl_nlp_th->request_sentence = NULL;
  ctrl_nlp_th->date_now = NULL;
  ctrl_nlp_th->show_explanation = FALSE;
  ctrl_nlp_th->show_private = FALSE;
  ctrl_nlp_th->primary_package = NULL;
  ctrl_nlp_th->primary_package_id = NULL;

  ctrl_nlp_th->basic_request_word_used = -1;
  ctrl_nlp_th->basic_group_request_word_nb = -1;

  ctrl_nlp_th->loginfo->trace = ctrl_nlp_th->regular_trace;

  DONE;
}

static og_status NlpInterpretRequestParse(og_nlp_th ctrl_nlp_th, json_t *json_request)
{
  json_t *json_primary_package = NULL;
  json_t *json_packages = NULL;
  json_t *json_contexts = NULL;
  json_t *json_sentence = NULL;
  json_t *json_accept_language = NULL;
  json_t *json_show_explanation = NULL;
  json_t *json_show_private = NULL;
  json_t *json_why_not_matching = NULL;
  json_t *json_auto_complete = NULL;
  json_t *json_trace = NULL;
  json_t *json_date_now = NULL;

  for (void *iter = json_object_iter(json_request); iter; iter = json_object_iter_next(json_request, iter))
  {
    og_string key = json_object_iter_key(iter);

    NlpLog(DOgNlpTraceInterpret, "NlpInterpretRequestParse: found key='%s'", key)

    if (Ogstricmp(key, "primary-package") == 0)
    {
      json_primary_package = json_object_iter_value(iter);
    }
    else if (Ogstricmp(key, "packages") == 0)
    {
      json_packages = json_object_iter_value(iter);
    }
    else if (Ogstricmp(key, "contexts") == 0)
    {
      json_contexts = json_object_iter_value(iter);
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
    else if (Ogstricmp(key, "show-private") == 0)
    {
      json_show_private = json_object_iter_value(iter);
    }
    else if (Ogstricmp(key, "why-not-matching") == 0)
    {
      json_why_not_matching = json_object_iter_value(iter);
    }
    else if (Ogstricmp(key, "auto-complete") == 0)
    {
      json_auto_complete = json_object_iter_value(iter);
    }
    else if (Ogstricmp(key, "trace") == 0)
    {
      json_trace = json_object_iter_value(iter);
    }
    else if (Ogstricmp(key, "now") == 0)
    {
      json_date_now = json_object_iter_value(iter);
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
    else if (json_is_boolean(json_show_explanation))
    {
      ctrl_nlp_th->show_explanation = json_boolean_value(json_show_explanation);
      if (ctrl_nlp_th->show_explanation)
      {
        NlpLog(DOgNlpTraceInterpret, "NlpInterpretRequestParse: showing explanation in answer")
      }
    }
    else
    {
      NlpThrowErrorTh(ctrl_nlp_th, "NlpInterpretRequestParse: json_show_explanation is not a string");
      DPcErr;
    }
  }

  IFX(json_show_private)
  {
    if (json_is_boolean(json_show_private))
    {
      ctrl_nlp_th->show_private = json_boolean_value(json_show_private);
      if (ctrl_nlp_th->show_private)
      {
        NlpLog(DOgNlpTraceInterpret, "NlpInterpretRequestParse: showing private interpretation in answer")
      }
    }
    else
    {
      NlpThrowErrorTh(ctrl_nlp_th, "NlpInterpretRequestParse: show-private is not a string");
      DPcErr;
    }
  }

  IFX(json_auto_complete)
  {
    if (json_is_boolean(json_auto_complete))
    {
      ctrl_nlp_th->auto_complete = json_boolean_value(json_auto_complete);
      if (ctrl_nlp_th->auto_complete)
      {

        NlpLog(DOgNlpTraceInterpret, "NlpInterpretRequestParse: auto-complete request")
      }
    }
    else
    {
      NlpThrowErrorTh(ctrl_nlp_th, "NlpInterpretRequestParse: json_auto_complete is not a boolean");
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

  IFX(json_date_now)
  {
    if (json_is_string(json_date_now))
    {
      ctrl_nlp_th->date_now = json_string_value(json_date_now);
      NlpLog(DOgNlpTraceInterpret, "NlpInterpretRequestParse: date_now set to %s", ctrl_nlp_th->date_now)
    }
    else
    {
      NlpThrowErrorTh(ctrl_nlp_th, "NlpInterpretRequestParse: date_now is not a string");
      DPcErr;
    }
  }

  // The Accept-Language string can be non extant

  // setup
  IFE(NlpJsRequestSetup(ctrl_nlp_th));

  IFE(OgNlpSynchroTestSleepIfTimeoutNeeded(ctrl_nlp_th, nlp_timeout_in_NlpInterpretRequestParse));

  IFE(NlpInterpretRequestBuildContexts(ctrl_nlp_th, json_contexts));
  IFE(NlpInterpretRequestBuildSentence(ctrl_nlp_th, json_sentence));
  IFE(NlpInterpretRequestBuildPackages(ctrl_nlp_th, json_packages));
  IFE(NlpInterpretRequestBuildPrimaryPackage(ctrl_nlp_th, json_primary_package));
  IFE(NlpInterpretRequestBuildAcceptLanguage(ctrl_nlp_th, json_accept_language));
  IFE(NlpWhyNotMatchingBuild(ctrl_nlp_th, json_why_not_matching));

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

static og_status NlpInterpretRequestBuildPackagesListCallback(og_nlp_th ctrl_nlp_th, og_string package_id)
{
  NlpLog(DOgNlpTraceInterpret, "NlpInterpretRequestBuildPackages: package id is '%s'", package_id)
  IF(NlpInterpretRequestBuildPackage(ctrl_nlp_th, package_id))
  {
    NlpThrowErrorTh(ctrl_nlp_th, "NlpInterpretRequestBuildPackagesListCallback: "
        "NlpInterpretRequestBuildPackage failed on package '%s'", package_id);
    DPcErr;
  }

  DONE;
}

static og_status NlpInterpretRequestBuildPackages(og_nlp_th ctrl_nlp_th, json_t *json_packages)
{
  if (json_is_string(json_packages))
  {
    og_string packages = json_string_value(json_packages);

    // joker '*' means all packages
    if (!strcmp(packages, "*"))
    {
      NlpLog(DOgNlpTraceInterpret, "NlpInterpretRequestBuildPackages: '*' using all available packages");
      IFE(NlpPackageListInternal(ctrl_nlp_th, NlpInterpretRequestBuildPackagesListCallback));
    }
    else
    {
      NlpThrowErrorTh(ctrl_nlp_th, "NlpInterpretRequestBuildPackages: 'packages' is not a array");
      DPcErr;
    }

  }
  else if (json_is_array(json_packages))
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
        og_string package_id = json_string_value(json_package);
        NlpLog(DOgNlpTraceInterpret, "NlpInterpretRequestBuildPackages: package id is '%s'", package_id)
        IFE(NlpInterpretRequestBuildPackage(ctrl_nlp_th, package_id));
      }
      else
      {
        NlpThrowErrorTh(ctrl_nlp_th, "NlpInterpretRequestBuildPackages: 'packages' at position %d is not a string", i);
        DPcErr;
      }

    }
  }
  else
  {
    NlpThrowErrorTh(ctrl_nlp_th, "NlpInterpretRequestBuildPackages: 'packages' is not a array");
    DPcErr;
  }

  IFE(NlpCheckPackages(ctrl_nlp_th));

  DONE;
}

static og_status NlpInterpretRequestBuildPrimaryPackage(og_nlp_th ctrl_nlp_th, json_t *json_primary_package)
{
  if (json_primary_package == NULL) CONT;

  if (json_typeof(json_primary_package) == JSON_NULL) CONT;

  if (!json_is_string(json_primary_package))
  {
    NlpThrowErrorTh(ctrl_nlp_th, "NlpInterpretRequestBuildPrimaryPackage: 'primary-packages' is not a string");
    DPcErr;
  }
  else
  {
    ctrl_nlp_th->primary_package_id = json_string_value(json_primary_package);
  }

  // lookup primary package
  ctrl_nlp_th->primary_package = NlpPackageGet(ctrl_nlp_th, ctrl_nlp_th->primary_package_id);
  IFN(ctrl_nlp_th->primary_package)
  {
    NlpThrowErrorTh(ctrl_nlp_th, "NlpInterpretRequestBuildPrimaryPackage: unknown primary-package '%s'",
        ctrl_nlp_th->primary_package_id);
    DPcErr;
  }

  // check if primary package is in packages list
  og_bool primary_package_is_used = FALSE;
  int package_used = OgHeapGetCellsUsed(ctrl_nlp_th->hinterpret_package);
  struct interpret_package *interpret_package_all = OgHeapGetCell(ctrl_nlp_th->hinterpret_package, 0);
  for (int i = 0; i < package_used; i++)
  {
    if (interpret_package_all[i].package == ctrl_nlp_th->primary_package)
    {
      primary_package_is_used = TRUE;
      break;
    }
  }

  if (!primary_package_is_used)
  {
    NlpThrowErrorTh(ctrl_nlp_th, "NlpInterpretRequestBuildPrimaryPackage: primary-package '%s' must"
        " be listed in 'packages'", ctrl_nlp_th->primary_package_id);
    DPcErr;
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

static og_status NlpInterpretRequestBuildContexts(og_nlp_th ctrl_nlp_th, json_t *json_contexts)
{
  IFN(json_contexts) DONE;
  if (json_is_array(json_contexts))
  {
    int array_size = json_array_size(json_contexts);
    for (int i = 0; i < array_size; i++)
    {
      json_t *json_context = json_array_get(json_contexts, i);
      IFN(json_context)
      {
        NlpThrowErrorTh(ctrl_nlp_th, "NlpInterpretRequestBuildContexts: null json_context at position %d", i);
        DPcErr;
      }

      if (json_is_string(json_context))
      {
        og_string flag = json_string_value(json_context);
        NlpLog(DOgNlpTraceInterpret, "NlpInterpretRequestBuildContexts: flag is '%s'", flag)
        IFE(NlpInterpretRequestBuildContext(ctrl_nlp_th, flag));
      }
      else
      {
        NlpThrowErrorTh(ctrl_nlp_th, "NlpInterpretRequestBuildContexts: 'contexts' at position %d is not a string", i);
        DPcErr;
      }

    }
  }
  else
  {
    NlpThrowErrorTh(ctrl_nlp_th, "NlpInterpretRequestBuildcontexts: 'contexts' is not a array");
    DPcErr;
  }

  DONE;
}

static og_status NlpInterpretRequestBuildContext(og_nlp_th ctrl_nlp_th, const char *flag)
{
  struct request_context request_context[1];

  request_context->flag_start = OgHeapGetCellsUsed(ctrl_nlp_th->hba);
  request_context->flag_length = strlen(flag);
  IFE(OgHeapAppend(ctrl_nlp_th->hba, request_context->flag_length, flag));
  IFE(OgHeapAppend(ctrl_nlp_th->hba, 1, ""));

  IFE(OgHeapAppend(ctrl_nlp_th->hrequest_context, 1, request_context));

  DONE;
}

