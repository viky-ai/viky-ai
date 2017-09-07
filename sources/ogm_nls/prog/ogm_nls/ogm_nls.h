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

/** simple server configuration file for this program **/
#define DOgFileConfOgmSsi_Txt  "conf/ogm_ssi.txt"

struct og_nls_ctrl
{
  char cwd[DPcPathSize];
  char WorkingDirectory[DPcPathSize];
  char DirControl[DPcPathSize];
  void *herr, *hmsg, *hsig;
  ogmutex_t *hmutex;
  struct og_loginfo *loginfo;
  struct og_loginfo cloginfo;

  struct og_nls_param *param;
  void *hnls;

  char sremote_addr[DPcPathSize];
  int must_stop;

  unsigned pid;
  int is_service;
  char service_user[DPcPathSize];
  char service_password[DPcPathSize];
  char service_postfix[DPcPathSize];
  int service_automatic;
  char service_name[DPcPathSize];
  char service_displayname[DPcPathSize];
};

/** ogm_nls.c **/
void DoExit(pr(struct og_nls_ctrl *));

/** nlsdaem.c **/
void Daemonize(struct og_nls_ctrl *nls_ctrl);

