/*
 * interpretjson.c
 *
 *  Created on: 7 sept. 2017
 *      Author: sebastien Manfredini
 */

#include "ogm_nls.h"

#include <stdarg.h>

static void heap_wrapper_yajl_printer(void * ctx, const char * str, size_t len);
static og_status yajlGenErrorManagement(struct og_listening_thread *lt, og_string function_name, yajl_gen_status status);

/**
 * Alloc and init lt->json->yajl
 *
 * @param lt og_listening_thread
 * @return status
 */
og_status OgNLSJsonInit(struct og_listening_thread *lt)
{
  struct json_object *json = lt->json;

  json->hb_json_buffer = OgHeapInit(lt->hmsg, "interpret_json_buffer", 1, DPcPathSize * 4);
  IFN(json->hb_json_buffer) DPcErr;

  // Alloc Yajl instance
  json->yajl_gen = yajl_gen_alloc(NULL);

  if (json->yajl_gen == NULL)
  {
    NlsThrowError(lt, "OgNLSJsonYajlInit : "
        "yajl_gen_alloc on json->yajl failed.");
    DPcErr;
  }

  // set options
  yajl_gen_config(json->yajl_gen, yajl_gen_print_callback, heap_wrapper_yajl_printer, lt);
  yajl_gen_config(json->yajl_gen, yajl_gen_validate_utf8, TRUE);
  yajl_gen_config(json->yajl_gen, yajl_gen_beautify, TRUE);
  yajl_gen_config(json->yajl_gen, yajl_gen_indent_string, "  ");

  DONE;
}

/**
 * Reset lt->json->yajl
 *
 * @param lt og_listening_thread
 * @return status
 */
og_status OgNLSJsonReset(struct og_listening_thread *lt)
{
  // reset state
  yajl_gen_reset(lt->json->yajl_gen, NULL);

  // reset buffer
  IFE(OgHeapReset(lt->json->hb_json_buffer));

  DONE;
}

/**
 * Reset  lt->json->yajl and fulsh hb_json_gen_heap buffer.
 *
 * @param lt og_listening_thread
 * @return status
 */
og_status OgNLSJsonFlush(struct og_listening_thread *lt)
{
  if (lt->json->yajl_gen != NULL)
  {
    yajl_gen_free(lt->json->yajl_gen);
    lt->json->yajl_gen = NULL;
  }

  // reset buffer
  IFE(OgHeapFlush(lt->json->hb_json_buffer));

  DONE;
}

og_status OgNLSJsonGenInteger(struct og_listening_thread *lt, long long int number)
{
  yajl_gen gen = lt->json->yajl_gen;

  // Previously detected error
  if (lt->json->error_detected)
  {
    DPcErr;
  }

  // get status
  yajl_gen_status status = yajl_gen_integer(gen, number);

  IFE(yajlGenErrorManagement(lt, "OgNLSJsonGenInteger", status));

  DONE;
}

og_status OgNLSJsonGenDouble(struct og_listening_thread *lt, double number)
{
  yajl_gen gen = lt->json->yajl_gen;

  // Previously detected error
  if (lt->json->error_detected)
  {
    DPcErr;
  }

  // get status
  yajl_gen_status status = yajl_gen_double(gen, number);

  IFE(yajlGenErrorManagement(lt, "OgNLSJsonGenDouble", status));

  DONE;
}

og_status OgNLSJsonGenNumber(struct og_listening_thread *lt, og_string number, int len)
{
  yajl_gen gen = lt->json->yajl_gen;

  // Previously detected error
  if (lt->json->error_detected)
  {
    DPcErr;
  }

  // get status
  yajl_gen_status status = yajl_gen_number(gen, number, (size_t) len);

  IFE(yajlGenErrorManagement(lt, "OgNLSJsonGenNumber", status));

  DONE;
}

