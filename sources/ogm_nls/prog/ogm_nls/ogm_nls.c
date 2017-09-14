/*
 *  This is the meta seach server, mainly using the Nls dll.
 *  Copyright (c) 2005-2007 Pertimm by Patrick Constant
 *  Dev : May,July 2005, February 2007, November 2008
 *  Version 1.3
 */
#include "ogm_nls.h"

/** Control variables for ogm_nls.exe **/
struct og_nls_ctrl OgNlsCtrl;

static int OgUse(struct og_nls_ctrl *nls_ctrl, void *hInstance, int nCmdShow);
static int GetNlsControlVariables(struct og_nls_ctrl *nls_ctrl);
static int GetNlsConfiguration(struct og_nls_ctrl *nls_ctrl, char *conf);
static void NlsSignalFunc(void *context, int signal_type);
static void LogErrors(struct og_nls_ctrl *nls_ctrl, char *label);

#define DOgNlsMaxArgc  20

int main(argc, argv)
  int argc;char *argv[];
{
  char *argv0 = argv[0];
  void *hInstance = 0;
  int nCmdShow = 0;

  og_bool must_exit = 0;
  char cmd_param[1024], cmd[4096], v[64];

  time_t ltime, clock_start;
  og_bool use_signal = TRUE;

  char prior_messages[DPcPathSize];
  prior_messages[0] = 0;

  struct og_nls_param param[1];
  memset(param, 0, sizeof(struct og_nls_param));
  param->loginfo.trace = DOgNlsTraceMinimal + DOgNlsTraceMemory;
  param->loginfo.where = "ogm_nls";

  struct og_nls_ctrl *nls_ctrl = &OgNlsCtrl;
  memset(nls_ctrl, 0, sizeof(struct og_nls_ctrl));
  nls_ctrl->loginfo->where = SSRV_EXE;
  nls_ctrl->loginfo->trace = DOgNlsTraceDefault;
  nls_ctrl->param = param;
  nls_ctrl->hmsg = OgLogInit("ogm_nls", "ogm_nls", DOgMsgTraceMinimal + DOgMsgTraceMemory, DOgMsgLogFile);
  IFN(nls_ctrl->hmsg) return 0;

  nls_ctrl->herr = OgLogGetErr(nls_ctrl->hmsg);
  nls_ctrl->hmutex = OgLogGetMutex(nls_ctrl->hmsg);

  param->hmsg = nls_ctrl->hmsg;
  param->herr = OgLogGetErr(nls_ctrl->hmsg);
  param->hmutex = OgLogGetMutex(nls_ctrl->hmsg);

  snprintf(nls_ctrl->WorkingDirectory, DPcPathSize, "%s", g_get_current_dir());

  snprintf(nls_ctrl->DirControl, DPcPathSize, "%s/%s", nls_ctrl->WorkingDirectory, DOgDirControl);
  IF(OgCheckOrCreateDir(nls_ctrl->DirControl,0,nls_ctrl->loginfo->where)) DoExit(nls_ctrl);

  char DirLog[DPcPathSize];
  snprintf(DirLog, DPcPathSize, "%s/log", nls_ctrl->WorkingDirectory);
  IF(OgCheckOrCreateDir(DirLog,0,nls_ctrl->loginfo->where)) DoExit(nls_ctrl);
  IF(OgSetWorkingDirLog(nls_ctrl->WorkingDirectory)) DoExit(nls_ctrl);

  /* Configuration arametres de nls*/
  strcpy(param->WorkingDirectory, nls_ctrl->WorkingDirectory);
  if (nls_ctrl->WorkingDirectory[0])
  {
    snprintf(param->configuration_file, DPcPathSize, "%s/%s", nls_ctrl->WorkingDirectory, DOgFileConfOgmSsi_Txt);
  }
  else
  {
    snprintf(param->configuration_file, DPcPathSize, "%s", DOgFileConfOgmSsi_Txt);
  }

  // Tune msg conf
  IFE(GetNlsConfiguration(nls_ctrl, param->configuration_file));

  struct og_sig_param sig_param[1];
  memset(sig_param, 0, sizeof(struct og_sig_param));
  sig_param->herr = nls_ctrl->herr;
  sig_param->hmutex = nls_ctrl->hmutex;
  sig_param->loginfo.trace = DOgSigTraceMinimal + DOgSigTraceMemory;
  sig_param->loginfo.where = nls_ctrl->loginfo->where;
  nls_ctrl->hsig = OgSigInit(sig_param);
  IFN(nls_ctrl->hsig) return 0;

  strcpy(cmd, "ogm_nls");
  nls_ctrl->pid = getpid();

  OgGetExecutablePath(argv0, nls_ctrl->cwd);
  chdir(nls_ctrl->cwd);

  char *plugin_argv[DOgNlsMaxArgc];
  memset(plugin_argv, 0, sizeof plugin_argv);
  nls_ctrl->param->argv = plugin_argv;
  nls_ctrl->param->argc = argc < DOgNlsMaxArgc ? argc : DOgNlsMaxArgc;

  for (int i = 1; i < argc; i++)
  {
    strcpy(cmd_param, argv[i]);
    if (i < DOgNlsMaxArgc) plugin_argv[i] = argv[i];

    sprintf(cmd + strlen(cmd), " %s", cmd_param);
    if (!strcmp(cmd_param, "-d"))
    {
      Daemonize(nls_ctrl);
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
      OgUse(nls_ctrl, hInstance, nCmdShow);
      must_exit = 1;
    }
    else if (!memcmp(cmd_param, "-nosignal", 9))
    {
      use_signal = 0;
    }
  }

  if (must_exit)
  {
    return (0);
  }

  IF(OgCheckOrCreateDir(DOgDirLog, 0, param->loginfo.where))
  {
    OgMsg(nls_ctrl->hmsg, "", DOgMsgDestInLog + DOgMsgDestInErr, "Can't create directory '%s'", DOgDirLog);
    return (1);
  }

  OgMsg(nls_ctrl->hmsg, "starting", DOgMsgDestInLog + DOgMsgParamDateIn, "\n\nProgram ogm_nls starting with pid %x",
      getpid());
  OgMsg(nls_ctrl->hmsg, "banner", DOgMsgDestInLog, "%s", DOgNlsBanner);
  if (prior_messages[0])
  {
    OgMsg(nls_ctrl->hmsg, "prior_messages", DOgMsgDestInLog + DOgMsgDestInErr, "prior_messages: \n[[%s]]",
        prior_messages);
  }

  OgMsg(nls_ctrl->hmsg, "cwd", DOgMsgDestInLog, "Current directory is '%s'", nls_ctrl->cwd);

  IF(GetNlsControlVariables(nls_ctrl)) DoExit(nls_ctrl);
  param->loginfo.trace = nls_ctrl->loginfo->trace;

  /* When debugging we want to avoid catching the signal to get the coredumps
   * thus we will use the -nosignal option */
  if (use_signal)
  {
    IF(OgSignal(nls_ctrl->hsig, SIGTERM, NlsSignalFunc, nls_ctrl, 0)) DoExit(nls_ctrl);
    IF(OgSignal(nls_ctrl->hsig, SIGINT, NlsSignalFunc, nls_ctrl, 0)) DoExit(nls_ctrl);
    IF(OgSignal(nls_ctrl->hsig, SIGSEGV, NlsSignalFunc, nls_ctrl, 0)) DoExit(nls_ctrl);
    IF(OgSignal(nls_ctrl->hsig, SIGABRT, NlsSignalFunc,nls_ctrl, 0)) DoExit(nls_ctrl);
    IF(OgSignal(nls_ctrl->hsig, SIGUSR1, NlsSignalFunc, nls_ctrl, 0)) DoExit(nls_ctrl);
  }

  clock_start = OgClock();
  /* We use &nls_ctrl->hnls so that this variable is instanciated
   * so that the status file is written correctly at any moment (signal function) */
  IFn(OgNlsInit(nls_ctrl->param,&nls_ctrl->hnls)) DoExit(nls_ctrl);
  OgFormatThousand((unsigned) (OgClock() - clock_start), v);
  OgMsg(nls_ctrl->hmsg, "", DOgMsgDestInLog, "ogm_nls initialized after %s milli-seconds", v);

  IF(OgNlsRun(nls_ctrl->hnls)) DoExit(nls_ctrl);

  IF(OgNlsFlush(nls_ctrl->hnls)) LogErrors(nls_ctrl, "OgNlsFlush");

  IF(OgFlushCriticalSection(nls_ctrl->hmutex)) LogErrors(nls_ctrl, "OgFlushCriticalSection");

  if (param->loginfo.trace & DOgNlsTraceMinimal)
  {
    time(&ltime);
    OgMsg(nls_ctrl->hmsg, "", DOgMsgDestInLog, "Program %s.exe exiting at %.24s\n", param->loginfo.where,
        OgGmtime(&ltime));
  }

  OgMsgFlush(nls_ctrl->hmsg);
  OgSigFlush(nls_ctrl->hsig);
  OgErrFlush(nls_ctrl->herr);

  return (0);
}

