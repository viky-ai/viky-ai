/*
 *  General socket handling.
 *  Copyright (c) 1999-2005 Pertimm by Patrick Constant
 *  Dev : October 1999, December 2004, October 2005, January 2007
 *  Version 1.4
*/
#include "ogm_sock.h"



#if (DPcSystem == DPcSystemUnix)

/* #include <netinet/in.h> */
#include <sys/time.h>
#include <unistd.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>


PUBLIC(int) OgStartupSockets(void *herr) { DONE; }
PUBLIC(int) OgFinalCloseSocket(int hsocket) { return(close(hsocket)); }
PUBLIC(int) OgCloseSocket(int hsocket) { return(close(hsocket)); }
PUBLIC(void) OgCleanupSocket() {}


#else
#if (DPcSystem == DPcSystemWin32)

/*
 *  Gets information on socket version
 *  and checks if the version is high enough
 *  Must be done anyway, because Windows asks for it.
*/

PUBLIC(int) OgStartupSockets(herr)
void *herr;
{
WSADATA wsaData;
char sys_erreur[DPcSzErr], erreur[DPcSzErr];
WORD wVersionRequested = MAKEWORD(2,0);

if (WSAStartup(wVersionRequested,&wsaData) == DOgSocketError) {
  int nerr = OgSysErrMes(DOgSocketErrno,DPcSzSysErr,sys_erreur);
  sprintf(erreur,"WSAStartup: (%d) %s\n",nerr,sys_erreur);
  OgCleanupSocket(); OgErr(herr,erreur); DPcErr;
  }
DONE;
}



PUBLIC(int) OgFinalCloseSocket(int hsocket)
{
int retour;
retour=closesocket(hsocket);
OgCleanupSocket();
return(retour);
}



PUBLIC(int) OgCloseSocket(int hsocket) { return(closesocket(hsocket)); }
PUBLIC(void) OgCleanupSocket() { WSACleanup(); }

#endif
#endif




PUBLIC(int) OgCreateSocket(herr, port_socket_in, hostent)
void *herr; int port_socket_in; struct og_hostent *hostent;
{
int hsocket_in, retour;
struct sockaddr_in socket_in;
int longueur_addr = sizeof (struct sockaddr_in);
char sys_erreur[DPcSzErr], erreur[DPcSzErr];
int one = 1;

/** creation of the socket **/
hsocket_in = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
if (hsocket_in == DOgInvalidSocket) {
  int nerr = OgSysErrMes(DOgSocketErrno,DPcSzSysErr,sys_erreur);
  sprintf(erreur,"OgCreateSocket: socket:(%d) %s\n",nerr,sys_erreur);
  OgCleanupSocket(); OgErr(herr,erreur); DPcErr;
  }

retour = setsockopt(hsocket_in, SOL_SOCKET, SO_REUSEADDR,  (void *)(&one), sizeof(int));
if (retour == DOgSocketError) {
  int nerr = OgSysErrMes(DOgSocketErrno,DPcSzSysErr,sys_erreur);
  sprintf(erreur,"OgCreateSocket: setsockopt:(%d) %s\n",nerr,sys_erreur);
  OgCleanupSocket(); OgErr(herr,erreur); DPcErr;
  }

/** binding of the socket **/
IFn(hostent) {
  socket_in.sin_family = AF_INET;
  socket_in.sin_addr.s_addr = htonl(INADDR_ANY);
  }
else {
  socket_in.sin_family = hostent->sin_family;
  socket_in.sin_addr = hostent->sin_addr;
  }
socket_in.sin_port = htons((unsigned short)port_socket_in);
if ( bind(hsocket_in, (struct sockaddr *)&socket_in, longueur_addr) == DOgSocketError) {
  int nerr = OgSysErrMes(DOgSocketErrno,DPcSzSysErr,sys_erreur);
  /* Can't get the system's message, but message WSAEADDRINUSE (10048) means :
   * The IP address/port has already been used for an existing socket.
  */
  sprintf( erreur, "OgCreateSocket: bind: (%d) IP address/port (%u) already in use, server might be up already", nerr, port_socket_in);
  OgFinalCloseSocket(hsocket_in); OgErr(herr,erreur); DPcErr;
  }

if ( listen(hsocket_in, SOMAXCONN) == DOgSocketError) {
  int nerr = OgSysErrMes(DOgSocketErrno,DPcSzSysErr,sys_erreur);
  sprintf(erreur,"OgCreateSocket: listen: (%d) %s\n",nerr,sys_erreur);
  OgFinalCloseSocket(hsocket_in); OgErr(herr,erreur); DPcErr;
  }

return hsocket_in;
}





