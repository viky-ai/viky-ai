/*
 * nls_endpoint_test.c
 *
 *  Created on: Sep 21, 2017
 *      Author: mouadh
 */

#include "ogm_nls.h"

static char *NlsDefaultResponse();

og_string NlsGetParamValue(struct og_nls_request_paramList *parametersList, og_string paramKey);
og_bool NlsParamExists(struct og_nls_request_paramList *parametersList, og_string paramKey);
char *NlsReturnJsonName(og_string name);
char *NlsReturnJsonWait(og_string wait, og_string _id);
static og_status NlsWait(og_string wait, int wait_int);

og_status NlsEndpointTest(struct og_listening_thread *lt, struct og_ucisw_input *winput, struct og_ucisr_output *output,
    struct og_nls_request_paramList *parametersList)
{
  char *response = NULL;
  json_t * body_json = NULL;
  json_error_t error[1];

  // setting timeout for request to value given in parameter timeout.
  // Using  request timeout if imeout > 0 else conf->request_processing_timeout
  og_string timeout = NlsGetParamValue(parametersList, "timeout");
  if (timeout != NULL)
  {
    int timeout_new = atoi(timeout);
    if (timeout_new > 0)
    {
      lt->options->request_processing_timeout = timeout_new;
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
  og_string name = NlsGetParamValue(parametersList, "name");
  if (name != NULL)
  {
    response = NlsReturnJsonName(name);
  }

  og_string wait = NlsGetParamValue(parametersList, "wait");
  if (wait != NULL)
  {
    IFE(NlsWait(wait, -1));

    og_string id = NlsGetParamValue(parametersList, "id");
    response = NlsReturnJsonWait(wait, id);
    if(response == NULL)
    {
      char erreur[DOgErrorSize];
      sprintf(erreur, "NlsEndpointTest: non numeric wait value : %s", wait);
      OgErr(lt->ctrl_nls->herr, erreur);
      DPcErr;
    }
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
        lt->options->request_processing_timeout = timeout_new;
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
        IFE(NlsWait(json_string_value(wait_json), -1));
      }
      else if (json_is_integer(wait_json))
      {
        IFE(NlsWait(NULL, json_integer_value(wait_json)));
      }
      else if (json_is_number(wait_json))
      {
        IFE(NlsWait(NULL, json_number_value(wait_json)));
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

  if (response == NULL)
  {
    response = NlsDefaultResponse();
  }

  winput->content = response;
  winput->content_length = strlen(response);

  // setting back timeout for request to default value
  lt->ctrl_nls->conf->tmp_request_processing_timeout = -1;

  DONE;
}

char *NlsReturnJsonName(og_string name)
{
  if (name == NULL) return NULL;
  json_t * response_json = json_object();
  json_object_set_new(response_json, "hello", json_string(name));

  char * response = json_dumps(response_json, 0);
  json_decref(response_json);
  return response;
}

char *NlsReturnJsonWait(og_string wait, og_string id)
{
  if (wait == NULL) return NULL;

  json_t *response_json = json_object();

  if (id != NULL)
  {
    json_object_set_new(response_json, "id", json_string(id));
  }

  char *endptr;
  int waitVal = strtol(wait, &endptr, 10);
  if(endptr[0] == '\0')
  {
    json_object_set_new(response_json, "wait", json_integer(waitVal));
    char * response = json_dumps(response_json, 0);
    json_decref(response_json);
    return response;
  }

  return NULL;
}

static og_status NlsWait(og_string wait, int wait_int)
{
  int sleep_for = wait_int;
  if (wait != NULL)
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
      sleep_for = atoi(wait);
    }

  }

  if (sleep_for > 0)
  {
    OgSleep(sleep_for);
  }

  DONE;
}


static char *NlsDefaultResponse()
{
  json_t * response_json = json_object();
  json_object_set_new(response_json, "hello", json_string("world"));

  char * response_string = json_dumps(response_json, 0);
  json_decref(response_json);
  return response_string;
}
