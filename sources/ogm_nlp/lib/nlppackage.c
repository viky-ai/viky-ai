/*
 *  Handling packages for Natural Language Processing library
 *  Copyright (c) 2017 Pertimm, by Patrick Constant
 *  Dev : September 2017
 *  Version 1.0
 */
#include "ogm_nlp.h"

static og_status NlpPackageRemove(og_nlp ctrl_nlp, const char *package_id);

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

void NlpPackageDestroy(gpointer data)
{
  package_t package = data;
  OgHeapFlush(package->hsentence);
  OgHeapFlush(package->hintent);
  OgHeapFlush(package->hba);
  DPcFree(package);
}

og_status NlpPackageAddOrReplace(og_nlp ctrl_nlp, package_t package)
{
  og_string package_id = OgHeapGetCell(package->hba, package->id_start);
  char *allocated_package_id = strdup(package_id);

  // TODO: synchro ecriture
  g_hash_table_replace(ctrl_nlp->packages_hash, allocated_package_id, package);

  DONE;
}

package_t NlpPackageGet(og_nlp ctrl_nlp, og_string package_id)
{

  // TODO: synchro lecture
  package_t package = g_hash_table_lookup(ctrl_nlp->packages_hash, package_id);

  return (package);
}

PUBLIC(og_status) OgNlpPackageAdd(og_nlp ctrl_nlp, struct og_nlp_compile_input *input)
{
  if (!json_is_object(input->json_input))
  {
    NlpThrowError(ctrl_nlp, "OgNlpPackageAdd: package must an object");
    DPcErr;
  }

  json_t *json_package_name = json_object_get(input->json_input, "id");
  if (json_package_name == NULL)
  {
    NlpThrowError(ctrl_nlp, "OgNlpPackageAdd: package name is not present");
    DPcErr;
  }

  if (!json_is_string(json_package_name))
  {
    NlpThrowError(ctrl_nlp, "OgNlpPackageAdd: package name must be a string");
    DPcErr;

  }
  og_string package_name = json_string_value(json_package_name);
  if (!package_name[0])
  {
    NlpThrowError(ctrl_nlp, "OgNlpPackageAdd: package name must not empty");
    DPcErr;
  }

  // on verifie que les noms des packages matchent
  if (strcmp(input->package_name, package_name) != 0)
  {
    NlpThrowError(ctrl_nlp, "OgNlpPackageAdd: url package id (%s) is not the same as body package id (%s)",
        input->package_name, package_name);
    DPcErr;
  }

  IFE(NlpCompilePackage(ctrl_nlp, input, input->json_input));

  DONE;
}

PUBLIC(og_status) OgNlpPackageDelete(og_nlp ctrl_nlp, const char *package_id)
{
  // si le package existe, on l'efface
  package_t package = NlpPackageGet(ctrl_nlp, package_id);
  if (package == NULL)
  {
    NlpThrowError(ctrl_nlp, "OgNlpPackageDelete: unknown package");
    DPcErr;
  }

  IFE(NlpPackageRemove(ctrl_nlp, package_id));

  DONE;
}

static og_status NlpPackageRemove(og_nlp ctrl_nlp, const char *package_id)
{
  // g_hash_table_remove : will call NlpPackageDestroy(package) on value;

  // TODO: synchro ecriture
  g_hash_table_remove(ctrl_nlp->packages_hash, package_id);

  DONE;
}

