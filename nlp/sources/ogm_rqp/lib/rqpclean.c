/*
 *	Main function for cleaning request
 *	Copyright (c) 2006 Pertimm by Patrick Constant
 *	Dev : June 2006
 *	Version 1.1
*/
#include "ogm_rqp.h"



static int RqpCleanMultipleOperator1(struct og_ctrl_rqp *, int, int);


/*
 *  Cleaning multiple operators
 *    a and -b -> a and except b -> a except b
 *    a or -b -> a or except b -> a except b
 *    a except -b -> a except b
 *  The general rule is that we take the last operator.
*/
int RqpCleanMultipleOperator(struct og_ctrl_rqp *ctrl_rqp)
{
int i,end=0,removed,start=0,depth,current_depth=(-1);

for (i=0; !end; i++) {
  if (i>=ctrl_rqp->NodeUsed) { depth=(-1); end=1; }
  else depth=ctrl_rqp->Node[i].depth;
  if (depth!=current_depth) {
    IFE(removed=RqpCleanMultipleOperator1(ctrl_rqp,start,i));
    if (ctrl_rqp->loginfo->trace & DOgRqpTraceClean) {
      OgMsg(ctrl_rqp->hmsg,"",DOgMsgDestInLog
        , "RqpCleanMultipleOperator: at %d, removed=%d",i,removed);
      IFE(OgRqpLogTree(ctrl_rqp));
      }
    i-=removed; start=i; current_depth=depth;
    }
  }

DONE;
}




static int RqpCleanMultipleOperator1(struct og_ctrl_rqp *ctrl_rqp, int start, int end)
{
struct node *node;
int i,finished=0,begin=0,state=1,is_operator,offset,removed=0;

if (ctrl_rqp->loginfo->trace & DOgRqpTraceClean) {
  OgMsg(ctrl_rqp->hmsg,"",DOgMsgDestInLog
    , "RqpCleanMultipleOperator1: from %d to %d ",start,end);
  }

for (i=start; !finished; i++) {
  node=ctrl_rqp->Node+i;
  if (i>=end) { is_operator=0; finished=1; }
  else if (node->boolean_operator.name == DOgBooleanNil) is_operator=0;
  else is_operator=1;

  switch (state) {
    case 1:
      if (is_operator) { state=2; begin=i; }
      break;
    case 2:
      if (!is_operator) {
        offset=i-begin-1;
        if (offset > 0) {
          int nb_nodes_to_move = ctrl_rqp->NodeUsed-begin-offset;
          if (ctrl_rqp->loginfo->trace & DOgRqpTraceClean) {
            OgMsg(ctrl_rqp->hmsg,"",DOgMsgDestInLog
              , "RqpCleanMultipleOperator1: moving from %d to %d size is %d",begin+offset,begin,nb_nodes_to_move);
            }
          memmove(ctrl_rqp->Node+begin,ctrl_rqp->Node+begin+offset,nb_nodes_to_move*sizeof(struct node));
          ctrl_rqp->NodeUsed-=offset;
          removed+=offset;
          end-=offset;
          }
        state=1;
        }
      break;
    }
  };

return(removed);
}






