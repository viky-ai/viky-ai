/*
 *	Main function for handling minus
 *	Copyright (c) 2006 Pertimm by Patrick Constant
 *	Dev : June 2006
 *	Version 1.1
*/
#include "ogm_rqp.h"



static int RqpChangeMinusOperator1(struct og_ctrl_rqp *, int, int);
static int RqpSwapExceptOperator1(struct og_ctrl_rqp *, int, int);
static int swap_except_cmp(const void *, const void *);




/*
 *  The minus operator is changed into the except operator.
 *  a and (c -b) -> a and (c except b)
 *  a and (-c b) -> a and (except c b)
 *  a -b -> a except b
*/

int RqpChangeMinusOperator(struct og_ctrl_rqp *ctrl_rqp)
{
int i,end=0,added,start=0,depth,current_depth=(-1);

for (i=0; !end; i++) {
  if (i>=ctrl_rqp->NodeUsed) { depth=(-1); end=1; }
  else depth=ctrl_rqp->Node[i].depth;
  if (depth!=current_depth) {
    IFE(added=RqpChangeMinusOperator1(ctrl_rqp,start,i));
    if (ctrl_rqp->loginfo->trace & DOgRqpTraceMinus) {
      OgMsg(ctrl_rqp->hmsg,"",DOgMsgDestInLog
        , "RqpChangeMinusOperator: at %d, added=%d",i,added);
      IFE(OgRqpLogTree(ctrl_rqp));
      }
    i+=added; start=i; current_depth=depth;
    }
  }

DONE;
}




static int RqpChangeMinusOperator1(struct og_ctrl_rqp *ctrl_rqp, int start, int end)
{
int i,nb_nodes_to_move,added=0;
struct node *node;
unsigned char *s;

for (i=start; i<end; i++) {
  node=ctrl_rqp->Node+i;
  if (node->boolean_operator.name != DOgBooleanNil) continue;
  s = ctrl_rqp->request+node->start;
  if (s[0]==0 && s[1]=='-') {
    int depth=node->depth, old_start=node->start; 
    node->start+=2; node->length-=2;
    /** Just to make sure we have enough room at the end **/
    IFE(RqpAddNode(ctrl_rqp,0,0,0,0,0,0,0,0));
    /** -1 because NodeUsed has been incremented by AddNode **/
    nb_nodes_to_move = ctrl_rqp->NodeUsed-i-1;
    if (nb_nodes_to_move > 0) {
      if (ctrl_rqp->loginfo->trace & DOgRqpTraceMinus) {
        OgMsg(ctrl_rqp->hmsg,"",DOgMsgDestInLog
          , "RqpChangeMinusOperator1: moving from %d size is %d",i,nb_nodes_to_move);
        }
      memmove(ctrl_rqp->Node+i+1,ctrl_rqp->Node+i,nb_nodes_to_move*sizeof(struct node));
      node=ctrl_rqp->Node+i;
      memset(node,0,sizeof(struct node));
      node->father=(-1); node->son=(-1);
      node->before=(-1); node->next=(-1);
      node->start=old_start; node->length=2;
      node->boolean_operator.name=DOgBooleanExcept;       
      node->depth=depth;
      end++; added++; i++;
      }
    }
  }

return(added);
}






/*
 *  Given a certain depth level, except and minus operation
 *  must be transferred to the end of the zone. Example
 *  a except b c except d -> a c except b except d
 *  The simplest way is to added two fields: Inode and is_except_node
 *  and sort by is_except_node and Inode.
*/

int RqpSwapExceptOperator(struct og_ctrl_rqp *ctrl_rqp)
{
int i,end=0,added,start=0,depth,current_depth=(-1);

for (i=0; !end; i++) {
  if (i>=ctrl_rqp->NodeUsed) { depth=(-1); end=1; }
  else depth=ctrl_rqp->Node[i].depth;
  if (depth!=current_depth) {
    IFE(added=RqpSwapExceptOperator1(ctrl_rqp,start,i));
    #ifdef DEVERMINE
    OgMsg(ctrl_rqp->hmsg,"",DOgMsgDestInLog
      , "RqpSwapExceptOperator: at %d, added=%d",i,added);
    IFE(OgRqpLogTree(ctrl_rqp));
    #endif
    start=i; current_depth=depth;
    }
  }

DONE;
}







static int RqpSwapExceptOperator1(struct og_ctrl_rqp *ctrl_rqp, int start, int end)
{
struct node *node, *nodep1;
int i;

for (i=start; i<end; i++) {
  node=ctrl_rqp->Node+i;
  node->Inode=i; node->is_except_zone=0;
  };

for (i=start; i<end; i++) {
  node=ctrl_rqp->Node+i;
  if (node->boolean_operator.name == DOgBooleanExcept) {
    if (i+1<end) {
      nodep1=ctrl_rqp->Node+i+1;
      if (nodep1->boolean_operator.name == DOgBooleanNil) {
        nodep1->is_except_zone=1;
        }
      }
    node->is_except_zone=1;
    }
  }

qsort(ctrl_rqp->Node+start,end-start,sizeof(struct node),swap_except_cmp);
DONE;
}






static int swap_except_cmp(const void *ptr1, const void *ptr2)
{
struct node *node1 = (struct node *)ptr1;
struct node *node2 = (struct node *)ptr2;

if (node1->is_except_zone != node2->is_except_zone) return(node1->is_except_zone - node2->is_except_zone);
return(node1->Inode - node2->Inode);
}





