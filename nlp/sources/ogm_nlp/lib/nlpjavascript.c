/*
 *  Eval javascript snipets
 *  Copyright (c) 2017 Pertimm, by Brice Ruzand
 *  Dev : Novembre 2017
 *  Version 1.0
 */

#include "ogm_nlp.h"
#include "duk_module_node.h"
#include "duk_console.h"
#include <iconv.h>

static og_string NlpJsDukTypeString(duk_int_t type);
static duk_ret_t NlpJsInitLoadModule(duk_context *ctx);
static duk_ret_t NlpJsInitResolvModule(duk_context *ctx);
static duk_ret_t push_file_as_string(duk_context *ctx, og_string filename);
static og_status NlpJsLoadLibMoment(og_nlp_th ctrl_nlp_th);
static og_status NlpJsInitIsolatedEval(og_nlp_th ctrl_nlp_th);
static og_status NlpJsDukCESU8toUTF8(og_nlp_th ctrl_nlp_th, og_string cesu, int cesu_length, og_string *utf8);

#define DOgNlpJsMomentSecretName "moment_lib_%06X"

static void NlpJsDuketapeErrorHandler(void *udata, const char *msg)
{
  og_nlp_th ctrl_nlp_th = udata;

  NlpThrowErrorTh(ctrl_nlp_th, "NlpJsDuketapeErrorHandler: %s", msg);

  abort();
}

og_status NlpJsInit(og_nlp_th ctrl_nlp_th)
{
  struct og_ctrl_nlp_js *js = ctrl_nlp_th->js;

  og_char_buffer heap_name[DPcPathSize];
  snprintf(heap_name, DPcPathSize, "%s_js_buffer_heap", ctrl_nlp_th->name);
  js->buffer = OgHeapInit(ctrl_nlp_th->hmsg, heap_name, sizeof(og_char_buffer), 256);
  IFN(js->buffer)
  {
    NlpThrowErrorTh(ctrl_nlp_th, "NlpJsInit : error on OgHeapInit(%s)", heap_name);
    DPcErr;
  }

  js->variables = g_string_chunk_new(sizeof(unsigned char));
  g_queue_init(js->variables_name_list);
  g_queue_init(js->variables_values);

  GRand *random_generator = g_rand_new();
  js->random_number = g_rand_int_range(random_generator, 1, 0xFFFFFF);
  g_rand_free(random_generator);

  js->duk_perm_context = duk_create_heap(NULL, NULL, NULL, ctrl_nlp_th, NlpJsDuketapeErrorHandler);
  if (js->duk_perm_context == NULL)
  {
    NlpThrowErrorTh(ctrl_nlp_th, "NlpJsInit: unable to init js context");
    DPcErr;
  }

  duk_context *ctx = js->duk_perm_context;

  // push og_nlp_th pointer as hidden og_nlp_th variable
  duk_push_pointer(ctx, ctrl_nlp_th);
  if (!duk_put_global_string(ctx, DUK_HIDDEN_SYMBOL("og_nlp_th")))
  {
    NlpThrowErrorTh(ctrl_nlp_th, "NlpJsInit: duk_put_global_string(og_nlp_th) failed");
    DPcErr;
  }

  // init console
  duk_console_init(ctx, 0);

  // load nodes modules
  duk_push_object(ctx);
  duk_push_c_function(ctx, NlpJsInitResolvModule, DUK_VARARGS);
  duk_put_prop_string(ctx, -2, "resolve");
  duk_push_c_function(ctx, NlpJsInitLoadModule, DUK_VARARGS);
  duk_put_prop_string(ctx, -2, "load");
  duk_module_node_init(ctx);

  // load and init libs
  IFE(NlpJsLoadLibMoment(ctrl_nlp_th));

  IFE(NlpJsInitIsolatedEval(ctrl_nlp_th));

  js->duk_request_context = NULL;

  js->reset_counter = 0;
  DONE;
}

static og_status NlpJsInitIsolatedEval(og_nlp_th ctrl_nlp_th)
{
  duk_context *ctx = ctrl_nlp_th->js->duk_perm_context;

  // https://github.com/rotaready/moment-range#node--npm
  og_string duktape_nlp = ""   // keep format
          "const duktape_nlp = Object.freeze({ \n"
          "  eval: function(script_to_eval) { \n"// protect moment change
          "    var duktape_nlp = undefined;\n"// protect duktape_nlp access/change
          "    var Duktape = undefined;\n"// protect duktape_nlp access/change
          "    var moment = "DOgNlpJsMomentSecretName";\n"// protect moment access/change
  "    return eval(script_to_eval);\n"
  "  } \n"
  "});\n"
  "";

  og_char_buffer duktape_nlp_buff[DPcPathSize];
  snprintf(duktape_nlp_buff, DPcPathSize, duktape_nlp, ctrl_nlp_th->js->random_number);

  if (duk_peval_string(ctx, duktape_nlp_buff) != 0)
  {
    NlpThrowErrorTh(ctrl_nlp_th, "%s", duk_safe_to_string(ctx, -1));
    NlpThrowErrorTh(ctrl_nlp_th, "NlpJsInit: NlpJsInitIsolatedEval failed : \n%s", duktape_nlp_buff);
    DPcErr;
  }
  else
  {
    NlpLog(DOgNlpTraceJs, "NlpJsInit: NlpJsInitIsolatedEval done.");
  }

  DONE;
}

