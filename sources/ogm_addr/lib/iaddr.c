/*
 *  Initialization for ogm_addr functions
 *  Copyright (c) 2005 Pertimm by Patrick Constant
 *  Dev : March 2005
 *  Version 1.0
*/
#include "ogm_addr.h"





PUBLIC(void *) OgAddrInit(param)
struct og_addr_param *param;
{
struct og_msg_param msg_param[1];
struct og_ctrl_addr *ctrl_addr;
char erreur[DOgErrorSize];
int size,ghbn_trace=0;

IFn(ctrl_addr=(struct og_ctrl_addr *)malloc(sizeof(struct og_ctrl_addr))) {
  sprintf(erreur,"OgAddrInit: malloc error on ctrl_addr");
  OgErr(param->herr,erreur); return(0);
  }
memset(ctrl_addr,0,sizeof(struct og_ctrl_addr));
ctrl_addr->herr = param->herr;
ctrl_addr->hmutex = param->hmutex;
ctrl_addr->loginfo[0] = param->loginfo;
snprintf(ctrl_addr->addr_name, DPcPathSize, "%s", param->addr_name);
ctrl_addr->backlog_max_pending_requests = param->backlog_max_pending_requests;
ctrl_addr->backlog_timeout = param->backlog_timeout;
ctrl_addr->down_timeout = param->down_timeout;
ctrl_addr->must_stop_func = param->must_stop_func;
ctrl_addr->search_unavailable_func = param->search_unavailable_func;
ctrl_addr->search_unavailable_ongoing_func = param->search_unavailable_ongoing_func;
ctrl_addr->search_unavailable_timed_out_func = param->search_unavailable_timed_out_func;
ctrl_addr->send_error_status_func = param->send_error_status_func;
ctrl_addr->func_context = param->func_context;

memset(msg_param,0,sizeof(struct og_msg_param));
msg_param->herr=ctrl_addr->herr;
msg_param->hmutex=ctrl_addr->hmutex;
msg_param->loginfo.trace = DOgMsgTraceMinimal+DOgMsgTraceMemory;
msg_param->loginfo.where = ctrl_addr->loginfo->where;
msg_param->module_name="addr";
IFn(ctrl_addr->hmsg=OgMsgInit(msg_param)) return(0);
IF(OgMsgTuneInherit(ctrl_addr->hmsg,param->hmsg)) return(0);

OgMsg(ctrl_addr->hmsg,"",DOgMsgDestInLog, "OgAddrInit: backlog_max_pending_requests=%d", ctrl_addr->backlog_max_pending_requests);
OgMsg(ctrl_addr->hmsg,"",DOgMsgDestInLog, "OgAddrInit: backlog_timeout=%d", ctrl_addr->backlog_timeout);

ctrl_addr->BaSize = DOgBaSize;
size = ctrl_addr->BaSize*sizeof(unsigned char);
IFn(ctrl_addr->Ba=(unsigned char *)malloc(size)) {
  sprintf(erreur,"OgAddrInit: malloc error on Ba (%d bytes)",size);
  OgErr(ctrl_addr->herr,erreur); return(0);
  }

ctrl_addr->FreeAso = DOgNMax;
ctrl_addr->AsoNumber = DOgAsoNumber;
IFn(ctrl_addr->Aso=(struct aso *)malloc(ctrl_addr->AsoNumber*sizeof(struct aso))) {
  sprintf(erreur,"OgAddrInit: malloc error on Aso");
  OgErr(ctrl_addr->herr,erreur); return(0);
  }

if (ctrl_addr->loginfo->trace & DOgAddrTraceGhbn) ghbn_trace=DOgGhbnTraceMinimal;
IFn(ctrl_addr->ghbn=OgGetHostByNameInit(ghbn_trace,ctrl_addr->loginfo->where)) return(0);

IF(OgSemaphoreInit(ctrl_addr->hsem,0)) return (0);

ctrl_addr->closed = FALSE;

return((void *)ctrl_addr);
}


PUBLIC(int) OgAddrClose(void *handle)
{
  struct og_ctrl_addr *ctrl_addr = (struct og_ctrl_addr *) handle;

  if (!ctrl_addr->closed)
  {

    for (int i = 0; i < ctrl_addr->AsoUsed; i++)
    {
      struct aso *aso = ctrl_addr->Aso + i;
      if (aso->hsocket == DOgSocketError) continue;
      OgCloseSocket(aso->hsocket);
    }

    ctrl_addr->closed = TRUE;
  }

  DONE;
}


PUBLIC(int) OgAddrFlush(void *handle)
{
  struct og_ctrl_addr *ctrl_addr = (struct og_ctrl_addr *) handle;

  IFE(OgAddrClose(ctrl_addr));

  if (ctrl_addr->async_socket_queue != NULL)
  {
    g_async_queue_unref(ctrl_addr->async_socket_queue);
    ctrl_addr->async_socket_queue = NULL;
  }

  IFE(OgGetHostByNameFlush(ctrl_addr->ghbn));

  IFE(OgMsgFlush(ctrl_addr->hmsg));
  DPcFree(ctrl_addr->Aso);
  DPcFree(ctrl_addr->Ba);
  DPcFree(ctrl_addr);
  DONE;
}



