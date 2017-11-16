/*
 *  Logging a boolean tree in a tree fashion (pretty print)
 *  Copyright (c) 2006-2010 Pertimm by Patrick Constant
 *  Dev : July,August,November 2006, August 2008, September 2010
 *  Version 1.4
*/
#include "ogm_rqp.h"



static int RqpPrettySubtree(struct og_ctrl_rqp *ctrl_rqp, int Inode, int in_buffer, int print_offset);
static int RqpPrettyNode(struct og_ctrl_rqp *ctrl_rqp, int Inode, int in_buffer, int print_offset);





PUBLIC(int) OgRqpLogPrettyTree(void *handle)
{
struct og_ctrl_rqp *ctrl_rqp = (struct og_ctrl_rqp *)handle;
if (ctrl_rqp->Iroot_node < 0) DONE;
IFE(OgRqpLogPrettySubtree(handle,ctrl_rqp->Iroot_node));
DONE;
}



PUBLIC(int) OgRqpLogPrettySubtree(void *handle, int Inode)
{
IFE(RqpPrettySubtree(handle,Inode,0,0));
DONE;
}




PUBLIC(int) OgRqpPrettyTree(void *handle, int global_print_offset, int *buffer_length, unsigned char **buffer)
{
struct og_ctrl_rqp *ctrl_rqp = (struct og_ctrl_rqp *)handle;
if (ctrl_rqp->Iroot_node < 0) DONE;
IFE(OgRqpPrettySubtree(handle,ctrl_rqp->Iroot_node,global_print_offset,buffer_length,buffer));
DONE;
}



PUBLIC(int) OgRqpPrettySubtree(void *handle, int Inode, int global_print_offset, int *buffer_length, unsigned char **buffer)
{
struct og_ctrl_rqp *ctrl_rqp = (struct og_ctrl_rqp *)handle;
*buffer_length=0;
ctrl_rqp->BxUsed=0;
IFE(RqpPrettySubtree(handle,Inode,1,global_print_offset));
IFE(RqpAppendBx(ctrl_rqp,1,"")); /* append zero at end */
*buffer=ctrl_rqp->Bx;
*buffer_length=ctrl_rqp->BxUsed-1; /* -1 because of appended zero at end */
DONE;
}




static int RqpPrettySubtree(struct og_ctrl_rqp *ctrl_rqp, int Inode, int in_buffer, int print_offset)
{
struct node *node=ctrl_rqp->Node+Inode;
struct node *son=0;
int i;

IFE(RqpPrettyNode(ctrl_rqp,Inode,in_buffer,print_offset));
if (node->son<0) DONE;

for (i=node->son; i>=0; i=son->next) {
  IFE(RqpPrettySubtree(ctrl_rqp,i,in_buffer,print_offset+1));
  son = ctrl_rqp->Node+i;
  }

DONE;
}






static int RqpPrettyNode(struct og_ctrl_rqp *ctrl_rqp, int Inode, int in_buffer, int print_offset)
{
int ilabel=DOgMlogMaxMessageSize/2;
char label[DOgMlogMaxMessageSize/2];
char proximity[DPcPathSize],buffer[DPcPathSize],offset[DPcPathSize];
char subtree_id[DPcPathSize],stree[DPcPathSize];
struct node *node = ctrl_rqp->Node + Inode;
int i,length,found,ibuffer;

offset[0]=0;
for (i=0; i<print_offset; i++) {
  sprintf(offset+strlen(offset),"  ");
  }
if (node->boolean_operator.name==DOgBooleanNil) {
  int max_length=(ilabel-1)*2;
  length=node->length; if (length>max_length) length=max_length;
  IFE(OgUniToCp(length,ctrl_rqp->request+node->start,DPcPathSize,&ibuffer,buffer,DOgCodePageUTF8,0,0));
  sprintf(label,"'%s'",buffer);
  }
else {
  sprintf(label, "%s", RqpOperatorString(node->boolean_operator.name));
  IFE(OgRqpProximityString(&node->boolean_operator,proximity));
  if (proximity[0]) sprintf(label,"%s",proximity);
  }

stree[0]=0;
if (node->subtree_number >= 0) {
  og_rqp_subtree_type type = DOgRqpSubtreeTypeNormal;
  IFE(found=RqpSubtreeNumberToId(ctrl_rqp,node->subtree_number,subtree_id, &type));
  //char *subtree_type = ((type == DOgRqpSubtreeTypeWithout) ? "without_subtree" : "subtree");
  if (found) {
    sprintf(stree,",%d:%s%s", node->subtree_number, subtree_id, node->is_subtree_root?":root":"");
    }
  else {
    sprintf(stree,",%d",node->subtree_number);
    }
  }

sprintf(buffer, "%s%s (%d%s)\n", offset, label, Inode, stree);
if (in_buffer) {
  IFE(RqpAppendBx(ctrl_rqp,strlen(buffer),buffer));
  }
else {
  OgMsg(ctrl_rqp->hmsg, "", DOgMsgDestInLog, "%s", buffer);
  }

DONE;
}




