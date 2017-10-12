/*
 *  Endpoint handling for interpretation of a request
 *  Copyright (c) 2017 by Patrick Constant
 *  Dev : August 2017
 *  Version 1.0
 */

#include "ogm_nls.h"

static og_bool getUntilStop(char* inputStr, int startFrom, char* detected);
static og_bool getJSonUrlValues(json_t *dataObject, char* url);

og_status NlsEndpointPackages(struct og_listening_thread *lt, struct og_nls_request *request,
    struct og_nls_response *response)
{
  json_t *uriData = json_object();
  getJSonUrlValues(uriData, request->raw->hh.request_uri);
  json_t *json_package_name = json_object_get(uriData,"package_name");
  if(json_package_name == NULL)
  {
    NlsThrowError(lt, "NlsEndpointPackages: error on NlsEndpointPackages : package name not found");
    DPcErr;
  }
  const char* package_name = json_string_value(json_package_name);

  struct og_nlp_interpret_input input[1];
  memset(input, 0, sizeof(struct og_nlp_interpret_input));
  input->json_input = request->body;

  IFE(OgNlpPackageAdd(lt->ctrl_nls->hnlp, input));

  char responsechar[DPcPathSize];
  sprintf(responsechar, "Package %s successfully updated", package_name);
  IFE(json_object_set_new(response->body, "Package update", json_string(responsechar)));

  DONE;
}

og_status NlsEndpointPackageDelete(struct og_listening_thread *lt, struct og_nls_request *request,
    struct og_nls_response *response)
{
  json_t *uriData = json_object();
  getJSonUrlValues(uriData, request->raw->hh.request_uri);
  const json_t *json_package_name = json_object_get(uriData,"package_name");
  if(json_package_name == NULL)
  {
    NlsThrowError(lt, "NlsEndpointPackageDelete: error on NlsEndpointPackageDelete : package name not found");
    DPcErr;
  }
  const char* package_name = json_string_value(json_package_name);

  IFE(OgNlpPackageDelete(lt->ctrl_nls->hnlp, package_name));

  char responsechar[DPcPathSize];
  sprintf(responsechar, "Package %s successfully deleted", package_name);
  IFE(json_object_set_new(response->body, "Package delete", json_string(responsechar)));

  DONE;
}

static og_bool getJSonUrlValues(json_t *dataObject, char* url)
{
  char * format = "/packages/:package_name";

  int i_url = 0;
  int i_format = 0;

  int ilen = strlen(url);
  for (i_url = 0; i_url < ilen - 1; i_url++)
  {
    if (format[i_format] == '/' && format[i_format + 1] == ':')
    {
      char detectedValue[DPcPathSize];
      getUntilStop(url, i_url + 1, detectedValue);
      char detectedKey[DPcPathSize];
      getUntilStop(format, i_format + 2, detectedKey);

      json_t *json_detected_value = json_string(detectedValue);
      json_object_set_new(dataObject, detectedKey, json_detected_value);

    }

    i_format++;
  }
  DONE;
}

static og_bool getUntilStop(char* inputStr, int startFrom, char* detected)
{
  int counter = 0;
  // int i=startFrom;
  int ilen = strlen(inputStr);
  for (int i = startFrom; i < ilen; i++)
  {
    if (inputStr[i] == '/' || inputStr[i] == '?' || inputStr[i] == '#') break;
    detected[counter] = inputStr[i];
    counter++;
  }
  detected[counter] = '\0';
  DONE;
}