static og_status NlpJsLoadLibMoment(og_nlp_th ctrl_nlp_th)
{
  duk_context *ctx = ctrl_nlp_th->js->duk_perm_context;

  // https://github.com/rotaready/moment-range#node--npm
  og_string require = ""   // keep format
          "const " DOgNlpJsMomentSecretName " = require('moment');\n"// moment
  "var moment_range = require('moment-range');\n"// moment-range
  "moment_range.extendMoment(" DOgNlpJsMomentSecretName ");\n"// extends
  "moment_range = undefined;\n"// remove unused variables
  "";

  og_char_buffer require_buff[DPcPathSize];
  snprintf(require_buff, DPcPathSize, require, ctrl_nlp_th->js->random_number, ctrl_nlp_th->js->random_number);

  if (duk_peval_string(ctx, require_buff) != 0)
  {
    NlpThrowErrorTh(ctrl_nlp_th, "%s", duk_safe_to_string(ctx, -1));
    NlpThrowErrorTh(ctrl_nlp_th, "NlpJsInit: loading libs 'moment' failed : \n%s", require_buff);
    DPcErr;
  }
  else
  {
    NlpLog(DOgNlpTraceJs, "NlpJsInit: loading libs 'moment' done. ("DOgNlpJsMomentSecretName")",
        ctrl_nlp_th->js->random_number);
  }

  DONE;
}

static duk_ret_t NlpJsInitResolvModule(duk_context *ctx)
{
  og_string module_id = duk_require_string(ctx, 0);
  og_string parent_id = duk_require_string(ctx, 1);

  // get og_nlp_th pointer
  duk_get_global_string(ctx, DUK_HIDDEN_SYMBOL("og_nlp_th"));
  og_nlp_th ctrl_nlp_th = duk_get_pointer(ctx, -1);
  if (ctrl_nlp_th == NULL)
  {
    return duk_type_error(ctx, "NlpJsInitResolvModule: duk_get_global_string(og_nlp_th) failed");
  }

  // push back file name
  if (ctrl_nlp_th->ctrl_nlp->WorkingDirectory[0])
  {
    duk_push_sprintf(ctx, "%s/node_modules/%s.js", ctrl_nlp_th->ctrl_nlp->WorkingDirectory, module_id);
  }
  else
  {
    duk_push_sprintf(ctx, "node_modules/%s.js", module_id);
  }

  NlpLog(DOgNlpTraceJs, "NlpJsInitResolvModule: resolve_cb: id:'%s', parent-id:'%s', resolve-to:'%s'", module_id,
      parent_id, duk_get_string(ctx, -1));

  return 1;
}

static duk_ret_t push_file_as_string(duk_context *ctx, og_string filename)
{
  size_t full_file_content_size = 0;
  char *full_file_content = NULL;

  GError *error = NULL;
  og_bool file_loaded = g_file_get_contents(filename, &full_file_content, &full_file_content_size, &error);
  if (!file_loaded)
  {
    og_string error_msg = "";
    if (error != NULL && error->message != NULL)
    {
      error_msg = error->message;
    }

    return duk_type_error(ctx, "push_file_as_string: impossible load file '%s' : %s", filename, error_msg);
  }

  // load javascript file
  duk_push_lstring(ctx, full_file_content, full_file_content_size);

  // free allocated buffer
  g_free(full_file_content);

  return 1;
}

static duk_ret_t NlpJsInitLoadModule(duk_context *ctx)
{
  og_string module_id = duk_require_string(ctx, 0);
  duk_get_prop_string(ctx, 2, "filename");
  og_string filename = duk_require_string(ctx, -1);

  // get og_nlp_th pointer
  duk_get_global_string(ctx, DUK_HIDDEN_SYMBOL("og_nlp_th"));
  og_nlp_th ctrl_nlp_th = duk_get_pointer(ctx, -1);
  if (ctrl_nlp_th == NULL)
  {
    return duk_type_error(ctx, "NlpJsInitLoadModule: duk_get_global_string(og_nlp_th) failed");
  }

  NlpLog(DOgNlpTraceJs, "NlpJsInitLoadModule: load_cb: id:'%s', filename:'%s'", module_id, filename);

  if (!push_file_as_string(ctx, filename))
  {
    return duk_type_error(ctx, "NlpJsInitLoadModule: impossible read file '%s'", filename);
  }

  return 1;
}

og_status NlpJsReset(og_nlp_th ctrl_nlp_th)
{
  duk_context *ctx = ctrl_nlp_th->js->duk_perm_context;
  if (ctx == NULL)
  {
    CONT;
  }

  IFE(NlpJsStackRequestWipe(ctrl_nlp_th));

  ctrl_nlp_th->js->reset_counter++;
  if ((ctrl_nlp_th->js->reset_counter % ctrl_nlp_th->ctrl_nlp->env->NlpJSDukGcPeriod) == 0)
  {
    // We need to call it twice to make sure everything
    duk_gc(ctx, 0);
    duk_gc(ctx, 0);
  }

  DONE;
}

