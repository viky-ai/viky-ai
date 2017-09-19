/*
 *  Initialization for Natural Language functions
 *  Copyright (c) 2017 by Patrick Constant
 *  Dev : August 2017
 *  Version 1.0
 */
#include "ogm_nls.h"
#include <stddef.h>

PUBLIC(og_nls) OgNlsInit(struct og_nls_param *param)
{
  char erreur[DOgErrorSize], filename[DPcPathSize];
  struct og_ctrl_nls *ctrl_nls = (struct og_ctrl_nls *) malloc(sizeof(struct og_ctrl_nls));
  IFn(ctrl_nls)
  {
    sprintf(erreur, "OgNlsInit: malloc error on ctrl_nls");
    OgErr(param->herr, erreur);
    return (0);
  }

  memset(ctrl_nls, 0, sizeof(struct og_ctrl_nls));
  ctrl_nls->herr = param->herr;
  ctrl_nls->hmutex = param->hmutex;
  memcpy(ctrl_nls->loginfo, &param->loginfo, sizeof(struct og_loginfo));
  strcpy(ctrl_nls->WorkingDirectory, param->WorkingDirectory);
  strcpy(ctrl_nls->configuration_file, param->configuration_file);

  struct og_msg_param msg_param[1];
  memset(msg_param, 0, sizeof(struct og_msg_param));
  msg_param->herr = ctrl_nls->herr;
  msg_param->hmutex = ctrl_nls->hmutex;
  msg_param->loginfo.trace = DOgMsgTraceMinimal + DOgMsgTraceMemory;
  msg_param->loginfo.where = ctrl_nls->loginfo->where;
  msg_param->module_name = "nls";
  IFn(ctrl_nls->hmsg=OgMsgInit(msg_param)) return (0);
  IF(OgMsgTuneInherit(ctrl_nls->hmsg,param->hmsg)) return (0);

  IF(NlsConfReadFile(ctrl_nls,1)) return (0);
  IF(NlsConfReadEnv(ctrl_nls)) return (0);

  if (ctrl_nls->WorkingDirectory[0] && !OgIsAbsolutePath(ctrl_nls->conf->data_directory))
  {
    sprintf(filename, "%s/%s", ctrl_nls->WorkingDirectory, ctrl_nls->conf->data_directory);
    strcpy(ctrl_nls->conf->data_directory, filename);
  }

  IFN(getcwd(ctrl_nls->cwd,DPcPathSize)) return NULL;
  ctrl_nls->icwd = strlen(ctrl_nls->cwd);
  for (int i = 0; i < ctrl_nls->icwd; i++)
  {
    if (ctrl_nls->cwd[i] == '\\') ctrl_nls->cwd[i] = '/';
  }

  IF(OgStartupSockets(param->herr)) return (0);

  ctrl_nls->LtNumber = ctrl_nls->conf->max_listening_threads;
  int size = ctrl_nls->LtNumber * sizeof(struct og_listening_thread);
  ctrl_nls->Lt = (struct og_listening_thread *) malloc(size);
  IFn(ctrl_nls->Lt)
  {
    sprintf(erreur, "OgNlsInit: malloc error on ctrl_nls->lt (%d bytes)", size);
    OgErr(ctrl_nls->herr, erreur);
    return (0);
  }
  memset(ctrl_nls->Lt, 0, size);

  struct og_uci_server_param uci_param[1];
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

  /** Maintenance thread initialization **/
  struct og_maintenance_thread *mt = &ctrl_nls->mt;
  memset(mt, 0, sizeof(struct og_maintenance_thread));
  IFn(mt->herr=OgErrInit())
  {
    sprintf(erreur, "OgSsrvInit: OgErrInit error");
    OgErr(ctrl_nls->herr, erreur);
    return (0);
  }
  mt->ctrl_nls = ctrl_nls;
  mt->hmutex = ctrl_nls->hmutex;

  IF(OgSemaphoreInit(ctrl_nls->hsem_run3,ctrl_nls->LtNumber)) return (0);

  /** Mutex for choosing lt */
  IF(OgInitCriticalSection(ctrl_nls->hmutex_run_lt,"hmutex_run_lt")) return (0);

  for (int i = 0; i < ctrl_nls->LtNumber; i++)
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

    IF(OgNLSJsonInit(lt)) return NULL;

  }

  if (ctrl_nls->conf->permanent_threads)
  {
    IF(NlsInitPermanentLtThreads(ctrl_nls)) return (0);
  }

  return ctrl_nls;
}

PUBLIC(og_status) OgNlsOnSignalStop(og_nls handle)
{

  // TODO implement stopping check

  OgNlsFlush(handle);

  DONE;
}

PUBLIC(og_status) OgNlsOnSignalEmergency(og_nls handle)
{
  struct og_ctrl_nls *ctrl_nls = handle;

  IFE(NlsOnEmergency(ctrl_nls));

  DONE;
}

PUBLIC(og_status) OgNlsOnSignalTimeout(og_nls handle)
{
  struct og_ctrl_nls *ctrl_nls = handle;
  int i = -1;
  for (i = 0; i < ctrl_nls->LtNumber; i++)
  {
    struct og_listening_thread *lt = ctrl_nls->Lt + i;
    if (lt->current_thread == pthread_self())
    {
      NlsThrowError(lt, "OgNlsOnSignalTimeout : Timeout on LT = %d",i);
      int is_error = OgListeningThreadError(lt);
      OgCloseSocket(lt->hsocket_in);
      IFE(is_error);
      IFE(OgNlsLtReleaseCurrentRunnning(lt));
      IFE(NlsListeningThreadReset(lt));
      break;
    }
  }

  pthread_exit((void *) 1);

  DONE;
}

PUBLIC(int) OgNlsFlush(og_nls handle)
{
  struct og_ctrl_nls *ctrl_nls = handle;

  if (ctrl_nls->conf->permanent_threads)
  {
    IFE(NlsFlushPermanentLtThreads(ctrl_nls));
  }

  for (int i = 0; i < ctrl_nls->LtNumber; i++)
  {
    struct og_listening_thread *lt = ctrl_nls->Lt + i;
    IFE(OgUciServerFlush(lt->hucis));
    IFE(OgNLSJsonFlush(lt));
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
  return DOgNlsBanner;
}

