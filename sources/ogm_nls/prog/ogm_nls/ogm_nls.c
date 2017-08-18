/*
 *  This is the meta seach server, mainly using the Nls dll.
 *  Copyright (c) 2005-2007 Pertimm by Patrick Constant
 *  Dev : May,July 2005, February 2007, November 2008
 *  Version 1.3
*/
#include "ogm_nls.h"



/** Control variables for ogm_nls.exe **/
struct og_nls_ctrl OgNlsCtrl;


static int OgUse(struct og_nls_ctrl *nls_ctrl,void *hInstance,int nCmdShow);
static int GetNlsControlVariables(struct og_nls_ctrl *nls_ctrl);
static int GetNlsConfiguration(struct og_nls_ctrl *nls_ctrl,char *conf);
static void NlsSignalFunc(void *context, int signal_type);
static void LogErrors(struct og_nls_ctrl *nls_ctrl, char *label);

#define DOgNlsMaxArgc  20



int main (argc,argv)
int argc; char *argv[];
{
char *argv0 = argv[0];
void *hInstance=0;
int nCmdShow=0;

int i;
int found;
int must_exit=0;
char cmd_param[1024],cmd[4096],v[64];
struct og_nls_ctrl *nls_ctrl = &OgNlsCtrl;
struct og_sig_param csig_param,*sig_param=&csig_param;
char working_directory[DPcPathSize],*DOgPIPE;
struct og_nls_param cparam,*param=&cparam;
char *plugin_argv[DOgNlsMaxArgc];
char prior_messages[DPcPathSize];
int force_as_regular_program=0;
time_t ltime,clock_start;
int use_signal=1;

prior_messages[0]=0;
memset(nls_ctrl,0,sizeof(struct og_nls_ctrl));
nls_ctrl->loginfo = &nls_ctrl->cloginfo;
nls_ctrl->loginfo->where = SSRV_EXE;
nls_ctrl->loginfo->trace = DOgNlsTraceDefault;
nls_ctrl->param = param;

memset(param,0,sizeof(struct og_nls_param));
param->loginfo.trace = DOgNlsTraceMinimal+DOgNlsTraceMemory;
param->loginfo.where = "ogm_nls";

/* Si la variable d'environnement DOgPIPE est d�finie, on utilise le mode de log en pipe*/
if((DOgPIPE=getenv("DOgPIPE"))) {
  IFn(nls_ctrl->hmsg=OgLogInit("ogm_nls",DOgPIPE, DOgMsgTraceMinimal+DOgMsgTraceMemory, DOgMsgLogPipe)) return(0);
  }
else {
  IFn(nls_ctrl->hmsg=OgLogInit("ogm_nls","ogm_nls", DOgMsgTraceMinimal+DOgMsgTraceMemory, DOgMsgLogFile)) return(0);
  }
param->hmsg=nls_ctrl->hmsg;
param->herr= OgLogGetErr(nls_ctrl->hmsg);
param->hmutex=OgLogGetMutex(nls_ctrl->hmsg);

nls_ctrl->herr = OgLogGetErr(nls_ctrl->hmsg);
nls_ctrl->hmutex = OgLogGetMutex(nls_ctrl->hmsg);


found = OgConfGetWorkingDirectory(working_directory,DPcPathSize);
IF(found) {
  PcErrLast(-1,cmd_param);
  sprintf(prior_messages+strlen(prior_messages),"OgConfGetWorkingDirectory error: %s\n",cmd_param);
  found=0;
  }
if (found) {
  char DirLog[DPcPathSize];
  strcpy(nls_ctrl->WorkingDirectory,working_directory);
  if (!Ogstricmp(working_directory,DOgTempDirectory)) {
    IF(OgGetTempPath(DPcPathSize,working_directory,nls_ctrl->herr)) DoExit(nls_ctrl);
    sprintf(working_directory+strlen(working_directory),"pertimm");
    }
  sprintf(nls_ctrl->DirControl,"%s/%s",nls_ctrl->WorkingDirectory,DOgDirControl);
  IF(OgCheckOrCreateDir(nls_ctrl->DirControl,0,nls_ctrl->loginfo->where)) DoExit(nls_ctrl);
  sprintf(DirLog,"%s/log",nls_ctrl->WorkingDirectory);
  IF(OgCheckOrCreateDir(DirLog,0,nls_ctrl->loginfo->where)) DoExit(nls_ctrl);
  IF(OgSetWorkingDirLog(nls_ctrl->WorkingDirectory)) DoExit(nls_ctrl);
  }
else {
  nls_ctrl->WorkingDirectory[0] = 0;
  sprintf(nls_ctrl->DirControl,"%s",DOgDirControl);
  }

strcpy(param->WorkingDirectory,nls_ctrl->WorkingDirectory);

/* personnalisation du log avec des parametres de nls*/
if(nls_ctrl->WorkingDirectory[0]) sprintf(param->configuration_file,"%s/%s",nls_ctrl->WorkingDirectory,DOgFileConfOgmSsi_Txt);
else strcpy(param->configuration_file,DOgFileConfOgmSsi_Txt);

IFE(GetNlsConfiguration(nls_ctrl,param->configuration_file));

memset(sig_param,0,sizeof(struct og_sig_param));
sig_param->herr=nls_ctrl->herr;
sig_param->hmutex=nls_ctrl->hmutex;
sig_param->loginfo.trace = DOgSigTraceMinimal+DOgSigTraceMemory;
sig_param->loginfo.where = nls_ctrl->loginfo->where;
IFn(nls_ctrl->hsig=OgSigInit(sig_param)) return(0);

strcpy(cmd,"ogm_nls");
nls_ctrl->pid = getpid();

OgGetExecutablePath(argv0,nls_ctrl->cwd); chdir(nls_ctrl->cwd);

memset(plugin_argv, 0, sizeof plugin_argv);
nls_ctrl->param->argv = plugin_argv;

nls_ctrl->param->argc = argc < DOgNlsMaxArgc ? argc : DOgNlsMaxArgc;

for (i=1; i<argc; i++) {
  strcpy(cmd_param,argv[i]);
  if (i < DOgNlsMaxArgc)
    plugin_argv[i] = argv[i];

  sprintf(cmd+strlen(cmd)," %s",cmd_param);
  if (!strcmp(cmd_param,"-d")) {
    Daemonize(nls_ctrl);
    }
  if (!strcmp(cmd_param,"-v")) {
    char banner[1024]; banner[0]=0;
    sprintf(banner+strlen(banner),"%s\n",DOgNlsBanner);
    sprintf(banner+strlen(banner),"lib:  %s\n",OgNlsBanner());
    OgMessageBox(0,banner,param->loginfo.where,DOgMessageBoxInformation);
    must_exit=1;
    }
  else if (!strcmp(cmd_param,"-h")) {
    OgUse(nls_ctrl,hInstance,nCmdShow);
    must_exit=1;
    }
  else if (!memcmp(cmd_param,"-auto",5)) {
  nls_ctrl->service_automatic=1;
    }
  else if (!memcmp(cmd_param,"-user=",6)) {
    strcpy(nls_ctrl->service_user,cmd_param+6);
    }
  else if (!memcmp(cmd_param,"-password=",10)) {
    strcpy(nls_ctrl->service_password,cmd_param+10);
    }
  else if (!memcmp(cmd_param,"-nosignal",9)) {
    use_signal=0;
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
  OgMsg(nls_ctrl->hmsg,"",DOgMsgDestInLog+DOgMsgDestInErr
    ,"Can't create directory '%s'",DOgDirLog);
  return(1);
  }

OgMsg(nls_ctrl->hmsg,"starting",DOgMsgDestInLog+DOgMsgParamDateIn
   ,"\n\nProgram ogm_nls starting with pid %x", getpid());
OgMsg(nls_ctrl->hmsg,"banner",DOgMsgDestInLog, "%s",DOgNlsBanner);
if (prior_messages[0]) {
  OgMsg(nls_ctrl->hmsg,"prior_messages",DOgMsgDestInLog+DOgMsgDestInErr, "prior_messages: \n[[%s]]",prior_messages);
  }
if (nls_ctrl->service_postfix[0]) {
  OgMsg(nls_ctrl->hmsg,"service_name",DOgMsgDestInLog, "ServiceName is: '%s'",nls_ctrl->service_postfix);
  }
OgMsg(nls_ctrl->hmsg,"cwd",DOgMsgDestInLog, "Current directory is '%s'",nls_ctrl->cwd);

if (force_as_regular_program) {
  OgMsg(nls_ctrl->hmsg,"",DOgMsgDestInLog
    ,"Forcing program to start as a regular program and not as a service");
  }

IF(GetNlsControlVariables(nls_ctrl)) DoExit(nls_ctrl);
param->loginfo.trace = nls_ctrl->loginfo->trace;

/* When debugging we want to avoid catching the signal to get the coredumps
 * thus we will use the -nosignal option */
if (use_signal) {
  IF(OgSignal(nls_ctrl->hsig,SIGTERM,NlsSignalFunc,nls_ctrl,0)) DoExit(nls_ctrl);
  IF(OgSignal(nls_ctrl->hsig,SIGINT,NlsSignalFunc,nls_ctrl,0)) DoExit(nls_ctrl);
  IF(OgSignal(nls_ctrl->hsig,SIGSEGV,NlsSignalFunc,nls_ctrl,0)) DoExit(nls_ctrl);
  IF(OgSignal(nls_ctrl->hsig,SIGABRT,NlsSignalFunc,nls_ctrl,0)) DoExit(nls_ctrl);
  IF(OgSignal(nls_ctrl->hsig,SIGUSR1,NlsSignalFunc,nls_ctrl,0)) DoExit(nls_ctrl);
  }

clock_start=OgClock();
/* We use &nls_ctrl->hnls so that this variable is instanciated
 * so that the status file is written correctly at any moment (signal function) */
IFn(OgNlsInit(nls_ctrl->param,&nls_ctrl->hnls))  DoExit(nls_ctrl);
OgFormatThousand((unsigned)(OgClock()-clock_start),v);
OgMsg(nls_ctrl->hmsg,"",DOgMsgDestInLog
  ,"ogm_nls initialized after %s milli-seconds",v);

IF(OgNlsRun(nls_ctrl->hnls)) DoExit(nls_ctrl);

IF(OgNlsFlush(nls_ctrl->hnls)) LogErrors(nls_ctrl,"OgNlsFlush");

IF(OgFlushCriticalSection(nls_ctrl->hmutex)) LogErrors(nls_ctrl,"OgFlushCriticalSection");

if (param->loginfo.trace&DOgNlsTraceMinimal) {
  time(&ltime);
  OgMsg(nls_ctrl->hmsg,"",DOgMsgDestInLog
    ,"Program %s.exe exiting at %.24s\n",param->loginfo.where,OgGmtime(&ltime));
  }

OgMsgFlush(nls_ctrl->hmsg);
OgSigFlush(nls_ctrl->hsig);
OgErrFlush(nls_ctrl->herr);

return(0);
}





static int OgUse(struct og_nls_ctrl *nls_ctrl,void *hInstance,int nCmdShow)
{
char buffer[8192];

sprintf(buffer,               "Usage : ogm_nls [-v] [-h] [options]\n");
sprintf(buffer+strlen(buffer),"options are:\n");
sprintf(buffer+strlen(buffer),"  -install installs the programme as a service\n");
sprintf(buffer+strlen(buffer),"      -user=<user> service is installed as <user> e.g.: admin@example.com\n");
sprintf(buffer+strlen(buffer),"      -password=<password> password for <user>\n");
sprintf(buffer+strlen(buffer),"      -auto service is installed in automatic starting mode\n");
sprintf(buffer+strlen(buffer),"  -d start as a daemon (Unix only)\n");
sprintf(buffer+strlen(buffer),"  -nosignal: gets a core dump, instead of a log set by the exception\n");
sprintf(buffer+strlen(buffer),"  -m<mode> metagrabbing emulation mode, values are:\n");
sprintf(buffer+strlen(buffer),"      1: read emulation mode (reads data from disk)\n");
sprintf(buffer+strlen(buffer),"      2: write emulation mode (writes data to disk)\n");
sprintf(buffer+strlen(buffer),"  -prof enables profiling for the search threads\n");
sprintf(buffer+strlen(buffer),"  -remove removes the programme as a service\n");
sprintf(buffer+strlen(buffer),"  -ssi=<ssi_dir> uses this specific ssi directory\n");
sprintf(buffer+strlen(buffer),"  -import=<import_dir> uses this specific import directory\n");
sprintf(buffer+strlen(buffer),"    -h prints this message\n");
sprintf(buffer+strlen(buffer),"    -eprog_<b> forces program to start as a program even if installed as a service\n");
sprintf(buffer+strlen(buffer),"    -nlst_<n>: trace options for logging (default 0x%x)\n",nls_ctrl->loginfo->trace);
sprintf(buffer+strlen(buffer),"      <n> has a combined hexadecimal value of:\n");
sprintf(buffer+strlen(buffer),"        0x1: minimal, 0x2: memory, 0x4: all request, 0x8: answer\n");
sprintf(buffer+strlen(buffer),"        0x10: socket, 0x20: socket size, 0x40: listening thread (LT), 0x80: LT search\n");
sprintf(buffer+strlen(buffer),"        0x100: indexing thread (IT), 0x200: IT answer, 0x400: leaf, 0x800: leaf detail\n");
sprintf(buffer+strlen(buffer),"        0x1000: node, 0x2000: node detail, 0x4000: attval sort, 0x8000: attval sort detail\n");
sprintf(buffer+strlen(buffer),"        0x10000: attval sort add item, 0x20000: feed, 0x40000: feed detail\n");
sprintf(buffer+strlen(buffer),"        0x100000: sync on repository, 0x200000: code, 0x400000: cache, 0x800000: ica\n");
sprintf(buffer+strlen(buffer),"        0x1000000: rich, 0x200000: miz\n");
sprintf(buffer+strlen(buffer),"        0x10000000: plugin minimal, 0x20000000: plugin memory\n");
sprintf(buffer+strlen(buffer),"        0x40000000: plugin interpretation, 0x80000000: plugin import\n");
sprintf(buffer+strlen(buffer),"    -v gives version number of the program\n");

OgMessageBox(0,buffer,nls_ctrl->loginfo->where,DOgMessageBoxInformation);

DONE;
}





/*
 *    Gets trace level which is defined as a file in CONTROL directory
 *    named jtrace_<x>.rqs, where x is hexadecimal of variable nls_ctrl->loginfo->trace.
 *    Example : ftrace_1.rqs to get minimal trace.
*/

static int GetNlsControlVariables(struct og_nls_ctrl *nls_ctrl)
{
char *stop,value[1024];
if (OgGetVariableFile(nls_ctrl->loginfo->where,"nlst_",value)) {
  int save_trace=nls_ctrl->loginfo->trace;
  nls_ctrl->loginfo->trace=strtoul(value,&stop,16);
  if (save_trace!=nls_ctrl->loginfo->trace) {
    OgMsg(nls_ctrl->hmsg,"",DOgMsgDestInLog,"Trace value is %x",nls_ctrl->loginfo->trace);
    if (nls_ctrl->loginfo->trace&DOgNlsTraceMinimal)
      OgMsg(nls_ctrl->hmsg,"",DOgMsgDestInLog,"  minimal trace");
    if (nls_ctrl->loginfo->trace&DOgNlsTraceMemory)
      OgMsg(nls_ctrl->hmsg,"",DOgMsgDestInLog,"  memory trace");
    if (nls_ctrl->loginfo->trace&DOgNlsTraceAllRequest)
      OgMsg(nls_ctrl->hmsg,"",DOgMsgDestInLog,"  all request trace");
    if (nls_ctrl->loginfo->trace&DOgNlsTraceAnswer)
      OgMsg(nls_ctrl->hmsg,"",DOgMsgDestInLog,"  answer trace");
    if (nls_ctrl->loginfo->trace&DOgNlsTraceSocket)
      OgMsg(nls_ctrl->hmsg,"",DOgMsgDestInLog,"  socket trace");
    if (nls_ctrl->loginfo->trace&DOgNlsTraceSocketSize)
      OgMsg(nls_ctrl->hmsg,"",DOgMsgDestInLog,"  socket size trace");
    if (nls_ctrl->loginfo->trace&DOgNlsTraceLT)
      OgMsg(nls_ctrl->hmsg,"",DOgMsgDestInLog,"  listening threads trace");
    if (nls_ctrl->loginfo->trace&DOgNlsTraceLTSearch)
      OgMsg(nls_ctrl->hmsg,"",DOgMsgDestInLog,"  listening threads search trace");
    }
  }
return(0);
}





static int GetNlsConfiguration(struct og_nls_ctrl *nls_ctrl,char *conf)
{
int msg_timestamp,msg_id,msg_mask;
char value[DPcPathSize];
int found;

if (!OgFileExists(conf)) DONE;

/** Since this variable concerns log tuning, we do it as soon as possible **/
IFE(found=OgDipperConfGetVar(conf,"msg_timestamp",value,DPcPathSize));
if (found) {
  OgTrimString(value,value); PcStrlwr(value);
  if (!strcmp(value,"true") || !strcmp(value,"yes"))  msg_timestamp=1;
  else msg_timestamp = 0;
  IF(OgMsgTune(nls_ctrl->hmsg,DOgMsgTuneShowTimestamp,(void *)msg_timestamp)) return(0);
  }

/** Since this variable concerns log tuning, we do it as soon as possible **/
IFE(found=OgDipperConfGetVar(conf,"msg_id",value,DPcPathSize));
if (found) {
  OgTrimString(value,value); msg_id=OgArgSize(value);
  IF(OgMsgTune(nls_ctrl->hmsg,DOgMsgTuneShowId,(void *)msg_id)) return(0);
  }

/** Since this variable concerns log tuning, we do it as soon as possible **/
IFE(found=OgDipperConfGetVar(conf,"msg_mask",value,DPcPathSize));
if (found) {
  OgTrimString(value,value);
  IFE(OgMsgMaskInterpret(nls_ctrl->hmsg,value,&msg_mask));
  IF(OgMsgTune(nls_ctrl->hmsg,DOgMsgTuneMask,(void *)msg_mask)) return(0);
  }

DONE;
}


static void NlsSignalFunc(void *context, int signal_type)
{
  struct og_nls_ctrl *nls_ctrl = (struct og_nls_ctrl *) context;

  int levelFlag = DOgMsgDestInLog + DOgMsgDestInErr + DOgMsgParamDateIn;
  if (!nls_ctrl->is_service) levelFlag += DOgMsgDestMBox;

  // log back trace
  {
    void *DPT_trace[255];
    int DPT_i = 0, DPT_trace_size = 255;
    char **DPT_messages = NULL;
    DPT_trace_size = backtrace(DPT_trace, sizeof(DPT_trace));
    DPT_messages = backtrace_symbols(DPT_trace, DPT_trace_size);

    OgLogCritical(nls_ctrl->hmsg, "Program ogm_nls received signal %d, Backtrace :", signal_type);

    for (DPT_i = 0; DPT_i < DPT_trace_size; DPT_i++)
    {
      OgLogCritical(nls_ctrl->hmsg, "\t%s", DPT_messages[DPT_i]);
    }
    OgLogCritical(nls_ctrl->hmsg, "\t%s\n", "...");

    free(DPT_messages);
  }

  OgMsg(nls_ctrl->hmsg, "received_signal", levelFlag, "Program ogm_nls received signal %d", signal_type);

  // save what we can
  IFx(nls_ctrl->hnls) OgNlsOnSignal(nls_ctrl->hnls);

  if(signal_type != SIGUSR1)
  {
    DoExit(nls_ctrl);
  }
}




static void LogErrors(struct og_nls_ctrl *nls_ctrl, char *label)
{
int mb=DOgMsgDestInLog+DOgMsgDestInErr;
OgMsgErr(nls_ctrl->hmsg,"ogm_nls_normal_exit",0,0,0,DOgMsgSeverityEmergency,0);
OgMsg(nls_ctrl->hmsg,"exiting_normally",mb,"Errors found in '%s', exiting normally",label);
}



void DoExit(nls_ctrl)
struct og_nls_ctrl *nls_ctrl;
{
int mb=DOgMsgDestInLog+DOgMsgDestInErr;
if (!nls_ctrl->is_service) mb+=DOgMsgDestMBox;
OgMsgErr(nls_ctrl->hmsg,"ogm_nls_error",0,0,0,DOgMsgSeverityEmergency,0);
OgMsg(nls_ctrl->hmsg,"exiting_on_error",mb,"Program ogm_nls exiting on error.");
exit(1);
}


