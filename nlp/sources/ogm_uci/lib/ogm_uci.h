/*
 *  Internal header for handling uci (upper communication interface)
 *  Copyright (c) 2006-2007 Pertimm, Inc. by Patrick Constant
 *  Dev : July 2006, May 2007
 *  Version 1.1
*/
#include <loguci.h>
#include <logheap.h>


//#define OgThnMessageLog OgThrMessageLog
//#define OgThnErrLog OgThrErrLog

#define DOgBaSize   0x1000 /* 4k */

#define DOgUciTraceMinimal          0x1
#define DOgUciTraceMemory           0x2
#define DOgUciTraceSocket           0x4
#define DOgUciTraceSocketSize       0x8


struct og_uci_param {
  void *herr,*hmsg; ogmutex_t *hmutex;
  struct og_loginfo loginfo;
  int socket_buffer_size;
  int header_mandatory;
  };

struct og_ucir_input {
  int hsocket,timeout;
  };

struct og_ucir_output {
  struct og_http_header2 hh;
  unsigned char top_level_tag[DOgMaxTopLevelTagSize];
  int elapsed_total,elapsed_recv;
  unsigned char *content;
  int content_length;
  int header_length;
  int timed_out;
  };

struct og_ctrl_uci {
  void *herr,*hmsg; ogmutex_t *hmutex;
  struct og_loginfo cloginfo;
  struct og_loginfo *loginfo;
  int header_mandatory;
  void *hhttp;
  int chunked;
  
  /** Buffer for all operations **/
  og_heap hba;

  };


struct og_ctrl_ucis {
  void *herr,*hmsg; ogmutex_t *hmutex;
  struct og_loginfo cloginfo;
  struct og_loginfo *loginfo;
  int connection_closed;
  void *huci;
  /** Buffer for all operations **/
  unsigned char *Ba;
  int BaSize,BaUsed;
  };


struct og_ctrl_ucic {
  void *herr,*hmsg; ogmutex_t *hmutex;
  struct og_loginfo cloginfo;
  struct og_loginfo *loginfo;
  void *huci;
  struct og_ucic_request *request;
  int hsocket;
  /** Buffer for all operations **/
  unsigned char *Ba;
  int BaSize,BaUsed;
  };


/** iuci.c **/
void *OgUciInit(pr(struct og_uci_param *));
int OgUciFlush(pr(void *));

/** uciread.c **/
int OgUciRead(pr_(void *) pr_(struct og_ucir_input *) pr(struct og_ucir_output *));

/** uciba.c **/
int UciAppendBa(pr_(struct og_ctrl_uci *) pr_(int) pr(unsigned char const *));
int UciTestReallocBa(pr_(struct og_ctrl_uci *) pr(int));
int UciReallocBa(pr_(struct og_ctrl_uci *) pr(int));

/** ucicba.c **/
int UcicAppendBa(pr_(struct og_ctrl_ucic *) pr_(int) pr(unsigned char const *));
int UcicTestReallocBa(pr_(struct og_ctrl_ucic *) pr(int));
int UcicReallocBa(pr_(struct og_ctrl_ucic *) pr(int));

/** ucisba.c **/
int UcisAppendBa(pr_(struct og_ctrl_ucis *) pr_(int) pr(unsigned char const *));
int UcisTestReallocBa(pr_(struct og_ctrl_ucis *) pr(int));
int UcisReallocBa(pr_(struct og_ctrl_ucis *) pr(int));



