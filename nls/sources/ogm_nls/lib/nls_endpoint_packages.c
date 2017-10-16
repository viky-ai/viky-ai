/*
 *  Endpoint handling for interpretation of a request
 *  Copyright (c) 2017 by Patrick Constant
 *  Dev : August 2017
 *  Version 1.0
 */

#include "ogm_nls.h"

og_status NlsEndpointPackagesPost(struct og_listening_thread *lt, struct og_nls_request *request,
    struct og_nls_response *response)
{

  IFE(OgNlsEndpointsParseParametersInUrlPath(lt, request, "/packages/:package_id"));

  og_string package_id = json_string_value(json_object_get(request->parameters, "package_id"));
  if (package_id == NULL)
  {
    NlsThrowError(lt, "NlsEndpointPackagesPost: no such param 'package_id'.");
    DPcErr;
  }

  struct og_nlp_compile_input input[1];
  memset(input, 0, sizeof(struct og_nlp_interpret_input));
  input->json_input = request->body;
  input->package_id = package_id;
  input->package_update = TRUE;

  IFE(OgNlpPackageAdd(lt->hnlp_th, input));

  og_char_buffer responsechar[DPcPathSize];
  sprintf(responsechar, "Package '%s' successfully updated", package_id);
  IFE(json_object_set_new(response->body, "status", json_string(responsechar)));

  DONE;
}

og_status NlsEndpointPackagesDelete(struct og_listening_thread *lt, struct og_nls_request *request,
    struct og_nls_response *response)
{
  IFE(OgNlsEndpointsParseParametersInUrlPath(lt, request, "/packages/:package_id"));

  og_string package_id = json_string_value(json_object_get(request->parameters, "package_id"));
  if (package_id == NULL)
  {
    NlsThrowError(lt, "NlsEndpointPackagesPost: no such param 'package_id'.");
    DPcErr;
  }

  IFE(OgNlpPackageDelete(lt->hnlp_th, package_id));

  og_char_buffer responsechar[DPcPathSize];
  sprintf(responsechar, "Package '%s' successfully deleted", package_id);
  IFE(json_object_set_new(response->body, "status", json_string(responsechar)));

  DONE;
}
