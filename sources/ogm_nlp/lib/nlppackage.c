/*
 *  Handling packages for Natural Language Processing library
 *  Copyright (c) 2017 Pertimm, by Patrick Constant
 *  Dev : September 2017
 *  Version 1.0
 */
#include "ogm_nlp.h"
static const char* getPackageIdForupdate(og_nlp ctrl_nlp, json_t *json_packages);

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

PUBLIC(og_status) OgNlpPackageAdd(og_nlp ctrl_nlp, struct og_nlp_interpret_input *input, const char *url_package_id)
{
  const char* package_id = getPackageIdForupdate(ctrl_nlp, input->json_input);
  if (package_id != NULL)
  {
    // on verifie que les noms des packages matchent
    if (strcmp(url_package_id, package_id) != 0)
    {
      NlpThrowError(ctrl_nlp, "OgNlpPackageAdd: url package id (%s) is not the same as body package id (%s)",
          url_package_id, package_id);
      DPcErr;
    }

    // si le package existe, on l'efface
    package_t package = NlpPackageGet(ctrl_nlp, package_id);
    if (package != NULL)
    {
      g_hash_table_remove(ctrl_nlp->packages_hash, package_id);
    }

    // on créée le nouveau package, on le rempli et on l'insere
    package_t new_package = NlpPackageCreate(ctrl_nlp, package_id);
    if (new_package == NULL)
    {
      NlpThrowError(ctrl_nlp, "OgNlpPackageAdd: error on package creation");
      DPcErr;
    }

    // on recupere le json du package, pour un update il n'y a qu'un seul package
    int package_array_size = json_array_size(input->json_input);
    if (package_array_size != 1)
    {
      NlpThrowError(ctrl_nlp, "OgNlpPackageAdd: Update package must contain only 1 package");
      DPcErr;
    }

    json_t *json_package_object = json_array_get(input->json_input, 0);
    IFN(json_package_object)
    {
      NlpThrowError(ctrl_nlp, "OgNlpPackageAdd: null json_package");
      DPcErr;
    }

    IFN(json_is_object(json_package_object))
    {
      NlpThrowError(ctrl_nlp, "OgNlpPackageAdd: package bad formatted");
      DPcErr;
    }

    // on recupere l'array des intents du package
    json_t *json_intents = json_object_get(json_package_object, "intents");

    int intents_array_size = json_array_size(json_intents);
    for (int i = 0; i < intents_array_size; i++)
    {
      // on insere chaque intent dans le package_t
      json_t *json_intent = json_array_get(json_intents, i);
      if (json_is_object(json_intent))
      {
        IFE(NlpCompilePackageIntent(new_package, json_intent));
      }
      else
      {
        NlpThrowError(ctrl_nlp, "OgNlpPackageAdd: json_intent at position %d is not an object", i);
        DPcErr;
      }
    }

    // on réinsere le package dans la liste des packages du serveur
    char *allocated_package_id = strdup(package_id);
    og_bool result = g_hash_table_insert(ctrl_nlp->packages_hash, allocated_package_id, new_package);
    if (result == TRUE)
    {
      // "youpy"
      int mb = DOgMsgDestInLog + DOgMsgDestMBox;
      IFE(OgMsg(ctrl_nlp->hmsg, "OgNlpPackageAdd", mb, "Package %s insertion successful", package_id));
    }
    else
    {
      NlpThrowError(ctrl_nlp, "OgNlpPackageAdd: Could not add package");
      DPcErr;
    }
  }
  else
  {
    NlpThrowError(ctrl_nlp, "OgNlpPackageAdd: empty package");
    DPcErr;
  }

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
  g_hash_table_remove(ctrl_nlp->packages_hash, package_id);
  // NlpPackageDestroy(package);

  DONE;
}

static const char* getPackageIdForupdate(og_nlp ctrl_nlp, json_t *json_packages)
{
  int array_size = json_array_size(json_packages);
  if (array_size != 1)
  {
    NlpThrowError(ctrl_nlp, "getPackageIdForupdate: Update package must contain only 1 package");
    return NULL;
  }

  json_t *json_package_object = json_array_get(json_packages, 0);
  IFN(json_package_object)
  {
    NlpThrowError(ctrl_nlp, "getPackageIdForupdate: null json_package");
    return NULL;
  }

  IFN(json_is_object(json_package_object))
  {
    NlpThrowError(ctrl_nlp, "getPackageIdForupdate: package bad formatted");
    return NULL;
  }

  json_t *json_package_id = json_object_get(json_package_object, "id");

  const char* package_id;
  if (json_is_string(json_package_id))
  {
    package_id = json_string_value(json_package_id);
  }
  else
  {
    NlpThrowError(ctrl_nlp, "getPackageIdForupdate: package id is not a string");
    return NULL;
  }

  return package_id;

}
