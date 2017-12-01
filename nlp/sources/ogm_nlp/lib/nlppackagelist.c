/*
 * nlppackagelist.c
 *
 *  Created on: 14 nov. 2017
 *      Author: sebastien
 */
#include "ogm_nlp.h"

static gint str_compar(gconstpointer a, gconstpointer b)
{
  return strcmp((const char*) a, (const char*) b);
}

static og_status NlpPackageListJsonbuildCallback(og_nlp_th ctrl_nlp_th, og_string package_id)
{
  json_t *json_packages = ctrl_nlp_th->json_answer;

  json_t *json_package_id = json_string(package_id);
  IF(json_array_append_new(json_packages, json_package_id))
  {
    if (json_package_id != NULL)
    {
      json_decrefp(&json_package_id);
    }
    NlpThrowErrorTh(ctrl_nlp_th, "NlpPackageListJsonbuildCallback : Error while calling list "
        "callback on package_id: '%s'", package_id);
    DPcErr;
  }

  DONE;
}

PUBLIC(int) OgNlpPackageList(og_nlp_th ctrl_nlp_th, struct og_nlp_dump_input *input, struct og_nlp_package_list *output)
{
  memset(output, 0, sizeof(struct og_nlp_package_list));
  ctrl_nlp_th->json_answer = json_array();

  og_status status = NlpPackageListInternal(ctrl_nlp_th, NlpPackageListJsonbuildCallback);
  IF(status)
  {
    NlpThrowErrorTh(ctrl_nlp_th,
        "OgNlpPackageList : NlpPackageListInternal failed with NlpPackageListJsonbuildCallback");
    DPcErr;
  }

  output->package_names = ctrl_nlp_th->json_answer;

  DONE;
}

static og_status NlpPackageListNoSyncFreeSafe(og_nlp_th ctrl_nlp_th, GList *sorted_key_kist,
    nlp_package_list_callback func)
{
  for (GList *iter = sorted_key_kist; iter; iter = iter->next)
  {
    og_string package_id = iter->data;

    og_status status = func(ctrl_nlp_th, package_id);
    IF(status)
    {
      NlpThrowErrorTh(ctrl_nlp_th, "NlpPackageListNoSyncFreeSafe : Error while list package_id : '%s'", package_id);
      DPcErr;
    }
  }

  DONE;
}

static og_status NlpPackageListNoSync(og_nlp_th ctrl_nlp_th, nlp_package_list_callback func)
{

  GList *key_list = g_hash_table_get_keys(ctrl_nlp_th->ctrl_nlp->packages_hash);

  GList *sorted_key_list = g_list_sort(key_list, str_compar);

  og_status status = NlpPackageListNoSyncFreeSafe(ctrl_nlp_th, sorted_key_list, func);

  g_list_free(sorted_key_list);

  return status;
}

og_status NlpPackageListInternal(og_nlp_th ctrl_nlp_th, nlp_package_list_callback func)
{

  IF(OgNlpSynchroReadLock(ctrl_nlp_th, ctrl_nlp_th->ctrl_nlp->rw_lock_packages_hash))
  {
    NlpThrowErrorTh(ctrl_nlp_th, "NlpPackageListInternal: error on OgNlpSynchroReadLock");
    DPcErr;
  }

  og_status status = NlpPackageListNoSync(ctrl_nlp_th, func);

  IF(OgNlpSynchroReadUnLock(ctrl_nlp_th, ctrl_nlp_th->ctrl_nlp->rw_lock_packages_hash))
  {
    NlpThrowErrorTh(ctrl_nlp_th, "NlpPackageListInternal: error on OgNlpSynchroReadUnLock");
    DPcErr;
  }

  return status;
}
