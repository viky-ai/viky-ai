/*
 *  This is the meta seach server, mainly using the Nls dll.
 *  Copyright (c) 2005-2007 Pertimm by Patrick Constant
 *  Dev : May,July 2005, February 2007, November 2008
 *  Version 1.3
 */
#include "ogm_nls.h"

/** Control variables for ogm_nls.exe **/
struct og_nls_prog OgNlsProg;

static int OgUse(struct og_nls_prog *nls_prog, void *hInstance, int nCmdShow);
static int GetNlsControlVariables(struct og_nls_prog *nls_prog);
static int GetNlsConfiguration(struct og_nls_prog *nls_prog, char *conf);
static void LogErrors(struct og_nls_prog *nls_prog, char *label);

static void NlsSignalOnEmergency(void *context, int signal_type);
static void NlsSignalOnStop(void *context, int signal_type);

#define DOgNlsMaxArgc  20

int main(int argc, char *argv[])

{
  char *argv0 = argv[0];
  void *hInstance = 0;
  int nCmdShow = 0;

  og_bool must_exit = 0;
  char cmd_param[1024], cmd[4096], v[64];

  og_bool use_signal = TRUE;

  char prior_messages[DPcPathSize];
  prior_messages[0] = 0;

  struct og_nls_param param[1];
  memset(param, 0, sizeof(struct og_nls_param));
  param->loginfo.trace = DOgNlsTraceMinimal + DOgNlsTraceMemory;
  param->loginfo.where = "ogm_nls";

  struct og_nls_prog *nls_prog = &OgNlsProg;
  memset(nls_prog, 0, sizeof(struct og_nls_prog));
  nls_prog->loginfo->where = SSRV_EXE;
  nls_prog->loginfo->trace = DOgNlsTraceDefault;
  nls_prog->param = param;
  nls_prog->hmsg = OgLogInit("ogm_nls", "ogm_nls", DOgMsgTraceMinimal + DOgMsgTraceMemory, DOgMsgLogFile);
  IFN(nls_prog->hmsg) return 0;

  nls_prog->herr = OgLogGetErr(nls_prog->hmsg);
  nls_prog->hmutex = OgLogGetMutex(nls_prog->hmsg);

  param->hmsg = nls_prog->hmsg;
  param->herr = OgLogGetErr(nls_prog->hmsg);
  param->hmutex = OgLogGetMutex(nls_prog->hmsg);

  snprintf(nls_prog->WorkingDirectory, DPcPathSize, "%s", g_get_current_dir());

  snprintf(nls_prog->DirControl, DPcPathSize, "%s/%s", nls_prog->WorkingDirectory, DOgDirControl);
  IF(OgCheckOrCreateDir(nls_prog->DirControl,0,nls_prog->loginfo->where)) DoExit(nls_prog);

  char DirLog[DPcPathSize];
  snprintf(DirLog, DPcPathSize, "%s/log", nls_prog->WorkingDirectory);
  IF(OgCheckOrCreateDir(DirLog,0,nls_prog->loginfo->where)) DoExit(nls_prog);
  IF(OgSetWorkingDirLog(nls_prog->WorkingDirectory)) DoExit(nls_prog);

  /* Configuration arametres de nls*/
  strcpy(param->WorkingDirectory, nls_prog->WorkingDirectory);
  if (nls_prog->WorkingDirectory[0])
  {
    snprintf(param->configuration_file, DPcPathSize, "%s/%s", nls_prog->WorkingDirectory, DOgFileConfOgmSsi_Txt);
  }
  else
  {
    snprintf(param->configuration_file, DPcPathSize, "%s", DOgFileConfOgmSsi_Txt);
  }

  // Tune msg conf
  IFE(GetNlsConfiguration(nls_prog, param->configuration_file));

  struct og_sig_param sig_param[1];
  memset(sig_param, 0, sizeof(struct og_sig_param));
  sig_param->herr = nls_prog->herr;
  sig_param->hmutex = nls_prog->hmutex;
  sig_param->loginfo.trace = DOgSigTraceMinimal + DOgSigTraceMemory;
  sig_param->loginfo.where = nls_prog->loginfo->where;
  nls_prog->hsig = OgSigInit(sig_param);
  IFN(nls_prog->hsig) return 0;

  strcpy(cmd, "ogm_nls");
  nls_prog->pid = getpid();

  OgGetExecutablePath(argv0, nls_prog->cwd);
  chdir(nls_prog->cwd);

  for (int i = 1; i < argc; i++)
  {
    sprintf(cmd + strlen(cmd), " %s", cmd_param);
    if (!strcmp(cmd_param, "-d"))
    {
      Daemonize(nls_prog);
    }
    if (!strcmp(cmd_param, "-v"))
    {
      char banner[1024];
      banner[0] = 0;
      sprintf(banner + strlen(banner), "%s\n", DOgNlsBanner);
      sprintf(banner + strlen(banner), "lib:  %s\n", OgNlsBanner());
      OgMessageBox(0, banner, param->loginfo.where, DOgMessageBoxInformation);
      must_exit = 1;
    }
    else if (!strcmp(cmd_param, "-h"))
    {
      OgUse(nls_prog, hInstance, nCmdShow);
      must_exit = 1;
    }
    else if (!memcmp(cmd_param, "-nosignal", 9))
    {
      use_signal = FALSE;
    }
  }

  if (must_exit)
  {
    return (0);
  }

  IF(OgCheckOrCreateDir(DOgDirLog, 0, param->loginfo.where))
  {
    OgMsg(nls_prog->hmsg, "", DOgMsgDestInLog + DOgMsgDestInErr, "Can't create directory '%s'", DOgDirLog);
    return (1);
  }

  OgMsg(nls_prog->hmsg, "starting", DOgMsgDestInLog + DOgMsgParamDateIn, "\n\nProgram ogm_nls starting with pid %x",
      getpid());
  OgMsg(nls_prog->hmsg, "banner", DOgMsgDestInLog, "%s", DOgNlsBanner);
  if (prior_messages[0])
  {
    OgMsg(nls_prog->hmsg, "prior_messages", DOgMsgDestInLog + DOgMsgDestInErr, "prior_messages: \n[[%s]]",
        prior_messages);
  }

  OgMsg(nls_prog->hmsg, "cwd", DOgMsgDestInLog, "Current directory is '%s'", nls_prog->cwd);

  IF(GetNlsControlVariables(nls_prog)) DoExit(nls_prog);
  param->loginfo.trace = nls_prog->loginfo->trace;

  /* When debugging we want to avoid catching the signal to get the coredumps
   * thus we will use the -nosignal option */
  if (use_signal)
  {
    IF(OgSignal(nls_prog->hsig, SIGSEGV, NlsSignalOnEmergency, nls_prog, 0)) DoExit(nls_prog);
    IF(OgSignal(nls_prog->hsig, SIGABRT, NlsSignalOnEmergency,nls_prog, 0)) DoExit(nls_prog);

  }

  // Signal stop
  IF(OgSignal(nls_prog->hsig, SIGTERM, NlsSignalOnStop, nls_prog, 0)) DoExit(nls_prog);
  IF(OgSignal(nls_prog->hsig, SIGINT, NlsSignalOnStop, nls_prog, 0)) DoExit(nls_prog);

  // Signal timeout
  //IF(OgSignal(nls_prog->hsig, SIGUSR1, NlsSignalFunc, nls_prog, 0)) DoExit(nls_prog);

  time_t clock_start = OgClock();

  nls_prog->hnls = OgNlsInit(nls_prog->param);
  IFn(nls_prog->hnls) DoExit(nls_prog);
  OgFormatThousand((unsigned) (OgClock() - clock_start), v);
  OgMsg(nls_prog->hmsg, "", DOgMsgDestInLog, "ogm_nls initialized after %s milli-seconds", v);

  IF(OgNlsRun(nls_prog->hnls)) DoExit(nls_prog);

  IF(OgNlsFlush(nls_prog->hnls)) LogErrors(nls_prog, "OgNlsFlush");

  IF(OgFlushCriticalSection(nls_prog->hmutex)) LogErrors(nls_prog, "OgFlushCriticalSection");

  if (param->loginfo.trace & DOgNlsTraceMinimal)
  {
    time_t ltime;
    time(&ltime);
    OgMsg(nls_prog->hmsg, "", DOgMsgDestInLog, "Program %s.exe exiting at %.24s\n", param->loginfo.where,
        OgGmtime(&ltime));
  }

  OgMsgFlush(nls_prog->hmsg);
  OgSigFlush(nls_prog->hsig);
  OgErrFlush(nls_prog->herr);

  return (0);
}

