/*
 *  Logging a boolean tree in a tree fashion (pretty print)
 *  Copyright (c) 2006-2010 Pertimm by Patrick Constant
 *  Dev : July, November 2006, March 2008, January 2010
 *  Version 1.3
*/
#include "ogm_rqp.h"



static int RqpReparenthesizeSubtree(struct og_ctrl_rqp *ctrl_rqp, int Inode, int *string_length, unsigned char **pstring);
static int RqpReparenthesizeNode(struct og_ctrl_rqp *ctrl_rqp, int Inode, int *pstring_length, unsigned char **pstring);
static int RqpReparenthesizeCopy(struct og_ctrl_rqp *ctrl_rqp, int string_length, unsigned char *string, unsigned char **pstring);




/*
 *  returns 1 if truncated, 0 otherwise.
*/

PUBLIC(int) OgRqpReparenthesize(void *handle, int *pstring_length, unsigned char **pstring)
{
struct og_ctrl_rqp *ctrl_rqp = (struct og_ctrl_rqp *)handle;

if (ctrl_rqp->Iroot_node < 0) {
  *pstring_length=0; *pstring="\0\0"; DONE;
  }
IFE(OgRqpReparenthesizeSubtree(handle,ctrl_rqp->Iroot_node,pstring_length,pstring));
DONE;
}




/*
 *  returns 1 if truncated, 0 otherwise.
*/

PUBLIC(int) OgRqpReparenthesizeSubtree(void *handle, int Inode, int *pstring_length, unsigned char **pstring)
{
struct og_ctrl_rqp *ctrl_rqp = (struct og_ctrl_rqp *)handle;
ctrl_rqp->Ireparenthesize_root_node=Inode;
IFE(RqpReparenthesizeSubtree(ctrl_rqp,Inode,pstring_length,pstring));
DONE;
}





static int RqpReparenthesizeSubtree(struct og_ctrl_rqp *ctrl_rqp, int Inode, int *pstring_length,
    unsigned char **pstring)
{
  int iuni_subtree_id;
  unsigned char uni_subtree_id[DPcPathSize * 2];
  int ibuffer;
  unsigned char buffer[DPcPathSize * 2];
  int i, n, Iequiv, same_equiv, BaStart, found;
  struct node *node = ctrl_rqp->Node + Inode;
  int sequiv_length;
  unsigned char *sequiv;
  int snode_length;
  unsigned char *snode;
  int sson_length;
  unsigned char *sson;
  unsigned char subtree_id[DPcPathSize];
  char erreur[DOgErrorSize];
  struct node *son = 0;

  BaStart = ctrl_rqp->BaUsed;

  IFE(RqpReparenthesizeNode(ctrl_rqp, Inode, &snode_length, &snode));

  if (node->is_subtree_root)
  {
    IFE(found = RqpSubtreeNumberToId(ctrl_rqp, node->subtree_number, subtree_id));
    if (!found)
    {
      sprintf(erreur, "RqpReparenthesizeSubtree: could not find subtree_id from subtree_number=%d",
          node->subtree_number);
      OgErr(ctrl_rqp->herr, erreur);
      DPcErr;
    }
    IFE(
        OgCpToUni(strlen(subtree_id), subtree_id, DPcPathSize, &iuni_subtree_id, uni_subtree_id, DOgCodePageUTF8, 0, 0));
    ibuffer = 0;
    memcpy(buffer + ibuffer, ctrl_rqp->subtree_function_name, ctrl_rqp->subtree_function_name_length);
    ibuffer += ctrl_rqp->subtree_function_name_length;
    memcpy(buffer + ibuffer, "\0(", 2);
    ibuffer += 2;
    memcpy(buffer + ibuffer, uni_subtree_id, iuni_subtree_id);
    ibuffer += iuni_subtree_id;
    memcpy(buffer + ibuffer, "\0,\0 ", 4);
    ibuffer += 4;
    IFE(RqpAppendBa(ctrl_rqp, ibuffer, buffer));
  }
  else if (Inode != ctrl_rqp->Ireparenthesize_root_node && node->son >= 0)
  {
    IFE(RqpAppendBa(ctrl_rqp, 2, "\0("));
  }

  if (node->son < 0)
  {
    IFE(RqpAppendBa(ctrl_rqp, snode_length, snode));
  }
  else
  {
    for (i = node->son, n = 0; i >= 0; i = son->next)
    {
      IFE(RqpReparenthesizeSubtree(ctrl_rqp, i, &sson_length, &sson));

      if (i != node->son)
      {
        IFE(RqpAppendBa(ctrl_rqp, 2, "\0 "));
        /** Using the first node, except if equivalent node is different **/
        same_equiv = 0;
        IF(Iequiv=RqpGetEquivalentOperator(ctrl_rqp,Inode,n-1)) same_equiv = 1;
        else if (RqpSameOperator(ctrl_rqp, Inode, Iequiv)) same_equiv = 1;
        else
        {
          IFE(RqpReparenthesizeNode(ctrl_rqp, Iequiv, &sequiv_length, &sequiv));
        }
        if (same_equiv)
        {
          IFE(RqpAppendBa(ctrl_rqp, snode_length, snode));
        }
        else
        {
          IFE(RqpAppendBa(ctrl_rqp, sequiv_length, sequiv));
          DPcFree(sequiv);
        }
        IFE(RqpAppendBa(ctrl_rqp, 2, "\0 "));
      }
      IFE(RqpAppendBa(ctrl_rqp, sson_length, sson));
      son = ctrl_rqp->Node + i;
      DPcFree(sson);
      n++;
    }
  }

  if (node->is_subtree_root)
  {
    IFE(RqpAppendBa(ctrl_rqp, 2, "\0)"));
  }
  else if (Inode != ctrl_rqp->Ireparenthesize_root_node && node->son >= 0)
  {
    IFE(RqpAppendBa(ctrl_rqp, 2, "\0)"));
  }

  *pstring_length = ctrl_rqp->BaUsed - BaStart;
  IFE(RqpReparenthesizeCopy(ctrl_rqp, *pstring_length, ctrl_rqp->Ba + BaStart, pstring));
  ctrl_rqp->BaUsed = BaStart;
  DPcFree(snode);

  DONE;
}





