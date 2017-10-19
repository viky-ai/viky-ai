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
#define DOgNlpTraceSynchro                0x4
#define DOgNlpTraceDefault               (DOgNlpTraceMinimal+DOgNlpTraceMemory)

/** Default value for configuration file information **/
#define DOgNlpMaxListeningThreads          4

typedef struct og_ctrl_nlp *og_nlp;
typedef struct og_ctrl_nlp_threaded *og_nlp_th;

struct og_nlp_param
{
  void *herr, *hmsg;
  ogmutex_t *hmutex;
  struct og_loginfo loginfo;
  char WorkingDirectory[DPcPathSize];
  char configuration_file[DPcPathSize];
};

struct og_nlp_threaded_param
{
  void *herr, *hmsg;
  ogmutex_t *hmutex;
  struct og_loginfo loginfo;

  og_string name;
};

struct og_nlp_compile_input
{
  json_t *json_input;

  /** TRUE if the package can be updated, FALSE if package must not exits (duplicate detection) */
  og_bool package_update;

  /** Used for add or update of package and NOT for initial loading */
  og_string package_id;

};

struct og_nlp_compile_output
{
  json_t *json_output;
};

struct og_nlp_dump_input
{
  /** Array of String : package name to dump, empty/NULL array means dump all package */
  json_t *package_names;
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

DEFPUBLIC(og_nlp_th) OgNlpThreadedInit(og_nlp ctrl_nlp, struct og_nlp_threaded_param *param);
DEFPUBLIC(og_status) OgNlpThreadedReset(og_nlp_th ctrl_nlp_th);
DEFPUBLIC(og_status) OgNlpThreadedFlush(og_nlp_th ctrl_nlp_th);

DEFPUBLIC(og_status) OgNlpSynchroTestRegisterTimeout(og_nlp_th ctrl_nlp_th, og_string timeout_in);

DEFPUBLIC(og_status) OgNlpCompile(og_nlp_th ctrl_nlp_th, struct og_nlp_compile_input *input,
    struct og_nlp_compile_output *output);
DEFPUBLIC(og_status) OgNlpDump(og_nlp_th ctrl_nlp_th, struct og_nlp_dump_input *input,
    struct og_nlp_dump_output *output);

DEFPUBLIC(og_status) OgNlpInterpret(og_nlp_th ctrl_nlp_th, struct og_nlp_interpret_input *input,
    struct og_nlp_interpret_output *output);

DEFPUBLIC(og_status) OgNlpPackageAdd(og_nlp_th ctrl_nlp_th, struct og_nlp_compile_input *input);
DEFPUBLIC(og_status) OgNlpPackageDelete(og_nlp_th ctrl_nlp_th, og_string package_id);

DEFPUBLIC(og_status) OgNlpFlushPackageMarkedAsDeleted(og_nlp_th ctrl_nlp_th);

#define _LOGNLPALIVE_
#endif

