/*
 *	Function listening to all added addresses.
 *	Copyright (c) 2005-2006 Pertimm by Patrick Constant
 *	Dev : March 2005, December 2006
 *	Version 1.1
*/
#include "ogm_addr.h"




/*
 *  Since 'func' is handling the request coming from a given socket,
 *  its return value controls the stopping of the loop:
 *  if func returns 0, loop is continuing , else if func returns 1, 
 *  the loop must break.
*/

PUBLIC(int) OgAddrLoop(handle,func,ptr)
void *handle; 
int (*func)(pr_(void *) pr(struct og_socket_info *));
void *ptr;  
{
struct og_ctrl_addr *ctrl_addr = (struct og_ctrl_addr *)handle;
char sys_erreur[DOgErrorSize],erreur[DOgErrorSize];
struct og_socket_info cinfo, *info=&cinfo;
int i,maxfd,size,nerr,must_stop;
struct aso *aso;
fd_set *fdset;

/** setup fd set for listen **/
maxfd=0;
for (i=0; i<ctrl_addr->AsoUsed; i++) {
  aso = ctrl_addr->Aso + i;
  if (aso->hsocket > maxfd) maxfd=aso->hsocket;
  }
size=(maxfd+1)*sizeof(fd_set);
IFn(fdset=(fd_set *)malloc(size)) {
  sprintf(erreur,"OgAddrLoop: malloc error on fdset (%d bytes)",size);
  OgErr(ctrl_addr->herr,erreur); return(0);
  }

while (1) {
  memset(fdset,0,size);
  for (i=0; i<ctrl_addr->AsoUsed; i++) {
    aso = ctrl_addr->Aso + i;
    FD_SET(aso->hsocket,fdset);
    }
  /** Wait in select until there is a connection, no timeout **/
  if (select(maxfd+1, fdset, NULL, NULL, NULL) == DOgSocketError) {
    nerr = OgSysErrMes(DOgSocketErrno,DOgErrorSize,sys_erreur);
    if (nerr == DOgErrorEINTR) {
      OgMsg(ctrl_addr->hmsg, "", DOgMlogInLog+DOgMlogInErr
        , "OgAddrLoop: caught %d error: %s (going on)",nerr,sys_erreur);
      }
    else {
      sprintf(erreur,"OgAddrLoop: select: (%d) %s\n",nerr,sys_erreur);
      OgErr(ctrl_addr->herr,erreur); DPcErr;
      }
    }
  for (i=0; i<ctrl_addr->AsoUsed; i++) {
    aso = ctrl_addr->Aso + i;
    if (!FD_ISSET(aso->hsocket,fdset)) continue;

    memset(info,0,sizeof(struct og_socket_info));
    info->address = ctrl_addr->Ba+aso->addr_start;
    info->port = aso->port;

    /** Connection with the client, because of select, we know there is a request **/
    IF(info->hsocket_service = SocketAcceptConnexion(ctrl_addr->herr, aso->hsocket, &info->socket_in)) {
      OgErrLast(ctrl_addr->herr,erreur,0); 
      OgMsg(ctrl_addr->hmsg, "", DOgMlogInLog+DOgMlogInErr
        , "OgAddrLoop: failed to connect with the client '%s:%d' \n%s"
        ,ctrl_addr->Ba+aso->addr_start, aso->port, erreur);
      continue;
      }
    #if 0
    OgMsg(ctrl_addr->hmsg, "", DOgMlogInLog+DOgMlogInErr
      , "OgAddrLoop: SocketAcceptConnexion returns socket %d", info->hsocket_service);
    #endif

    /** Here we get the IP address of the calling program through the socket **/
    IFE(OgGetRemoteAddrSocket(&info->socket_in.sin_addr,info->sremote_addr,0));

    IFE(must_stop=(*func)(ptr,info));
    if (must_stop) goto endOgAddrLoop;
    }
  } 

endOgAddrLoop:

DPcFree(fdset);

DONE;
}





