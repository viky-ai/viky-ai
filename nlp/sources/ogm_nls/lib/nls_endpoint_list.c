/*
 * nls_endpoint_list.c
 *
 *  Created on: 14 nov. 2017
 *      Author: sebastien
 */
#include "ogm_nls.h"

og_status NlsEndpointList(struct og_listening_thread *lt, struct og_nls_request *request,
    struct og_nls_response *response)
{
  struct og_nlp_package_list output[1];
  IF(OgNlpPackageList(lt->hnlp_th, NULL, output))
  {
    NlsThrowError(lt, "NlsEndpointList: error on OgNlpPackageList");
    DPcErr;
  }

  // Replace it with the output->json_ouput pointer
  response->body = json_incref(output->package_names);

  DONE;
}
