/*
 *  Test program for the Natural Language Processing libognlp.so library.
 *  Copyright (c) 2027 Pertimm by Patrick Constant
 *  Dev : September 2017
 *  Version 1.0
 */
#include <lognlp.h>
#include <logxml.h>
#include <logmsg.h>
#include <getopt.h>
#include <logpath.h>
#include <logheap.h>
#include <jansson.h>

struct og_filename
{
  int start;
  int length;
};

/* structure de controle du programme */
struct og_info
{
  void *hmsg, *herr;
  ogmutex_t *hmutex;
  struct og_nlp_param *param;
  og_heap hfilename_ba, hfilename;
  og_nlp hnlp;
  og_nlpi hnlpi;
  char output_filename[DPcPathSize];
  char interpret_filename[DPcPathSize];
  int dump;
};

/* functions for using main api */
static int nlp(struct og_info *info, int argc, char * argv[]);
static int nlp_compile(struct og_info *info, char *json_compilation_filename);
static og_status nlp_dump(struct og_info *info, char *json_compilation_filename);
static int nlp_interpret(struct og_info *info, char *json_interpret_filename);
static int nlp_send_errors_as_json(struct og_info *info);

/* default function to define */
static int OgUse(struct og_info *);
static void OgExit(struct og_info *);

int main(int argc, char * argv[])
{
  struct og_info info[1];
  struct og_nlp_param param[1];
  time_t ltime;

  /* initialization of program info structure */
  memset(info, 0, sizeof(struct og_info));
  info->param = param;
  IFn(info->hmsg = OgLogInit("ognlp", "ognlp", DOgMsgTraceMinimal + DOgMsgTraceMemory, DOgMsgLogFile)) exit(1);
  info->herr = OgLogGetErr(info->hmsg);
  info->hmutex = OgLogGetMutex(info->hmsg);

  memset(param, 0, sizeof(struct og_nlp_param));
  param->hmsg = info->hmsg;
  param->herr = info->herr;
  param->hmutex = info->hmutex;
  param->loginfo.trace = DOgNlpTraceMinimal + DOgNlpTraceMemory;
  param->loginfo.where = "ognlp";

  if (param->loginfo.trace & DOgNlpTraceMinimal)
  {
    time(&ltime);
    OgMsg(info->hmsg, "", DOgMsgDestInLog, "\nProgram %s starting with pid %x at %.24s", OgLogGetModuleName(info->hmsg),
        getpid(), OgGmtime(&ltime));
  }

  IF(nlp(info,argc,argv))
  {
    OgExit(info);
  }

  if (param->loginfo.trace & DOgNlpTraceMinimal)
  {
    time(&ltime);
    OgMsg(info->hmsg, "", DOgMsgDestInLog, "\nProgram %s exiting at %.24s\n", OgLogGetModuleName(info->hmsg),
        OgGmtime(&ltime));
  }

  IFE(OgLogFlush(info->hmsg));

  return (0);
}

