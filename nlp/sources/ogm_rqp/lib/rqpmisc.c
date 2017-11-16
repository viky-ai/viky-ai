/*
 *	Miscellaneous functions
 *	Copyright (c) 2006 Pertimm by Patrick Constant
 *	Dev : July 2006
 *	Version 1.0
*/
#include "ogm_rqp.h"





int RqpSameOperator(struct og_ctrl_rqp *ctrl_rqp, int Inode1, int Inode2)
{
struct node *node1 = ctrl_rqp->Node + Inode1;
struct node *node2 = ctrl_rqp->Node + Inode2;
int retour;

retour=memcmp(&node1->boolean_operator,&node2->boolean_operator,sizeof(struct og_boolean_operator));
IFn(retour) return(1);
return(0);
}




