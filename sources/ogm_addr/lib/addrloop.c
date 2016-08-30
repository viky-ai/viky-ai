/*
 *  Function listening to all added addresses.
 *  Copyright (c) 2005-2006 Pertimm by Patrick Constant
 *  Dev : March 2005, December 2006
 *  Version 1.1
 */
#include "ogm_addr.h"

struct addr_context
{
  struct aso *aso;
  fd_set *fdset;
  struct og_socket_info *info;
};

static og_status AddrLoopInit(struct og_ctrl_addr *ctrl_addr, struct aso *aso, fd_set **fdset, int *maxfd);
static og_status AddrWaitConnection(struct og_ctrl_addr *ctrl_addr, struct aso *aso, fd_set *fdset, int maxfd);
static og_status AddrAcceptConnection(struct og_ctrl_addr *ctrl_addr, struct addr_context *addr_ctx, int i);

/*
 *  Since 'func' is handling the request coming from a given socket,
 *  its return value controls the stopping of the loop:
 *  if func returns 0, loop is continuing , else if func returns 1,
 *  the loop must break.
 */

PUBLIC(int) OgAddrLoop(void *handle, int (*answer_func)(void *, struct og_socket_info *info), void *answer_func_context)
{
  struct og_ctrl_addr *ctrl_addr = (struct og_ctrl_addr *) handle;
  ctrl_addr->answer_func = answer_func;
  ctrl_addr->answer_func_context = answer_func_context;

  int maxfd = 0;
  struct aso *aso = NULL;
  fd_set *fdset = NULL;
  IFE(AddrLoopInit(ctrl_addr, aso, &fdset, &maxfd));

  while (1)
  {
    IFE(AddrWaitConnection(ctrl_addr, aso, fdset, maxfd));

    IFE(ctrl_addr->must_stop = ctrl_addr->must_stop_func(ctrl_addr->func_context));

    for (int i = 0; i < ctrl_addr->AsoUsed; i++)
    {
      struct addr_context addr_ctx[1];
      addr_ctx->aso = aso;
      addr_ctx->fdset = fdset;

      struct og_socket_info info[1];
      memset(info, 0, sizeof(struct og_socket_info));
      addr_ctx->info = info;

      og_status status = AddrAcceptConnection(ctrl_addr, addr_ctx, i);
      IFE(status);
      if (status == CONTINUE) continue;

      // g_async_queue_length returns the number of data items in the queue minus the number of waiting threads,
      // so a negative value means waiting threads, and a positive value means available entries in the queue .
      // A return value of 0 could mean n entries in the queue and n threads waiting.
      // This can happen due to locking of the queue or due to scheduling
      int queue_length = g_async_queue_length(ctrl_addr->async_socket_queue);
      if ((ctrl_addr->backlog_max_pending_requests > 0) && (queue_length >= ctrl_addr->backlog_max_pending_requests))
      {
        time_t error_time[1];
        time(error_time);

        // do not push the request we just received, because we have too many requests
        OgMsg(ctrl_addr->hmsg, "", DOgMsgDestInLog, "OgAddrLoop %s: request dropped on socket %d because"
            " too many requests in the queue (%d >= %d) at %.24s UTC", ctrl_addr->addr_name, info->hsocket_service,
            queue_length, ctrl_addr->backlog_max_pending_requests, OgGmtime(error_time));
        IFE(ctrl_addr->send_error_status_func(ctrl_addr->func_context, info, 503, "Service Unavailable (queue full)"));
      }
      else
      {

        // copy socket info on sliced heap
        struct og_socket_info *info_to_store = g_slice_new(struct og_socket_info);
        memcpy(info_to_store, info, sizeof(struct og_socket_info));

        // push that socket in the queue to be processed later
        g_async_queue_push(ctrl_addr->async_socket_queue, info_to_store);

      }

    }

    if (ctrl_addr->must_stop)
    {
      OgMsg(ctrl_addr->hmsg, "", DOgMsgDestInLog, "OgAddrLoop %s: must stop service", ctrl_addr->addr_name);
      break;
    }

  }

  // ajouter un timeout sur le wait si besoins
  IFE(OgSemaphoreWait(ctrl_addr->hsem));

  DPcFree(fdset);
  OgMsg(ctrl_addr->hmsg, "", DOgMsgDestInLog, "OgAddrLoop %s: finished", ctrl_addr->addr_name);

  DONE;
}

