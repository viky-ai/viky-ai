/**
 * Nls endpoint management : registration, param and body parsing, ...
 *
 * Copyright (c) 2017, Pertimm by Brice Ruzand, Sebastien Manfredini
 */

#include "ogm_nls.h"

static og_bool isEndPointRegistered(struct og_nls_request *request, og_string registered_uri, int registered_method);
static og_bool compareEndPointWithString(og_string str1, og_string str2);
static og_status paramInUrlPathUntilStop(og_string url_path, og_char_buffer *detected);
static og_status NlsEndpointsParseParametersAdd(struct og_listening_thread *lt, struct og_nls_request *request,
    og_string key, og_string value);

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
  else if (isEndPointRegistered(request, "/packages/", DOgHttpHeaderTypePost))
  {
    IF(NlsEndpointPackagesPost(lt, request, response))
    {
      NlsThrowError(lt, "OgNlsEndpoints : request error on endpoint : \"POST NlsEndpointPackages\"");
      DPcErr;
    }
  }
  else if (isEndPointRegistered(request, "/packages/", DOgHttpHeaderTypeDelete))
  {
    IF(NlsEndpointPackagesDelete(lt, request, response))
    {
      NlsThrowError(lt, "OgNlsEndpoints : request error on endpoint : \"DELETE NlsEndpointPackages\"");
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
  og_string timeout = json_string_value(json_object_get(request->parameters, "timeout"));
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
        // add param in request
        status = NlsEndpointsParseParametersAdd(lt, request, pQItem->key, pQItem->value);
        IF(status)
        {
          break;
        }
      }

      uriFreeQueryListA(queryList);
    }
  }

  uriFreeUriMembersA(uri);

  IFE(status);

  DONE;
}

static og_status NlsEndpointsParseParametersAdd(struct og_listening_thread *lt, struct og_nls_request *request,
    og_string key, og_string value)
{
  if (key == NULL || value == NULL || !key[0] || !value[0])
  {
    CONT;
  }

  // check  if key is already in param
  if (json_object_get(request->parameters, key))
  {
    NlsThrowError(lt, "NlsEndpointsParseParametersAdd: a parameter named '%s' already exists", key);
    DPcErr;
  }

  // add key/value in parameters
  json_t *key_json = json_string(key);

  // add key in array 'parameters_keys' to keep it in memory
  og_status status = json_array_append(request->parameters_keys, key_json);
  IF(status)
  {
    NlsThrowError(lt, "NlsEndpointsParseParametersAdd: error on json_array_append with key : '%s'", key);
    json_decrefp(&key_json);
    DPcErr;
  }

  status = json_object_set_new(request->parameters, json_string_value(key_json), json_string(value));
  IF(status)
  {
    NlsThrowError(lt, "NlsEndpointsParseParametersAdd: error on json_object_set_new with key : '%s'", value);
    json_decrefp(&key_json);
    DPcErr;
  }

  json_decrefp(&key_json);

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

static og_bool compareEndPointWithString(og_string registered_uri, og_string request_uri)
{
  og_bool are_the_same = TRUE;
  int registered_uri_len = strlen(registered_uri);

  for (int i = 0; i < registered_uri_len; i++)
  {
    if (request_uri[i] == '?')
    {
      are_the_same = FALSE;
      break;
    }
    else if (registered_uri[i] != request_uri[i])
    {
      are_the_same = FALSE;
      break;
    }
    else if (i != 0 && registered_uri[i] == '/' && request_uri[i] == '/')
    {
      break;
    }
    else if (i == registered_uri_len - 1)
    {
      if (request_uri[i + 1] != '?' && request_uri[i + 1] != '\0')
      {
        are_the_same = FALSE;
        break;
      }
    }

  }

  return are_the_same;
}

og_status OgNlsEndpointsParseParametersInUrlPath(struct og_listening_thread *lt, struct og_nls_request *request,
    og_string format)
{
  og_string url = request->raw->hh.request_uri;

  int format_len = strlen(format);
  int url_len = strlen(url);

  // url :    /test/hello/my friends/of/pertimm/
  // format : /test/hello/:path_param1/of/:path_param2/

  for (int i_format = 0, i_url = 0; i_format < format_len - 2; i_format++)
  {
    // match format part
    if (format[i_format] == '/')
    {
      og_string format_part = format + i_format + 1;

      // move forward in url
      og_string url_part = NULL;
      while (i_url < url_len)
      {
        if (url[i_url] == '/')
        {
          i_url++;
          url_part = url + i_url;
          break;
        }
        i_url++;
      }

      // format is matching param
      if (format_part[0] == ':' && url_part != NULL)
      {

        og_char_buffer detectedKey[DPcPathSize];
        paramInUrlPathUntilStop(format_part + 1, detectedKey);

        og_char_buffer detectedValue[DPcPathSize];
        paramInUrlPathUntilStop(url_part, detectedValue);

        // url decode
        uriUnescapeInPlaceA(detectedValue);

        // add param in request
        IFE(NlsEndpointsParseParametersAdd(lt, request, detectedKey, detectedValue));

      }

    }

  }

  DONE;
}


static og_status paramInUrlPathUntilStop(og_string url_path, og_char_buffer *detected)
{
  int url_path_len = strlen(url_path);
  int i = 0;
  for (i = 0; i < url_path_len; i++)
  {
    if (url_path[i] == '/' || url_path[i] == '?' || url_path[i] == '#')
    {
      break;
    }
    detected[i] = url_path[i];
  }

  detected[i] = '\0';

  DONE;
}
