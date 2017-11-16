/*
 *	Normalizing depth, i.e. removing depth level that do not exist
 *	Copyright (c) 2006 Pertimm by Patrick Constant
 *	Dev : June 2006
 *	Version 1.0
*/
#include "ogm_rqp.h"



static int NormalizeDepth1(struct og_ctrl_rqp *, int);
static int NormalizeDepth2(struct og_ctrl_rqp *, int, int, int);





int NormalizeDepth(struct og_ctrl_rqp *ctrl_rqp)
{
int i,max_depth=0;
struct node *node;

for (i=0; i<ctrl_rqp->NodeUsed; i++) {
  node=ctrl_rqp->Node+i;
  if (max_depth < node->depth) max_depth=node->depth;
  }

for (i=0; i<=max_depth; i++) {
  if (ctrl_rqp->loginfo->trace & DOgRqpTraceNormalizeDepth) {
    OgMsg(ctrl_rqp->hmsg,"",DOgMsgDestInLog
      , "NormalizeDepth: depth level %d before",i);
    IFE(OgRqpLogTree(ctrl_rqp));
    }
  IFE(NormalizeDepth1(ctrl_rqp,i));
  if (ctrl_rqp->loginfo->trace & DOgRqpTraceNormalizeDepth) {
    OgMsg(ctrl_rqp->hmsg,"",DOgMsgDestInLog
      , "NormalizeDepth: depth level %d after",i);
    IFE(OgRqpLogTree(ctrl_rqp));
    }
  }

DONE;
}






static int NormalizeDepth1(struct og_ctrl_rqp *ctrl_rqp, int depth)
{
int i,end=0,start=0,state=1,node_depth;

for (i=0; !end; i++) {
  if (i>=ctrl_rqp->NodeUsed) { node_depth=(-1); end=1; }
  else node_depth=ctrl_rqp->Node[i].depth;
  switch (state) {
    case 1:
      if (node_depth>=depth) { start=i; state=2; }
      break;
    case 2:
      if (node_depth<depth) { 
        IFE(NormalizeDepth2(ctrl_rqp,depth,start,i));
        state=1;
        }
      break;
    }
  }

DONE;
}




static int NormalizeDepth2(struct og_ctrl_rqp *ctrl_rqp, int depth, int start, int end)
{
struct node *node;
int i,min_depth=0x7fffffff;

for (i=start; i<end; i++) {
  node=ctrl_rqp->Node+i;
  if (min_depth > node->depth) min_depth=node->depth;  
  }

if (min_depth!=0x7fffffff && min_depth > depth) {
  int difference = min_depth-depth;
  for (i=start; i<end; i++) {
    node=ctrl_rqp->Node+i;
    node->depth-=difference;
    }
  }

DONE;
}







