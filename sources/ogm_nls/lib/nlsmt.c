/*
 *  Main maintenance thread function.
 *  Copyright (c) 2006 Pertimm by Patrick Constant
 *  Dev : September 2006
 *  Version 1.0
 */
#include "ogm_nls.h"

#define DOgMaintenanceWaitingPeriod                     1000  // ms
#define DOgMaintenanceWritingCounterProtectionPeriod    1000  // µs time max to update one lt counter
#define DOgMaintenanceStartingPeriod                    2000  // ms

static int OgMaintenanceThreadTick(struct og_maintenance_thread *, int);
static int OgMaintenanceThreadHandleError(struct og_maintenance_thread *);

/*
 *  The maintenance thread never stops, except when the whole program is stopping.
 */

int OgMaintenanceThread(void *ptr)
{
  struct og_maintenance_thread *mt = (struct og_maintenance_thread *) ptr;
  struct og_ctrl_nls *ctrl_nls = mt->ctrl_nls;
  int clock_tick_time, elapsed;
  int error = 0;

  IFE(OgThreadSetCurrentName("nls_mt"));

  if (ctrl_nls->loginfo->trace & DOgNlsTraceMT)
  {
    OgMsg(ctrl_nls->hmsg, "", DOgMsgDestInLog, "OgMaintenanceThread starting");
  }

  elapsed = 0;

  while (1)
  {
    if (elapsed < DOgNlsClockTick)
    {
      /* this avoids using the CPU when doing nothing
       using nanosleep-based OgSleep will cause a drift (~2ms) in the ticker
       due to nanosleep's poor precision
       */
      OgSleep(DOgNlsClockTick - elapsed);
    }

    if (ctrl_nls->must_stop) break;
    clock_tick_time = OgMilliClock();

    /* do it every tick */
    IF(OgMaintenanceThreadTick(mt, clock_tick_time))
    {
      OgMaintenanceThreadHandleError(mt);

      error = 1;
      goto endOgMaintenanceThread;
    }

    elapsed = OgMilliClock() - clock_tick_time;

  } // while (1)

  endOgMaintenanceThread:

  if (ctrl_nls->loginfo->trace & DOgNlsTraceMT)
  {
    OgMsg(ctrl_nls->hmsg, "", DOgMsgDestInLog, "OgMaintenanceThread finished%s", error ? " on error" : "");
  }
  mt->running = 0;

  DONE;
}

static int OgMaintenanceThreadTick(struct og_maintenance_thread *mt, int clock_tick_time)
{
  struct og_ctrl_nls *ctrl_nls = mt->ctrl_nls;

  int nb_running_threads = 0;
  for (int i = 0; i < ctrl_nls->LtNumber; i++)
  {
    struct og_listening_thread *lt = ctrl_nls->Lt + i;
    if (lt->request_running)
    {
      lt->request_running_time = clock_tick_time - lt->request_running_start;

//      OgMsg(ctrl_nls->hmsg, "", DOgMsgDestInLog + DOgMsgDestInErr + DOgMsgParamDateIn,
//          "OgMaintenanceThreadTick %d: request_processing_timeout=%d request_running_time=%d request_running=%d",
//          i, lt->ctrl_nls->conf->request_processing_timeout, lt->request_running_time, lt->request_running);
//
      nb_running_threads++;
      if (lt->ctrl_nls->conf->request_processing_timeout)
      {
        /* The lt->looping is designed so that the request is logged only once
         * This does not kill the request, thus the only way to stop that request
         * (if it is really looping) is to kill the process. Best would be to kill
         * the thread, but as of now (August 29th 2010), that does not seem possible
         * without killing the whole process, and this not the place to do it. */
        if (!lt->looping && lt->request_running_time >= lt->ctrl_nls->conf->request_processing_timeout)
        {
          lt->looping = TRUE;

          OgMsg(ctrl_nls->hmsg, "", DOgMsgDestInLog + DOgMsgDestInErr + DOgMsgParamDateIn,
              "OgMaintenanceThreadTick: a request is running for more than %d milliseconds (%d milliseconds) on thread %d",
              lt->ctrl_nls->conf->request_processing_timeout, lt->request_running_time, lt->ID);

        }
      }
    }
  }

  DONE;
}

static int OgMaintenanceThreadHandleError(struct og_maintenance_thread *mt)
{
  struct og_ctrl_nls *ctrl_nls = mt->ctrl_nls;

  OgMsg(mt->hmsg, "", DOgMsgDestInLog + DOgMsgDestInErr + DOgMsgParamLogDate,
      "OgMaintenanceThread: error on maintenance thread");
  OgMsgErr(mt->hmsg, "maintenance_thread_error", 0, 0, 0, DOgMsgSeverityError,
      DOgErrLogFlagNoSystemError/*+DOgErrLogFlagNotInErr*/);

  /** Making sure we get errors from all error head **/
  OgMsg(mt->hmsg, "", DOgMsgDestInLog + DOgMsgDestInErr + DOgMsgParamLogDate,
      "OgMaintenanceThread: error on maintenance thread (main error stack)");
  OgMsgErr(ctrl_nls->hmsg, "maintenance_thread_error", 0, 0, 0, DOgMsgSeverityError,
      DOgErrLogFlagNoSystemError/*+DOgErrLogFlagNotInErr*/);

  DONE;
}

