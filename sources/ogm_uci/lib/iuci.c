/*
 *  Initialization for ogm_uci functions
 *  Copyright (c) 2006 Pertimm by Patrick Constant
 *  Dev : July 2006
 *  Version 1.0
*/
#include "ogm_uci.h"





void *OgUciInit(param)
struct og_uci_param *param;
{
struct og_http_param chttp_param, *http_param=&chttp_param;
struct og_msg_param msg_param[1];
struct og_ctrl_uci *ctrl_uci;
char erreur[DOgErrorSize];
int size;

IFn(ctrl_uci=(struct og_ctrl_uci *)malloc(sizeof(struct og_ctrl_uci))) {
  sprintf(erreur,"OgUciInit: malloc error on ctrl_uci");
  OgErr(param->herr,erreur); return(0);
  }
memset(ctrl_uci,0,sizeof(struct og_ctrl_uci));

ctrl_uci->herr = param->herr;
ctrl_uci->hmutex = param->hmutex;
ctrl_uci->cloginfo = param->loginfo;
ctrl_uci->loginfo = &ctrl_uci->cloginfo;

memset(msg_param,0,sizeof(struct og_msg_param));
msg_param->herr=ctrl_uci->herr;
msg_param->hmutex=ctrl_uci->hmutex;
msg_param->loginfo.trace = DOgMsgTraceMinimal+DOgMsgTraceMemory;
msg_param->loginfo.where = ctrl_uci->loginfo->where;
msg_param->module_name="uci";
IFn(ctrl_uci->hmsg=OgMsgInit(msg_param)) return(0);
IF(OgMsgTuneInherit(ctrl_uci->hmsg,param->hmsg)) return(0);

ctrl_uci->BaSize = DOgBaSize;
if (ctrl_uci->BaSize < param->socket_buffer_size) ctrl_uci->BaSize = param->socket_buffer_size;
size = ctrl_uci->BaSize*sizeof(unsigned char);
IFn(ctrl_uci->Ba=(unsigned char *)malloc(size)) {
  sprintf(erreur,"OgEmlInit: malloc error on Ba (%d bytes)",size);
  OgErr(ctrl_uci->herr,erreur); return(0);
  }
ctrl_uci->header_mandatory = param->header_mandatory;

memset(http_param,0,sizeof(struct og_http_param));
http_param->herr=ctrl_uci->herr;
http_param->hmutex=ctrl_uci->hmutex;
http_param->loginfo.trace = DOgHttpTraceMinimal+DOgHttpTraceMemory;
http_param->loginfo.where = ctrl_uci->loginfo->where;
IFn(ctrl_uci->hhttp=OgHttpInit(http_param)) return(0);

return((void *)ctrl_uci);
}





int OgUciFlush(handle)
void *handle;
{
struct og_ctrl_uci *ctrl_uci = (struct og_ctrl_uci *)handle;

IFE(OgHttpFlush(ctrl_uci->hhttp));
IFE(OgMsgFlush(ctrl_uci->hmsg));
DPcFree(ctrl_uci->Ba);

DPcFree(ctrl_uci);
DONE;
}




PUBLIC(char *) OgUciBanner()
{
return(DOgUciBanner);
}




