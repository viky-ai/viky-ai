/*
 *  This is the meta seach server, mainly using the Ucis dll.
 *  Copyright (c) 2005 Pertimm by Patrick Constant
 *  Dev : May,July 2005
 *  Version 1.1
*/
#include "ogucis.h"



/** Control variables for ogm_ucis.exe **/
struct og_ucis_ctrl OgUcisCtrl;


STATICF(int) OgUse(pr_(struct og_ucis_ctrl *) pr_(void *) pr(int));
STATICF(int) GetUcisControlVariables(pr(struct og_ucis_ctrl *));



#if ( DPcSystem == DPcSystemUnix)
int main (argc,argv)
int argc; char *argv[];
{
char *argv0 = argv[0];
void *hInstance=0;
int nCmdShow=0;
int i;

#else
#if ( DPcSystem == DPcSystemWin32)




int WINAPI WinMain(HANDLE hInstance, HANDLE hPrevInstance, LPSTR sCmdParameters, int nCmdShow)
{
int pos=0;
char *argv0=0;
char value[1024];
int must_install=0,must_remove=0;
int retour;

SERVICE_TABLE_ENTRY dispatchTable[] = {
  { TEXT(SZSERVICENAME), (LPSERVICE_MAIN_FUNCTION)service_main },
  { NULL, NULL }
  };

#endif
#endif

int found;
int must_exit=0;
char cmd_param[1024],cmd[4096];
struct og_ucis_ctrl *ucis_ctrl = &OgUcisCtrl;
struct og_uci_server_param cparam,*param=&cparam; 
char working_directory[DPcPathSize];
int force_as_regular_program=0;
ogmutex_t mutex;
time_t ltime;

memset(ucis_ctrl,0,sizeof(struct og_ucis_ctrl));
ucis_ctrl->loginfo = &ucis_ctrl->cloginfo;
ucis_ctrl->loginfo->where = UCIS_EXE;
ucis_ctrl->loginfo->trace = DOgUciServerTraceMinimal
                          + DOgUciServerTraceMemory
                          + DOgUciServerTraceSocket
                          + DOgUciServerTraceSocketSize;
ucis_ctrl->param = param;

memset(param,0,sizeof(struct og_uci_server_param));
param->loginfo.trace = DOgUciServerTraceMinimal+DOgUciServerTraceMemory; 
param->loginfo.where = UCIS_EXE;

/** main thread error handle **/
IFn(ucis_ctrl->herr=OgErrInit()) {
  OgMessageLog(DOgMlogInLog+DOgMlogInErr,param->loginfo.where,0,"ogm_Ucis: OgErrInit error");
  return(1);
  }
param->herr=ucis_ctrl->herr;

/** general mutex handle **/
ucis_ctrl->hmutex=&mutex;
param->hmutex=ucis_ctrl->hmutex;
IF(OgInitCriticalSection(ucis_ctrl->hmutex,"ogucis")) DoExit(ucis_ctrl);

strcpy(cmd,"ogm_ucis");
ucis_ctrl->pid = getpid();

OgGetExecutablePath(argv0,ucis_ctrl->cwd); chdir(ucis_ctrl->cwd);
#if ( DPcSystem == DPcSystemWin32)
IFE(retour=OgConfGetVarLine(DOgFileOgmConf_Txt,"ServiceName",value,1024));
if (retour) strcpy(ucis_ctrl->service_postfix,value);
ucis_ctrl->is_service = CmdExistService();
if (OgGetVariableFile(ucis_ctrl->loginfo->where,"eprog",value)) {
  if (!memcmp(value,"t",1)) { 
    force_as_regular_program = 1;
    ucis_ctrl->is_service = 0;
    }
  }
#endif

found = OgConfGetVar(DOgFileOgmConf_Txt,"WorkingDirectory",working_directory,DPcPathSize,0);
IF(found) {
  OgErrLast(ucis_ctrl->herr,cmd_param,0); 
  OgMessageLog(DOgMlogInLog+DOgMlogInErr,ucis_ctrl->loginfo->where,0,"%s",cmd_param);
  found=0;
  }
if (found) {
  char DirLog[DPcPathSize];
  ucis_ctrl->WorkingDirectory = working_directory;
  if (!Ogstricmp(working_directory,DOgTempDirectory)) {
    IF(OgGetTempPath(DPcPathSize,working_directory,ucis_ctrl->herr)) DoExit(ucis_ctrl);
    sprintf(working_directory+strlen(working_directory),"pertimm");
    }
  sprintf(ucis_ctrl->DirControl,"%s/%s",ucis_ctrl->WorkingDirectory,DOgDirControl);
  IF(OgCheckOrCreateDir(ucis_ctrl->DirControl,0,ucis_ctrl->loginfo->where)) DoExit(ucis_ctrl);
  sprintf(DirLog,"%s/log",ucis_ctrl->WorkingDirectory);
  IF(OgCheckOrCreateDir(DirLog,0,ucis_ctrl->loginfo->where)) DoExit(ucis_ctrl);
  IF(OgSetWorkingDirLog(ucis_ctrl->WorkingDirectory)) DoExit(ucis_ctrl);
  }
else {
  ucis_ctrl->WorkingDirectory = 0;
  sprintf(ucis_ctrl->DirControl,"%s",DOgDirControl);
  }

/** Declare as soon as possible **/
if (!ucis_ctrl->is_service) {
  IF(OgDirDeclareNamedPid(ucis_ctrl->DirControl,0,DOgUcisName)) DoExit(ucis_ctrl);
  }

#if ( DPcSystem == DPcSystemUnix)

for (i=1; i<argc; i++) {
  strcpy(cmd_param,argv[i]);

#else
#if ( DPcSystem == DPcSystemWin32)

while(OgGetCmdParameter(sCmdParameters,cmd_param,&pos)) {
#endif
#endif

  sprintf(cmd+strlen(cmd)," %s",cmd_param);
  if (!strcmp(cmd_param,"-v")) {
    char banner[1024]; banner[0]=0;
    sprintf(banner+strlen(banner),"%s\n",DOgUciBanner);
    sprintf(banner+strlen(banner),"lib:  %s\n",OgUciBanner());
    OgMessageBox(0,banner,param->loginfo.where,DOgMessageBoxInformation);
    must_exit=1; 
    }
#if ( DPcSystem == DPcSystemWin32)
  else if (!strcmp(cmd_param,"-install")) {
    must_install=1;
    must_exit=1;
    }
  else if (!strcmp(cmd_param,"-remove")) {
    must_remove=1;
    must_exit=1;
    }
#endif
  else if (!strcmp(cmd_param,"-h")) {
    OgUse(ucis_ctrl,hInstance,nCmdShow);
    must_exit=1;
    }
  else if (!memcmp(cmd_param,"-auto",5)) {
	ucis_ctrl->service_automatic=1;    
    }
  else if (!memcmp(cmd_param,"-user=",6)) {
    strcpy(ucis_ctrl->service_user,cmd_param+6);
    }
  else if (!memcmp(cmd_param,"-password=",10)) {
    strcpy(ucis_ctrl->service_password,cmd_param+10);
    }
  }
if (must_exit) {
  #if ( DPcSystem == DPcSystemWin32)
  if (must_install) {
    CmdInstallService();
    }
  else if (must_remove) {
    CmdRemoveService();
    }
  #endif
  return(0);
  }

IF(OgCheckOrCreateDir(DOgDirLog,0,param->loginfo.where)) { 
  OgMessageLog(DOgMlogInLog+DOgMlogInErr,param->loginfo.where,0
    ,"Can't create directory '%s'",DOgDirLog);    
  return(1);
  }

OgMessageLog(DOgMlogInLog+DOgMlogDateIn,ucis_ctrl->loginfo->where,0
   ,"\n\nProgram ogm_ucis starting with pid %x", getpid());
OgMessageLog(DOgMlogInLog, ucis_ctrl->loginfo->where, 0, "%s",DOgUciBanner);
if (ucis_ctrl->service_postfix[0]) {
  OgMessageLog(DOgMlogInLog, ucis_ctrl->loginfo->where, 0, "ServiceName is: '%s'",ucis_ctrl->service_postfix);
  }
OgMessageLog(DOgMlogInLog, ucis_ctrl->loginfo->where, 0, "Current directory is '%s'",ucis_ctrl->cwd);

if (force_as_regular_program) {
  OgMessageLog(DOgMlogInLog,ucis_ctrl->loginfo->where,0
    ,"Forcing program to start as a regular program and not as a service");
  }

IFE(GetUcisControlVariables(ucis_ctrl));
param->loginfo.trace = ucis_ctrl->loginfo->trace;

IFn(ucis_ctrl->hucis=OgUciServerInit(ucis_ctrl->param))  DoExit(ucis_ctrl);

#if ( DPcSystem == DPcSystemUnix)

IF(UcisRun(ucis_ctrl)) DoExit(ucis_ctrl);

#else
#if ( DPcSystem == DPcSystemWin32)

/** This function cannot work under Windows 9x, but we don't care **/
if (ucis_ctrl->is_service) {
  OgMessageLog(DOgMlogInLog,ucis_ctrl->loginfo->where,0
     ,"ogm_ucis starting as a service");
  if (!StartServiceCtrlDispatcher(dispatchTable)) {
    char erreur[DOgErrorSize];
    sprintf(erreur,"Program %s.exe could not start service '%s'"
      ,ucis_ctrl->loginfo->where, SZSERVICEDISPLAYNAME);
    MessageErrorLog(0,ucis_ctrl->loginfo->where,0,erreur);
    AddToMessageLog(TEXT(erreur));
    }
  }
else {
  OgMessageLog(DOgMlogInLog,ucis_ctrl->loginfo->where,0
     ,"ogm_ucis starting as regular program");
  IF(UcisRun(ucis_ctrl)) DoExit(ucis_ctrl);
  }

#endif
#endif

if (!ucis_ctrl->is_service) {
  IF(OgDirCleanNamedPid(ucis_ctrl->DirControl,DOgUcisName)) DoExit(ucis_ctrl);
  }

IF(OgUciServerFlush(ucis_ctrl->hucis)) DoExit(ucis_ctrl);

IF(OgFlushCriticalSection(ucis_ctrl->hmutex)) DoExit(ucis_ctrl);
OgErrFlush(ucis_ctrl->herr);

if (param->loginfo.trace&DOgUciServerTraceMinimal) {
  time(&ltime);
  OgMessageLog(DOgMlogInLog,param->loginfo.where,0
    ,"\nProgram %s.exe exiting at %.24s\n",param->loginfo.where,OgGmtime(&ltime));
  }

return(0);
}





STATICF(int) OgUse(ucis_ctrl,hInstance,nCmdShow)
struct og_ucis_ctrl *ucis_ctrl;
void *hInstance;
int nCmdShow;
{
char buffer[8192],edit_buffer[8192];
#if ( DPcSystem == DPcSystemWin32)
int j,k;
#endif

sprintf(buffer,               "Usage : ogucis [-v] [-h] [options]\n");
sprintf(buffer+strlen(buffer),"options are:\n");
sprintf(buffer+strlen(buffer),"  -install installs the programme as a service\n");
sprintf(buffer+strlen(buffer),"      -user=<user> service is installed as <user> e.g.: admin@example.com\n");
sprintf(buffer+strlen(buffer),"      -password=<password> password for <user>\n");
sprintf(buffer+strlen(buffer),"      -auto service is installed in automatic starting mode\n");
sprintf(buffer+strlen(buffer),"  -m<mode> metagrabbing emulation mode, values are:\n");
sprintf(buffer+strlen(buffer),"      1: read emulation mode (reads data from disk)\n");
sprintf(buffer+strlen(buffer),"      2: write emulation mode (writes data to disk)\n");
sprintf(buffer+strlen(buffer),"  -remove removes the programme as a service\n");
sprintf(buffer+strlen(buffer),"    -h prints this message\n");
sprintf(buffer+strlen(buffer),"    -eprog_<b> forces program to start as a program even if installed as a service\n");
sprintf(buffer+strlen(buffer),"    -ucist_<n>: trace options for logging (default 0x%x)\n",ucis_ctrl->loginfo->trace);
sprintf(buffer+strlen(buffer),"      <n> has a combined hexadecimal value of:\n");
sprintf(buffer+strlen(buffer),"      0x1: minimal, 0x2: memory, 0x4: socket, 0x8: socket size\n");
sprintf(buffer+strlen(buffer),"    -v gives version number of the program\n");

#if ( DPcSystem == DPcSystemWin32)
for (j=k=0; buffer[j]; j++) {
  if (buffer[j]=='\n') { edit_buffer[k++]='\r'; edit_buffer[k++]='\n'; }
  else edit_buffer[k++]=buffer[j];
  }
edit_buffer[k-2]=0; /* removing last CR */
OgEditBox(hInstance,nCmdShow,"ogucis help",edit_buffer,480,260);
#else
OgMessageBox(0,buffer,ucis_ctrl->loginfo->where,DOgMessageBoxInformation);
#endif

DONE;
}





/*
 *    Gets trace level which is defined as a file in CONTROL directory
 *    named jtrace_<x>.rqs, where x is hexadecimal of variable ucis_ctrl->loginfo->trace.
 *    Example : ftrace_1.rqs to get minimal trace.
*/

STATICF(int) GetUcisControlVariables(ucis_ctrl)
struct og_ucis_ctrl *ucis_ctrl;
{
char *stop,value[1024];
if (OgGetVariableFile(ucis_ctrl->loginfo->where,"ucist_",value)) {
  int save_trace=ucis_ctrl->loginfo->trace;
  ucis_ctrl->loginfo->trace=strtol(value,&stop,16);
  if (save_trace!=ucis_ctrl->loginfo->trace) {
    OgMessageLog(DOgMlogInLog,ucis_ctrl->loginfo->where,0,"Trace value is %x",ucis_ctrl->loginfo->trace);
    if (ucis_ctrl->loginfo->trace&DOgUciServerTraceMinimal)
      OgMessageLog(DOgMlogInLog,ucis_ctrl->loginfo->where,0,"  minimal trace");
    if (ucis_ctrl->loginfo->trace&DOgUciServerTraceMemory)
      OgMessageLog(DOgMlogInLog,ucis_ctrl->loginfo->where,0,"  memory trace");
    if (ucis_ctrl->loginfo->trace&DOgUciServerTraceSocket)
      OgMessageLog(DOgMlogInLog,ucis_ctrl->loginfo->where,0,"  socket trace");
    if (ucis_ctrl->loginfo->trace&DOgUciServerTraceSocketSize)
      OgMessageLog(DOgMlogInLog,ucis_ctrl->loginfo->where,0,"  socket size trace");
    }
  }
return(0);
}




void DoExit(ucis_ctrl)
struct og_ucis_ctrl *ucis_ctrl;
{
int is_error;
time_t ltime;
is_error=OgErrLog(ucis_ctrl->herr,ucis_ctrl->loginfo->where,0,0,0,0);
time(&ltime);
OgMessageLog(DOgMlogInLog,ucis_ctrl->param->loginfo.where,0
  ,"\nProgram %s.exe exiting on error at %.24s\n",ucis_ctrl->param->loginfo.where,OgGmtime(&ltime));
exit(1);
}