PUBLIC(int) OgListenSocket(herr,hsocket_in,nb_pendantes)
void *herr; int hsocket_in,nb_pendantes;
{
if ( listen(hsocket_in, nb_pendantes) == DOgSocketError) {
  char sys_erreur[DPcSzErr], erreur[DPcSzErr];
  int nerr = OgSysErrMes(DOgSocketErrno,DPcSzSysErr,sys_erreur);
  sprintf(erreur,"OgCreateSocket: listen: (%d) %s\n",nerr,sys_erreur);
  OgFinalCloseSocket(hsocket_in); OgErr(herr,erreur); DPcErr;
  }
DONE;
}



/*
 *  la fonction retourne un descripteur de socket de dialogue avec le client
*/

PUBLIC(int) SocketAcceptConnexion(herr,hsocket_in,socket_in_effective)
void *herr; int hsocket_in; struct sockaddr_in *socket_in_effective;
{
int lg_adresse = sizeof(struct sockaddr_in);
struct linger linger;
int hsocket_service;
int on=1;

if ((hsocket_service = accept(hsocket_in, (struct sockaddr *)socket_in_effective, &lg_adresse)) == DOgInvalidSocket) {
  char sys_erreur[DPcSzErr], erreur[DPcSzErr];
  int nerr = OgSysErrMes(DOgSocketErrno,DPcSzSysErr,sys_erreur);
  sprintf(erreur,"OgCreateSocket: accept: (%d) %s\n",nerr,sys_erreur);
  OgErr(herr,erreur); DPcErr;
  }

linger.l_onoff = 0;
linger.l_linger = 1;
setsockopt(hsocket_service, SOL_SOCKET, SO_LINGER, (char *)&linger, sizeof(linger));
setsockopt(hsocket_service, IPPROTO_TCP, TCP_NODELAY, (char *)&on, sizeof(on));

return(hsocket_service);
}




PUBLIC(int) OgSendSocket(herr,hsocket,string,istring)
void *herr; int hsocket; char *string; int istring;
{
int nb_sent_chars,flags=0;
#if (DPcSystem == DPcSystemUnix)
#if (DPcArch == DPcArchsolaris)
#define MSG_NOSIGNAL  0x0
#endif
/** We don't want to receive a signal and the EPIPE error is ok **/
flags = MSG_NOSIGNAL;
#endif
if ((nb_sent_chars=send(hsocket, string, istring, flags)) == DOgSocketError) {
  char sys_erreur[DPcSzErr], erreur[DPcSzErr];
  int nerr = OgSysErrMes(DOgSocketErrno,DPcSzSysErr,sys_erreur);
  sprintf(erreur,"OgSendSocket: (%d) %s",nerr,sys_erreur);
  OgErr(herr,erreur); DPcErr;
  }
return(nb_sent_chars);
}





PUBLIC(int) OgRecvSocket(herr,hsocket,string,istring)
void *herr; int hsocket; char *string; int istring;
{
int nb_received_chars;
if ((nb_received_chars=recv(hsocket, string, istring, 0)) == DOgSocketError) {
  char sys_erreur[DPcSzErr], erreur[DPcSzErr];
  int nerr = OgSysErrMes(DOgSocketErrno,DPcSzSysErr,sys_erreur);
  sprintf(erreur,"OgRecvSocket: (%d) %s",nerr,sys_erreur);
  OgErr(herr,erreur); DPcErr;
  }
return(nb_received_chars);
}





