/*
 *  Initialization for Natural Language functions
 *  Copyright (c) 2017 by Patrick Constant
 *  Dev : August 2017
 *  Version 1.0
 */
#include "ogm_nls.h"
#include <stddef.h>
#include <glib-2.0/glib/gstdio.h>

PUBLIC(og_nls) OgNlsInit(struct og_nls_param *param)
{
  char erreur[DOgErrorSize], filename[DPcPathSize];
  struct og_ctrl_nls *ctrl_nls = (struct og_ctrl_nls *) malloc(sizeof(struct og_ctrl_nls));
  IFn(ctrl_nls)
  {
    sprintf(erreur, "OgNlsInit: malloc error on ctrl_nls");
    OgErr(param->herr, erreur);
    return NULL;
  }

  memset(ctrl_nls, 0, sizeof(struct og_ctrl_nls));
  ctrl_nls->herr = param->herr;
  ctrl_nls->hmutex = param->hmutex;
  memcpy(ctrl_nls->loginfo, &param->loginfo, sizeof(struct og_loginfo));
  strcpy(ctrl_nls->WorkingDirectory, param->WorkingDirectory);
  strcpy(ctrl_nls->configuration_file, param->configuration_file);
  strcpy(ctrl_nls->pidfile, param->pidfile);
  strcpy(ctrl_nls->import_directory, param->import_directory);
  IFn(ctrl_nls->import_directory[0]) strcpy(ctrl_nls->import_directory, "import");

  struct og_msg_param msg_param[1];
  memset(msg_param, 0, sizeof(struct og_msg_param));
  msg_param->herr = ctrl_nls->herr;
  msg_param->hmutex = ctrl_nls->hmutex;
  msg_param->loginfo.trace = DOgMsgTraceMinimal + DOgMsgTraceMemory;
  msg_param->loginfo.where = ctrl_nls->loginfo->where;
  msg_param->module_name = "nls";
  IFn(ctrl_nls->hmsg=OgMsgInit(msg_param)) return NULL;
  IF(OgMsgTuneInherit(ctrl_nls->hmsg,param->hmsg)) return NULL;

  IF(NlsConfReadFile(ctrl_nls,1)) return NULL;
  IF(NlsConfReadEnv(ctrl_nls)) return NULL;

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

  IF(OgStartupSockets(param->herr)) return NULL;

  ctrl_nls->LtNumber = ctrl_nls->conf->max_listening_threads;
  int size = ctrl_nls->LtNumber * sizeof(struct og_listening_thread);
  ctrl_nls->Lt = (struct og_listening_thread *) malloc(size);
  IFn(ctrl_nls->Lt)
  {
    sprintf(erreur, "OgNlsInit: malloc error on ctrl_nls->lt (%d bytes)", size);
    OgErr(ctrl_nls->herr, erreur);
    return NULL;
  }
  memset(ctrl_nls->Lt, 0, size);

  struct og_uci_server_param ucis_param[1];
  memset(ucis_param, 0, sizeof(struct og_uci_server_param));
  ucis_param->hmsg = ctrl_nls->hmsg;
  ucis_param->herr = ctrl_nls->herr;
  ucis_param->hmutex = ctrl_nls->hmutex;
  ucis_param->loginfo.trace = DOgUciServerTraceMinimal + DOgUciServerTraceMemory;
  if (ctrl_nls->loginfo->trace & DOgNlsTraceSocket) ucis_param->loginfo.trace |= DOgUciServerTraceSocket;
  if (ctrl_nls->loginfo->trace & DOgNlsTraceSocketSize) ucis_param->loginfo.trace |= DOgUciServerTraceSocketSize;
  ucis_param->socket_buffer_size = ctrl_nls->conf->max_request_size;
  ucis_param->loginfo.where = ctrl_nls->loginfo->where;
  IFn(ctrl_nls->hucis=OgUciServerInit(ucis_param)) return NULL;

  // Use to send http request to unblock other thread
  struct og_uci_client_param ucic_param[1];
  memset(ucic_param, 0, sizeof(struct og_uci_client_param));
  ucic_param->herr = ctrl_nls->herr;
  ucic_param->hmutex = ctrl_nls->hmutex;
  ucic_param->loginfo.trace = DOgUciClientTraceMinimal + DOgUciClientTraceMemory;
  ucic_param->loginfo.where = ctrl_nls->loginfo->where;
  IFn(ctrl_nls->hucic = OgUciClientInit(ucic_param)) return NULL;

  /** Maintenance thread initialization **/
  struct og_maintenance_thread *mt = ctrl_nls->mt;
  memset(mt, 0, sizeof(struct og_maintenance_thread));
  IF(OgMaintenanceThreadInit(ctrl_nls))
  {
    sprintf(erreur, "OgNlsInit: OgMaintenanceThreadInit error");
    OgErr(ctrl_nls->herr, erreur);
    return NULL;
  }

  IF(OgSemaphoreInit(ctrl_nls->hsem_run3,ctrl_nls->LtNumber)) return NULL;

  /** Mutex for choosing lt */
  IF(OgInitCriticalSection(ctrl_nls->hmutex_run_lt,"hmutex_run_lt")) return NULL;

  struct og_nlp_param nlp_param[1];
  memset(nlp_param, 0, sizeof(struct og_nlp_param));
  nlp_param->hmsg = ctrl_nls->hmsg;
  nlp_param->herr = ctrl_nls->herr;
  nlp_param->hmutex = ctrl_nls->hmutex;
  nlp_param->loginfo.trace = DOgNlpTraceMinimal + DOgNlpTraceMemory;
  nlp_param->loginfo.where = ctrl_nls->loginfo->where;
  ctrl_nls->hnlp = OgNlpInit(nlp_param);
  IFN(ctrl_nls->hnlp) return NULL;

  struct og_nlp_threaded_param nlpi_param[1];
  memset(nlpi_param, 0, sizeof(struct og_nlp_threaded_param));
  nlpi_param->herr = ctrl_nls->herr;
  nlpi_param->hmsg = ctrl_nls->hmsg;
  nlpi_param->hmutex = ctrl_nls->hmutex;
  nlpi_param->loginfo.trace = DOgNlpTraceMinimal + DOgNlpTraceMemory;
  nlpi_param->loginfo.where = ctrl_nls->loginfo->where;
  nlpi_param->name = "nls_cltr_main_nlpth";
  ctrl_nls->hnlpi_main = OgNlpThreadedInit(ctrl_nls->hnlp, nlpi_param);
  IFN(ctrl_nls->hnlpi_main) return NULL;

  for (int i = 0; i < ctrl_nls->LtNumber; i++)
  {
    og_listening_thread *lt = ctrl_nls->Lt + i;

    lt->ID = i;
    lt->ctrl_nls = ctrl_nls;

    IF(NlsLtInit(lt)) return NULL;

  }

  if (ctrl_nls->conf->permanent_threads)
  {
    IF(NlsInitPermanentLtThreads(ctrl_nls)) return NULL;
  }

  IF(NlsReadImportFiles(ctrl_nls)) return NULL;

  return ctrl_nls;
}

