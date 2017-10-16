/*
 *  Implements the body of the service
 *  Copyright (c) 2006 Pertimm
 *  Dev : July 2006
 *  Version 1.0
*/
#include "ogucis.h"
#include <tchar.h>

extern struct og_ucis_ctrl OgUcisCtrl;

SERVICE_STATUS_HANDLE   sshStatusHandle;
SERVICE_STATUS          ssStatus;       // current status of the service

/** This is also define in the hservice.c source for ogm_phox **/
#define DOG_SERVICE_CONTROL_TERMINATE   128


STATICF(int) OgCreateServiceName(pr(struct og_ucis_ctrl *));


/*
 *  Actual initialization of the service
 *  This routine performs the service initialization and then calls
 *  the user defined RunService() routine to perform majority
 *  of the work.
*/

void WINAPI service_main(DWORD dwArgc, LPTSTR *lpszArgv)
{
struct og_ucis_ctrl *ucis_ctrl = &OgUcisCtrl;

// register our service control handler:
sshStatusHandle = RegisterServiceCtrlHandler( TEXT(ucis_ctrl->service_name), service_ctrl);

if (!sshStatusHandle) {
  MessageErrorLog(0,ucis_ctrl->loginfo->where,0, "service_main: RegisterServiceCtrlHandler failed");
  goto cleanup;
  }

// SERVICE_STATUS members that don't change in example
ssStatus.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
ssStatus.dwServiceSpecificExitCode = 0;

// report the status to the service control manager.
if (!ReportStatusToSCMgr(
    SERVICE_START_PENDING, // service state
    NO_ERROR,              // exit code
    3000)) {               // wait hint
  MessageErrorLog(0,ucis_ctrl->loginfo->where,0, "service_main: ReportStatusToSCMgr failed");
  goto cleanup;
  }

IF(RunService(dwArgc,lpszArgv)) DoExit(ucis_ctrl);

cleanup:

// try to report the stopped status to the service control manager.
if (sshStatusHandle)
  (VOID)ReportStatusToSCMgr(SERVICE_STOPPED,NO_ERROR,0);
return;
}



/*
 *  Actual code of the service
 *  dwArgc: number of command line arguments
 *  lpszArgv: array of command line arguments
*/

int RunService(DWORD dwArgc, LPTSTR *lpszArgv)
{
struct og_ucis_ctrl *ucis_ctrl = &OgUcisCtrl;

OgMessageLog(DOgMlogInLog,ucis_ctrl->loginfo->where,0,"Start initializing service...");

// report the status to the service control manager.
//
if (!ReportStatusToSCMgr(
    SERVICE_RUNNING,       // service state
    NO_ERROR,              // exit code
    0)) {                  // wait hint
  MessageErrorLog(0,ucis_ctrl->loginfo->where,0,"RunService: error on ReportStatusToSCMgr");
  DONE;
  }

OgMessageLog(DOgMlogInLog,ucis_ctrl->loginfo->where,0, "Service is running");

IFE(UcisRun(ucis_ctrl));

DONE;
}




/*
 *  This function is called by the SCM whenever
 *  ControlService() is called on this service.
 *  dwCtrlCode - type of control requested
*/

VOID WINAPI service_ctrl(DWORD dwCtrlCode)
{
struct og_ucis_ctrl *ucis_ctrl = &OgUcisCtrl;

switch(dwCtrlCode) {
  
  // Stop the service.
  case SERVICE_CONTROL_STOP:
    ReportStatusToSCMgr(SERVICE_STOP_PENDING, NO_ERROR, 0);
    OgMessageLog(DOgMlogInLog,ucis_ctrl->loginfo->where,0, "Service is asked SERVICE_CONTROL_STOP");
    UcisStop(ucis_ctrl);
    return;

  //SHUTDOWN SERVICE
  case SERVICE_CONTROL_SHUTDOWN:
    ReportStatusToSCMgr(SERVICE_STOP_PENDING, NO_ERROR, 0);
    OgMessageLog(DOgMlogInLog,ucis_ctrl->loginfo->where,0, "Service is asked SERVICE_CONTROL_SHUTDOWN");
    UcisStop(ucis_ctrl);
    return;

  // Update the service status.
  case SERVICE_CONTROL_INTERROGATE:			
    MessageInfoLog(0,ucis_ctrl->loginfo->where,0, "Service is asked SERVICE_CONTROL_INTERROGATE");
    break;

  // Ucis is not responding to a socket request, we simply terminate
  case DOG_SERVICE_CONTROL_TERMINATE:
    OgMessageLog(DOgMlogInLog,ucis_ctrl->loginfo->where,0, "Service is asked DOG_SERVICE_CONTROL_TERMINATE");
    OgDirCleanNamedPid(ucis_ctrl->DirControl,DOgUcisName);
    OgThrMessageLog(ucis_ctrl->hmutex,DOgMlogInLog+DOgMlogDateIn,ucis_ctrl->loginfo->where,0
      ,"Program %s exiting",ucis_ctrl->loginfo->where);
    ReportStatusToSCMgr(SERVICE_STOPPED, NO_ERROR, 0);
    exit(-1);

  // invalid control code
  default:
    MessageInfoLog(0,ucis_ctrl->loginfo->where,0, "Service is asked dwCtrlCode %d",dwCtrlCode);
    break;
  }
ReportStatusToSCMgr(ssStatus.dwCurrentState, NO_ERROR, 0);
}




