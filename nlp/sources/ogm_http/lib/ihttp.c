/*
 *  Initialization for ogm_http functions
 *  Copyright (c) 2006 Pertimm by Patrick Constant
 *  Dev : August, December 2006
 *  Version 1.1
*/
#include "ogm_http.h"


#define DOgHttpHaHeaderSize 1024 /* 1k */





PUBLIC(void *) OgHttpInit(param)
struct og_http_param *param;
{
struct og_aut_param caut_param, *aut_param=&caut_param;
struct og_msg_param msg_param[1];
struct og_ctrl_http *ctrl_http;
char erreur[DOgErrorSize];

IFn(ctrl_http=(struct og_ctrl_http *)malloc(sizeof(struct og_ctrl_http))) {
  sprintf(erreur,"OgHttpInit: malloc error on ctrl_http");
  OgErr(param->herr,erreur); return(0);
  }
memset(ctrl_http,0,sizeof(struct og_ctrl_http));

ctrl_http->herr = param->herr;
ctrl_http->hmutex = param->hmutex;
ctrl_http->cloginfo = param->loginfo;
ctrl_http->loginfo = &ctrl_http->cloginfo;

memset(msg_param,0,sizeof(struct og_msg_param));
msg_param->herr=ctrl_http->herr;
msg_param->hmutex=ctrl_http->hmutex;
msg_param->loginfo.trace = DOgMsgTraceMinimal+DOgMsgTraceMemory;
msg_param->loginfo.where = ctrl_http->loginfo->where;
msg_param->module_name="uci";
IFn(ctrl_http->hmsg=OgMsgInit(msg_param)) return(0);
IF(OgMsgTuneInherit(ctrl_http->hmsg,param->hmsg)) return(0);

memset(aut_param,0,sizeof(struct og_aut_param));
aut_param->herr=ctrl_http->herr;
aut_param->hmutex=ctrl_http->hmutex;
aut_param->loginfo.trace = DOgAutTraceMinimal+DOgAutTraceMemory;
aut_param->loginfo.where = ctrl_http->loginfo->where;
aut_param->state_number = DOgHttpHaHeaderSize;
sprintf(aut_param->name,"header");
IFn(ctrl_http->ha_header=OgAutInit(aut_param)) return(0);

return((void *)ctrl_http);
}





PUBLIC(int) OgHttpFlush(handle)
void *handle;
{
struct og_ctrl_http *ctrl_http = (struct og_ctrl_http *)handle;
IFE(OgAutFlush(ctrl_http->ha_header));
IFE(OgMsgFlush(ctrl_http->hmsg));
DPcFree(ctrl_http);
DONE;
}



