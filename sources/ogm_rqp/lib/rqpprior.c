/*
 *	Applying priorities for operator
 *	Copyright (c) 2006-2008 Pertimm by Patrick Constant
 *	Dev : June,July 2006, November 2008
 *	Version 1.2
*/
#include "ogm_rqp.h"


static int RqpApplyPriorities1(struct og_ctrl_rqp *, int, int);
static int RqpApplyPriorities2(struct og_ctrl_rqp *, int, int, int);
static int RqpApplyPriorities3(struct og_ctrl_rqp *, int, int, int, int, int);
static int RqpSameProximity(struct og_ctrl_rqp *,struct node *,int);




/*
 * Priorities are applied on each depth level.
 * order is except, and, or.
*/

int RqpApplyPriorities(struct og_ctrl_rqp *ctrl_rqp)
{
IFE(RqpApplyPriorities1(ctrl_rqp,DOgBooleanExcept,0));
IFE(RqpApplyPriorities1(ctrl_rqp,DOgBooleanAnd,0));
IFE(RqpApplyPriorities1(ctrl_rqp,DOgBooleanAnd,1));
IFE(RqpApplyPriorities1(ctrl_rqp,DOgBooleanOr,0));
IFE(NormalizeDepth(ctrl_rqp));
DONE;
}





int RqpApplyPriorities1(struct og_ctrl_rqp *ctrl_rqp, int boolean_operator, int has_proximity)
{
int i,end=0,start=0,depth,min_depth,current_depth=(-1);

while(1) {
  min_depth=0x7fffffff;
  for (i=0; i<ctrl_rqp->NodeUsed; i++) {
    depth=ctrl_rqp->Node[i].depth;
    if (depth <= current_depth) continue;
    if (min_depth > depth) min_depth=depth;
    }
  if (min_depth == 0x7fffffff) break;
  IFE(RqpApplyPriorities2(ctrl_rqp,boolean_operator,has_proximity,min_depth));
  current_depth = min_depth;
  }

DONE;
}




int RqpApplyPriorities2(struct og_ctrl_rqp *ctrl_rqp, int boolean_operator, int has_proximity, int min_depth)
{
int i,end=0,start=0,depth;

for (i=0; !end; i++) {
  if (i>=ctrl_rqp->NodeUsed) { depth=(-1); end=1; }
  else depth=ctrl_rqp->Node[i].depth;
  if (depth<min_depth) {
    IFE(RqpApplyPriorities3(ctrl_rqp,boolean_operator,has_proximity,min_depth,start,i));
    if (ctrl_rqp->loginfo->trace & DOgRqpTracePriorities) {
      OgMsg(ctrl_rqp->hmsg,"",DOgMsgDestInLog
        , "RqpApplyPriorities2: at %d for operator %s and min_depth=%d (from=%d to=%d)"
        ,i,RqpOperatorString(boolean_operator),min_depth,start,i);
      IFE(OgRqpLogTree(ctrl_rqp));
      }
    start=i+1;
    }
  }
DONE;
}






static int RqpApplyPriorities3(struct og_ctrl_rqp *ctrl_rqp, int boolean_operator, int has_proximity, int min_depth, int start, int end)
{
struct node *node, *last;
int i,bo_exists=0,Ilast;
int same_proximity;

for (i=start; i<end; i++) {
  node=ctrl_rqp->Node+i;
  IFE(same_proximity=RqpSameProximity(ctrl_rqp,node,has_proximity));
  if (node->boolean_operator.name==boolean_operator && same_proximity && node->depth==min_depth) { bo_exists=1; break; }
  }

if (!bo_exists) DONE;

/* Setting the links between equivalent boolean operators node.
 * This is necessary when equivalent operators are not fully
 * equivalent, for example when we have different proximities. */
Ilast=(-1);
for (i=start; i<end; i++) {
  node=ctrl_rqp->Node+i;
  IFE(same_proximity=RqpSameProximity(ctrl_rqp,node,has_proximity));
  if (node->boolean_operator.name==boolean_operator && same_proximity && node->depth==min_depth) { 
    if (Ilast >= 0) {
      last=ctrl_rqp->Node+Ilast;
      last->nxop=i;
      node->bfop=Ilast;
      }
    Ilast=i;
    }
  }

for (i=start; i<end; i++) {
  node=ctrl_rqp->Node+i;
  IFE(same_proximity=RqpSameProximity(ctrl_rqp,node,has_proximity));
  if (node->boolean_operator.name==boolean_operator  && same_proximity && node->depth==min_depth) continue;
  //if (node->depth < min_depth) continue;
  node->depth++;
  }

DONE;
}





static int RqpSameProximity(struct og_ctrl_rqp *ctrl_rqp, struct node *node, int has_proximity)
{
int same_proximity;
if (has_proximity) {
  if (node->boolean_operator.proximity_value <= 0) same_proximity=0;
  else same_proximity=1;
  }
else {
  if (node->boolean_operator.proximity_value <= 0) same_proximity=1;
  else same_proximity=0;
  }  
return(same_proximity);
}