og_status NlpJsFlush(og_nlp_th ctrl_nlp_th)
{
  g_queue_clear(ctrl_nlp_th->js->variables_name_list);
  g_queue_clear(ctrl_nlp_th->js->variables_values);
  g_string_chunk_free(ctrl_nlp_th->js->variables);
  ctrl_nlp_th->js->variables = NULL;

  OgHeapFlush(ctrl_nlp_th->js->buffer);
  ctrl_nlp_th->js->buffer = NULL;

  duk_context *ctx = ctrl_nlp_th->js->duk_perm_context;
  if (ctx != NULL)
  {
    duk_destroy_heap(ctx);
    ctrl_nlp_th->js->duk_perm_context = NULL;
  }

  DONE;
}

og_status NlpJsStackRequestSetup(og_nlp_th ctrl_nlp_th)
{
  duk_context *ctx = ctrl_nlp_th->js->duk_perm_context;
  if (ctx == NULL)
  {
    CONT;
  }

  IFE(NlpJsStackRequestWipe(ctrl_nlp_th));

  // create a new thread (ctx) for the request, nothing must add in duk_perm_context after that
  duk_push_thread(ctx);
  ctrl_nlp_th->js->duk_request_context = duk_require_context(ctx, -1);
  if (ctrl_nlp_th->js->duk_request_context == NULL)
  {
    NlpThrowErrorTh(ctrl_nlp_th, "NlpJsInit: NlpJsRequestSetup unable to create request ctx : duk_push_thread.");
    DPcErr;
  }

  // set now for the whole request
  IFE(NlpJsSetNow(ctrl_nlp_th));

  DONE;
}

og_bool NlpJsStackRequestWipe(og_nlp_th ctrl_nlp_th)
{
  duk_context *request_ctx = ctrl_nlp_th->js->duk_request_context;
  if (request_ctx == NULL)
  {
    return FALSE;
  }

  duk_context *perm_ctx = ctrl_nlp_th->js->duk_perm_context;
  if (perm_ctx == NULL)
  {
    return FALSE;
  }

  IFE(NlpJsStackLocalWipe(ctrl_nlp_th));

  /* This duk_pop() makes the "request_ctx" thread unreachable (assuming there
   * is no other reference to it), so "request_ctx" is no longer valid
   * afterwards.
   */
  duk_pop(perm_ctx);
  ctrl_nlp_th->js->duk_request_context = NULL;

  IFE(OgHeapResetWithoutReduce(ctrl_nlp_th->js->buffer));

  return TRUE;
}

og_bool NlpJsStackLocalWipe(og_nlp_th ctrl_nlp_th)
{
  duk_context *ctx = ctrl_nlp_th->js->duk_request_context;
  if (ctx == NULL)
  {
    return FALSE;
  }

  og_bool cleaned = FALSE;
  for (GList *iter = ctrl_nlp_th->js->variables_name_list->head; iter; iter = iter->next)
  {
    og_string variable_name = iter->data;

    // on désinitialise la variable
    og_char_buffer var_command[DPcPathSize];
    snprintf(var_command, DPcPathSize, "delete %s;", variable_name);

    NlpLog(DOgNlpTraceJs, "NlpJsStackLocalWipe: Sending eval to duktape: %s", var_command);

    if (duk_peval_string(ctx, var_command) != 0)
    {
      NlpThrowErrorTh(ctrl_nlp_th, "NlpJsStackLocalWipe: duk_peval_lstring eval failed: %s : '%s'",
          duk_safe_to_string(ctx, -1), var_command);
      DPcErr;
    }

    cleaned = TRUE;
  }

  // on nettoie la liste des variables enregistrées
  g_queue_clear(ctrl_nlp_th->js->variables_name_list);
  g_queue_clear(ctrl_nlp_th->js->variables_values);
  g_string_chunk_clear(ctrl_nlp_th->js->variables);

  IFE(OgHeapReset(ctrl_nlp_th->js->buffer));

  return cleaned;
}

static og_status split_error_message(json_t *json_errors, og_string multiple_errors)
{
  if (multiple_errors == NULL) CONT;

  int imultiple_errors = strlen(multiple_errors);
  char errors[imultiple_errors + 1];
  strncpy(errors, multiple_errors, imultiple_errors + 1);

  char *saveptr = NULL;
  char *error_line = strtok_r(errors, "\n", &saveptr);
  while (error_line != NULL)
  {
    json_array_append_new(json_errors, json_string(error_line));
    error_line = strtok_r(NULL, "\n", &saveptr);
  }

  DONE;
}

