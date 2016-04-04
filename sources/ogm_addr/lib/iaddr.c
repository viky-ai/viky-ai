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
ctrl_addr->cloginfo = param->loginfo;
ctrl_addr->loginfo = &ctrl_addr->cloginfo;

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


memset(msg_param,0,sizeof(struct og_msg_param));
msg_param->herr=ctrl_addr->herr;
msg_param->hmutex=ctrl_addr->hmutex;
msg_param->loginfo.trace = DOgMsgTraceMinimal+DOgMsgTraceMemory;
msg_param->loginfo.where = ctrl_addr->loginfo->where;
msg_param->module_name="addr";
IFn(ctrl_addr->hmsg=OgMsgInit(msg_param)) return(0);
IF(OgMsgTuneInherit(ctrl_addr->hmsg,param->hmsg)) return(0);

if (ctrl_addr->loginfo->trace & DOgAddrTraceGhbn) ghbn_trace=DOgGhbnTraceMinimal;
IFn(ctrl_addr->ghbn=OgGetHostByNameInit(ghbn_trace,ctrl_addr->loginfo->where)) return(0);

IFn(ctrl_addr->sockets = OgHeapSliceInit(ctrl_addr->hmsg, "addr_sockets", sizeof(struct og_socket_info), 256, 64)) return NULL;
IFn(ctrl_addr->error_messages = OgHeapSliceInit(ctrl_addr->hmsg, "addr_error_messages", sizeof(unsigned char), 256, 64)) return NULL;


return((void *)ctrl_addr);
}






PUBLIC(int) OgAddrFlush(handle)
void *handle;
{
struct og_ctrl_addr *ctrl_addr = (struct og_ctrl_addr *)handle;
struct aso *aso;
int i;

for (i=0; i<ctrl_addr->AsoUsed; i++) {
  aso = ctrl_addr->Aso + i;
  if (aso->hsocket == DOgSocketError) continue;
  OgCloseSocket(aso->hsocket);
  }

OgHeapFlush(ctrl_addr->sockets);
OgHeapFlush(ctrl_addr->error_messages);

IFE(OgGetHostByNameFlush(ctrl_addr->ghbn));

IFE(OgMsgFlush(ctrl_addr->hmsg));
DPcFree(ctrl_addr->Aso);
DPcFree(ctrl_addr->Ba);
DPcFree(ctrl_addr);
DONE;
}



