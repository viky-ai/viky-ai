/*
 *  Main function for the thread that is handling the requests
 *  Copyright (c) 2005-2006 Pertimm by Patrick Constant
 *  Dev : March 2005, December 2006, April 2016
 *  Version 1.2
 */
#include "ogm_addr.h"

og_status OgAddrSocketQueueStop(struct og_ctrl_addr *ctrl_addr)
{
  // Closes all the unprocessed requests

  // send nil socket to stop QueueMaintenanceLoop thread
  struct og_socket_info *nil_socket1 = g_slice_new(struct og_socket_info);
  nil_socket1->nil_socket = TRUE;
  g_async_queue_push(ctrl_addr->async_socket_queue, nil_socket1);

  DONE;
}


/**
 * Check timed out
 *
 * @param ctrl_addr
 * @param info
 * @param p_check_must_stop
 * @param p_time_before_timeout
 * @return
 */
static og_bool OgAddrSocketQueueCheck(struct og_ctrl_addr *ctrl_addr, struct og_socket_info *info,
    og_bool *p_check_must_stop, int *p_time_before_timeout)
{
  og_bool must_stop = ctrl_addr->must_stop_func(ctrl_addr->func_context);
  IFE(must_stop);
  if (must_stop)
  {
    time_t error_time[1];
    time(error_time);

    OgMsg(ctrl_addr->hmsg, "", DOgMsgDestInLog, "OgAddrSocketQueue %s: request dropped on socket %d because "
        "service stopping at %.24s UTC", ctrl_addr->addr_name, info->hsocket_service, OgGmtime(error_time));

    IFE(ctrl_addr->send_error_status_func(ctrl_addr->func_context, info, 503, "Service Unavailable (stopping)"));

    *p_check_must_stop = TRUE;

    return FALSE;
  }
  else
  {
    *p_check_must_stop = FALSE;
  }

  int current_clock = OgMilliClock();
  int time_already_passed = current_clock - info->time_start;

  // We need to use a function since backlog_timeout can change in ssrv at real time
  int timeout = ctrl_addr->get_backlog_timeout_func(ctrl_addr->func_context);
  int down_timeout = ctrl_addr->down_timeout;
  if ((timeout > 0) && (time_already_passed >= timeout))
  {
    time_t error_time[1];
    time(error_time);

    OgMsg(ctrl_addr->hmsg, "", DOgMsgDestInLog, "OgAddrSocketQueue %s: request dropped on socket %d because "
        "timeout (%d >= %d) at %.24s UTC)", ctrl_addr->addr_name, info->hsocket_service, time_already_passed, timeout,
        OgGmtime(error_time));

    IFE(ctrl_addr->send_error_status_func(ctrl_addr->func_context, info, 503, "Service Unavailable (timeout)"));

    return FALSE;

  }
  else if ((down_timeout > 0) && (time_already_passed >= down_timeout))
  {
    time_t error_time[1];
    time(error_time);

    OgMsg(ctrl_addr->hmsg, "", DOgMsgDestInLog, "OgAddrSocketQueue %s: request dropped on socket %d because "
        "a search_unavailable has been send and down_timeout is reached at %.24s UTC)", ctrl_addr->addr_name,
        info->hsocket_service, OgGmtime(error_time));

    og_status status = ctrl_addr->send_error_status_func(ctrl_addr->func_context, info, 503,
        "Service Unavailable (parameter search unavailable activated)");
    IFE(status);

    return FALSE;

  }

  if (p_time_before_timeout != NULL)
  {
    if (time_already_passed > timeout)
    {
      *p_time_before_timeout = (timeout - time_already_passed);
    }
    else
    {
      *p_time_before_timeout = 0;
    }
  }

  return TRUE;
}


/**
 * Check periodically if socket in backlog are not timedout
 */
