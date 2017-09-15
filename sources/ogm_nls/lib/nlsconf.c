/*
 *  reading configuration file
 *  Copyright (c) 2006-2010 Pertimm by Patrick Constant
 *  Dev: July,October,November 2006, January,February,June,August 2007
 *  Dev: February 2008, March 2010, September 2010
 *  Version 1.9
 */
#include "ogm_nls.h"

static og_bool setTimeout(struct og_ctrl_nls *ctrl_nls, char *conf, og_bool init, struct timeout_conf_context *ctx,
    int *timeout);

static og_bool setTimeout(struct og_ctrl_nls *ctrl_nls, char *conf, og_bool init, struct timeout_conf_context *ctx,
    int *timeout)
{
  int new_timeout = ctx->default_timeout;
  og_bool answer_timeout_specified = FALSE;

  char value[DPcPathSize];
  og_bool found = OgDipperConfGetVar(conf, ctx->timeout_name, value, DPcPathSize);
  IFE(found);
  if (found)
  {
    OgTrimString(value, value);
    new_timeout = OgArgSize(value);
    answer_timeout_specified = TRUE;
  }

  // change 0 timeout to very big timeout
  if (new_timeout == 0)
  {

    new_timeout = 1E9;
  }

  if (answer_timeout_specified)
  {
    if (init || new_timeout != ctx->old_timeout)
    {
      *timeout = new_timeout;
    }
    if (ctrl_nls->loginfo->trace & DOgNlsTraceMinimal)
    {
      if (init)
      {
        OgMsg(ctrl_nls->hmsg, "answer_timeout", DOgMsgDestInLog, "%s: %d milli-seconds", ctx->timeout_name, *timeout);
      }
      else if (new_timeout != ctx->old_timeout)
      {
        OgMsg(ctrl_nls->hmsg, "answer_timeout", DOgMsgDestInLog, "%s changed from %d to %d milli-seconds",
            ctx->timeout_name, ctx->old_timeout, *timeout);
      }
    }
  }
  return answer_timeout_specified;
}

og_status NlsConfReadEnv(struct og_ctrl_nls *ctrl_nl)
{
  // Default values
  struct og_nls_env *env = ctrl_nl->conf->env;
  snprintf(env->listenning_address, DPcPathSize, "0.0.0.0");
  env->listenning_port = DOgNlsPortNumber;

  // Values from env
  const gchar *listenning_address = g_getenv("NLS_LISTENNING_ADDRESS");
  if (listenning_address != NULL)
  {
    IFE(OgParseServerAddress(listenning_address, env->listenning_address, &env->listenning_port));
  }

  DONE;
}

