/*
 *	Flow chart node
 *	Copyright (c) 2010 Pertimm, by Patrick Constant
 *	Dev : January 2010
 *	Version 1.0
*/
#include "ogm_ltras.h"





int LtrasAllocNode(struct og_ctrl_ltras *ctrl_ltras,struct node **pnode)
{
char erreur[DOgErrorSize];
struct node *node = 0;
int i=ctrl_ltras->NodeNumber;

beginAllocNode:

if (ctrl_ltras->NodeUsed < ctrl_ltras->NodeNumber) {
  i = ctrl_ltras->NodeUsed++; 
  }

if (i == ctrl_ltras->NodeNumber) {
  unsigned a, b; struct node *og_node;

  if (ctrl_ltras->loginfo->trace & DOgLtrasTraceMemory) {
    OgMsg(ctrl_ltras->hmsg,"",DOgMsgDestInLog
      , "AllocNode: max Node number (%d) reached"
      , ctrl_ltras->NodeNumber);
    }
  a = ctrl_ltras->NodeNumber; b = a + (a>>2) + 1;
  IFn(og_node=(struct node *)malloc(b*sizeof(struct node))) {
    sprintf(erreur,"AllocNode: malloc error on Node");
    OgErr(ctrl_ltras->herr,erreur); DPcErr;
    }

  memcpy( og_node, ctrl_ltras->Node, a*sizeof(struct node));
  DPcFree(ctrl_ltras->Node); ctrl_ltras->Node = og_node;
  ctrl_ltras->NodeNumber = b;

  if (ctrl_ltras->loginfo->trace & DOgLtrasTraceMemory) {
    OgMsg(ctrl_ltras->hmsg,"",DOgMsgDestInLog
      , "AllocNode: new Node number is %d\n", ctrl_ltras->NodeNumber);
    }

#ifdef DOgNoMainBufferReallocation
  sprintf(erreur,"AllocNode: NodeNumber reached (%d)",ctrl_ltras->NodeNumber);
  OgErr(ctrl_ltras->herr,erreur); DPcErr;
#endif

  goto beginAllocNode;
  }

node = ctrl_ltras->Node + i;
memset(node,0,sizeof(struct node));

if (pnode) *pnode = node;
return(i);
}


