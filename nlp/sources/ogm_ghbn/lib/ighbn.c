/*
 *  Encapsulation of gethostbyname with the handling of a cache
 *  Copyright (c) 2004-2006 Pertimm
 *  Dev : June 2004, March 2006
 *  Version 1.0
*/
#include "ogm_ghbn.h"


#define DOgAutGhbnSize  0x1000 /* 4kb */



PUBLIC(void *) OgGetHostByNameInit(trace,where)
int trace; char *where;
{
char erreur[DPcSzErr];
struct og_ctrl_ghbn *ctrl_ghbn;
struct og_aut_param caut_param, *aut_param=&caut_param;

IFn(ctrl_ghbn=(struct og_ctrl_ghbn *)malloc(sizeof(struct og_ctrl_ghbn))) {
  sprintf(erreur,"OgReqInit: malloc error on ctrl_ghbn");
  PcErr(-1,erreur); return(0);
  }
memset(ctrl_ghbn,0,sizeof(struct og_ctrl_ghbn));

ctrl_ghbn->loginfo = &ctrl_ghbn->cloginfo;
ctrl_ghbn->loginfo->trace = trace; ctrl_ghbn->loginfo->where=where;

ctrl_ghbn->HostnameNumber = DOgHostnameNumber;
IFn(ctrl_ghbn->Hostname=(struct og_hostname *)malloc(ctrl_ghbn->HostnameNumber*sizeof(struct og_hostname))) {
  sprintf(erreur,"OgGetHostByNameInit: malloc error on Hostname");
  PcErr(-1,erreur); return(0);
  }
/** So that FindHostname returns 0 when not found **/
ctrl_ghbn->HostnameUsed=1;

memset(aut_param,0,sizeof(struct og_aut_param));
aut_param->herr=ctrl_ghbn->herr; 
aut_param->hmutex=ctrl_ghbn->hmutex;
aut_param->loginfo.trace = DOgAutTraceMinimal+DOgAutTraceMemory; 
aut_param->loginfo.where = ctrl_ghbn->loginfo->where;
aut_param->state_number = DOgAutGhbnSize;
sprintf(aut_param->name,"ghbn");
IFn(ctrl_ghbn->ha=OgAutInit(aut_param)) return(0);

return((void *)ctrl_ghbn);
}





PUBLIC(int) OgGetHostByNameFlush(handle)
void *handle;
{
struct og_ctrl_ghbn *ctrl_ghbn = (struct og_ctrl_ghbn *)handle;
IFn(handle) DONE;

IFE(OgAutFlush(ctrl_ghbn->ha));
DPcFree(ctrl_ghbn->Hostname);

DPcFree(ctrl_ghbn);
DONE;
}