og_status OgNLSJsonGenBool(struct og_listening_thread *lt, og_bool boolean)
{
  yajl_gen gen = lt->json->yajl_gen;

  // Previously detected error
  if (lt->json->error_detected)
  {
    DPcErr;
  }

  // get status
  yajl_gen_status status = yajl_gen_bool(gen, boolean);

  IFE(yajlGenErrorManagement(lt, "OgNLSJsonGenBool", status));

  DONE;
}

og_status OgNLSJsonGenString(struct og_listening_thread *lt, og_string string)
{
  yajl_gen gen = lt->json->yajl_gen;

  // Previously detected error
  if (lt->json->error_detected)
  {
    DPcErr;
  }

  // get status
  yajl_gen_status status = yajl_gen_string(gen, string, strlen(string));

  IFE(yajlGenErrorManagement(lt, "OgNLSJsonGenString", status));

  DONE;
}

og_status OgNLSJsonGenStringSized(struct og_listening_thread *lt, og_string string, int length)
{
  yajl_gen gen = lt->json->yajl_gen;

  // Previously detected error
  if (lt->json->error_detected)
  {
    DPcErr;
  }

  // get status
  yajl_gen_status status = yajl_gen_string(gen, string, length);

  IFE(yajlGenErrorManagement(lt, "OgNLSJsonGenStringSized", status));

  DONE;
}

//og_status OgNLSJsonGenStringUnicode(struct og_listening_thread *lt, og_string string_unicode, int length)
//{
//  yajl_gen gen = lt->json->yajl_gen;
//
//  // Previously detected error
//  if (lt->json->error_detected)
//  {
//    DPcErr;
//  }
//
//  og_char_buffer buffer_utf8[DPcPathSize];
//  int ibuffer_utf8 = 0;
//  IFE(OgUniToCp(length, string_unicode, DPcPathSize, &ibuffer_utf8, buffer_utf8, DOgCodePageUTF8, 0, 0));
//
//  // get status
//  yajl_gen_status status = yajl_gen_string(gen, buffer_utf8, ibuffer_utf8);
//
//  IFE(yajlGenErrorManagement(lt, "OgNLSJsonGenStringUnicode", status));
//
//  DONE;
//}

og_status OgNLSJsonGenKeyValueString(struct og_listening_thread *lt, og_string key, og_string value_string)
{

  IFE(OgNLSJsonGenString(lt, key));
  IFE(OgNLSJsonGenString(lt, value_string));

  DONE;
}

og_status OgNLSJsonGenKeyValueStringSized(struct og_listening_thread *lt, og_string key, og_string value_string,
    int length)
{

  IFE(OgNLSJsonGenString(lt, key));
  IFE(OgNLSJsonGenStringSized(lt, value_string, length));

  DONE;
}

//og_status OgNLSJsonGenKeyValueStringUnicode(struct og_listening_thread *lt, og_string key,
//    og_string value_string_unicode, int length)
//{
//
//  IFE(OgNLSJsonGenString(lt, key));
//  IFE(OgNLSJsonGenStringUnicode(lt, value_string_unicode, length));
//
//  DONE;
//}

og_status OgNLSJsonGenKeyValueInteger(struct og_listening_thread *lt, og_string key, int number)
{

  IFE(OgNLSJsonGenString(lt, key));
  IFE(OgNLSJsonGenInteger(lt, number));

  DONE;
}

og_status OgNLSJsonGenKeyValueDouble(struct og_listening_thread *lt, og_string key, double number)
{

  IFE(OgNLSJsonGenString(lt, key));
  IFE(OgNLSJsonGenDouble(lt, number));

  DONE;
}

og_status OgNLSJsonGenNull(struct og_listening_thread *lt)
{
  yajl_gen gen = lt->json->yajl_gen;

  // Previously detected error
  if (lt->json->error_detected)
  {
    DPcErr;
  }

  // get status
  yajl_gen_status status = yajl_gen_null(gen);

  IFE(yajlGenErrorManagement(lt, "OgNLSJsonGenNull", status));

  DONE;
}

