/*
 *  Initialization for ogm_stm functions
 *  Copyright (c) 2007 Pertimm by Patrick Constant, Lo�s Rigouste
 *  Dev : September 2007
 *  Version 1.0
 */
#include "ogm_stm.h"

static gboolean equivalent_letter_key_equal(gconstpointer key1, gconstpointer key2);
static guint equivalent_letter_key_hash(gconstpointer key);

PUBLIC(void *) OgStmInit(struct og_stm_param *param)
{
  char erreur[DOgErrorSize];
  struct og_ctrl_stm *ctrl_stm;
  struct og_msg_param cmsg_param,*msg_param=&cmsg_param;

  IFn(ctrl_stm=(struct og_ctrl_stm *)malloc(sizeof(struct og_ctrl_stm)))
  {
    sprintf(erreur,"OgStmInit: malloc error on ctrl_stm");
    OgErr(param->herr,erreur); return(0);
  }
  memset(ctrl_stm,0,sizeof(struct og_ctrl_stm));

  ctrl_stm->herr = param->herr;
  ctrl_stm->hmutex = param->hmutex;
  ctrl_stm->cloginfo = param->loginfo;
  ctrl_stm->loginfo = &ctrl_stm->cloginfo;

  memset(msg_param,0,sizeof(struct og_msg_param));
  msg_param->herr=param->herr;
  msg_param->hmutex=param->hmutex;
  msg_param->loginfo.trace = DOgMsgTraceMinimal+DOgMsgTraceMemory;
  msg_param->loginfo.where = param->loginfo.where;
  msg_param->module_name="ogm_stm";

  IFn(ctrl_stm->hmsg=OgMsgInit(msg_param)) return(0);
  IF(OgMsgTuneInherit(ctrl_stm->hmsg,param->hmsg)) return(0);

  strcpy(ctrl_stm->WorkingDirectory, param->WorkingDirectory);

  og_char_buffer lt_heap_name[DOgAutNameSize];
  snprintf(lt_heap_name, DOgAutNameSize, "stm_hequivalent_letters_costs");
  IFn(ctrl_stm->hequivalent_letters_costs = OgHeapSliceInit(ctrl_stm->hmsg, lt_heap_name, sizeof(double), 2, 8)) return NULL;

  snprintf(lt_heap_name, DOgAutNameSize, "stm_hequivalent_letters");
  IFn(ctrl_stm->hequivalent_letters = OgHeapSliceInit(ctrl_stm->hmsg, lt_heap_name, sizeof(struct equivalent_letter), 2, 8)) return NULL;

  ctrl_stm->equivalent_letters_hash = g_hash_table_new(equivalent_letter_key_hash, equivalent_letter_key_equal);

  for(int i=0; i<DOgStmSpaceCostLength; i++)
  {
    ctrl_stm->space_deletion_cost[i] = -1;
    ctrl_stm->space_insertion_cost[i] = -1;
  }

  return ctrl_stm;
}

PUBLIC(og_status) OgStmInitDefaultCosts(void *hstm, struct og_stm_levenshtein_input_param *lev_param)
{
  memset(lev_param, 0, sizeof(struct og_stm_levenshtein_input_param));
  lev_param->insertion_cost = DOgStmDefaultInsertionCost;
  lev_param->deletion_cost = DOgStmDefaultDeletionCost;
  lev_param->substitution_cost = DOgStmDefaultSubstitutionCost;
  lev_param->swap_cost = DOgStmDefaultSwapCost;
  lev_param->accents_substitution_cost = DOgStmDefaultAccentSubstitutionCost;
  lev_param->same_letter_insertion_cost = DOgStmDefaultSameLetterInsertionCost;
  lev_param->same_letter_deletion_cost = DOgStmDefaultSameLetterDeletionCost;
  lev_param->case_cost = DOgStmDefaultCaseCost;
  lev_param->punctuation_cost = DOgStmDefaultPunctuationCost;
  DONE;
}

PUBLIC(og_status) StmInitDefaultSpaceCost(void *hstm, og_bool is_insertion)
{
  struct og_ctrl_stm *ctrl_stm = (struct og_ctrl_stm *) hstm;
  for (int i = 0; i < DOgStmSpaceCostLength; i++)
  {
    if (!is_insertion)
    {
      ctrl_stm->space_deletion_cost[i] = DOgStmDefaultSpaceCost;
    }
    else
    {
      ctrl_stm->space_insertion_cost[i] = DOgStmDefaultSpaceCost;
    }
  }

  DONE;
}

PUBLIC(og_status) OgStmAddSpaceCost(void *hstm, int occurence, double cost, og_bool is_insertion)
{
  struct og_ctrl_stm *ctrl_stm = (struct og_ctrl_stm *) hstm;

  if (occurence > DOgStmSpaceCostLength)
  {
    char erreur[DOgErrorSize];
    sprintf(erreur, "OgStmAddSpaceCost: occurence of a space cost cannot be greater than %d",
    DOgStmSpaceCostLength);
    OgErr(ctrl_stm->herr, erreur);
    DPcErr;
  }
  if (occurence < 0)
  {
    char erreur[DOgErrorSize];
    sprintf(erreur, "OgStmAddSpaceCost: occurence cannot be negative");
    OgErr(ctrl_stm->herr, erreur);
    DPcErr;
  }

  if (is_insertion)
  {
    ctrl_stm->space_insertion_cost[occurence] = cost;
  }
  else
  {
    ctrl_stm->space_deletion_cost[occurence] = cost;
  }

  DONE;

}

