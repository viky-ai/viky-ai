/*
 *  Handling permanent listening lt function.
 *  Copyright (c) 2007 Pertimm by Patrick Constant
 *  Dev : February 2007
 *  Version 1.0
 */
#include "ogm_nls.h"

int OgPermanentLtThread(void *ptr)
{
  struct og_listening_thread *lt = (struct og_listening_thread *) ptr;

  char thread_name[OgThrLabelSize];
  snprintf(thread_name, OgThrLabelSize, "nls_lt%d", lt->ID);
  IFE(OgThreadSetCurrentName(thread_name));

  if (lt->loginfo->trace & DOgNlsTraceLT)
  {
    OgMsg(lt->hmsg, "", DOgMsgDestInLog, "OgPermanentLtThread %d: starting", lt->ID);
  }

  while (1)
  {
    IFE(OgSemaphoreWait(lt->hsem));
    if (lt->must_stop) break;
    IF(OgListeningThread(ptr))
    {
      OgMsg(lt->hmsg, "", DOgMsgDestInLog, "OgPermanentLtThread %d: OgListeningThread exiting on error (continuing):",
          lt->ID);
      OgMsgErr(lt->hmsg, "permanent_thread_error", 0, 0, 0, DOgMsgSeverityError,
          DOgErrLogFlagNoSystemError + DOgErrLogFlagNotInErr);
    }
  }

  if (lt->loginfo->trace & DOgNlsTraceLT)
  {
    OgMsg(lt->hmsg, "", DOgMsgDestInLog, "OgPermanentLtThread %d: finished", lt->ID);
  }

  lt->is_stopped = TRUE;

  DONE;
}

int NlsInitPermanentLtThreads(struct og_ctrl_nls *ctrl_nls)
{
  for (int i = 0; i < ctrl_nls->LtNumber; i++)
  {
    struct og_listening_thread *lt = ctrl_nls->Lt + i;
    IFE(OgSemaphoreInit(lt->hsem, 0));
    IFE(OgCreateThread(&lt->IT, OgPermanentLtThread, (void * )lt));
  }

  DONE;
}

int NlsStopPermanentLtThreads(struct og_ctrl_nls *ctrl_nls)
{


  for (int i = 0; i < ctrl_nls->LtNumber; i++)
  {
    struct og_listening_thread *lt = ctrl_nls->Lt + i;
    lt->must_stop = 1;
    IFE(OgSemaphorePost(lt->hsem));
  }

  DONE;
}

int NlsFlushPermanentLtThreads(struct og_ctrl_nls *ctrl_nls)
{
  for (int i = 0; i < ctrl_nls->LtNumber; i++)
  {
    struct og_listening_thread *lt = ctrl_nls->Lt + i;
    IFE(OgSemaphoreFlush(lt->hsem));
  }

  for (int i = 0; i < ctrl_nls->LtNumber; i++)
  {
    int max_wait_count = 0;
    struct og_listening_thread *lt = ctrl_nls->Lt + i;
    while (!lt->is_stopped && max_wait_count < 200)
    {
      IFE(OgSleep(DOgNlsClockTick));
      max_wait_count++;
    }

  }

  DONE;
}

