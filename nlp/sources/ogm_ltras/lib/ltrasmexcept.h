/*
 *  Header for the Except module for linguistic transformations
 *  Copyright (c) 2010 Pertimm by Patrick Constant
 *  Dev: February 2010
 *  Version 1.0
*/
#include <logltras.h>
#include <logaut.h>
#include <logpho.h>
#include <logstm.h>


#define DOgLtrasExceptBaSize            0x400


struct og_ctrl_except {
  void *herr,*hmsg,*hltras; ogmutex_t *hmutex;
  struct og_loginfo cloginfo,*loginfo;
  int use_phonetics;
  double exc_score;
  double exc_phonetic_cost_reduction_ratio;
  struct og_stm_levenshtein_input_param levenshtein_costs[1];
  void *ha_except,*hpho,*hstm;

  int BaSize,BaUsed;
  unsigned char *Ba;

  int is_inherited;
  };


int LtrasModuleExceptReadConf(struct og_ctrl_except *ctrl_except, char *filename);