og_status OgNLSJsonGenArrayOpen(struct og_listening_thread *lt)
{
  yajl_gen gen = lt->json->yajl_gen;

  // Previously detected error
  if (lt->json->error_detected)
  {
    DPcErr;
  }

  // get status
  yajl_gen_status status = yajl_gen_array_open(gen);

  IFE(yajlGenErrorManagement(lt, "OgNLSJsonGenArrayOpen", status));

  DONE;
}

og_status OgNLSJsonGenArrayClose(struct og_listening_thread *lt)
{
  yajl_gen gen = lt->json->yajl_gen;

  // Previously detected error
  if (lt->json->error_detected)
  {
    DPcErr;
  }

  // get status
  yajl_gen_status status = yajl_gen_array_close(gen);

  IFE(yajlGenErrorManagement(lt, "OgNLSJsonGenArrayClose", status));

  DONE;
}

og_status OgNLSJsonGenMapOpen(struct og_listening_thread *lt)
{
  yajl_gen gen = lt->json->yajl_gen;

  // Previously detected error
  if (lt->json->error_detected)
  {
    DPcErr;
  }

  // get status
  yajl_gen_status status = yajl_gen_map_open(gen);

  IFE(yajlGenErrorManagement(lt, "OgNLSJsonGenMapOpen", status));

  DONE;
}

og_status OgNLSJsonGenMapClose(struct og_listening_thread *lt)
{
  yajl_gen gen = lt->json->yajl_gen;

  // Previously detected error
  if (lt->json->error_detected)
  {
    DPcErr;
  }

  // get status
  yajl_gen_status status = yajl_gen_map_close(gen);

  IFE(yajlGenErrorManagement(lt, "OgNLSJsonGenMapClose", status));

  DONE;
}

/**
 * Generate error from yajl status
 *
 * @param lt
 * @param function_name current function name to provied better error
 * @param status yajl status
 * @return ssrv status
 */
static og_status yajlGenErrorManagement(struct og_listening_thread *lt, og_string function_name, yajl_gen_status status)
{
  // Previously detected error
  if (lt->json->error_detected)
  {
    DPcErr;
  }

  og_char_buffer *buffer;
  switch (status)
  {
    case yajl_gen_status_ok:
      DONE;
    case yajl_gen_keys_must_be_strings:
      buffer = "yajl_gen_keys_must_be_strings : at a point where a map key is generated, a function other than"
          " yajl_gen_string was called";
      break;
    case yajl_max_depth_exceeded:
      buffer = "yajl_max_depth_exceeded : YAJL's maximum generation depth was exceeded.  see  YAJL_MAX_DEPTH";
      break;
    case yajl_gen_in_error_state:
      buffer = "yajl_gen_in_error_state : A generator function (yajl_gen_XXX) was called while in an error state";
      break;
    case yajl_gen_generation_complete:
      buffer = "yajl_gen_generation_complete : A complete JSON document has been generated";
      break;
    case yajl_gen_invalid_number:
      buffer =
          "yajl_gen_invalid_number : yajl_gen_double was passed an invalid floating point value (infinity or NaN).";
      break;
    case yajl_gen_no_buf:
      buffer = "yajl_gen_no_buf : A print callback was passed in, so there is no internal buffer to get from";
      break;
    case yajl_gen_invalid_string:
      buffer = "yajl_gen_invalid_string : returned from yajl_gen_string() when the yajl_gen_validate_utf8 option is"
          " enabled and an invalid was passed by client code.";
      break;
    default:
      buffer = "unknown error";
      break;
  }

  lt->json->error_detected = TRUE;

  NlsThrowError(lt, "Error during json generation in '%s' (%d) : %s", function_name, status, buffer);

  DPcErr;
}

/**
 * A callback used for "printing" the results in lt->hbn.
 */
static void heap_wrapper_yajl_printer(void * ctx, const char * str, size_t len)
{
  struct og_listening_thread *lt = (struct og_listening_thread *) ctx;

  // Append content in TAS buffer
  IF(OgHeapAppend(lt->json->hb_json_buffer, len, (void *) str))
  {
    NlsThrowError(lt, "heap_wrapper_yajl_printer : error while calling OgHeapAppend");
  }
}

