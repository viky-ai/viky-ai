/*
 *  Handling error messages for Natural Language Processing library
 *  Copyright (c) 2017 Pertimm, by Patrick Constant
 *  Dev : September 2017
 *  Version 1.0
 */
#include "ogm_nlp.h"


/**
 * Add Error to error stack and log it
 */
og_status NlpThrowError(og_nlp ctrl_nlp, og_string format, ...)
{

  // level dependent
  int levelFlag = DOgMsgDestInLog + DOgMsgSeverityError;
  og_char_buffer *levelText = "[ERROR]";

  // consistency checking
  IFN(ctrl_nlp) DPcErr;

  og_char_buffer textBuffer[DOgErrorSize];
  va_list vl;

  // var_args processing
  va_start(vl, format);
  vsnprintf(textBuffer, DOgErrorSize, format, vl);
  va_end(vl);

  // log message
  IFE(OgMsg(ctrl_nlp->hmsg, levelText, levelFlag, textBuffer));

  // add to error to stack
  return OgErr(ctrl_nlp->herr, textBuffer);

}



/**
 * Add Error to error stack and log it
 */
og_status NlpThrowErrorTh(og_nlp_th ctrl_nlp_th, og_string format, ...)
{

  // level dependent
  int levelFlag = DOgMsgDestInLog + DOgMsgSeverityError;
  og_char_buffer *levelText = "[ERROR]";

  // consistency checking
  IFN(ctrl_nlp_th) DPcErr;

  og_char_buffer textBuffer[DOgErrorSize];
  va_list vl;

  // var_args processing
  va_start(vl, format);
  vsnprintf(textBuffer, DOgErrorSize, format, vl);
  va_end(vl);

  // log message
  IFE(OgMsg(ctrl_nlp_th->hmsg, levelText, levelFlag, textBuffer));

  // add to error to stack
  return OgErr(ctrl_nlp_th->herr, textBuffer);

}