/*
 *  PURPOSE: Sets the current status of the service and
 *           reports it to the Service Control Manager
 *
 *  PARAMETERS:
 *    dwCurrentState - the state of the service
 *    dwWin32ExitCode - error code to report
 *    dwWaitHint - worst case estimate to next checkpoint
 *
 *  RETURN VALUE:
 *    TRUE  - success
 *    FALSE - failure
*/

BOOL ReportStatusToSCMgr(DWORD dwCurrentState,
                         DWORD dwWin32ExitCode,
                         DWORD dwWaitHint)
{
static DWORD dwCheckPoint = 1;
BOOL fResult = TRUE;

if (dwCurrentState == SERVICE_START_PENDING) ssStatus.dwControlsAccepted = 0;
else ssStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP|SERVICE_ACCEPT_SHUTDOWN;

ssStatus.dwCurrentState = dwCurrentState;
ssStatus.dwWin32ExitCode = dwWin32ExitCode;
ssStatus.dwWaitHint = dwWaitHint;

if ( ( dwCurrentState == SERVICE_RUNNING ) ||
     ( dwCurrentState == SERVICE_STOPPED ) ) ssStatus.dwCheckPoint = 0;
else ssStatus.dwCheckPoint = dwCheckPoint++;

// Report the status of the service to the service control manager.
if (!(fResult = SetServiceStatus( sshStatusHandle, &ssStatus))) {
  AddToMessageLog(TEXT("SetServiceStatus"));
  }
return fResult;
}




/*
 *  Allows any thread to log an error message
 *  lpszMsg: text for message
*/

VOID AddToMessageLog(LPTSTR lpszMsg)
{
TCHAR   szMsg[256];
HANDLE  hEventSource;
LPTSTR  lpszStrings[2];
DWORD dwErr = GetLastError();
struct og_ucis_ctrl *ucis_ctrl = &OgUcisCtrl;
char *service_name = ucis_ctrl->service_name;
char *service_displayname = ucis_ctrl->service_displayname;

OgCreateServiceName(ucis_ctrl);

// Use event logging to log the error.
hEventSource = RegisterEventSource(NULL, TEXT(service_name));

_stprintf(szMsg, TEXT("%s error: %d"), TEXT(service_name), dwErr);
lpszStrings[0] = szMsg;
lpszStrings[1] = lpszMsg;

if (hEventSource != NULL) {
  ReportEvent(hEventSource, // handle of event source
        EVENTLOG_ERROR_TYPE,  // event type
        0,                    // event category
        0,                    // event ID
        NULL,                 // current user's SID
        2,                    // strings in lpszStrings
        0,                    // no bytes of raw data
        lpszStrings,          // array of error strings
        NULL);                // no raw data

  (VOID) DeregisterEventSource(hEventSource);
  }
}


/*
 *  PURPOSE: Installs the service
*/

void CmdInstallService()
{
struct og_ucis_ctrl *ucis_ctrl = &OgUcisCtrl;
char *service_name = ucis_ctrl->service_name;
char *service_displayname = ucis_ctrl->service_displayname;
int dwStartType=SERVICE_DEMAND_START; /** Manual start by default **/
char *user=NULL, *password=NULL;
char loguser[DPcPathSize];
SC_HANDLE schSCManager;
SC_HANDLE schService;
TCHAR szPath[512];

//Ver.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
//Ver.dwPlatformId = 0;
//Ver.dwMajorVersion=0;
//GetVersionEx( &Ver );
	
//if(Ver.dwMajorVersion==4)  MB_SERVICE_SPC =    0x00200000L;		
//if(Ver.dwMajorVersion==3)  MB_SERVICE_SPC =    0x00040000L;
		
if (ucis_ctrl->service_user[0]) user = ucis_ctrl->service_user;
if (ucis_ctrl->service_password[0]) password = ucis_ctrl->service_password;
if (ucis_ctrl->service_automatic) dwStartType = SERVICE_AUTO_START;
OgCreateServiceName(ucis_ctrl);

if (GetModuleFileName(NULL, szPath, 512)==0) {
  MessageErrorLog(0,ucis_ctrl->loginfo->where,0
    ,"CmdInstallService: unable to GetModuleFileName for %s",SZSERVICEDISPLAYNAME);
  return;
  }

loguser[0]=0; 
if (ucis_ctrl->service_user[0]) {
  sprintf(loguser," with user '%s'",ucis_ctrl->service_user);
  }
OgMessageLog(DOgMlogInLog,ucis_ctrl->loginfo->where,0
  ,"Creating service '%s'%s in %s starting mode"
  , service_name, loguser
  , ucis_ctrl->service_automatic?"automatic":"manual"
  );

schSCManager = OpenSCManager(
                 NULL,                   // machine (NULL == local)
                 NULL,                   // database (NULL == default)
                 SC_MANAGER_ALL_ACCESS   // access required
                 );
if (schSCManager) {
  schService = CreateService(
      schSCManager,               // SCManager database
      TEXT(service_name),         // name of service
      TEXT(service_displayname),  // name to display
      SERVICE_ALL_ACCESS,         // desired access
      SERVICE_WIN32_OWN_PROCESS,  // service type
      dwStartType,                // start type
      SERVICE_ERROR_NORMAL,       // error control type
      szPath,                     // service's binary
      NULL,                       // no load ordering group
      NULL,                       // no tag identifier
      TEXT(SZDEPENDENCIES),       // dependencies
      user,                       // NULL = LocalSystem account
      password);                  // NULL = no password

  if (schService) {
    OgMessageLog(DOgMlogInLog+DOgMlogDateIn,ucis_ctrl->loginfo->where,0
      ,"Service '%s' created", service_displayname);
    CloseServiceHandle(schService);
    }
  else {
    MessageErrorLog(0,ucis_ctrl->loginfo->where,0
      ,"CmdInstallService: unable to CreateService for %s", service_displayname);
    }

  CloseServiceHandle(schSCManager);
  }
else{
  MessageErrorLog(0,ucis_ctrl->loginfo->where,0
    ,"CmdInstallService: unable to OpenSCManager for %s", service_displayname);
  }
}




