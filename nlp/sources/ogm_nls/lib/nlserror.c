/*
 *  Handling errors for listening lt function.
 *  Copyright (c) 2006 Pertimm by Patrick Constant
 *  Dev : October 2006
 *  Version 1.0
 */
#include "ogm_nls.h"

static og_status split_error_message(json_t *errors, char *multiple_errors);

/*
 *  Handling errors for listening threads.
 */
og_status OgListeningThreadError(struct og_listening_thread *lt)
{
  json_t *root = lt->response->body;
  if (root == NULL || ((json_is_object(root) && json_object_size(root) > 0) || json_is_array(root)))
  {
    // erase preview root;
    json_decrefp(&root);
    lt->response->body = json_object();
    root = lt->response->body;
  }

  json_t *errors = json_array();
  json_object_set_new(root, "errors", errors);

  char erreur[DOgErrorSize];
  while (OgErrLast(lt->herr, erreur, 0))
  {
    split_error_message(errors, erreur);
  }

  int h = 0;
  while (PcErrDiag(&h, erreur))
  {
    split_error_message(errors, erreur);
  }

  if (json_array_size(errors) == 0)
  {
    json_array_append_new(errors, json_string("Unexpected errors"));
  }

  unsigned char *response = json_dumps(root, JSON_INDENT(2));

  struct og_ucisw_input winput[1];
  memset(winput, 0, sizeof(struct og_ucisw_input));
  winput->http_status = 500;
  winput->http_status_message = "Internal Server Error";
  winput->hsocket = lt->hsocket_in;
  winput->content_type = "application/json";
  winput->content_length = strlen(response);
  winput->content = response;

  IF(OgUciServerWrite(lt->hucis,winput))
  {
    OgMsg(lt->hmsg, "", DOgMsgDestInLog + DOgMsgDestInErr,
        "OgUciServerWrite (%d): connexion was prematurely closed by client on an error message, giving up", lt->ID);
  }

  DPcFree(response);

  DONE;
}

static og_status split_error_message(json_t *json_errors, char *multiple_errors)
{
  if (multiple_errors == NULL) CONT;

  char *saveptr = NULL;
  char *errors = multiple_errors;
  char *error_line = strtok_r(errors, "\n", &saveptr);
  while (error_line != NULL)
  {

    json_array_append_new(json_errors, json_string(error_line));

    error_line = strtok_r(NULL, "\n", &saveptr);
  }

  DONE;
}

/**
 * Add Error to error stack and log it
 */
og_status NlsThrowError(struct og_listening_thread *lt, og_string format, ...)
{

  // level dependent
  int levelFlag = DOgMsgDestInLog + DOgMsgSeverityError;
  og_char_buffer *levelText = "[ERROR]";

  // consistency checking
  IFN(lt) DPcErr;

  // prefix with lt number
  og_char_buffer format_extended[DPcPathSize];
  snprintf(format_extended, DPcPathSize, "lt %d: %s", lt->ID, format);

  og_char_buffer textBuffer[DOgErrorSize];
  va_list vl;

  // var_args processing
  va_start(vl, format);
  vsnprintf(textBuffer, DOgErrorSize, format_extended, vl);
  va_end(vl);

  // log message
  IFE(OgMsg(lt->hmsg, levelText, levelFlag, textBuffer));

  // add to error to stack
  return OgErr(lt->herr, textBuffer);

}

og_status NlsJSONThrowError(struct og_listening_thread *lt, og_string function_name, json_error_t * error)
{
  NlsThrowError(lt, "%s : Your json contains error in ligne %d and column %d , %s , %s ", function_name, error->line,
      error->column, error->source, error->text);
  DONE;

}

og_status NlsMainThrowError(og_nls ctrl_nls, og_string format, ...)
{

  // level dependent
  int levelFlag = DOgMsgDestInLog + DOgMsgSeverityError;
  og_char_buffer *levelText = "[ERROR]";

  // consistency checking
  IFN(ctrl_nls) DPcErr;

  // prefix with lt number
  og_char_buffer format_extended[DPcPathSize];
  snprintf(format_extended, DPcPathSize, "main: %s", format);

  og_char_buffer textBuffer[DOgErrorSize];
  va_list vl;

  // var_args processing
  va_start(vl, format);
  vsnprintf(textBuffer, DOgErrorSize, format_extended, vl);
  va_end(vl);

  // log message
  IFE(OgMsg(ctrl_nls->hmsg, levelText, levelFlag, textBuffer));

  // add to error to stack
  return OgErr(ctrl_nls->herr, textBuffer);

}

