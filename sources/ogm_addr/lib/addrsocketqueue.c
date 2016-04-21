/*
 *  Main function for the thread that is handling the requests
 *  Copyright (c) 2005-2006 Pertimm by Patrick Constant
 *  Dev : March 2005, December 2006, April 2016
 *  Version 1.2
 */
#include "ogm_addr.h"

og_status OgAddrSocketQueue(void *ptr)
{
  struct og_ctrl_addr *ctrl_addr = (struct og_ctrl_addr *) ptr;
  struct og_socket_info *info;

  while (1)
  {
    info = (struct og_socket_info *) g_async_queue_pop(ctrl_addr->async_socket_queue);

    if (ctrl_addr->must_stop)
    {
      OgMsg(ctrl_addr->hmsg,"",DOgMsgDestInLog, ctrl_addr->loginfo->where, 0, "OgAddrSocketQueue: request dropped on socket %d because service stopping", info->hsocket_service);
      IFE(ctrl_addr->send_error_status_func(ctrl_addr->func_context,info,503));
      break;
    }

    int time_already_passed = OgMilliClock() - info->time_start;
    int timeout = ctrl_addr->backlog_timeout * 1000;
    if ((timeout > 0) && (time_already_passed >= timeout))
    {
      OgMsg(ctrl_addr->hmsg,"",DOgMsgDestInLog, ctrl_addr->loginfo->where, 0, "OgAddrSocketQueue: request dropped on socket %d because timeout (%d >= %d)"
           , info->hsocket_service, time_already_passed,  timeout);
      IFE(ctrl_addr->send_error_status_func(ctrl_addr->func_context,info,503));
    }
    else
    {
      og_status must_stop;
      IFE(must_stop = ctrl_addr->answer_func(ctrl_addr->answer_func_context, info));
      if (must_stop)
      {
        OgMsg(ctrl_addr->hmsg,"",DOgMsgDestInLog, ctrl_addr->loginfo->where, 0, "OgAddrSocketQueue must_stop request received");
        break;
      }
    }

  }

// Closes all the unprocessed requests
  if (ctrl_addr->must_stop)
  {
    while ((info = (struct og_socket_info *) g_async_queue_try_pop(ctrl_addr->async_socket_queue)))
    {
      OgMsg(ctrl_addr->hmsg,"",DOgMsgDestInLog, ctrl_addr->loginfo->where, 0, "OgAddrSocketQueue: remaining request dropped on socket %d because service stopping", info->hsocket_service);
      IFE(ctrl_addr->send_error_status_func(ctrl_addr->func_context,info,503));
    }
    IFE(OgSemaphorePost(ctrl_addr->hsem));
  }
  DONE;
}