static og_status NlpJsEvalBuildError(og_nlp_th ctrl_nlp_th, duk_context *ctx, og_string script)
{

  ctrl_nlp_th->json_answer_error = json_object();
  json_t *errors_javascript = json_object();
  json_object_set_new(ctrl_nlp_th->json_answer_error, "errors_code", json_string("javascript"));
  json_object_set_new(ctrl_nlp_th->json_answer_error, "errors", json_array());
  json_object_set_new(ctrl_nlp_th->json_answer_error, "errors_javascript", errors_javascript);

  if (duk_get_prop_string(ctx, -1, "lineNumber"))
  {
    og_string lineNumber = duk_safe_to_string(ctx, -1);
    ctrl_nlp_th->js->last_error_linenumber = atol(lineNumber);
  }
  duk_pop(ctx);

  og_string error = duk_to_string(ctx, -1);
  json_object_set_new(errors_javascript, "message", json_string(error));

  og_char_buffer now_eval[DPcPathSize];
  snprintf(now_eval, DPcPathSize, DOgNlpJsMomentSecretName ".now_form_request.toJSON()",
      ctrl_nlp_th->js->random_number);

  json_t *errors_context = json_array();
  json_object_set_new(errors_javascript, "context", errors_context);
  json_t *errors_code = json_array();
  json_object_set_new(errors_javascript, "code", errors_code);
  split_error_message(errors_code, script);

  // show now
  og_char_buffer now[DPcPathSize];
  now[0] = '\0';
  if (duk_peval_string(ctx, now_eval) == 0)
  {
    og_char_buffer now_json[DPcPathSize];
    snprintf(now_json, DPcPathSize, "// Current datetime: '%s'", duk_safe_to_string(ctx, -1));
    og_string with_moment = "// available with Moment.js";
    json_array_append_new(errors_context, json_string(now_json));
    json_array_append_new(errors_context, json_string(with_moment));
    snprintf(now, DPcPathSize, "%s available with Moment.js\n", now_json);
  }

  IFE(NlpThrowErrorTh(ctrl_nlp_th, "%s", script));
  IFE(NlpThrowErrorTh(ctrl_nlp_th, "\n// ====== Eval =======\n"
      "%s// JavaScript error : %s from here :\n// ===================\n", now, error));

  // read variables in backward to provide better message
  for (GList *iter = ctrl_nlp_th->js->variables_values->head; iter; iter = iter->next)
  {
    og_string variable_value = iter->data;
    IFE(NlpThrowErrorTh(ctrl_nlp_th, "%s", variable_value));
    json_array_append_new(errors_context, json_string(variable_value));
  }

  IFE(NlpThrowErrorTh(ctrl_nlp_th, "NlpJsEval: duk_peval_lstring eval failed: %s :\n// ===== Context =====", error));

  DPcErr;

  DONE;
}

