/*
 *  The header for the cut module for linguistic transformations
 *  Copyright (c) 2010 Pertimm by Patrick Constant
 *  Dev: January-March 2010
 *  Version 1.0
*/
#include <logltras.h>
#include <logltrap.h>


struct og_ctrl_cut {
  int new_words_length; unsigned char new_words[DPcPathSize];
  struct og_ltra_add_trf_input ctinput,*tinput;
  void *herr,*hmsg,*hltras,*hstm; ogmutex_t *hmutex;
  struct og_loginfo cloginfo,*loginfo;
  int Itrf,iuni; unsigned char *uni;
  struct og_ltra_trfs *trfs;
  double removed_score;
  int use_swap_automaton; /** < nb substitution/suppression letters */
  int max_nb_candidates;
  int max_nb_solutions;
  double cut_cost;

  void *hltrap,*ha_cut;
  int max_small_word_length;

  int is_inherited;
  struct og_stm_levenshtein_input_param levenshtein_costs[1];
  };


int LtrasModuleCutReadConf(struct og_ctrl_cut *ctrl_cut, char *filename);


