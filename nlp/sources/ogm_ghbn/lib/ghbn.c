/*
 *  Encapsulation of gethostbyname with the handling of a cache
 *  Copyright (c) 2004 Pertimm
 *  Dev : June 2004
 *  Version 1.0
*/
#include "ogm_ghbn.h"


STATICF(int) FindHostname(pr_(struct og_ctrl_ghbn *) pr(char *));
STATICF(int) AddHostname(pr_(struct og_ctrl_ghbn *) pr_(char *) pr_(int) pr(struct og_hostent *));
STATICF(int) GetHostname(pr_(struct og_ctrl_ghbn *) pr(struct og_hostname **));



/*  
 * We experienced long period of time (more than 300 seconds) with the gethostbyname 
 * function. So we create a cache of the resolution (a small memory DNS).
 * Moreover, the gethostbyname function is blocking, and we even experienced crashes.
 * Thus, the best way is to call this function in a thread, and kill the thread
 * after a certain period of time if we do not have any answers.
*/
PUBLIC(int) OgGetHostByName(handle,hostname,hostent,timeout)
void *handle; char *hostname; 
struct og_hostent *hostent;
int timeout;
{
struct og_ctrl_ghbn *ctrl_ghbn = (struct og_ctrl_ghbn *)handle;
int Ihostname,retour,clock_start;
struct og_hostname *og_hostname;

memset(hostent,0,sizeof(struct og_hostent));
IFn(handle) { hostent->found=0; DONE; }

IFx(Ihostname=FindHostname(ctrl_ghbn,hostname)) {
  if (ctrl_ghbn->loginfo->trace & DOgGhbnTraceCache) {
    OgMessageLog( DOgMlogInLog, ctrl_ghbn->loginfo->where, 0
      , "OgGetHostByName: found cached hostent for '%s'",hostname);
    }
  *hostent=ctrl_ghbn->Hostname[Ihostname].hostent;
  /** The 'found' value is also cached (could be zero when hostname was not found) **/
  if (hostent->found) hostent->found=2;
  DONE;
  }

IFE(Ihostname=GetHostname(ctrl_ghbn,&og_hostname));

clock_start=OgClock();
IFE(retour=OgGetHostByNameTimeout(ctrl_ghbn,hostname,hostent,timeout));
if (ctrl_ghbn->loginfo->trace & DOgGhbnTraceMinimal) {
  char v[128]; OgFormatThousand(OgClock()-clock_start,v);
  switch(retour) {
    case 0: 
      OgMessageLog( DOgMlogInLog, ctrl_ghbn->loginfo->where, 0
        , "OgGetHostByName: '%s' timed-out after %s milli-seconds",hostname,v);
      break;
    case 1: 
      OgMessageLog( DOgMlogInLog, ctrl_ghbn->loginfo->where, 0
        , "OgGetHostByName: '%s' not found after %s milli-seconds",hostname,v);
      break;
    case 2: 
      OgMessageLog( DOgMlogInLog, ctrl_ghbn->loginfo->where, 0
        , "OgGetHostByName: '%s' found after %s milli-seconds",hostname,v);
      break;
    }
  }
IFE(AddHostname(ctrl_ghbn,hostname,Ihostname,hostent));
DONE;
}




STATICF(int) FindHostname(ctrl_ghbn,hostname)
struct og_ctrl_ghbn *ctrl_ghbn;
char *hostname;
{
int iout,retour,nstate0,nstate1,id;
oindex states[DPcAutMaxBufferSize+9];
unsigned char out[DPcAutMaxBufferSize+9];
unsigned char buffer[DPcAutMaxBufferSize+9];

sprintf(buffer,"%s:",hostname);
if ((retour=OgAutScanf(ctrl_ghbn->ha,-1,buffer,&iout,out,&nstate0,&nstate1,states))) {
  do { 
    IFE(retour); 
    id=atoi(out);
    return(id);
    }
  while((retour=OgAutScann(ctrl_ghbn->ha,&iout,out,nstate0,&nstate1,states)));
  }
return(0);
}




/*
 *  Hostnames are stored in an automaton under form:
 *  hostname:number where number is the index to the 
 *  Hostname structure. 
*/
STATICF(int) AddHostname(ctrl_ghbn,hostname,Ihostname,hostent)
struct og_ctrl_ghbn *ctrl_ghbn;
char *hostname; int Ihostname;
struct og_hostent *hostent;
{
unsigned char buffer[DPcAutMaxBufferSize+9];

sprintf(buffer,"%s:%d",hostname,Ihostname);
IFE(OgAutAdd(ctrl_ghbn->ha,strlen(buffer),buffer));  

ctrl_ghbn->Hostname[Ihostname].hostent = *hostent;

DONE;
}





STATICF(int) GetHostname(ctrl_ghbn,phostname)
struct og_ctrl_ghbn *ctrl_ghbn;
struct og_hostname **phostname;
{
int i = ctrl_ghbn->HostnameNumber;
struct og_hostname *hostname = 0;

beginGetHostname:

if (ctrl_ghbn->HostnameUsed < ctrl_ghbn->HostnameNumber) {
  i = ctrl_ghbn->HostnameUsed++; 
  hostname = ctrl_ghbn->Hostname + i;
  }

if (i == ctrl_ghbn->HostnameNumber) {
  char erreur[DPcSzErr];
  unsigned a, b; struct og_hostname *og_hostname;
  if (ctrl_ghbn->loginfo->trace & DOgGhbnTraceMemory) {
    OgMessageLog( DOgMlogInLog, ctrl_ghbn->loginfo->where, 0
      , "GetHostname from OgGetHostByName: max hostname number (%d) reached"
      , ctrl_ghbn->HostnameNumber);
    }
  a = ctrl_ghbn->HostnameNumber; b = a + (a>>2) + 1;
  IFn(og_hostname=(struct og_hostname *)malloc(b*sizeof(struct og_hostname))) {
    sprintf(erreur,"GetHostname: malloc error on Hostname");
    PcErr(-1,erreur); DPcErr;
    }

  memset( og_hostname + a, 0, (b-a)*sizeof(struct og_hostname));
  memcpy( og_hostname, ctrl_ghbn->Hostname, a*sizeof(struct og_hostname));
  
  DPcFree(ctrl_ghbn->Hostname);       ctrl_ghbn->Hostname = og_hostname;
  ctrl_ghbn->HostnameNumber = b;

  if (ctrl_ghbn->loginfo->trace & DOgGhbnTraceMemory) {
    OgMessageLog( DOgMlogInLog, ctrl_ghbn->loginfo->where, 0
      , "GetHostname from OgGetHostByName: new hostname number is %d"
      , ctrl_ghbn->HostnameNumber);
    }

#ifdef DOgNoMainBufferReallocation
  sprintf(erreur, "GetHostname from OgGetHostByName: no more string workspace max is %d"
    , ctrl_ghbn->HostnameNumber);
  PcErr(-1,erreur); DPcErr;
#endif

  goto beginGetHostname;
  }

memset(hostname,0,sizeof(struct og_hostname));
if (phostname) *phostname = hostname;
return(i);  
}