PUBLIC(int) OgNlsFlush(og_nls handle)
{
  if (handle == NULL) CONT;

  struct og_ctrl_nls *ctrl_nls = handle;

  if (ctrl_nls->conf->permanent_threads)
  {
    IFE(NlsFlushPermanentLtThreads(ctrl_nls));
  }

  for (int i = 0; i < ctrl_nls->LtNumber; i++)
  {
    struct og_listening_thread *lt = ctrl_nls->Lt + i;

    IFE(NlsLtFlush(lt));

  }
  ctrl_nls->LtNumber = 0;
  DPcFree(ctrl_nls->Lt);

  IFE(OgNlpThreadedFlush(ctrl_nls->hnlpi_main));

  IFE(OgNlpFlush(ctrl_nls->hnlp));

  IFE(OgUciClientFlush(ctrl_nls->hucic));
  IFE(OgUciServerFlush(ctrl_nls->hucis));

  IFE(OgMaintenanceThreadFlush(ctrl_nls->mt));

  IFE(OgMsgFlush(ctrl_nls->hmsg));

  OgCleanupSocket();

  // clean up pidfile
  if (ctrl_nls->pidfile[0] && g_file_test(ctrl_nls->pidfile, G_FILE_TEST_EXISTS | G_FILE_TEST_IS_REGULAR))
  {
    g_unlink(ctrl_nls->pidfile);
  }

  DPcFree(ctrl_nls);
  DONE;
}

PUBLIC(og_status) OgNlsOnSignalStop(og_nls ctrl_nls)
{
  // stop non blocking thread
  ctrl_nls->must_stop = TRUE;

  struct og_ucic_request request[1];
  memset(request, 0, sizeof(struct og_ucic_request));

  // bind all addresses '0.0.0.0' need to use localhost instead '127.0.0.1'
  strcpy(request->hostname, ctrl_nls->conf->env->listenning_address);
  if (strcmp(request->hostname, "0.0.0.0") == 0)
  {
    strcpy(request->hostname, "127.0.0.1");
  }
  request->port = ctrl_nls->conf->env->listenning_port;
  request->timeout = ctrl_nls->conf->backlog_timeout * 1.2;
  request->request = "";
  request->request_length = 0;

  struct og_ucic_answer answer[1];
  IFE(OgUciClientRequest(ctrl_nls->hucic, request, answer));

  DONE;
}

PUBLIC(og_status) OgNlsOnSignalEmergency(og_nls ctrl_nls)
{
  IFE(NlsOnEmergency(ctrl_nls));

  DONE;
}

PUBLIC(char *) OgNlsBanner(void)
{
  return DOgNlsBanner;
}

og_status OgNlsWritePidFile(og_nls ctrl_nls)
{
  if (!ctrl_nls->pidfile[0]) CONT;

  FILE* file = fopen(ctrl_nls->pidfile, "w");
  IFN(file)
  {
    og_char_buffer erreur[DPcPathSize];
    sprintf(erreur, "OgNlsWritePidFile: failed to open pidfile %s.", ctrl_nls->pidfile);
    OgMsg(ctrl_nls->hmsg, "OgNlsWritePidFile", DOgMsgDestInLog + DOgMsgDestInErr + DOgMsgDestMBox, erreur);
    OgErr(ctrl_nls->herr, erreur);
    DPcErr;
  }

  fprintf(file, "%ld\n", (long) getpid());

  int close_status = fclose(file);
  if (close_status != 0)
  {
    og_char_buffer erreur[DPcPathSize];
    sprintf(erreur, "OgNlsWritePidFile: failed to write in pidfile %s.", ctrl_nls->pidfile);
    OgMsg(ctrl_nls->hmsg, "OgNlsWritePidFile", DOgMsgDestInLog + DOgMsgDestInErr + DOgMsgDestMBox, erreur);
    OgErr(ctrl_nls->herr, erreur);
    DPcErr;
  }

  DONE;
}