og_status NlpJsEval(og_nlp_th ctrl_nlp_th, int original_js_script_size, og_string original_js_script,
    json_t **p_json_anwser)
{
  // ignore answer
  if (p_json_anwser == NULL)
  {
    DONE;
  }
  else
  {
    *p_json_anwser = NULL;
  }

  duk_context *ctx = ctrl_nlp_th->js->duk_request_context;
  if (ctx == NULL)
  {
    NlpLog(DOgNlpTraceJs, "NlpJsEval no javascript request ctx initialised");
    CONT;
  }

  // check size according to VLA use on stack, use a heap instead
  if (original_js_script_size > DOgNlpInterpretationSolutionMaxLength)
  {
    NlpThrowErrorTh(ctrl_nlp_th, "NlpJsEval : js script size is too long");
    DPcErr;
  }

  // use heap buffer
  // // 1 is \0 and 6 is minimal length for (\n\n)\0
  int buffer_size = original_js_script_size + 1 + original_js_script_size + 6;
  og_char_buffer *buffer = OgHeapGetBufferReuse(ctrl_nlp_th->js->buffer, buffer_size);
  if (buffer == NULL)
  {
    IFE(NlpThrowErrorTh(ctrl_nlp_th, "NlpJsEval: OgHeapGetBufferReuse failed"));
    DPcErr;
  }

  // 1 is \0
  og_char_buffer *js_script = OgHeapGetBufferNew(ctrl_nlp_th->js->buffer, original_js_script_size + 1);
  if (js_script == NULL)
  {
    IFE(NlpThrowErrorTh(ctrl_nlp_th, "NlpJsEval: OgHeapGetBufferNew failed"));
    DPcErr;
  }

  int js_script_size = original_js_script_size;
  snprintf(js_script, js_script_size + 1, "%.*s", original_js_script_size, original_js_script);

  // trim
  og_string trimed_script = g_strstrip(js_script);
  int trimed_script_length = strlen(trimed_script);
  IFE(OgHeapSetCellsUsed(ctrl_nlp_th->js->buffer, trimed_script_length + 1));

  // 6 is minimal length for (\n\n)\0
  int enhanced_script_size = trimed_script_length + 6;
  og_char_buffer *enhanced_script = OgHeapGetBufferNew(ctrl_nlp_th->js->buffer, enhanced_script_size);
  enhanced_script[0] = '\0';

  // starting and ending '{' '}' => surround by '(' ')'
  if (trimed_script[0] == '{' && trimed_script[trimed_script_length - 1] == '}')
  {
    snprintf(enhanced_script, enhanced_script_size, "( %s )", trimed_script);
  }
  else
  {
    snprintf(enhanced_script, enhanced_script_size, "%s", trimed_script);
  }

  if (ctrl_nlp_th->loginfo->trace & DOgNlpTraceJs)
  {
    og_char_buffer now_eval[DPcPathSize];
    snprintf(now_eval, DPcPathSize, DOgNlpJsMomentSecretName ".now_form_request.toJSON()",
        ctrl_nlp_th->js->random_number);

    // show now
    og_char_buffer now[DPcPathSize];
    now[0] = '\0';
    if (duk_peval_string(ctx, now_eval) == 0)
    {
      snprintf(now, DPcPathSize, "// now returned by `moment()` is '%s'\n", duk_safe_to_string(ctx, -1));
    }
    NlpLog(DOgNlpTraceJs, "NlpJsEval js to evaluate : \n// =========\n%s%s\n//=========\n", now, enhanced_script);
  }

  // reset line number
  ctrl_nlp_th->js->last_error_linenumber = -1;

  // put object, method + args
  if (!duk_get_global_string(ctx, "duktape_nlp"))
  {
    IFE(NlpThrowErrorTh(ctrl_nlp_th, "NlpJsEval: unable to access to 'duktape_nlp' variable."));
    DPcErr;
  }
  duk_push_string(ctx, "eval");
  duk_push_string(ctx, enhanced_script);

  // eval securely
  if (duk_pcall_prop(ctx, -3, 1) != 0)
  {
    NlpJsEvalBuildError(ctrl_nlp_th, ctx, enhanced_script);
    DPcErr;
  }

  // get result
  duk_int_t type = duk_get_type(ctx, -1);
  switch (type)
  {
    case DUK_TYPE_NONE:
    case DUK_TYPE_UNDEFINED:
    case DUK_TYPE_NULL:
    {
      *p_json_anwser = json_null();

      NlpLog(DOgNlpTraceJs, "NlpJsEval : computed value is %s", NlpJsDukTypeString(type));

      break;
    }

    case DUK_TYPE_BOOLEAN:
    {
      double computed_boolean = duk_get_boolean(ctx, -1);
      *p_json_anwser = json_boolean(computed_boolean);

      NlpLog(DOgNlpTraceJs, "NlpJsEval : computed value is a boolean : %s", ((computed_boolean) ? "TRUE" : "FALSE"));

      break;
    }

    case DUK_TYPE_NUMBER:
    {
      double computed_number = duk_get_number(ctx, -1);

      // check if it is an integer
      if ((json_int_t) ((computed_number * 100) / 100) == computed_number)
      {
        json_int_t int_computed_number = computed_number;
        *p_json_anwser = json_integer(computed_number);

        NlpLog(DOgNlpTraceJs, "NlpJsEval : computed value is a number (int) : %" JSON_INTEGER_FORMAT,
            int_computed_number);
      }
      else
      {
        *p_json_anwser = json_real(computed_number);

        NlpLog(DOgNlpTraceJs, "NlpJsEval : computed value is a number (double): " DOgPrintDouble, computed_number);
      }

      break;
    }

    case DUK_TYPE_STRING:
    {
      duk_size_t icomputed_string = 0;
      og_string computed_string = duk_get_lstring(ctx, -1, &icomputed_string);

      if (g_utf8_validate(computed_string, icomputed_string, NULL))
      {
        NlpLog(DOgNlpTraceJs, "NlpJsEval : computed value is a string : '%s'", computed_string);
        *p_json_anwser = json_stringn(computed_string, icomputed_string);
      }
      else
      {
        NlpLog(DOgNlpTraceJs, "NlpJsEval : computed string contains error duk_get_string"
            " return dummy string, try to convert it from CESU-8");

        og_string computed_string_converted = NULL;
        IFE(NlpJsDukCESU8toUTF8(ctrl_nlp_th, computed_string, icomputed_string, &computed_string_converted));
        if (computed_string_converted != NULL)
        {
          *p_json_anwser = json_string(computed_string_converted);

          NlpLog(DOgNlpTraceJs, "NlpJsEval : computed string contains error duk_get_string"
              " return dummy string, try to convert it from CESU-8 successful : %s", computed_string_converted);

          // free converted string
          g_free((gchar*) computed_string_converted);

          if (*p_json_anwser == NULL)
          {
            NlpThrowErrorTh(ctrl_nlp_th, "%s", enhanced_script);
            NlpThrowErrorTh(ctrl_nlp_th, "NlpJsEval : NlpJsDukCESU8toUTF8 failed computed string"
                " contains error duk_get_string return dummy string");
            DPcErr;
          }
        }
        else
        {
          NlpThrowErrorTh(ctrl_nlp_th, "%s", enhanced_script);
          NlpThrowErrorTh(ctrl_nlp_th, "NlpJsEval : NlpJsDukCESU8toUTF8 failed computed string"
              " contains error duk_get_string return dummy string");
          DPcErr;
        }

      }

      break;
    }

    case DUK_TYPE_OBJECT:
    {
      // object, array, function
      og_string computed_json = duk_json_encode(ctx, -1);
      if (computed_json == NULL)
      {
        computed_json = "null";
        *p_json_anwser = json_null();
      }
      else
      {
        int icomputed_json = strlen(computed_json);

        og_string computed_json_converted = NULL;
        if (!g_utf8_validate(computed_json, icomputed_json, NULL))
        {
          NlpLog(DOgNlpTraceJs, "NlpJsEval : computed json object contains error duk_get_string"
              " return dummy string, try to convert it from CESU-8");

          IFE(NlpJsDukCESU8toUTF8(ctrl_nlp_th, computed_json, icomputed_json, &computed_json_converted));
          if (computed_json_converted != NULL)
          {
            computed_json = computed_json_converted;
            icomputed_json = strlen(computed_json);

            NlpLog(DOgNlpTraceJs, "NlpJsEval : computed json object contains error duk_get_string"
                " return dummy string, try to convert it from CESU-8 successful : %s", computed_json_converted);
          }
          else
          {
            NlpThrowErrorTh(ctrl_nlp_th, "%s", enhanced_script);
            NlpThrowErrorTh(ctrl_nlp_th, "NlpJsEval : NlpJsDukCESU8toUTF8 failed computed json object"
                " contains error duk_get_string return dummy string");
            // free converted string
            g_free((gchar*) computed_json_converted);
            DPcErr;
          }

        }

        // pure string value are surrounded by backquote "\"toto\""
        if (icomputed_json >= 2 && computed_json[0] == '"' && computed_json[icomputed_json - 1] == '"')
        {
          *p_json_anwser = json_stringn(computed_json + 1, icomputed_json - 2);

          // free converted string
          g_free((gchar*) computed_json_converted);
        }
        else
        {

          json_error_t error[1];
          *p_json_anwser = json_loadb(computed_json, icomputed_json, 0, error);
          if (*p_json_anwser == NULL)
          {
            NlpThrowErrorTh(ctrl_nlp_th,
                "NlpJsEval : computed json object contains error in ligne %d and column %d , %s , %s \n'%s'",
                error->line, error->column, error->source, error->text, computed_json);

            // free converted string
            g_free((gchar*) computed_json_converted);
            DPcErr;
          }

          // free converted string
          g_free((gchar*) computed_json_converted);

        }

      }

      NlpLog(DOgNlpTraceJs, "NlpJsEval : computed value is an object/array : '%s'", computed_json);

      break;
    }

    default:
    {
      NlpThrowErrorTh(ctrl_nlp_th, "NlpJsEval: unsupported answer duktype : %s", NlpJsDukTypeString(type));
      DPcErr;
    }

  }

  duk_pop(ctx);

  DONE;
}

