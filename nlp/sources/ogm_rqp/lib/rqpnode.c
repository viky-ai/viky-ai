/*
 *  Handling of node structure.
 *  Copyright (c) 2005 Pertimm by Patrick Constant
 *  Dev : July 2005
 *  Version 1.0
*/
#include "ogm_rqp.h"


static int AllocNode(struct og_ctrl_rqp *, struct node **);





int RqpAddNode(struct og_ctrl_rqp *ctrl_rqp, struct og_boolean_operator *bo, int start, int length, int depth, int in_dquote, int start_dquote, int is_function, int subtree_number)
{
int Inode;
struct node *node;

IFE(Inode=AllocNode(ctrl_rqp,&node));
if (bo) node->boolean_operator=*bo;
node->start=start;
node->length=length;
node->depth=depth;
node->in_dquote=in_dquote;
node->start_dquote=start_dquote;
node->is_function=is_function;
node->subtree_number=subtree_number;
return(Inode);
}





static int AllocNode(struct og_ctrl_rqp *ctrl_rqp, struct node **pnode)
{
char erreur[DOgErrorSize];
struct node *node = 0;
int i=ctrl_rqp->NodeNumber;

beginAllocNode:

if (ctrl_rqp->NodeUsed < ctrl_rqp->NodeNumber) {
  i = ctrl_rqp->NodeUsed++; 
  }

if (i == ctrl_rqp->NodeNumber) {
  unsigned a, b; struct node *og_l;

  if (ctrl_rqp->loginfo->trace & DOgRqpTraceMemory) {
    OgMsg(ctrl_rqp->hmsg,"",DOgMsgDestInLog
      , "AllocNode: max Node number (%d) reached"
      , ctrl_rqp->NodeNumber);
    }
  a = ctrl_rqp->NodeNumber; b = a + (a>>2) + 1;
  IFn(og_l=(struct node *)malloc(b*sizeof(struct node))) {
    sprintf(erreur,"AllocNode: malloc error on Node");
    OgErr(ctrl_rqp->herr,erreur); DPcErr;
    }

  memcpy( og_l, ctrl_rqp->Node, a*sizeof(struct node));
  DPcFree(ctrl_rqp->Node); ctrl_rqp->Node = og_l;
  ctrl_rqp->NodeNumber = b;

  if (ctrl_rqp->loginfo->trace & DOgRqpTraceMemory) {
    OgMsg(ctrl_rqp->hmsg,"",DOgMsgDestInLog
      , "AllocNode: new Node number is %d\n", ctrl_rqp->NodeNumber);
    }

#ifdef DOgNoMainBufferReallocation
  sprintf(erreur,"AllocNode: NodeNumber reached (%d)",ctrl_rqp->NodeNumber);
  OgErr(ctrl_rqp->herr,erreur); DPcErr;
#endif

  goto beginAllocNode;
  }

node = ctrl_rqp->Node + i;
memset(node,0,sizeof(struct node));
node->father=(-1); node->son=(-1);
node->before=(-1); node->next=(-1);
node->bfop=(-1); node->nxop=(-1);
node->subtree_number=(-1);

if (pnode) *pnode = node;
return(i);
}