static int OgUse(struct og_nls_ctrl *nls_ctrl, void *hInstance, int nCmdShow)
{
  char buffer[8192];

  sprintf(buffer, "Usage : ogm_nls [-v] [-h] [options]\n");
  sprintf(buffer + strlen(buffer), "options are:\n");
  sprintf(buffer + strlen(buffer), "  -d start as a daemon (Unix only)\n");
  sprintf(buffer + strlen(buffer), "  -nosignal: gets a core dump, instead of a log set by the exception\n");
  sprintf(buffer + strlen(buffer), "    -h prints this message\n");
  sprintf(buffer + strlen(buffer), "    -nlst_<n>: trace options for logging (default 0x%x)\n",
      nls_ctrl->loginfo->trace);
  sprintf(buffer + strlen(buffer), "      <n> has a combined hexadecimal value of:\n");
  sprintf(buffer + strlen(buffer), "        0x1: minimal, 0x2: memory, 0x4: all request, 0x8: answer\n");
  sprintf(buffer + strlen(buffer), "        0x10: socket, 0x20: socket size, 0x40: LT, 0x80: LT search\n");
  sprintf(buffer + strlen(buffer), "    -v gives version number of the program\n");

  OgMessageBox(0, buffer, nls_ctrl->loginfo->where, DOgMessageBoxInformation);

  DONE;
}

