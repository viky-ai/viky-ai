/*
 *  Handling packages for Natural Language Processing library
 *  Copyright (c) 2017 Pertimm, by Patrick Constant
 *  Dev : September 2017
 *  Version 1.0
 */
#include "ogm_nlp.h"

package_t NlpPackageCreate(og_nlp ctrl_nlp, const char *string_id)
{
  package_t package = (package_t) malloc(sizeof(struct package));
  IFn(package)
  {
    NlpThrowError(ctrl_nlp, "NlpPackageCreate: malloc error on package");
    return (0);
  }
  memset(package, 0, sizeof(struct package));
  package->ctrl_nlp = ctrl_nlp;

  IFn(package->hba=OgHeapInit(ctrl_nlp->hmsg,"package_ba",sizeof(unsigned char),DOgNlpPackageBaNumber)) return (0);
  package->id_start = OgHeapGetCellsUsed(package->hba);
  package->id_length = strlen(string_id);
  IF(OgHeapAppend(package->hba,package->id_length+1,string_id)) return (0);

  IFn(package->hintent=OgHeapInit(ctrl_nlp->hmsg,"package_intent",sizeof(struct intent),DOgNlpPackageIntentNumber)) return (0);
  IFn(package->hsentence=OgHeapInit(ctrl_nlp->hmsg,"package_phrase",sizeof(struct sentence),DOgNlpPackagePhraseNumber)) return (0);

  return (package);
}

og_status NlpPackageDestroy(package_t package)
{
  OgHeapFlush(package->hsentence);
  OgHeapFlush(package->hintent);
  OgHeapFlush(package->hba);
  DPcFree(package);
  DONE;
}

og_status NlpPackageAdd(og_nlp ctrl_nlp, package_t package)
{
  og_string package_id = OgHeapGetCell(package->hba, package->id_start);
  char *allocated_package_id = strdup(package_id);

  // TODO: synchro ecriture
  g_hash_table_insert(ctrl_nlp->packages_hash, allocated_package_id, package);

  DONE;
}

package_t NlpPackageGet(og_nlp ctrl_nlp, og_string package_id)
{

  // TODO: synchro lecture
  package_t package = g_hash_table_lookup(ctrl_nlp->packages_hash, package_id);

  return (package);
}
