/*
 *  Handling packages for Natural Language Processing library
 *  Copyright (c) 2017 Pertimm, by Patrick Constant
 *  Dev : September 2017
 *  Version 1.0
 */
#include "ogm_nlp.h"

static og_status NlpPackageFlush(package_t package);

package_t NlpPackageCreate(og_nlp_th ctrl_nlp_th, const char *string_id, const char *string_slug)
{
  package_t package = (package_t) malloc(sizeof(struct package));
  IFn(package)
  {
    NlpThrowErrorTh(ctrl_nlp_th, "NlpPackageCreate: malloc error on package");
    return NULL;
  }
  memset(package, 0, sizeof(struct package));

  package->ref_counter = 0;

  void *hmsg = ctrl_nlp_th->ctrl_nlp->hmsg;
  IFn(package->hba=OgHeapInit(hmsg, "package_ba", sizeof(unsigned char), DOgNlpPackageBaNumber)) return NULL;
  package->id_start = OgHeapGetCellsUsed(package->hba);
  package->id_length = strlen(string_id);
  IF(OgHeapAppend(package->hba, package->id_length + 1, string_id)) return NULL;

  package->slug_start = OgHeapGetCellsUsed(package->hba);
  package->slug_length = strlen(string_slug);
  IF(OgHeapAppend(package->hba, package->slug_length + 1, string_slug)) return NULL;

  IFn(package->hinterpretation = OgHeapInit(hmsg, "package_interpretation", sizeof(struct interpretation), DOgNlpPackageInterpretationNumber)) return NULL;
  IFn(package->hexpression = OgHeapInit(hmsg, "package_expression", sizeof(struct expression), DOgNlpPackageExpressionNumber)) return NULL;
  IFn(package->halias = OgHeapInit(hmsg, "package_alias", sizeof(struct alias), DOgNlpPackageAliasNumber)) return NULL;

  return (package);
}

static og_status NlpPackageAddOrReplaceNosync(og_nlp_th ctrl_nlp_th, package_t package)
{
  // Timeout trigger for synchro test
  IFE(OgNlpSynchroTestSleepIfTimeoutNeeded(ctrl_nlp_th, nlp_timeout_in_NlpPackageAddOrReplace));

  og_string package_id = OgHeapGetCell(package->hba, package->id_start);
  char *allocated_package_id = strdup(package_id);

  og_nlp ctrl_nlp = ctrl_nlp_th->ctrl_nlp;

  // update package and remove preview package : see NlpPackageDestroyIfNotUsed
  g_hash_table_replace(ctrl_nlp->packages_hash, allocated_package_id, package);

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

  IF(OgNlpSynchroReadUnLock(ctrl_nlp_th, ctrl_nlp_th->ctrl_nlp->rw_lock_packages_hash))
  {
    NlpThrowErrorTh(ctrl_nlp_th, "NlpPackageGet: error on OgNlpSynchroReadUnLock");
    return NULL;
  }

  if (package != NULL)
  {
    // flag package as used
    g_atomic_int_inc(&package->ref_counter);

    // keep all used package to free it in fallback reset
    g_queue_push_head(ctrl_nlp_th->package_in_used, package);
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
  NlpLogInfo(ctrl_nlp_th, DOgNlpTracePackage, "NlpPackageMarkAllInUsedAsUnused : marking %d packages",
      ctrl_nlp_th->package_in_used);

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

static og_status NlpPackageFlush(package_t package)
{
  if (package == NULL) CONT;

  OgHeapFlush(package->hexpression);
  OgHeapFlush(package->hinterpretation);
  OgHeapFlush(package->halias);
  OgHeapFlush(package->hba);
  DPcFree(package);

  DONE;
}