static int nlp(struct og_info *info, int argc, char * argv[])
{
  struct og_nlp_param *param = info->param;

  IFn(info->hfilename=OgHeapInit(info->hmsg,"ognlp_filename",sizeof(struct og_filename),0x10)) return (0);
  IFn(info->hfilename_ba=OgHeapInit(info->hmsg,"ognlp_filename_ba",sizeof(unsigned char),0x100)) return (0);

  /* definition of program options */
  struct option longOptions[] = { { "compile", required_argument, NULL, 'c' },   //
      { "interpret", required_argument, NULL, 'i' },   //
      { "output", required_argument, NULL, 'o' },   //
      { "dump", no_argument, NULL, 'd' },   //
      { "help", no_argument, NULL, 'h' },   //
      { "trace", required_argument, NULL, 't' },   //
      { 0, 0, 0, 0 }   //
  };

  char *nil, carlu;
  int optionIndex;

  /* parsing options to exclusion of compilation and interpretation files */
  optionIndex = 0;
  while ((carlu = getopt_long(argc, argv, "c:i:o:t:dh?", longOptions, &optionIndex)) != EOF)
  {
    struct og_filename filename[1];
    switch (carlu)
    {
      case 0:
        break;
      case 'c': /* compiling file 'optarg' */
        filename->start = OgHeapGetCellsUsed(info->hfilename_ba);
        filename->length = strlen(optarg);
        IFE(OgHeapAppend(info->hfilename_ba, filename->length + 1, optarg));   // +1 because we want to keep the zero
        IFE(OgHeapAppend(info->hfilename, 1, filename));
        break;
      case 'i': /* interpret file 'optarg' */
        strcpy(info->interpret_filename, optarg);
        break;
      case 'd': /* dump */
        info->dump = TRUE;
        break;
      case 'o': /* output_filename */
        strcpy(info->output_filename, optarg);
        break;
      case 't':
        param->loginfo.trace = strtol(optarg, &nil, 16);
        break;
      case 'h': /* help */
      case '?':
        OgUse(info);
        DONE;
        break;
    }
  }

  IFN(info->hnlp = OgNlpInit(info->param)) DPcErr;

  int nb_compilation_files = OgHeapGetCellsUsed(info->hfilename);
  for (int i = 0; i < nb_compilation_files; i++)
  {
    struct og_filename *filename = OgHeapGetCell(info->hfilename, i);
    char *compilation_filename = OgHeapGetCell(info->hfilename_ba, filename->start);
    IFE(nlp_compile(info, compilation_filename));
  }

  if (info->dump == TRUE)
  {
    if (info->output_filename[0] == 0) sprintf(info->output_filename, "/dev/stdout");
    IFE(nlp_dump(info, info->output_filename));
  }

  if (info->interpret_filename[0])
  {
    IFE(nlp_interpret(info, info->interpret_filename));
  }

  DONE;
}

static int nlp_compile(struct og_info *info, char *json_compilation_filename)
{
  OgMsg(info->hmsg, "", DOgMsgDestInLog, "Compiling '%s'", json_compilation_filename);

  json_error_t error[1];
  json_auto_t *json = json_load_file(json_compilation_filename, JSON_REJECT_DUPLICATES, error);
  IFN(json)
  {
    OgErr(info->herr, "nlp_compile: error while reading 'json_compilation_filename'");
    nlp_send_errors_as_json(info);
    DPcErr;
  }
  struct og_nlp_compile_input input[1];
  struct og_nlp_compile_output output[1];

  memset(input, 0, sizeof(struct og_nlp_compile_input));
  input->json_input = json;
  input->filename = json_compilation_filename;

  IF(OgNlpCompile(info->hnlp, input, output))
  {
    nlp_send_errors_as_json(info);
    DPcErr;
  }

  IFN(output->json_output)
  {
    OgErr(info->herr, "nlp_compile: null output->json_output");
    nlp_send_errors_as_json(info);
    DPcErr;
  }
  else
  {
    og_status status = json_dump_file(output->json_output, "/dev/stdout", JSON_INDENT(2));
    printf("\n");
    json_decref(output->json_output);
    IF(status)
    {
      char buffer[DPcPathSize];
      sprintf(buffer, "nlp_compile: error on json_dump_file");
      OgErr(info->herr, buffer);
      DPcErr;
    }
  }

  DONE;
}

static og_status nlp_dump(struct og_info *info, char *json_dump_filename)
{
  OgMsg(info->hmsg, "", DOgMsgDestInLog, "Dumping '%s'", json_dump_filename);

  json_error_t error[1];
  json_auto_t *json = json_load_file(json_dump_filename, JSON_REJECT_DUPLICATES, error);
//  IFN(json)
//  {
//    DPcErr;
//  }
  struct og_nlp_dump_input input[1];
  struct og_nlp_dump_output output[1];

  memset(input, 0, sizeof(struct og_nlp_dump_input));
  input->json_input = json;

  IFE(OgNlpDump(info->hnlp, input, output));

  IFN(output->json_output)
  {
    OgErr(info->herr, "nlp_dump: null output->json_output");
    DPcErr;
  }
  else
  {
    if (info->output_filename != NULL)
    {
      og_status status = json_dump_file(output->json_output, info->output_filename, JSON_INDENT(2));
      json_decref(output->json_output);
      IF(status)
      {
        char buffer[DPcPathSize];
        sprintf(buffer, "nlp_dump: error on json_dump_file");
        OgErr(info->herr, buffer);
        DPcErr;
      }
    }
  }

  DONE;
}

