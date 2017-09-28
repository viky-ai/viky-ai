/*
 *  Main listening lt function.
 *  Copyright (c) 2005-2017 Pertimm by Patrick Constant
 *  Dev : May 2005, February 2007, August 2017
 *  Version 1.2
 */
#include <loggen.h>

#include "ogm_nls.h"

static int OgListeningThread1(void *ptr);
static void NlsCancelCleanupOnTimeout(void* context);

int OgListeningThread(void *ptr)
{
  int retour = CORRECT;

  pthread_cleanup_push(NlsCancelCleanupOnTimeout, ptr);
  retour = OgListeningThread1(ptr);
  pthread_cleanup_pop(0);

  return (retour);
}

/**
 *  Listening threads are not really listening the port but they
 *  are started just after the main lt (which is listening)
 *  received a call to the socket.
 */
static int OgListeningThread1(void *ptr)
{
  struct og_listening_thread *lt = (struct og_listening_thread *) ptr;
  struct og_ctrl_nls *ctrl_nls = lt->ctrl_nls;
  ogint64_t micro_clock_start;
  char v[128];
  int retour;

  lt->current_thread = pthread_self();

  IFE(NlsLtReset(lt));

  if (!ctrl_nls->conf->permanent_threads)
  {
    char thread_name[OgThrLabelSize];
    snprintf(thread_name, OgThrLabelSize, "nls_lt%d", lt->ID);
    IFE(OgThreadSetCurrentName(thread_name));
  }

  if (lt->loginfo->trace & DOgNlsTraceLT)
  {
    OgMsg(lt->hmsg, "", DOgMsgDestInLog, "lt %d: OgListeningThread starting", lt->ID);
  }

  micro_clock_start = OgMicroClock();

  IF(retour=OgListeningThreadAnswerUci(lt))
  {
    /** Sends error message to the connected client and exits **/
    int is_error = 0;
    if (lt->connection_closed)
    {
      if (lt->loginfo->trace & DOgNlsTraceMinimal)
      {
        OgMsg(lt->hmsg, "", DOgMsgDestInLog + DOgMsgDestInErr,
            "lt %d: OgListeningThread: connexion was prematurely closed by client, going on", lt->ID);
      }
      OgCloseSocket(lt->hsocket_in);
    }
    else
    {
      is_error = OgListeningThreadError(lt);
      OgCloseSocket(lt->hsocket_in);
      IFE(is_error);
    }
    if (is_error)
    {
      if (lt->loginfo->trace & DOgNlsTraceMinimal)
      {
        OgMsg(lt->hmsg, "", DOgMsgDestInLog + DOgMsgDestInErr + DOgMsgParamDateIn,
            "lt %d: OgListeningThread exiting on error while answering", lt->ID);
      }

      NlsLtReset(lt);

      // release lt to be use later
      IFE(OgNlsLtReleaseCurrentRunnning(lt));

      DONE;
    }

  }

  if (lt->loginfo->trace & DOgNlsTraceLT)
  {
    OgFormatThousand((int) (OgMicroClock() - micro_clock_start), v);
    OgMsg(lt->hmsg, "", DOgMsgDestInLog, "lt %d: OgListeningThread finished after %s micro-seconds", lt->ID, v);
  }

  NlsLtReset(lt);

  // release lt to be use later
  IFE(OgNlsLtReleaseCurrentRunnning(lt));

  DONE;
}

og_status OgNlsLtReleaseCurrentRunnning(struct og_listening_thread * lt)
{
  struct og_ctrl_nls *ctrl_nls = lt->ctrl_nls;
  lt->request_running = FALSE;
  lt->current_thread = 0;

  IFE(OgSemaphorePost(ctrl_nls->hsem_run3));

  DONE;
}

/**
 * Reset current request options
 *
 * @param lt current listening thread
 * @return status
 */
static og_status NlsLtResetOptions(struct og_listening_thread *lt)
{

  memset(lt->options, 0, sizeof(struct og_nls_options));
  lt->options->request_processing_timeout = lt->ctrl_nls->conf->request_processing_timeout;

  DONE;
}

