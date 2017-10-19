/**
 * Handling synchronization for servers and indexers (Multiple reader / One Writer)
 * Copyright (c) 2014 Pertimm by Brice Ruzand
 * Dev : January 2014
 * Version 2.0
 */
#include "ogm_sysi.h"
#include <execinfo.h>

/**
 * Log a backtrace if an error occurred sysi for better investigation
 *
 * @param ctrl_sysi sysihandle
 * @param msg log message
 * @param rwl_status status code to convert into an errro message
 * @return execution status
 */
og_status OgSysiLogError(struct og_ctrl_sysi *ctrl_sysi, unsigned char *msg, int rwl_status)
{
  // level dependent
  int levelFlag = DOgMsgDestInLog + DOgMsgSeverityError;
  char *levelText = "[ERROR]";
  char textBuffer[DOgErrorSize];

  char *status_msg = rwl_status != 0 ? strerror(rwl_status) : "Ok";

  snprintf(textBuffer, DOgErrorSize, "OgSysiLogError : %s - %s : %s (%d)\n", ctrl_sysi->name, msg, status_msg,
      rwl_status);

  {
    // print backtrace for investigation

    void * DPT_trace[255];
    int DPT_i = 0, DPT_trace_size = 255;
    char **DPT_messages = NULL;
    DPT_trace_size = backtrace(DPT_trace, sizeof(DPT_trace));
    DPT_messages = backtrace_symbols(DPT_trace, DPT_trace_size);
    for (DPT_i = 0; DPT_i < DPT_trace_size; DPT_i++)
    {
      int prev_size = strlen(textBuffer);
      snprintf(textBuffer + prev_size, DOgErrorSize - prev_size, "\t%s\n", DPT_messages[DPT_i]);
    }
    free(DPT_messages);
  }


  {
    // print message to see it in test log
    time_t ltime;
    time(&ltime);

    printf("%s > %s", OgGmtime(&ltime), textBuffer);
  }

  // log message
  OgMsg(ctrl_sysi->hmsg, levelText, levelFlag, textBuffer);

  // add to error to stack
  OgErr(ctrl_sysi->herr, textBuffer);

  DONE;

}

/**
 * Enable or disable RW lock stat on flush
 *
 * @param handle handle
 * @param enable boolean to enable or disable log
 * @return execution status
 */
PUBLIC(og_status) OgSysiLogStatEnable(ogsysi_rwlock handle, int enable)
{
  struct og_ctrl_sysi *ctrl_sysi = (struct og_ctrl_sysi *) handle;
  if (ctrl_sysi == NULL) CONT;

  if (ctrl_sysi->log_rw_lock_stat_on_flush && !enable)
  {
    og_char_buffer buffer[DOgErrorSize];
    snprintf(buffer, DOgErrorSize, "OgSysiLogStatEnable: disable rwlock stat on '%s'", ctrl_sysi->name);
    OgMsg(ctrl_sysi->hmsg, "[INFO]", DOgMsgDestInLog + DOgMsgSeverityDebug, buffer);
  }
  else if (!ctrl_sysi->log_rw_lock_stat_on_flush && enable)
  {
    og_char_buffer buffer[DOgErrorSize];
    snprintf(buffer, DOgErrorSize, "OgSysiLogStatEnable: enable rwlock stat on '%s'", ctrl_sysi->name);
    OgMsg(ctrl_sysi->hmsg, "[INFO]", DOgMsgDestInLog + DOgMsgSeverityDebug, buffer);
  }

  // change rw lock log status
  ctrl_sysi->log_rw_lock_stat_on_flush = enable;

  DONE;
}

/**
 * Log lock wait statistic
 *
 * @param handle sysi handle
 * @return execution status
 */
PUBLIC(og_status) OgSysiLogStat(ogsysi_rwlock handle)
{

  struct og_ctrl_sysi *ctrl_sysi = (struct og_ctrl_sysi *) handle;

  if (ctrl_sysi->log_rw_lock_stat_on_flush)
  {

    // level dependent
    int levelFlag = DOgMsgDestInLog + DOgMsgSeverityDebug;
    og_char_buffer * levelText = "[DEBUG]";
    og_char_buffer buffer[DOgErrorSize];
    og_char_buffer read_cmpt[128];
    og_char_buffer read_wait_elapsed[128];
    og_char_buffer write_cmpt[128];
    og_char_buffer write_wait_elapsed[128];
    og_char_buffer write_owned_elapsed[128];

    IFE(OgFormatThousand(ctrl_sysi->read_cmpt, read_cmpt));
    IFE(OgFormatThousand(ctrl_sysi->read_wait_elapsed, read_wait_elapsed));
    IFE(OgFormatThousand(ctrl_sysi->write_cmpt, write_cmpt));
    IFE(OgFormatThousand(ctrl_sysi->write_wait_elapsed, write_wait_elapsed));
    IFE(OgFormatThousand(ctrl_sysi->write_owned_elapsed, write_owned_elapsed));

    snprintf(buffer, DOgErrorSize, "OgSysiLogStat: rwlock '%s': "
        "R[access: %s times ; wait: %s µs] "
        "W[access: %s times; wait: %s µs; own: %s µs]", ctrl_sysi->name, read_cmpt, read_wait_elapsed, write_cmpt,
        write_wait_elapsed, write_owned_elapsed);
    IFE(OgMsg(ctrl_sysi->hmsg, levelText, levelFlag, buffer));

  }

  DONE;
}

/**
 * Advanced log trace to investigate deadlock
 *
 * @param ctrl_sysi sysihandle
 * @param action log message : READ/WRITE ...
 * @param nb_read current number read lock owned
 * @param nb_write  current number write lock owned
 * @return execution status
 */
og_status OgSysiLogDebug(struct og_ctrl_sysi *ctrl_sysi, unsigned char *action, int nb_read, int nb_write)
{
  // level dependent
  int levelFlag = DOgMsgDestInLog + DOgMsgSeverityDebug;
  char *levelText = "[DEBUG]";
  char textBuffer[DOgErrorSize];
  unsigned int TID = (unsigned int) pthread_self();

  snprintf(textBuffer, DOgErrorSize, "OgSysiLogDebug : %s (R:%d W:%d TID:%X)\n", action, nb_read, nb_write, TID);

  {
    // print backtrace for investigation

    void * DPT_trace[255];
    int DPT_i = 0, DPT_trace_size = 255;
    char **DPT_messages = NULL;
    int prev_size = 0;
    DPT_trace_size = backtrace(DPT_trace, sizeof(DPT_trace));
    DPT_messages = backtrace_symbols(DPT_trace, DPT_trace_size);

    for (DPT_i = 0; DPT_i < DPT_trace_size; DPT_i++)
    {
      prev_size = strlen(textBuffer);
      snprintf(textBuffer + prev_size, DOgErrorSize - prev_size - 4, "\t%s\n", DPT_messages[DPT_i]);
    }

    prev_size = strlen(textBuffer);
    snprintf(textBuffer + prev_size, DOgErrorSize - prev_size, "...\n");

    free(DPT_messages);
  }

  // log message
  OgMsg(ctrl_sysi->hmsg, levelText, levelFlag, textBuffer);

  DONE;

}

PUBLIC(og_string) OgSysiGetName(ogsysi_rwlock handle)
{
  struct og_ctrl_sysi *ctrl_sysi = (struct og_ctrl_sysi *) handle;

  return ctrl_sysi->name;
}
