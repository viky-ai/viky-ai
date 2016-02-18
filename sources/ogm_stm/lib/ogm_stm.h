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

/** stmlevutils.c*/
double StmGetMaxLevenshteinDistance(void *hstm, struct og_stm_levenshtein_input_param *lev_input_params);
og_status StmLogLevenshteinPathAndScore(struct og_ctrl_stm *ctrl_stm, int ustring1, unsigned char *string1,
    int ustring2, unsigned char* string2);
unsigned char *StmUtf8Letter(unsigned char *string, int i, unsigned char *buffer);
