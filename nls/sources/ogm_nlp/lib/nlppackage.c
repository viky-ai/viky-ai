/*
 *  Handling packages for Natural Language Processing library
 *  Copyright (c) 2017 Pertimm, by Patrick Constant
 *  Dev : September 2017
 *  Version 1.0
 */
#include "ogm_nlp.h"

static og_status NlpPackageRemove(og_nlp_th ctrl_nlp_th, const char *package_id);

package_t NlpPackageCreate(og_nlp_th ctrl_nlp_th, const char *string_id)
{
  package_t package = (package_t) malloc(sizeof(struct package));
  IFn(package)
  {
    NlpThrowErrorTh(ctrl_nlp_th, "NlpPackageCreate: malloc error on package");
    return NULL;
  }
  memset(package, 0, sizeof(struct package));

  void *hmsg = ctrl_nlp_th->ctrl_nlp->hmsg;
  IFn(package->hba=OgHeapInit(hmsg, "package_ba", sizeof(unsigned char), DOgNlpPackageBaNumber)) return NULL;
  package->id_start = OgHeapGetCellsUsed(package->hba);
  package->id_length = strlen(string_id);
  IF(OgHeapAppend(package->hba,package->id_length+1, string_id)) return NULL;

  IFn(package->hintent = OgHeapInit(hmsg, "package_intent", sizeof(struct intent), DOgNlpPackageIntentNumber)) return NULL;
  IFn(package->hsentence = OgHeapInit(hmsg, "package_phrase", sizeof(struct sentence), DOgNlpPackagePhraseNumber)) return NULL;

  return (package);
}

void NlpPackageDestroy(gpointer data)
{
  package_t package = data;
  OgHeapFlush(package->hsentence);
  OgHeapFlush(package->hintent);
  OgHeapFlush(package->hba);
  DPcFree(package);
}

static og_status NlpPackageAddOrReplaceNosync(og_nlp_th ctrl_nlp_th, package_t package)
{
  // Timeout trigger for synchro test
  IFE(OgNlpSynchroTestSleepIfTimeoutNeeded(ctrl_nlp_th, nlp_timeout_in_NlpPackageAddOrReplace));

  og_string package_id = OgHeapGetCell(package->hba, package->id_start);
  char *allocated_package_id = strdup(package_id);

  g_hash_table_replace(ctrl_nlp_th->ctrl_nlp->packages_hash, allocated_package_id, package);

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

  return package;
}

static og_status NlpPackageRemoveNosync(og_nlp_th ctrl_nlp_th, const char *package_id)
{
  // g_hash_table_remove : will call NlpPackageDestroy(package) on value;

  g_hash_table_remove(ctrl_nlp_th->ctrl_nlp->packages_hash, package_id);

  DONE;
}

static og_status NlpPackageRemove(og_nlp_th ctrl_nlp_th, const char *package_id)
{
  IF(OgNlpSynchroWriteLock(ctrl_nlp_th, ctrl_nlp_th->ctrl_nlp->rw_lock_packages_hash))
  {
    NlpThrowErrorTh(ctrl_nlp_th, "NlpPackageRemoveNosync: error on OgNlpSynchroWriteLock");
    DPcErr;
  }

  og_status status = NlpPackageRemoveNosync(ctrl_nlp_th, package_id);

  IF(OgNlpSynchroWriteUnLock(ctrl_nlp_th, ctrl_nlp_th->ctrl_nlp->rw_lock_packages_hash))
  {
    NlpThrowErrorTh(ctrl_nlp_th, "NlpPackageRemoveNosync: error on OgNlpSynchroWriteUnLock");
    DPcErr;
  }

  return status;
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
  // si le package existe, on l'efface
  package_t package = NlpPackageGet(ctrl_nlp_th, package_id);
  if (package == NULL)
  {
    NlpThrowErrorTh(ctrl_nlp_th, "OgNlpPackageDelete: unknown package");
    DPcErr;
  }

  IFE(NlpPackageRemove(ctrl_nlp_th, package_id));

  DONE;
}

