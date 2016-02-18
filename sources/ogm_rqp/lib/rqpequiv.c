/*
 *	Getting equivalent operators
 *	Copyright (c) 2006 Pertimm by Patrick Constant
 *	Dev : July 2006
 *	Version 1.0
*/
#include "ogm_rqp.h"



/*
 * Equivalent operators are operators such as: a or b or c
 * sometimes we need to get them because they are slighly different
 * example: a <1< b <2< c which is a and b and c but with proximities.
 * first operator is at position 0, then 1, etc.
 * return the Inode or -1 if not found.
*/

int RqpGetEquivalentOperator(struct og_ctrl_rqp *ctrl_rqp, int Ibasic_node, int position)
{
struct node *node;
int i,n=0;

for (i=Ibasic_node; i>=0; i=node->nxop) {
  if (n==position) break;
  node = ctrl_rqp->Node+i;
  n++;
  }

return(i);
}




