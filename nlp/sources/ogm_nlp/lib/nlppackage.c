/*
 *  Handling packages for Natural Language Processing library
 *  Copyright (c) 2017 Pertimm, by Patrick Constant
 *  Dev : September 2017
 *  Version 1.0
 */
#include "ogm_nlp.h"

static og_status NlpPackageFlush(package_t package);

package_t NlpPackageCreate(og_nlp_th ctrl_nlp_th, og_string string_id, og_string string_slug)
{
  ctrl_nlp_th->package_in_progress = (package_t) malloc(sizeof(struct package));
  IFn(ctrl_nlp_th->package_in_progress )
  {
    NlpThrowErrorTh(ctrl_nlp_th, "NlpPackageCreate: malloc error on package");
    return NULL;
  }
  package_t package = ctrl_nlp_th->package_in_progress;
  memset(package, 0, sizeof(struct package));
  package->ctrl_nlp = ctrl_nlp_th->ctrl_nlp;
  package->ref_counter = 0;
  package->id = strdup(string_id);
  package->slug = strdup(string_slug);

  void *hmsg = ctrl_nlp_th->ctrl_nlp->hmsg;
  og_char_buffer heap_name[DPcPathSize];

  // interpretation
  snprintf(heap_name, DPcPathSize, "package_interpretation_ba_%s", package->id);
  IFn(package->hinterpretation_ba = OgHeapInit(hmsg, heap_name, sizeof(unsigned char), DOgNlpPackageBaNumber)) return NULL;
  snprintf(heap_name, DPcPathSize, "package_interpretation_compile_%s", package->id);
  IFn(package->hinterpretation_compile = OgHeapInit(hmsg, heap_name, sizeof(struct interpretation_compile), DOgNlpPackageInterpretationNumber)) return NULL;
  snprintf(heap_name, DPcPathSize, "package_interpretation_%s", package->id);
  IFn(package->hinterpretation = OgHeapInit(hmsg, heap_name, sizeof(struct interpretation), 1)) return NULL;

  // context
  snprintf(heap_name, DPcPathSize, "package_context_ba_%s", package->id);
  IFn(package->hcontext_ba = OgHeapInit(hmsg, heap_name, sizeof(unsigned char), DOgNlpPackageBaNumber)) return NULL;
  snprintf(heap_name, DPcPathSize, "package_context_compile_%s", package->id);
  IFn(package->hcontext_compile = OgHeapInit(hmsg, heap_name, sizeof(struct context_compile), DOgNlpPackageContextNumber)) return NULL;
  snprintf(heap_name, DPcPathSize, "package_context_%s", package->id);
  IFn(package->hcontext = OgHeapInit(hmsg, heap_name, sizeof(struct context), 1)) return NULL;

  // expression
  snprintf(heap_name, DPcPathSize, "package_expression_ba_%s", package->id);
  IFn(package->hexpression_ba = OgHeapInit(hmsg, heap_name, sizeof(unsigned char), DOgNlpPackageBaNumber)) return NULL;
  snprintf(heap_name, DPcPathSize, "package_expression_compile_%s", package->id);
  IFn(package->hexpression_compile = OgHeapInit(hmsg, heap_name, sizeof(struct expression_compile), DOgNlpPackageExpressionNumber)) return NULL;
  snprintf(heap_name, DPcPathSize, "package_expression_%s", package->id);
  IFn(package->hexpression = OgHeapInit(hmsg, heap_name, sizeof(struct expression), 1)) return NULL;

  // alias
  snprintf(heap_name, DPcPathSize, "package_alias_ba_%s", package->id);
  IFn(package->halias_ba = OgHeapInit(hmsg, heap_name, sizeof(unsigned char), DOgNlpPackageBaNumber)) return NULL;
  snprintf(heap_name, DPcPathSize, "package_alias_compile_%s", package->id);
  IFn(package->halias_compile = OgHeapInit(hmsg, heap_name, sizeof(struct alias_compile), DOgNlpPackageAliasNumber)) return NULL;
  snprintf(heap_name, DPcPathSize, "package_alias_%s", package->id);
  IFn(package->halias = OgHeapInit(hmsg, heap_name, sizeof(struct alias), 1)) return NULL;

  // input_part
  snprintf(heap_name, DPcPathSize, "package_input_part_ba_%s", package->id);
  IFn(package->hinput_part_ba = OgHeapInit(hmsg, heap_name, sizeof(unsigned char), DOgNlpPackageBaNumber)) return NULL;
  snprintf(heap_name, DPcPathSize, "package_input_part_%s", package->id);
  IFn(package->hinput_part = OgHeapInit(hmsg, heap_name, sizeof(struct input_part), 1)) return NULL;

  snprintf(heap_name, DPcPathSize, "package_number_input_part_%s", package->id);
  IFn(package->hnumber_input_part = OgHeapInit(hmsg, heap_name, sizeof(struct number_input_part), 1)) return NULL;

  IF(NlpInputPartWordInit(ctrl_nlp_th, package)) return NULL;
  IF(NlpInputPartAliasInit(ctrl_nlp_th, package)) return NULL;
  IF(NlpRegexInit(ctrl_nlp_th, package)) return NULL;

  return (package);
}