// =====================================================================
//
// REFORMAT JSON CONTENT
//
// =====================================================================

static int reformat_null(void * ctx)
{
  yajl_gen g = (yajl_gen) ctx;
  yajl_gen_status s = yajl_gen_null(g);
  return s == yajl_gen_status_ok;
}

static int reformat_boolean(void * ctx, int boolean)
{
  yajl_gen g = (yajl_gen) ctx;
  yajl_gen_status s = yajl_gen_bool(g, boolean);
  return s == yajl_gen_status_ok;
}

static int reformat_integer(void * ctx, long long integerVal)
{
  yajl_gen g = (yajl_gen) ctx;
  yajl_gen_status s = yajl_gen_integer(g, integerVal);
  return s == yajl_gen_status_ok;
}

static int reformat_double(void * ctx, double doubleVal)
{
  yajl_gen g = (yajl_gen) ctx;
  yajl_gen_status s = yajl_gen_double(g, doubleVal);
  return s == yajl_gen_status_ok;
}

static int reformat_number(void * ctx, const char * numberVal, size_t l)
{
  yajl_gen g = (yajl_gen) ctx;
  yajl_gen_status s = yajl_gen_number(g, numberVal, l);
  return s == yajl_gen_status_ok;
}

static int reformat_string(void * ctx, const unsigned char * stringVal, size_t stringLen)
{
  yajl_gen g = (yajl_gen) ctx;
  yajl_gen_status s = yajl_gen_string(g, stringVal, stringLen);
  return s == yajl_gen_status_ok;
}

static int reformat_map_key(void * ctx, const unsigned char * stringVal, size_t stringLen)
{
  yajl_gen g = (yajl_gen) ctx;
  yajl_gen_status s = yajl_gen_string(g, stringVal, stringLen);
  return s == yajl_gen_status_ok;
}

static int reformat_start_map(void * ctx)
{
  yajl_gen g = (yajl_gen) ctx;
  yajl_gen_status s = yajl_gen_map_open(g);
  return s == yajl_gen_status_ok;
}

static int reformat_end_map(void * ctx)
{
  yajl_gen g = (yajl_gen) ctx;
  yajl_gen_status s = yajl_gen_map_close(g);
  return s == yajl_gen_status_ok;
}

static int reformat_start_array(void * ctx)
{
  yajl_gen g = (yajl_gen) ctx;
  yajl_gen_status s = yajl_gen_array_open(g);
  return s == yajl_gen_status_ok;
}

static int reformat_end_array(void * ctx)
{
  yajl_gen g = (yajl_gen) ctx;
  yajl_gen_status s = yajl_gen_array_close(g);
  return s == yajl_gen_status_ok;
}

// =====================================================================
//
// Getting JSON data
//
// =====================================================================

int get_null(void * ctx)
{

  return TRUE;
}

int get_boolean(void * ctx, int boolean)
{

  return TRUE;
}

int get_integer(void * ctx, long long integerVal)
{

  return TRUE;
}

int get_double(void * ctx, double doubleVal)
{

  return TRUE;
}

int get_number(void * ctx, const char * numberVal, size_t l)
{

  return TRUE;
}

int get_string(void * ctx, const unsigned char * stringVal, size_t stringLen)
{
  struct jsonValuesContext *jsonCtx = ctx;
  int iStringLen = (int) stringLen;
  if (strcmp(jsonCtx->mapKey, "name") == 0)
  {
    snprintf(jsonCtx->stringValue, DPcPathSize, "%.*s", iStringLen, stringVal);
  }

  if (strcmp(jsonCtx->mapKey, "name") == 0)
  {
    og_char_buffer tmpString[DPcPathSize];
    if (strcmp(jsonCtx->stringValue, "Zorglub") == 0)
    {
      snprintf(tmpString, DPcPathSize, "Eviv Bulgroz!");
    }
    else
    {
      snprintf(tmpString, DPcPathSize, "Hello %.*s", iStringLen, jsonCtx->stringValue);
    }
    IFE(OgNLSJsonGenKeyValueString(jsonCtx->lt, "answer", tmpString));
  }
  else
  {
    IFE(OgNLSJsonGenKeyValueString(jsonCtx->lt, "answer", "Greuh !!!"));
  }

  return TRUE;
}

