/*
 *  This is header for ogucis.exe
 *  Copyright (c) 2006 Pertimm by Patrick Constant
 *  Dev : July 2006
 *  Version 1.0
*/
#include <loguci.h>
#include <logpath.h>
#include <logaddr.h>


#if (DPcSystem == DPcSystemUnix)
#else
#if (DPcSystem == DPcSystemWin32)
// name of the executable
#define SZAPPNAME            "Uci Server"
// internal name of the service
#define SZSERVICENAME        "Uci Server"
// displayed name of the service
#define SZSERVICEDISPLAYNAME "Uci Server"
// list of service dependencies - "dep1\0dep2\0\0"
#define SZDEPENDENCIES       ""
#endif
#endif


struct og_ucis_ctrl {
  char cwd[DPcPathSize];
  char *WorkingDirectory;
  char DirControl[DPcPathSize];
  void *herr; ogmutex_t *hmutex;
  struct og_loginfo *loginfo;
  struct og_loginfo cloginfo;

  struct og_uci_server_param *param;
  void *hucis;

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

  void *haddr;
  struct og_conf_lines addresses;
  char hostname[DPcPathSize];
  int port_number;
  };


/** ogucis.c **/
void DoExit(pr(struct og_ucis_ctrl *));

/** ucisrun.c **/
int UcisRun(pr(struct og_ucis_ctrl *));
int UcisStop(pr(struct og_ucis_ctrl *));


#if (DPcSystem == DPcSystemWin32)
/** yservice.c **/
VOID WINAPI service_main(pr_(DWORD) pr(LPTSTR *));
int RunService(pr_(DWORD) pr(LPTSTR *));
VOID WINAPI service_ctrl(pr(DWORD));
BOOL ReportStatusToSCMgr(pr_(DWORD) pr_(DWORD) pr(DWORD));
void AddToMessageLog(pr(LPTSTR));
void CmdInstallService(pr(void));
void CmdRemoveService(pr(void));
int CmdExistService(pr(void));
#endif


