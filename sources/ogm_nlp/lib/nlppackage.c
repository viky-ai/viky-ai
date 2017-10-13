/*
 *  Handling packages for Natural Language Processing library
 *  Copyright (c) 2017 Pertimm, by Patrick Constant
 *  Dev : September 2017
 *  Version 1.0
 */
#include "ogm_nlp.h"

static og_status NlpPackageRemove(og_nlpi ctrl_nlpi, const char *package_id);

package_t NlpPackageCreate(og_nlpi ctrl_nlpi, const char *string_id)
{
  package_t package = (package_t) malloc(sizeof(struct package));
  IFn(package)
  {
    NlpiThrowError(ctrl_nlpi, "NlpPackageCreate: malloc error on package");
    return NULL;
  }
  memset(package, 0, sizeof(struct package));

  void *hmsg = ctrl_nlpi->ctrl_nlp->hmsg;
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

og_status NlpPackageAddOrReplace(og_nlpi ctrl_nlpi, package_t package)
{
  og_string package_id = OgHeapGetCell(package->hba, package->id_start);
  char *allocated_package_id = strdup(package_id);

  // TODO: synchro ecriture
  g_hash_table_replace(ctrl_nlpi->ctrl_nlp->packages_hash, allocated_package_id, package);

  DONE;
}

package_t NlpPackageGet(og_nlpi ctrl_nlpi, og_string package_id)
{

  // TODO: synchro lecture
  package_t package = g_hash_table_lookup(ctrl_nlpi->ctrl_nlp->packages_hash, package_id);

  return (package);
}

PUBLIC(og_status) OgNlpPackageAdd(og_nlpi ctrl_nlpi, struct og_nlp_compile_input *input)
{
  if (!json_is_object(input->json_input))
  {
    NlpiThrowError(ctrl_nlpi, "OgNlpPackageAdd: package must an object");
    DPcErr;
  }

  json_t *json_package_name = json_object_get(input->json_input, "id");
  if (json_package_name == NULL)
  {
    NlpiThrowError(ctrl_nlpi, "OgNlpPackageAdd: package name is not present");
    DPcErr;
  }

  if (!json_is_string(json_package_name))
  {
    NlpiThrowError(ctrl_nlpi, "OgNlpPackageAdd: package name must be a string");
    DPcErr;

  }
  og_string package_name = json_string_value(json_package_name);
  if (!package_name[0])
  {
    NlpiThrowError(ctrl_nlpi, "OgNlpPackageAdd: package name must not empty");
    DPcErr;
  }

  // on verifie que les noms des packages matchent
  if (strcmp(input->package_name, package_name) != 0)
  {
    NlpiThrowError(ctrl_nlpi, "OgNlpPackageAdd: url package id (%s) is not the same as body package id (%s)",
        input->package_name, package_name);
    DPcErr;
  }

  IFE(NlpCompilePackage(ctrl_nlpi, input, input->json_input));

  DONE;
}

PUBLIC(og_status) OgNlpPackageDelete(og_nlpi ctrl_nlpi, const char *package_id)
{
  // si le package existe, on l'efface
  package_t package = NlpPackageGet(ctrl_nlpi, package_id);
  if (package == NULL)
  {
    NlpiThrowError(ctrl_nlpi, "OgNlpPackageDelete: unknown package");
    DPcErr;
  }

  IFE(NlpPackageRemove(ctrl_nlpi, package_id));

  DONE;
}

static og_status NlpPackageRemove(og_nlpi ctrl_nlpi, const char *package_id)
{
  // g_hash_table_remove : will call NlpPackageDestroy(package) on value;

  // TODO: synchro ecriture
  g_hash_table_remove(ctrl_nlpi->ctrl_nlp->packages_hash, package_id);

  DONE;
}