static og_status NlpPackageAddOrReplaceNosync(og_nlp_th ctrl_nlp_th, package_t package)
{
  // Timeout trigger for synchro test
  IFE(OgNlpSynchroTestSleepIfTimeoutNeeded(ctrl_nlp_th, nlp_timeout_in_NlpPackageAddOrReplace));

  og_nlp ctrl_nlp = ctrl_nlp_th->ctrl_nlp;

  // update package and remove preview package : see NlpPackageDestroyIfNotUsed
  gpointer key = (unsigned char *) package->id;
  g_hash_table_replace(ctrl_nlp->packages_hash, key, package);
  ctrl_nlp_th->package_in_progress = NULL;

  DONE;
}

og_status NlpPackageAddOrReplace(og_nlp_th ctrl_nlp_th, package_t package)
{
  IF(OgNlpSynchroWriteLock(ctrl_nlp_th, ctrl_nlp_th->ctrl_nlp->rw_lock_packages_hash))
  {
    NlpThrowErrorTh(ctrl_nlp_th, "NlpPackageAddOrReplace: error on OgNlpSynchroWriteLock");
    DPcErr;
  }

  og_status status = NlpPackageAddOrReplaceNosync(ctrl_nlp_th, package);

  IF(OgNlpSynchroWriteUnLock(ctrl_nlp_th, ctrl_nlp_th->ctrl_nlp->rw_lock_packages_hash))
  {
    NlpThrowErrorTh(ctrl_nlp_th, "NlpPackageAddOrReplace: error on OgNlpSynchroWriteUnLock");
    DPcErr;
  }

  return status;
}

static package_t NlpPackageGetNosync(og_nlp_th ctrl_nlp_th, og_string package_id)
{

  // Timeout trigger for synchro test
  IF(OgNlpSynchroTestSleepIfTimeoutNeeded(ctrl_nlp_th, nlp_timeout_in_NlpPackageGet))
  {
    return NULL;
  }

  package_t package = g_hash_table_lookup(ctrl_nlp_th->ctrl_nlp->packages_hash, package_id);

  return package;
}

