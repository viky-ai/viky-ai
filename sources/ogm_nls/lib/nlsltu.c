/*
 *  This where we read the socket because a request has arrived - UCI style
 *  Copyright (c) 2006-2008 Pertimm by Patrick Constant
 *  Dev : September 2006, February 2008
 *  Version 1.1
 */
#include "ogm_nls.h"

static og_bool OgListeningAnswer(struct og_listening_thread *lt, struct og_ucisw_input *winput,
    struct og_ucisr_output *output);
static og_status OgListeningProcessSearchRequest(struct og_listening_thread *lt, struct og_ucisw_input *winput,
    struct og_ucisr_output *output);
static og_status OgListeningRead(struct og_listening_thread *lt, struct og_ucisr_input *input,
    struct og_ucisr_output *output);

static og_status OgGetRequestParameters(struct og_listening_thread *lt, og_string url, struct og_nls_request_paramList *parametersList);
static og_bool compareEndPointWithString(og_string str1, og_string str2);
static int sizeOfString(og_string str);
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

  IFE(OgListeningProcessSearchRequest(lt, winput, output));

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

  /*
   if (output->hh.request_method > 0 && output->hh.request_method != DOgHttpHeaderTypePost)
   {
   NlsThrowError(lt, "OgListeningThreadAnswerUci: UCI request is not an HTTP POST Request.");
   DPcErr;
   }
   */
  if (output->hh.request_method > 0 && output->hh.request_method == DOgHttpHeaderTypePost)
  {
    if (output->content_length <= 0 || (output->content_length - output->header_length) <= 1)
    {
      NlsThrowError(lt, "OgListeningThreadAnswerUci: UCI request body is empty.");
      DPcErr;
    }
  }
  DONE;
}

static og_status OgListeningProcessSearchRequest(struct og_listening_thread *lt, struct og_ucisw_input *winput,
    struct og_ucisr_output *output)
{
  lt->request_running = TRUE;

  struct og_nls_request_paramList parametersList[1];

  // TODO move hba init in NLS init
  IFn(parametersList->hba=OgHeapSliceInit(lt->hmsg,"parametersList_ba",sizeof(unsigned char),0x100,0x100))
  {
    DPcErr;
  }

  IFE(OgGetRequestParameters(lt, output->hh.request_uri, parametersList));

  // TODO add a function register
  if ((compareEndPointWithString("/test", output->hh.request_uri) == 1
      || compareEndPointWithString("/test/", output->hh.request_uri) == 1))
  {
    IFE(NlsEndpointTest(lt, winput, output, parametersList));
  }
  else
  {
    winput->http_status = 404;
    winput->http_status_message = "No endpoint found";
    winput->hsocket = lt->hsocket_in;
    winput->content_length = 0;
  }

  IFE(OgHeapFlush(parametersList->hba));

  DONE;
}

static og_bool OgListeningAnswer(struct og_listening_thread *lt, struct og_ucisw_input *winput,
    struct og_ucisr_output *output)
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


og_bool NlsParamExists(struct og_nls_request_paramList *parametersList, og_string paramKey)
{
  if (parametersList->length > 0)
  {
    for (int i = 0; i < parametersList->length; i++)
    {
      if (strcmp(parametersList->params[i].key, paramKey) == 0) return TRUE;
    }
  }
  return FALSE;
}

og_string NlsGetParamValue(struct og_nls_request_paramList *parametersList, og_string paramKey)
{
  if (parametersList->length > 0)
  {
    for (int i = 0; i < parametersList->length; i++)
    {
      if (strcmp(parametersList->params[i].key, paramKey) == 0) return parametersList->params[i].value;
    }
  }
  return NULL;
}

static og_status OgGetRequestParameters(struct og_listening_thread *lt, og_string url, struct og_nls_request_paramList *parametersList)
{



  UriParserStateA state[1];
  memset(state, 0, sizeof(UriParserStateA));

  UriUriA uri[1];
  memset(uri, 0, sizeof(UriUriA));
  state->uri = uri;

  parametersList->length = 0;

  if (uriParseUriA(state, url) == URI_SUCCESS)
  {

    UriQueryListA *queryList = NULL;
    int itemCount = 0;
    if (uriDissectQueryMallocA(&queryList, &itemCount, uri->query.first, uri->query.afterLast) == URI_SUCCESS)
    {
      for (UriQueryListA * pQItem = queryList; pQItem; pQItem = pQItem->next)
      {
        if (pQItem->key != NULL && pQItem->value != NULL)
        {
          struct og_nls_request_param param[1];
          memset(param, 0, sizeof(struct og_nls_request_param));

          // TODO Heap can be reallocated

          int start = OgHeapGetCellsUsed(parametersList->hba);
          IFE(OgHeapAppend(parametersList->hba, strlen(pQItem->key) + 1, pQItem->key));
          IFn(param->key=OgHeapGetCell(parametersList->hba,start)) DPcErr;

          start = OgHeapGetCellsUsed(parametersList->hba);
          IFE(OgHeapAppend(parametersList->hba, strlen(pQItem->value) + 1, pQItem->value));
          IFn(param->value=OgHeapGetCell(parametersList->hba,start)) DPcErr;

          parametersList->params[parametersList->length] = *param;
          parametersList->length++;
        }
      }

      // TODO ensure FREE
      uriFreeQueryListA(queryList);
    }
  }

  // TODO ensure FREE
  uriFreeUriMembersA(uri);

  DONE;
}

static og_bool compareEndPointWithString(og_string str1, og_string str2)
{
  int are_the_same = 1;
  int size_of_str1 = sizeOfString(str1);

  for (int i = 0; i < size_of_str1; i++)
  {
    if (str2[i] == '?')
    {
      are_the_same = 0;
      break;
    }
    else if (str1[i] != str2[i])
    {
      are_the_same = 0;
      break;
    }
    else if (i == size_of_str1 - 1)
    {
      if (str2[i + 1] != '?' && str2[i + 1] != '\0')
      {
        are_the_same = 0;
        break;
      }
    }
  }
  return are_the_same;
}

static int sizeOfString(og_string str)
{
  int count = 0;
  do
  {
    if (str[count] == '\0') break;
    else count++;
  }
  while (count >= 0);
  return count;
}

