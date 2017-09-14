/*
 *  Main listening lt function.
 *  Copyright (c) 2005-2017 Pertimm by Patrick Constant
 *  Dev : May 2005, February 2007, August 2017
 *  Version 1.2
 */
#include <loggen.h>

#include "ogm_nls.h"

static og_status OgNlsLtReleaseCurrentRunnning(struct og_listening_thread * lt);

/**
 *  Listening threads are not really listening the port but they
 *  are started just after the main lt (which is listening)
 *  received a call to the socket.
 */
int OgListeningThread(void *ptr)
{
  struct og_listening_thread *lt = (struct og_listening_thread *) ptr;
  struct og_ctrl_nls *ctrl_nls = lt->ctrl_nls;
  ogint64_t micro_clock_start;
  char v[128];
  int retour;

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

static og_status OgNlsLtReleaseCurrentRunnning(struct og_listening_thread * lt)
{
  struct og_ctrl_nls *ctrl_nls = lt->ctrl_nls;
  lt->running = FALSE;

  IFE(OgSemaphorePost(ctrl_nls->hsem_run3));

  DONE;
}