static og_status OgAddrSocketQueueMaintenanceLoop(void *void_ctrl_addr)
{
  struct og_ctrl_addr *ctrl_addr = (struct og_ctrl_addr *) void_ctrl_addr;

  // set thread name to identify it in debug
  if (ctrl_addr->addr_name[0])
  {
    char current_thread_name[32];
    snprintf(current_thread_name, 32, "%s_mt", ctrl_addr->addr_name);
    IFE(OgThreadSetCurrentName(current_thread_name));
  }

  while (1)
  {
    struct og_socket_info *stored_info = (struct og_socket_info *) g_async_queue_pop(ctrl_addr->async_socket_queue);

    if (stored_info->nil_socket)
    {
      g_async_queue_push(ctrl_addr->async_socket_queue, stored_info);

      break;
    }

    // copy info on the stack
    struct og_socket_info info[1];
    memcpy(info, stored_info, sizeof(struct og_socket_info));

    int time_before_timeout = 0;
    og_bool check_must_stop = FALSE;

    og_bool keep_info = OgAddrSocketQueueCheck(ctrl_addr, info, &check_must_stop, &time_before_timeout);
    IF(keep_info)
    {
      // free stored info
      g_slice_free(struct og_socket_info, stored_info);

      OgMsg(ctrl_addr->hmsg, "", DOgMsgDestInLog + DOgMsgDestInErr,
          "OgAddrSocketQueueLoopMaintenance %s: OgAddrSocketQueueLoopCheck failed on socket %d.", ctrl_addr->addr_name,
          info->hsocket_service);
      IFE(OgSemaphorePost(ctrl_addr->hsem));
      DPcErr;
    }

    if (keep_info)
    {
      // re push socket at top of queue
      g_async_queue_push_front(ctrl_addr->async_socket_queue, stored_info);

      int wait = 0;
      while (wait <= time_before_timeout)
      {
        int wait_iteration = 50;

        // for next timeout
        OgSleep(wait_iteration);

        og_bool must_stop = ctrl_addr->must_stop_func(ctrl_addr->func_context);
        if (must_stop)
        {
          break;
        }

        wait += wait_iteration;
      }

    }
    else
    {
      // free stored info
      g_slice_free(struct og_socket_info, stored_info);
    }

  }

  IFE(OgSemaphorePost(ctrl_addr->hsem));

  OgMsg(ctrl_addr->hmsg, "", DOgMsgDestInLog, "OgAddrSocketQueueMaintenanceLoop %s: finished", ctrl_addr->addr_name);

  DONE;
}

static og_status OgAddrSocketQueueProcessingLoop(struct og_ctrl_addr *ctrl_addr)
{

  while (1)
  {

    struct og_socket_info *stored_info = (struct og_socket_info *) g_async_queue_pop(ctrl_addr->async_socket_queue);

    if (stored_info->nil_socket)
    {
      g_async_queue_push(ctrl_addr->async_socket_queue, stored_info);

      break;
    }

    // copy info on the stack
    struct og_socket_info info[1];
    memcpy(info, stored_info, sizeof(struct og_socket_info));

    int time_before_timeout = 0;
    og_bool check_must_stop = FALSE;

    og_bool call_func = OgAddrSocketQueueCheck(ctrl_addr, info, &check_must_stop, &time_before_timeout);
    IF(call_func)
    {
      // free stored info
      g_slice_free(struct og_socket_info, stored_info);

      OgMsg(ctrl_addr->hmsg, "", DOgMsgDestInLog + DOgMsgDestInErr,
          "OgAddrSocketQueueLoop %s: OgAddrSocketQueueLoopCheck failed on socket %d.", ctrl_addr->addr_name,
          info->hsocket_service);

      DPcErr;
    }

    if (call_func)
    {
      // delegate processing
      og_status must_stop = ctrl_addr->answer_func(ctrl_addr->answer_func_context, info);

      g_slice_free(struct og_socket_info, stored_info);

      IFE(must_stop);
      if (must_stop)
      {
        OgMsg(ctrl_addr->hmsg, "", DOgMsgDestInLog, "OgAddrSocketQueue %s: must_stop request received",
            ctrl_addr->addr_name);
        break;
      }
    }
    else
    {
      // free stored info
      g_slice_free(struct og_socket_info, stored_info);

      // stop processing loop
      if (check_must_stop)
      {
        break;
      }

    }

  }

  OgMsg(ctrl_addr->hmsg, "", DOgMsgDestInLog, "OgAddrSocketQueueProcessingLoop %s: finished", ctrl_addr->addr_name);

  DONE;
}

og_status OgAddrSocketQueue(struct og_ctrl_addr *ctrl_addr)
{
  // add thread to check the queue
  IFE(OgCreateThread(ctrl_addr->maintenance_thread, OgAddrSocketQueueMaintenanceLoop, ctrl_addr));

  // loop to consume queue
  og_status status = OgAddrSocketQueueProcessingLoop(ctrl_addr);
  IF(status)
  {
    OgMsg(ctrl_addr->hmsg, "", DOgMsgDestInLog, "OgAddrSocketQueue %s: OgAddrSocketQueueLoop failed",
        ctrl_addr->addr_name);
  }

  // Closes all the unprocessed requests
  IFE(OgAddrSocketQueueStop(ctrl_addr));

  return status;
}

