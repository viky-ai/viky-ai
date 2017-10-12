/**
 * Nls endpoint management : registration, param and body parsing, ...
 *
 * Copyright (c) 2017, Pertimm by Brice Ruzand, Sebastien Manfredini
 */

#include "ogm_nls.h"

static og_bool isEndPointRegistered(struct og_nls_request *request, og_string registered_uri, int registered_method);
static og_bool compareEndPointWithString(og_string str1, og_string str2);

/**
 * Dispatch NLS enpoints
 *
 * @param lt og_listening_thread
 * @param request current request info
 * @param response current response
 * @return TRUE if current request match an endpoint, either FALSE or ERROR
 */
og_bool OgNlsEndpoints(struct og_listening_thread *lt, struct og_nls_request *request, struct og_nls_response *response)
{
  // interpret must be the first endpoint (because it is the most used)
  if (isEndPointRegistered(request, "/interpret/", DOgHttpHeaderTypePost))
  {
    IF(NlsEndpointInterpret(lt, request, response))
    {
      NlsThrowError(lt, "OgNlsEndpoints : request error on endpoint : \"POST NlsEndpointInterpret\"");
      DPcErr;
    }
  }
  else if (isEndPointRegistered(request, "/test/", DOgHttpHeaderTypeGet))
  {
    IF(NlsEndpointTest(lt, request, response))
    {
      NlsThrowError(lt, "OgNlsEndpoints : request error on endpoint : \"GET NlsEndpointTest\"");
      DPcErr;
    }
  }
  else if (isEndPointRegistered(request, "/test/", DOgHttpHeaderTypePost))
  {
    IF(NlsEndpointTest(lt, request, response))
    {
      NlsThrowError(lt, "OgNlsEndpoints : request error on endpoint : \"POST NlsEndpointTest\"");
      DPcErr;
    }
  }
  else if (isEndPointRegistered(request, "/dump/", DOgHttpHeaderTypeGet))
  {
    IF(NlsEndpointDump(lt, request, response))
    {
      NlsThrowError(lt, "OgNlsEndpoints : request error on endpoint : \"GET NlsEndpointDump\"");
      DPcErr;
    }
  }
  else if (isEndPointRegistered(request, "/packages/", DOgHttpHeaderTypePost))
    {
      IF(NlsEndpointPackages(lt, request, response))
      {
        NlsThrowError(lt, "OgNlsEndpoints : request error on endpoint : \"POST NlsEndpointPackages\"");
        DPcErr;
      }
    }
  else if (isEndPointRegistered(request, "/packages/", DOgHttpHeaderTypeDelete))
      {
        IF(NlsEndpointPackageDelete(lt, request, response))
        {
          NlsThrowError(lt, "OgNlsEndpoints : request error on endpoint : \"DELETE NlsEndpointPackages\"");
          DPcErr;
        }
      }
  else
  {
    return FALSE;
  }

  return TRUE;
}

og_status OgNlsEndpointsCommonParameters(struct og_listening_thread *lt, struct og_nls_request *request)
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

  DONE;
}


og_bool NlsRequestParamExists(struct og_nls_request *request, og_string paramKey)
{
  if (json_object_get(request->parameters, paramKey) == NULL) return FALSE;
  return TRUE;
}

og_string NlsRequestGetParamValue(struct og_nls_request *request, og_string paramKey)
{
  json_t * object = json_object_get(request->parameters, paramKey);
  if (object == NULL)
  {
    return NULL;
  }
  else
  {
    const char * str = json_string_value(object);
    return str;
  }
}

og_status OgNlsEndpointsMemoryReset(struct og_listening_thread *lt)
{
  struct og_nls_request *request = lt->request;
  json_decrefp(&request->parameters);
  json_decrefp(&request->parameters_keys);
  json_decrefp(&request->body);

  struct og_nls_response *response = lt->response;
  if (response->default_body != response->body)
  {
    json_decrefp(&response->default_body);
  }
  else
  {
    response->default_body = NULL;
  }
  json_decrefp(&response->body);

  DONE;
}

og_status OgNlsEndpointsParseParameters(struct og_listening_thread *lt, og_string url, struct og_nls_request *request)
{
  UriParserStateA state[1];
  memset(state, 0, sizeof(UriParserStateA));

  UriUriA uri[1];
  memset(uri, 0, sizeof(UriUriA));
  state->uri = uri;

  og_status status = CORRECT;

  if (uriParseUriA(state, url) == URI_SUCCESS)
  {

    UriQueryListA *queryList = NULL;
    int itemCount = 0;
    if (uriDissectQueryMallocA(&queryList, &itemCount, uri->query.first, uri->query.afterLast) == URI_SUCCESS)
    {
      for (UriQueryListA * pQItem = queryList; pQItem; pQItem = pQItem->next)
      {
        if (pQItem->key != NULL && pQItem->value != NULL)
        {
          json_t *key = json_string(pQItem->key);

          status = json_array_append(request->parameters_keys, key);
          IF(status)
          {
            NlsThrowError(lt, "OgNlsEndpointsParseParameters: error on json_array_append with key : '%s'", pQItem->key);
            json_decrefp(&key);
            break;
          }

          status = json_object_set_new(request->parameters, json_string_value(key), json_string(pQItem->value));
          IF(status)
          {
            NlsThrowError(lt, "OgNlsEndpointsParseParameters: error on json_object_set_new with key : '%s'",
                pQItem->key);
            json_decrefp(&key);
            break;
          }

          json_decrefp(&key);

        }
      }

      uriFreeQueryListA(queryList);
    }
  }

  uriFreeUriMembersA(uri);

  IFE(status);

  DONE;
}

static og_bool isEndPointRegistered(struct og_nls_request *request, og_string registered_uri, int registered_method)
{
  og_string request_uri = request->raw->hh.request_uri;
  int request_method = request->raw->hh.request_method;

  if (registered_method == request_method)
  {
    if (compareEndPointWithString(registered_uri, request_uri))
    {
      return TRUE;
    }
    else
    {
      og_char_buffer base_uri[DOgHttpHeaderLineSize];
      int uri_lenght = strlen(registered_uri);
      if (registered_uri[uri_lenght - 1] == '/')
      {
        snprintf(base_uri, DOgHttpHeaderLineSize, "%.*s", uri_lenght - 1, registered_uri);
      }

      if (compareEndPointWithString(base_uri, request_uri))
      {
        return TRUE;
      }
    }
  }

  return FALSE;
}

static og_bool compareEndPointWithString(og_string str1, og_string str2)
{
  og_bool are_the_same = TRUE;
  int size_of_str1 = strlen(str1);

  for (int i = 0; i < size_of_str1; i++)
  {

    if (str2[i] == '?')
    {
      are_the_same = FALSE;
      break;
    }
    else if (str1[i] != str2[i])
    {
      are_the_same = FALSE;
      break;
    }
    else if ( i != 0 && str1[i] == '/' && str2[i] == '/')
    {
      break;
    }
    else if (i == size_of_str1 - 1)
    {
      if (str2[i + 1] != '?' && str2[i + 1] != '\0')
      {
        are_the_same = FALSE;
        break;
      }
    }

  }
  return are_the_same;
}