PUBLIC(int) OgTimeoutRecvSocket(herr,hsocket,string,istring,timeout,timed_out)
void *herr; int hsocket; char *string; int istring, timeout, *timed_out;
{
fd_set read_fds;
struct timeval tv;
int n;

FD_ZERO(&read_fds);
FD_SET(hsocket, &read_fds);
tv.tv_sec = timeout/1000; /* seconds */
tv.tv_usec = (timeout%1000)*1000; /* milli-seconds */

*timed_out = 0;

if (timeout==0) {
  return(OgRecvSocket(herr,hsocket,string,istring));
  }


/* From Windows documentation on select, Windows does not care about n,
 * but we use the value given in an example to be on the safe side. */
#if (DPcSystem == DPcSystemUnix)
n = hsocket+1;
#else
#if (DPcSystem == DPcSystemWin32)
n = FD_SETSIZE;
#endif
#endif

before_select:
if (select(n, &read_fds, NULL, NULL, &tv) == DOgSocketError) {
  char sys_erreur[DPcSzErr], erreur[DPcSzErr];
  int nerr = OgSysErrMes(DOgSocketErrno,DPcSzSysErr,sys_erreur);
  /** December 20th 2006: this interupt is not interesting **/
  if (nerr == DOgErrorEINTR) goto before_select;
  sprintf(erreur,"OgTimeoutRecvSocket: (%d) %s\n",nerr,sys_erreur);
  OgErr(herr,erreur); DPcErr;
  }

if (FD_ISSET(hsocket, &read_fds)) {
  return(OgRecvSocket(herr,hsocket,string,istring));
  }
else {
  *timed_out = 1;
  }
DONE;
}




/*
 *  This function is used on the Client side to get a connection socket to the server.
 *  On error, you don't have to close the socket.
*/

PUBLIC(int) OgCallSocket(herr,hostname,portnum)
void *herr; char *hostname; int portnum;
{
return(OgTimeoutCallSocketStat(herr,hostname,0,portnum,0,0,0));
}




PUBLIC(int) OgTimeoutCallSocket(herr,hostname,portnum,timeout,timed_out)
void *herr; char *hostname; int portnum,timeout,*timed_out;
{
return(OgTimeoutCallSocketStat(herr,hostname,0,portnum,timeout,timed_out,0));
}




PUBLIC(int) OgCallSocketStat(herr,hostname,portnum,st)
void *herr; char *hostname; int portnum;
struct og_call_socket_stat *st;
{
return(OgTimeoutCallSocketStat(herr,hostname,0,portnum,0,0,st));
}




