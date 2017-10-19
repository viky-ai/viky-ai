/*
 *  Initialization for Natural Language functions
 *  Copyright (c) 2017 by Patrick Constant
 *  Dev : August 2017
 *  Version 1.0
 */
#include "ogm_nlp.h"
#include <stddef.h>

static guint package_hash_func(gconstpointer key);
static gboolean package_key_equal_func(gconstpointer  key_a, gconstpointer  key_b);



PUBLIC(og_nlp) OgNlpInit(struct og_nlp_param *param)
{
  char erreur[DOgErrorSize];
  struct og_ctrl_nlp *ctrl_nlp = (struct og_ctrl_nlp *) malloc(sizeof(struct og_ctrl_nlp));
  IFn(ctrl_nlp)
  {
    sprintf(erreur, "OgNlpInit: malloc error on ctrl_nlp");
    OgErr(param->herr, erreur);
    return (0);
  }

  memset(ctrl_nlp, 0, sizeof(struct og_ctrl_nlp));
  ctrl_nlp->herr = param->herr;
  ctrl_nlp->hmutex = param->hmutex;
  memcpy(ctrl_nlp->loginfo, &param->loginfo, sizeof(struct og_loginfo));

  struct og_msg_param msg_param[1];
  memset(msg_param, 0, sizeof(struct og_msg_param));
  msg_param->herr = ctrl_nlp->herr;
  msg_param->hmutex = ctrl_nlp->hmutex;
  msg_param->loginfo.trace = DOgMsgTraceMinimal + DOgMsgTraceMemory;
  msg_param->loginfo.where = ctrl_nlp->loginfo->where;
  msg_param->module_name = "nlp";
  IFn(ctrl_nlp->hmsg=OgMsgInit(msg_param)) return (0);
  IF(OgMsgTuneInherit(ctrl_nlp->hmsg,param->hmsg)) return (0);

  struct og_sysi_param sysi_param[1];
  memset(sysi_param, 0, sizeof(struct og_sysi_param));
  sysi_param->herr = ctrl_nlp->herr;
  sysi_param->hmsg = ctrl_nlp->hmsg;
  // TODO remove disable_log_rw_lock_stat_on_flush
  sysi_param->disable_log_rw_lock_stat_on_flush = FALSE;
  snprintf(sysi_param->lock_name, DPcPathSize, "nlp_rw_lock_packages_hash");

  ctrl_nlp->rw_lock_packages_hash = OgSysiInit(sysi_param);

  ctrl_nlp->packages_hash = g_hash_table_new_full(package_hash_func, package_key_equal_func, g_free, NULL);
  g_queue_init(ctrl_nlp->deleted_packages);

  return ctrl_nlp;
}

static guint package_hash_func(gconstpointer key)
{
  og_string package_id = key;
  return g_str_hash(package_id);
}

static gboolean package_key_equal_func(gconstpointer  key_a, gconstpointer  key_b)
{
  og_string package_id_a = key_a;
  og_string package_id_b = key_b;
  return g_str_equal(package_id_a, package_id_b);
}

PUBLIC(int) OgNlpFlush(og_nlp ctrl_nlp)
{

  OgMsg(ctrl_nlp->hmsg, "", DOgMsgDestInLog, "OgNlpFlush in progress");

  // flush package mark as deleted
  NlpFlushPackageMarkedAsDeletedNosync(ctrl_nlp);

  g_hash_table_destroy(ctrl_nlp->packages_hash);
  ctrl_nlp->packages_hash = NULL;

  OgSysiFlush(ctrl_nlp->rw_lock_packages_hash);
  ctrl_nlp->rw_lock_packages_hash = NULL;

  OgMsgFlush(ctrl_nlp->hmsg);

  DPcFree(ctrl_nlp);

  DONE;
}