og_status NlpJsAddVariable(og_nlp_th ctrl_nlp_th, og_string variable_name, og_string variable_eval,
    int variable_eval_length)
{
  int variable_name_size = strlen(variable_name);
  if (variable_name_size == 0)
  {
    CONT;
  }

  int variable_eval_size = variable_eval_length;
  if (variable_eval_size == -1)
  {
    variable_eval_size = strlen(variable_eval);
  }
  else if (variable_eval_size == 0)
  {
    variable_eval = "null";
    variable_eval_size = 4;
  }

  // https://github.com/svaarala/duktape/issues/284 : ES2015 Block Scoping is not available now to use `let`
  og_string var_template = "const %s = %s;";
  int var_template_size = strlen(var_template);
  int var_command_size = variable_eval_size + variable_name_size + var_template_size;

  og_char_buffer var_command[var_command_size];
  snprintf(var_command, var_command_size, var_template, variable_name, variable_eval);

  NlpLog(DOgNlpTraceJs, "Sending variable to duktape: %s", var_command);

  duk_context *ctx = ctrl_nlp_th->js->duk_request_context;
  if (ctx == NULL)
  {
    NlpLog(DOgNlpTraceJs, "NlpJsEval no javascript request ctx initialised");
    CONT;
  }

  if (duk_peval_string(ctx, var_command) != 0)
  {
    NlpThrowErrorTh(ctrl_nlp_th, "NlpJsAddVariable: duk_peval_lstring eval failed: %s : '%s'",
        duk_safe_to_string(ctx, -1), var_command);
    DPcErr;
  }

  // keep variable command for better error message
  gchar *variable_command_stored = g_string_chunk_insert(ctrl_nlp_th->js->variables, var_command);
  g_queue_push_tail(ctrl_nlp_th->js->variables_values, variable_command_stored);

  // keep variables names for later cleanup
  gchar *variable_name_stored = g_string_chunk_insert(ctrl_nlp_th->js->variables, variable_name);
  g_queue_push_tail(ctrl_nlp_th->js->variables_name_list, variable_name_stored);

  DONE;
}

