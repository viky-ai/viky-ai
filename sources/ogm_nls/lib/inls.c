/*
 *  Initialization for Natural Language functions
 *  Copyright (c) 2017 by Patrick Constant
 *  Dev : August 2017
 *  Version 1.0
 */
#include "ogm_nls.h"
#include <stddef.h>

static void * OgNlsInit1(struct og_nls_param *, void **);

PUBLIC(void *) OgNlsInit(struct og_nls_param *param, void **phandle)
{
  struct og_ctrl_nls *ctrl_nls;
  struct og_listening_thread *lt;
  void *handle; int i;
  IFn(handle=OgNlsInit1(param,phandle))
  {
    ctrl_nls = (struct og_ctrl_nls *) *phandle;
    IFx(ctrl_nls)
    {
      /* We need to send the errors from listening threads here,
       * because they cannot be reached after this point */
      for (i=0; i<ctrl_nls->LtNumber; i++)
      {
        lt = ctrl_nls->Lt + i;
        IFn(lt->herr) continue;
        OgMsgErr(lt->hmsg,"",0,0,0,DOgMsgSeverityEmergency+DOgMsgDestInLog+DOgMsgDestInErr+DOgMsgDestMBox
            , DOgErrLogFlagNoSystemError/*+DOgErrLogFlagNotInErr*/);
      }
    }
  }

  return(handle);
}