static int OgUse(struct og_nls_prog *nls_prog, void *hInstance, int nCmdShow)
{
  char buffer[8192];

  sprintf(buffer, "Usage : ogm_nls [-v] [-h] [options]\n");
  sprintf(buffer + strlen(buffer), "options are:\n");
  sprintf(buffer + strlen(buffer), "  -d start as a daemon (Unix only)\n");
  sprintf(buffer + strlen(buffer), "  -nosignal: gets a core dump, instead of a log set by the exception\n");
  sprintf(buffer + strlen(buffer), "    -h prints this message\n");
  sprintf(buffer + strlen(buffer), "    -nlst_<n>: trace options for logging (default 0x%x)\n",
      nls_prog->loginfo->trace);
  sprintf(buffer + strlen(buffer), "      <n> has a combined hexadecimal value of:\n");
  sprintf(buffer + strlen(buffer), "        0x1: minimal, 0x2: memory, 0x4: all request, 0x8: answer\n");
  sprintf(buffer + strlen(buffer), "        0x10: socket, 0x20: socket size, 0x40: LT, 0x80: LT search\n");
  sprintf(buffer + strlen(buffer), "    -v gives version number of the program\n");

  OgMessageBox(0, buffer, nls_prog->loginfo->where, DOgMessageBoxInformation);

  DONE;
}

