/*
 *	Internal header for linguistic trf search
 *	Copyright (c) 2010 Pertimm by Patrick Constant
 *	Dev: January-March 2010
 *	Version 1.1
*/
#include <logltrap.h>


#define DOgLtrapSpanNumber      0x100
#define DOgLtrapSpansolNumber   0x100
#define DOgLtrapSequenceNumber  0x100
#define DOgLtrapSolNumber       0x100
#define DOgLtrapBaSize          0x400


struct span {
  int position,length,frequency;
  int start_word,length_word;
  int Isol,selected;
  };

struct spansol {
  int position,length,frequency;
  int Ispan;
  };

struct sequence {
  int Ispan;
  };

struct sol {
  int start_spansol,length_spansol;
  double weight,global_score; int global_frequency;
  int complete;
  };




struct og_ctrl_ltrap {
  void *herr,*hmsg; ogmutex_t *hmutex;
  struct og_loginfo cloginfo,*loginfo;
  char WorkingDirectory[DPcPathSize];
  char configuration_file[DPcPathSize];
  int synchronization_length,max_candidates,max_solutions;
  int max_header_length,min_swap_word_length;
  void *ha_base,*ha_swap,*ha_phon;

  struct og_ltrap_input *input;

  int SpanNumber;
  int SpanUsed;
  struct span *Span;

  int SequenceNumber;
  int SequenceUsed;
  struct sequence *Sequence;

  int SpansolNumber;
  int SpansolUsed;
  struct spansol *Spansol;

  int SolNumber;
  int SolUsed;
  struct sol *Sol;

  /** Buffer for all operations **/
  unsigned char *Ba;
  int BaSize,BaUsed;  

  int (*send_token)(void *context, struct og_ltrap_token *token);
  void *context;

  int span_overlap_length;
  int send_spans_by_solution;

  int max_small_word_length;
  int offset_small_word;
  void *ha_small_words;
  void *ha_false;
  
  double cut_cost; 
  double score_factor; 
  int max_word_frequency;
  double max_word_frequency_log10;
  };


/** iltrap.c **/
int LtrapReset(struct og_ctrl_ltrap *ctrl_ltrap);

/** ltrapba.c **/
int LtrapAppendBa(pr_(struct og_ctrl_ltrap *) pr_(int) pr(unsigned char *));
int LtrapTestReallocBa(pr_(struct og_ctrl_ltrap *) pr(int));

/** ltrapspan.c **/
int LtrapSpanAdd(struct og_ctrl_ltrap *ctrl_ltrap,int position,int length,int iword,unsigned char *word,int frequency);

/** ltrapseq.c **/
int LtrapTestReallocSequence(struct og_ctrl_ltrap *ctrl_ltrap,int Isequence);

/** ltrapspansol.c **/
int LtrapSpansolAdd(struct og_ctrl_ltrap *ctrl_ltrap,int Isol,int Ispan,int position,int length,int frequency);

/** ltrapsol.c **/
int LtrapSolAdd(struct og_ctrl_ltrap *ctrl_ltrap);

/** ltrapsolution.c **/
int OgLtrapBuildSolution(struct og_ctrl_ltrap *ctrl_ltrap);
int OgLtrapSortSolutions(struct og_ctrl_ltrap *ctrl_ltrap,int is_end,int *pstart);
int LtrapSendSolutions(struct og_ctrl_ltrap *ctrl_ltrap, int iuni, unsigned char *uni);

/** ltraplog.c **/
int LtrapSolsLog(struct og_ctrl_ltrap *ctrl_ltrap);
int LtrapSolLog(struct og_ctrl_ltrap *ctrl_ltrap, int Isol);

/** ltrapsmall.c **/
int LtrapIsValidAsSmallWord(struct og_ctrl_ltrap *ctrl_ltrap,int is, unsigned char *s);
int LtrapIsFalseWord(struct og_ctrl_ltrap *ctrl_ltrap,int is, unsigned char *s);

