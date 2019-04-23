/*
 *  The server read API function
 *  Copyright (c) 2006 Pertimm by Patrick Constant
 *  Dev : July 2006, January 2007
 *  Version 1.1
 */
#include "ogm_uci.h"

PUBLIC(int) OgUciServerRead(void *handle, struct og_ucisr_input *input, struct og_ucisr_output *output)
{
  struct og_ctrl_ucis *ctrl_ucis = (struct og_ctrl_ucis *) handle;
  struct og_ucir_output cucir_output, *ucir_output = &cucir_output;
  struct og_ucir_input cucir_input, *ucir_input = &cucir_input;

  memset(ucir_input, 0, sizeof(struct og_ucir_input));
  ucir_input->hsocket = input->hsocket;
  ucir_input->timeout = input->timeout;

  memset(output, 0, sizeof(struct og_ucisr_output));

  IF(OgUciRead(ctrl_ucis->huci,ucir_input,ucir_output))
  {
    output->timed_out = ucir_output->timed_out;
    DPcErr;
  }

  output->hh = ucir_output->hh;
  strcpy(output->top_level_tag, ucir_output->top_level_tag);
  output->content = ucir_output->content;
  output->content_length = ucir_output->content_length;
  output->header_length = ucir_output->header_length;
  output->elapsed_total = ucir_output->elapsed_total;
  output->elapsed_recv = ucir_output->elapsed_recv;

  DONE;
}


PUBLIC(og_status) OgUciServerReadReset(void *handle, og_bool reset_to_minimal)
{
  struct og_ctrl_ucis *ctrl_ucis = (struct og_ctrl_ucis *) handle;
  struct og_ctrl_uci *ctrl_uci = (struct og_ctrl_uci *) ctrl_ucis->huci;

  if (reset_to_minimal)
  {
    IFE(OgHeapResetToMinimal(ctrl_uci->hba));
  }
  else
  {
    IFE(OgHeapReset(ctrl_uci->hba));
  }

  DONE;
}