/**
 *    Gets trace level which is defined as a file in CONTROL directory
 *    named jtrace_<x>.rqs, where x is hexadecimal of variable nls_prog->loginfo->trace.
 *    Example : ftrace_1.rqs to get minimal trace.
 */
static int GetNlsControlVariables(struct og_nls_prog *nls_prog)
{
  char *stop, value[1024];
  if (OgGetVariableFile(nls_prog->loginfo->where, "nlst_", value))
  {
    int save_trace = nls_prog->loginfo->trace;
    nls_prog->loginfo->trace = strtoul(value, &stop, 16);
    if (save_trace != nls_prog->loginfo->trace)
    {
      OgMsg(nls_prog->hmsg, "", DOgMsgDestInLog, "Trace value is %x", nls_prog->loginfo->trace);
    }

    if (nls_prog->loginfo->trace & DOgNlsTraceMinimal)
    {
      OgMsg(nls_prog->hmsg, "", DOgMsgDestInLog, "  minimal trace");
    }

    if (nls_prog->loginfo->trace & DOgNlsTraceMemory)
    {
      OgMsg(nls_prog->hmsg, "", DOgMsgDestInLog, "  memory trace");
    }

    if (nls_prog->loginfo->trace & DOgNlsTraceAllRequest)
    {
      OgMsg(nls_prog->hmsg, "", DOgMsgDestInLog, "  all request trace");
    }

    if (nls_prog->loginfo->trace & DOgNlsTraceAnswer)
    {
      OgMsg(nls_prog->hmsg, "", DOgMsgDestInLog, "  answer trace");
    }

    if (nls_prog->loginfo->trace & DOgNlsTraceSocket)
    {
      OgMsg(nls_prog->hmsg, "", DOgMsgDestInLog, "  socket trace");
    }

    if (nls_prog->loginfo->trace & DOgNlsTraceSocketSize)
    {
      OgMsg(nls_prog->hmsg, "", DOgMsgDestInLog, "  socket size trace");
    }

    if (nls_prog->loginfo->trace & DOgNlsTraceLT)
    {
      OgMsg(nls_prog->hmsg, "", DOgMsgDestInLog, "  listening threads trace");
    }

    if (nls_prog->loginfo->trace & DOgNlsTraceLTSearch)
    {
      OgMsg(nls_prog->hmsg, "", DOgMsgDestInLog, "  listening threads search trace");
    }

  }
  return (0);
}

