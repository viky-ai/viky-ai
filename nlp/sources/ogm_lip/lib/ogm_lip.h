/*
 *	Internal header for handling the linguistic parser.
 *	Copyright (c) 2006-2008 Pertimm, by Patrick Constant
 *	Dev : October 2006, August 2007, November 2008
 *	Version 1.2
*/
#include <loglip.h>
#include <loguni.h>
#include <logxml.h>
#include <logmsg.h>
#include <logauta.h>
#include <logheap.h>

#define DOgPawoNumber   0x100 /* 256 */


struct pawo {
  int start, length;
  int real_start, real_length;
  int has_quote,has_dot_separation;
  int punctuation_before_word;
  int Iexpression_group;
  int is_indivisible_expression;
  struct og_lip_lang lang;
  };


struct language {
  int lang; double score;
  };


struct punctword {
  int start,length;
  };

struct og_ctrl_lip {
  void *herr, *hmsg; ogmutex_t *hmutex;
  struct og_loginfo cloginfo;
  struct og_loginfo *loginfo;
  void *hentity;

  struct og_lip_input *input;

  int PawoNumber;
  int PawoUsed;
  struct pawo *Pawo;

  /** Special char can be : punctuation char, punctuation word or separator char */
  struct og_lip_conf *conf;
  struct og_lip_conf default_conf;

  void *ha_lang;
  struct language language[DOgLangMax];

  /** for OgLipOutputXml **/
  struct og_lip_input output_xml_input[1];
  FILE *fdout;
  };



/** lippawo.c **/
int LipAddPawo(struct og_ctrl_lip *ctrl_lip, int start, int length, int next_char, int punctuation_before_word,
    int Iexpression_group, int is_indivisible_expression);

/** liplang.c **/
int LipLang(struct og_ctrl_lip *);

/** lipglang.c **/
int LipGetLang(struct og_ctrl_lip *);

/** lipalang.c **/
int LipAdjustLang(struct og_ctrl_lip *ctrl_lip, int document_language_code);

/** lip.c */
int OgLipIsSingleSeparator(struct og_lip_conf *conf, int c);
int OgLipIsIndivisibleSeparator(struct og_lip_conf *conf, int c);
int OgLipIsOpenSeparator(struct og_lip_conf *conf, int c);
int OgLipIsCloseSeparator(struct og_lip_conf_separator *preview_open_sperator, int c);
struct og_lip_conf_separator* OgLipGetSeparator(struct og_lip_conf *conf, int c);

