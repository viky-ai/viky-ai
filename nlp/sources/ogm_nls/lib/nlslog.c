/*
 *  Logging functions.
 *  Copyright (c) 2006 Pertimm by Patrick Constant
 *  Dev : August, December 2006
 *  Version 1.1
 */
#include "ogm_nls.h"
#include <stdio.h>

#define DOgNlsLogTruncatedBufferSize 64
#define DOgNlsLogLabelBufferSize     64

og_status NlsRequestLog(struct og_listening_thread *lt, og_string function_name, og_string label,
    int additional_log_flags)
{
  int is = lt->output->content_length - lt->output->header_length;

  IFN(lt->output->content) DONE;
  og_string s = lt->output->content + lt->output->header_length;

  og_char_buffer truncated_buffer[DOgNlsLogTruncatedBufferSize];
  truncated_buffer[0] = 0;

  og_char_buffer label_buffer[DOgNlsLogTruncatedBufferSize];
  label_buffer[0] = 0;

  int max_log_size = is;

  if (max_log_size > DOgMlogMaxMessageSize / 2)
  {
    max_log_size = DOgMlogMaxMessageSize / 2;
    snprintf(truncated_buffer, DOgNlsLogTruncatedBufferSize, " (log truncated)");
  }

  if (label != NULL && label[0] != 0)
  {
    snprintf(label_buffer, DOgNlsLogLabelBufferSize, " for '%s'", label);
  }

  OgMsg(lt->hmsg, "", DOgMsgDestInLog + additional_log_flags, "lt %d: %s%s: request is%s:[\n%.*s]", lt->ID,
      function_name, label_buffer, truncated_buffer, max_log_size, s);

  DONE;
}