#define DOgNlpJsSetNowNowScriptSize DPcPathSize * 2
og_status NlpJsSetNow(og_nlp_th ctrl_nlp_th)
{

  og_string date_now = ctrl_nlp_th->date_now;
  if (date_now == NULL)
  {
    NlpLog(DOgNlpTraceJs, "NlpJsSetNow: current UTC time");
  }
  else
  {
    NlpLog(DOgNlpTraceJs, "NlpJsSetNow: %s", date_now);
  }

  og_string offset_header_tpl = "// ====================================================\n"
      "var moment = " DOgNlpJsMomentSecretName " ;\n";

  og_char_buffer offset_header[DPcPathSize];
  snprintf(offset_header, DPcPathSize, offset_header_tpl, ctrl_nlp_th->js->random_number);

  og_string offset_reset = "\n"   //
          "// Reset now state \n"//
          "moment.now_form_request = null; \n"
          "moment.updateOffset = function() { }; \n"//
          "moment.now = function() { \n"//
          "  return new Date(); \n"//
          "}; \n"//
          " \n"//
          "// Set now\n";

  og_string offset_setup = ""   //
          "// format moment in ISO with timezone  \n"//
          "moment.fn.toJSON = function() { \n"//
          "   return this.format(); \n"//
          "}; \n"//
          " \n"//
          "// Use now from request \n"//
          "moment.now = function() { \n"//
          "  return moment.now_form_request.toDate(); \n"//
          "}; \n"//
          " \n"//
          "// Adjust utcOffset to now utcOffset, replace setOffsetToParsedOffset() \n"//
          "moment.updateOffset = function(m, keepLocalTime) { \n"//
          "  if (!m.updateOffsetInProgress) { \n"//
          "    m.updateOffsetInProgress = true; \n"//
          "    m.parseZone = function() { \n"//
          "      if (this._tzm != null) { \n"//
          "        //this.utcOffset(this._tzm, false, true); \n"//
          "        this.utcOffset(moment.now_form_request.utcOffset(), false, true); \n"//
          "      } else if (typeof this._i === 'string') { \n"//
          "        this.utcOffset(moment.now_form_request.utcOffset(), true, true); \n"//
          "      } else { \n"//
          "        this.utcOffset(moment.now_form_request.utcOffset(), false, true); \n"//
          "      } \n"//
          "      return this; \n"//
          "    } \n"//
          "    m.parseZone(); \n"//
          "    m.updateOffsetInProgress = false; \n"//
          "  } \n"//
          "}; \n"//
          "// ====================================================\n";

  og_char_buffer now_setup_command[DOgNlpJsSetNowNowScriptSize];
  if (date_now == NULL || date_now[0] == '\0')
  {
    snprintf(now_setup_command, DOgNlpJsSetNowNowScriptSize, "%s%smoment.now_form_request ="
        " moment.utc();\n\n%s", offset_header, offset_reset, offset_setup);
  }
  else
  {
    snprintf(now_setup_command, DOgNlpJsSetNowNowScriptSize, "%s%smoment.now_form_request ="
        " moment.parseZone('%s');\n\n%s", offset_header, offset_reset, date_now, offset_setup);
  }

  NlpLog(DOgNlpTraceJs, "Sending command to duktape:\n%s", now_setup_command);

  duk_context *ctx = ctrl_nlp_th->js->duk_request_context;
  if (ctx == NULL)
  {
    NlpLog(DOgNlpTraceJs, "NlpJsSetNow no javascript request ctx initialised");
    CONT;
  }

  if (duk_peval_string(ctx, now_setup_command) != 0)
  {
    NlpThrowErrorTh(ctrl_nlp_th, "NlpJsSetNow: duk_peval_lstring eval failed: %s :\n%s", duk_safe_to_string(ctx, -1),
        now_setup_command);
    DPcErr;
  }

  DONE;
}

og_status NlpJsAddVariableJson(og_nlp_th ctrl_nlp_th, og_string variable_name, json_t *variable_value)
{
  og_bool truncated = FALSE;
  og_char_buffer variable_eval_buffer[DOgMlogMaxMessageSize / 2];

  IFE(NlpJsonToBuffer(variable_value, variable_eval_buffer, DOgMlogMaxMessageSize / 2, &truncated, JSON_ENCODE_ANY));
  if (truncated)
  {
    NlpThrowErrorTh(ctrl_nlp_th, "NlpJsAddVariableJson: truncated : %s", variable_eval_buffer);
    DPcErr;
  }

  return NlpJsAddVariable(ctrl_nlp_th, variable_name, variable_eval_buffer, -1);
}

static og_string NlpJsDukTypeString(duk_int_t type)
{
  switch (type)
  {
    case DUK_TYPE_NONE:
      return "none";
    case DUK_TYPE_UNDEFINED:
      return "undefined";
    case DUK_TYPE_NULL:
      return "null";
    case DUK_TYPE_BOOLEAN:
      return "boolean";
    case DUK_TYPE_NUMBER:
      return "number";
    case DUK_TYPE_STRING:
      return "string";
    case DUK_TYPE_OBJECT:
      return "object";
    case DUK_TYPE_BUFFER:
      return "buffer";
    case DUK_TYPE_POINTER:
      return "pointer";
    case DUK_TYPE_LIGHTFUNC:
      return "lightfunc";
  }

  return "nil";
}

struct data_CESU8toUTF8
{
  og_nlp_th ctrl_nlp_th;

  og_string original_cesu_str;
  int original_cesu_str_length;

  og_bool match;
  og_bool error;
};

