/*
 *  This where we read the socket because a request has arrived - UCI style
 *  Copyright (c) 2006-2008 Pertimm by Patrick Constant
 *  Dev : September 2006, February 2008
 *  Version 1.1
 */
#include "ogm_nls.h"

static og_bool OgListeningAnswer(struct og_listening_thread *lt, struct og_ucisw_input *winput,
    struct og_ucisr_output *output);
static og_status OgListeningProcessEndpoint(struct og_listening_thread *lt, struct og_ucisw_input *winput,
    struct og_ucisr_output *output);
static og_status OgListeningRead(struct og_listening_thread *lt, struct og_ucisr_input *input,
    struct og_ucisr_output *output);
static int json_dump_answer_callback(const char *buffer, size_t size, void *data);

/**
 *  Returns 1 if server must stop, 0 otherwise
 *  Returns -1 on error.
 */
og_bool OgListeningThreadAnswerUci(struct og_listening_thread *lt)
{
  struct og_ctrl_nls *ctrl_nls = lt->ctrl_nls;

  lt->t0 = OgMicroClock();

  if (ctrl_nls->must_stop)
  {
    if (lt->loginfo->trace & DOgNlsTraceLT)
    {
      OgMsg(lt->hmsg, "", DOgMsgDestInLog, "OgListeningThreadAnswerUci must stop");
    }
    return TRUE;
  }

  /** Now we read the request and answer the request as an answer **/
  struct og_ucisr_input input[1];
  memset(input, 0, sizeof(struct og_ucisr_input));
  input->hsocket = lt->hsocket_in;

  // input->timeout is used later on select() system function and needs to be converted in seconds
  input->timeout = ctrl_nls->conf->socket_read_timeout;

  struct og_ucisr_output *output = lt->output;

  IFE(OgListeningRead(lt, input, output));

  if (lt->loginfo->trace & DOgNlsTraceLT)
  {
    OgMsg(lt->hmsg, "", DOgMsgDestInLog, "OgListeningThreadErrorUci: lt_%d : request receive", lt->ID);
  }

  lt->t1 = OgMicroClock();

  struct og_ucisw_input winput[1];
  memset(winput, 0, sizeof(struct og_ucisw_input));
  winput->hsocket = input->hsocket;

  IFE(OgListeningProcessEndpoint(lt, winput, output));

  int retour = OgListeningAnswer(lt, winput, output);

  return retour;
}

static og_status OgListeningRead(struct og_listening_thread *lt, struct og_ucisr_input *input,
    struct og_ucisr_output *output)
{

  IF(OgUciServerRead(lt->hucis,input,output))
  {
    int waited_time = (OgMicroClock() - lt->t0) / 1000;
    if (output->timed_out)
    {
      NlsThrowError(lt,
          "OgListeningThreadAnswerUci: timed-out in OgUciServerRead after %d milli-seconds, with timeout=%d milli-seconds",
          waited_time, input->timeout);
      DPcErr;
    }
    else
    {
      OgMsgErr(lt->hmsg, "OgUciRead", 0, 0, 0, DOgMsgSeverityError, DOgErrLogFlagNoSystemError + DOgErrLogFlagNotInErr);
      NlsThrowError(lt,
          "OgListeningThreadAnswerUci: error in OgUciServerRead after %d milli-seconds, if the error is related to \"buffer full\", set max_request_size to a larger value",
          waited_time);
      DPcErr;
    }
  }

  DONE;
}

static og_status OgListeningProcessEndpoint(struct og_listening_thread *lt, struct og_ucisw_input *winput,
    struct og_ucisr_output *output)
{
  winput->content_type = "application/json";
  winput->content_length = 0;
  winput->content = NULL;

  struct og_nls_request *request = lt->request;
  memset(request, 0, sizeof(struct og_nls_request));
  request->raw = output;
  request->parameters = json_object();
  request->parameters_keys = json_array();

  struct og_nls_response *response = lt->response;
  memset(response, 0, sizeof(struct og_nls_response));

  IFE(OgNlsEndpointsParseParameters(lt, output->hh.request_uri, request));

  // Parse json body
  if (request->raw->hh.request_method != DOgHttpHeaderTypeGet)
  {
    int headerSize = output->header_length;
    int contentSize = output->content_length - headerSize;
    if (contentSize > 0)
    {
      json_error_t error[1];
      request->body = json_loads(output->content + headerSize, contentSize, error);
      if (!request->body)
      {
        NlsJSONThrowError(lt, "OgListeningProcessSearchRequest", error);
        DPcErr;
      }
    }
  }

  // Setup an empty json response
  response->default_body = json_object();
  response->body = response->default_body;

  og_bool endpoint_status = OgNlsEndpoints(lt, request, response);
  IFE(endpoint_status);

  // current endpoint not found
  if (endpoint_status == FALSE)
  {
    response->http_status = 404;
    response->http_message = "No endpoint found";

    json_t *errors = json_array();
    json_array_append_new(errors, json_string(response->http_message));
    json_object_set_new(response->body, "errors", errors);

  }

  // convert json_t body to json string
  IF(json_dump_callback(response->body, json_dump_answer_callback, lt, JSON_INDENT(2)))
  {
    NlsThrowError(lt, "OgListeningProcessEndpoint: error on json_dump_callback");
    DPcErr;
  }

  // Build json string from response->body
  winput->http_status = response->http_status;
  winput->http_status_message = response->http_message;
  winput->content = OgHeapGetCell(lt->h_json_answer, 0);
  winput->content_length = OgHeapGetCellsUsed(lt->h_json_answer);



  DONE;
}

int json_dump_answer_callback(const char *buffer, size_t size, void *data)
{
  struct og_listening_thread *lt = data;

  IFE(OgHeapAppend(lt->h_json_answer, size, buffer));

  DONE;
}

static og_bool OgListeningAnswer(struct og_listening_thread *lt, struct og_ucisw_input *winput,
    struct og_ucisr_output *output)
{
  lt->t2 = OgMicroClock();

  IF(OgUciServerWrite(lt->hucis, winput))
  {
    if (lt->loginfo->trace & DOgNlsTraceMinimal)
    {
      OgMsg(lt->hmsg, "", DOgMsgDestInLog + DOgMsgDestInErr,
          "OgListeningThreadAnswerUci: connexion was prematurely closed by client, going on");
    }
  }

  /** This is the only place where we need to close the socket **/
  if (lt->loginfo->trace & DOgNlsTraceSocketSize)
  {
    OgMsg(lt->hmsg, "", DOgMsgDestInLog, "OgListeningThreadAnswerUci: closing socket %d", lt->hsocket_in);
  }
  OgCloseSocket(lt->hsocket_in);

  return FALSE;
}

