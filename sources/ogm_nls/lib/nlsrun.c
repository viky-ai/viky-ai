/*
 *  Main Natural Language Server function.
 *  Copyright (c) 2017 Pertimm by Patrick Constant
 *  Dev : August 2017
 *  Version 1.0
 */
#include "ogm_nls.h"

static int OgNlsRun1(struct og_ctrl_nls *ctrl_nls);
static int OgNlsRun2(void *ptr, struct og_socket_info *info);
static int OgNlsRun3(void *ptr, struct og_socket_info *info);
static int NlsRunMustStop(void *ptr);
static int NlsRunGetBacklogTimeout(void *ptr);

static struct og_listening_thread *OgNlsRunAcquireRunnningLt(struct og_ctrl_nls *ctrl_nls, struct og_socket_info *info);

PUBLIC(int) OgNlsRun(og_nls handle)
{
  struct og_ctrl_nls *ctrl_nls = handle;
  IF(OgNlsRun1(ctrl_nls))
  {
    // handling server states here, if necessary
    DPcErr;
  }
  DONE;
}

static int OgNlsRun1(struct og_ctrl_nls *ctrl_nls)
{
  struct og_maintenance_thread *mt = &ctrl_nls->mt;
  IFE(OgCreateThread(&mt->IT, OgMaintenanceThread, (void *)mt));

  struct og_addr_param addr_param[1];
  memset(addr_param, 0, sizeof(struct og_addr_param));
  addr_param->herr = ctrl_nls->herr;
  addr_param->hmsg = ctrl_nls->hmsg;
  addr_param->hmutex = ctrl_nls->hmutex;
  addr_param->loginfo.trace = DOgAddrTraceMinimal + DOgAddrTraceMemory;
  addr_param->loginfo.where = ctrl_nls->loginfo->where;
  addr_param->must_stop_func = NlsRunMustStop;
  addr_param->get_backlog_timeout_func = NlsRunGetBacklogTimeout;
  addr_param->send_error_status_func = NlsRunSendErrorStatus;
  addr_param->func_context = ctrl_nls;
  addr_param->backlog_max_pending_requests = ctrl_nls->conf->backlog_max_pending_requests;
  snprintf(addr_param->addr_name, DPcPathSize, "nls_lt_addr");
  IFn(ctrl_nls->haddr = OgAddrInit(addr_param))
  DPcErr;

  if (ctrl_nls->loginfo->trace & DOgNlsTraceMinimal)
  {
    OgMsg(ctrl_nls->hmsg, "", DOgMsgDestInLog, "OgNlsRun starting with following adresses:");
  }

  IFE(OgAddrAdd(ctrl_nls->haddr, ctrl_nls->conf->env->listenning_address, ctrl_nls->conf->env->listenning_port));

  OgMsg(ctrl_nls->hmsg, "", DOgMsgDestInLog + DOgMsgDestMBox, "NLS listening on %s:%d ...",
      ctrl_nls->conf->env->listenning_address, ctrl_nls->conf->env->listenning_port);

  ctrl_nls->must_stop = 0;
  IFE(OgAddrLoop(ctrl_nls->haddr, OgNlsRun2, (void * ) ctrl_nls));

  if (ctrl_nls->conf->permanent_threads)
  {
    IFE(NlsStopPermanentLtThreads(ctrl_nls));
  }

  IFE(OgAddrClose(ctrl_nls->haddr));

  IFE(NlsWaitForListeningThreads("Main thread", ctrl_nls));

  if (ctrl_nls->loginfo->trace & DOgNlsTraceMinimal)
  {
    OgMsg(ctrl_nls->hmsg, "", DOgMsgDestInLog + DOgMsgParamDateIn, "OgNlsRun finished");
  }

  IFE(OgAddrFlush(ctrl_nls->haddr));

  DONE;
}

static int OgNlsRun2(void *ptr, struct og_socket_info *info)
{
  struct og_ctrl_nls *ctrl_nls = (struct og_ctrl_nls *) ptr;
  struct og_listening_thread *lt;
  int i, retour;

  IF(retour = OgNlsRun3(ptr, info))
  {
    /* We need to send the errors from listening threads here,
     * because they cannot be reached after this point */
    for (i = 0; i < ctrl_nls->LtNumber; i++)
    {
      lt = ctrl_nls->Lt + i;
      IFn(lt->herr) continue;
      OgMsgErr(lt->hmsg, "", 0, 0, 0, DOgMsgSeverityEmergency + DOgMsgDestInLog + DOgMsgDestInErr + DOgMsgDestMBox,
      DOgErrLogFlagNoSystemError/*+DOgErrLogFlagNotInErr*/);
    }
  }

  return (retour);
}

