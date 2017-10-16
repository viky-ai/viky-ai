/*
 *  Timeout call of gethostbyname because it is blocking
 *  Copyright (c) 2004 Pertimm
 *  Dev : June 2004
 *  Version 1.0
*/
#include "ogm_ghbn.h"
#include <logxpt.h>


STATICF(int) OgGhbnTimeout(pr(void *));
STATICF(int) OgGhbnTimeout1(pr(void *));
STATICF(int) OgGhbnTimeoutException(pr_(void *) pr(int));



/*
 *  Returns 0 if timed-out, 1 if not found and 2 if found.
*/

int OgGetHostByNameTimeout(ctrl_ghbn,hostname,hostent,timeout)
struct og_ctrl_ghbn *ctrl_ghbn; char *hostname;
struct og_hostent *hostent; int timeout;
{
struct og_ghbn_timeout cgt,*gt=&cgt;

if (ctrl_ghbn->loginfo->trace & DOgGhbnTraceTimeout) {
  OgMessageLog(DOgMlogInLog,ctrl_ghbn->loginfo->where,0, "OgGetHostByNameTimeout: starting");
  }

memset(hostent,0,sizeof(struct og_hostent));
memset(gt,0,sizeof(struct og_ghbn_timeout));
gt->ctrl_ghbn = ctrl_ghbn;
gt->hostname = hostname;

IFE(gt->timed_out= OgRunWithTimeout((double) timeout, OgGhbnTimeout, gt, ctrl_ghbn->loginfo->where));

if (gt->timed_out) { hostent->found=0; return(0); }
if (!gt->found) { hostent->found=0; return(1); }

*hostent=gt->chostent;
hostent->found=1;
if (ctrl_ghbn->loginfo->trace & DOgGhbnTraceTimeout) {
  OgMessageLog(DOgMlogInLog,ctrl_ghbn->loginfo->where,0, "OgGetHostByNameTimeout: finished");
  }
return(2);
}





STATICF(int) OgGhbnTimeout(ptr)
void *ptr;
{
struct og_ghbn_timeout *gt = (struct og_ghbn_timeout *)ptr;
IFE(OgExceptionSafe(OgGhbnTimeout1,OgGhbnTimeoutException,ptr));
DONE;
}









STATICF(int) OgGhbnTimeoutException(ptr,exception_number)
void *ptr; int exception_number;
{
struct og_ghbn_timeout *gt = (struct og_ghbn_timeout *)ptr;
struct og_ctrl_ghbn *ctrl_ghbn = gt->ctrl_ghbn;
if (ctrl_ghbn->loginfo->trace & DOgGhbnTraceMinimal) {
  OgMessageLog(DOgMlogInLog,ctrl_ghbn->loginfo->where,0
    ,"OgGetHostByNameTimeout: gethostbyname on hostname '%s', exception %x:", gt->hostname, exception_number);
  OgMessageLog(DOgMlogInLog,ctrl_ghbn->loginfo->where,0
    ,"OgGetHostByNameTimeout: exception was '%s'", OgExceptionToStr(exception_number));
  }
gt->found=0;
DONE;
}







#if (DPcSystem == DPcSystemUnix) 
/*
 *  Uses newer getaddrinfo function
*/

