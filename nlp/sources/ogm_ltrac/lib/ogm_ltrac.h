/*
 *  Internal header for linguistic transformation search
 *  Copyright (c) 2009 Pertimm, Inc. by Patrick Constant
 *  Dev: November 2009
 *  Version 1.0
*/
#include <logltrac.h>
#include <logaut.h>
#include <logpho.h>



/* Value for separator in automatons ha_base ha_swap and ha_phon
 * separator is a non valid unicode char, we cannot use extended
 * automaton, as we need to create fast automatons (.auf)
 * because they are 4 times smaller than normal automations (.aut)
*/
#define DOgLtracExtStringSeparator  1
#define DOgLtracMaxWordsSize 300

struct og_ltrac_input
{
  int dictionaries_to_export;
  int dictionaries_minimization;
  int min_frequency;
  int min_frequency_swap;
};

struct ltrac_dic_input
{
  int value_length;
  og_string value;
  int language_code;
  int frequency;
  og_bool is_expression;
};

struct og_ctrl_ltrac {
  void *herr,*hmsg; ogmutex_t *hmutex;
  struct og_loginfo cloginfo,*loginfo;

  char WorkingDirectory[DPcPathSize];
  char configuration_file[DPcPathSize];

  void *ha_base,*ha_swap,*ha_phon;
  void *hpho;

  struct og_ltrac_input input[1];
  };

struct og_ltrac_scan
{
  int iword;
  og_string word;
  int language_code;
  int Iltraf;
  og_bool is_expression;
};

struct read_ltraf_context
{
    int ientry;
    og_string entry;
    int language_code;
    int frequency;
    int min_frequency;
};

/** ltraca.c **/
int LtracAdd(struct og_ctrl_ltrac *ctrl_ltrac, struct og_ltrac_input *input);


/** ltracdic.c **/
int LtracDicAdd(struct og_ctrl_ltrac *ctrl_ltrac, struct ltrac_dic_input *dic_input);

/** ltracdica.c **/
int LtracDicAspellAdd(struct og_ctrl_ltrac *ctrl_ltrac,struct ltrac_dic_input *dic_input);

/** ltracdicb.c **/
int LtracDicBaseAdd(struct og_ctrl_ltrac *ctrl_ltrac,struct ltrac_dic_input *dic_input);

/** ltracdics.c **/
int LtracDicSwapAdd(struct og_ctrl_ltrac *ctrl_ltrac,struct ltrac_dic_input *dic_input);

/** ltracdicp.c **/
int LtracDicPhonAdd(struct og_ctrl_ltrac *ctrl_ltrac,struct ltrac_dic_input *dic_input);