static int RqpReparenthesizeNode(struct og_ctrl_rqp *ctrl_rqp, int Inode, int *pstring_length, unsigned char **pstring)
{
char label[DPcPathSize],proximity[DPcPathSize];
struct node *node=ctrl_rqp->Node+Inode;
int i,ilabel,length;

if (node->boolean_operator.name==DOgBooleanNil) {
  if (node->nlength > 0) {
    length=node->nlength;
    IFE(RqpReparenthesizeCopy(ctrl_rqp,length,ctrl_rqp->Ba+node->nstart,pstring));
    }
  else {
    length=node->length;
    IFE(RqpReparenthesizeCopy(ctrl_rqp,length,ctrl_rqp->request+node->start,pstring));
    }
  *pstring_length=length;
  }
else {
  unsigned char ulabel[DPcPathSize]; int iulabel=0;
  sprintf(label,"%s", RqpOperatorString(node->boolean_operator.name));
  IFE(OgRqpProximityString(&node->boolean_operator,proximity));
  if (proximity[0]) sprintf(label,"%s",proximity); ilabel=strlen(label);
  for (i=0; i<ilabel; i++) {
    ulabel[iulabel++]=0; ulabel[iulabel++]=label[i];
    }
  IFE(RqpReparenthesizeCopy(ctrl_rqp,iulabel,ulabel,pstring));
  *pstring_length=iulabel;
  }

DONE;
}




static int RqpReparenthesizeCopy(struct og_ctrl_rqp *ctrl_rqp, int string_length, unsigned char *string, unsigned char **pstring)
{
char erreur[DOgErrorSize];
IFn(*pstring=malloc(string_length)) {
  sprintf(erreur,"RqpReparenthesizeMalloc: malloc error (%d bytes)",string_length);
  OgErr(ctrl_rqp->herr,erreur); DPcErr;
  }
memcpy(*pstring,string,string_length);
DONE;
}