STATICF(int) OgGhbnTimeout1(ptr)
void *ptr;
{
struct og_ghbn_timeout *gt = (struct og_ghbn_timeout *)ptr;
struct og_ctrl_ghbn *ctrl_ghbn = gt->ctrl_ghbn;
struct og_hostent *hostent = &gt->chostent;
struct addrinfo *res = NULL;
struct addrinfo hints;
char sys_erreur[512];

if (ctrl_ghbn->loginfo->trace & DOgGhbnTraceTimeout) {
  OgMessageLog(DOgMlogInLog,ctrl_ghbn->loginfo->where,0 ,"OgGhbnTimeout1: starting on '%s'", gt->hostname);
  }

memset(&hints, 0, sizeof(hints));
/* ai_family  will  be  set  to  either  PF_INET or  PF_INET6 
 * PF_UNSPEC in ai_family specifies any protocol family 
 * (either IPv4 or IPv6, for example) */
hints.ai_family = AF_INET; 
hints.ai_socktype = SOCK_STREAM;
hints.ai_protocol = IPPROTO_TCP;

gt->found=0;
IFx(getaddrinfo(gt->hostname, NULL, &hints, &res)) {
  if (ctrl_ghbn->loginfo->trace & DOgGhbnTraceMinimal) {
    int nerr = OgSysErrMes(DOgSocketErrno,512,sys_erreur);
    OgMessageLog(DOgMlogInLog,ctrl_ghbn->loginfo->where,0
      ,"OgGetHostByNameTimeout: getaddrinfo on hostname '%s', error %d:", gt->hostname, nerr);
    OgMessageLog(DOgMlogInLog,ctrl_ghbn->loginfo->where,0
      ,"OgGetHostByNameTimeout: %.180s", ((nerr==11001)?"unknown host":sys_erreur));
    }
  }
if (res) {
  if (res->ai_addr && res->ai_addrlen) {
    struct sockaddr_in *ogsin = (struct sockaddr_in *)res->ai_addr; 
    if (ctrl_ghbn->loginfo->trace & DOgGhbnTraceTimeout) {
      OgMessageLog(DOgMlogInLog,ctrl_ghbn->loginfo->where,0 ,"OgGhbnTimeout1: found");
      }
    memcpy(&hostent->sin_addr,&ogsin->sin_addr, sizeof(struct in_addr));
    hostent->sin_family= res->ai_family;
    hostent->h_length=sizeof(struct in_addr);
    gt->found=1;
    }
  freeaddrinfo(res);
  } 

if (ctrl_ghbn->loginfo->trace & DOgGhbnTraceTimeout) {
  OgMessageLog(DOgMlogInLog,ctrl_ghbn->loginfo->where,0 ,"OgGhbnTimeout1: finished");
  }
DONE;
}


#else

/*
 *  Uses gethostbyname function
*/

STATICF(int) OgGhbnTimeout1(ptr)
void *ptr;
{
struct og_ghbn_timeout *gt = (struct og_ghbn_timeout *)ptr;
struct og_ctrl_ghbn *ctrl_ghbn = gt->ctrl_ghbn;
struct og_hostent *hostent = &gt->chostent;
char sys_erreur[512];
struct hostent *hp;

if (ctrl_ghbn->loginfo->trace & DOgGhbnTraceTimeout) {
  OgMessageLog(DOgMlogInLog,ctrl_ghbn->loginfo->where,0 ,"OgGhbnTimeout1: starting on '%s'", gt->hostname);
  }

IFn(hp = gethostbyname(gt->hostname)) {
  if (ctrl_ghbn->loginfo->trace & DOgGhbnTraceMinimal) {
    int nerr = OgSysErrMes(DOgSocketErrno,512,sys_erreur);
    OgMessageLog(DOgMlogInLog,ctrl_ghbn->loginfo->where,0
      ,"OgGetHostByNameTimeout: gethostbyname on hostname '%s', error %d:", gt->hostname, nerr);
    OgMessageLog(DOgMlogInLog,ctrl_ghbn->loginfo->where,0
      ,"OgGetHostByNameTimeout: %.180s", ((nerr==11001)?"unknown host":sys_erreur));
    }
  gt->found=0;
  }
else {
  if (ctrl_ghbn->loginfo->trace & DOgGhbnTraceTimeout) {
    OgMessageLog(DOgMlogInLog,ctrl_ghbn->loginfo->where,0 ,"OgGhbnTimeout1: found");
    }
  memcpy((char *)&hostent->sin_addr,(char *)hp->h_addr,hp->h_length);
  hostent->sin_family= hp->h_addrtype;
  hostent->h_length=hp->h_length;
  gt->found=1;
  }
if (ctrl_ghbn->loginfo->trace & DOgGhbnTraceTimeout) {
  OgMessageLog(DOgMlogInLog,ctrl_ghbn->loginfo->where,0 ,"OgGhbnTimeout1: finished");
  }
DONE;
}

#endif