static int GetNlsConfiguration(struct og_nls_prog *nls_prog, char *conf)
{
  char value[DPcPathSize];
  int found;

  if (!OgFileExists(conf)) DONE;

  /** Since this variable concerns log tuning, we do it as soon as possible **/
  IFE(found=OgDipperConfGetVar(conf, "msg_timestamp", value, DPcPathSize));
  if (found)
  {
    OgTrimString(value, value);
    PcStrlwr(value);
    size_t msg_timestamp = FALSE;
    if (!strcmp(value, "true") || !strcmp(value, "yes")) msg_timestamp = TRUE;
    IF(OgMsgTune(nls_prog->hmsg, DOgMsgTuneShowTimestamp, (void *) msg_timestamp)) return (0);
  }

  /** Since this variable concerns log tuning, we do it as soon as possible **/
  IFE(found=OgDipperConfGetVar(conf,"msg_id",value,DPcPathSize));
  if (found)
  {
    OgTrimString(value, value);
    size_t msg_id = OgArgSize(value);
    IF(OgMsgTune(nls_prog->hmsg, DOgMsgTuneShowId, (void *) msg_id)) return (0);
  }

  /** Since this variable concerns log tuning, we do it as soon as possible **/
  IFE(found=OgDipperConfGetVar(conf,"msg_mask", value, DPcPathSize));
  if (found)
  {
    OgTrimString(value, value);
    int msg_mask;
    IFE(OgMsgMaskInterpret(nls_prog->hmsg, value, &msg_mask));

    size_t clean_msg_mask = msg_mask;
    IF(OgMsgTune(nls_prog->hmsg, DOgMsgTuneMask, (void *) clean_msg_mask)) return (0);
  }

  DONE;
}

static void NlsSignalOnStop(void *context, int signal_type)
{
  struct og_nls_prog *nls_prog = (struct og_nls_prog *) context;

  OgMsg(nls_prog->hmsg, "received_stop_signal", DOgMsgDestInLog, "Program ogm_nls received STOP signal (%d),"
      " stopping in progress ...", signal_type);

  if (nls_prog->hnls != NULL)
  {
    OgNlsOnSignalStop(nls_prog->hnls);
  }

  OgMsg(nls_prog->hmsg, "received_stop_signal", DOgMsgDestInLog, "Program ogm_nls stopped");

  exit(0);
}

static void NlsSignalOnEmergency(void *context, int signal_type)
{
  struct og_nls_prog *nls_prog = (struct og_nls_prog *) context;

  int levelFlag = DOgMsgDestInLog + DOgMsgDestInErr + DOgMsgParamDateIn;

  // log back trace
  {
    void *DPT_trace[255];
    int DPT_i = 0, DPT_trace_size = 255;
    char **DPT_messages = NULL;
    DPT_trace_size = backtrace(DPT_trace, sizeof(DPT_trace));
    DPT_messages = backtrace_symbols(DPT_trace, DPT_trace_size);

    OgLogCritical(nls_prog->hmsg, "Program ogm_nls received signal %d, Backtrace :", signal_type);

    for (DPT_i = 0; DPT_i < DPT_trace_size; DPT_i++)
    {
      OgLogCritical(nls_prog->hmsg, "\t%s", DPT_messages[DPT_i]);
    }
    OgLogCritical(nls_prog->hmsg, "\t%s\n", "...");

    free(DPT_messages);
  }

  OgMsg(nls_prog->hmsg, "received_signal", levelFlag, "Program ogm_nls received signal %d", signal_type);

  // save what we can
  if (nls_prog->hnls != NULL)
  {
    OgNlsOnSignalEmergency(nls_prog->hnls);
  }

  DoExit(nls_prog);
}

static void LogErrors(struct og_nls_prog *nls_prog, char *label)
{
  int mb = DOgMsgDestInLog + DOgMsgDestInErr;
  OgMsgErr(nls_prog->hmsg, "ogm_nls_normal_exit", 1, 0, 0, DOgMsgSeverityEmergency, 0);
  OgMsg(nls_prog->hmsg, "exiting_normally", mb, "Errors found in '%s', exiting normally", label);
}

void DoExit(struct og_nls_prog *nls_prog)
{
  int mb = DOgMsgDestInLog + DOgMsgDestInErr;
  OgMsgErr(nls_prog->hmsg, "ogm_nls_error", 1, 0, 0, DOgMsgSeverityEmergency, 0);
  OgMsg(nls_prog->hmsg, "exiting_on_error", mb, "Program ogm_nls exiting on error.");
  exit(1);
}

