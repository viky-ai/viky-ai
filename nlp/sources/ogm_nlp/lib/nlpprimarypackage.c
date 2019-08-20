/*
 *  Handling a hash table of primary packages
 *  Copyright (c) 2010 Pertimm, by Patrick Constant
 *  Dev : August 2019
 *  Version 1.0
 */
#include <loggen.h>

#include "ogm_nlp.h"

og_status NlpPrimaryPackageInit(og_nlp_th ctrl_nlp_th)
{
  ctrl_nlp_th->primary_package_hash = g_hash_table_new(g_direct_hash, g_direct_equal);
  DONE;
}

og_status NlpPrimaryPackageFlush(og_nlp_th ctrl_nlp_th)
{
  g_hash_table_destroy(ctrl_nlp_th->primary_package_hash);
  DONE;
}

og_status NlpPrimaryPackageReset(og_nlp_th ctrl_nlp_th)
{
  ctrl_nlp_th->enable_list = FALSE;
  g_hash_table_remove_all(ctrl_nlp_th->primary_package_hash);
  ctrl_nlp_th->nb_primary_packages = 0;
  DONE;
}

og_status NlpAddPrimaryPackage(og_nlp_th ctrl_nlp_th, package_t primary_package)
{
  g_hash_table_insert(ctrl_nlp_th->primary_package_hash, GINT_TO_POINTER(primary_package), GINT_TO_POINTER(1));
  DONE;
}

og_bool NlpIsPrimaryPackage(og_nlp_th ctrl_nlp_th, package_t primary_package)
{
  gpointer exists = g_hash_table_lookup(ctrl_nlp_th->primary_package_hash, primary_package);
  IFN(exists) return FALSE;
  return TRUE;
}


