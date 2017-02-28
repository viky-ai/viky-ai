/*
 *  Internal header for linguistic trf search
 *  Copyright (c) 2009-2010 Pertimm by Patrick Constant
 *  Dev: November 2009, January 2010
 *  Version 1.1
*/
#include <lpcgentype.h>
#include <logltras.h>
#include <logheap.h>
#include <loglip.h>
#include <logrqp.h>
#include <logaut.h>
#include <logpho.h>
#include <logldi.h>
#include <logis639_3166.h>


#define DOgLtrasSuggestionSize    100

#define DOgLtrasModuleNumber      100
#define DOgLtrasNodeNumber        100
#define DOgLtrasBaSize            0x800

#define DOgLtrasTrfsTrfNumber     0x200
#define DOgLtrasTrfsWordNumber    0x800
#define DOgLtrasTrfsModuleNumber  0x400
#define DOgLtrasTrfsBaSize        0x4000
#define DOgLtrasTrfsTransposition 0x100

#define DOgLtrasMinimumScoreLength          256
#define DOgLtrasMaximumTransformationLength 256

#define DOgLtrasMaxNbOperations  30


/** There are problems with real number so we adjust values with this when necessary **/
#define DOgLtrasTinyScore  0.00000001

#define DOgLtrasScorePrecision 10000

#define DOgLtrasDefaultEquivalentLetterScore 0.01


struct module {
  int start_name,length_name;
  void *(*init)(struct og_ltra_module_param *param);
  int (*module)(struct og_ltra_module_input *module_input, struct og_ltra_trfs *input,  struct og_ltra_trfs **output, ogint64_t *elapsed);
  int (*flush)(void *hmod);
  void *handle;

  //Global response time of each module
  ogint64_t elapsed;

  //NB requested words. Used to compute mean elapsed time of each module (elapsed/nb_requests)
  int nb_requests;
  };

struct node {
  struct og_ltra_trfs *trfs;
  int Irqp_node;
  };

struct statistics_range {
  double final_score,score; int frequency,nb_trfs;
  };

struct statistics {
  ogint64_t global_start,global_elapsed; int nb_requests,nb_trfs;
  ogint64_t total_elapsed,min_elapsed,max_elapsed;
  struct statistics_range mean_range;
  struct statistics_range range[DOgLtrasScorePrecision+1];
  int range_length;
  };


struct suggestion {
  int nb_propositions;
  double mean_final_score;
  double mean_score;
  double mean_frequency;
  int operations[DOgLtrasMaxNbOperations];
  };


struct og_ctrl_ltras
{
  void *herr, *hmsg;
  ogmutex_t *hmutex;
  struct og_loginfo cloginfo, *loginfo;
  char WorkingDirectory[DPcPathSize];
  char configuration_file[DPcPathSize];
  char dictionaries_directory[DPcPathSize];
  struct og_ltras_input *input;

  void *hltras_to_inherit;
  int ha_base_accessed, ha_swap_accessed, ha_phon_accessed, ha_expressions_accessed;
  void *ha_base, *ha_swap, *ha_phon, *ha_expressions;
  void *hpho, *hldi, *hstm;
  void *ha_false;

  double max_word_frequency_log10;
  int max_word_frequency;

  int phonetic_default_language;

  char caller_label[DPcPathSize];
  char output_file[DPcPathSize];
  int first_added_result;
  FILE *fdout;

  void *hlip;
  struct og_lip_conf lip_conf;

  int nb_suggestion_levels;
  struct suggestion suggestion[DOgLtrasSuggestionSize];

  int ModuleNumber;
  int ModuleUsed;
  struct module *Module;

  int NodeNumber;
  int NodeUsed;
  struct node *Node;

  int BaSize, BaUsed, BaModuleUsed;
  unsigned char *Ba;

  og_heap hba1;
  og_heap hba2;

  void *hrqp;

  struct statistics statistics;

  int suggestion_found;
  int nb_suggestions;
  void *ha_suggest;
  int nb_operations;
  int current_op;
  void *ha_oper;
  int current_trf;

  double minimum_score[DOgLtrasMinimumScoreLength];
  double minimum_final_score[DOgLtrasMinimumScoreLength];
  int maximum_transformation[DOgLtrasMaximumTransformationLength];
  double space_insertion_cost[DOgStmSpaceCostLength];
  double space_deletion_cost[DOgStmSpaceCostLength];

  void *ha_param;
  struct og_stm_levenshtein_input_param levenshtein_costs[1];
  int levenshtein_costs_calculated;
  };


/** iltras.c **/
int LtrasReset(struct og_ctrl_ltras *ctrl_ltras);

/** ltrasmba.c **/
int LtrasTrfsAppendBa(struct og_ctrl_ltras *ctrl_ltras,struct og_ltra_trfs *trfs,int is, unsigned char *s);
int LtrasTrfsTestReallocBa(struct og_ctrl_ltras *ctrl_ltras,struct og_ltra_trfs *trfs,int added_size);
int LtrasTrfsReallocBa(struct og_ctrl_ltras *ctrl_ltras,struct og_ltra_trfs *trfs,int added_size);

/** ltrasmtrans.c **/
int LtrasResetTrf(struct og_ctrl_ltras *ctrl_ltras, struct og_ltra_trfs *trfs, int Itrf);
int LtrasAllocTrf(struct og_ctrl_ltras *ctrl_ltras, struct og_ltra_trfs *trfs, struct og_ltra_trf **ptrf);

/** ltrasmword.c **/
int LtrasAddWord(struct og_ctrl_ltras *ctrl_ltras, struct og_ltra_trfs *trfs, struct og_ltra_trf *trf
  , int string_length, unsigned char *string, int frequency, int base_frequency
  , int start_position, int length_position, int language);

/** ltrasba.c **/
int LtrasAppendBa(struct og_ctrl_ltras *ctrl_ltras,int is, unsigned char *s);
int LtrasTestReallocBa(struct og_ctrl_ltras *ctrl_ltras, int added_size);
int LtrasReallocBa(struct og_ctrl_ltras *ctrl_ltras,int added_size);

/** ltrasmmodule.c **/
int LtrasAddModule(struct og_ctrl_ltras *ctrl_ltras, struct og_ltra_trfs *trfs, struct og_ltra_trf *trf, int module_id);

/** ltrasmod.c **/
int LtrasAddPluginModule(struct og_ctrl_ltras *ctrl_ltras,unsigned char *name,unsigned char *function_name,unsigned char *library_name);
int LtrasAllocModule(struct og_ctrl_ltras *ctrl_ltras,struct module **pmodule);

/** ltrasfcb.c **/
int LtrasFlowChartBoolean(struct og_ctrl_ltras *ctrl_ltras
  , struct og_ltra_trfs *input, struct og_ltra_trfs **output);

/** ltrasnode.c **/
int LtrasAllocNode(struct og_ctrl_ltras *ctrl_ltras,struct node **pnode);

/** ltrasres.c **/
int LtrasWordString(struct og_ctrl_ltras *ctrl_ltras,struct og_ltra_trfs *trfs,int Iword,int *piword,unsigned char *sword);

/** ltrassug.c **/
int LtrasSuggestionGet(void *handle, int is, unsigned char *s, int *psuggestion_number);

/** ltrasfalsec.c **/
int LtrasFalseReadConf(struct og_ctrl_ltras *ctrl_ltras, char *filename);

/** ltrasparam.c **/
int LtrasAddParameter(struct og_ctrl_ltras *ctrl_ltras,unsigned char *parameter_name,unsigned char *parameter_value);


