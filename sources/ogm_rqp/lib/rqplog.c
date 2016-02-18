/*
 *	Logging a boolean tree linearly
 *	Copyright (c) 2006-2007 Pertimm by Patrick Constant
 *	Dev : May 2006, December 2007
 *	Version 1.1
*/
#include "ogm_rqp.h"



static int OgRqpLogTree1(struct og_ctrl_rqp *, int);
static int OgRqpFunctionString(struct og_ctrl_rqp *, int, unsigned char *);




PUBLIC(int) OgRqpLogTree(void *handle)
{
struct og_ctrl_rqp *ctrl_rqp = (struct og_ctrl_rqp *)handle;
int i;

OgMsg(ctrl_rqp->hmsg,"",DOgMsgDestInLog
  ,"Root Node: %d", ctrl_rqp->Iroot_node);

for (i=0; i<ctrl_rqp->NodeUsed; i++) {
  IFE(OgRqpLogTree1(ctrl_rqp,i));
  }

DONE;
}





static int OgRqpLogTree1(struct og_ctrl_rqp *ctrl_rqp, int Inode)
{
int subtree_numbers_length,subtree_numbers[DPcPathSize]; char ssubtree_numbers[DPcPathSize];
char label[DPcPathSize],proximity[DPcPathSize],buffer[DPcPathSize],function[DPcPathSize];
char subtree_id[DPcPathSize],stree[DPcPathSize];
struct node *node = ctrl_rqp->Node + Inode;
int i,found;
int ilabel;

IFE(OgRqpProximityString(&node->boolean_operator,proximity));
if (proximity[0]) sprintf(buffer," %s",proximity);
else buffer[0]=0;

if (node->is_function) {
  IFE(OgRqpFunctionString(ctrl_rqp,Inode,function));
  }
else function[0]=0;

IFE(OgCpToUni(node->length, ctrl_rqp->request+node->start
  , DPcPathSize, &ilabel, label, DOgCodePageUTF8, 0, 0));

stree[0]=0; ssubtree_numbers[0]=0;
if (node->subtree_number >= 0) {
  sprintf(ssubtree_numbers+strlen(ssubtree_numbers)," [");
  IFE(OgRqpNodeSubtreeNumbers(ctrl_rqp,Inode,DPcPathSize,&subtree_numbers_length,subtree_numbers));
  for (i=0; i<subtree_numbers_length; i++) {
    sprintf(ssubtree_numbers+strlen(ssubtree_numbers),"%s%d",i?" ":"",subtree_numbers[i]);
    }
  sprintf(ssubtree_numbers+strlen(ssubtree_numbers),"]");
  IFE(found=RqpSubtreeNumberToId(ctrl_rqp,node->subtree_number,subtree_id));
  if (found) {
    sprintf(stree,"subtree=%d:%s%s%s",node->subtree_number,subtree_id, node->is_subtree_root?":root":"",ssubtree_numbers);
    }
  else {
    sprintf(stree,"subtree=%d%s",node->subtree_number,ssubtree_numbers);
    }
  }

OgMsg(ctrl_rqp->hmsg,"",DOgMsgDestInLog
  , "%3d: father=%d son=%d before=%d next=%d bfop=%d nxop=%d depth=%d %s operator=%s%s '%s' %s"
  , Inode, node->father, node->son, node->before, node->next, node->bfop, node->nxop, node->depth, stree
  , RqpOperatorString(node->boolean_operator.name), buffer, label, function);

DONE;
}





static int OgRqpFunctionString(struct og_ctrl_rqp *ctrl_rqp, int Inode, unsigned char *buffer)
{
struct node *node = ctrl_rqp->Node + Inode;
int ilabel; char label[DPcPathSize];
int i,arg_size=10,arg_length;
struct og_rqp_arg arg[10];
int is; unsigned char *s;

s=ctrl_rqp->request+node->start;
is=node->length;

IFE(OgRqpParseFunction(ctrl_rqp,is,s,arg_size,&arg_length,arg));

buffer[0]=0;
for (i=0; i<arg_length; i++) {
  IFE(OgUniToCp(arg[i].length,s+arg[i].start
    ,DPcPathSize,&ilabel,label,DOgCodePageUTF8,0,0));
  strcat(buffer+strlen(buffer),label);
  if (i==0) sprintf(buffer+strlen(buffer),"(");
  else if (i+1<arg_length) sprintf(buffer+strlen(buffer),",");
  else sprintf(buffer+strlen(buffer),")");
  }

DONE;
}





char *RqpOperatorString(int boolean_operator)
{
switch(boolean_operator) {
  case DOgBooleanNil: return("nil");
  case DOgBooleanAnd: return("and");
  case DOgBooleanOr: return("or");
  case DOgBooleanExcept: return("except");
  }
return("unknown");
}




int OgRqpProximityString(struct og_boolean_operator *bo, char *s)
{
char order=(bo->same_order?'<':'>');

s[0]=0;
switch (bo->proximity_type) {
  case DOgProximityWord:
    sprintf(s,"<%d%c",bo->proximity_value,order);
    break;
  case DOgProximitySentence:
    sprintf(s,"<s%c",order);
    break;
  case DOgProximityParagraph:
    sprintf(s,"<§%c",order);
    break;
  case DOgProximityPage:
    sprintf(s,"<s%c",order);
    break;
  }

DONE;
}



