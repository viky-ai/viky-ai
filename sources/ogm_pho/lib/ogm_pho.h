/*
 *  Internal header for phonetizer
 *  Copyright (c) 2008 Pertimm, Inc. by Patrick Constant
 *  Dev : November 2006, April 2008
 *  Version 1.1
*/
#include <lpcgentype.h>
#include <logpho.h>
#include <logaut.h>
#include <logxml.h>
#include <logheap.h>

#include <glib.h>


#define DOgMaxRuleStringSize        0x0100 /* 256 */

#define DOgMaxXmlPath               10
#define DOgMaxXmlStringPath         0x1000 /* 4k */

#define DOgRuleNumber               0x0400 /* 1k */
#define DOgMatchingNumber           0x0400 /* 1k */
#define DOgBaSize                   0x0400 /* 1k */
#define DOgBaClassSize              0x0400 /* 1k */
#define DOgChar_classNumber         0x0010 /* 16 */

#define DOgMaxAppendingCharacters   0x0200 /* 512 */

struct rule {
  char left[DOgMaxRuleStringSize];int ileft;
  char key[DOgMaxRuleStringSize];int ikey;
  char right[DOgMaxRuleStringSize];int iright;
  char phonetic[DOgMaxRuleStringSize];int iphonetic;
  int replace;
  int step;
  int context_size;
  };

struct matching {
  struct lang_context *lang_context;
  int offset;
  int Irule;
  int selected;
  };

struct char_class {
  unsigned char character[2];
  int start,number;
  };


struct lang_context
{
  struct og_ctrl_pho *ctrl_pho;

  /* config */

  int max_steps;

  unsigned char space_character[2];
  int non_alpha_to_space;
  og_bool keep_digit;
  unsigned char appending_characters[2 * DOgMaxAppendingCharacters];
  int appending_characters_number;

  // Rule info
  void *ha_rules;
  struct rule *Rule;
  int RuleNumber;
  int RuleUsed;

  // Class info
  struct char_class *Char_class;
  int Char_classNumber;
  int Char_classUsed;
  unsigned char *BaClass;
  int BaClassSize;
  int BaClassUsed;

  /* process */

  unsigned char input[DPcPathSize * 4];
  int iinput;

  unsigned char bufferIn[DPcPathSize * 4 + 10];
  int ibufferIn;

  struct matching *Matching;
  int MatchingNumber;
  int MatchingUsed;

  unsigned char *Ba;
  int BaSize;
  int BaUsed;

};

struct og_ctrl_pho
{
  void *herr, *hmsg;
  ogmutex_t *hmutex;
  struct og_loginfo cloginfo;
  struct og_loginfo *loginfo;

  int is_inherited;

  char conf_directory[DPcPathSize];

  GHashTable *lang_context_map;
  og_heap lang_context_heap;
};

struct og_xml_info {
  struct og_ctrl_pho *ctrl_pho;
  struct lang_context *lang_context;

  int iB; unsigned char *B;
  int end_tag, auto_tag, ixml_path, xml_path[DOgMaxXmlPath];

  int option_type;

  int step;
  char left[DPcPathSize];int ileft;
  char key[DPcPathSize];int ikey;
  char right[DPcPathSize];int iright;
  char phonetic[DPcPathSize];int iphonetic;
  int replace;

  int Ichar_class;
  };

struct og_tree_xml_tag {
  int value, iname; char *name;
  };


/** phoreadc.c **/
og_status PhoReadConfFiles(struct og_ctrl_pho *ctrl_pho, unsigned char *conf_directory);

/** phoaut.c **/
int PhoRulesAutExceptionAdd(pr_(struct og_ctrl_pho *) pr_(char *) pr(char *));
og_status PhoRulesAutRuleAdd(struct lang_context *lang_context, int iword, char *word, int Irule);

/** phorules.c **/
og_status PhoRulesRuleAdd(pr(struct og_xml_info *));
og_status PhoRulesRuleGet(struct lang_context *lang_context, int offset, int step);
og_status RulesLog(struct lang_context *lang_context, char *filename);

/** phorule.c **/
int AllocRule(struct lang_context *lang_context, struct rule **prule);

/** phoforma.c **/
og_status PhoFormatClean(struct lang_context *lang_context);
og_status PhoFormatAppendingCharAdd(struct lang_context *lang_context, int ib, unsigned char *b);

/** phomatch.c **/
og_status PhoMatchingRules(struct lang_context *lang_context, int step);
og_status PhoMatchingAdd(struct lang_context *lang_context, int offset, int Irule);
og_status MatchingLog(struct lang_context *lang_context, int step);

/** phoba.c **/
og_status PhoAppendBa(struct lang_context *lang_context, int is, unsigned char *s);
og_status PhoAppendBaClass(struct lang_context *lang_context, int is, unsigned char *s);

/** phoanaly.c **/
og_status PhoAnalysing(struct lang_context *lang_context);

/** phowrite.c **/
og_status PhoWriting(struct lang_context *lang_context);

/** phoexp.c **/
og_status PhoRulesRuleAddExpense(struct lang_context *lang_context, int iword, char *word, int indice, int Irule);

/** phoclass.c **/
int ClassCreate(struct lang_context *lang_context, unsigned char *b);
og_status ClassAddC(struct lang_context *lang_context, int Ichar_class, unsigned char *b);
og_status ClassLog(struct lang_context *lang_context, char *filename);

/** ipho.c **/
void PhoLangContextDestroy(struct lang_context *lang_context);
og_status PhoInitLangContext(struct og_ctrl_pho *ctrl_pho, struct lang_context *lang_context);