static int OgNlsRun3(void *ptr, struct og_socket_info *info)
{
  struct og_ctrl_nls *ctrl_nls = (struct og_ctrl_nls *) ptr;

  if (ctrl_nls->must_stop)
  {
    if (ctrl_nls->loginfo->trace & DOgNlsTraceMinimal)
    {
      OgMsg(ctrl_nls->hmsg, "", DOgMsgDestInLog, "OgNlsRun must stop");
    }
    return (1);
  }

  /** Here we get the IP address of the calling program (direct socket) **/
  IFE(OgGetRemoteAddrSocket(&info->socket_in.sin_addr, ctrl_nls->sremote_addr, 0));
  if (ctrl_nls->loginfo->trace & DOgNlsTraceAllRequest)
  {
    OgMsg(ctrl_nls->hmsg, "", DOgMsgDestInLog + DOgMsgParamDateIn,
        "\n\nOgNlsRun2: receiving request on '%s' at port %d (socket %d) from %s", info->address, info->port,
        info->hsocket_service, ctrl_nls->sremote_addr);
  }

  /** Wait for free running lt */
  struct og_listening_thread *lt = OgNlsRunAcquireRunnningLt(ctrl_nls, info);
  IFN(lt)
  {
    OgMsg(ctrl_nls->hmsg, "", DOgMsgDestInLog + DOgMsgParamDateIn,
        "OgNlsRun: OgNlsRunAcquireRunnningLt: failed '%s' at port %d (socket %d) from %s, closing socket",
        info->address, info->port, info->hsocket_service, ctrl_nls->sremote_addr);
    OgCloseSocket(info->hsocket_service);
    DPcErr;
  }

  /** Copy the socket into the lt structure **/
  lt->hsocket_in = info->hsocket_service;
  lt->request_running_start = info->time_start;

  if (ctrl_nls->loginfo->trace & DOgNlsTraceSocketSize)
  {
    OgMsg(ctrl_nls->hmsg, "", DOgMsgDestInLog, "OgNlsRun2: lt %d got socket %d", lt->ID, lt->hsocket_in);
  }

  if (ctrl_nls->conf->permanent_threads)
  {
    /** Unlocking the lt that will answer the socket **/
    //OgMsg(ctrl_nls->hmsg,"",DOgMsgDestInLog
    //  ,"OgNlsRun2: unlocking lt %d",i);
    IFE(OgSemaphorePost(lt->hsem));
  }
  else
  {
    /** Create the lt that will answer the socket **/
    IFE(OgCreateThread(&lt->IT, OgListeningThread, (void * ) lt));
  }

  return (0);
}

static struct og_listening_thread *OgNlsRunAcquireRunnningLtNoSync(struct og_ctrl_nls *ctrl_nls)
{

  /** Looking for a free lt structure **/
  for (int i = 0; i < ctrl_nls->LtNumber; i++)
  {
    struct og_listening_thread *lt = ctrl_nls->Lt + i;
    if (lt->running) continue;
    lt->running = TRUE;
    return lt;

  }

  return NULL;
}

static struct og_listening_thread *OgNlsRunAcquireRunnningLt(struct og_ctrl_nls *ctrl_nls, struct og_socket_info *info)
{

  struct og_listening_thread * lt = NULL;
  while (lt == NULL)
  {

    // Waiting for a listening thread to be available
    IF(OgSemaphoreWait(ctrl_nls->hsem_run3))
    {
      OgMsg(ctrl_nls->hmsg, "", DOgMsgDestInLog + DOgMsgParamDateIn,
          "OgNlsRun: OgNlsRunAcquireRunnningLt: OgSemaphoreWait failed: '%s' at port %d (socket %d) from %s",
          info->address, info->port, info->hsocket_service, ctrl_nls->sremote_addr);
      return NULL;
    }

    OgEnterCriticalSection(ctrl_nls->hmutex_run_lt);
    lt = OgNlsRunAcquireRunnningLtNoSync(ctrl_nls);
    OgLeaveCriticalSection(ctrl_nls->hmutex_run_lt);
    IFN(lt)
    {
      OgMsg(ctrl_nls->hmsg, "", DOgMsgDestInLog + DOgMsgParamDateIn,
          "OgNlsRun: OgNlsRunAcquireRunnningLt: no lt found on request '%s' at port %d (socket %d) from %s retrying",
          info->address, info->port, info->hsocket_service, ctrl_nls->sremote_addr);
      continue;
    }

  }

