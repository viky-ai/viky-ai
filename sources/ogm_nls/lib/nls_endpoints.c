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
  if (isEndPointRegistered(request, "/test/", DOgHttpHeaderTypeGet))
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
  else if (isEndPointRegistered(request, "/interpret/", DOgHttpHeaderTypePost))
  {
    IF(NlsEndpointInterpret(lt, request, response))
    {
      NlsThrowError(lt, "OgNlsEndpoints : request error on endpoint : \"POST NlsEndpointInterpret\"");
      DPcErr;
    }
  }
  else
  {
    return FALSE;
  }

  return TRUE;
}

og_bool NlsRequestParamExists(struct og_nls_request *request, og_string paramKey)
{
  struct og_nls_request_paramList *parametersList = request->parameters;

  if (parametersList->length > 0)
  {
    for (int i = 0; i < parametersList->length; i++)
    {
      if (strcmp(parametersList->params[i].key, paramKey) == 0) return TRUE;
    }
  }

  return FALSE;
}

og_string NlsRequestGetParamValue(struct og_nls_request *request, og_string paramKey)
{

  struct og_nls_request_paramList *parametersList = request->parameters;
  if (parametersList->length > 0)
  {
    for (int i = 0; i < parametersList->length; i++)
    {
      if (strcmp(parametersList->params[i].key, paramKey) == 0) return parametersList->params[i].value;
    }
  }
  return NULL;
}

og_status OgNlsEndpointsParseParameters(struct og_listening_thread *lt, og_string url,
    struct og_nls_request_paramList *parametersList)
{

  UriParserStateA state[1];
  memset(state, 0, sizeof(UriParserStateA));

  UriUriA uri[1];
  memset(uri, 0, sizeof(UriUriA));
  state->uri = uri;

  parametersList->length = 0;

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
          struct og_nls_request_param param[1];
          memset(param, 0, sizeof(struct og_nls_request_param));

          int start = OgHeapGetCellsUsed(parametersList->hba);
          IFE(OgHeapAppend(parametersList->hba, strlen(pQItem->key) + 1, pQItem->key));
          IFn(param->key=OgHeapGetCell(parametersList->hba,start)) DPcErr;

          start = OgHeapGetCellsUsed(parametersList->hba);
          IFE(OgHeapAppend(parametersList->hba, strlen(pQItem->value) + 1, pQItem->value));
          IFn(param->value=OgHeapGetCell(parametersList->hba,start)) DPcErr;

          parametersList->params[parametersList->length] = *param;
          parametersList->length++;
        }
      }

      // TODO ensure FREE
      uriFreeQueryListA(queryList);
    }
  }

  // TODO ensure FREE
  uriFreeUriMembersA(uri);

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

