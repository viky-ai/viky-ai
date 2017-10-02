/*
 * nls_endpoint_test.c
 *
 *  Created on: Sep 21, 2017
 *      Author: mouadh
 */

#include "ogm_nls.h"

static og_status NlsWait(og_string wait, int wait_int);

og_status NlsEndpointTest(struct og_listening_thread *lt, struct og_nls_request *request,
    struct og_nls_response *response)
{



  // setting timeout for request to value given in parameter timeout.
  // Using  request timeout if timeout > 0 else conf->request_processing_timeout
  og_string timeout = NlsRequestGetParamValue(request, "timeout");
  if (timeout != NULL)
  {
    int timeout_new = atoi(timeout);
    if (timeout_new > 0)
    {
      lt->options->request_processing_timeout = timeout_new;
    }
  }

  /**
   * Parameters parsing
   */
  og_string name = NlsRequestGetParamValue(request, "name");
  if (name != NULL)
  {
    IFE(json_object_set_new(response->body, "hello", json_string(name)));
  }

  og_string wait = NlsRequestGetParamValue(request, "wait");
  if (wait != NULL)
  {
    IFE(NlsWait(wait, -1));

    og_string id = NlsRequestGetParamValue(request, "id");
    if (id != NULL)
    {
      IFE(json_object_set_new(response->body, "id", json_string(id)));
    }

    char *endptr;
    int waitVal = strtol(wait, &endptr, 10);
    if (endptr[0] == '\0')
    {
      IFE(json_object_set_new(response->body, "wait", json_integer(waitVal)));
    }
    else
    {
      IFE(json_object_set_new(response->body, "wait", json_string(wait)));
    }

  }

  /**
   * Handle POST Data
   */
  if (request->body != NULL)
  {
    json_t *id_json = json_object_get(request->body, "id");
    json_t *wait_json = json_object_get(request->body, "wait");
    json_t *name_json = json_object_get(request->body, "name");
    json_t *timeout_json = json_object_get(request->body, "timeout");

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
      json_object_set(response->body, "hello", name_json);
    }

    if (id_json != NULL)
    {
      json_object_set(response->body, "id", id_json);
    }

    if (wait_json != NULL)
    {

      json_object_set(response->body, "wait", wait_json);
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

  }

  // if no params and no body
  if (json_object_size(response->body) == 0)
  {
    IFE(json_object_set_new(response->body, "hello", json_string("world")));
  }

  DONE;
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

