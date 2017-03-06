/*
 *  Internal header for linguistic transformation search
 *  Copyright (c) 2009 Pertimm, Inc. by Patrick Constant
 *  Dev: November 2009
 *  Version 1.0
*/
#include <logltrac.h>
#include <logsidx.h>
#include <loglip.h>
#include <logaut.h>
#include <logpho.h>
#include <logattribute.h>



/* Value for separator in automatons ha_base ha_swap and ha_phon
 * separator is a non valid unicode char, we cannot use extended
 * automaton, as we need to create fast automatons (.auf)
 * because they are 4 times smaller than normal automations (.aut)
*/
#define DOgLtracExtStringSeparator  1
#define DOgLtracMaxWordsSize 300

struct ltrac_dic_input
{
  int value_length;
  og_string value;
  int language_code;
  int frequency;
  og_bool is_expression;
};

struct ltraf
{
  int frequency;
};

struct og_ctrl_ltrac {
  void *herr,*hmsg; ogmutex_t *hmutex;
  struct og_loginfo cloginfo,*loginfo;

  char WorkingDirectory[DPcPathSize];
  char configuration_file[DPcPathSize];
  char data_directory[DPcPathSize];

  char name_base[DPcPathSize];
  char name_swap[DPcPathSize];
  char name_phon[DPcPathSize];
  char name_aspell[DPcPathSize];
  char log_base[DPcPathSize];
  char log_swap[DPcPathSize];
  char log_phon[DPcPathSize];
  void *ha_base,*ha_swap,*ha_phon,*ha_filter,*ha_seen;
  void *hpho;

  /** Used to scan the attributes **/
  og_bool has_ltraf_requests;

  void *ha_ltrac;

  int LtrafNumber;
  int LtrafUsed;
  struct ltraf *Ltraf;

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
int LtracDicAdd(struct og_ctrl_ltrac *ctrl_ltrac, struct og_ltrac_input *input, struct ltrac_dic_input *dic_input);
int LtracDicInit(struct og_ctrl_ltrac *ctrl_ltrac, struct og_ltrac_input *input);
int LtracDicWrite(struct og_ctrl_ltrac *ctrl_ltrac, struct og_ltrac_input *input);
int LtracDicFlush(struct og_ctrl_ltrac *ctrl_ltrac, struct og_ltrac_input *input);

/** ltracdica.c **/
int LtracDicAspellAdd(struct og_ctrl_ltrac *ctrl_ltrac,struct ltrac_dic_input *dic_input);

/** ltracdicb.c **/
int LtracDicBaseAdd(struct og_ctrl_ltrac *ctrl_ltrac,struct ltrac_dic_input *dic_input);

/** ltracdics.c **/
int LtracDicSwapAdd(struct og_ctrl_ltrac *ctrl_ltrac,struct ltrac_dic_input *dic_input);

/** ltracdicexpressions.c **/
og_status LtracDicExpressionAdd(struct og_ctrl_ltrac *ctrl_ltrac, struct ltrac_dic_input *dic_input);

/** ltracdicp.c **/
int LtracDicPhonAdd(struct og_ctrl_ltrac *ctrl_ltrac,struct ltrac_dic_input *dic_input);

/** ltracread.c **/
og_status LtracReadLtrafs(struct og_ctrl_ltrac *ctrl_ltrac, og_string filename, int min_frequency, int (*func)(struct og_ctrl_ltrac *ctrl_ltrac, struct read_ltraf_context *ctx));
og_status LtracScan(struct og_ctrl_ltrac *ctrl_ltrac, int (*func)(void *context, struct og_ltrac_scan *scan),
    void *context);
og_status LtracAddLtrafEntry(struct og_ctrl_ltrac *ctrl_ltrac, struct read_ltraf_context *ctx);
og_status LtracAddLtrafRequestEntry(struct og_ctrl_ltrac *ctrl_ltrac, struct read_ltraf_context *ctx);

/** ltraclog.c **/
og_status LtracLogLtrac(struct og_ctrl_ltrac *ctrl_ltrac);
og_status LtracLogLtracRequests(struct og_ctrl_ltrac *ctrl_ltrac);
