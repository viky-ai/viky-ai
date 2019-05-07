/**
 * Nls endpoint ready : nls initialisation status
 *
 * Copyright (c) 2019, Pertimm by Brice Ruzand
 */

#include "ogm_nls.h"

og_status NlsEndpointReadyGet(struct og_listening_thread *lt, struct og_nls_request *request,
    struct og_nls_response *response)
{
  if (lt->ctrl_nls->nls_ready)
  {
    json_object_set_new(response->body, "status", json_string("NLS is now ready"));
  }
  else
  {
    response->http_status = 503;
    response->http_message = "NLS temporarily unavailable";
    json_object_set_new(response->body, "status", json_string("NLS is NOT yet ready, please check again."));
  }
  DONE;
}

og_status NlsEndpointReadySet(struct og_listening_thread *lt, struct og_nls_request *request,
    struct og_nls_response *response)
{
  if (!lt->ctrl_nls->nls_ready)
  {
    OgNlsMemLogPeakUsage(lt->ctrl_nls, "NlsEndpointReadySet");
    OgMsg(lt->ctrl_nls->hmsg, "", DOgMsgDestInLog + DOgMsgDestMBox, "NLS is now ready.");
  }
  lt->ctrl_nls->nls_ready = TRUE;
  json_object_set_new(response->body, "status", json_string("NLS is now ready"));
  DONE;
}

