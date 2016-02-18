/*
 *	Header for library ogm_rqp.dll
 *	Copyright (c) 2006-2007 Pertimm by Patrick Constant
 *	Dev: May,June,July,August,October,December 2006, March,June,December 2007
 *  Dev: March,May,December 2008
 *	Version 2.2
*/
#ifndef _LOGRQPALIVE_
#include <loggen.h>
#include <logthr.h>


#define DOgRqpBanner  "ogm_rqp V1.52, Copyright (c) 2006-2008 Pertimm, Inc."
#define DOgRqpVersion 152


/** Trace levels **/
#define DOgRqpTraceMinimal          0x1
#define DOgRqpTraceMemory           0x2
#define DOgRqpTraceParse            0x4
#define DOgRqpTraceFirstParse       0x8
#define DOgRqpTraceMinus            0x10
#define DOgRqpTraceClean            0x20
#define DOgRqpTraceNormalizeDepth   0x40
#define DOgRqpTracePriorities       0x80
#define DOgRqpTraceStructure        0x100

/** Boolean operator values **/
#define DOgBooleanNil       0
#define DOgBooleanAnd       1
#define DOgBooleanOr        2
#define DOgBooleanExcept    3

/** Proximity operator type **/
#define DOgProximityFile        0
#define DOgProximityWord	    1
#define DOgProximitySentence    2
#define DOgProximityParagraph   3
#define DOgProximityPage        4


struct og_rqp_param {
  void *herr,*hmsg; ogmutex_t *hmutex;
  struct og_loginfo loginfo;
  }; 


struct og_boolean_operator {
  int name;
  int proximity_type,proximity_value;
  int same_order;
  };

struct og_rqp_node {
  int depth,is_function;
  int name_length;  unsigned char *name;
  int new_name_length;  unsigned char *new_name;
  struct og_boolean_operator boolean_operator;
  int subtree_number,is_subtree_root;
  void *structure;
  int before,next;
  int father,son;
  int bfop,nxop;
  };

struct og_rqp_arg {
  int start,length;
  };


DEFPUBLIC(char *) OgRqpBanner(void);
DEFPUBLIC(void *) OgRqpInit(struct og_rqp_param *);
DEFPUBLIC(int) OgRqpReset(void *);
DEFPUBLIC(int) OgRqpFlush(void *);

DEFPUBLIC(int) OgRqpParse(void *, int, unsigned char *);
DEFPUBLIC(int) OgRqpLogTree(void *);
DEFPUBLIC(int) OgRqpLogPrettyTree(void *handle);
DEFPUBLIC(int) OgRqpLogPrettySubtree(void *handle, int Inode);
DEFPUBLIC(int) OgRqpPrettyTree(void *handle, int global_print_offset, int *buffer_length, unsigned char **buffer);
DEFPUBLIC(int) OgRqpPrettySubtree(void *handle, int Inode, int global_print_offset, int *buffer_length, unsigned char **buffer);
DEFPUBLIC(int) OgRqpReparenthesize(void *handle, int *pstring_length, unsigned char **pstring);
DEFPUBLIC(int) OgRqpReparenthesizeSubtree(void *handle, int Inode, int *pstring_length, unsigned char **pstring);

DEFPUBLIC(int) OgRqpGetRootNode(void *);
DEFPUBLIC(int) OgRqpNodeInformation(void *, int, struct og_rqp_node *);
DEFPUBLIC(int) OgRqpNodeAddNewName(void *, int, int, unsigned char *);
DEFPUBLIC(int) OgRqpNodeAddStructure(void *, int , void *);
DEFPUBLIC(int) OgRqpNodeDelNewName(void *, int);

DEFPUBLIC(int) OgRqpParseFunction(void *, int, unsigned char *, int, int *, struct og_rqp_arg *);

DEFPUBLIC(int) OgRqpXmlPrettyTree(void *handle
  , int global_print_offset, int *buffer_length, unsigned char **buffer
  , int (*func)(void *context, int print_offset, int node_id, int in_length, unsigned char *in, int *out_length, unsigned char **out)
  , void *context);
DEFPUBLIC(int) OgRqpXmlPrettySubtree(void *handle, int Inode
  , int global_print_offset, int *buffer_length, unsigned char **buffer
  , int (*func)(void *context, int print_offset, int node_id, int in_length, unsigned char *in, int *out_length, unsigned char **out)
  , void *context);  

DEFPUBLIC(int) OgRqpSubtreeSetFunctionName(void *handle, unsigned char *subtree_function_name);
DEFPUBLIC(int) OgRqpNodeSubtreeNumber(void *handle, int Inode, int *psubtree_number, int *pis_subtree_root);
DEFPUBLIC(int) OgRqpSubtreeNumberToId(void *handle, int subtree_number, unsigned char *subtree_id);
DEFPUBLIC(int) OgRqpSubtreeIdToNumber(void *handle , unsigned char *subtree_id, int *psubtree_number);
DEFPUBLIC(int) OgRqpNodeSubtreeNumbers(void *handle, int Inode
  , int subtree_numbers_size, int *psubtree_numbers_length, int *subtree_numbers);
DEFPUBLIC(int) OgRqpSubtreeScan(void *handle, int (*func)(void *context, int subtree_number, unsigned char *subtree_id), void *context);

DEFPUBLIC(int) OgRqpMem(void *hrqp, int must_log, int module_level, ogint64_t *pmem);

#define _LOGRQPALIVE_
#endif


