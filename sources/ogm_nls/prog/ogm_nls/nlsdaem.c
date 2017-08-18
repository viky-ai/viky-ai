/*
 *  Transform process to a daemon (Unix only)
 *  Copyright (c) 2007 Pertimm
 *  Dev : March 2007
 *  Version 1.0
*/
#include "ogm_nls.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>



void Daemonize(struct og_nls_ctrl *nls_ctrl)
{
int pid;
int i;

pid = fork();
if (pid<0) {
   MessageErrorLog(0,nls_ctrl->loginfo->where,0,"Can't make first fork");
   exit(EXIT_FAILURE);
   }
if (pid>0) {
   exit(EXIT_SUCCESS);
   }

setsid();

pid=fork();

if (pid<0) {
   MessageErrorLog(0,nls_ctrl->loginfo->where,0,"Can't make second fork");
   exit(EXIT_FAILURE);
   }
if (pid>0) {
  exit(EXIT_SUCCESS);
  }

for (i= getdtablesize(); i >= 0; --i)
   close(i);

i = open ("/dev/null", O_RDWR);
dup(i);
dup(i);

OgMsg(nls_ctrl->hmsg,"",DOgMsgDestInLog
   ,"Program ogm_nls starting in daemon mode");

}

