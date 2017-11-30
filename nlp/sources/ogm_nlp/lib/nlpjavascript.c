/*
 *  Eval javascript snipets
 *  Copyright (c) 2017 Pertimm, by Brice Ruzand
 *  Dev : Novembre 2017
 *  Version 1.0
 */

#include "ogm_nlp.h"
#include "duk_module_node.h"
#include "duk_console.h"

static og_string NlpJsDukTypeString(duk_int_t type);
static duk_ret_t NlpJsInitLoadModule(duk_context *ctx);
static duk_ret_t NlpJsInitResolvModule(duk_context *ctx);
static duk_ret_t push_file_as_string(duk_context *ctx, og_string filename);
static og_status NlpJsLoadLibMoment(og_nlp_th ctrl_nlp_th);

static void NlpJsDuketapeErrorHandler(void *udata, const char *msg)
{
  og_nlp_th ctrl_nlp_th = udata;

  NlpThrowErrorTh(ctrl_nlp_th, "NlpJsDuketapeErrorHandler: %s", msg);

  abort();
}

og_status NlpJsInit(og_nlp_th ctrl_nlp_th)
{
  ctrl_nlp_th->js->varibale_values = g_string_chunk_new(0x10);
  g_queue_init(ctrl_nlp_th->js->variable_list);

  ctrl_nlp_th->js->duk_context = duk_create_heap(NULL, NULL, NULL, ctrl_nlp_th, NlpJsDuketapeErrorHandler);
  if (ctrl_nlp_th->js->duk_context == NULL)
  {
    NlpThrowErrorTh(ctrl_nlp_th, "NlpJsInit: unable to init js context");
    DPcErr;
  }

  duk_context *ctx = ctrl_nlp_th->js->duk_context;

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

  // stack after init
  ctrl_nlp_th->js->init_stack_idx = duk_get_top(ctx);
  if (ctrl_nlp_th->js->init_stack_idx != DUK_INVALID_INDEX)
  {
    ctrl_nlp_th->js->init_stack_idx = 0;
  }

  DONE;
}

