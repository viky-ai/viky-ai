/*
 * nls_endpoint_test.c
 *
 *  Created on: Sep 21, 2017
 *      Author: mouadh
 */

#include "ogm_nls.h"

og_status NlsEndpointTest(struct og_listening_thread *lt, struct og_ucisw_input *winput, struct og_ucisr_output *output,
    nls_request_paramList *parametersList)
{
  char *response = NULL;
  json_t * body_json = NULL;
  json_error_t error[1];

  /*
   * setting timeout for request to value given in parameter
   * timeout for request is tmp_request_processing_timeout if != -1
   * equest_processing_timeout else
   */
  if (NlsParamExists(parametersList, "timeout") == TRUE)
  {
    int timeout_new = atoi(NlsGetParamValue(parametersList, "timeout"));
    if (timeout_new > 0)
    {
      lt->ctrl_nls->conf->tmp_request_processing_timeout = timeout_new;
    }

  }

  if (output->hh.request_method == DOgHttpHeaderTypePost)
  {
    int headerSize = output->header_length;
    int contentSize = output->content_length - headerSize;
    if (contentSize > 0)
    {
      body_json = json_loads(output->content + headerSize, contentSize, error);
      if (!body_json)
      {
        NlsJSONThrowError(lt, "endpoint_test", error);
        DPcErr;
      }
    }
  }

  /*
   * Parameters parsing
   */

  if (NlsParamExists(parametersList, "name") == TRUE)
  {
    response = NlsReturnJsonName(NlsGetParamValue(parametersList, "name"));
  }
  if (NlsParamExists(parametersList, "wait") == TRUE)
  {
    NlsWait(NlsGetParamValue(parametersList, "wait"));
    response = NlsReturnJsonWait(NlsGetParamValue(parametersList, "wait"));
  }
  if (NlsParamExists(parametersList, "id") == TRUE)
  {
    json_t * response_json = json_object();
    json_object_set(response_json, "id", json_integer(atoi(NlsGetParamValue(parametersList, "id"))));
    json_object_set(response_json, "wait", json_integer(atoi(NlsGetParamValue(parametersList, "wait"))));
    response = json_dumps(response_json, 0);
    json_decref(response_json);
  }

  /*
   * Handle POST Data
   */

  if (body_json != NULL)
  {
    json_t * reponse_json = json_object();

    json_t * id_json = json_object_get(body_json, "id");
    json_t * wait_json = json_object_get(body_json, "wait");
    json_t * name_json = json_object_get(body_json, "name");
    json_t * timeout_json = json_object_get(body_json, "timeout");

    if (timeout_json != NULL)
    {
      int timeout_new = 0;
      if (json_is_string(timeout_json) == TRUE)
      {
        timeout_new = atoi(json_string_value(timeout_json));
      }
      else if (json_is_number(timeout_json))
      {
        timeout_new = json_number_value(timeout_json);
      }
      if (timeout_new > 0)
      {
        lt->ctrl_nls->conf->tmp_request_processing_timeout = timeout_new;
      }
    }

    if (name_json != NULL)
    {
      json_object_set(reponse_json, "hello", name_json);
    }

    if (id_json != NULL)
    {
      json_object_set(reponse_json, "id", id_json);
    }

    if (wait_json != NULL)
    {

      json_object_set(reponse_json, "wait", wait_json);
      if (json_is_string(wait_json) == TRUE)
      {
        NlsWait(json_string_value(wait_json));
      }
      else if (json_is_integer(wait_json))
      {
        OgSleep(json_integer_value(wait_json));
      }
      else if (json_is_number(wait_json))
      {
        OgSleep(json_number_value(wait_json));
      }
    }
    if (response == NULL)
    {
      if (json_object_size(reponse_json) > 0)
      {
        response = json_dumps(reponse_json, 0);
      }
    }
  }

  if (response == NULL) response = NlsDefaultResponse();

  winput->content = response;
  winput->content_length = strlen(response);

  // setting back timeout for request to default value
  lt->ctrl_nls->conf->tmp_request_processing_timeout = -1;

  DONE;
}

char * NlsReturnJsonName(const char * name)
{
  if (name == NULL) return NULL;
  json_t * response_json = json_object();
  json_object_set(response_json, "hello", json_string(name));
  char * response = json_dumps(response_json, 0);
  json_decref(response_json);
  return response;
}

char * NlsReturnJsonWait(const char * wait)
{
  if (wait == NULL) return NULL;
  json_t * response_json = json_object();
  json_object_set(response_json, "wait", json_string(wait));
  char * response = json_dumps(response_json, 0);
  json_decref(response_json);
  return response;
}

og_status NlsWait(const char * wait)
{
  if (strcmp(wait, "infinite") == 0)
  {
    int locala, localb = 0;
    while (1)
    {
      locala++;
      localb = locala;
    }
    locala = localb;
  }
  else
  {
    int sleep_for = atoi(wait);
    OgSleep(sleep_for);
  }
  DONE;
}

og_bool NlsParamExists(nls_request_paramList *parametersList, char * paramKey)
{
  if (parametersList->length > 0)
  {
    for (int i = 0; i < parametersList->length; i++)
    {
      if (strcmp(parametersList->params[i].key, paramKey) == 0) return TRUE;
    }
  }
  return FALSE;
}

const char * NlsGetParamValue(nls_request_paramList *parametersList, char * paramKey)
{
  if (parametersList->length > 0)
  {
    for (int i = 0; i < parametersList->length; i++)
    {
      if (strcmp(parametersList->params[i].key, paramKey) == 0) return parametersList->params[i].value;
    }
  }
  return NULL;
}

char * NlsDefaultResponse()
{
  json_t * response_json = json_object();
  json_object_set(response_json, "hello", json_string("world"));
  char * response_string = json_dumps(response_json, 0);
  json_decref(response_json);
  return response_string;
}