/*
 *  Stops and removes the service
*/

void CmdRemoveService()
{
struct og_ucis_ctrl *ucis_ctrl = &OgUcisCtrl;
char *service_name = ucis_ctrl->service_name;
char *service_displayname = ucis_ctrl->service_displayname;
SC_HANDLE   schSCManager;
SC_HANDLE   schService;
OgCreateServiceName(ucis_ctrl);

schSCManager = OpenSCManager(
                  NULL,                   // machine (NULL == local)
                  NULL,                   // database (NULL == default)
                  SC_MANAGER_ALL_ACCESS   // access required
                  );

if (schSCManager) {
  schService = OpenService(schSCManager, TEXT(service_name), SERVICE_ALL_ACCESS);
  if (schService) {
    // try to stop the service
    if (ControlService(schService, SERVICE_CONTROL_STOP, &ssStatus)) {
      Sleep(1000);
      while(QueryServiceStatus(schService, &ssStatus)) {
        if (ssStatus.dwCurrentState == SERVICE_STOP_PENDING) Sleep(1000);
        else break;
        }
      if (ssStatus.dwCurrentState == SERVICE_STOPPED )
        OgMessageLog(DOgMlogInLog,ucis_ctrl->loginfo->where,0,"Service '%s' stopped",service_displayname);
      else
        MessageErrorLog(0,ucis_ctrl->loginfo->where,0,"CmdRemoveService: failed to stop");
      }
    // now remove the service
    if(DeleteService(schService)) 
      OgMessageLog(DOgMlogInLog+DOgMlogDateIn,ucis_ctrl->loginfo->where,0
        ,"Service '%s' removed",service_displayname);
    else MessageErrorLog(0,ucis_ctrl->loginfo->where,0,"CmdRemoveService: DeleteService failed on '%s'",service_displayname);
    CloseServiceHandle(schService);
    }
  else MessageErrorLog(0,ucis_ctrl->loginfo->where,0,"CmdRemoveService: OpenService failed");
  CloseServiceHandle(schSCManager);
  }
else MessageErrorLog(0,ucis_ctrl->loginfo->where,0,"CmdRemoveService: OpenSCManager failed ");            
}




int CmdExistService()
{
struct og_ucis_ctrl *ucis_ctrl = &OgUcisCtrl;
char *service_name = ucis_ctrl->service_name;
char *service_displayname = ucis_ctrl->service_displayname;
SC_HANDLE   schSCManager;
SC_HANDLE   schService;
int exist=0;

OgCreateServiceName(ucis_ctrl);

schSCManager = OpenSCManager(
                  NULL,                   // machine (NULL == local)
                  NULL,                   // database (NULL == default)
                  SC_MANAGER_ALL_ACCESS   // access required
                  );

if (schSCManager) {
  schService = OpenService(schSCManager, TEXT(service_displayname), SERVICE_ALL_ACCESS);
  if (schService) {
    CloseServiceHandle(schService);
    exist=1;
    }
  CloseServiceHandle(schSCManager);
  }
return(exist);
}







STATICF(int) OgCreateServiceName(ucis_ctrl)
struct og_ucis_ctrl *ucis_ctrl;
{
if (ucis_ctrl->service_postfix[0]) {
  sprintf(ucis_ctrl->service_name,"%s %s",SZSERVICENAME,ucis_ctrl->service_postfix);
  }
else {
  sprintf(ucis_ctrl->service_name,"%s",SZSERVICENAME);
  }
strcpy(ucis_ctrl->service_displayname,ucis_ctrl->service_name);
DONE;
}


