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
static int NlsControlImportFile(og_nls ctrl_nls, json_t *json, char *import_file);

og_status NlsReadImportFiles(og_nls ctrl_nls)
{
  struct og_stat filestat;

  IFx(OgStat(ctrl_nls->import_directory,DOgStatMask_is_dir,&filestat))
  {
    NlsMainThrowError(ctrl_nls, "NlsReadImportFiles: impossible to stat data_directory '%s'",
        ctrl_nls->import_directory);
    DPcErr;
  }

  GQueue queue_files[1];
  g_queue_init(queue_files);
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
    NlsMainThrowError(ctrl_nls, "NlsReadImportFile: error while reading '%s'", import_file);
    NlsMainThrowError(ctrl_nls, "NlsReadImportFile: Json contains error in ligne %d and column %d , %s , %s ",
        error->line, error->column, error->source, error->text);
    DPcErr;
  }

  IFE(NlsControlImportFile(ctrl_nls, json, import_file));

  struct og_nlp_compile_input input[1];
  struct og_nlp_compile_output output[1];

  memset(input, 0, sizeof(struct og_nlp_compile_input));
  input->json_input = json;

  IFE(OgNlpCompile(ctrl_nls->hnlp, input, output));

  IFN(output->json_output)
  {
    NlsMainThrowError(ctrl_nls, "NlsReadImportFile: OgNlpCompile returns null json from '%s'", import_file);
    DPcErr;
  }

  DONE;
}

