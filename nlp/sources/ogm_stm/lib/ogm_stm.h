/*
 *  Internal header for string matching
 *  Copyright (c) 2007 Pertimm, Inc. by Patrick Constant, Lo�s Rigouste
 *  Dev : September 2007
 *  Version 1.0
*/

#include <logstm.h>
#include <logauta.h>
#include <loglip.h>
#include <logheap.h>
#include <glib.h>


#define DOgStmDefaultInsertionCost     0.1
#define DOgStmDefaultDeletionCost      0.1
#define DOgStmDefaultSubstitutionCost  0.1
#define DOgStmDefaultSwapCost          0.1
#define DOgStmDefaultAccentSubstitutionCost  0.01
#define DOgStmDefaultSameLetterDeletionCost  0.02
#define DOgStmDefaultSameLetterInsertionCost  0.02
#define DOgStmDefaultSpaceCost  0.005
#define DOgStmDefaultCaseCost  0.001
#define DOgStmDefaultPunctuationCost  0.0

#define DOgStmSpaceCostLength 256

struct og_ctrl_stm
{
  void *herr, *hmsg;
  ogmutex_t *hmutex;
  struct og_loginfo cloginfo;
  struct og_loginfo *loginfo;

  void *hlip;
  struct og_lip_conf lip_conf;
  char WorkingDirectory[DPcPathSize];

  LEV_OPERATION path[DOgStmMaxWordLength + 9][DOgStmMaxWordLength + 9];
  double score[DOgStmMaxWordLength + 9][DOgStmMaxWordLength + 9];
  int from_i[DOgStmMaxWordLength + 9][DOgStmMaxWordLength + 9],
      from_j[DOgStmMaxWordLength + 9][DOgStmMaxWordLength + 9];
  int length1;
  int length2;

  og_heap hequivalent_letters_costs;
  og_heap hequivalent_letters;
  GHashTable *equivalent_letters_hash;
  og_bool has_equivalent_letters;

  double space_insertion_cost[DOgStmSpaceCostLength]; /**< chat => ch at*/
  double space_deletion_cost[DOgStmSpaceCostLength];  /**< ch at => chat */

};

struct equivalent_letter
{
    int letter1;
    int letter2;
};

/** stmlevcompute.c*/
og_status StmComputeLevenshteinBoard(struct og_ctrl_stm *ctrl_stm, unsigned char *string1,
    unsigned char *string2,
    struct og_stm_levenshtein_input_param *lev_params);
og_status StmInitBorderScores(struct og_ctrl_stm *ctrl_stm, unsigned char *string1, unsigned char *string2,
    struct og_stm_levenshtein_input_param *lev_params);
og_status StmGetSpaceCost(struct og_ctrl_stm *ctrl_stm, int occurence, og_bool is_insertion, double *space_cost);

/** stmlevutils.c*/
double StmGetMaxLevenshteinDistance(void *hstm, struct og_stm_levenshtein_input_param *lev_input_params);
og_status StmLogLevenshteinPathAndScore(struct og_ctrl_stm *ctrl_stm, int ustring1, unsigned char *string1,
    int ustring2, unsigned char* string2);
unsigned char *StmUtf8Letter(unsigned char *string, int i, unsigned char *buffer);
