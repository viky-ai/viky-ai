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
  int (*func)(void *, struct og_socket_info *info);
  void *ptr;
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

PUBLIC(int) OgAddrLoop(void *handle, int timeout, int queue_max_length,
    int (*func)(void *, struct og_socket_info *info), void *ptr)
{
  struct og_ctrl_addr *ctrl_addr = (struct og_ctrl_addr *) handle;

  int maxfd = 0;
  struct aso *aso = NULL;
  fd_set *fdset = NULL;
  IFE(AddrLoopInit(ctrl_addr, aso, &fdset, &maxfd));

  while (1)
  {
    IFE(AddrWaitConnection(ctrl_addr, aso, fdset, maxfd));

    for (int i = 0; i < ctrl_addr->AsoUsed; i++)
    {
      struct addr_context addr_ctx[1];
      addr_ctx->aso = aso;
      addr_ctx->fdset = fdset;
      addr_ctx->func = func;
      addr_ctx->ptr = ptr;

      struct og_socket_info info[1];
      memset(info, 0, sizeof(struct og_socket_info));
      addr_ctx->info = info;

      og_status status = AddrAcceptConnection(ctrl_addr, addr_ctx, i);
      IFE(status);
      if (status == CONTINUE) continue;

      // TODO size queue
      if ((queue_max_length > 0) && (g_async_queue_length(ctrl_addr->async_socket_queue) >= (queue_max_length - 1)))
      {
        IFE(AddrSendStatusCodeServiceUnavailable(ctrl_addr, info->hsocket_service));

        OgMessageLog(DOgMlogInLog, ctrl_addr->loginfo->where, 0, "socket %d closed because of size",
            info->hsocket_service);

        OgCloseSocket(info->hsocket_service);
      }
      else
      {
        info->time_start = OgMilliClock();
        info->timeout = timeout;

        struct og_socket_info *info_to_store = OgHeapNewCell(ctrl_addr->sockets, NULL);
        memcpy(info_to_store, info, sizeof(struct og_socket_info));
        g_async_queue_push(ctrl_addr->async_socket_queue, info_to_store);
      }

    }
  }

  g_async_queue_unref(ctrl_addr->async_socket_queue);
  DPcFree(fdset);

  DONE;
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
    sprintf(erreur, "OgAddrLoop: malloc error on fdset (%d bytes)", size);
    OgErr(ctrl_addr->herr, erreur);
    return (0);
  }

  IFE(OgCreateThread(&ctrl_addr->thread, OgAddrSocketQueue, ctrl_addr));
  ctrl_addr->async_socket_queue = g_async_queue_new();

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
      OgMsg(ctrl_addr->hmsg, "", DOgMlogInLog + DOgMlogInErr, "OgAddrLoop: caught %d error: %s (going on)", nerr,
          sys_erreur);
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
  info->func = addr_ctx->func;
  info->ptr = addr_ctx->ptr;

  /** Connection with the client, because of select, we know there is a request **/
  IF(info->hsocket_service = SocketAcceptConnexion(ctrl_addr->herr, aso->hsocket, &info->socket_in))
  {
    char erreur[DOgErrorSize];
    OgErrLast(ctrl_addr->herr, erreur, 0);
    OgMsg(ctrl_addr->hmsg, "", DOgMlogInLog + DOgMlogInErr,
        "OgAddrLoop: failed to connect with the client '%s:%d' \n%s", ctrl_addr->Ba + aso->addr_start, aso->port,
        erreur);
    CONT;
  }

  OgMessageLog(DOgMlogInLog, ctrl_addr->loginfo->where, 0, "socket %d accepted", info->hsocket_service);

  /** Here we get the IP address of the calling program through the socket **/
  IFE(OgGetRemoteAddrSocket(&info->socket_in.sin_addr, info->sremote_addr, 0));

  DONE;
}
