/*
 *  Initialization for ogm_uci functions
 *  Copyright (c) 2006 Pertimm by Patrick Constant
 *  Dev : July 2006
 *  Version 1.0
*/
#include "ogm_uci.h"





PUBLIC(void *) OgUciClientInit(param)
struct og_uci_client_param *param;
{
struct og_uci_param cuci_param, *uci_param=&cuci_param;
struct og_msg_param msg_param[1];
struct og_ctrl_ucic *ctrl_ucic;
char erreur[DOgErrorSize];
int size;

IFn(ctrl_ucic=(struct og_ctrl_ucic *)malloc(sizeof(struct og_ctrl_ucic))) {
  sprintf(erreur,"OgUciInit: malloc error on ctrl_ucic");
  OgErr(param->herr,erreur); return(0);
  }
memset(ctrl_ucic,0,sizeof(struct og_ctrl_ucic));

ctrl_ucic->herr = param->herr;
ctrl_ucic->hmutex = param->hmutex;
ctrl_ucic->cloginfo = param->loginfo;
ctrl_ucic->loginfo = &ctrl_ucic->cloginfo;

IF(OgStartupSockets(param->herr)) return(0);

memset(msg_param,0,sizeof(struct og_msg_param));
msg_param->herr=ctrl_ucic->herr;
msg_param->hmutex=ctrl_ucic->hmutex;
msg_param->loginfo.trace = DOgMsgTraceMinimal+DOgMsgTraceMemory;
msg_param->loginfo.where = ctrl_ucic->loginfo->where;
msg_param->module_name="ucic";
IFn(ctrl_ucic->hmsg=OgMsgInit(msg_param)) return(0);
IF(OgMsgTuneInherit(ctrl_ucic->hmsg,param->hmsg)) return(0);


ctrl_ucic->BaSize = DOgBaSize;
if (ctrl_ucic->BaSize < param->socket_buffer_size) ctrl_ucic->BaSize = param->socket_buffer_size;
size = ctrl_ucic->BaSize*sizeof(unsigned char);
IFn(ctrl_ucic->Ba=(unsigned char *)malloc(size)) {
  sprintf(erreur,"OgEmlInit: malloc error on Ba (%d bytes)",size);
  OgErr(ctrl_ucic->herr,erreur); return(0);
  }

memset(uci_param,0,sizeof(struct og_uci_param));
uci_param->herr=ctrl_ucic->herr;
uci_param->hmsg=ctrl_ucic->hmsg;
uci_param->hmutex=ctrl_ucic->hmutex;
uci_param->loginfo.trace=0;
if (ctrl_ucic->loginfo->trace & DOgUciClientTraceMinimal) uci_param->loginfo.trace|=DOgUciTraceMinimal;
if (ctrl_ucic->loginfo->trace & DOgUciClientTraceMemory) uci_param->loginfo.trace|=DOgUciTraceMemory;
if (ctrl_ucic->loginfo->trace & DOgUciClientTraceSocket) uci_param->loginfo.trace|=DOgUciTraceSocket;
if (ctrl_ucic->loginfo->trace & DOgUciClientTraceSocketSize) uci_param->loginfo.trace|=DOgUciTraceSocketSize;
uci_param->loginfo.where = ctrl_ucic->loginfo->where;
uci_param->socket_buffer_size=param->socket_buffer_size;
uci_param->header_mandatory=param->header_mandatory;
IFn(ctrl_ucic->huci=OgUciInit(uci_param)) return(0);

return((void *)ctrl_ucic);
}






PUBLIC(int) OgUciClientFlush(handle)
void *handle;
{
struct og_ctrl_ucic *ctrl_ucic = (struct og_ctrl_ucic *)handle;

IFE(OgUciFlush(ctrl_ucic->huci));
DPcFree(ctrl_ucic->Ba);

IFE(OgMsgFlush(ctrl_ucic->hmsg));

DPcFree(ctrl_ucic);
DONE;
}







