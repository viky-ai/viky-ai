/*
 *  Handling why an expression or an interpretation has not been found, create the json structure
 *  Copyright (c) 2017 Pertimm, by Patrick Constant
 *  Dev : November 2017
 *  Version 1.0
 */
#include "ogm_nlp.h"

og_status NlpWarningReset(og_nlp_th ctrl_nlp_th)
{
  ctrl_nlp_th->nb_warnings = 0;
  DONE;
}

og_status NlpWarningAdd(og_nlp_th ctrl_nlp_th, og_string format, ...)
{
  og_char_buffer textBuffer[DOgErrorSize];
  va_list vl;

  // var_args processing
  va_start(vl, format);
  vsnprintf(textBuffer, DOgErrorSize, format, vl);
  va_end(vl);

  IFn(ctrl_nlp_th->nb_warnings)
  {
    ctrl_nlp_th->json_warnings = json_array();
    IF(json_object_set_new(ctrl_nlp_th->json_answer_unit, "warnings", ctrl_nlp_th->json_warnings))
    {
      NlpThrowErrorTh(ctrl_nlp_th, "NlpWarningAdd: error setting json_warnings");
      DPcErr;
    }
  }

  json_t *json_warning = json_string(textBuffer);
  IF(json_array_append_new(ctrl_nlp_th->json_warnings, json_warning))
  {
    NlpThrowErrorTh(ctrl_nlp_th, "NlpWarningAdd : error appending json_warning");
    DPcErr;
  }

  ctrl_nlp_th->nb_warnings++;

  DONE;

}