static void AddrLoopAsyncSocketQueueDestroyNotify(gpointer p_og_socket_info)
{
  if (p_og_socket_info != NULL)
  {
    // free remainding stored info
    g_slice_free(struct og_socket_info, p_og_socket_info);
  }
}

static og_status AddrLoopInit(struct og_ctrl_addr *ctrl_addr, struct aso *aso, fd_set **fdset, int *maxfd)
{
  for (int i = 0; i < ctrl_addr->AsoUsed; i++)
  {
    aso = ctrl_addr->Aso + i;
    if (aso->hsocket > *maxfd)
    {
      *maxfd = aso->hsocket;
    }
  }
  int size = (*maxfd + 1) * sizeof(fd_set);
  IFn(*fdset=(fd_set *)malloc(size))
  {
    char erreur[DOgErrorSize];
    sprintf(erreur, "OgAddrLoop %s: malloc error on fdset (%d bytes)", ctrl_addr->addr_name, size);
    OgErr(ctrl_addr->herr, erreur);
    return (0);
  }

  ctrl_addr->async_socket_queue = g_async_queue_new_full(AddrLoopAsyncSocketQueueDestroyNotify);
  IFE(OgCreateThread(&ctrl_addr->thread, OgAddrSocketQueue, ctrl_addr));

  DONE;
}

static og_status AddrWaitConnection(struct og_ctrl_addr *ctrl_addr, struct aso *aso, fd_set *fdset, int maxfd)
{

  int size = (maxfd + 1) * sizeof(fd_set);
  memset(fdset, 0, size);
  for (int i = 0; i < ctrl_addr->AsoUsed; i++)
  {
    aso = ctrl_addr->Aso + i;
    FD_SET(aso->hsocket, fdset);
  }

  /** Wait in select until there is a connection, no timeout **/
  if (select(maxfd + 1, fdset, NULL, NULL, NULL) == DOgSocketError)
  {
    char sys_erreur[DOgErrorSize];
    int nerr = OgSysErrMes(DOgSocketErrno, DOgErrorSize, sys_erreur);
    if (nerr == DOgErrorEINTR)
    {
      OgMsg(ctrl_addr->hmsg, "", DOgMsgDestInLog + DOgMsgDestInErr, "OgAddrLoop %s: caught %d error: %s (going on)",
          ctrl_addr->addr_name, nerr, sys_erreur);
    }
    else
    {
      char erreur[DOgErrorSize];
      sprintf(erreur, "OgAddrLoop: select: (%d) %s\n", nerr, sys_erreur);
      OgErr(ctrl_addr->herr, erreur);

      //main_is_on_error
      DPcErr;
    }
  }

  DONE;
}

static og_status AddrAcceptConnection(struct og_ctrl_addr *ctrl_addr, struct addr_context *addr_ctx, int i)
{
  struct aso *aso = addr_ctx->aso;
  aso = ctrl_addr->Aso + i;
  if (!FD_ISSET(aso->hsocket, addr_ctx->fdset))
  {
    CONT;
  }

  struct og_socket_info *info = addr_ctx->info;
  info->address = ctrl_addr->Ba + aso->addr_start;
  info->port = aso->port;

  /** Connection with the client, because of select, we know there is a request **/
  IF(info->hsocket_service = SocketAcceptConnexion(ctrl_addr->herr, aso->hsocket, &info->socket_in))
  {
    char erreur[DOgErrorSize];
    OgErrLast(ctrl_addr->herr, erreur, 0);
    OgMsg(ctrl_addr->hmsg, "", DOgMsgDestInLog + DOgMsgDestInErr,
        "OgAddrLoop %s: failed to connect with the client '%s:%d' \n%s", ctrl_addr->Ba + aso->addr_start, aso->port,
        erreur, ctrl_addr->addr_name);
    CONT;
  }

  // set start time
  info->time_start = OgMilliClock();

  /** Here we get the IP address of the calling program through the socket **/
  IFE(OgGetRemoteAddrSocket(&info->socket_in.sin_addr, info->sremote_addr, 0));

  DONE;
}