static gboolean NlpJsDukCESU8toUTF8Callback(const GMatchInfo *match_info, GString *result, gpointer user_data)
{

  struct data_CESU8toUTF8 *data = user_data;
  og_nlp_th ctrl_nlp_th = data->ctrl_nlp_th;

  int start_pos = 0;
  int end_pos = 0;

  og_bool match = g_match_info_fetch_pos(match_info, 0, &start_pos, &end_pos);
  if (match)
  {
    int match_length = (end_pos - start_pos);
    if (match_length <= 0)
    {
      data->error = TRUE;
      NlpThrowErrorTh(ctrl_nlp_th, "NlpJsDukCESU8toUTF8Callback: match length is %d", match_length);
      return TRUE;
    }

    if (match_length > data->original_cesu_str_length)
    {
      data->error = TRUE;
      NlpThrowErrorTh(ctrl_nlp_th, "NlpJsDukCESU8toUTF8Callback: match length is %d > %d (orginal lentgh)",
          match_length, data->original_cesu_str_length);
      return TRUE;
    }

    // matching buffer must contains 6 bytes
    if (match_length != 6)
    {
      data->error = TRUE;
      NlpThrowErrorTh(ctrl_nlp_th, "NlpJsDukCESU8toUTF8Callback: match length not 6 bytes length", match_length);
      return TRUE;
    }

    og_char_buffer buffer[8];
    memcpy(buffer, data->original_cesu_str + start_pos, end_pos - start_pos);

    // create UCS-4 character from CESU-8 encoded surrogate pair
    // http://www.unicode.org/reports/tr26/#definitions

    // 3 bytes CESU-8 to UNICODE high surrogate:
    gunichar high = ((buffer[0] & 0x0F) << 12) + ((buffer[1] & 0x3F) << 6) + (buffer[2] & 0x3F);
    // 3 bytes CESU-8 to UNICODE low surrogate:
    gunichar low = ((buffer[3] & 0x0F) << 12) + ((buffer[4] & 0x3F) << 6) + (buffer[5] & 0x3F);

    gunichar codepoint = ((high - 0xD800) * 0x400) + (low - 0xDC00) + 0x10000;

    // convert it to UTF-8
    og_char_buffer utf8_buffer[8];
    memset(utf8_buffer, 0, sizeof(og_char_buffer) * 8);
    int utf8_buffer_length = g_unichar_to_utf8(codepoint, utf8_buffer);
    utf8_buffer[utf8_buffer_length] = 0;

    // append it
    g_string_append(result, utf8_buffer);

    data->match = TRUE;

  }

  return FALSE;
}

static og_status NlpJsDukCESU8toUTF8(og_nlp_th ctrl_nlp_th, og_string cesu, int cesu_length, og_string *utf8)
{

  // https://github.com/svaarala/duktape-wiki/blob/master/HowtoNonBmpCharacters.md
  // https://github.com/svaarala/duktape/issues/996
  // https://github.com/svaarala/duktape/blob/master/doc/utf8-internal-representation.rst
  // http://fileformats.archiveteam.org/wiki/CESU-8

  // CESU-8 is an inefficient Unicode character encoding related to UTF-8. It is not an accepted standard,
  // but has been documented in the interest of practicality.
  // It's what you get if you take UTF-16 data, reinterpret it as UCS-2, then convert it to UTF-8
  // (while ignoring any rules forbidding the use of code points in the range U+D800 to U+DFFF).
  // A code point thus uses 1, 2, 3, or 6 bytes.
  // It is sometimes used by accident, but may be used deliberately to accommodate systems
  // that don't support 4-byte UTF-8 sequences, or when a close correspondence between UTF-16 and a UTF-8-like
  // encoding is deemed necessary.

  // original string
  struct data_CESU8toUTF8 data[1];
  memset(data, 0, sizeof(struct data_CESU8toUTF8));
  data->ctrl_nlp_th = ctrl_nlp_th;
  data->original_cesu_str = cesu;
  data->original_cesu_str_length = cesu_length;

  // implemented from PHP https://stackoverflow.com/questions/34151138/convert-cesu-8-to-utf-8-with-high-performance

  og_string pattern = "(\\xED[\\xA0-\\xAF][\\x80-\\xBF]\\xED[\\xB0-\\xBF][\\x80-\\xBF])";

  GError *regexp_error = NULL;
  GRegex *regex = g_regex_new(pattern, G_REGEX_RAW, 0, &regexp_error);
  if (!regex || regexp_error)
  {
    NlpThrowErrorTh(ctrl_nlp_th, "NlpJsDukCESU8toUTF8: g_regex_new failed: %s\npattern: %s\n", regexp_error->message,
        pattern);
    g_error_free(regexp_error);
    DPcErr;
  }

  gchar *result = g_regex_replace_eval(regex, cesu, cesu_length, 0, 0, NlpJsDukCESU8toUTF8Callback, data,
      &regexp_error);

  // free memmory
  g_regex_unref(regex);

  if (!result || regexp_error)
  {
    NlpThrowErrorTh(ctrl_nlp_th, "NlpJsDukCESU8toUTF8: g_regex_replace_eval failed: %s", regexp_error->message);
    g_error_free(regexp_error);
    DPcErr;
  }
  else if (data->error)
  {
    NlpThrowErrorTh(ctrl_nlp_th, "NlpJsDukCESU8toUTF8: g_regex_replace_eval NlpJsDukCESU8toUTF8Callback failed");
    DPcErr;
  }
  else if (!data->match)
  {
    NlpThrowErrorTh(ctrl_nlp_th, "NlpJsDukCESU8toUTF8: cannot be converted");
    DPcErr;
  }
  else if (!g_utf8_validate(result, -1, NULL))
  {
    NlpThrowErrorTh(ctrl_nlp_th, "NlpJsDukCESU8toUTF8: result is not valide UTF-8");
    DPcErr;
  }

  *utf8 = result;

  DONE;

}
