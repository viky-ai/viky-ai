/*
 *  Header for library ogm_uci.dll
 *  Copyright (c) 2006 Pertimm by Patrick Constant
 *  Dev : July,August,September 2006, January,May,August 2007
 *  Version 1.5
*/
#ifndef _LOGUCIALIVE_
#include <loggen.h>
#include <logthn.h>
#include <logsock.h>
#include <loghttp.h>
#include <logmsg.h>


#define DOgUciBanner  "ogm_uci V1.36, Copyright (c) 2006-2007 Pertimm, Inc."
#define DOgUciVersion 136

/** Trace levels for server side **/
#define DOgUciServerTraceMinimal          0x1
#define DOgUciServerTraceMemory           0x2
#define DOgUciServerTraceSocket           0x4
#define DOgUciServerTraceSocketSize       0x8

/** Trace levels for server side **/
#define DOgUciClientTraceMinimal          0x1
#define DOgUciClientTraceMemory           0x2
#define DOgUciClientTraceSocket           0x4
#define DOgUciClientTraceSocketSize       0x8

#define DOgUciPortNumber    9184

#define DOgMaxTopLevelTagSize   256


struct og_uci_server_param {
  void *herr,*hmsg; ogmutex_t *hmutex;
  struct og_loginfo loginfo;
  int socket_buffer_size;
  int header_mandatory;
  };

struct og_ucisr_input {
  int hsocket,timeout;
  };

struct og_ucisr_output {
  struct og_http_header2 hh;
  unsigned char top_level_tag[DOgMaxTopLevelTagSize];
  int elapsed_total,elapsed_recv;
  unsigned char *content;
  int content_length;
  int header_length;
  };

struct og_ucisw_input {
  int hsocket,socket_buffer_size;
  struct og_http_header2 hh;
  unsigned char *content;
  int content_length;
  unsigned char *content_type;
  int http_status;
  unsigned char *http_status_message;
  };


struct og_uci_client_param {
  void *herr,*hmsg; ogmutex_t *hmutex;
  struct og_loginfo loginfo;
  int socket_buffer_size;
  int header_mandatory;
  };

struct og_ucic_request {
  char hostname[DPcPathSize]; int port,timeout;
  int request_length; unsigned char *request;
  };

struct og_ucic_answer {
  int answer_length; unsigned char *answer;
  int header_length;
  int timed_out;
  };


DEFPUBLIC(char *) OgUciBanner(pr(void));

DEFPUBLIC(void *) OgUciServerInit(pr(struct og_uci_server_param *));
DEFPUBLIC(int) OgUciServerFlush(pr(void *));
DEFPUBLIC(int) OgUciServerRead(pr_(void *) pr_(struct og_ucisr_input *) pr(struct og_ucisr_output *));
DEFPUBLIC(int) OgUciServerWrite(pr_(void *) pr(struct og_ucisw_input *));
DEFPUBLIC(int) OgUcisMem(void *hucis, int must_log, int module_level,ogint64_t *pmem);

DEFPUBLIC(void *) OgUciClientInit(pr(struct og_uci_client_param *));
DEFPUBLIC(int) OgUciClientFlush(pr(void *));
DEFPUBLIC(int) OgUciClientRequest(pr_(void *) pr_(struct og_ucic_request *) pr(struct og_ucic_answer *));
DEFPUBLIC(int) OgUciMem(void *huci, int must_log, int module_level,ogint64_t *pmem);

#define _LOGUCIALIVE_
#endif


