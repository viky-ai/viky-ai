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
static void OgGetRequestParameters(struct og_listening_thread *lt , char * url, nls_request_paramList *paramatersList);

static int compareEndPointWithString(const char * str1,const char *str2);
static int sizeOfString(const char * str);
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
  int headerSize = output->header_length;
  int contentSize = output->content_length - headerSize;
  lt->request_running = 1;

  nls_request_paramList parametersList ;
  IFn(parametersList.hba=OgHeapSliceInit(lt->hmsg,"parametersList_ba",sizeof(unsigned char),0x100,0x100)) DPcErr;

  OgGetRequestParameters(lt,output->hh.request_uri,&parametersList);
  if(parametersList.length > 0)
  {
    for(int i=0;i<parametersList.length;i++)
    {
      OgMsg(lt->hmsg, "", DOgMsgDestInLog + DOgMsgDestMBox,"%s = %s",parametersList.params[i].key,parametersList.params[i].value);
    }
  }

  if(compareEndPointWithString("/hello-world", output->hh.request_uri) == 1 && output->hh.request_method == DOgHttpHeaderTypeGet)
  {
    json_t *helloworld = json_object();

    json_object_set(helloworld, "hello", json_string("world"));
    char * json = json_dumps(helloworld,0);

    winput->content_length = sizeOfString(json);
    winput->content = json;

    //free(json);
    json_decref(helloworld);

  }
  else
  {
    char * error = "error no endpoint";
    winput->content_length = sizeOfString(error);
    winput->content = error;
  }

  IFE(OgHeapFlush(parametersList.hba));

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

static void OgGetRequestParameters(struct og_listening_thread *lt , char * url, nls_request_paramList *parametersList)
{
    UriUriA uri;
    UriQueryListA * queryList;
    UriQueryListA * pQItem;
    int itemCount;

    UriParserStateA state;
    state.uri = &uri;

    parametersList->length = 0;

    if (uriParseUriA(&state, url) == URI_SUCCESS)
    {
      if (uriDissectQueryMallocA(&queryList, &itemCount, uri.query.first,uri.query.afterLast) == URI_SUCCESS)
      {
        for (pQItem = queryList ; pQItem ; pQItem = pQItem->next)
        {
          nls_request_param param;

          int start = OgHeapGetCellsUsed(parametersList->hba);
          IFE(OgHeapAppend(parametersList->hba, strlen(pQItem->key)+1, pQItem->key)) ;
          IFn(param.key=OgHeapGetCell(parametersList->hba,start)) DPcErr;

          start = OgHeapGetCellsUsed(parametersList->hba);
          IFE(OgHeapAppend(parametersList->hba, strlen(pQItem->value)+1, pQItem->value)) ;
          IFn(param.value=OgHeapGetCell(parametersList->hba,start)) DPcErr;

          parametersList->params[parametersList->length] = param ;
          parametersList->length++;
        };
        uriFreeQueryListA(queryList);
      }
    }
    uriFreeUriMembersA(&uri);

}


static int compareEndPointWithString(const char * str1,const char *str2)
{
  int are_the_same = 1 ;
  int size_of_str1 = sizeOfString(str1);

  for(int i=0 ; i< size_of_str1;i++)
  {
    if(str2[i] == '?')
    {
      are_the_same = 0;
      break;
    }
    else if(str1[i] != str2[i])
    {
      are_the_same = 0;
      break;
    }
    else if(i == size_of_str1 - 1)
    {
      if(str2[i+1] != '?' && str2[i+1] != '\0')
        are_the_same=0;
    }
  }
  return are_the_same;
}

static int sizeOfString(const char * str)
{
  int count = 0 ;
  do
  {
    if(str[count] == '\0')
      break;
    else
      count++;
  }while(count>=0);
  return count;
}


