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

void Daemonize(struct og_nls_prog *nls_prog)
{
  int pid = fork();
  if (pid < 0)
  {
    MessageErrorLog(0, nls_prog->loginfo->where, 0, "Can't make first fork");
    exit(EXIT_FAILURE);
  }
  if (pid > 0)
  {
    exit(EXIT_SUCCESS);
  }

  setsid();

  pid = fork();

  if (pid < 0)
  {
    MessageErrorLog(0, nls_prog->loginfo->where, 0, "Can't make second fork");
    exit(EXIT_FAILURE);
  }

  if (pid > 0)
  {
    exit(EXIT_SUCCESS);
  }

  for (int i = getdtablesize(); i >= 0; --i)
  {
    close(i);
  }

  int i = open("/dev/null", O_RDWR);
  dup(i);
  dup(i);

  OgMsg(nls_prog->hmsg, "", DOgMsgDestInLog, "Program ogm_nls starting in daemon mode");

}

