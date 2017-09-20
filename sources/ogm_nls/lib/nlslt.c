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
  int retour;
  pthread_cleanup_push(NlsCancelCleanupOnTimeout,ptr)
    ;
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
  lt->looping = 0;
  lt->loginfo->trace = ctrl_nls->loginfo->trace;

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

  // release lt to be use later
  IFE(OgNlsLtReleaseCurrentRunnning(lt));

  DONE;
}

og_status OgNlsLtReleaseCurrentRunnning(struct og_listening_thread * lt)
{
  struct og_ctrl_nls *ctrl_nls = lt->ctrl_nls;
  lt->running = FALSE;
  lt->current_thread = 0;

  IFE(OgSemaphorePost(ctrl_nls->hsem_run3));

  DONE;
}

og_status NlsListeningThreadReset(struct og_listening_thread * lt)
{
  IFE(OgNLSJsonReset(lt));

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

  NlsListeningThreadReset(lt);

  // restart cancelled thread
  if (lt->ctrl_nls->conf->permanent_threads)
  {
    OgCreateThread(&lt->IT, OgPermanentLtThread, lt);
  }

  OgNlsLtReleaseCurrentRunnning(lt);


  OgMsg(lt->hmsg, "", DOgMsgDestInLog, "lt %d: NlsCancelCleanupOnTimeout finished", lt->ID);
}

