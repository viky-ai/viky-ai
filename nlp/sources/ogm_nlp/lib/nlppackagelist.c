/*
 * nlppackagelist.c
 *
 *  Created on: 14 nov. 2017
 *      Author: sebastien
 */
#include "ogm_nlp.h"

static og_status NlpPackageListNoSync(og_nlp_th ctrl_nlp_th, struct og_nlp_dump_input *input,
    struct og_nlp_package_list *output);
static gint str_compar(gconstpointer a, gconstpointer b)
{
  return strcmp((const char*) a, (const char*) b);
}

PUBLIC(int) OgNlpPackageList(og_nlp_th ctrl_nlp_th, struct og_nlp_dump_input *input, struct og_nlp_package_list *output)
{
  memset(output, 0, sizeof(struct og_nlp_package_list));
  ctrl_nlp_th->json_answer = json_array();

  IF(OgNlpSynchroReadLock(ctrl_nlp_th, ctrl_nlp_th->ctrl_nlp->rw_lock_packages_hash))
  {
    NlpThrowErrorTh(ctrl_nlp_th, "OgNlpPackageList: error on OgNlpSynchroReadLock");
    DPcErr;
  }

  og_status dump_status = NlpPackageListNoSync(ctrl_nlp_th, input, output);

  IF(OgNlpSynchroReadUnLock(ctrl_nlp_th, ctrl_nlp_th->ctrl_nlp->rw_lock_packages_hash))
  {
    NlpThrowErrorTh(ctrl_nlp_th, "OgNlpPackageList: error on OgNlpSynchroReadUnLock");
    DPcErr;
  }

  output->package_names = ctrl_nlp_th->json_answer;

  return dump_status;

}

static og_status NlpPackageListNoSyncFreeSafe(og_nlp_th ctrl_nlp_th, GList *sorted_key_kist)
{
  json_t *json_packages = ctrl_nlp_th->json_answer;

  for (GList *iter = sorted_key_kist; iter; iter = iter->next)
  {
    og_string package_id = iter->data;

    json_t *json_package_id = json_string(package_id);
    IF(json_array_append_new(json_packages, json_package_id))
    {
      NlpThrowErrorTh(ctrl_nlp_th, "NlpDumpNoSyncFreeSafe : Error while dumping package_id");
      DPcErr;
    }
  }

  DONE;
}

static og_status NlpPackageListNoSync(og_nlp_th ctrl_nlp_th, struct og_nlp_dump_input *input,
    struct og_nlp_package_list *output)
{

  GList *key_list = g_hash_table_get_keys(ctrl_nlp_th->ctrl_nlp->packages_hash);

  GList *sorted_key_list = g_list_sort(key_list, str_compar);

  og_status dump_status = NlpPackageListNoSyncFreeSafe(ctrl_nlp_th, sorted_key_list);

  g_list_free(sorted_key_list);

  return dump_status;
}
