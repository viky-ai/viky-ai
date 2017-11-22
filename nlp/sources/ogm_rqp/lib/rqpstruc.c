/*
 *	Creating the boolean structure
 *	Copyright (c) 2006 Pertimm by Patrick Constant
 *	Dev : July 2006
 *	Version 1.0
*/
#include "ogm_rqp.h"



static int RqpCreateStructure1(struct og_ctrl_rqp *, int);
static int RqpCreateStructure2(struct og_ctrl_rqp *, int, int, int); 




/*
 * Creating the boolean structure, based on the depth level.
*/

int RqpCreateStructure(struct og_ctrl_rqp *ctrl_rqp)
{
struct node *node,*father_node;
int i,max_depth=0;

for (i=0; i<ctrl_rqp->NodeUsed; i++) {
  node=ctrl_rqp->Node+i;
  if (max_depth < node->depth) max_depth=node->depth;
  }

for (i=max_depth; i>0; i--) {
  if (ctrl_rqp->loginfo->trace & DOgRqpTracePriorities) {
    OgMsg(ctrl_rqp->hmsg,"",DOgMsgDestInLog
      , "RqpCreateStructure: depth level %d before",i);
    IFE(OgRqpLogTree(ctrl_rqp));
    }
  IFE(RqpCreateStructure1(ctrl_rqp,i));
  if (ctrl_rqp->loginfo->trace & DOgRqpTracePriorities) {
    OgMsg(ctrl_rqp->hmsg,"",DOgMsgDestInLog
      , "RqpCreateStructure: depth level %d after",i);
    IFE(OgRqpLogTree(ctrl_rqp));
    }
  }

/** Getting the root of each tree **/
for (i=0; i<ctrl_rqp->NodeUsed; i++) {
  node=ctrl_rqp->Node+i;
  node->is_subtree_root=0;
  if (node->subtree_number < 0) continue;
  if (node->father < 0) node->is_subtree_root=1;
  else {
    father_node = ctrl_rqp->Node + node->father;
    if (father_node->subtree_number != node->subtree_number) node->is_subtree_root=1;
    }
  }

/** Getting the root node **/
if (ctrl_rqp->NodeUsed==1) {
  ctrl_rqp->Iroot_node=0;
  }
else {
  for (i=0; i<ctrl_rqp->NodeUsed; i++) {
    node=ctrl_rqp->Node+i;
    if (node->depth==0 && node->son>=0) { ctrl_rqp->Iroot_node=i; break; }
    }
  }

DONE;
}






static int RqpCreateStructure1(struct og_ctrl_rqp *ctrl_rqp, int basic_depth)
{
int i,end=0,start=0,depth;

for (i=0; !end; i++) {
  if (i>=ctrl_rqp->NodeUsed) { depth=(-1); end=1; }
  else depth=ctrl_rqp->Node[i].depth;
  if (depth>=basic_depth-1) {
    if (start==(-1)) start=i;  
    }
  else {
    if (start>=0) {
      if (ctrl_rqp->loginfo->trace & DOgRqpTracePriorities) {
        OgMsg(ctrl_rqp->hmsg,"",DOgMsgDestInLog
          , "RqpCreateStructure1: depth level %d start=%d end=%d before",basic_depth,start,i);
        IFE(OgRqpLogTree(ctrl_rqp));
        }
      IFE(RqpCreateStructure2(ctrl_rqp,basic_depth,start,i));
      if (ctrl_rqp->loginfo->trace & DOgRqpTracePriorities) {
        OgMsg(ctrl_rqp->hmsg,"",DOgMsgDestInLog
          , "RqpCreateStructure1: depth level %d start=%d end=%d after",basic_depth,start,i);
        IFE(OgRqpLogTree(ctrl_rqp));
        }
      }
    start=(-1);
    }
  }

DONE;
}






static int RqpCreateStructure2(struct og_ctrl_rqp *ctrl_rqp, int basic_depth, int start, int end)
{
struct node *node,*father,*last;
int i,Ifather,Ilast=(-1),first;
char erreur[DOgErrorSize];

Ifather=(-1);
for (i=start; i<end; i++) {
  node=ctrl_rqp->Node+i;
  if (node->depth>basic_depth) continue;
  if (node->depth==basic_depth-1 && node->boolean_operator.name!=DOgBooleanNil) { Ifather=i; break; }
  }

/** When we do not have any father we forget about the structure **/
if (Ifather<0) DONE;

father=ctrl_rqp->Node+Ifather;

first=1;
for (i=start; i<end; i++) {
  node=ctrl_rqp->Node+i;
  if (node->depth>basic_depth) continue;
  if (node->depth==basic_depth-1) continue;
  if (node->depth!=basic_depth) {
    sprintf(erreur,"RqpCreateStructure2: ->depth (%d) !=basic_depth (%d)",node->depth,basic_depth);
    OgErr(ctrl_rqp->herr,erreur); DPcErr;
    }
  if (node->son>=0 || node->boolean_operator.name==DOgBooleanNil) {
    node->father=Ifather;
    if (first) { first=0;
      father->son=i;
      }
    else {
      last=ctrl_rqp->Node+Ilast;
      last->next=i;
      node->before=Ilast;
      }
    Ilast=i;
    }
  }

DONE;
}





