/*
 *  Adding an address in the list of addresses
 *  Copyright (c) 2005 Pertimm by Patrick Constant
 *  Dev : March 2005
 *  Version 1.0
*/
#include "ogm_addr.h"




/*
 *  Adding an address with a port.
 *  address format can be:
 *    www.pertimm.net
 *    localhost
 *    127.0.0.1
 *    0.0.0.0
 *    <address>:<port>
 *  In the last case <port> is chosen instead of the 'port' parameter.
*/

PUBLIC(int) OgAddrAdd(void *handle, char *hostname, int port)
{
struct og_ctrl_addr *ctrl_addr = (struct og_ctrl_addr *)handle;
int ihn; char hn[DPcPathSize];
int i,dp=(-1), hport=port;
struct og_hostent hostent;
struct aso *aso;
int timeout=30;

strcpy(hn,hostname);
ihn=strlen(hn);

for (i=0; i<ihn; i++) {
  if (hn[i]==':') { dp=i; break; }
  }

if (dp==0) {
  strcpy(hn,"0.0.0.0");
  ihn=strlen(hn);
  }
else if (dp>0) {
  hn[dp]=0; hport=atoi(hn+dp+1);
  ihn=strlen(hn);
  }

IF(OgGetHostByName(ctrl_addr->ghbn,hn,&hostent,timeout)) {
  OgMsg(ctrl_addr->hmsg,"",DOgMlogInLog
    ,"OgAddrAdd: OgGetHostByName error on url '%.180s':", ihn, hn);
  OgMsgErr(ctrl_addr->hmsg,"OgAddrAdd",0,0,0,DOgMsgSeverityError
    ,DOgErrLogFlagNoSystemError+DOgErrLogFlagNotInErr);

  DONE;
  }

if (ctrl_addr->loginfo->trace & DOgAddrTraceAdding) {
  unsigned long addr;
  memcpy((char*)&addr,(char*)&hostent.sin_addr,sizeof(long));
  OgMsg(ctrl_addr->hmsg,"",DOgMlogInLog
    ,"OgAddrAdd: adding listening socket on '%s' at port %d, IP is %ld.%ld.%ld.%ld"
    , hn, hport, (addr&0xff), ((addr>>8)&0xff), ((addr>>16)&0xff), ((addr>>24)&0xff));
  }

IFE(GetAso(ctrl_addr,&aso));

aso->addr_start = ctrl_addr->BaUsed;
IFE(OgAddrAppendBa(ctrl_addr,ihn,hn));
ctrl_addr->Ba[ctrl_addr->BaUsed++]=0;
aso->addr_length = ihn;
aso->hostent = hostent;
aso->port=hport;

/** Now we can create the socket **/
IFE(aso->hsocket = OgCreateSocket(ctrl_addr->herr,aso->port,&aso->hostent));

DONE;
}