PUBLIC(og_status) OgStmGetSpaceCost(void *hstm, int occurence, og_bool is_insertion, double *space_cost)
{
  struct og_ctrl_stm *ctrl_stm = (struct og_ctrl_stm *) hstm;
  return StmGetSpaceCost(ctrl_stm, occurence, is_insertion, space_cost);
}

og_status StmGetSpaceCost(struct og_ctrl_stm *ctrl_stm, int occurence, og_bool is_insertion, double *space_cost)
{
  if (occurence > DOgStmSpaceCostLength)
  {
    char erreur[DOgErrorSize];
    sprintf(erreur, "OgStmGetSpaceCost: occurence of a space cost cannot be greater than %d",
    DOgStmSpaceCostLength);
    OgErr(ctrl_stm->herr, erreur);
    DPcErr;
  }
  if (occurence < 0)
  {
    char erreur[DOgErrorSize];
    sprintf(erreur, "OgStmGetSpaceCost: occurence cannot be negative");
    OgErr(ctrl_stm->herr, erreur);
    DPcErr;
  }
  double *space_cost_tab = NULL;
  if (is_insertion)
  {
    space_cost_tab = ctrl_stm->space_insertion_cost;
  }
  else
  {
    space_cost_tab = ctrl_stm->space_deletion_cost;
  }

  double cost = space_cost_tab[occurence];

  if (cost == -1)
  {
    for (int i = (occurence - 1); i >= 0; i--)
    {
      if (space_cost_tab[i] != (-1))
      {
        cost = space_cost_tab[i];
      }
    }

    if (cost == -1)
    {
      cost = DOgStmDefaultSpaceCost;
    }
  }

  *space_cost = cost;
  DONE;

}

// If element already exist, we insert it only if the cost is lower
PUBLIC(og_status) OgStmInitCreateEquivalentLetterEntry(void *hstm, int letter1, int letter2, double cost)
{
  struct og_ctrl_stm *ctrl_stm = (struct og_ctrl_stm *) hstm;

  // Init and store the cost of the equivalent letter entry
  double *pcost = (double *) OgHeapNewCell(ctrl_stm->hequivalent_letters_costs, NULL);
  *pcost = cost;

  // Init and store the equivalent letter entry
  struct equivalent_letter *equivalent_letter = (struct equivalent_letter *) OgHeapNewCell(
      ctrl_stm->hequivalent_letters, NULL);

  equivalent_letter->letter1 = letter1;
  equivalent_letter->letter2 = letter2;

  // Store in the map the equivalent letter entry as a key and the cost as a value
  // check if element is already present and add it only if cost is lower
  double *pcost_already_present = (double *) g_hash_table_lookup(ctrl_stm->equivalent_letters_hash, equivalent_letter);
  if (pcost_already_present != NULL)
  {
    if (*pcost_already_present > *pcost)
    {
      g_hash_table_insert(ctrl_stm->equivalent_letters_hash, equivalent_letter, pcost);
    }
  }
  else
  {
    g_hash_table_insert(ctrl_stm->equivalent_letters_hash, equivalent_letter, pcost);
  }

  if (!ctrl_stm->has_equivalent_letters)
  {
    ctrl_stm->has_equivalent_letters = TRUE;
  }
  DONE;
}

// The equality is dependant from the order of letter1 and letter2
static gboolean equivalent_letter_key_equal(gconstpointer key1, gconstpointer key2)
{
  struct equivalent_letter *k1 = (struct equivalent_letter *) key1;
  struct equivalent_letter *k2 = (struct equivalent_letter *) key2;

  if ((k1->letter1 == k2->letter1) && (k1->letter2 == k2->letter2))
  {
    return TRUE;
  }
  else
  {
    return FALSE;
  }
}

/**
 * Implementation of the FNV 32 bits hash function.
 * See http://www.isthe.com/chongo/tech/comp/fnv/ for details.
 * This is public domain code.
 */
static guint equivalent_letter_key_hash(gconstpointer key)
{
  struct equivalent_letter *k = (struct equivalent_letter *) key;

  unsigned magic_number1 = 2166136261;
  unsigned magic_number2 = 16777619;
  unsigned h = k->letter1 * magic_number1 + k->letter2 * magic_number2;

  return h;
}

PUBLIC(int) OgStmFlush(void *hstm)
{
  struct og_ctrl_stm *ctrl_stm = (struct og_ctrl_stm *) hstm;

  IFE(OgHeapFlush(ctrl_stm->hequivalent_letters_costs));
  IFE(OgHeapFlush(ctrl_stm->hequivalent_letters));
  g_hash_table_destroy(ctrl_stm->equivalent_letters_hash);

  IFE(OgMsgFlush(ctrl_stm->hmsg));
  DPcFree(ctrl_stm);
  DONE;
}

