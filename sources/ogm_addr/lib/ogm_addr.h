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
  struct og_loginfo cloginfo;
  struct og_loginfo *loginfo;

  int AsoNumber, AsoUsed, FreeAso;
  struct aso *Aso;

  /** Buffer for all operations **/
  unsigned char *Ba;
  int BaSize, BaUsed;

  void *ghbn;

  GAsyncQueue *async_socket_queue;
  ogthread_t thread;

  og_heap sockets;
  og_heap error_messages;

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

