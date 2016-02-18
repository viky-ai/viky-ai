/*
 *	Initialization for msg library
 *	Copyright (c) 2007 Pertimm by Patrick Constant and Guillaume Logerot
 *	Dev : March,April 2007
 *	Version 1.1
*/
#include "ogm_msg.h"
#include <logxml.h>

PUBLIC(void *) OgMsgInit(struct og_msg_param *param)
{
struct og_ctrl_msg *ctrl_msg;
char erreur[DOgErrorSize];
char *DOgINSTANCE=NULL;

assert(param->herr!=NULL && param->hmutex!=NULL);
// assert sur le where de loginfo ?

IFn(ctrl_msg=(struct og_ctrl_msg *)malloc(sizeof(struct og_ctrl_msg))) {
  sprintf(erreur,"OgMsgInit: malloc error on ctrl_msg");
  OgErr(param->herr,erreur); return(0);
  }
memset(ctrl_msg,0,sizeof(struct og_ctrl_msg));

ctrl_msg->herr = param->herr;
ctrl_msg->hmutex = param->hmutex;
ctrl_msg->cloginfo = param->loginfo;
ctrl_msg->loginfo = &ctrl_msg->cloginfo;

if(param->prog_loginfo!=NULL) memcpy(ctrl_msg->prog_loginfo,param->prog_loginfo,sizeof(struct og_loginfo));
ctrl_msg->pipe = param->pipe;
ctrl_msg->consoleStream=stdout;
if (param->module_name) {
  strcpy(ctrl_msg->module_name,param->module_name);
 }
else {
  strcpy(ctrl_msg->module_name,"??????????");
 }

if(!ctrl_msg->prog_name[0]) strcpy(ctrl_msg->prog_name,ctrl_msg->module_name);

if ((DOgINSTANCE=getenv("DOgINSTANCE"))!=NULL) {
  strcpy(ctrl_msg->instance_name,DOgINSTANCE);
 }
 else {
   ctrl_msg->instance_name[0]='\0';
 }

return((void *)ctrl_msg);
}




PUBLIC(int) OgMsgFlush(void *handle)
{
struct og_ctrl_msg *ctrl_msg = (struct og_ctrl_msg *)handle;

DPcFree(ctrl_msg);
DONE;
}