/**
 *    Gets trace level which is defined as a file in CONTROL directory
 *    named jtrace_<x>.rqs, where x is hexadecimal of variable nls_ctrl->loginfo->trace.
 *    Example : ftrace_1.rqs to get minimal trace.
 */
static int GetNlsControlVariables(struct og_nls_ctrl *nls_ctrl)
{
  char *stop, value[1024];
  if (OgGetVariableFile(nls_ctrl->loginfo->where, "nlst_", value))
  {
    int save_trace = nls_ctrl->loginfo->trace;
    nls_ctrl->loginfo->trace = strtoul(value, &stop, 16);
    if (save_trace != nls_ctrl->loginfo->trace)
    {
      OgMsg(nls_ctrl->hmsg, "", DOgMsgDestInLog, "Trace value is %x", nls_ctrl->loginfo->trace);
    }

    if (nls_ctrl->loginfo->trace & DOgNlsTraceMinimal)
    {
      OgMsg(nls_ctrl->hmsg, "", DOgMsgDestInLog, "  minimal trace");
    }

    if (nls_ctrl->loginfo->trace & DOgNlsTraceMemory)
    {
      OgMsg(nls_ctrl->hmsg, "", DOgMsgDestInLog, "  memory trace");
    }

    if (nls_ctrl->loginfo->trace & DOgNlsTraceAllRequest)
    {
      OgMsg(nls_ctrl->hmsg, "", DOgMsgDestInLog, "  all request trace");
    }

    if (nls_ctrl->loginfo->trace & DOgNlsTraceAnswer)
    {
      OgMsg(nls_ctrl->hmsg, "", DOgMsgDestInLog, "  answer trace");
    }

    if (nls_ctrl->loginfo->trace & DOgNlsTraceSocket)
    {
      OgMsg(nls_ctrl->hmsg, "", DOgMsgDestInLog, "  socket trace");
    }

    if (nls_ctrl->loginfo->trace & DOgNlsTraceSocketSize)
    {
      OgMsg(nls_ctrl->hmsg, "", DOgMsgDestInLog, "  socket size trace");
    }

    if (nls_ctrl->loginfo->trace & DOgNlsTraceLT)
    {
      OgMsg(nls_ctrl->hmsg, "", DOgMsgDestInLog, "  listening threads trace");
    }

    if (nls_ctrl->loginfo->trace & DOgNlsTraceLTSearch)
    {
      OgMsg(nls_ctrl->hmsg, "", DOgMsgDestInLog, "  listening threads search trace");
    }

  }
  return (0);
}

