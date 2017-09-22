/*
 * endpoint-test.c
 *
 *  Created on: Sep 21, 2017
 *      Author: mouadh
 */

#include "ogm_nls.h"


og_status endpoint_test(struct og_listening_thread *lt, struct og_ucisw_input *winput, struct og_ucisr_output *output,
    nls_request_paramList *parametersList)
{
  char *response = NULL;
  json_t * body_json = NULL;
  json_error_t error[1];

  /*
   * Body parsing
   */
  if (param_exists(parametersList, "timeout") == 1)
  {
    int timeout_new = atoi(get_param_value(parametersList, "timeout"));
    if(timeout_new > 0)
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

  if (param_exists(parametersList, "name") == 1)
  {
    response = returnJsonName(get_param_value(parametersList, "name"));
  }
  if (param_exists(parametersList, "wait") == 1)
  {
    wait(get_param_value(parametersList, "wait"));
    response = returnJsonWait(get_param_value(parametersList, "wait"));
  }
  if (param_exists(parametersList, "id") == 1)
  {
    json_t * response_json = json_object();
    json_object_set(response_json, "id", json_integer(atoi(get_param_value(parametersList, "id"))));
    json_object_set(response_json, "wait", json_integer(atoi(get_param_value(parametersList, "wait"))));
    response = json_dumps(response_json, 0);
    json_decref(response_json);
  }

  /*
   * Handel POST Data
   */

  if (body_json != NULL)
  {
    json_t * reponse_json = json_object();

    json_t * id_json = json_object_get(body_json, "id");
    json_t * wait_json = json_object_get(body_json, "wait");
    json_t * name_json = json_object_get(body_json, "name");
    json_t * timeout_json = json_object_get(body_json, "timeout");

    if(timeout_json != NULL)
    {
      int timeout_new = 0;
      if(json_is_string(timeout_json)==TRUE)
      {
        timeout_new = atoi(json_string_value(timeout_json));
      }
      else if(json_is_number(timeout_json))
      {
        timeout_new = json_number_value(timeout_json);
      }
      if(timeout_new > 0)
      {
        lt->ctrl_nls->conf->tmp_request_processing_timeout = timeout_new;
      }
    }

    if(name_json != NULL)
    {
      json_object_set(reponse_json, "name", name_json);
    }

    if (id_json != NULL)
    {
      json_object_set(reponse_json, "id", id_json);
    }

    if (wait_json != NULL)
    {

      json_object_set(reponse_json, "wait", wait_json);
      if(json_is_string(wait_json)==TRUE)
      {
        wait(json_string_value(wait_json));
      }
      else if(json_is_integer(wait_json))
      {
        OgSleep(json_integer_value(wait_json));
      }
      else if(json_is_number(wait_json))
      {
        OgSleep(json_number_value(wait_json));
      }
    }
    if(response == NULL)
    {
      if(json_object_size(reponse_json) > 0)
      {
        response = json_dumps(reponse_json, 0);
      }
    }
  }

  if (response == NULL) response = default_response();

  winput->content = response;
  winput->content_length = strlen(response);
  lt->ctrl_nls->conf->tmp_request_processing_timeout = -1 ;
  DONE;
}

char * returnJsonName(const char * name)
{
  if(name == NULL)
    return NULL;
  json_t * response_json = json_object();
  json_object_set(response_json, "hello", json_string(name));
  char * response = json_dumps(response_json, 0);
  json_decref(response_json);
  return response;
}

char * returnJsonWait(const char * wait)
{
  if(wait == NULL)
    return NULL;
  json_t * response_json = json_object();
  json_object_set(response_json, "wait", json_string(wait));
  char * response = json_dumps(response_json, 0);
  json_decref(response_json);
  return response;
}

og_status wait(const char * wait)
{
  if (strcmp(wait, "infinite") == 0)
  {
    int locala, localb=0;
    while (1)
    {
      locala++;
      localb=locala;
    }
    locala=localb;
  }
  else
  {
    int sleep_for = atoi(wait);
    OgSleep(sleep_for);
  }
  DONE;
}

int param_exists(nls_request_paramList *parametersList, char * paramKey)
{
  if (parametersList->length > 0)
  {
    for (int i = 0; i < parametersList->length; i++)
    {
      if (strcmp(parametersList->params[i].key, paramKey) == 0) return 1;
    }
  }
  return 0;
}

const char * get_param_value(nls_request_paramList *parametersList, char * paramKey)
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

char * default_response()
{
  json_t * response_json = json_object();
  json_object_set(response_json, "hello", json_string("world"));
  char * response_string = json_dumps(response_json, 0);
  json_decref(response_json);
  return response_string;
}