og_status NlsConfReadFile(struct og_ctrl_nls *ctrl_nls, int init)
{
  struct og_nls_conf new_conf[1], old_conf[1];
  char *conf = ctrl_nls->configuration_file;
  char value[DPcPathSize];
  int found;

  if (init) memset(old_conf, 0, sizeof(struct og_nls_conf));
  else memcpy(old_conf, ctrl_nls->conf, sizeof(struct og_nls_conf));

  if (!OgFileExists(conf))
  {
    if (ctrl_nls->loginfo->trace & DOgNlsTraceMinimal)
    {
      OgMsg(ctrl_nls->hmsg, "noconf", DOgMsgDestInLog, "Configuration file '%s' does not exist", conf);
    }
    DONE;
  }

  if (init)
  {
    if (ctrl_nls->loginfo->trace & DOgNlsTraceMinimal)
    {
      OgMsg(ctrl_nls->hmsg, "readconf", DOgMsgDestInLog, "Reading configuration file '%s'", conf);
    }
  }

  if (init)
  {
    IFE(found=OgDipperConfGetVar(conf,"max_listening_threads",value,DPcPathSize));
    if (found)
    {
      OgTrimString(value, value);
      ctrl_nls->conf->max_listening_threads = OgArgSize(value);
      if (ctrl_nls->loginfo->trace & DOgNlsTraceMinimal)
      {
        OgMsg(ctrl_nls->hmsg, "max_listening_threads", DOgMsgDestInLog, "max_listening_threads: %d",
            ctrl_nls->conf->max_listening_threads);
      }
    }
    else ctrl_nls->conf->max_listening_threads = DOgNlsMaxListeningThreads;
  }

  if (init)
  {
    IFE(found=OgDipperConfGetVar(conf,"permanent_threads",value,DPcPathSize));
    if (found)
    {
      OgTrimString(value, value);
      PcStrlwr(value);
      if (!strcmp(value, "true") || !strcmp(value, "yes")) ctrl_nls->conf->permanent_threads = 1;
      else ctrl_nls->conf->permanent_threads = 0;
      if (ctrl_nls->loginfo->trace & DOgNlsTraceMinimal)
      {
        OgMsg(ctrl_nls->hmsg, "permanent_threads", DOgMsgDestInLog, "permanent_threads: %d",
            ctrl_nls->conf->permanent_threads);
      }
    }
    else ctrl_nls->conf->permanent_threads = DOgNlsPermanentThreads;
  }

  IFE(found=OgDipperConfGetVar(conf,"backlog_max_pending_requests",value,DPcPathSize));
  if (found)
  {
    OgTrimString(value, value);
    new_conf->backlog_max_pending_requests = OgArgSize(value);
  }
  else new_conf->backlog_max_pending_requests = DOgNlsBacklogMaxPendingRequests;
  if (init || new_conf->backlog_max_pending_requests != old_conf->backlog_max_pending_requests)
  {
    ctrl_nls->conf->backlog_max_pending_requests = new_conf->backlog_max_pending_requests;
  }
  if (ctrl_nls->loginfo->trace & DOgNlsTraceMinimal)
  {
    if (init)
    {
      OgMsg(ctrl_nls->hmsg, "backlog_max_pending_requests", DOgMsgDestInLog, "backlog_max_pending_requests: %d seconds",
          ctrl_nls->conf->backlog_max_pending_requests);
    }
    else if (new_conf->backlog_max_pending_requests != old_conf->backlog_max_pending_requests)
    {
      OgMsg(ctrl_nls->hmsg, "backlog_max_pending_requests", DOgMsgDestInLog,
          "backlog_max_pending_requests changed from %d to %d seconds", old_conf->backlog_max_pending_requests,
          ctrl_nls->conf->backlog_max_pending_requests);
    }
  }

  /** TIMEOUT specifications */

  struct timeout_conf_context timeout_ctx[1];
  memset(timeout_ctx, 0, sizeof(struct timeout_conf_context));
  char timeout_name[DPcPathSize];

  /** answer_timeout */
  sprintf(timeout_name, "answer_timeout");
  timeout_ctx->timeout_name = timeout_name;
  timeout_ctx->default_timeout = DOgNlsAnswerTimeout;
  timeout_ctx->old_timeout = old_conf->answer_timeout;

  og_bool answer_timeout_specified = setTimeout(ctrl_nls, conf, init, timeout_ctx, &ctrl_nls->conf->answer_timeout);
  IFE(answer_timeout_specified);

  /** backlog_timeout */
  sprintf(timeout_name, "backlog_timeout");
  timeout_ctx->timeout_name = timeout_name;
  timeout_ctx->default_timeout = DOgNlsBacklogTimeout;
  timeout_ctx->old_timeout = old_conf->backlog_timeout;

  og_bool backlog_timeout_specified = setTimeout(ctrl_nls, conf, init, timeout_ctx, &ctrl_nls->conf->backlog_timeout);
  IFE(backlog_timeout_specified);

  /** socket_read_timeout */
  sprintf(timeout_name, "socket_read_timeout");
  timeout_ctx->timeout_name = timeout_name;
  timeout_ctx->default_timeout = DOgNlsSocketReadTimeout;
  timeout_ctx->old_timeout = old_conf->socket_read_timeout;

  og_bool socket_read_timeout_specified = setTimeout(ctrl_nls, conf, init, timeout_ctx,
      &ctrl_nls->conf->socket_read_timeout);
  IFE(socket_read_timeout_specified);

  /** request_processing_timeout */
  sprintf(timeout_name, "request_processing_timeout");
  timeout_ctx->timeout_name = timeout_name;
  timeout_ctx->default_timeout = DOgNlsRequestProcessingTimeout;
  timeout_ctx->old_timeout = old_conf->request_processing_timeout;

  og_bool request_processing_timeout_specified = setTimeout(ctrl_nls, conf, init, timeout_ctx,
      &ctrl_nls->conf->request_processing_timeout);
  IFE(request_processing_timeout_specified);

  /** check timeouts conf */
  char error_buffer[DPcPathSize];
  if (answer_timeout_specified)
  {
    if (backlog_timeout_specified || socket_read_timeout_specified || request_processing_timeout_specified)
    {
      if (backlog_timeout_specified)
      {
        sprintf(error_buffer,
            "NlsReadConfigurationFile: backlog_timeout_specified in conf file has been set to value %d while "
                "answer_timeout is already specified", ctrl_nls->conf->backlog_timeout);
        OgErr(ctrl_nls->herr, error_buffer);
        DPcErr;
      }
      if (socket_read_timeout_specified)
      {
        sprintf(error_buffer,
            "NlsReadConfigurationFile: socket_read_timeout in conf file has been set to value %d while "
                "answer_timeout is already specified", ctrl_nls->conf->socket_read_timeout);
        OgErr(ctrl_nls->herr, error_buffer);
        DPcErr;
      }
      if (request_processing_timeout_specified)
      {
        sprintf(error_buffer,
            "NlsReadConfigurationFile: request_processing_timeout in conf file has been set to value %d while "
                "answer_timeout is already specified", ctrl_nls->conf->request_processing_timeout);
        OgErr(ctrl_nls->herr, error_buffer);
        DPcErr;
      }
    }
    ctrl_nls->conf->backlog_timeout = 0.5 * ctrl_nls->conf->answer_timeout;
    ctrl_nls->conf->socket_read_timeout = 0.1 * ctrl_nls->conf->answer_timeout;
    ctrl_nls->conf->request_processing_timeout = ctrl_nls->conf->answer_timeout;
  }
  else
  {
    if (backlog_timeout_specified && socket_read_timeout_specified && request_processing_timeout_specified) ;
    else if (backlog_timeout_specified || socket_read_timeout_specified || request_processing_timeout_specified)
    {
      sprintf(error_buffer,
          "NlsReadConfigurationFile: backlog_timeout, socket_read_timeout and request_processing_timeout must all be specified while "
              "backlog_timeout is %s, socket_read_timeout is %s and request_processing_timeout is %s",
          backlog_timeout_specified ? "specified" : "not specified",
          socket_read_timeout_specified ? "specified" : "not specified",
          request_processing_timeout_specified ? "specified" : "not specified");
      OgErr(ctrl_nls->herr, error_buffer);
      DPcErr;
    }
    else
    {
      ctrl_nls->conf->backlog_timeout = 0.1 * ctrl_nls->conf->answer_timeout;
      ctrl_nls->conf->socket_read_timeout = 0.1 * ctrl_nls->conf->answer_timeout;
      ctrl_nls->conf->request_processing_timeout = 0.8 * ctrl_nls->conf->answer_timeout;
    }
  }

  /** indexing backlog_timeout */
  sprintf(timeout_name, "backlog_indexing_timeout");
  timeout_ctx->timeout_name = timeout_name;
  timeout_ctx->default_timeout = ctrl_nls->conf->backlog_timeout;
  timeout_ctx->old_timeout = old_conf->backlog_indexing_timeout;

  IFE(setTimeout(ctrl_nls, conf, init, timeout_ctx, &ctrl_nls->conf->backlog_indexing_timeout));

  og_bool backlog_indexing_timeout_specified = setTimeout(ctrl_nls, conf, init, timeout_ctx,
      &ctrl_nls->conf->backlog_indexing_timeout);
  IFE(backlog_indexing_timeout_specified);
  if (!backlog_indexing_timeout_specified)
  {
    ctrl_nls->conf->backlog_indexing_timeout = timeout_ctx->default_timeout;
  }

  /** Specific rule: when the variable is not found, its value is 2*answer_timeout **/
  IFE(found=OgDipperConfGetVar(conf,"loop_answer_timeout",value,DPcPathSize));
  if (found)
  {
    OgTrimString(value, value);
    new_conf->loop_answer_timeout = OgArgSize(value);
  }
  else new_conf->loop_answer_timeout = ctrl_nls->conf->answer_timeout * 2;
  if (init || new_conf->loop_answer_timeout != old_conf->loop_answer_timeout)
  {
    ctrl_nls->conf->loop_answer_timeout = new_conf->loop_answer_timeout;
  }
  if (ctrl_nls->loginfo->trace & DOgNlsTraceMinimal)
  {
    if (init)
    {
      OgMsg(ctrl_nls->hmsg, "loop_answer_timeout", DOgMsgDestInLog, "loop_answer_timeout: %d milli-seconds",
          ctrl_nls->conf->loop_answer_timeout);
    }
    else if (new_conf->loop_answer_timeout != old_conf->loop_answer_timeout)
    {
      OgMsg(ctrl_nls->hmsg, "loop_answer_timeout", DOgMsgDestInLog,
          "loop_answer_timeout changed from %d to %d milli-seconds", old_conf->loop_answer_timeout,
          ctrl_nls->conf->loop_answer_timeout);
    }
  }

  DONE;
}