static int GetNlsConfiguration(struct og_nls_ctrl *nls_ctrl, char *conf)
{
  char value[DPcPathSize];
  int found;

  if (!OgFileExists(conf)) DONE;

  /** Since this variable concerns log tuning, we do it as soon as possible **/
  IFE(found=OgDipperConfGetVar(conf,"msg_timestamp",value,DPcPathSize));
  if (found)
  {
    OgTrimString(value, value);
    PcStrlwr(value);
    size_t msg_timestamp = FALSE;
    if (!strcmp(value, "true") || !strcmp(value, "yes")) msg_timestamp = TRUE;
    IF(OgMsgTune(nls_ctrl->hmsg, DOgMsgTuneShowTimestamp, (void *) msg_timestamp)) return (0);
  }

  /** Since this variable concerns log tuning, we do it as soon as possible **/
  IFE(found=OgDipperConfGetVar(conf,"msg_id",value,DPcPathSize));
  if (found)
  {
    OgTrimString(value, value);
    size_t msg_id = OgArgSize(value);
    IF(OgMsgTune(nls_ctrl->hmsg, DOgMsgTuneShowId, (void *) msg_id)) return (0);
  }

  /** Since this variable concerns log tuning, we do it as soon as possible **/
  IFE(found=OgDipperConfGetVar(conf,"msg_mask", value, DPcPathSize));
  if (found)
  {
    OgTrimString(value, value);
    int msg_mask;
    IFE(OgMsgMaskInterpret(nls_ctrl->hmsg, value, &msg_mask));

    size_t clean_msg_mask = msg_mask;
    IF(OgMsgTune(nls_ctrl->hmsg, DOgMsgTuneMask, (void *) clean_msg_mask)) return (0);
  }

  DONE;
}

static void NlsSignalFunc(void *context, int signal_type)
{
  struct og_nls_ctrl *nls_ctrl = (struct og_nls_ctrl *) context;

  int levelFlag = DOgMsgDestInLog + DOgMsgDestInErr + DOgMsgParamDateIn;

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

  if (signal_type != SIGUSR1)
  {
    DoExit(nls_ctrl);
  }
}

static void LogErrors(struct og_nls_ctrl *nls_ctrl, char *label)
{
  int mb = DOgMsgDestInLog + DOgMsgDestInErr;
  OgMsgErr(nls_ctrl->hmsg, "ogm_nls_normal_exit", 1, 0, 0, DOgMsgSeverityEmergency, 0);
  OgMsg(nls_ctrl->hmsg, "exiting_normally", mb, "Errors found in '%s', exiting normally", label);
}

void DoExit(nls_ctrl)
  struct og_nls_ctrl *nls_ctrl;
{
  int mb = DOgMsgDestInLog + DOgMsgDestInErr;
  OgMsgErr(nls_ctrl->hmsg, "ogm_nls_error", 1, 0, 0, DOgMsgSeverityEmergency, 0);
  OgMsg(nls_ctrl->hmsg, "exiting_on_error", mb, "Program ogm_nls exiting on error.");
  exit(1);
}

