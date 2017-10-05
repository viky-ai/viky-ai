/*
 *  Header for library libognlp.dll
 *  Copyright (c) 2017 Pertimm by P.Constant
 *  Dev : August 2017
 *  Version 1.0
 * Natural Language Processing library
 */
#ifndef _LOGNLPALIVE_

#include <jansson.h>

#include <lpcgentype.h>
#include <loggen.h>
#include <logmsg.h>

#define DOgNlpBanner  "libognlp V1.00, Copyright (c) 2017 Pertimm"
#define DOgNlpVersion 100

/** Trace levels **/
#define DOgNlpTraceMinimal                0x1
#define DOgNlpTraceMemory                 0x2
#define DOgNlpTraceDefault               (DOgNlpTraceMinimal+DOgNlpTraceMemory)

/** Default value for configuration file information **/
#define DOgNlpMaxListeningThreads          4

typedef struct og_ctrl_nlp *og_nlp;
typedef struct og_ctrl_nlpi *og_nlpi;

struct og_nlp_param
{
  void *herr, *hmsg;
  ogmutex_t *hmutex;
  struct og_loginfo loginfo;
  char WorkingDirectory[DPcPathSize];
  char configuration_file[DPcPathSize];
};

struct og_nlpi_param
{
  void *herr, *hmsg;
  ogmutex_t *hmutex;
  struct og_loginfo loginfo;
};

struct og_nlp_compile_input
{
  json_t *json_input;

  /** TRUE if the package can be updated, FALSE if package must not exits (duplicate detection) */
  og_bool package_update;

};

struct og_nlp_compile_output
{
  json_t *json_output;
};

struct og_nlp_dump_input
{
  json_t *json_input;
};

struct og_nlp_dump_output
{
  json_t *json_output;
};

struct og_nlp_interpret_input
{
  json_t *json_input;
};

struct og_nlp_interpret_output
{
  json_t *json_output;
};

DEFPUBLIC(og_nlp) OgNlpInit(struct og_nlp_param *param);
DEFPUBLIC(og_status) OgNlpFlush(og_nlp hnlp);

DEFPUBLIC(og_status) OgNlpCompile(og_nlp ctrl_nlp, struct og_nlp_compile_input *input, struct og_nlp_compile_output *output);
DEFPUBLIC(og_status) OgNlpDump(og_nlp ctrl_nlp, struct og_nlp_dump_input *input, struct og_nlp_dump_output *output);

DEFPUBLIC(og_nlpi) OgNlpInterpretInit(og_nlp ctrl_nlp, struct og_nlpi_param *param);
DEFPUBLIC(og_status) OgNlpInterpretReset(og_nlpi ctrl_nlpi);
DEFPUBLIC(og_status) OgNlpInterpretFlush(og_nlpi ctrl_nlpi);
DEFPUBLIC(og_status) OgNlpInterpret(og_nlpi ctrl_nlpi, struct og_nlp_interpret_input *input,
    struct og_nlp_interpret_output *output);

#define _LOGNLPALIVE_
#endif