int get_map_key(void * ctx, const unsigned char * stringVal, size_t stringLen)
{
  struct jsonValuesContext *jsonCtx = ctx;
  int iStringLen = (int) stringLen;
  snprintf(jsonCtx->mapKey, DPcPathSize, "%.*s", iStringLen, stringVal);
  return TRUE;
}

int get_start_map(void * ctx)
{

  return TRUE;
}

int get_end_map(void * ctx)
{

  return TRUE;
}

int get_start_array(void * ctx)
{

  return TRUE;
}

int get_end_array(void * ctx)
{

  return TRUE;
}

/**
 * Generate error from yajl status
 *
 * @param lt og_listening_thread
 * @param function_name current function name to provied better error
 * @param status yajl status
 * @return ssrv status
 */
static og_status yajlParseErrorManagement(struct og_listening_thread *lt, yajl_handle parser, yajl_status status,
    og_string json, size_t json_size)
{
  // Previously detected error
  if (lt->json->error_detected)
  {
    DPcErr;
  }

  og_char_buffer *buffer;
  switch (status)
  {
    case yajl_status_ok:
      DONE;
    case yajl_status_client_canceled:
      buffer = "yajl_status_client_canceled : a client callback returned zero, stopping the parse";
      break;
    case yajl_status_error:
      buffer = "yajl_status_error : An error occured during the parse.\n";
      break;
    default:
      buffer = "unknow error";
      break;
  }

  unsigned char *error_message = yajl_get_error(parser, TRUE, json, json_size);
  if (error_message == NULL)
  {
    error_message = "";
  }

  NlsThrowError(lt, "Error during json reformat (%d) : %s%s", status, buffer, error_message);

  DPcErr;
}

static yajl_callbacks parser_callbacks = { reformat_null, reformat_boolean, reformat_integer, reformat_double,
    reformat_number, reformat_string, reformat_start_map, reformat_map_key, reformat_end_map, reformat_start_array,
    reformat_end_array };

static yajl_callbacks get_callbacks = { get_null, get_boolean, get_integer, get_double, get_number, get_string,
    get_start_map, get_map_key, get_end_map, get_start_array, get_end_array };

/**
 * Refomat json
 */
og_status OgNLSJsonReFormat(struct og_listening_thread *lt, og_string json, size_t json_size)
{

  yajl_handle parser = yajl_alloc(&parser_callbacks, NULL, lt->json->yajl_gen);

  yajl_status yajl_parser_status = yajl_parse(parser, json, json_size);
  og_status status = yajlParseErrorManagement(lt, parser, yajl_parser_status, json, json_size);
  NIF(status)
  {
    yajl_parser_status = yajl_complete_parse(parser);
    status = yajlParseErrorManagement(lt, parser, yajl_parser_status, json, json_size);
  }

  yajl_free(parser);

  return status;
}

/**
 * Getting data from JSon
 */
og_status OgNLSJsonAnswer(struct og_listening_thread *lt, og_string json, size_t json_size)
{
  IFE(OgNLSJsonReset(lt));

  IFE(OgNLSJsonGenMapOpen(lt));

  struct jsonValuesContext jsonCtx;
  jsonCtx.lt = lt;

  yajl_handle parser = yajl_alloc(&get_callbacks, NULL, &jsonCtx);

  yajl_status yajl_parser_status = yajl_parse(parser, json, json_size);
  og_status status = yajlParseErrorManagement(lt, parser, yajl_parser_status, json, json_size);

  NIF(status)
  {
    yajl_parser_status = yajl_complete_parse(parser);
    status = yajlParseErrorManagement(lt, parser, yajl_parser_status, json, json_size);
  }

  IFE(OgNLSJsonGenMapClose(lt));

  yajl_free(parser);

  return status;
}
