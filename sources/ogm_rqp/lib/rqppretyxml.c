/*
 *	Logging a boolean tree in a tree fashion (pretty print) as XML
 *	Copyright (c) 2010 Pertimm by Patrick Constant
 *	Dev : May 2010
 *	Version 1.0
*/
#include "ogm_rqp.h"



static int RqpXmlPrettySubtree(struct og_ctrl_rqp *ctrl_rqp, int Inode, int print_offset);
static int RqpXmlPrettyNode(struct og_ctrl_rqp *ctrl_rqp, int Inode, int print_offset, int starting);






PUBLIC(int) OgRqpXmlPrettyTree(void *handle 
  , int global_print_offset, int *buffer_length, unsigned char **buffer
  , int (*func)(void *context, int print_offset, int node_id, int in_length, unsigned char *in, int *out_length, unsigned char **out)
  , void *context)
{
struct og_ctrl_rqp *ctrl_rqp = (struct og_ctrl_rqp *)handle;
IFE(OgRqpXmlPrettySubtree(handle,ctrl_rqp->Iroot_node,global_print_offset,buffer_length,buffer,func,context));
DONE;
}





PUBLIC(int) OgRqpXmlPrettySubtree(void *handle, int Inode
  , int global_print_offset, int *buffer_length, unsigned char **buffer
  , int (*func)(void *context, int print_offset, int node_id, int in_length, unsigned char *in, int *out_length, unsigned char **out)
  , void *context)
{
struct og_ctrl_rqp *ctrl_rqp = (struct og_ctrl_rqp *)handle;
*buffer=0; *buffer_length=0; 
if (ctrl_rqp->Iroot_node < 0) DONE;
ctrl_rqp->BxUsed=0;
ctrl_rqp->xml_pretty_func = func;
ctrl_rqp->xml_pretty_context = context;
IFE(RqpXmlPrettySubtree(handle,Inode,global_print_offset));
IFE(RqpAppendBx(ctrl_rqp,1,"")); /* append zero at end */
*buffer=ctrl_rqp->Bx;
*buffer_length=ctrl_rqp->BxUsed-1; /* -1 because of appended zero at end */
DONE;
}






static int RqpXmlPrettySubtree(struct og_ctrl_rqp *ctrl_rqp, int Inode, int print_offset)
{
struct node *node=ctrl_rqp->Node+Inode;
struct node *son=0;
int i;

IFE(RqpXmlPrettyNode(ctrl_rqp,Inode,print_offset,1));
if (node->son<0) DONE;

for (i=node->son; i>=0; i=son->next) {
  IFE(RqpXmlPrettySubtree(ctrl_rqp,i,print_offset+1));
  son = ctrl_rqp->Node+i;
  }

IFE(RqpXmlPrettyNode(ctrl_rqp,Inode,print_offset,0));

DONE;
}






static int RqpXmlPrettyNode(struct og_ctrl_rqp *ctrl_rqp, int Inode, int print_offset, int starting)
{
int ilabel,slabel=DOgMlogMaxMessageSize/2;
char label[DOgMlogMaxMessageSize/2],line[DOgMlogMaxMessageSize/2];
char proximity[DPcPathSize],offset[DPcPathSize],tag[DPcPathSize];
int ixmlproximity; unsigned char xmlproximity[DPcPathSize];
int ixmllabel; unsigned char xmllabel[DPcPathSize];
char subtree_id[DPcPathSize],stree[DPcPathSize];
struct node *node = ctrl_rqp->Node + Inode;
int iout; unsigned char *out;
int i,length,found;


offset[0]=0;
for (i=0; i<print_offset; i++) {
  sprintf(offset+strlen(offset),"  ");
  }

stree[0]=0;
if (node->subtree_number >= 0) {
  IFE(found=RqpSubtreeNumberToId(ctrl_rqp,node->subtree_number,subtree_id));
  if (found) {
    sprintf(stree," subtree_number=\"%d\" subtree_id=\"%s\"%s", node->subtree_number, subtree_id
      , node->is_subtree_root?" subtree_root=\"yes\"":"");
    }
  else {
    sprintf(stree," subtree_number=\"%d\"",node->subtree_number);
    }
  }

if (node->boolean_operator.name==DOgBooleanNil) {
  int max_length=(slabel-1)*2;
  length=node->length; if (length>max_length) length=max_length;
  IFE(OgUniToCp(length,ctrl_rqp->request+node->start,slabel,&ilabel,label,DOgCodePageUTF8,0,0));
  IFE(OgXmlEncode(ilabel,label,DPcPathSize,&ixmllabel,xmllabel,DOgCodePageUTF8,0));
  if (ctrl_rqp->xml_pretty_func) {
    IFE(RqpAppendBx(ctrl_rqp,strlen(offset),offset));
    IFE((*ctrl_rqp->xml_pretty_func)(ctrl_rqp->xml_pretty_context
      , print_offset, Inode ,ixmllabel, xmllabel, &iout, &out));
    IFE(RqpAppendBx(ctrl_rqp,iout,out));
    }
  else {
    sprintf(line,"%s<leaf Inode=\"%d\"%s>%s</leaf>\n",offset,Inode,stree,xmllabel);
    IFE(RqpAppendBx(ctrl_rqp,strlen(line),line));
    }
  }
else {
  sprintf(tag,"%s", RqpOperatorString(node->boolean_operator.name));
  IFE(OgRqpProximityString(&node->boolean_operator,proximity));
  IFE(OgXmlEncode(strlen(proximity),proximity,DPcPathSize,&ixmlproximity,xmlproximity,DOgCodePageUTF8,0));
  if (proximity[0]) sprintf(label," proximity=\"%s\"",xmlproximity);
  else label[0]=0;
  if (starting) {
    sprintf(line,"%s<%s Inode=\"%d\"%s%s>\n",offset,tag,Inode,label,stree);
    }
  else {
    sprintf(line,"%s</%s>\n",offset,tag);
    }
  IFE(RqpAppendBx(ctrl_rqp,strlen(line),line));
  }

DONE;
}




