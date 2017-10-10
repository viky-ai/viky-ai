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

  ctrl_nlp->packages_hash = g_hash_table_new_full(package_hash_func, package_key_equal_func, g_free, NlpPackageDestroy);

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

PUBLIC(int) OgNlpFlush(og_nlp handle)
{
  struct og_ctrl_nlp *ctrl_nlp = handle;

  OgMsgFlush(ctrl_nlp->hmsg);

  g_hash_table_destroy(ctrl_nlp->packages_hash);

  DPcFree(ctrl_nlp);
  DONE;
}

