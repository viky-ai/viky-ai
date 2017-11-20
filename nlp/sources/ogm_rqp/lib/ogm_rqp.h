/*
 *  Internal header for parsing requests with parenthesis
 *  Copyright (c) 2006-2010 Pertimm, Inc. by Patrick Constant
 *  Dev : May,June,December 2006, November 2008, May 2010
 *  Version 1.4
*/
#include <logrqp.h>
#include <loguni.h>
#include <logmsg.h>
#include <logaut.h>
#include <logxml.h>


#define DOgNodeNumber       0x400  /* 1k */
#define DOgBaSize           0x1000 /* 1m */
#define DOgBxSize           0x1000 /* 64k */


struct node {
  int father,son;
  int before,next;
  int bfop,nxop;
  struct og_boolean_operator boolean_operator;
  int depth,in_dquote,start_dquote,is_function;
  int subtree_number,is_subtree_root;
  int start,length;
  int Inode, is_except_zone; /** just for sorting **/
  int nstart,nlength;
  void *structure;
  };


struct og_ctrl_rqp {
  void *herr,*hmsg; ogmutex_t *hmutex;
  struct og_loginfo cloginfo;
  struct og_loginfo *loginfo;

  int request_length;
  unsigned char *request;

  int Iroot_node;
  int NodeNumber;
  int NodeUsed;
  struct node *Node;

  int Ireparenthesize_root_node;

  /** Buffer for all operations **/
  unsigned char *Ba;
  int BaSize,BaUsed;

  /** Buffer for OgRqpXmlPrettyTree **/
  unsigned char *Bx;
  int BxSize,BxUsed;

  rqp_xml_pretty_func xml_pretty_func;
  void *xml_pretty_context;

  void *ha_subtree_id;
  };



/** irqp.c **/
int RqpReset(struct og_ctrl_rqp *);

/** rqpfpar.c **/
int RqpFirstParse(struct og_ctrl_rqp *);

/** rqpnode.c **/
int RqpAddNode(struct og_ctrl_rqp *, struct og_boolean_operator *, int, int, int, int, int, int, int);

/** rqplog.c **/
char *RqpOperatorString(int);
int OgRqpProximityString(struct og_boolean_operator *, char *);

/** rqpprior.c **/
int RqpApplyPriorities(struct og_ctrl_rqp *);

/** rqpnormd.c **/
int NormalizeDepth(struct og_ctrl_rqp *);

/** rqpminus.c **/
int RqpChangeMinusOperator(struct og_ctrl_rqp *);
int RqpSwapExceptOperator(struct og_ctrl_rqp *);

/** rqpclean.c **/
int RqpCleanMultipleOperator(struct og_ctrl_rqp *);

/** rqpstruct.c **/
int RqpCreateStructure(struct og_ctrl_rqp *);

/** rqpequiv.c **/
int RqpGetEquivalentOperator(struct og_ctrl_rqp *, int, int);

/** rqpmisc.c **/
int RqpSameOperator(struct og_ctrl_rqp *, int, int);

/** rqpba.c **/
int RqpAppendBa(struct og_ctrl_rqp *, int, unsigned char *);
int RqpTestReallocBa(struct og_ctrl_rqp *, int);
int RqpReallocBa(struct og_ctrl_rqp *, int);

/** rqpbx.c **/
int RqpAppendBx(struct og_ctrl_rqp *, int, unsigned char *);
int RqpTestReallocBx(struct og_ctrl_rqp *, int);
int RqpReallocBx(struct og_ctrl_rqp *, int);

/** rqptreeid.c **/
int RqpSubtreeAddId(struct og_ctrl_rqp *ctrl_rqp, int subtree_number, int uni_subtree_id_length,
    unsigned char *uni_subtree_id, og_rqp_subtree_type type);
int RqpSubtreeNumberToId(struct og_ctrl_rqp *ctrl_rqp, int subtree_number, unsigned char *subtree_id,
    og_rqp_subtree_type *type);