  return lt;

}

/**
 * Wait for listening thread finished
 *
 * @param label label to wait for
 * @param ctrl_nls ctrl_nls
 * @return the number of running lt (if waiting timeout)
 */
int NlsWaitForListeningThreads(char *label, struct og_ctrl_nls *ctrl_nls)
{
  int overall_waiting_time = 0;
  int lt_running = 0;

  int timeout_period = ctrl_nls->conf->request_processing_timeout * 1.2;

  // Waiting for threads to finish nicely
  while (TRUE)
  {
    int waiting_time = 200;
    lt_running = 0;

    og_char_buffer lt_running_str[DPcPathSize];
    lt_running_str[0] = 0;

    for (int i = 0; i < ctrl_nls->LtNumber; i++)
    {
      struct og_listening_thread *lt = ctrl_nls->Lt + i;
      if (lt->running)
      {
        int ilt_running_str = strlen(lt_running_str);
        snprintf(lt_running_str + ilt_running_str, DPcPathSize - ilt_running_str, "%s%d", lt_running > 0 ? "," : "", i);
        lt_running++;
      }
    }

    if (lt_running)
    {

      if (ctrl_nls->loginfo->trace & DOgNlsTraceMinimal)
      {
        OgMsg(ctrl_nls->hmsg, "", DOgMsgDestInLog, "%s: lt ( %s ) still running", label, lt_running_str);
      }

      if (overall_waiting_time == 0)
      {
        if (ctrl_nls->loginfo->trace & DOgNlsTraceMinimal)
        {
          OgMsg(ctrl_nls->hmsg, "", DOgMsgDestInLog + DOgMsgParamDateIn,
              "%s: waiting for %d running lt%s before exiting", label, lt_running, (lt_running <= 1) ? "" : "s");
        }
      }
      overall_waiting_time += waiting_time;
      if (overall_waiting_time >= timeout_period)
      {
        if (ctrl_nls->loginfo->trace & DOgNlsTraceMinimal)
        {
          OgMsg(ctrl_nls->hmsg, "", DOgMsgDestInLog + DOgMsgSeverityWarning,
              "%s: some listening threads still running after %d milli-seconds, forcing stop of server", label,
              overall_waiting_time);
        }
        break;
      }
    }
    else
    {
      break;
    }

    OgSleep(waiting_time);
  }

  if (overall_waiting_time)
  {
    if (ctrl_nls->loginfo->trace & DOgNlsTraceMinimal)
    {
      OgMsg(ctrl_nls->hmsg, "", DOgMsgDestInLog, "%s: all listening threads have finished after %d milli-seconds",
          label, overall_waiting_time);
    }
  }

  return lt_running;
}

static int NlsRunMustStop(void *ptr)
{
  struct og_ctrl_nls *ctrl_nls = (struct og_ctrl_nls *) ptr;
  return (ctrl_nls->must_stop);
}

static int NlsRunGetBacklogTimeout(void *ptr)
{
  struct og_ctrl_nls *ctrl_nls = (struct og_ctrl_nls *) ptr;
  return (ctrl_nls->conf->backlog_timeout);
}

int NlsRunSendErrorStatus(void *ptr, struct og_socket_info *info, int error_status, og_string message)
{
  struct og_ctrl_nls *ctrl_nls = (struct og_ctrl_nls *) ptr;
  struct og_ucisw_input winput[1];

  memset(winput, 0, sizeof(struct og_ucisw_input));
  winput->hsocket = info->hsocket_service;

  winput->http_status = error_status;
  winput->http_status_message = message;

  winput->content = "";
  winput->content_length = strlen(winput->content);

  IF(OgUciServerWrite(ctrl_nls->hucis, winput))
  {
    if (ctrl_nls->loginfo->trace & DOgNlsTraceMinimal)
    {
      OgMsg(ctrl_nls->hmsg, "", DOgMsgDestInLog + DOgMsgDestInErr,
          "NlsRunErrorStatus: connexion was prematurely closed by client, going on");
    }
  }

  // close current socket
  OgCloseSocket(info->hsocket_service);

  DONE;
}

