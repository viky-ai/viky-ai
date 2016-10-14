/*
 *  API navigation functions
 *  Copyright (c) 2006 Pertimm by Patrick Constant
 *  Dev : July,October 2006
 *  Version 1.1
*/
#include "ogm_rqp.h"





PUBLIC(int) OgRqpGetRootNode(void *handle)
{
struct og_ctrl_rqp *ctrl_rqp = (struct og_ctrl_rqp *)handle;
char erreur[DOgErrorSize];

IF(ctrl_rqp->Iroot_node) {
  sprintf(erreur,"OgRqpGetRootNode: could not find root node");
  OgErr(ctrl_rqp->herr,erreur); DPcErr;
  }

return(ctrl_rqp->Iroot_node);
}






PUBLIC(int) OgRqpNodeInformation(void *handle, int Inode, struct og_rqp_node *node)
{
struct og_ctrl_rqp *ctrl_rqp = (struct og_ctrl_rqp *)handle;
struct node *inode = ctrl_rqp->Node + Inode;

node->Inode=Inode;
node->depth=inode->depth;
node->is_function=inode->is_function;
node->name_length=inode->length;
node->name=ctrl_rqp->request+inode->start;
node->new_name_length=inode->nlength;
node->new_name=ctrl_rqp->Ba+inode->nstart;
node->boolean_operator=inode->boolean_operator;
node->structure=inode->structure;
node->subtree_number=inode->subtree_number;
node->is_subtree_root=inode->is_subtree_root;
node->before=inode->before;
node->next=inode->next;
node->father=inode->father;
node->son=inode->son;
node->bfop=inode->bfop;
node->nxop=inode->nxop;

DONE;
}






PUBLIC(int) OgRqpNodeAddNewName(void *handle, int Inode, int new_name_length, unsigned char *new_name)
{
struct og_ctrl_rqp *ctrl_rqp = (struct og_ctrl_rqp *)handle;
struct node *node = ctrl_rqp->Node + Inode;

node->nlength = new_name_length;
node->nstart = ctrl_rqp->BaUsed;

IFE(RqpAppendBa(ctrl_rqp,node->nlength,new_name));
ctrl_rqp->Ba[ctrl_rqp->BaUsed++]=0;

DONE;
}







PUBLIC(int) OgRqpNodeDelNewName(void *handle, int Inode)
{
struct og_ctrl_rqp *ctrl_rqp = (struct og_ctrl_rqp *)handle;
struct node *node = ctrl_rqp->Node + Inode;

node->nlength = 0;
node->nstart = 0;

DONE;
}







PUBLIC(int) OgRqpNodeAddStructure(void *handle, int Inode, void *structure)
{
struct og_ctrl_rqp *ctrl_rqp = (struct og_ctrl_rqp *)handle;
struct node *node = ctrl_rqp->Node + Inode;

node->structure = structure;

DONE;
}




/*
 * Gets subtree number in a given node.
 * A zero subtree number means there is no subtree.
*/

PUBLIC(int) OgRqpNodeSubtreeNumber(void *handle, int Inode, int *psubtree_number, int *pis_subtree_root)
{
struct og_ctrl_rqp *ctrl_rqp = (struct og_ctrl_rqp *)handle;
struct node *node = ctrl_rqp->Node + Inode;

if (psubtree_number) *psubtree_number = node->subtree_number;
if (pis_subtree_root) *pis_subtree_root = node->is_subtree_root;

DONE;
}





PUBLIC(int) OgRqpNodeSubtreeNumbers(void *handle, int Inode
  , int subtree_numbers_size, int *psubtree_numbers_length, int *subtree_numbers)
{
struct og_ctrl_rqp *ctrl_rqp = (struct og_ctrl_rqp *)handle;
struct node *node = ctrl_rqp->Node + Inode;
int Ifather_node,must_add_subtree_number;
int subtree_numbers_length = 0;
char erreur[DOgErrorSize];

while(1) {
  if (node->subtree_number >= 0) {
    must_add_subtree_number = 0;
    if (subtree_numbers_length == 0) must_add_subtree_number=1;
    else if (subtree_numbers[subtree_numbers_length-1] != node->subtree_number) must_add_subtree_number=1;
    if (must_add_subtree_number) {
      if (subtree_numbers_length >= subtree_numbers_size) {
        sprintf(erreur,"OgRqpNodeSubtreeNumbers: subtree_numbers_length (%d) >= subtree_numbers_size(%d)"
          , subtree_numbers_length, subtree_numbers_size);
        OgErr(ctrl_rqp->herr,erreur); DPcErr;
        }
      subtree_numbers[subtree_numbers_length++]=node->subtree_number;
      }
    }
  Ifather_node = node->father;
  if (Ifather_node < 0) break;
  node = ctrl_rqp->Node + Ifather_node;
  }

*psubtree_numbers_length = subtree_numbers_length;

DONE;
}