PUBLIC(int) OgTimeoutCallSocketStat(herr,hostname,hostent,portnum,timeout,timed_out,st)
void *herr; char *hostname; struct og_hostent *hostent;
int portnum, timeout, *timed_out;
struct og_call_socket_stat *st;
{
unsigned int addr;
struct sockaddr_in sa;
char sys_erreur[512], erreur[DOgErrorSize];
int ogclock,clock_start=0,overall_clock_start=0;
struct timeval tv;
int hsocket,nerr;

memset(&sa,0,sizeof(sa));

if (timeout>0) {
  tv.tv_sec = timeout; /* seconds */
  tv.tv_usec = 0; /* milli-seconds */
  }

IFx(st) {
  memset(st,0,sizeof(struct og_call_socket_stat));
  overall_clock_start = clock_start = OgClock();
  }

/** We should use inet_aton instead of inet_addr because of return value **/
addr = inet_addr(hostname);

IFx(st) {
  ogclock=OgClock(); st->elapsed_inet_addr= ogclock-clock_start; clock_start=ogclock;
  }

if (addr == (unsigned)-1) {
  if (hostent) {
    sa.sin_addr=hostent->sin_addr;
    sa.sin_family= hostent->sin_family;
    }
  else {

    #if (DPcSystem == DPcSystemUnix)
    struct addrinfo *res = NULL;
    struct addrinfo hints;

    memset(&hints, 0, sizeof(hints));
    /* ai_family  will  be  set  to  either  PF_INET or  PF_INET6
     * PF_UNSPEC in ai_family specifies any protocol family
     * (either IPv4 or IPv6, for example) */
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    IFx(getaddrinfo(hostname, NULL, &hints, &res)) {
      nerr = OgSysErrMes(DOgSocketErrno,512,sys_erreur);
      sprintf(erreur,"OgTimeoutCallSocketStat: getaddrinfo on hostname '%s', error %d:", hostname, nerr);
      OgErr(herr,erreur);
      sprintf(erreur,"OgTimeoutCallSocketStat: %.180s", ((nerr==11001)?"unknown host":sys_erreur));
      OgErr(herr,erreur);
      DPcErr;
      }
    if (res) {
      if (res->ai_addr && res->ai_addrlen) {
        struct sockaddr_in *ogsin = (struct sockaddr_in *)res->ai_addr;
        //memcpy(&hostent->sin_addr,&ogsin->sin_addr, sizeof(struct in_addr));
        //hostent->sin_family= res->ai_family;
        //hostent->h_length=sizeof(struct in_addr);
        memcpy(&sa.sin_addr,&ogsin->sin_addr, sizeof(struct in_addr));
        sa.sin_family= res->ai_family;
        freeaddrinfo(res);
        }
      }

    #else
    struct hostent *hp;

    IFn ((hp = gethostbyname(hostname))) {
      nerr = OgSysErrMes(DOgSocketErrno,512,sys_erreur);
      sprintf(erreur,"OgTimeoutCallSocketStat: %.230s", ((nerr==11001)?"unknown host":sys_erreur));
      OgErr(herr,erreur);
      sprintf(erreur,"OgTimeoutCallSocketStat: gethostbyname on hostname '%s', error %d:", hostname, nerr);
      OgErr(herr,erreur);
      DPcErr;
      }
    memcpy((char *)&sa.sin_addr,(char *)hp->h_addr,hp->h_length);     /* set address */
    sa.sin_family= hp->h_addrtype;
    #endif

    }
  }
else {
  memcpy((char *)&sa.sin_addr, (char *)&addr, sizeof(addr));
  sa.sin_family = AF_INET;
  }

IFx(st) {
  ogclock=OgClock(); st->elapsed_gethostbyname= ogclock-clock_start; clock_start=ogclock;
  }

sa.sin_port= htons((unsigned short)portnum);

IFx(st) {
  ogclock=OgClock(); st->elapsed_htons= ogclock-clock_start; clock_start=ogclock;
  }

IF(hsocket = socket(sa.sin_family,SOCK_STREAM,0)) {
  nerr = OgSysErrMes(DOgSocketErrno,512,sys_erreur);
  sprintf(erreur,"OgTimeoutCallSocketStat: %.230s", ((nerr==11001)?"unknown host":sys_erreur));
  OgErr(herr,erreur);
  sprintf(erreur,"OgTimeoutCallSocketStat: socket on hostname '%s:%d', error %d:", hostname, portnum, nerr);
  OgErr(herr,erreur);
  DPcErr;
  }

IFx(st) {
  ogclock=OgClock(); st->elapsed_socket=ogclock-clock_start; clock_start=ogclock;
  }

if (timeout>0) {
  IF(php_connect_nonb(hsocket,(struct sockaddr *)&sa,sizeof sa,&tv,timed_out,st)){ /* connect */
    nerr = OgSysErrMes(DOgSocketErrno,512,sys_erreur);
    sprintf(erreur,"OgTimeoutCallSocketStat: %.230s", ((nerr==11001)?"unknown host":sys_erreur));
    OgErr(herr,erreur);
    sprintf(erreur,"OgTimeoutCallSocketStat: php_connect_nonb on hostname '%s:%d', error %d:", hostname, portnum, nerr);
    OgErr(herr,erreur);
    OgCloseSocket(hsocket);
    DPcErr;
    }
  }
else {
  IF(connect(hsocket,(struct sockaddr *)&sa,sizeof sa)){ /* connect */
    nerr = OgSysErrMes(DOgSocketErrno,512,sys_erreur);
    sprintf(erreur,"OgTimeoutCallSocketStat: %.230s", ((nerr==11001)?"unknown host":sys_erreur));
    OgErr(herr,erreur);
    sprintf(erreur,"OgTimeoutCallSocketStat: connect on hostname '%s:%d', error %d:", hostname, portnum, nerr);
    OgErr(herr,erreur);
    OgCloseSocket(hsocket);
    DPcErr;
    }
  }

IFx(st) {
  ogclock=OgClock(); st->elapsed_connect=ogclock-clock_start; clock_start=ogclock;
  st->elapsed=ogclock-overall_clock_start;
  }

return(hsocket);
}




