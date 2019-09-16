/*
 *  Endpoint handling for interpretation of a request
 *  Copyright (c) 2017 by Patrick Constant
 *  Dev : August 2017
 *  Version 1.0
 */

#include "ogm_nls.h"

og_status NlsEndpointInterpret(struct og_listening_thread *lt, struct og_nls_request *request,
    struct og_nls_response *response)
{
  if (!lt->ctrl_nls->nls_ready)
  {
    response->http_status = 503;
    response->http_message = "NLS temporarily unavailable";

    json_t *errors = json_array();
    json_array_append_new(errors, json_string(response->http_message));
    json_array_append_new(errors, json_string("NLS is NOT yet ready, please check again."));
    json_object_set_new(response->body, "errors", errors);

    DONE;
  }

  IFN(request->body)
  {
    NlsThrowError(lt, "NlsEndpointInterpret : request body is null");
    DPcErr;
  }

  struct og_nlp_interpret_input input[1];
  struct og_nlp_interpret_output output[1];

  memset(input, 0, sizeof(struct og_nlp_interpret_input));
  input->json_input = request->body;

  IF(OgNlpInterpret(lt->hnlp_th, input, output))
  {
    IFE(OgNlpThreadedGetCustomError(lt->hnlp_th, &response->custom_errors));
    DPcErr;
  }

  response->body = json_incref(output->json_output);

  DONE;
}

