/*
 *  This where we read the socket because a request has arrived - UCI style
 *  Copyright (c) 2006-2008 Pertimm by Patrick Constant
 *  Dev : September 2006, February 2008
 *  Version 1.1
 */
#include "ogm_nls.h"

static og_bool OgListeningAnswer(struct og_listening_thread *lt, struct og_ucisw_input *winput,
    struct og_ucisr_output *output, og_bool is_ssi_control_commands);
static og_status OgListeningProcessSearchRequest(struct og_listening_thread *lt, struct og_ucisw_input *winput,
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

  og_bool is_ssi_control_commands = FALSE;
  IFE(OgListeningProcessSearchRequest(lt, winput, output));

  int retour= OgListeningAnswer(lt, winput, output, is_ssi_control_commands);
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

  if (output->hh.request_method > 0 && output->hh.request_method != DOgHttpHeaderTypePost)
  {
    NlsThrowError(lt, "OgListeningThreadAnswerUci: UCI request is not an HTTP POST Request.");
    DPcErr;
  }

  if (output->content_length <= 0 || (output->content_length - output->header_length) <= 1)
  {
    NlsThrowError(lt, "OgListeningThreadAnswerUci: UCI request body is empty.");
    DPcErr;
  }

  DONE;
}

static og_status OgListeningProcessSearchRequest(struct og_listening_thread *lt, struct og_ucisw_input *winput,
    struct og_ucisr_output *output)
{
  int headerSize = output->header_length;
  int contentSize = output->content_length - headerSize;
  lt->request_running = 1;
  IFE(OgNLSJsonReadRequest(lt, output->content + headerSize, contentSize));

  winput->content_length = OgHeapGetCellsUsed(lt->json->hb_json_buffer);
  winput->content = OgHeapGetCell(lt->json->hb_json_buffer, 0);


  // Do all the process

  DONE;
}

static og_bool OgListeningAnswer(struct og_listening_thread *lt, struct og_ucisw_input *winput,
    struct og_ucisr_output *output, og_bool is_ssi_control_commands)
{
  lt->t2 = OgMicroClock();

  IF(OgUciServerWrite(lt->hucis,winput))
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