static int nlp_interpret(struct og_info *info, char *json_interpret_filename)
{
  OgMsg(info->hmsg, "", DOgMsgDestInLog, "Compiling '%s'", json_interpret_filename);

  json_error_t error[1];
  json_auto_t *json = json_load_file(json_interpret_filename, JSON_REJECT_DUPLICATES, error);
  IFN(json)
  {
    OgErr(info->herr, "nlp_interpret: error while reading 'json_compilation_filename'");
    nlp_send_errors_as_json(info);
    DPcErr;
  }
  struct og_nlp_interpret_input input[1];
  struct og_nlp_interpret_output output[1];

  memset(input, 0, sizeof(struct og_nlp_interpret_input));
  input->json_input = json;

  struct og_nlpi_param param[1];
  memset(param,0,sizeof(struct og_nlpi_param));
  param->herr = info->param->herr;
  param->hmsg = info->param->hmsg;
  param->hmutex = info->param->hmutex;
  param->loginfo = info->param->loginfo;
  info->hnlpi = OgNlpInterpretInit(info->hnlp,param);
  IFN(info->hnlpi)
  {
    nlp_send_errors_as_json(info);
    DPcErr;
  }

  IF(OgNlpInterpret(info->hnlpi, input, output))
  {
    nlp_send_errors_as_json(info);
    DPcErr;
  }

  IFN(output->json_output)
  {
    OgErr(info->herr, "nlp_interpret: null output->json_output");
    nlp_send_errors_as_json(info);
    DPcErr;
  }
  else
  {
    og_status status = json_dump_file(output->json_output, "/dev/stdout", JSON_INDENT(2));
    printf("\n");
    json_decref(output->json_output);
    IF(status)
    {
      char buffer[DPcPathSize];
      sprintf(buffer, "nlp_interpret: error on json_dump_file");
      OgErr(info->herr, buffer);
      DPcErr;
    }
  }

  DONE;
}

static int nlp_send_errors_as_json(struct og_info *info)
{
  json_t * root = json_object();
  json_t * errors = json_array();

  int nb_error = 0;
  char erreur[DOgErrorSize];
  while (OgErrLast(info->herr, erreur, 0))
  {
    json_array_append(errors, json_string(erreur));
    nb_error++;
  }

  int h = 0;
  while (PcErrDiag(&h, erreur))
  {
    json_array_append(errors, json_string(erreur));
    nb_error++;
  }

  json_object_set(root, "errors", errors);

  json_dump_file(root, "/dev/stdout", JSON_INDENT(2));
  printf("\n");

  json_decref(errors);
  json_decref(root);
  DONE;
}

static int OgUse(struct og_info *info)
{
  char buffer[8192];
  int ibuffer = 0;

  ibuffer += sprintf(buffer, "Usage : oginterpret [options]\n");
  ibuffer += sprintf(buffer + ibuffer, "options are:\n");
  ibuffer += sprintf(buffer + ibuffer, "   -c,  --compile=<compilation_filename>: input context filters\n");
  ibuffer += sprintf(buffer + ibuffer, "   -i,  --interpret=<input_filename>: specify input filename\n");
  ibuffer += sprintf(buffer + ibuffer, "   -o,  --output=<output_filename>: "
      "specify output filename (defaut is stdout)\n");
  ibuffer += sprintf(buffer + ibuffer, "   -h,  --help prints this message\n");
  ibuffer += sprintf(buffer + ibuffer, "   -t<n>: trace options for "
      "logging (default 0x%x)\n", info->param->loginfo.trace);
  ibuffer += sprintf(buffer + ibuffer, "    <n> has a combined hexadecimal value of:\n");
  ibuffer += sprintf(buffer + ibuffer, "      0x1: minimal, 0x2: memory, 0x4: conf, 0x8: input\n");
  ibuffer += sprintf(buffer + ibuffer, "      0x10: ignore, 0x20: reparse, 0x40: match, 0x80: select\n");
  ibuffer += sprintf(buffer + ibuffer, "      0x100: output\n");

  OgLogConsole(info->hmsg, "%.*s", ibuffer, buffer);

  DONE;
}

static void OgExit(struct og_info *info)
{
  time_t ltime;
  OgMsgErr(info->hmsg, "ogintepret_error", 0, 0, 0, DOgMsgSeverityEmergency, 0);
  time(&ltime);
  OgMsg(info->hmsg, "", DOgMsgDestInLog + DOgMsgDestInErr + DOgMsgSeverityEmergency,
      "Program %s exiting on error at %.24s\n", OgLogGetModuleName(info->hmsg), OgGmtime(&ltime));

  exit(1);
}

