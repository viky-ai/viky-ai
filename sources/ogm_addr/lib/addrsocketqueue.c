/*
 *  Main function for the thread that is handling the requests
 *  Copyright (c) 2005-2006 Pertimm by Patrick Constant
 *  Dev : March 2005, December 2006, April 2016
 *  Version 1.2
 */
#include "ogm_addr.h"

static og_status OgAddrSocketQueueLoop(struct og_ctrl_addr *ctrl_addr)
{

  while (1)
  {
    struct og_socket_info *stored_info = (struct og_socket_info *) g_async_queue_pop(ctrl_addr->async_socket_queue);

    // copy info on the stack
    struct og_socket_info info[1];
    memcpy(info, stored_info, sizeof(struct og_socket_info));

    // free stored info
    g_slice_free(struct og_socket_info, stored_info);

    if (ctrl_addr->must_stop)
    {
      time_t error_time[1];
      time(error_time);

      OgMsg(ctrl_addr->hmsg, "", DOgMsgDestInLog, "OgAddrSocketQueue %s: request dropped on socket %d because "
          "service stopping at %.24s UTC", ctrl_addr->addr_name, info->hsocket_service, OgGmtime(error_time));

      IFE(ctrl_addr->send_error_status_func(ctrl_addr->func_context, info, 503, "Service Unavailable (stopping)"));

      break;
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
    }
    else
    {
      og_status must_stop = ctrl_addr->answer_func(ctrl_addr->answer_func_context, info);
      IFE(must_stop);
      if (must_stop)
      {
        OgMsg(ctrl_addr->hmsg, "", DOgMsgDestInLog, "OgAddrSocketQueue %s: must_stop request received",
            ctrl_addr->addr_name);
        break;
      }
    }

  }

  DONE;
}

og_status OgAddrSocketQueue(void *ptr)
{
  struct og_ctrl_addr *ctrl_addr = (struct og_ctrl_addr *) ptr;

  // set thread name to identify it in debug
  if (ctrl_addr->addr_name[0])
  {
    IFE(OgThreadSetCurrentName(ctrl_addr->addr_name));
  }

  // loop to consume queue
  og_status status = OgAddrSocketQueueLoop(ctrl_addr);
  IF(status)
  {
    OgMsg(ctrl_addr->hmsg, "", DOgMsgDestInLog, "OgAddrSocketQueue %s: OgAddrSocketQueueLoop failed",
        ctrl_addr->addr_name);
  }

  // Closes all the unprocessed requests
  struct og_socket_info *info = NULL;
  while ((info = (struct og_socket_info *) g_async_queue_try_pop(ctrl_addr->async_socket_queue)))
  {
    time_t error_time[1];
    time(error_time);

    OgMsg(ctrl_addr->hmsg, "", DOgMsgDestInLog, "OgAddrSocketQueue %s: remaining request dropped on socket %d because"
        " service is stopping at %.24s UTC", ctrl_addr->addr_name, info->hsocket_service, OgGmtime(error_time));

    og_status status_callback = ctrl_addr->send_error_status_func(ctrl_addr->func_context, info, 503,
        "Service Unavailable (stopping)");

    // free stored info
    g_slice_free(struct og_socket_info, info);

    IF(status_callback)
    {
      OgMsg(ctrl_addr->hmsg, "", DOgMsgDestInLog, "OgAddrSocketQueue %s: send_error_status_func failed on socket %d.",
          ctrl_addr->addr_name, info->hsocket_service);
    }

  }

  IFE(OgSemaphorePost(ctrl_addr->hsem));

  return status;
}