static void NlsCancelCleanupOnTimeout(void* context)
{
  struct og_listening_thread *lt = (struct og_listening_thread *) context;
  lt->current_thread = 0;

  OgMsg(lt->hmsg, "", DOgMsgDestInLog, "lt %d: NlsCancelCleanupOnTimeout starting", lt->ID);

  int elapsed = OgMilliClock() - lt->request_running_start;

  NlsThrowError(lt, "NlsCancelCleanupOnTimeout : Request timeout after %d ms", elapsed);

  OgListeningThreadError(lt);
  OgCloseSocket(lt->hsocket_in);

  // reset memory
  NlsLtReset(lt);

  // restart cancelled thread
  if (lt->ctrl_nls->conf->permanent_threads)
  {
    OgCreateThread(&lt->IT, OgPermanentLtThread, lt);
  }

  OgNlsLtReleaseCurrentRunnning(lt);

  OgMsg(lt->hmsg, "", DOgMsgDestInLog, "lt %d: NlsCancelCleanupOnTimeout finished", lt->ID);
}

/**
 * Reset all memory allocated during request
 *
 * @param lt current listening thread
 * @return status
 */
og_status NlsLtInit(struct og_listening_thread *lt)
{
  struct og_ctrl_nls *ctrl_nls = lt->ctrl_nls;
  lt->loginfo[0] = ctrl_nls->loginfo[0];

  IFn(lt->herr=OgErrInit())
  {
    og_char_buffer erreur[DPcPathSize];
    sprintf(erreur, "OgNlsInit: OgErrInit error");
    OgErr(ctrl_nls->herr, erreur);
    DPcErr;
  }
  lt->hmutex = ctrl_nls->hmutex;

  struct og_msg_param msg_param[1];
  memset(msg_param, 0, sizeof(struct og_msg_param));
  msg_param->herr = lt->herr;
  msg_param->hmutex = lt->hmutex;
  msg_param->loginfo.trace = DOgMsgTraceMinimal + DOgMsgTraceMemory;
  msg_param->loginfo.where = ctrl_nls->loginfo->where;
  msg_param->module_name = "nls";
  IFn(lt->hmsg=OgMsgInit(msg_param)) DPcErr;
  IFE(OgMsgTuneInherit(lt->hmsg, ctrl_nls->hmsg));

  struct og_uci_server_param uci_param[1];
  memset(uci_param, 0, sizeof(struct og_uci_server_param));
  uci_param->hmsg = lt->hmsg;
  uci_param->herr = lt->herr;
  uci_param->hmutex = lt->hmutex;
  uci_param->loginfo.trace = DOgUciServerTraceMinimal + DOgUciServerTraceMemory;
  if (ctrl_nls->loginfo->trace & DOgNlsTraceSocket) uci_param->loginfo.trace |= DOgUciServerTraceSocket;
  if (ctrl_nls->loginfo->trace & DOgNlsTraceSocketSize) uci_param->loginfo.trace |= DOgUciServerTraceSocketSize;
  uci_param->socket_buffer_size = ctrl_nls->conf->max_request_size;
  uci_param->loginfo.where = ctrl_nls->loginfo->where;
  IFn(lt->hucis=OgUciServerInit(uci_param)) DPcErr;


  lt->h_json_answer = OgHeapInit(lt->hmsg, "lt_json_answer", sizeof(unsigned char), DPcPathSize / 4);
  IFN(lt->h_json_answer) DPcErr;

  struct og_nlpi_param nlpi_param[1];
  memset(nlpi_param,0,sizeof(struct og_nlpi_param));
  nlpi_param->herr = lt->herr;
  nlpi_param->hmsg = lt->hmsg;
  nlpi_param->hmutex = lt->hmutex;
  nlpi_param->loginfo.trace = DOgNlpTraceMinimal + DOgNlpTraceMemory;
  nlpi_param->loginfo.where = ctrl_nls->loginfo->where;
  lt->hnlpi = OgNlpInterpretInit(ctrl_nls->hnlp,nlpi_param);

  DONE;
}

/**
 * Reset all memory allocated during request
 *
 * @param lt current listening thread
 * @return status
 */
og_status NlsLtReset(struct og_listening_thread *lt)
{
  lt->loginfo->trace = lt->ctrl_nls->loginfo->trace;

  IFE(NlsLtResetOptions(lt));
  IFE(OgHeapReset(lt->h_json_answer));

  DONE;
}

/**
 * Reset all memory allocated during request
 *
 * @param lt current listening thread
 * @return status
 */
og_status NlsLtFlush(struct og_listening_thread *lt)
{
  IFE(OgHeapFlush(lt->h_json_answer));
  IFE(OgUciServerFlush(lt->hucis));
  IFE(OgMsgFlush(lt->hmsg));
  OgErrFlush(lt->herr);

  DONE;
}
