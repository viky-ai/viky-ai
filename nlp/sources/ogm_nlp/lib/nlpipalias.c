/*
 *  handling packages word automaton
 *  Copyright (c) 2017 Pertimm, by Patrick Constant
 *  Dev : October 2017
 *  Version 1.0
 */
#include "ogm_nlp.h"

static guint package_hash_func(gconstpointer key);
static gboolean package_key_equal_func(gconstpointer key_a, gconstpointer key_b);

og_status NlpInputPartAliasInit(og_nlp_th ctrl_nlp_th, package_t package)
{
  package->interpretation_id_hash = g_hash_table_new(package_hash_func, package_key_equal_func);
  DONE;
}

static guint package_hash_func(gconstpointer key)
{
  og_string interpretation_id = key;
  return g_str_hash(interpretation_id);
}

static gboolean package_key_equal_func(gconstpointer key_a, gconstpointer key_b)
{
  og_string interpretation_id_a = key_a;
  og_string interpretation_id_b = key_b;
  return g_str_equal(interpretation_id_a, interpretation_id_b);
}

og_status NlpInputPartAliasAdd(og_nlp_th ctrl_nlp_th, package_t package, og_string interpretation_id,
    size_t Iinput_part)
{
  gpointer key = (unsigned char *) interpretation_id;
  g_hash_table_insert(package->interpretation_id_hash, key, GINT_TO_POINTER(Iinput_part));
  DONE;
}

static gint str_compar(gconstpointer a, gconstpointer b)
{
  return strcmp((const char*) a, (const char*) b);
}

og_status NlpInputPartAliasLog(og_nlp_th ctrl_nlp_th, package_t package)
{
  OgMsg(ctrl_nlp_th->hmsg, "", DOgMsgDestInLog, "Interpretation ids for package '%s' '%s':", package->id, package->slug);

  GList *key_list = g_hash_table_get_keys(package->interpretation_id_hash);
  GList *sorted_key_list = g_list_sort(key_list, str_compar);

  for (GList *iter = sorted_key_list; iter; iter = iter->next)
  {
    og_string interpretation_id = iter->data;
    size_t Iinput_part = (size_t) g_hash_table_lookup(package->interpretation_id_hash, interpretation_id);
    OgMsg(ctrl_nlp_th->hmsg, "", DOgMsgDestInLog, "  %s : %d", interpretation_id, Iinput_part);
  }

  g_list_free(sorted_key_list);

  DONE;
}

