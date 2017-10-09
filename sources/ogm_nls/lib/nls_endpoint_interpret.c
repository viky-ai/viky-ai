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
  IFN(request->body)
  {
    NlsThrowError(lt, "NlsEndpointInterpret : request body is null");
    DPcErr;
  }

  struct og_nlp_interpret_input input[1];
  struct og_nlp_interpret_output output[1];

  memset(input, 0, sizeof(struct og_nlp_interpret_input));
  input->json_input = request->body;

  IFE(OgNlpInterpret(lt->hnlpi, input, output));

  response->body = json_incref(output->json_output);

  DONE;
}

