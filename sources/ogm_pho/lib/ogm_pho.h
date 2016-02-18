/*
 *	Internal header for phonetizer
 *	Copyright (c) 2008 Pertimm, Inc. by Patrick Constant
 *	Dev : November 2006, April 2008
 *	Version 1.1
*/

#include <logpho.h>
#include <logaut.h>
#include <logxml.h>


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
  struct og_ctrl_pho *ctrl_pho;
  int offset;
  int Irule;
  int selected;
  };

struct char_class {
  unsigned char character[2];
  int start,number;
  };

struct og_ctrl_pho {
  void *herr,*hmsg; ogmutex_t *hmutex;
  struct og_loginfo cloginfo;
  struct og_loginfo *loginfo; 
  
  /* config */
  
  int max_steps;
  
  unsigned char space_character[2];
  int non_alpha_to_space;
  unsigned char appending_characters[2*DOgMaxAppendingCharacters];
  int appending_characters_number;

  void *ha_rules;
  struct rule *Rule;
  int RuleNumber,RuleUsed;

  struct char_class *Char_class;
  int Char_classNumber,Char_classUsed;
  unsigned char *BaClass;
  int BaClassSize,BaClassUsed;
  int is_inherited;
  
  /* process */
  
  unsigned char input[DPcPathSize*4];
  int iinput;
  
  unsigned char bufferIn[DPcPathSize*4+10];
  int ibufferIn;

  struct matching *Matching;
  int MatchingNumber,MatchingUsed;

  unsigned char *Ba;
  int BaSize;
  int BaUsed;
  };

struct og_xml_info {
  struct og_ctrl_pho *ctrl_pho;
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
int PhoReadConf(pr_(struct og_ctrl_pho *) pr(char *));

/** phoaut.c **/
int PhoRulesAutExceptionAdd(pr_(struct og_ctrl_pho *) pr_(char *) pr(char *));
int PhoRulesAutRuleAdd(pr_(struct og_ctrl_pho *) pr_(int) pr_(char *) pr(int));

/** phorules.c **/
int PhoRulesRuleAdd(pr(struct og_xml_info *));
int PhoRulesRuleGet(pr_(struct og_ctrl_pho *) pr_(int) pr(int));
int RulesLog(pr_(struct og_ctrl_pho *) pr(char *));

/** phorule.c **/
int AllocRule(pr_(struct og_ctrl_pho *) pr(struct rule **));

/** phoforma.c **/
int PhoFormatClean(pr(struct og_ctrl_pho *));
int PhoFormatAppendingCharAdd(pr_(struct og_ctrl_pho *) pr_(int) pr(unsigned char *));

/** phomatch.c **/
int PhoMatchingRules(pr_(struct og_ctrl_pho *) pr(int));
int PhoMatchingAdd(pr_(struct og_ctrl_pho *) pr_(int) pr(int));
int MatchingLog(pr_(struct og_ctrl_pho *) pr(int));

/** phoba.c **/
int PhoAppendBa(pr_(struct og_ctrl_pho *) pr_(int) pr(unsigned char *));
int PhoAppendBaClass(pr_(struct og_ctrl_pho *) pr_(int) pr(unsigned char *));

/** phoanaly.c **/
int PhoAnalysing(pr(struct og_ctrl_pho *));

/** phowrite.c **/
int PhoWriting(pr(struct og_ctrl_pho *));

/** phoexp.c **/
int PhoRulesRuleAddExpense(pr_(struct og_ctrl_pho *) pr_(int) pr_(char *) pr_(int) pr(int));

/** phoclass.c ***/
int ClassCreate(pr_(struct og_ctrl_pho *) pr(unsigned char *));
int ClassAddC(pr_(struct og_ctrl_pho *) pr_(int) pr(unsigned char *));
int ClassLog(pr_(struct og_ctrl_pho *) pr(char *));
