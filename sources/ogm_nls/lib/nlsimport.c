/*
 *  Reading import files for initialization
 *  Copyright (c) 2017 Pertimm by Patrick Constant
 *  Dev: September 2027
 *  Version 1.0
 */
#include "ogm_nls.h"

static int NlsReadImportFilesBuildList(og_nls ctrl_nls, GQueue *queue_files);
static int NlsReadImportFilesBuildList1(og_nls ctrl_nls, GQueue *queue_files, char *pattern);
static int NlsReadImportFile(og_nls ctrl_nls, char *import_file);
static og_status NlsReadImportFilesFreeSafe(og_nls ctrl_nls, GQueue *queue_files);

og_status NlsReadImportFiles(og_nls ctrl_nls)
{
  struct og_stat filestat;

  IFx(OgStat(ctrl_nls->import_directory,DOgStatMask_is_dir,&filestat))
  {
    NlsMainThrowError(ctrl_nls, "NlsReadImportFiles: impossible to stat data_directory '%s'",
        ctrl_nls->import_directory);
    DPcErr;
  }

  GQueue *queue_files = g_queue_new();

  og_status status = NlsReadImportFilesFreeSafe(ctrl_nls, queue_files);

  g_queue_free_full(queue_files, g_free);

  IFE(status);

  DONE;
}

static og_status NlsReadImportFilesFreeSafe(og_nls ctrl_nls, GQueue *queue_files)
{
  IFE(NlsReadImportFilesBuildList(ctrl_nls, queue_files));

  for (GList *iter = queue_files->head; iter; iter = iter->next)
  {
    char *import_file = iter->data;
    OgMsg(ctrl_nls->hmsg, "", DOgMsgDestInLog, "NlsReadImportFilesBuildList: compiling file '%s'", import_file);
    IFE(NlsReadImportFile(ctrl_nls, import_file));
  }

  DONE;
}

static int NlsReadImportFilesBuildList(og_nls ctrl_nls, GQueue *queue_files)
{
  int retour, found = 0;

  IFE(retour = NlsReadImportFilesBuildList1(ctrl_nls, queue_files, "*.json"));
  if (retour) found = 1;
  IFE(retour = NlsReadImportFilesBuildList1(ctrl_nls, queue_files, "*.json.gz"));
  if (retour) found = 1;

  g_queue_sort(queue_files, (GCompareDataFunc) g_strcmp0, NULL);

  return (found);
}

static int NlsReadImportFilesBuildList1(og_nls ctrl_nls, GQueue *queue_files, char *pattern)
{
  char import_file[DPcPathSize], search_path[DPcPathSize];
  struct og_file cstr_file, *str_file = &cstr_file;
  struct og_stat filestat;
  int retour, found = 0;

  memset(str_file, 0, sizeof(struct og_file));
  sprintf(search_path, "%s/%s", ctrl_nls->import_directory, pattern);
  IFE(retour = OgFindFirstFile(str_file, search_path));
  if (retour)
  {
    do
    {
      sprintf(import_file, "%s/%s", ctrl_nls->import_directory, str_file->File_Path);
      IFx(OgStat(import_file,DOgStatMask_mtime,&filestat)) continue;
      if (filestat.is_dir) continue;
      char *queue_import_file = strdup(import_file);
      g_queue_push_tail(queue_files, queue_import_file);

      found = 1;
    }
    while (OgFindNextFile(str_file));
    OgFindClose(str_file);
  }
  return (found);
}

static int NlsReadImportFile(og_nls ctrl_nls, char *import_file)
{
  json_error_t error[1];
  json_auto_t *json = json_load_file(import_file, JSON_REJECT_DUPLICATES, error);
  IFN(json)
  {
    NlsMainThrowError(ctrl_nls, "NlsReadImportFile: Json contains error in ligne %d and column %d , %s , %s ",
        error->line, error->column, error->source, error->text);
    NlsMainThrowError(ctrl_nls, "NlsReadImportFile: error while reading '%s'", import_file);
    DPcErr;
  }

  struct og_nlp_compile_input input[1];
  struct og_nlp_compile_output output[1];

  memset(input, 0, sizeof(struct og_nlp_compile_input));
  input->json_input = json;

  IF(OgNlpCompile(ctrl_nls->hnlp, input, output))
  {
    json_decrefp(&output->json_output);
    NlsMainThrowError(ctrl_nls, "NlsReadImportFile: error OgNlpCompile from file '%s'", import_file);
    DPcErr;
  }

  IFN(output->json_output)
  {
    json_decrefp(&output->json_output);
    NlsMainThrowError(ctrl_nls, "NlsReadImportFile: OgNlpCompile returns null json from '%s'", import_file);
    DPcErr;
  }

  json_decrefp(&output->json_output);

  DONE;
}