package_t NlpPackageGet(og_nlp_th ctrl_nlp_th, og_string package_id)
{

  IF(OgNlpSynchroReadLock(ctrl_nlp_th, ctrl_nlp_th->ctrl_nlp->rw_lock_packages_hash))
  {
    NlpThrowErrorTh(ctrl_nlp_th, "NlpPackageGet: error on OgNlpSynchroReadLock");
    return NULL;
  }

  package_t package = NlpPackageGetNosync(ctrl_nlp_th, package_id);

  if (package != NULL)
  {
    // flag package as used
    g_atomic_int_inc(&package->ref_counter);

    // keep all used package to free it in fallback reset
    g_queue_push_head(ctrl_nlp_th->package_in_used, package);
  }

  IF(OgNlpSynchroReadUnLock(ctrl_nlp_th, ctrl_nlp_th->ctrl_nlp->rw_lock_packages_hash))
  {
    NlpThrowErrorTh(ctrl_nlp_th, "NlpPackageGet: error on OgNlpSynchroReadUnLock");
    return NULL;
  }

  return package;
}

og_status NlpPackageMarkAsUnused(og_nlp_th ctrl_nlp_th, package_t package)
{
  if (package == NULL)
  {
    CONT;
  }

  // remove package in package_in_used list
  g_queue_remove(ctrl_nlp_th->package_in_used, package);

  // flag package as used
  og_bool no_more_used = g_atomic_int_dec_and_test(&package->ref_counter);
  if (no_more_used && package->is_removed)
  {
    IFE(NlpPackageFlush(package));
  }

  DONE;
}

og_status NlpPackageMarkAllInUsedAsUnused(og_nlp_th ctrl_nlp_th)
{

  NlpLog(DOgNlpTracePackage, "NlpPackageMarkAllInUsedAsUnused : marking %d packages",
      ctrl_nlp_th->package_in_used->length)

  // flush package mark as deleted
  package_t package = NULL;
  while ((package = g_queue_peek_head(ctrl_nlp_th->package_in_used)))
  {
    NlpPackageMarkAsUnused(ctrl_nlp_th, package);
  }

  DONE;
}

void NlpPackageDestroyIfNotUsed(gpointer package_void)
{
  package_t package = package_void;
  if (package != NULL)
  {

    // mark package as removed
    package->is_removed = TRUE;

    // if package is no more used, free it
    if (package->ref_counter <= 0)
    {
      NlpPackageFlush(package);
    }

  }
}

static og_status NlpPackageDeleteNosync(og_nlp_th ctrl_nlp_th, og_string package_id)
{
  og_nlp ctrl_nlp = ctrl_nlp_th->ctrl_nlp;

  package_t preview_package = g_hash_table_lookup(ctrl_nlp->packages_hash, package_id);

  if (preview_package == NULL)
  {
    NlpThrowErrorTh(ctrl_nlp_th, "NlpPackageDeleteNosync: unknown package");
    DPcErr;
  }

  // remove package lookup
  g_hash_table_remove(ctrl_nlp->packages_hash, package_id);

  DONE;
}

PUBLIC(og_status) OgNlpPackageAdd(og_nlp_th ctrl_nlp_th, struct og_nlp_compile_input *input)
{
  if (!json_is_object(input->json_input))
  {
    NlpThrowErrorTh(ctrl_nlp_th, "OgNlpPackageAdd: package must be an object");
    DPcErr;
  }

  json_t *json_package_id = json_object_get(input->json_input, "id");
  if (json_package_id == NULL)
  {
    NlpThrowErrorTh(ctrl_nlp_th, "OgNlpPackageAdd: package id is not present");
    DPcErr;
  }

  if (!json_is_string(json_package_id))
  {
    NlpThrowErrorTh(ctrl_nlp_th, "OgNlpPackageAdd: package id must be a string");
    DPcErr;

  }
  og_string package_id = json_string_value(json_package_id);
  if (!package_id[0])
  {
    NlpThrowErrorTh(ctrl_nlp_th, "OgNlpPackageAdd: package id must not empty");
    DPcErr;
  }

  // on verifie que les noms des packages matchent
  if (strcmp(input->package_id, package_id) != 0)
  {
    NlpThrowErrorTh(ctrl_nlp_th, "OgNlpPackageAdd: url package id (%s) is not the same as body package id (%s)",
        input->package_id, package_id);
    DPcErr;
  }

  IFE(NlpCompilePackage(ctrl_nlp_th, input, input->json_input));

  DONE;
}