static void * OgNlsInit1(struct og_nls_param *param, void **phandle)
{
  char erreur[DOgErrorSize], sys_erreur[DOgErrorSize], filename[DPcPathSize];
  struct og_uci_server_param uci_param[1];
  struct og_msg_param msg_param[1];
  struct og_ctrl_nls *ctrl_nls;
  int i, size;

  *phandle = 0;

  IFn(ctrl_nls=(struct og_ctrl_nls *)malloc(sizeof(struct og_ctrl_nls)))
  {
    sprintf(erreur, "OgNlsInit: malloc error on ctrl_nls");
    OgErr(param->herr, erreur);
    return (0);
  }

  *phandle = ctrl_nls;

  memset(ctrl_nls, 0, sizeof(struct og_ctrl_nls));
  ctrl_nls->herr = param->herr;
  ctrl_nls->hmutex = param->hmutex;
  memcpy(ctrl_nls->loginfo, &param->loginfo, sizeof(struct og_loginfo));
  strcpy(ctrl_nls->WorkingDirectory, param->WorkingDirectory);
  strcpy(ctrl_nls->configuration_file, param->configuration_file);

  memset(msg_param, 0, sizeof(struct og_msg_param));
  msg_param->herr = ctrl_nls->herr;
  msg_param->hmutex = ctrl_nls->hmutex;
  msg_param->loginfo.trace = DOgMsgTraceMinimal + DOgMsgTraceMemory;
  msg_param->loginfo.where = ctrl_nls->loginfo->where;
  msg_param->module_name = "nls";
  IFn(ctrl_nls->hmsg=OgMsgInit(msg_param)) return (0);
  IF(OgMsgTuneInherit(ctrl_nls->hmsg,param->hmsg)) return (0);

  IF(NlsReadConfigurationFile(ctrl_nls,1)) return (0);

  if (ctrl_nls->WorkingDirectory[0] && !OgIsAbsolutePath(ctrl_nls->conf->data_directory))
  {
    sprintf(filename, "%s/%s", ctrl_nls->WorkingDirectory, ctrl_nls->conf->data_directory);
    strcpy(ctrl_nls->conf->data_directory, filename);
  }

  IFN(getcwd(ctrl_nls->cwd,DPcPathSize)) return NULL;
  ctrl_nls->icwd = strlen(ctrl_nls->cwd);
  for (i = 0; i < ctrl_nls->icwd; i++)
    if (ctrl_nls->cwd[i] == '\\') ctrl_nls->cwd[i] = '/';

  IF(OgStartupSockets(param->herr)) return (0);
  if (gethostname(ctrl_nls->hostname, DPcPathSize) != 0)
  {
    int nerr = OgSysErrMes(DOgSocketErrno, DPcSzSysErr, sys_erreur);
    sprintf(erreur, "gethostname: (%d) %s", nerr, sys_erreur);
    OgErr(ctrl_nls->herr, erreur);
    return (0);
  }
  if (ctrl_nls->loginfo->trace & DOgNlsTraceMinimal)
  {
    OgMsg(ctrl_nls->hmsg, "gethostname", DOgMsgDestInLog, "OgNlsInit: hostname is '%s'", ctrl_nls->hostname);
  }

  ctrl_nls->LtNumber = ctrl_nls->conf->max_listening_threads;
  size = ctrl_nls->LtNumber * sizeof(struct og_listening_thread);
  IFn(ctrl_nls->Lt=(struct og_listening_thread *)malloc(size))
  {
    sprintf(erreur, "OgNlsInit: malloc error on ctrl_nls->lt (%d bytes)", size);
    OgErr(ctrl_nls->herr, erreur);
    return (0);
  }
  memset(ctrl_nls->Lt, 0, size);

  memset(uci_param, 0, sizeof(struct og_uci_server_param));
  uci_param->hmsg = ctrl_nls->hmsg;
  uci_param->herr = ctrl_nls->herr;
  uci_param->hmutex = ctrl_nls->hmutex;
  uci_param->loginfo.trace = DOgUciServerTraceMinimal + DOgUciServerTraceMemory;
  if (ctrl_nls->loginfo->trace & DOgNlsTraceSocket) uci_param->loginfo.trace |= DOgUciServerTraceSocket;
  if (ctrl_nls->loginfo->trace & DOgNlsTraceSocketSize) uci_param->loginfo.trace |= DOgUciServerTraceSocketSize;
  uci_param->socket_buffer_size = ctrl_nls->conf->max_request_size;
  uci_param->loginfo.where = ctrl_nls->loginfo->where;
  IFn(ctrl_nls->hucis=OgUciServerInit(uci_param)) return (0);

  IF(OgSemaphoreInit(ctrl_nls->hsem_run3,ctrl_nls->LtNumber)) return (0);
  /** Mutex for choosing lt */
  IF(OgInitCriticalSection(ctrl_nls->hmutex_run_lt,"hmutex_run_lt")) return (0);

  for (i = 0; i < ctrl_nls->LtNumber; i++)
  {
    og_listening_thread *lt = ctrl_nls->Lt + i;
    lt->ID = i;
    lt->ctrl_nls = ctrl_nls;
    lt->loginfo[0] = ctrl_nls->loginfo[0];

    IFn(lt->herr=OgErrInit())
    {
      sprintf(erreur, "OgNlsInit: OgErrInit error");
      OgErr(ctrl_nls->herr, erreur);
      return (0);
    }
    lt->hmutex = ctrl_nls->hmutex;

    memset(msg_param, 0, sizeof(struct og_msg_param));
    msg_param->herr = lt->herr;
    msg_param->hmutex = lt->hmutex;
    msg_param->loginfo.trace = DOgMsgTraceMinimal + DOgMsgTraceMemory;
    msg_param->loginfo.where = ctrl_nls->loginfo->where;
    msg_param->module_name = "nls";
    IFn(lt->hmsg=OgMsgInit(msg_param)) return (0);
    IF(OgMsgTuneInherit(lt->hmsg,ctrl_nls->hmsg)) return (0);

    memset(uci_param, 0, sizeof(struct og_uci_server_param));
    uci_param->hmsg = lt->hmsg;
    uci_param->herr = lt->herr;
    uci_param->hmutex = lt->hmutex;
    uci_param->loginfo.trace = DOgUciServerTraceMinimal + DOgUciServerTraceMemory;
    if (ctrl_nls->loginfo->trace & DOgNlsTraceSocket) uci_param->loginfo.trace |= DOgUciServerTraceSocket;
    if (ctrl_nls->loginfo->trace & DOgNlsTraceSocketSize) uci_param->loginfo.trace |= DOgUciServerTraceSocketSize;
    uci_param->socket_buffer_size = ctrl_nls->conf->max_request_size;
    uci_param->loginfo.where = ctrl_nls->loginfo->where;
    IFn(lt->hucis=OgUciServerInit(uci_param)) return (0);

  }

  if (ctrl_nls->conf->permanent_threads)
  {
    IF(NlsInitPermanentLtThreads(ctrl_nls)) return (0);
  }

  return ((void *) ctrl_nls);
}

PUBLIC(int) OgNlsOnSignal(void *handle)
{
  struct og_ctrl_nls *ctrl_nls = (struct og_ctrl_nls *) handle;

  IFE(NlsOnEmergency(ctrl_nls));

  DONE;
}

PUBLIC(int) OgNlsFlush(void *handle)
{
  struct og_ctrl_nls *ctrl_nls = (struct og_ctrl_nls *) handle;

  if (ctrl_nls->conf->permanent_threads)
  {
    IFE(NlsFlushPermanentLtThreads(ctrl_nls));
  }

  for (int i = 0; i < ctrl_nls->LtNumber; i++)
  {
    struct og_listening_thread *lt = ctrl_nls->Lt + i;
    IFE(OgUciServerFlush(lt->hucis));
    OgErrFlush(lt->herr);
  }
  DPcFree(ctrl_nls->Lt);

  IFE(OgMsgFlush(ctrl_nls->hmsg));

  OgCleanupSocket();

  DPcFree(ctrl_nls);
  DONE;
}

PUBLIC(char *) OgNlsBanner(void)
{
  return(DOgNlsBanner);
}

