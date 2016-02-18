/*
 *  The header for the split module for linguistic transformations
 *  Copyright (c) 2010 Pertimm by Patrick Constant, Lo�s Rigouste
 *  Dev: January-November 2010
 *  Version 1.0
*/
#include <logltras.h>
#include <logaut.h>


struct og_ctrl_split {
  void *herr,*hmsg,*hltras,*ha_split,*hstm; ogmutex_t *hmutex;
  struct og_loginfo cloginfo,*loginfo;
  int max_small_word_length;
  int max_nb_splits;
  int start_trf;
  struct og_stm_levenshtein_input_param levenshtein_costs[1];
  };


int LtrasModuleSplitReadConf(struct og_ctrl_split *ctrl_split, char *filename);


