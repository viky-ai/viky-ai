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
  lt->request_running = 0;
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
  lt->request_running = TRUE;

  winput->content_type = "application/json";
  winput->content_length = 0;
  winput->content = NULL;

  struct og_nls_request request[1];
  memset(request, 0, sizeof(struct og_nls_request));
  request->raw = output;

  struct og_nls_response response[1];
  memset(response, 0, sizeof(struct og_nls_response));


  // TODO move hba init in NLS init
  request->parameters->length = 0;
  IFn(request->parameters->hba=OgHeapSliceInit(lt->hmsg,"parametersList_ba",sizeof(unsigned char),0x100,0x100))
  {
    DPcErr;
  }

  IFE(OgNlsEndpointsParseParameters(lt, output->hh.request_uri, request->parameters));

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
  response->body = json_object();

  og_bool endpoint_status = OgNlsEndpoints(lt, request, response);
  IFE(endpoint_status);

  // current endpoint not found
  if(endpoint_status == FALSE)
  {
    winput->http_status = 404;
    winput->http_status_message = "No endpoint found";

    json_decref(response->body);

    DONE;
  }

  // Build json string from response->body
  winput->http_status = response->http_status;
  winput->content = json_dumps(response->body, JSON_INDENT(2));
  winput->content_length = strlen(winput->content);
  json_decref(response->body);

  IFE(OgHeapFlush(request->parameters->hba));

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

