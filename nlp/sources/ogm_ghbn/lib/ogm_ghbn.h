/*
 *  Header Encapsulation of gethostbyname with the handling of a cache
 *  Copyright (c) 2004 Pertimm
 *  Dev : June 2004
 *  Version 1.0
*/
#define DPcInDll
#include <logghbn.h>
#include <logthr.h>
#include <logaut.h>


#define DOgHostnameNumber   100


struct og_hostname {
  struct og_hostent hostent;
  };


struct og_ctrl_ghbn {
  void *herr; ogmutex_t *hmutex;
  struct og_loginfo cloginfo;
  struct og_loginfo *loginfo; 
  int HostnameNumber,HostnameUsed;
  struct og_hostname *Hostname;
  void *ha;
  };


/** data structure for a gethostbyname thread **/
struct og_ghbn_timeout {
  struct og_ctrl_ghbn *ctrl_ghbn;
  struct og_hostent chostent;
  int found,timed_out;
  char *hostname;
  };


/** ghbn_to.c  **/
int OgGetHostByNameTimeout(pr_(struct og_ctrl_ghbn *) pr_(char *) pr_(struct og_hostent *) pr(int));