static int NlsControlImportFile(og_nls ctrl_nls, json_t *json, char *import_file)
{
  GArray *packageList = g_array_new(FALSE, FALSE, sizeof(char*));
  int packageListNumber = 0;
  GArray *intentList = g_array_new(FALSE, FALSE, sizeof(char*));
  int intentListNumber = 0;

  // shape of the file
  if (json_is_array(json))
  {
    json_t *json_package;
    size_t package_iterator;
    json_array_foreach(json,package_iterator,json_package)
    {

      if (json_is_object(json_package) == FALSE)
      {
        g_array_free(packageList, TRUE);
        g_array_free(intentList, TRUE);
        NlsMainThrowError(ctrl_nls,
            "NlsControleImportFile: content of '%s' does not fit expected format : package element is not a map",
            import_file);
        DPcErr;
      }

      if (json_object_size(json_package) != 2)
      {
        g_array_free(packageList, TRUE);
        g_array_free(intentList, TRUE);
        NlsMainThrowError(ctrl_nls,
            "NlsControleImportFile: content of '%s' does not fit expected format : package element must have 2 items",
            import_file);
        DPcErr;
      }

      json_t *json_package_id = json_object_get(json_package, "id");
      if (json_package_id == NULL)
      {
        g_array_free(packageList, TRUE);
        g_array_free(intentList, TRUE);
        NlsMainThrowError(ctrl_nls,
            "NlsControleImportFile: content of '%s' does not fit expected format : package element must have an \"id\" item",
            import_file);
        DPcErr;
      }
      if (json_is_string(json_package_id) == FALSE)
      {
        g_array_free(packageList, TRUE);
        g_array_free(intentList, TRUE);
        NlsMainThrowError(ctrl_nls,
            "NlsControleImportFile: content of '%s' does not fit expected format : package id must be a string",
            import_file);
        DPcErr;
      }
      const char* string_package_id = json_string_value(json_package_id);
      for (int i = 0; i < packageListNumber; i++)
      {
        if (strcmp(string_package_id, g_array_index(packageList, char*, i)) == 0)
        {
          g_array_free(packageList, TRUE);
          g_array_free(intentList, TRUE);
          NlsMainThrowError(ctrl_nls,
              "NlsControleImportFile: content of '%s' does not fit expected format : several packages with same id : %s",
              import_file, string_package_id);
          DPcErr;
        }
      }

      g_array_append_val(packageList, string_package_id);
      packageListNumber++;

      json_t *json_intents = json_object_get(json_package, "intents");
      IFN(json_intents)
      {
        g_array_free(packageList, TRUE);
        g_array_free(intentList, TRUE);
        NlsMainThrowError(ctrl_nls,
            "NlsControleImportFile: content of '%s' does not fit expected format : package element must have an \"intents\" item",
            import_file);
        DPcErr;
      }
      if (json_is_array(json_intents) == FALSE)
      {
        g_array_free(packageList, TRUE);
        g_array_free(intentList, TRUE);
        NlsMainThrowError(ctrl_nls,
            "NlsControleImportFile: content of '%s' does not fit expected format : intents element is not an array",
            import_file);
        DPcErr;
      }

      json_t *json_intent;
      size_t intent_iterator;
      json_array_foreach(json_intents,intent_iterator,json_intent)
      {
        if (json_is_object(json_intent) == FALSE)
        {
          g_array_free(packageList, TRUE);
          g_array_free(intentList, TRUE);
          NlsMainThrowError(ctrl_nls,
              "NlsControleImportFile: content of '%s' does not fit expected format : intent element is not a map",
              import_file);
          DPcErr;
        }
        if (json_object_size(json_intent) != 2)
        {
          g_array_free(packageList, TRUE);
          g_array_free(intentList, TRUE);
          NlsMainThrowError(ctrl_nls,
              "NlsControleImportFile: content of '%s' does not fit expected format : intent element must have 2 items",
              import_file);
          DPcErr;
        }

        json_t *json_intent_id = json_object_get(json_intent, "id");
        if (json_intent_id == NULL)
        {
          g_array_free(packageList, TRUE);
          g_array_free(intentList, TRUE);
          NlsMainThrowError(ctrl_nls,
              "NlsControleImportFile: content of '%s' does not fit expected format : intent element must have an \"id\" item",
              import_file);
          DPcErr;
        }
        if (json_is_string(json_intent_id) == FALSE)
        {
          g_array_free(packageList, TRUE);
          g_array_free(intentList, TRUE);
          NlsMainThrowError(ctrl_nls,
              "NlsControleImportFile: content of '%s' does not fit expected format : intent id must be a string",
              import_file);
          DPcErr;
        }
        const char* textIntentId = json_string_value(json_intent_id);

        for (int i = 0; i < intentListNumber; i++)
        {
          const char* storedIntentId = g_array_index(intentList, char*, i);
          if (strcmp(textIntentId, storedIntentId) == 0)
          {
            g_array_free(packageList, TRUE);
            g_array_free(intentList, TRUE);
            NlsMainThrowError(ctrl_nls,
                "NlsControleImportFile: content of '%s' does not fit expected format : several intents with same id : %s",
                import_file, textIntentId);
            DPcErr;
          }
        }

        g_array_append_val(intentList, textIntentId);
        intentListNumber++;

        json_t *json_sentences = json_object_get(json_intent, "sentences");
        IFN(json_sentences)
        {
          g_array_free(packageList, TRUE);
          g_array_free(intentList, TRUE);
          NlsMainThrowError(ctrl_nls,
              "NlsControleImportFile: content of '%s' does not fit expected format : intent element must have a \"sentences\" item",
              import_file);
          DPcErr;
        }
        if (json_is_array(json_sentences) == FALSE)
        {
          g_array_free(packageList, TRUE);
          g_array_free(intentList, TRUE);
          NlsMainThrowError(ctrl_nls,
              "NlsControleImportFile: content of '%s' does not fit expected format : sentences element must be an array",
              import_file);
          DPcErr;
        }

        json_t *json_sentence;
        size_t sentence_iterator;
        json_array_foreach(json_sentences,sentence_iterator,json_sentence)
        {
          if (json_is_object(json_sentence) == FALSE)
          {
            g_array_free(packageList, TRUE);
            g_array_free(intentList, TRUE);
            NlsMainThrowError(ctrl_nls,
                "NlsControleImportFile: content of '%s' does not fit expected format : sentences elements are not maps",
                import_file);
            DPcErr;
          }
          if (json_object_size(json_sentence) != 2)
          {
            g_array_free(packageList, TRUE);
            g_array_free(intentList, TRUE);
            NlsMainThrowError(ctrl_nls,
                "NlsControleImportFile: content of '%s' does not fit expected format : sentences elements must have 2 items",
                import_file);
            DPcErr;
          }

          json_t *json_sentence_text = json_object_get(json_sentence, "sentence");
          IFN(json_sentence_text)
          {
            g_array_free(packageList, TRUE);
            g_array_free(intentList, TRUE);
            NlsMainThrowError(ctrl_nls,
                "NlsControleImportFile: content of '%s' does not fit expected format : sentences elements must have a \"sentence\" item",
                import_file);
            DPcErr;
          }
          if (json_is_string(json_sentence_text) == FALSE)
          {
            g_array_free(packageList, TRUE);
            g_array_free(intentList, TRUE);
            NlsMainThrowError(ctrl_nls,
                "NlsControleImportFile: content of '%s' does not fit expected format : sentence item must be a string",
                import_file);
            DPcErr;
          }

          json_t *json_locale = json_object_get(json_sentence, "locale");
          IFN(json_locale)
          {
            g_array_free(packageList, TRUE);
            g_array_free(intentList, TRUE);
            NlsMainThrowError(ctrl_nls,
                "NlsControleImportFile: content of '%s' does not fit expected format : sentences elements must have a \"locale\" item",
                import_file);
            DPcErr;
          }
          if (json_is_string(json_locale) == FALSE)
          {
            g_array_free(packageList, TRUE);
            g_array_free(intentList, TRUE);
            NlsMainThrowError(ctrl_nls,
                "NlsControleImportFile: content of '%s' does not fit expected format : locale item must be a string",
                import_file);
            DPcErr;
          }

        }

      }
    }
  }
  else
  {
    g_array_free(packageList, TRUE);
    g_array_free(intentList, TRUE);
    NlsMainThrowError(ctrl_nls,
        "NlsControleImportFile: content of '%s' does not fit expected format : main container is not an array",
        import_file);
    DPcErr;
  }

  g_array_free(packageList, TRUE);
  g_array_free(intentList, TRUE);

  DONE;
}
