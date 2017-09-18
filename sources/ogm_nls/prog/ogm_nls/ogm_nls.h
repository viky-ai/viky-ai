/*
 *  This is header for ogm_nls.exe
 *  Copyright (c) 2017 Pertimm by Patrick Constant
 *  Dev : August 2017
 *  Version 1.1
 */
#include <lognls.h>
#include <logpath.h>
#include <logsig.h>
#include <signal.h>
#include <execinfo.h>
#include <glib.h>

/** simple server configuration file for this program **/
#define DOgFileConfOgmSsi_Txt  "conf/ogm_ssi.txt"

struct og_nls_prog
{
  char cwd[DPcPathSize];
  char WorkingDirectory[DPcPathSize];
  char DirControl[DPcPathSize];

  unsigned pid;

  void *herr, *hmsg, *hsig;
  ogmutex_t *hmutex;

  struct og_loginfo loginfo[1];

  struct og_nls_param *param;
  og_nls hnls;

  char sremote_addr[DPcPathSize];

};

/** ogm_nls.c **/
void DoExit(struct og_nls_prog *nls_ctrl);

/** nlsdaem.c **/
void Daemonize(struct og_nls_prog *nls_ctrl);

