/*
 *  Header for library ogm_addr.dll
 *  Copyright (c) 2005-2007 Pertimm by Patrick Constant
 *  Dev : March,September 2005, January 2007
 *  Version 1.2
*/
#ifndef _LOGADDRALIVE_
#include <logsock.h>
#include <logghbn.h>
#include <loggen.h>
#include <logthr.h>
#include <logmsg.h>
#include <lpcgentype.h>


#define DOgAddrBanner  "ogm_addr V1.08, Copyright (c) 2005-2007 Pertimm, Inc."
#define DOgAddrVersion 108


/** Trace levels **/
#define DOgAddrTraceMinimal              0x1
#define DOgAddrTraceMemory              0x2
#define DOgAddrTraceAdding              0x4
#define DOgAddrTraceGhbn                0x8


struct og_socket_info
{
  char *address;
  int port;
  struct sockaddr_in socket_in;
  char sremote_addr[DPcPathSize];
  int hsocket_service;

  //socket function
  int (*func)(void *, struct og_socket_info *info);

  //socket context
  void *ptr;

  int time_start;
  int timeout;
};

struct og_addr_param {
  void *herr,*hmsg; ogmutex_t *hmutex;
  struct og_loginfo loginfo;
  };


DEFPUBLIC(void *) OgAddrInit(pr(struct og_addr_param *));
DEFPUBLIC(int) OgAddrAdd(pr_(void *) pr_(char *) pr(int));
DEFPUBLIC(int) OgAddrFlush(pr(void *));
DEFPUBLIC(int) OgAddrLoop(void *handle, int timeout, int queue_max_length, int (*func)(void *, struct og_socket_info *info), void *ptr);
DEFPUBLIC(int) OgAddrSocketQueue(void *ptr);

#define _LOGADDRALIVE_
#endif

