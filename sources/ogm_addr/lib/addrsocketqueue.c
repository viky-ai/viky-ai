/*
 *  Function listening to all added addresses.
 *  Copyright (c) 2005-2006 Pertimm by Patrick Constant
 *  Dev : March 2005, December 2006
 *  Version 1.1
 */
#include "ogm_addr.h"

og_status OgAddrSocketQueue(void *ptr)
{
  struct og_ctrl_addr *ctrl_addr = (struct og_ctrl_addr *) ptr;

  while (1)
  {
    struct og_socket_info *info = (struct og_socket_info *) g_async_queue_pop(ctrl_addr->async_socket_queue);

    int time_already_passed = OgMilliClock() - info->time_start;
    int timeout = info->timeout * 1000;
    if ((timeout > 0) && (time_already_passed >= timeout))
    {
      OgMessageLog(DOgMlogInLog, ctrl_addr->loginfo->where, 0, "socket %d timeout", info->hsocket_service);
      IFE(AddrSendStatusCodeServiceUnavailable(ctrl_addr, info->hsocket_service));
      OgCloseSocket(info->hsocket_service);
    }
    else
    {
      OgMessageLog(DOgMlogInLog, ctrl_addr->loginfo->where, 0, "socket %d treated", info->hsocket_service);

      // TODO gestion du must_stop
      og_bool must_stop = info->func(info->ptr, info);
    }
  }

  //IFE(must_stop = (*func)(ptr, info));
  //if (must_stop) goto endOgAddrLoop;

  //OgCloseSocket(info->hsocket_service);
  DONE;
}