PUBLIC(og_status) OgNlpPackageDelete(og_nlp_th ctrl_nlp_th, og_string package_id)
{
  IF(OgNlpSynchroWriteLock(ctrl_nlp_th, ctrl_nlp_th->ctrl_nlp->rw_lock_packages_hash))
  {
    NlpThrowErrorTh(ctrl_nlp_th, "NlpPackageRemoveNosync: error on OgNlpSynchroWriteLock");
    DPcErr;
  }

  og_status status = NlpPackageDeleteNosync(ctrl_nlp_th, package_id);

  IF(OgNlpSynchroWriteUnLock(ctrl_nlp_th, ctrl_nlp_th->ctrl_nlp->rw_lock_packages_hash))
  {
    NlpThrowErrorTh(ctrl_nlp_th, "NlpPackageRemoveNosync: error on OgNlpSynchroWriteUnLock");
    DPcErr;
  }

  return status;
}

static og_status NlpPackageInterpretationsFlush(package_t package)
{
  OgHeapFlush(package->hinterpretation_ba);
  package->hinterpretation_ba = NULL;

  OgHeapFlush(package->hinterpretation_compile);
  package->hinterpretation_compile = NULL;

  int interpretation_used = OgHeapGetCellsUsed(package->hinterpretation);
  for (int i = 0; i < interpretation_used; i++)
  {
    struct interpretation *interpretation = OgHeapGetCell(package->hinterpretation, i);
    IFN(interpretation) DPcErr;

    json_decrefp(&interpretation->json_solution);
  }

  OgHeapFlush(package->hinterpretation);
  package->hinterpretation = NULL;

  DONE;
}

static og_status NlpPackageContextsFlush(package_t package)
{
  OgHeapFlush(package->hcontext_ba);
  package->hcontext_ba = NULL;

  OgHeapFlush(package->hcontext_compile);
  package->hcontext_compile = NULL;

  OgHeapFlush(package->hcontext);
  package->hcontext = NULL;

  DONE;
}

static og_status NlpPackageExpressionsFlush(package_t package)
{
  OgHeapFlush(package->hexpression_ba);
  package->hexpression_ba = NULL;

  OgHeapFlush(package->hexpression_compile);
  package->hexpression_compile = NULL;

  int expression_used = OgHeapGetCellsUsed(package->hexpression);
  for (int i = 0; i < expression_used; i++)
  {
    struct expression *expression = OgHeapGetCell(package->hexpression, i);
    IFN(expression) DPcErr;

    json_decrefp(&expression->json_solution);
  }

  OgHeapFlush(package->hexpression);
  package->hexpression = NULL;

  DONE;
}

static og_status NlpPackageFlush(package_t package)
{
  if (package == NULL) CONT;

  NlpInputPartWordFlush(package);
  NlpInputPartAliasFlush(package);
  NlpLtracPackageFlush(package);
  NlpRegexFlush(package);

  NlpPackageInterpretationsFlush(package);
  NlpPackageContextsFlush(package);
  NlpPackageExpressionsFlush(package);

  OgHeapFlush(package->halias_ba);
  package->halias_ba = NULL;
  OgHeapFlush(package->halias_compile);
  package->halias_compile = NULL;
  OgHeapFlush(package->halias);
  package->halias = NULL;

  OgHeapFlush(package->hinput_part_ba);
  package->hinput_part_ba = NULL;
  OgHeapFlush(package->hinput_part);
  package->hinput_part = NULL;

  OgHeapFlush(package->hnumber_input_part);
  package->hnumber_input_part = NULL;

  unsigned char * slug = (unsigned char *) package->slug;
  DPcFree(slug);
  package->slug = NULL;

  unsigned char * id = (unsigned char *) package->id;
  DPcFree(id);
  package->id = NULL;

  DPcFree(package);

  DONE;
}

