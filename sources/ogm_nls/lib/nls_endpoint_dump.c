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
  IF(OgNlpDump(lt->hnlpi, NULL, output))
  {
    NlsThrowError(lt, "NlsEndpointDump: error on OgNlpDump");
    DPcErr;
  }

  // Replace it with the output->json_ouput pointer
  response->body = json_incref(output->json_output);

  DONE;
}