static og_status NlpJsLoadLibMoment(og_nlp_th ctrl_nlp_th)
{
  duk_context *ctx = ctrl_nlp_th->js->duk_context;

  // https://github.com/rotaready/moment-range#node--npm
  og_string require = ""   // keep format
          "const moment = require('moment');\n"// moment
          "var moment_range = require('moment-range');\n"// moment-range
          "moment_range.extendMoment(moment);\n"// extends
          "moment_range = undefined;\n"// remove unused variables
          "";

  if (duk_peval_string(ctx, require) != 0)
  {
    NlpThrowErrorTh(ctrl_nlp_th, "%s", duk_safe_to_string(ctx, -1));
    NlpThrowErrorTh(ctrl_nlp_th, "NlpJsInit: loading libs 'moment' failed : \n%s", require);
    DPcErr;
  }
  else
  {
    NlpLog(DOgNlpTraceJs, "NlpJsInit: loading libs 'moment' done.");
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
  duk_context *ctx = ctrl_nlp_th->js->duk_context;
  if (ctx == NULL)
  {
    CONT;
  }

  IFE(NlpJsStackWipe(ctrl_nlp_th));

  // We need to call it twice to make sure everything
  duk_gc(ctx, 0);
  duk_gc(ctx, 0);

  DONE;
}

og_status NlpJsFlush(og_nlp_th ctrl_nlp_th)
{
  duk_context *ctx = ctrl_nlp_th->js->duk_context;
  if (ctx == NULL)
  {
    CONT;
  }

  duk_destroy_heap(ctx);
  ctrl_nlp_th->js->duk_context = NULL;

  g_queue_clear(ctrl_nlp_th->js->variable_list);
  g_string_chunk_free(ctrl_nlp_th->js->varibale_values);

  DONE;
}

og_bool NlpJsStackWipe(og_nlp_th ctrl_nlp_th)
{
  duk_context *ctx = ctrl_nlp_th->js->duk_context;
  if (ctx == NULL)
  {
    CONT;
  }

  g_queue_clear(ctrl_nlp_th->js->variable_list);
  g_string_chunk_clear(ctrl_nlp_th->js->varibale_values);

  duk_idx_t top = duk_get_top(ctx);
  if (top > 0 && top - ctrl_nlp_th->js->init_stack_idx > 0)
  {
    duk_pop_n(ctx, top - ctrl_nlp_th->js->init_stack_idx);
    return TRUE;
  }

  return FALSE;
}

og_status NlpJsEval(og_nlp_th ctrl_nlp_th, int js_script_size, og_string js_script, json_t **p_json_anwser)
{
  // ingnore answer
  if (p_json_anwser == NULL)
  {
    DONE;
  }
  else
  {
    *p_json_anwser = NULL;
  }

  NlpLog(DOgNlpTraceJs, "NlpJsEval js to evaluate : '%.*s'", js_script_size, js_script);

  duk_context *ctx = ctrl_nlp_th->js->duk_context;
  if (ctx == NULL)
  {
    NlpLog(DOgNlpTraceJs, "NlpJsEval no javascript ctx initialised");
    CONT;
  }

  // eval securely
  if (duk_peval_lstring(ctx, js_script, js_script_size) != 0)
  {
    NlpThrowErrorTh(ctrl_nlp_th, "%.*s", js_script_size, js_script);
    NlpThrowErrorTh(ctrl_nlp_th, "\n// ====== Eval =======\n// Error : %s\n", duk_safe_to_string(ctx, -1));
    for (GList *iter = ctrl_nlp_th->js->variable_list->tail; iter; iter = iter->next)
    {
      og_string variable = iter->data;
      NlpThrowErrorTh(ctrl_nlp_th, "%s", variable);
    }
    NlpThrowErrorTh(ctrl_nlp_th, "NlpJsEval: duk_peval_lstring eval failed: %s :\n// ===== Context =====", duk_safe_to_string(ctx, -1));

    DPcErr;
  }

  // get result
  duk_int_t type = duk_get_type(ctrl_nlp_th->js->duk_context, -1);
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

        NlpLog(DOgNlpTraceJs, "NlpJsEval : computed value is a number (double): %g", computed_number);
      }

      break;
    }

    case DUK_TYPE_STRING:
    {
      og_string computed_string = duk_get_string(ctx, -1);
      *p_json_anwser = json_string(computed_string);

      NlpLog(DOgNlpTraceJs, "NlpJsEval : computed value is a string : '%s'", computed_string);
      break;
    }

    case DUK_TYPE_OBJECT:
    {
      // object, array, function
      og_string computed_json_string = duk_json_encode(ctx, -1);
      int computed_json_string_size = strlen(computed_json_string);

      // pure string value are surrounded by backquote "\"toto\""
      if (computed_json_string_size >= 2 && computed_json_string[0] == '"'
          && computed_json_string[computed_json_string_size - 1] == '"')
      {
        *p_json_anwser = json_stringn(computed_json_string + 1, computed_json_string_size - 2);
      }
      else
      {

        json_error_t error[1];
        *p_json_anwser = json_loadb(computed_json_string, computed_json_string_size, 0, error);
        if (*p_json_anwser == NULL)
        {
          NlpThrowErrorTh(ctrl_nlp_th,
              "NlpJsEval : computed json object contains error in ligne %d and column %d , %s , %s \n'%s'", error->line,
              error->column, error->source, error->text, computed_json_string);
          DPcErr;
        }

      }

      NlpLog(DOgNlpTraceJs, "NlpJsEval : computed value is an object/array : '%s'", computed_json_string);

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

og_status NlpJsAddVariable(og_nlp_th ctrl_nlp_th, og_string variable_name, og_string variable_eval)
{
  int variable_name_size = strlen(variable_name);
  if (variable_name_size == 0)
  {
    CONT;
  }

  int variable_eval_size = strlen(variable_eval);
  if (variable_eval_size == 0)
  {
    variable_eval = "null";
    variable_eval_size = 4;
  }

  og_string var_template = "const %s = %s;";
  int var_template_size = strlen(var_template);
  int var_command_size = variable_eval_size + variable_name_size + var_template_size;

  og_char_buffer var_command[var_command_size];
  snprintf(var_command, var_command_size, var_template, variable_name, variable_eval);

  NlpLog(DOgNlpTraceJs, "Sending variable to duktape: %s", var_command);

  duk_context *ctx = ctrl_nlp_th->js->duk_context;
  if (ctx == NULL)
  {
    NlpLog(DOgNlpTraceJs, "NlpJsEval no javascript ctx initialised");
    CONT;
  }

  if (duk_peval_lstring(ctx, var_command, strlen(var_command)) != 0)
  {
    NlpThrowErrorTh(ctrl_nlp_th, "NlpJsAddVariable: duk_peval_lstring eval failed: %s : '%s'",
        duk_safe_to_string(ctx, -1), var_command);
    DPcErr;
  }

  // keep variable value for better error message
  char *new_var = g_string_chunk_insert(ctrl_nlp_th->js->varibale_values, var_command);
  g_queue_push_tail(ctrl_nlp_th->js->variable_list, new_var);

  DONE;
}

og_status NlpJsSetNow(og_nlp_th ctrl_nlp_th)
{
  // TODO improve NlpJsSetNow call to one time per request

  og_string date_now = ctrl_nlp_th->date_now;
  og_string offset_setup = ""   //
          "moment.updateOffset = function (m) { return m.utcOffset(_now_form_request.utcOffset()); };\n"//
          "moment.now = function () { return _now_form_request.toDate(); };\n"//
          "moment.fn.toJSON = function() { return this.format(); };\n";

  og_char_buffer now_setup_command[DPcPathSize];
  if (date_now == NULL || date_now[0] == '\0')
  {
    snprintf(now_setup_command, DPcPathSize, "const _now_form_request = moment.utc();\n%s", offset_setup);
  }
  else
  {
    snprintf(now_setup_command, DPcPathSize, "const _now_form_request = moment.parseZone('%s');\n%s", date_now, offset_setup);
  }


  NlpLog(DOgNlpTraceJs, "Sending command to duktape: %s", now_setup_command);

  duk_context *ctx = ctrl_nlp_th->js->duk_context;
  if (ctx == NULL)
  {
    NlpLog(DOgNlpTraceJs, "NlpJsSetNow no javascript ctx initialised");
    CONT;
  }

  if (duk_peval_string(ctx, now_setup_command) != 0)
  {
    NlpThrowErrorTh(ctrl_nlp_th, "NlpJsSetNow: duk_peval_lstring eval failed: %s : '%s'", duk_safe_to_string(ctx, -1),
        now_setup_command);
    DPcErr;
  }

  // keep variable value for better error message
  char *new_var = g_string_chunk_insert(ctrl_nlp_th->js->varibale_values, now_setup_command);
  g_queue_push_tail(ctrl_nlp_th->js->variable_list, new_var);

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

  return NlpJsAddVariable(ctrl_nlp_th, variable_name, variable_eval_buffer);
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

