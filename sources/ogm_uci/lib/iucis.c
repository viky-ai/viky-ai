/*
 *  Initialization for ogm_uci functions
 *  Copyright (c) 2006-2007 Pertimm by Patrick Constant
 *  Dev : July 2006, June 2007
 *  Version 1.1
*/
#include "ogm_uci.h"





PUBLIC(void *) OgUciServerInit(param)
struct og_uci_server_param *param;
{
char erreur[DOgErrorSize];
struct og_ctrl_ucis *ctrl_ucis;
struct og_uci_param cuci_param, *uci_param=&cuci_param;
struct og_msg_param msg_param[1];
int size;

IFn(ctrl_ucis=(struct og_ctrl_ucis *)malloc(sizeof(struct og_ctrl_ucis))) {
  sprintf(erreur,"OgUciServerInit: malloc error on ctrl_ucis");
  OgErr(param->herr,erreur); return(0);
  }
memset(ctrl_ucis,0,sizeof(struct og_ctrl_ucis));

ctrl_ucis->herr = param->herr;
ctrl_ucis->hmutex = param->hmutex;
ctrl_ucis->cloginfo = param->loginfo;
ctrl_ucis->loginfo = &ctrl_ucis->cloginfo;

IF(OgStartupSockets(param->herr)) return(0);

memset(msg_param,0,sizeof(struct og_msg_param));
msg_param->herr=ctrl_ucis->herr;
msg_param->hmutex=ctrl_ucis->hmutex;
msg_param->loginfo.trace = DOgMsgTraceMinimal+DOgMsgTraceMemory;
msg_param->loginfo.where = ctrl_ucis->loginfo->where;
msg_param->module_name="ucis";
IFn(ctrl_ucis->hmsg=OgMsgInit(msg_param)) return(0);
IF(OgMsgTuneInherit(ctrl_ucis->hmsg,param->hmsg)) return(0);

ctrl_ucis->BaSize = DOgBaSize;
if (ctrl_ucis->BaSize < param->socket_buffer_size) ctrl_ucis->BaSize = param->socket_buffer_size;
size = ctrl_ucis->BaSize*sizeof(unsigned char);
IFn(ctrl_ucis->Ba=(unsigned char *)malloc(size)) {
  sprintf(erreur,"OgUciServerInit: malloc error on Ba (%d bytes)",size);
  OgErr(ctrl_ucis->herr,erreur); return(0);
  }

memset(uci_param,0,sizeof(struct og_uci_param));
uci_param->herr=ctrl_ucis->herr;
uci_param->hmsg=ctrl_ucis->hmsg;
uci_param->hmutex=ctrl_ucis->hmutex;
uci_param->loginfo.trace=0;
if (ctrl_ucis->loginfo->trace & DOgUciServerTraceMinimal) uci_param->loginfo.trace|=DOgUciTraceMinimal;
if (ctrl_ucis->loginfo->trace & DOgUciServerTraceMemory) uci_param->loginfo.trace|=DOgUciTraceMemory;
if (ctrl_ucis->loginfo->trace & DOgUciServerTraceSocket) uci_param->loginfo.trace|=DOgUciTraceSocket;
if (ctrl_ucis->loginfo->trace & DOgUciServerTraceSocketSize) uci_param->loginfo.trace|=DOgUciTraceSocketSize;
uci_param->loginfo.where = ctrl_ucis->loginfo->where;
uci_param->socket_buffer_size=param->socket_buffer_size;
uci_param->header_mandatory=param->header_mandatory;
IFn(ctrl_ucis->huci=OgUciInit(uci_param)) return(0);

return((void *)ctrl_ucis);
}






PUBLIC(int) OgUciServerFlush(handle)
void *handle;
{
struct og_ctrl_ucis *ctrl_ucis = (struct og_ctrl_ucis *)handle;

IFE(OgUciFlush(ctrl_ucis->huci));
IFE(OgMsgFlush(ctrl_ucis->hmsg));
DPcFree(ctrl_ucis->Ba);
DPcFree(ctrl_ucis);
DONE;
}







