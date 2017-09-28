/*
 * nls_endpoint_dump.c
 *
 *  Created on: Sep 28, 2017
 *      Author: mouadh
 */

#include "ogm_nls.h"
#include "lognlp.h"

og_status NlsEndpointDump(struct og_listening_thread *lt, struct og_nls_request *request,
    struct og_nls_response *response)
{
  struct og_nlp_dump_output output[1];
  IF(OgNlpDump(lt->ctrl_nls->hnlp, NULL, output))
  {
    NlsThrowError(lt, "NlsEndpointDump: error on OgNlpDump");
    DPcErr;
  }

  // Remove the initialized response->body
  json_decref(response->body);

  // Replace it with the output->json_ouput pointer
  response->body = output->json_output;

//  // if no params and no body
//  if (json_object_size(response->body) == 0)
//  {
//    IFE(json_object_set_new(response->body, "hello", json_string("world")));
//  }

  DONE;
}

