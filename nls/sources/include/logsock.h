/*
 *  Public header for socket handling
 *  Copyright (c) 2001-2004 Pertimm
 *  Dev : April,July 2001, February 2003, February,March,April,June,December 2004
 *  Dev: March,April,October 2005, February,June,July,October,December 2006
 *  Dev: January 2007
 *  Version 2.5
*/
#ifndef _LOGSOCKALIVE_
#include <loggen.h>

#define DOgSockBanner  "ogm_sock V1.70, Copyright (c) 2004-2008 Pertimm, Inc."
#define DOgSockVersion 170

#if (DPcSystem == DPcSystemUnix)
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include<netinet/tcp.h>
#include <netdb.h>
#include <fcntl.h>
#define DOgSocketErrno      errno
#define DOgSocketError      (-1)
#define DOgInvalidSocket    (-1)
#define DOgErrorEINTR       EINTR
extern int errno;
#else
#if (DPcSystem == DPcSystemWin32)
/* <winsock.h> is included in <windows.h> 
 * which is included in <loggen.h> 
 * but inclusion of <winsock2.h> before
 * <windows.h> prevents inclusion of <winsock.h>
 * this way, <winsock2.h> prevails over <winsock.h>
*/
#define DOgSocketErrno      WSAGetLastError()
#define DOgSocketError      SOCKET_ERROR
#define DOgInvalidSocket    INVALID_SOCKET
#define DOgErrorEINTR       WSAEINTR
#endif
#endif

#define DOgProtocolNil      0
#define DOgProtocolHttp     1
#define DOgProtocolFile     2
#define DOgProtocolHttps    3
#define DOgProtocolFtp      4

#define DOgUrlSize 2048

/** Max HTTP Header Line Size **/
#define DOgMhhls 1024

/** All header lines per RFC 2616 **/
#define DOgHnHttp                   0
#define DOgHnAccept                 1
#define DOgHnAcceptCharset          2
#define DOgHnAcceptEncoding         3
#define DOgHnAcceptLanguage         4
#define DOgHnAcceptRanges           5
#define DOgHnAge                    6
#define DOgHnAllow                  7
#define DOgHnAuthorization          8
#define DOgHnCacheControl           9
#define DOgHnConnection            10
#define DOgHnContentEncoding       11
#define DOgHnContentLanguage       12
#define DOgHnContentLength         13
#define DOgHnContentLocation       14
#define DOgHnContentMD5            15
#define DOgHnContentRange          16
#define DOgHnContentType           17
#define DOgHnCookie                18
#define DOgHnCookie2               19
#define DOgHnDate                  20
#define DOgHnETag                  21
#define DOgHnExpect                22
#define DOgHnExpires               23
#define DOgHnFrom                  24
#define DOgHnHost                  25
#define DOgHnIfMatch               26
#define DOgHnIfModifiedSince       27
#define DOgHnIfNoneMatch           28
#define DOgHnIfRange               29
#define DOgHnIfUnmodifiedSince     30
#define DOgHnLastModified          31
#define DOgHnLocation              32
#define DOgHnMaxForwards           33
#define DOgHnPragma                34
#define DOgHnProxyAuthenticate     35
#define DOgHnProxyAuthorization    36
#define DOgHnProxyConnection       37
#define DOgHnRange                 38
#define DOgHnReferer               39
#define DOgHnRetryAfter            40
#define DOgHnServer                41
#define DOgHnStartSetCookie        42
#define DOgHnEndSetCookie          62
#define DOgHnStartSetCookie2       62
#define DOgHnEndSetCookie2         82
#define DOgHnTE                    82
#define DOgHnTrailer               83
#define DOgHnTransferEncoding      84
#define DOgHnUpgrade               85
#define DOgHnUserAgent             86
#define DOgHnVary                  87
#define DOgHnVia                   88
#define DOgHnWarning               89
#define DOgHnWWWAuthenticate       90
#define DOgMaxHhl                  91


struct og_http_header_line {
  int ivalue; char value[DOgMhhls];
  };

struct og_http_header {
  struct og_http_header_line line[DOgMaxHhl];
  int nbSetCookie,nbSetCookie2;
  int major,minor,status;
  size_t content_length; 
  time_t last_modified;
  int header_length;
  };

struct og_call_socket_stat {
  int elapsed, elapsed_inet_addr, elapsed_gethostbyname;
  int elapsed_htons, elapsed_socket, elapsed_connect;
  int connect_set_non_block,connect_connect,connect_select,connect_getsockopt,connect_set_block;
  int nb_connect;
  };

struct og_hostent {
  struct in_addr sin_addr;
  short sin_family;
  int h_length;
  int found;
  };


DEFPUBLIC(int) OgStartupSockets(pr(void *));
DEFPUBLIC(int) OgCreateSocket(pr_(void *) pr_(int) pr(struct og_hostent *));
DEFPUBLIC(int) OgFinalCloseSocket(pr(int));
DEFPUBLIC(void) OgCleanupSocket(pr(void)); 
DEFPUBLIC(int) OgCloseSocket(pr(int));
DEFPUBLIC(int) OgListenSocket(pr_(void *) pr_(int) pr(int));
DEFPUBLIC(int) SocketAcceptConnexion(pr_(void *) pr_(int) pr(struct sockaddr_in *));
DEFPUBLIC(int) OgSendSocket(pr_(void *) pr_(int) pr_(char *) pr(int));
DEFPUBLIC(int) OgRecvSocket(pr_(void *) pr_(int) pr_(char *) pr(int));
DEFPUBLIC(int) OgTimeoutRecvSocket(pr_(void *) pr_(int) pr_(char *) pr_(int) pr_(int) pr(int *));
DEFPUBLIC(int) OgCallSocket(pr_(void *) pr_(char *) pr(int));
DEFPUBLIC(int) OgCallSocketStat(pr_(void *) pr_(char *) pr_(int) pr(struct og_call_socket_stat *));
DEFPUBLIC(int) OgTimeoutCallSocket(pr_(void *) pr_(char *) pr_(int) pr_(int) pr(int *));
DEFPUBLIC(int) OgTimeoutCallSocketStat(pr_(void *) pr_(char *) pr_(struct og_hostent *) 
                 pr_(int) pr_(int) pr_(int *) pr(struct og_call_socket_stat *));

DEFPUBLIC(int) OgGetRemoteAddrSocket(pr_(struct in_addr *) pr_(char *) pr(int *));

DEFPUBLIC(int) OgGetHostByAddr(pr_(char *) pr(char *));

DEFPUBLIC(int) OgHttpHeaderParse(pr_(int) pr_(char *) pr(struct og_http_header *));
DEFPUBLIC(int) OgHttpHeaderGetLastModified(pr_(char *) pr(time_t *));
DEFPUBLIC(int) OgHttpHeaderLog(pr_(struct og_http_header *) pr(char *));

#define _LOGSOCKALIVE_
#endif

