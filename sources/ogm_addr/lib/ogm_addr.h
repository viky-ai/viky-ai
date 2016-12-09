/*
 *  Internal header for handling listening to several IP addresses
 *  Copyright (c) 2005 Pertimm, Inc. by Patrick Constant
 *  Dev : March 2005
 *  Version 1.0
*/

#include <logaddr.h>
#include <glib.h>
#include <logheap.h>


#define DOgAsoNumber    0x400
#define DOgBaSize       0x1000 /* 4k */


/*
 *  Handling address to listen to and corresponding socket information.
*/

struct aso {
  int addr_start,addr_length,port;
  struct og_hostent hostent;
  int hsocket;
  int nx_free;
  };


struct og_ctrl_addr
{
  void *herr, *hmsg;
  ogmutex_t *hmutex;
  struct og_loginfo loginfo[1];

  og_char_buffer addr_name[DPcPathSize];

  int backlog_max_pending_requests;
  int backlog_timeout;
  int down_timeout;

  int AsoNumber, AsoUsed, FreeAso;
  struct aso *Aso;

  /** Buffer for all operations **/
  unsigned char *Ba;
  int BaSize, BaUsed;

  void *ghbn;

  /** AsyncQueue of (struct og_socket_info *) */
  GAsyncQueue *async_socket_queue;
  ogthread_t thread;

  int (*answer_func)(void *answer_func_context, struct og_socket_info *info);
  void *answer_func_context;
  int (*must_stop_func)(void *func_context);
  int (*get_backlog_timeout_func)(void *func_context);
  og_bool (*search_unavailable_state_func)(void *func_context);
  int (*send_error_status_func)(void *func_context, struct og_socket_info *info, int error_status, og_string message);
  void *func_context;
  int must_stop;
  ogsem_t hsem[1];

  og_bool closed;

};



/** addraso.c **/
int GetAso(pr_(struct og_ctrl_addr *) pr(struct aso **));


/** addrba.c **/
int OgAddrAppendBa(pr_(struct og_ctrl_addr *) pr_(int) pr(unsigned char *));
int OgAddrTestReallocBa(pr_(struct og_ctrl_addr *) pr(int));
int OgAddrReallocBa(pr_(struct og_ctrl_addr *) pr(int));

/** addsocketqueue.c **/
og_status OgAddrSocketQueue(void *ptr);

/** addrutils.c **/
og_status AddrSendStatusCodeServiceUnavailable(struct og_ctrl_addr *ctrl_addr, int socket);

