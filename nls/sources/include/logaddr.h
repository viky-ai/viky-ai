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
  og_bool nil_socket;
  char *address;
  int port;
  struct sockaddr_in socket_in;
  char sremote_addr[DPcPathSize];
  int hsocket_service;
  int time_start;
};

struct og_addr_param
{
  void *herr, *hmsg;
  ogmutex_t *hmutex;
  struct og_loginfo loginfo;
  og_char_buffer addr_name[DPcPathSize];

  int backlog_max_pending_requests;
  int backlog_timeout;
  int (*must_stop_func)(void *func_context);
  int (*get_backlog_timeout_func)(void *func_context);
  int (*send_error_status_func)(void *func_context, struct og_socket_info *info, int error_status, og_string message);
  void *func_context;
  };


DEFPUBLIC(void *) OgAddrInit(struct og_addr_param *params);
DEFPUBLIC(int) OgAddrAdd(void *handle, char *hostname, int port);
DEFPUBLIC(int) OgAddrLoop(void *handle, int (*answer_func)(void *, struct og_socket_info *info), void *answer_func_context);
DEFPUBLIC(int) OgAddrClose(void *handle);
DEFPUBLIC(int) OgAddrFlush(void *handle);

#define _LOGADDRALIVE_
#endif

