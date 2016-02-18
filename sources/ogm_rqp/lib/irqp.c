/*
 *	Initialization for ogm_rqp functions
 *	Copyright (c) 2006 Pertimm by Patrick Constant
 *	Dev : May 2006
 *	Version 1.0
*/
#include "ogm_rqp.h"






PUBLIC(void *) OgRqpInit(struct og_rqp_param *param)
{
struct og_msg_param cmsg_param, *msg_param=&cmsg_param;
struct og_ctrl_rqp *ctrl_rqp;
char erreur[DOgErrorSize];
struct og_aut_param caut_param,*aut_param=&caut_param;
int size;

IFn(ctrl_rqp=(struct og_ctrl_rqp *)malloc(sizeof(struct og_ctrl_rqp))) {
  sprintf(erreur,"OgRqpInit: malloc error on ctrl_rqp");
  OgErr(param->herr,erreur); return(0);
  }
memset(ctrl_rqp,0,sizeof(struct og_ctrl_rqp));

ctrl_rqp->herr = param->herr;
ctrl_rqp->hmutex = param->hmutex;
ctrl_rqp->cloginfo = param->loginfo;
ctrl_rqp->loginfo = &ctrl_rqp->cloginfo;

memset(msg_param,0,sizeof(struct og_msg_param));
msg_param->herr=ctrl_rqp->herr; 
msg_param->hmutex=ctrl_rqp->hmutex;
msg_param->loginfo.trace = DOgMsgTraceMinimal+DOgMsgTraceMemory; 
msg_param->loginfo.where = ctrl_rqp->loginfo->where;
msg_param->module_name="rqp";
IFn(ctrl_rqp->hmsg=OgMsgInit(msg_param)) return(0);
IF(OgMsgTuneInherit(ctrl_rqp->hmsg,param->hmsg)) return(0);

ctrl_rqp->NodeNumber = DOgNodeNumber;
size = ctrl_rqp->NodeNumber*sizeof(struct node);
IFn(ctrl_rqp->Node=(struct node *)malloc(size)) {
  sprintf(erreur,"OgRqpInit: malloc error on Node (%d bytes)",size);
  OgErr(ctrl_rqp->herr,erreur); return(0);
  }

ctrl_rqp->BaSize = DOgBaSize;
size = ctrl_rqp->BaSize*sizeof(unsigned char);
IFn(ctrl_rqp->Ba=(unsigned char *)malloc(size)) {
  sprintf(erreur,"OgRqpInit: malloc error on Ba (%d bytes)",size);
  OgErr(ctrl_rqp->herr,erreur); return(0);
  } 

ctrl_rqp->BxSize = DOgBxSize;
size = ctrl_rqp->BxSize*sizeof(unsigned char);
IFn(ctrl_rqp->Bx=(unsigned char *)malloc(size)) {
  sprintf(erreur,"OgRqpInit: malloc error on Bx (%d bytes)",size);
  OgErr(ctrl_rqp->herr,erreur); return(0);
  } 

memset(aut_param,0,sizeof(struct og_aut_param));
aut_param->herr=ctrl_rqp->herr; 
aut_param->hmutex=ctrl_rqp->hmutex;
aut_param->loginfo.trace = DOgAutTraceMinimal+DOgAutTraceMemory; 
aut_param->loginfo.where = ctrl_rqp->loginfo->where;
aut_param->state_number = 0x1000;
sprintf(aut_param->name,"subtree id");
IFn(ctrl_rqp->ha_subtree_id=OgAutInit(aut_param)) return(0);

IF(OgRqpSubtreeSetFunctionName(ctrl_rqp,"subtree")) return(0);

IF(OgRqpReset(ctrl_rqp)) return(0);

return((void *)ctrl_rqp);
}






PUBLIC(int) OgRqpReset(void *handle)
{
struct og_ctrl_rqp *ctrl_rqp = (struct og_ctrl_rqp *)handle;
IFE(OgAutReset(ctrl_rqp->ha_subtree_id));
ctrl_rqp->Iroot_node=(-1);
ctrl_rqp->NodeUsed=0;
ctrl_rqp->BaUsed=0;
DONE;
}






PUBLIC(int) OgRqpFlush(void *handle)
{
struct og_ctrl_rqp *ctrl_rqp = (struct og_ctrl_rqp *)handle;

IFE(OgAutFlush(ctrl_rqp->ha_subtree_id));
IFE(OgMsgFlush(ctrl_rqp->hmsg));
DPcFree(ctrl_rqp->Node);
DPcFree(ctrl_rqp->Ba);
DPcFree(ctrl_rqp->Bx);
DPcFree(ctrl_rqp);

DONE;
}






PUBLIC(char *) OgRqpBanner(void)
{
return(DOgRqpBanner);
}
 



