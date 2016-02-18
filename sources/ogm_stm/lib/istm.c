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
  struct og_lip_param lip_param[1];
  memset(lip_param,0,sizeof(struct og_lip_param));
  lip_param->herr=ctrl_stm->herr;
  lip_param->hmsg=ctrl_stm->hmsg;
  lip_param->hmutex=ctrl_stm->hmutex;
  lip_param->loginfo.trace = DOgLipTraceMinimal+DOgLipTraceMemory;
  lip_param->loginfo.where = ctrl_stm->loginfo->where;
  lip_param->conf = &ctrl_stm->lip_conf;

  if(ctrl_stm->WorkingDirectory[0])
  {
    sprintf(lip_param->language_dictionary,"%s/ling/languages.auf",ctrl_stm->WorkingDirectory);
  }
  else
  {
    strcpy(lip_param->language_dictionary,"ling/languages.auf");
  }

  if (ctrl_stm->WorkingDirectory[0])
  {
    sprintf(lip_param->filename, "%s/conf/%s", ctrl_stm->WorkingDirectory, DOgLipConfPunctuationFileName);
  }
  else
  {
    sprintf(lip_param->filename, "conf/%s", DOgLipConfPunctuationFileName);
  }

  IFn(ctrl_stm->hlip=OgLipInit(lip_param)) return(0);


  og_char_buffer lt_heap_name[DOgAutNameSize];

  snprintf(lt_heap_name, DOgAutNameSize, "stm_hequivalent_letters_costs");
  IFn(ctrl_stm->hequivalent_letters_costs = OgHeapSliceInit(ctrl_stm->hmsg, lt_heap_name, sizeof(double), 2, 8)) return NULL;

  snprintf(lt_heap_name, DOgAutNameSize, "stm_hequivalent_letters");
  IFn(ctrl_stm->hequivalent_letters = OgHeapSliceInit(ctrl_stm->hmsg, lt_heap_name, sizeof(struct equivalent_letter), 2, 8)) return NULL;

  ctrl_stm->equivalent_letters_hash = g_hash_table_new(equivalent_letter_key_hash, equivalent_letter_key_equal);

  return ctrl_stm;
}


PUBLIC(og_status) OgStmInitDefaultCosts(void *hstm, struct og_stm_levenshtein_input_param *lev_param)
{
  memset(lev_param, 0, sizeof(struct og_stm_levenshtein_input_param));
  lev_param->insertion_cost = 0.1;
  lev_param->deletion_cost = 0.1;
  lev_param->substitution_cost = 0.1;
  lev_param->swap_cost = 0.1;
  lev_param->accents_substitution_cost = 0.01;
  lev_param->same_letter_insertion_cost = 0.02;
  lev_param->same_letter_deletion_cost = 0.02;
  lev_param->space_cost = 0.005;
  lev_param->case_cost = 0.001;
  lev_param->punctuation_cost = 0.001;
  DONE;
}

// If element already exist, we insert it only if the cost is lower
PUBLIC(og_status) OgStmInitCreateEquivalentLetterEntry(void *hstm, int letter1, int letter2, double cost)
{
  struct og_ctrl_stm *ctrl_stm = (struct og_ctrl_stm *)hstm;

  // Init and store the cost of the equivalent letter entry
  double *pcost = (double *) OgHeapNewCell(ctrl_stm->hequivalent_letters_costs, NULL);
  *pcost = cost;

  // Init and store the equivalent letter entry
  struct equivalent_letter *equivalent_letter = (struct equivalent_letter *) OgHeapNewCell(ctrl_stm->hequivalent_letters, NULL);

  equivalent_letter->letter1 = letter1;
  equivalent_letter->letter2 = letter2;

  // Store in the map the equivalent letter entry as a key and the cost as a value
  // check if element is already present and add it only if cost is lower
  double *pcost_already_present = (double *) g_hash_table_lookup(ctrl_stm->equivalent_letters_hash, equivalent_letter);
  if(pcost_already_present!=NULL)
  {
    if(*pcost_already_present > *pcost)
    {
      g_hash_table_insert(ctrl_stm->equivalent_letters_hash, equivalent_letter, pcost);
    }
  }
  else
  {
    g_hash_table_insert(ctrl_stm->equivalent_letters_hash, equivalent_letter, pcost);
  }

  if(!ctrl_stm->has_equivalent_letters)
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
  unsigned h = k->letter1*magic_number1 + k->letter2*magic_number2;

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



